/*
 * File      : listbox.c
 * This file is part of RTGUI in RT-Thread RTOS
 * COPYRIGHT (C) 2010, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 *
 */

#include <rtgui/widgets/widget.h>
#include <rtgui/widgets/listbox.h>
#include <rtgui/widgets/scrollbar.h>

static rt_bool_t rtgui_listbox_sbar_handle(rtgui_object_t *object, rtgui_event_t* event);

static void _rtgui_listbox_constructor(rtgui_listbox_t *box)
{
	/* set default widget rect and set event handler */
	rtgui_object_set_event_handler(RTGUI_OBJECT(box),rtgui_listbox_event_handler);
	rtgui_widget_set_onfocus(RTGUI_WIDGET(box), RT_NULL);
	rtgui_widget_set_onunfocus(RTGUI_WIDGET(box), rtgui_listbox_unfocus);
	RTGUI_WIDGET_FLAG(box) |= RTGUI_WIDGET_FLAG_FOCUSABLE;
	box->first_item = 0;
	box->now_item = 0;
	box->old_item = 0;
	box->item_count = 0;
	box->item_size	= 20;
	box->item_per_page = 0;
	box->select_fc = white;
	box->select_bc = dark_grey;
	box->widget_link = RT_NULL;

	RTGUI_WIDGET_BACKGROUND(box) = white;
	RTGUI_WIDGET_TEXTALIGN(box) = RTGUI_ALIGN_LEFT|RTGUI_ALIGN_CENTER_VERTICAL;

	box->items = RT_NULL;
	box->scrollbar = RT_NULL;
	box->vindex = 0;
	box->on_item = RT_NULL;
	box->updown = RT_NULL;
}
static void _rtgui_listbox_destructor(rtgui_listbox_t *box)
{
	rtgui_listbox_clear_items(box);
}

DEFINE_CLASS_TYPE(listbox, "listbox",
                  RTGUI_CONTAINER_TYPE,
                  _rtgui_listbox_constructor,
                  _rtgui_listbox_destructor,
                  sizeof(struct rtgui_listbox));

rtgui_listbox_t* rtgui_listbox_create(rtgui_container_t *container, int left,int top,int w,int h,rt_uint32_t style)
{
	rtgui_listbox_t* box = RT_NULL;

	RT_ASSERT(container != RT_NULL);

	box = (rtgui_listbox_t *) rtgui_widget_create(RTGUI_LISTBOX_TYPE);
	if(box != RT_NULL)
	{
		rtgui_rect_t rect;
		rtgui_widget_get_rect(RTGUI_WIDGET(container),&rect);
		rtgui_widget_rect_to_device(RTGUI_WIDGET(container), &rect);
		rect.x1 += left;
		rect.y1 += top;
		rect.x2 = rect.x1+w;
		rect.y2 = rect.y1+h;
		rtgui_widget_set_rect(RTGUI_WIDGET(box),&rect);
		rtgui_container_add_child(container, RTGUI_WIDGET(box));

		rtgui_widget_set_border(RTGUI_WIDGET(box),style);

		if(box->scrollbar == RT_NULL)
		{
			/* create scrollbar */
			rt_uint32_t sLeft,sTop,sWidth=RTGUI_DEFAULT_SB_WIDTH,sLen;
			sLeft = RC_W(rect)-RTGUI_WIDGET_BORDER(box)-sWidth;
			sTop = RTGUI_WIDGET_BORDER(box);
			sLen = RC_H(rect)-RTGUI_WIDGET_BORDER(box)*2;

			box->scrollbar = rtgui_scrollbar_create(RTGUI_CONTAINER(box),sLeft,sTop,sWidth,sLen,RTGUI_VERTICAL);

			if(box->scrollbar != RT_NULL)
			{
				box->scrollbar->widget_link = (rtgui_widget_t*)box;
				box->scrollbar->on_scroll = rtgui_listbox_sbar_handle;
				RTGUI_WIDGET_HIDE(box->scrollbar);/* default hide scrollbar */
			}
		}
	}

	return box;
}

