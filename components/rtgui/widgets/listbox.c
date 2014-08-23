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

#include <rtgui/widgets/listbox.h>
#include <topwin.h>
#include <rtgui/widgets/panel.h>
#include <rtgui/widgets/scrollbar.h>
#include <rtgui/widgets/window.h>

static rt_bool_t rtgui_listbox_sbar_handle(pvoid wdt, rtgui_event_t* event);

static void _rtgui_listbox_constructor(rtgui_listbox_t *box)
{
	/* set default widget rect and set event handler */
	rtgui_widget_set_event_handler(box,rtgui_listbox_event_handler);
	rtgui_widget_set_onfocus(box, RT_NULL);
	rtgui_widget_set_onunfocus(box, rtgui_listbox_unfocus);
	RTGUI_WIDGET_FLAG(box) |= RTGUI_WIDGET_FLAG_FOCUSABLE;
	if(theme.style == RTGUI_BORDER_UP)
		rtgui_widget_set_border_style(box,RTGUI_BORDER_DOWN);
	else if(theme.style == RTGUI_BORDER_EXTRA)
		rtgui_widget_set_border_style(box,RTGUI_BORDER_SUNKEN);
	
	box->first_item = 0;
	box->now_item = 0;
	box->old_item = 0;
	box->item_count = 0;
	box->item_size	= RTGUI_SEL_H;
	box->item_per_page = 0;
	box->select_fc = theme.blankspace;
	box->select_bc = DarkBlue;
	box->ispopup = RT_FALSE;
	box->widget_link = RT_NULL;

	RTGUI_WIDGET_BC(box) = theme.blankspace;
	RTGUI_WIDGET_TEXTALIGN(box) = RTGUI_ALIGN_LEFT|RTGUI_ALIGN_CENTER_VERTICAL;

	box->items = RT_NULL;
	box->scroll = RT_NULL;
	box->multi_valid = 1;
	box->multi_sel = 0;
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

rtgui_listbox_t* rtgui_listbox_create(pvoid parent, int left,int top,int w,int h)
{
	rtgui_container_t *container;
	rtgui_listbox_t* box = RT_NULL;

	RT_ASSERT(parent != RT_NULL);
	container = RTGUI_CONTAINER(parent);

	box = rtgui_widget_create(RTGUI_LISTBOX_TYPE);
	if(box != RT_NULL)
	{
		rtgui_rect_t rect;
		rtgui_widget_get_rect(container,&rect);
		rtgui_widget_rect_to_device(container, &rect);
		rect.x1 += left;
		rect.y1 += top;
		rect.x2 = rect.x1+w;
		rect.y2 = rect.y1+h;
		rtgui_widget_set_rect(box,&rect);
		rtgui_container_add_child(container, box);

		if(box->scroll == RT_NULL)
		{
			/* create scrollbar */
			rt_uint32_t sLeft,sTop,sWidth=RTGUI_DEFAULT_SB_WIDTH,sLen;
			sLeft = RC_W(rect)-RTGUI_WIDGET_BORDER_SIZE(box)-sWidth;
			sTop = RTGUI_WIDGET_BORDER_SIZE(box);
			sLen = RC_H(rect)-RTGUI_WIDGET_BORDER_SIZE(box)*2;

			box->scroll = rtgui_scrollbar_create(box,sLeft,sTop,sWidth,sLen,RTGUI_VERTICAL);

			if(box->scroll != RT_NULL)
			{
				box->scroll->widget_link = (pvoid)box;
				box->scroll->on_scroll = rtgui_listbox_sbar_handle;
				RTGUI_WIDGET_HIDE(box->scroll);/* default hide scrollbar */
			}
		}
	}

	return box;
}

void rtgui_listbox_set_items(rtgui_listbox_t* box, rtgui_listbox_item_t* items, rt_int16_t count)
{
	rtgui_rect_t rect;
	rt_uint32_t i,h;;

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
		box->items[i].ext_flag = 0;
	}

	box->item_count = count;
	box->now_item = 0;
	box->old_item = 0;

	rtgui_widget_get_rect(box, &rect);

	box->item_per_page = RC_H(rect) / (box->item_size+2);

	if(box->ispopup)/* created by popup */
	{
		if(box->item_count < 5)
			box->item_per_page = count;
		else
			box->item_per_page = 5;

		h = 2+(box->item_size+2)*box->item_per_page;
		rect.y2 = rect.y1+h;
		rtgui_widget_rect_to_device(box,&rect);
		rtgui_widget_set_rect(box,&rect);/* update listbox extent */

		if(box->scroll != RT_NULL) /* update scrollbar extent */
		{
			rtgui_widget_get_rect(box->scroll,&rect);
			rect.y2 = rect.y1+h-RTGUI_WIDGET_BORDER_SIZE(box)*2;
			rtgui_widget_rect_to_device(box->scroll,&rect);
			rtgui_widget_set_rect(box->scroll,&rect);
		}
	}

	if(box->scroll != RT_NULL)/* update scrollbar value */
	{
		if(box->item_count > box->item_per_page)
		{
			RTGUI_WIDGET_SHOW(box->scroll);
			rtgui_scrollbar_set_line_step(box->scroll, 1);
			rtgui_scrollbar_set_page_step(box->scroll, box->item_per_page);
			rtgui_scrollbar_set_range(box->scroll, box->item_count);
		}
		else
		{
			RTGUI_WIDGET_HIDE(box->scroll);
		}
		rtgui_widget_update_clip(box);
	}

}