void rtgui_listbox_set_items(rtgui_listbox_t* box, const rtgui_listbox_item_t* items, rt_int16_t count)
{
	rtgui_rect_t rect;
	rt_uint32_t i;

	RT_ASSERT(box != RT_NULL);

	if(box->items != RT_NULL)
	{
		rt_free(box->items);
		box->items = RT_NULL;
	}
	/* support add/del/append, dynamic memory */
	box->items = (rtgui_listbox_item_t*) rt_malloc(sizeof(rtgui_listbox_item_t)*count);
	if(box->items == RT_NULL) return;

	for(i=0; i<count; i++)
	{
		box->items[i].name = rt_strdup(items[i].name);
		box->items[i].image = items[i].image;
	}

	box->item_count = count;
	box->now_item = 0;
	box->old_item = 0;

	rtgui_widget_get_rect(RTGUI_WIDGET(box), &rect);

	box->item_per_page = RC_H(rect) / (box->item_size+2);

	if(box->scrollbar != RT_NULL)/* update scrollbar value */
	{
		if(box->item_count > box->item_per_page)
		{
			RTGUI_WIDGET_UNHIDE(box->scrollbar);
			rtgui_scrollbar_set_line_step(box->scrollbar, 1);
			rtgui_scrollbar_set_page_step(box->scrollbar, box->item_per_page);
			rtgui_scrollbar_set_range(box->scrollbar, box->item_count);
		}
		else
		{
			RTGUI_WIDGET_HIDE(box->scrollbar);
		}
		rtgui_widget_update_clip(RTGUI_WIDGET(box));
	}

}

void rtgui_listbox_destroy(rtgui_listbox_t* box)
{
	/* destroy box */
	rtgui_widget_destroy(RTGUI_WIDGET(box));
}

static const unsigned char lb_ext_flag[]=
{0x00,0x10,0x00,0x30,0x00,0x70,0x80,0xE0,0xC1,0xC0,0xE3,0x80,0x77,0x00,0x3E,0x00,0x1C,0x00,0x08,0x00};

/* draw listbox all item */
void rtgui_listbox_ondraw(rtgui_listbox_t* box)
{
	rtgui_rect_t rect, item_rect, image_rect;
	rt_uint16_t first, i;
	const rtgui_listbox_item_t* item;
	struct rtgui_dc* dc;

	RT_ASSERT(box != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(RTGUI_WIDGET(box));
	if(dc == RT_NULL)return;

	rtgui_widget_get_rect(RTGUI_WIDGET(box), &rect);

	/* draw listbox border */
	rtgui_dc_draw_border(dc, &rect,RTGUI_WIDGET_BORDER_STYLE(box));
	rtgui_rect_inflate(&rect,-RTGUI_WIDGET_BORDER(box));
	RTGUI_DC_BC(dc) = white;
	
	if(box->items==RT_NULL)/* not exist items. */
	{
		rtgui_dc_fill_rect(dc, &rect);
	}
	rtgui_rect_inflate(&rect,RTGUI_WIDGET_BORDER(box));

	if(box->scrollbar && !RTGUI_WIDGET_IS_HIDE(box->scrollbar))
	{
		rect.x2 -= RC_W(box->scrollbar->parent.extent);
	}

	/* get item base rect */
	item_rect = rect;
	item_rect.x1 += RTGUI_WIDGET_BORDER(box);
	item_rect.x2 -= RTGUI_WIDGET_BORDER(box);
	item_rect.y1 += RTGUI_WIDGET_BORDER(box);
	item_rect.y2 = item_rect.y1 + (2+box->item_size);

	/* get first loc */
	first = box->first_item;
	for(i = 0; i < box->item_per_page; i ++)
	{
		char buf[32];
		rtgui_color_t bc;
		
		bc = RTGUI_DC_BC(dc);
		RTGUI_DC_BC(dc) = white;
		rtgui_dc_fill_rect(dc, &item_rect);
		RTGUI_DC_BC(dc) = bc;
		
		if(first + i < box->item_count)
		{
			item = &(box->items[first + i]);

			if(first + i == box->now_item)
			{
				//draw current item
				if(RTGUI_WIDGET_IS_FOCUSED(box))
				{
					RTGUI_DC_FC(dc) = box->select_fc;
					RTGUI_DC_BC(dc) = box->select_bc;
					rtgui_dc_fill_rect(dc, &item_rect);
					rtgui_dc_draw_focus_rect(dc, &item_rect);
				}
				else
				{
					RTGUI_DC_BC(dc) = light_grey;
					RTGUI_DC_FC(dc) = black;
					rtgui_dc_fill_rect(dc, &item_rect);
				}
			}
			item_rect.x1 += RTGUI_WIDGET_DEFAULT_MARGIN;

			if(item->image != RT_NULL)
			{
				/* get image base rect */
				image_rect.x1 = 0;
				image_rect.y1 = 0;
				image_rect.x2 = item->image->w;
				image_rect.y2 = item->image->h;
				rtgui_rect_moveto_align(&item_rect, &image_rect, RTGUI_ALIGN_CENTER_VERTICAL);
				rtgui_image_blit(item->image, dc, &image_rect);
				item_rect.x1 += item->image->w + 2;
			}
			
			if(box->vindex)
				rt_sprintf(buf, "%d.%s", first+i+1, item->name);
			else
				rt_sprintf(buf, "%s", item->name);
			
			/* draw text */
			if(first + i == box->now_item && RTGUI_WIDGET_IS_FOCUSED(box))
			{
				RTGUI_DC_FC(dc) = box->select_fc;
				rtgui_dc_draw_text(dc, buf, &item_rect);
			}
			else
			{
				RTGUI_DC_FC(dc) = black;
				rtgui_dc_draw_text(dc, buf, &item_rect);
			}

			if(item->image != RT_NULL)
				item_rect.x1 -= (item->image->w + 2);
			item_rect.x1 -= RTGUI_WIDGET_DEFAULT_MARGIN;
		}
		/* move to next item position */
		item_rect.y1 += (box->item_size + 2);
		item_rect.y2 += (box->item_size + 2);
		if (item_rect.y2 > rect.y2)
			item_rect.y2 = rect.y2;
	}

	if(box->scrollbar && !RTGUI_WIDGET_IS_HIDE(box->scrollbar))
	{
		rtgui_scrollbar_ondraw(box->scrollbar);
	}

	rtgui_dc_end_drawing(dc);
}

/* update listbox new/old focus item */
void rtgui_listbox_update(rtgui_listbox_t* box)
{
	const rtgui_listbox_item_t* item;
	rtgui_rect_t rect, item_rect, image_rect;
	struct rtgui_dc* dc;
	char buf[32];

	RT_ASSERT(box != RT_NULL);

	if(RTGUI_WIDGET_IS_HIDE(box))return;
	if(box->items==RT_NULL)return;

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(RTGUI_WIDGET(box));
	if(dc == RT_NULL)return;

	rtgui_widget_get_rect(RTGUI_WIDGET(box), &rect);
	if(box->scrollbar && !RTGUI_WIDGET_IS_HIDE(box->scrollbar))
	{
		rect.x2 -= RC_W(box->scrollbar->parent.extent);
	}

	if((box->old_item >= box->first_item) && /* int front some page */
	        (box->old_item < box->first_item+box->item_per_page) && /* int later some page */
	        (box->old_item != box->now_item)) /* change location */
	{
		/* these condition dispell blinked when drawed */
		item_rect = rect;
		/* get old item's rect */
		item_rect.x1 += RTGUI_WIDGET_BORDER(box);
		item_rect.x2 -= RTGUI_WIDGET_BORDER(box);
		item_rect.y1 += RTGUI_WIDGET_BORDER(box);
		item_rect.y1 += ((box->old_item-box->first_item) % box->item_per_page) * (2 + box->item_size);
		item_rect.y2 = item_rect.y1 + (2+box->item_size);
		if (item_rect.y2 > rect.y2) item_rect.y2 = rect.y2;

		/* draw old item */
		RTGUI_DC_BC(dc) = white;
		RTGUI_DC_FC(dc) = black;
		rtgui_dc_fill_rect(dc,&item_rect);

		item_rect.x1 += RTGUI_WIDGET_DEFAULT_MARGIN;

		item = &(box->items[box->old_item]);
		if(item->image != RT_NULL)
		{
			image_rect.x1 = 0;
			image_rect.y1 = 0;
			image_rect.x2 = item->image->w;
			image_rect.y2 = item->image->h;
			rtgui_rect_moveto_align(&item_rect, &image_rect, RTGUI_ALIGN_CENTER_VERTICAL);
			rtgui_image_blit(item->image, dc, &image_rect);
			item_rect.x1 += item->image->w + 2;
		}
		
		if(box->vindex)
			rt_sprintf(buf, "%d.%s", box->old_item + 1, item->name);
		else
			rt_sprintf(buf, "%s", item->name);
		
		rtgui_dc_draw_text(dc, buf, &item_rect);
	}

	/* draw now item */
	item_rect = rect;
	/* get now item's rect */
	item_rect.x1 += RTGUI_WIDGET_BORDER(box);
	item_rect.x2 -= RTGUI_WIDGET_BORDER(box);
	item_rect.y1 += RTGUI_WIDGET_BORDER(box);
	item_rect.y1 += ((box->now_item-box->first_item) % box->item_per_page) * (2 + box->item_size);
	item_rect.y2 = item_rect.y1 + (2 + box->item_size);
	if (item_rect.y2 > rect.y2) item_rect.y2 = rect.y2;

	/* draw current item */
	if(RTGUI_WIDGET_IS_FOCUSED(box))
	{
		RTGUI_DC_FC(dc) = box->select_fc;
		RTGUI_DC_BC(dc) = box->select_bc;
		rtgui_dc_fill_rect(dc, &item_rect);
		rtgui_dc_draw_focus_rect(dc, &item_rect); /* draw focus rect */
	}
	else
	{
		RTGUI_DC_BC(dc) = light_grey;
		RTGUI_DC_FC(dc) = black;
		rtgui_dc_fill_rect(dc, &item_rect);
	}

	item_rect.x1 += RTGUI_WIDGET_DEFAULT_MARGIN;

	item = &(box->items[box->now_item]);
	if(item->image != RT_NULL)
	{
		image_rect.x1 = 0;
		image_rect.y1 = 0;
		image_rect.x2 = item->image->w;
		image_rect.y2 = item->image->h;
		rtgui_rect_moveto_align(&item_rect, &image_rect, RTGUI_ALIGN_CENTER_VERTICAL);
		rtgui_image_blit(item->image, dc, &image_rect);
		item_rect.x1 += (item->image->w + 2);
	}
	
	if(box->vindex)
		rt_sprintf(buf, "%d.%s", box->now_item + 1, item->name);
	else
		rt_sprintf(buf, "%s", item->name);
		
	if(RTGUI_WIDGET_IS_FOCUSED(box))
	{
		RTGUI_DC_FC(dc) = box->select_fc;
		rtgui_dc_draw_text(dc, buf, &item_rect);
	}
	else
	{
		RTGUI_DC_FC(dc) = black;
		rtgui_dc_draw_text(dc, buf, &item_rect);
	}

	rtgui_dc_end_drawing(dc);
}

static void rtgui_listbox_onmouse(rtgui_listbox_t* box, struct rtgui_event_mouse* emouse)
{
	rtgui_rect_t rect;

	RT_ASSERT(box != RT_NULL);

	/* get physical extent information */
	rtgui_widget_get_rect(RTGUI_WIDGET(box), &rect);
	if(box->scrollbar && !RTGUI_WIDGET_IS_HIDE(box->scrollbar))
		rect.x2 -= RC_W(box->scrollbar->parent.extent);

	if((rtgui_region_contains_point(&RTGUI_WIDGET(box)->clip, emouse->x, emouse->y,&rect) == RT_EOK))
	{
		rt_uint16_t i;
		/* set focus */
		rtgui_widget_focus(RTGUI_WIDGET(box));

		if(box->item_count <=0)return;
		i = (emouse->y - rect.y1) / (2 + box->item_size);

		if((i < box->item_count) && (i < box->item_per_page))
		{
			if(emouse->button & RTGUI_MOUSE_BUTTON_DOWN)
			{
				box->old_item = box->now_item;
				/* set selected item */
				box->now_item = box->first_item + i;

				if(box->on_item != RT_NULL)
				{
					box->on_item(RTGUI_OBJECT(box), (rtgui_event_t*)emouse);
				}

				/* down event */
				rtgui_listbox_update(box);
			}
			else if(emouse->button & RTGUI_MOUSE_BUTTON_UP)
			{
				rtgui_listbox_update(box);
			}
			if(box->scrollbar && !RTGUI_WIDGET_IS_HIDE(box))
			{
				if(!RTGUI_WIDGET_IS_HIDE(box->scrollbar))
					rtgui_scrollbar_set_value(box->scrollbar,box->first_item);
			}
		}
	}
}

rt_bool_t rtgui_listbox_event_handler(rtgui_object_t *object, rtgui_event_t* event)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(object);
	rtgui_listbox_t* box = RTGUI_LISTBOX(object);

	RT_ASSERT(box != RT_NULL);

	switch(event->type)
	{
	case RTGUI_EVENT_PAINT:
#ifndef RTGUI_USING_SMALL_SIZE
		if(widget->on_draw)
			widget->on_draw(widget, event);
		else
#endif
			rtgui_listbox_ondraw(box);
		return RT_FALSE;

	case RTGUI_EVENT_MOUSE_BUTTON:
#ifndef RTGUI_USING_SMALL_SIZE
		if(widget->on_mouseclick != RT_NULL)
		{
			widget->on_mouseclick(widget, event);
		}
		else
#endif
		{
			rtgui_listbox_onmouse(box, (struct rtgui_event_mouse*)event);
		}
		rtgui_container_event_handler(RTGUI_OBJECT(box), event);
		break;

	case RTGUI_EVENT_KBD:
	{
		struct rtgui_event_kbd *ekbd = (struct rtgui_event_kbd*)event;

		if((RTGUI_KBD_IS_DOWN(ekbd)) && (box->item_count > 0))
		{
			switch(ekbd->key)
			{
			case RTGUIK_UP:
				if(box->now_item > 0)
				{
					box->old_item = box->now_item;
					box->now_item --;
					if(box->now_item < box->first_item)
					{
						if(box->first_item)box->first_item--;
						rtgui_listbox_ondraw(box);
					}
					else
					{
						rtgui_listbox_update(box);
					}

					if(box->scrollbar && !RTGUI_WIDGET_IS_HIDE(box))
					{
						if(!RTGUI_WIDGET_IS_HIDE(box->scrollbar))
							rtgui_scrollbar_set_value(box->scrollbar,box->first_item);
					}

					if(box->updown != RT_NULL)
					{
						box->updown(RTGUI_OBJECT(box), event);
					}
				}
				break;

			case RTGUIK_DOWN:
				if(box->now_item < box->item_count - 1)
				{
					box->old_item = box->now_item;
					box->now_item ++;
					if(box->now_item >= box->first_item+box->item_per_page)
					{
						box->first_item++;
						rtgui_listbox_ondraw(box);
					}
					else
					{
						rtgui_listbox_update(box);
					}
					if(box->scrollbar && !RTGUI_WIDGET_IS_HIDE(box))
					{
						if(!RTGUI_WIDGET_IS_HIDE(box->scrollbar))
							rtgui_scrollbar_set_value(box->scrollbar,box->first_item);
					}

					if(box->updown != RT_NULL)
					{
						box->updown(RTGUI_OBJECT(box), event);
					}
				}
				break;

			case RTGUIK_RETURN:
				if(box->on_item != RT_NULL)
				{
					box->on_item(RTGUI_OBJECT(box), event);
				}
				break;
			case RTGUIK_BACKSPACE:
				break;

			default:
				break;
			}
		}
		return RT_TRUE;
	}
	case RTGUI_EVENT_SHOW:
		rtgui_widget_onshow(object, event);
		break;
	case RTGUI_EVENT_HIDE:
		rtgui_widget_onhide(object, event);
		break;
	default:
		return rtgui_container_event_handler(RTGUI_OBJECT(box), event);
	}

	/* use box event handler */
	return RT_FALSE;
}