void rtgui_listbox_destroy(rtgui_listbox_t* box)
{
	/* destroy box */
	rtgui_widget_destroy(box);
}

static const unsigned char lb_ext_flag[]=
{
	0x00,0x10,0x00,0x30,0x00,0x70,0x80,0xE0,0xC1,0xC0,
	0xE3,0x80,0x77,0x00,0x3E,0x00,0x1C,0x00,0x08,0x00,
};

/* draw listbox all item */
void rtgui_listbox_ondraw(rtgui_listbox_t* box)
{
	rtgui_rect_t rect, item_rect, image_rect;
	rt_uint16_t first, i;
	const rtgui_listbox_item_t* item;
	rtgui_dc_t* dc;

	RT_ASSERT(box != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(box);
	if(dc == RT_NULL)return;

	rtgui_widget_get_rect(box, &rect);

	/* draw listbox border */
	rtgui_dc_draw_border(dc, &rect,RTGUI_WIDGET_BORDER_STYLE(box));
	rtgui_rect_inflate(&rect,-RTGUI_WIDGET_BORDER_SIZE(box));
	RTGUI_DC_BC(dc) = theme.blankspace;
	
	if(box->items==RT_NULL)/* not exist items. */
	{
		rtgui_dc_fill_rect(dc, &rect);
	}
	rtgui_rect_inflate(&rect,RTGUI_WIDGET_BORDER_SIZE(box));

	if(box->scroll && !RTGUI_WIDGET_IS_HIDE(box->scroll))
	{
		rect.x2 -= RC_W(box->scroll->parent.extent);
	}

	/* get item base rect */
	item_rect = rect;
	item_rect.x1 += RTGUI_WIDGET_BORDER_SIZE(box);
	item_rect.x2 -= RTGUI_WIDGET_BORDER_SIZE(box);
	item_rect.y1 += RTGUI_WIDGET_BORDER_SIZE(box);
	item_rect.y2 = item_rect.y1 + (2+box->item_size);

	/* get first loc */
	first = box->first_item;
	for(i = 0; i < box->item_per_page; i ++)
	{
		char buf[32];
		rtgui_color_t bc;
		//if(first + i >= box->item_count) break;
		if(i==0)
		{	//仅第一次清第一条区域,后面区域的由后面的语句清除
			bc = RTGUI_DC_BC(dc);
			RTGUI_DC_BC(dc) = theme.blankspace;
			rtgui_dc_fill_rect(dc, &item_rect);
			RTGUI_DC_BC(dc) = bc;
		}
		
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
					RTGUI_DC_BC(dc) = Silver;
					RTGUI_DC_FC(dc) = theme.foreground;
					rtgui_dc_fill_rect(dc, &item_rect);
				}
			}
			item_rect.x1 += RTGUI_MARGIN;

			if(item->image != RT_NULL)
			{
				/* get image base rect */
				image_rect.x1 = 0;
				image_rect.y1 = 0;
				image_rect.x2 = item->image->w;
				image_rect.y2 = item->image->h;
				rtgui_rect_moveto_align(&item_rect, &image_rect, RTGUI_ALIGN_CENTER_VERTICAL);
				rtgui_image_paste(item->image, dc, &image_rect, theme.blankspace);
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
				RTGUI_DC_FC(dc) = theme.foreground;
				rtgui_dc_draw_text(dc, buf, &item_rect);
			}
			
			if(box->multi_sel && item->ext_flag)
			{
				rtgui_rect_t tmprect = rect;
				tmprect.x2 -= RTGUI_MARGIN;
				tmprect.x2 -= RTGUI_WIDGET_BORDER_SIZE(box);
				tmprect.x1 = tmprect.x2 - 12;
				tmprect.y1 = item_rect.y1;
				tmprect.y2 = item_rect.y2;
				rtgui_dc_draw_word(dc, tmprect.x1, tmprect.y1+6, 10, lb_ext_flag);
			}
		
			if(item->image != RT_NULL)
				item_rect.x1 -= (item->image->w + 2);
			item_rect.x1 -= RTGUI_MARGIN;
		}
		/* move to next item position */
		item_rect.y1 += (box->item_size + 2);
		item_rect.y2 += (box->item_size + 2);
		if (item_rect.y2 > rect.y2)
			item_rect.y2 = rect.y2-RTGUI_WIDGET_BORDER_SIZE(box);
		
		bc = RTGUI_DC_BC(dc);
		RTGUI_DC_BC(dc) = theme.blankspace;
		rtgui_dc_fill_rect(dc, &item_rect);
		RTGUI_DC_BC(dc) = bc;
	}

	if(box->scroll && !RTGUI_WIDGET_IS_HIDE(box->scroll))
	{
		rtgui_scrollbar_ondraw(box->scroll);
	}

	rtgui_dc_end_drawing(dc);
}