void rtgui_listbox_set_onitem(rtgui_listbox_t* box, rtgui_event_handler_ptr func)
{
	if(box == RT_NULL) return;

	box->on_item = func;
}

void rtgui_listbox_set_updown(rtgui_listbox_t* box, rtgui_event_handler_ptr func)
{
	if(box == RT_NULL) return;

	box->updown = func;
}
/* adjust&update scrollbar widget value */
void rtgui_listbox_adjust_scrollbar(rtgui_listbox_t* box)
{
	RT_ASSERT(box != RT_NULL);

	if(box->scrollbar != RT_NULL)
	{
		if(RTGUI_WIDGET_IS_HIDE(box->scrollbar))
		{
			if(box->item_count > box->item_per_page)
			{
				RTGUI_WIDGET_UNHIDE(box->scrollbar);
				rtgui_scrollbar_set_line_step(box->scrollbar, 1);
				rtgui_scrollbar_set_page_step(box->scrollbar, box->item_per_page);
				rtgui_scrollbar_set_range(box->scrollbar, box->item_count);
			}
			else
			{
				RTGUI_WIDGET_HIDE(box->scrollbar);
			}

			rtgui_widget_update_clip(RTGUI_WIDGET(box));
		}
		else
		{
			rtgui_scrollbar_set_range(box->scrollbar, box->item_count);
		}
	}
}