/* update listbox new/old focus item */
void rtgui_listbox_update(rtgui_listbox_t* box)
{
	const rtgui_listbox_item_t* item;
	rtgui_rect_t rect, item_rect, image_rect;
	rtgui_dc_t* dc;
	char buf[32];

	RT_ASSERT(box != RT_NULL);

	if(RTGUI_WIDGET_IS_HIDE(box))return;
	if(box->items==RT_NULL)return;

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(box);
	if(dc == RT_NULL)return;

	rtgui_widget_get_rect(box, &rect);
	if(box->scroll && !RTGUI_WIDGET_IS_HIDE(box->scroll))
	{
		rect.x2 -= RC_W(box->scroll->parent.extent);
	}

	if((box->old_item >= box->first_item) && /* int front some page */
	        (box->old_item < box->first_item+box->item_per_page) && /* int later some page */
	        (box->old_item != box->now_item)) /* change location */
	{
		/* these condition dispell blinked when drawed */
		item_rect = rect;
		/* get old item's rect */
		item_rect.x1 += RTGUI_WIDGET_BORDER_SIZE(box);
		item_rect.x2 -= RTGUI_WIDGET_BORDER_SIZE(box);
		item_rect.y1 += RTGUI_WIDGET_BORDER_SIZE(box);
		item_rect.y1 += ((box->old_item-box->first_item) % box->item_per_page) * (2 + box->item_size);
		item_rect.y2 = item_rect.y1 + (2+box->item_size);
		if (item_rect.y2 > rect.y2) item_rect.y2 = rect.y2;

		/* draw old item */
		RTGUI_DC_BC(dc) = theme.blankspace;
		RTGUI_DC_FC(dc) = theme.foreground;
		rtgui_dc_fill_rect(dc,&item_rect);

		item_rect.x1 += RTGUI_MARGIN;

		item = &(box->items[box->old_item]);
		if(item->image != RT_NULL)
		{
			image_rect.x1 = 0;
			image_rect.y1 = 0;
			image_rect.x2 = item->image->w;
			image_rect.y2 = item->image->h;
			rtgui_rect_moveto_align(&item_rect, &image_rect, RTGUI_ALIGN_CENTER_VERTICAL);
			rtgui_image_paste(item->image, dc, &image_rect, theme.blankspace);
			item_rect.x1 += item->image->w + 2;
		}
		
		if(box->vindex)
			rt_sprintf(buf, "%d.%s", box->old_item + 1, item->name);
		else
			rt_sprintf(buf, "%s", item->name);
		
		rtgui_dc_draw_text(dc, buf, &item_rect);
		
		if(box->multi_sel && item->ext_flag)
		{
			rtgui_rect_t tmprect = rect;
			tmprect.x2 -= RTGUI_WIDGET_BORDER_SIZE(box);
			tmprect.x2 -= RTGUI_MARGIN;
			tmprect.x1 = tmprect.x2 - 12;
			tmprect.y1 = item_rect.y1;
			tmprect.y2 = item_rect.y2;
			rtgui_dc_draw_word(dc, tmprect.x1, tmprect.y1+6, 10, lb_ext_flag);
		}
	}

	/* draw now item */
	item_rect = rect;
	/* get now item's rect */
	item_rect.x1 += RTGUI_WIDGET_BORDER_SIZE(box);
	item_rect.x2 -= RTGUI_WIDGET_BORDER_SIZE(box);
	item_rect.y1 += RTGUI_WIDGET_BORDER_SIZE(box);
	item_rect.y1 += ((box->now_item-box->first_item) % box->item_per_page) * (2 + box->item_size);
	item_rect.y2 = item_rect.y1 + (2 + box->item_size);
	if (item_rect.y2 > rect.y2-RTGUI_WIDGET_BORDER_SIZE(box)) item_rect.y2 = rect.y2-RTGUI_WIDGET_BORDER_SIZE(box);

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
		RTGUI_DC_BC(dc) = Silver;
		RTGUI_DC_FC(dc) = theme.foreground;
		rtgui_dc_fill_rect(dc, &item_rect);
	}

	item_rect.x1 += RTGUI_MARGIN;

	item = &(box->items[box->now_item]);
	if(item->image != RT_NULL)
	{
		image_rect.x1 = 0;
		image_rect.y1 = 0;
		image_rect.x2 = item->image->w;
		image_rect.y2 = item->image->h;
		rtgui_rect_moveto_align(&item_rect, &image_rect, RTGUI_ALIGN_CENTER_VERTICAL);
		rtgui_image_paste(item->image, dc, &image_rect, theme.blankspace);
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
		RTGUI_DC_FC(dc) = theme.foreground;
		rtgui_dc_draw_text(dc, buf, &item_rect);
	}
	
	if(box->multi_sel && item->ext_flag)
	{
		rtgui_rect_t tmprect = rect;
		tmprect.x2 -= RTGUI_WIDGET_BORDER_SIZE(box);
		tmprect.x2 -= RTGUI_MARGIN;
		tmprect.x1 = tmprect.x2 - 12;
		tmprect.y1 = item_rect.y1;
		tmprect.y2 = item_rect.y2;
		rtgui_dc_draw_word(dc, tmprect.x1, tmprect.y1+6, 10, lb_ext_flag);
	}

	rtgui_dc_end_drawing(dc);
}

static void rtgui_listbox_onmouse(rtgui_listbox_t* box, struct rtgui_event_mouse* emouse)
{
	rtgui_rect_t rect,scroll_rect;

	RT_ASSERT(box != RT_NULL);

	/* get physical extent information */
	rtgui_widget_get_rect(box, &rect);
	rtgui_widget_rect_to_device(box, &rect);
	rtgui_rect_inflate(&rect, -RTGUI_WIDGET_BORDER_SIZE(box));
	scroll_rect = rect;
	if(box->scroll && !RTGUI_WIDGET_IS_HIDE(box->scroll))
	{
		rect.x2 -= RC_W(box->scroll->parent.extent);
		scroll_rect.x1 = scroll_rect.x2 - RC_W(box->scroll->parent.extent);
	}
	

	if(rtgui_rect_contains_point(&rect, emouse->x, emouse->y) == RT_EOK)
	{
		rt_uint16_t i;
		/* set focus */
		rtgui_widget_focus(box);

		if(box->item_count <=0)return;
		i = (emouse->y - rect.y1) / (2 + box->item_size);
_recheck:
		if((i < box->item_count) && (i < box->item_per_page))
		{
			if(emouse->button & RTGUI_MOUSE_BUTTON_DOWN)
			{
				box->old_item = box->now_item;
				/* set selected item */
				box->now_item = box->first_item + i;

				if(box->on_item != RT_NULL)
				{
					box->on_item(box, (rtgui_event_t*)emouse);
				}

				/* down event */
				rtgui_listbox_update(box);
			}
			else if(emouse->button & RTGUI_MOUSE_BUTTON_UP)
			{
				rtgui_listbox_update(box);

				if(box->ispopup && !RTGUI_WIDGET_IS_HIDE(box))
				{
					rtgui_win_t *win;
					RTGUI_WIDGET_HIDE(box);
					box->first_item=0;
					box->now_item = 0;

					win = rtgui_win_get_win_by_widget(box);
					if(win != RT_NULL)
					{
						/* it was in a window box */
						if(rtgui_rect_is_intersect(&(RTGUI_WIDGET_EXTENT(win)),
						                           &(RTGUI_WIDGET_EXTENT(box))) == RT_EOK)
						{
							rtgui_topwin_move(win,RTGUI_WIDGET_EXTENT(win).x1,
							                  RTGUI_WIDGET_EXTENT(win).y1);
							rtgui_widget_focus(win);
							rtgui_widget_update_clip(win);
							rtgui_toplevel_redraw(&(RTGUI_WIDGET_EXTENT(box)));
						}
					}
					else
					{
						rtgui_widget_update_clip(RTGUI_WIDGET_PARENT(box));
						rtgui_widget_update(RTGUI_WIDGET_PARENT(box));
					}

					return;
				}
			}
			if(box->scroll && !RTGUI_WIDGET_IS_HIDE(box))
			{
				if(!RTGUI_WIDGET_IS_HIDE(box->scroll))
					rtgui_scrollbar_set_value(box->scroll,box->first_item);
			}
		}
		else if(box->item_count > 0)
		{
			i = box->item_count-1;
			goto _recheck;
		}
	}
}