void rtgui_listbox_add_item(rtgui_listbox_t* box,rtgui_listbox_item_t* item, rt_bool_t update)
{
	rtgui_listbox_item_t* _items;
	RT_ASSERT(box != RT_NULL);

	if(box->item_count==0)
	{
		rtgui_listbox_set_items(box, item, 1);
		if(!RTGUI_WIDGET_IS_HIDE(box) && update)
		{
			rtgui_listbox_ondraw(box);
		}
		return;
	}

	_items = rt_realloc(box->items,sizeof(rtgui_listbox_item_t)*(box->item_count+1));

	if(_items != RT_NULL)
	{
		box->items = _items;
		box->items[box->item_count].name = rt_strdup(item->name);
		box->items[box->item_count].image= item->image;
		box->item_count += 1;
		/* adjust scrollbar value */

		rtgui_listbox_adjust_scrollbar(box);
		rt_kprintf("box is %s\n",RTGUI_WIDGET_IS_HIDE(box)?"show":"hide");
		if(!RTGUI_WIDGET_IS_HIDE(box) && update)
		{
			rtgui_listbox_ondraw(box);
		}
	}
}

void rtgui_listbox_insert_item(rtgui_listbox_t* box, rtgui_listbox_item_t* item, rt_int16_t item_num)
{
	int i=0;
	rtgui_listbox_item_t* _items;
	RT_ASSERT(box != RT_NULL);

	if(box->item_count==0)
	{
		rtgui_listbox_set_items(box, item, 1);
		if(!RTGUI_WIDGET_IS_HIDE(box))
		{
			rtgui_listbox_ondraw(box);
		}
		return;
	}

	if(item_num >= box->item_count)
	{
		/* call add-item function */
		rtgui_listbox_add_item(box, item, 1);
		return;
	}

	box->item_count += 1;
	/* adjust items */
	_items = rt_realloc(box->items,sizeof(rtgui_listbox_item_t)*(box->item_count));

	if(_items != RT_NULL)
	{
		box->items = _items;
		/* move other item */
		for(i=box->item_count-1; i>(item_num+1); i--)
		{
			box->items[i].name = box->items[i-1].name;
			box->items[i].image = box->items[i-1].image;
		}
		/* insert location */
		box->items[item_num+1].name = rt_strdup(item->name);
		box->items[item_num+1].image= item->image;
		rtgui_listbox_adjust_scrollbar(box);
		if(!RTGUI_WIDGET_IS_HIDE(box))
		{
			rtgui_listbox_ondraw(box);
		}
	}
}