rt_bool_t rtgui_listbox_event_handler(pvoid wdt, rtgui_event_t* event)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	rtgui_listbox_t* box = RTGUI_LISTBOX(wdt);

	RT_ASSERT(box != RT_NULL);

	switch(event->type)
	{
	case RTGUI_EVENT_PAINT:
		if(widget->on_draw)
			widget->on_draw(widget, event);
		else
			rtgui_listbox_ondraw(box);
		return RT_FALSE;

	case RTGUI_EVENT_MOUSE_BUTTON:
		if(widget->on_mouseclick != RT_NULL)
		{
			widget->on_mouseclick(widget, event);
		}
		else
		{
			rtgui_listbox_onmouse(box, (struct rtgui_event_mouse*)event);
		}
		//可能还有滚动条,分发...
		rtgui_container_dispatch_mouse_event(box, (struct rtgui_event_mouse*)event);
		return RT_TRUE;

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

					if(box->scroll && !RTGUI_WIDGET_IS_HIDE(box))
					{
						if(!RTGUI_WIDGET_IS_HIDE(box->scroll))
							rtgui_scrollbar_set_value(box->scroll,box->first_item);
					}

					if(box->updown != RT_NULL)
					{
						box->updown(box, event);
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
					if(box->scroll && !RTGUI_WIDGET_IS_HIDE(box))
					{
						if(!RTGUI_WIDGET_IS_HIDE(box->scroll))
							rtgui_scrollbar_set_value(box->scroll,box->first_item);
					}

					if(box->updown != RT_NULL)
					{
						box->updown(box, event);
					}
				}
				break;

			case RTGUIK_RETURN:
				if(box->on_item != RT_NULL)
				{
					box->on_item(box, event);
				}
				
				if(box->multi_sel && box->multi_valid)
				{
					rtgui_listbox_item_t *item;
					item = &(box->items[box->now_item]);
					if(!item->ext_flag)
					{
						item->ext_flag = 1;
						rtgui_listbox_update_loc(box,box->now_item);
					}
				}
				
				if(box->ispopup && !RTGUI_WIDGET_IS_HIDE(box))
				{
					RTGUI_WIDGET_HIDE(box);
					box->first_item=0;
					box->now_item = 0;
					rtgui_widget_update_clip(RTGUI_WIDGET_PARENT(box));
					rtgui_widget_update(RTGUI_WIDGET_PARENT(box));
				}
				break;
			case RTGUIK_BACKSPACE:
				if(box->multi_sel)
				{
					rtgui_listbox_item_t *item;
					item = &(box->items[box->now_item]);
					if(item->ext_flag)
					{
						item->ext_flag = 0;
						rtgui_listbox_update_loc(box,box->now_item);
					}
				}
				break;

			default:
				break;
			}
		}
		return RT_TRUE;
	}
	default:
		return rtgui_container_event_handler(box, event);
	}
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

	if(box->scroll != RT_NULL)
	{
		rtgui_panel_t *panel = rtgui_panel_get();
		if(RTGUI_WIDGET_IS_HIDE(box->scroll))
		{
			if(box->item_count > box->item_per_page)
			{
				RTGUI_WIDGET_SHOW(box->scroll);
				rtgui_scrollbar_set_line_step(box->scroll, 1);
				rtgui_scrollbar_set_page_step(box->scroll, box->item_per_page);
				rtgui_scrollbar_set_range(box->scroll, box->item_count);
			}
			else
			{
				RTGUI_WIDGET_HIDE(box->scroll);
			}

			rtgui_widget_update_clip(box);
			/* if(external_clip_size > 0)
			{
				rt_int32_t i;
				rtgui_rect_t *rect = external_clip_rect;
				for(i=0;i<external_clip_size;i++)
				{
					if(rtgui_rect_is_intersect(rect, &RTGUI_WIDGET_EXTENT(box)) == RT_EOK)
					{
						rtgui_toplevel_update_clip(panel);
						rtgui_toplevel_redraw(&RTGUI_WIDGET_EXTENT(box));
						break;
					}
					rect++;
				}
			}*/
		}
		else
		{
			rtgui_scrollbar_set_range(box->scroll, box->item_count);
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
		box->items[box->item_count].ext_flag = 0;
		box->item_count += 1;
		/* adjust scrollbar value */

		rtgui_listbox_adjust_scrollbar(box);
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
			box->items[i].ext_flag = box->items[i-1].ext_flag;
		}
		/* insert location */
		box->items[item_num+1].name = rt_strdup(item->name);
		box->items[item_num+1].image= item->image;
		box->items[item_num+1].ext_flag = 0;
		rtgui_listbox_adjust_scrollbar(box);
		if(!RTGUI_WIDGET_IS_HIDE(box))
		{
			rtgui_listbox_ondraw(box);
		}
	}
}