void rtgui_listbox_del_item(rtgui_listbox_t* box, rt_int16_t item_num)
{
	rtgui_listbox_item_t* _items;
	int i;

	if(box == RT_NULL) return;
	if(box->item_count==0) return;
	if(box->items == RT_NULL) return;

	if(box->items[item_num].name != RT_NULL)
	{
		rt_free(box->items[item_num].name);
		box->items[item_num].name = RT_NULL;
		/* whether free image together? it maybe used other! */
	}

	if(box->item_count > 1)
	{
		for(i=item_num; i<box->item_count-1; i++)
		{
			box->items[i].name = box->items[i+1].name;
			box->items[i].image = box->items[i+1].image;
		}
		box->item_count -= 1;
		if(box->now_item >= box->item_count)
		{
			box->now_item = box->item_count-1;
		}
		if((box->now_item-box->first_item) < box->item_per_page)
		{
			if(box->first_item > 0) box->first_item--;
		}
	}
	else
	{
		box->item_count = 0;
		box->first_item = 0;
		box->now_item = 0;
	}

	if(box->item_count > 0)
	{
		/* adjust items memory */
		_items = rt_realloc(box->items,sizeof(rtgui_listbox_item_t)*(box->item_count));
		if(_items != RT_NULL)
		{
			box->items = _items;
		}
	}
	else
	{
		/* set null. */
		if(box->items != RT_NULL) rt_free(box->items);
		box->items = RT_NULL;
	}

	if(box->scrollbar != RT_NULL && !RTGUI_WIDGET_IS_HIDE(box->scrollbar))
	{
		if(box->item_count > box->item_per_page)
		{
			RTGUI_WIDGET_UNHIDE(box->scrollbar);
			rtgui_scrollbar_set_range(box->scrollbar, box->item_count);
			/* set new location at scrollbar */
			rtgui_scrollbar_set_value(box->scrollbar, box->first_item);
		}
		else
		{
			RTGUI_WIDGET_HIDE(box->scrollbar);
		}
		rtgui_widget_update_clip(RTGUI_WIDGET(box));
	}

	if(item_num >= box->first_item && item_num <= (box->first_item+box->item_per_page))
	{
		rtgui_listbox_ondraw(box);
	}
}