void rtgui_listbox_del_item(rtgui_listbox_t* box, rt_int16_t item_num)
{
	rtgui_panel_t *panel = rtgui_panel_get();
	rtgui_listbox_item_t* _items;
	rt_base_t i;

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
			box->items[i].ext_flag = box->items[i+1].ext_flag;
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

	if(box->scroll != RT_NULL && !RTGUI_WIDGET_IS_HIDE(box->scroll))
	{
		rtgui_panel_t *panel = rtgui_panel_get();
		if(box->item_count > box->item_per_page)
		{
			RTGUI_WIDGET_SHOW(box->scroll);
			rtgui_scrollbar_set_range(box->scroll, box->item_count);
			/* set new location at scrollbar */
			rtgui_scrollbar_set_value(box->scroll, box->first_item);
		}
		else
		{
			RTGUI_WIDGET_HIDE(box->scroll);
		}
		rtgui_widget_update_clip(box);
		if(external_clip_size > 0)
		{
			rt_int32_t i;
			rtgui_rect_t *rect = external_clip_rect;
			for(i=0; i<external_clip_size; i++)
			{
				if(rtgui_rect_is_intersect(rect, &RTGUI_WIDGET_EXTENT(box)) == RT_EOK)
				{
					rtgui_toplevel_update_clip(panel);
					rtgui_toplevel_redraw(&RTGUI_WIDGET_EXTENT(box));
					break;
				}
				rect++;
			}
		}
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
void rtgui_listbox_update_loc(rtgui_listbox_t* box, rt_int16_t loc)
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

		if(!RTGUI_WIDGET_IS_HIDE(box->scroll))
		{
			rtgui_scrollbar_set_value(box->scroll, box->first_item);
		}
	}
}