rt_uint32_t rtgui_listbox_get_count(rtgui_listbox_t* box)
{
	return box->item_count;
}

void rtgui_listbox_clear_items(rtgui_listbox_t* box)
{
	int i;
	rtgui_listbox_item_t* item=RT_NULL;

	if(box->items != RT_NULL)
	{
		for(i=0; i<box->item_count; i++)
		{
			item = &(box->items[i]);
			if(item->name != RT_NULL)
			{
				rt_free(item->name);
				item->name = RT_NULL;
			}
			item->image = RT_NULL;
		}
		rt_free(box->items);
		box->items = RT_NULL;
		box->item_count = 0;
		box->first_item = 0;
		box->now_item = 0;
		box->old_item = 0;
	}
}

/* update listbox with assign row */
void rtgui_listbox_update_current(rtgui_listbox_t* box, rt_int16_t loc)
{
	RT_ASSERT(box != RT_NULL);

	if(box != RT_NULL)
	{
		if(loc > (box->item_count-1)) return;
		if(box->item_count > box->item_per_page)
		{
			if((loc+box->item_per_page) > (box->item_count-1))
			{
				box->now_item = loc;
				box->old_item = loc;
				box->first_item = box->item_count-box->item_per_page;
			}
			else
			{
				box->now_item = loc;
				box->old_item = loc;
				box->first_item = loc;
			}
		}
		else
		{
			box->now_item = loc;
		}

		rtgui_listbox_ondraw(box);

		if(!RTGUI_WIDGET_IS_HIDE(box->scrollbar))
		{
			rtgui_scrollbar_set_value(box->scrollbar, box->first_item);
		}
	}
}

void rtgui_listbox_set_current_item(rtgui_listbox_t *box, int index)
{
	RT_ASSERT(box != RT_NULL);

	if (index != box->now_item)
	{
		int old_item;

		old_item = box->now_item;
		box->now_item = index;

		rtgui_listbox_update_current(box, old_item);
	}
}
RTM_EXPORT(rtgui_listbox_set_current_item);

rt_bool_t rtgui_listbox_unfocus(rtgui_object_t *object, rtgui_event_t* event)
{
	rtgui_listbox_t *box = RTGUI_LISTBOX(object);
	RT_ASSERT(box != RT_NULL);

	if(!RTGUI_WIDGET_IS_FOCUSED(box))
	{
		/* clear focus rectangle */
		rtgui_listbox_update(box);
	}

	return RT_TRUE;
}

static rt_bool_t rtgui_listbox_sbar_handle(rtgui_object_t* object, rtgui_event_t* event)
{
	rtgui_listbox_t *box = RTGUI_LISTBOX(object);
	
	/* adjust first display row when dragging */
	if(box->first_item == box->scrollbar->value) return RT_FALSE;
	
	box->first_item = box->scrollbar->value;
	rtgui_listbox_ondraw(box);

	return RT_TRUE;
}