rt_bool_t rtgui_listbox_unfocus(pvoid wdt, rtgui_event_t* event)
{
	rtgui_listbox_t *box = RTGUI_LISTBOX(wdt);
	if(box == RT_NULL)return RT_FALSE;

	if(!RTGUI_WIDGET_IS_FOCUSED(box))
	{
		/* clear focus rectangle */
		rtgui_listbox_update(box);
	}

	if(box->ispopup)
	{
		/* this is a popup listbox ,so it hang on the parent widget */
		rtgui_win_t *win;

		RTGUI_WIDGET_HIDE(box);
		box->first_item=0;
		box->now_item = 0;

		win = rtgui_win_get_win_by_widget(box);
		if(win != RT_NULL)
		{
			/* it in a window box */
			if(rtgui_rect_is_intersect(&(RTGUI_WIDGET_EXTENT(win)),
			                           &(RTGUI_WIDGET_EXTENT(box))) == RT_EOK)
			{
				rtgui_topwin_move(win,RTGUI_WIDGET_EXTENT(win).x1,
				                  RTGUI_WIDGET_EXTENT(win).y1);
			}
			rtgui_widget_focus(win);
			rtgui_widget_update_clip(win);
		}
		rtgui_toplevel_redraw(&(RTGUI_WIDGET_EXTENT(box)));
	}

	return RT_TRUE;
}

static rt_bool_t rtgui_listbox_sbar_handle(pvoid wdt, rtgui_event_t* event)
{
	rtgui_listbox_t *box = RTGUI_LISTBOX(wdt);

	/* adjust first display row when dragging */
	if(box->first_item == box->scroll->value) return RT_FALSE;
	
	box->first_item = box->scroll->value;
	rtgui_listbox_ondraw(box);

	return RT_TRUE;
}

