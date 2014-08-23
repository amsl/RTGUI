/*
 * File      : combobox.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 *
 */
#include <rtgui/widgets/scrollbar.h>
#include <rtgui/widgets/combobox.h>
#include <rtgui/widgets/view.h>

static void _rtgui_combo_constructor(rtgui_combo_t *cbo)
{
	RTGUI_WIDGET_FLAG(cbo) |= RTGUI_WIDGET_FLAG_FOCUSABLE;
	cbo->style = RTGUI_COMBO_STYLE_DOWNARROW_UP;
	cbo->lbox = RT_NULL;
	cbo->tbox = RT_NULL;
	if(theme.style == RTGUI_BORDER_UP)
		rtgui_widget_set_border_style(cbo,RTGUI_BORDER_DOWN);
	else if(theme.style == RTGUI_BORDER_EXTRA)
		rtgui_widget_set_border_style(cbo,RTGUI_BORDER_SUNKEN);
	rtgui_widget_set_event_handler(cbo, rtgui_combo_event_handler);

	RTGUI_WIDGET_BC(cbo) = theme.blankspace;
	/* set default text align */
	RTGUI_WIDGET_TEXTALIGN(cbo) = RTGUI_ALIGN_CENTER_VERTICAL;
	cbo->on_selected = RT_NULL;
}

static void _rtgui_combo_destructor(rtgui_combo_t *cbo)
{
}

DEFINE_CLASS_TYPE(combo, "combo",
	RTGUI_CONTAINER_TYPE,
	_rtgui_combo_constructor,
	_rtgui_combo_destructor,
	sizeof(struct rtgui_combo));

rtgui_combo_t* rtgui_combo_create(pvoid parent,const char* text,int left,int top,int w,int h)
{
	rtgui_container_t *container;
	rtgui_combo_t* cbo;

	RT_ASSERT(parent != RT_NULL);
	container = RTGUI_CONTAINER(parent);

	cbo = rtgui_widget_create(RTGUI_COMBO_TYPE);
	if(cbo != RT_NULL)
	{
		rtgui_rect_t rect;

		rtgui_widget_get_rect(container, &rect);
		rtgui_widget_rect_to_device(container,&rect);
		rect.x1 += left;
		rect.y1 += top;
		rect.x2 = rect.x1+w;
		rect.y2 = rect.y1+RTGUI_COMBO_HEIGHT;
		rtgui_widget_set_rect(cbo,&rect);

		rtgui_container_add_child(container, cbo);

		if(cbo->tbox == RT_NULL)
		{
			int tw,th;
			tw = w-RTGUI_COMBO_BUTTON_WIDTH-RTGUI_WIDGET_BORDER_SIZE(cbo)*2;
			th = h-RTGUI_WIDGET_BORDER_SIZE(cbo)*2;
			cbo->tbox = rtgui_textbox_create(cbo,text,
			                                 RTGUI_WIDGET_BORDER_SIZE(cbo),
			                                 RTGUI_WIDGET_BORDER_SIZE(cbo),
			                                 tw,th,RTGUI_TEXTBOX_NONE);

			if(cbo->tbox == RT_NULL) return RT_NULL;
			rtgui_widget_set_border_style(cbo->tbox,RTGUI_BORDER_NONE);
			cbo->tbox->isedit = RT_FALSE;/* default combo textbox cannot edit */
		}
		if(cbo->lbox == RT_NULL)
		{
			rtgui_point_t point;
			rt_uint32_t mleft,mtop,mwidth;

			rtgui_widget_get_position(cbo, &point);
			mleft = point.x;
			mtop = point.y+RTGUI_COMBO_HEIGHT;
			mwidth = rtgui_widget_get_width(cbo);

			/* create pull down listbox */
			cbo->lbox = rtgui_listbox_create(parent,mleft,mtop,mwidth,4);
			if(cbo->lbox == RT_NULL)return RT_NULL;
			rtgui_widget_set_border_style(cbo->lbox,RTGUI_BORDER_SIMPLE);
			cbo->lbox->ispopup = RT_TRUE;
			cbo->lbox->widget_link = cbo;
			rtgui_listbox_set_onitem(cbo->lbox,rtgui_combo_onitem);

			RTGUI_WIDGET_HIDE(cbo->lbox);
		}
	}

	return cbo;
}

void rtgui_combo_destroy(rtgui_combo_t* cbo)
{
	rtgui_widget_destroy(cbo);
}

const rt_uint8_t combo_down_bmp[]=
{
	0xFE,0x7C,0x38,0x10,
};

void rtgui_combo_ondraw(rtgui_combo_t *cbo)
{
	rtgui_rect_t rect;
	rtgui_dc_t* dc;

	RT_ASSERT(cbo != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(cbo);
	if(dc == RT_NULL)return;

	rtgui_widget_get_rect(cbo, &rect);

	rtgui_rect_inflate(&rect,-RTGUI_WIDGET_BORDER_SIZE(cbo));
	if(RTGUI_WIDGET_IS_ENABLE(cbo))
		RTGUI_DC_BC(dc) = theme.blankspace;
	else
		RTGUI_DC_BC(dc) = theme.background;
	rtgui_dc_fill_rect(dc, &rect);

	rtgui_rect_inflate(&rect,RTGUI_WIDGET_BORDER_SIZE(cbo));
	rtgui_dc_draw_border(dc, &rect,RTGUI_WIDGET_BORDER_STYLE(cbo));

	if(RC_H(rect)<RTGUI_COMBO_HEIGHT)return;

	/* draw downarrow button */
	rect.x1 = rect.x2-RTGUI_COMBO_BUTTON_WIDTH;
	RTGUI_DC_BC(dc) = theme.background;
	rtgui_dc_fill_rect(dc, &rect);

	if(cbo->style & RTGUI_COMBO_STYLE_DOWNARROW_UP)
	{
		rtgui_dc_draw_border(dc, &rect, theme.style);
		rtgui_dc_draw_byte(dc,rect.x1+(rect.x2-rect.x1-7)/2, rect.y1+(rect.y2-rect.y1-4)/2, 4, combo_down_bmp);
	}
	else if(cbo->style & RTGUI_COMBO_STYLE_DOWNARROW_DOWN)
	{
		rtgui_dc_draw_border(dc, &rect, theme.style);
		rtgui_dc_draw_byte(dc,rect.x1+(rect.x2-rect.x1-7)/2+1, rect.y1+(rect.y2-rect.y1-4)/2+1, 4, combo_down_bmp);
	}

	if(cbo->tbox != RT_NULL)
	{
		RTGUI_DC_FC(dc) = theme.foreground;
		rtgui_textbox_ondraw(cbo->tbox);
	}

	rtgui_dc_end_drawing(dc);
}

void rtgui_combo_draw_downarrow(rtgui_combo_t *cbo)
{
	rtgui_rect_t rect;
	rtgui_dc_t* dc;

	RT_ASSERT(cbo != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(cbo);
	if(dc == RT_NULL)return;

	rtgui_widget_get_rect(cbo, &rect);

	rect.x1 = rect.x2-RTGUI_COMBO_BUTTON_WIDTH-RTGUI_WIDGET_BORDER_SIZE(cbo);
	rect.y1 += RTGUI_WIDGET_BORDER_SIZE(cbo);
	rect.x2 -= RTGUI_WIDGET_BORDER_SIZE(cbo);
	rect.y2 -= RTGUI_WIDGET_BORDER_SIZE(cbo);
	RTGUI_DC_BC(dc) = theme.background;
	rtgui_dc_fill_rect(dc, &rect);

	if(cbo->style & RTGUI_COMBO_STYLE_DOWNARROW_UP)
	{
		rtgui_dc_draw_border(dc, &rect,RTGUI_WIDGET_BORDER_STYLE(cbo));
		rtgui_dc_draw_byte(dc,rect.x1+4, rect.y1+8, 4, combo_down_bmp);
	}
	else if(cbo->style & RTGUI_COMBO_STYLE_DOWNARROW_DOWN)
	{
		rtgui_dc_draw_border(dc, &rect,RTGUI_WIDGET_BORDER_STYLE(cbo));
		rtgui_dc_draw_byte(dc,rect.x1+5, rect.y1+9, 4, combo_down_bmp);
	}

	rtgui_dc_end_drawing(dc);
}

void rtgui_combo_set_onitem(rtgui_combo_t* cbo, rtgui_event_handler_ptr func)
{
	if(cbo == RT_NULL) return;

	if(cbo->lbox != RT_NULL)
	{
		rtgui_listbox_set_onitem(cbo->lbox,func);
	}
}

rt_uint32_t rtgui_combo_get_select(rtgui_combo_t* cbo)
{
	return cbo->lbox->now_item;
}

char* rtgui_combo_get_string(rtgui_combo_t* cbo)
{
	if(cbo->lbox->items != RT_NULL)
	{
		return cbo->lbox->items[cbo->lbox->now_item].name;
	}

	return RT_NULL;
}

/* return item index */
rt_bool_t rtgui_combo_onitem(pvoid wdt, rtgui_event_t* event)
{
	rtgui_listbox_t* box = RTGUI_LISTBOX(wdt);

	if(box != RT_NULL && box->ispopup)
	{
		char *str;
		rtgui_combo_t* cbo=box->widget_link;
		if(cbo==RT_NULL)return RT_FALSE;

		str = rtgui_combo_get_string(cbo);

		rtgui_textbox_set_value(cbo->tbox,str);
	}

	return RT_TRUE;
}

void rtgui_combo_get_downarrow_rect(rtgui_combo_t* cbo, rtgui_rect_t* rect)
{
	rtgui_widget_get_rect(cbo, rect);
	rect->x1 = rect->x2 - RTGUI_COMBO_BUTTON_WIDTH;
}

void rtgui_combo_add_string(rtgui_combo_t* cbo,const char* string)
{
	if(cbo->lbox != RT_NULL)
	{
		rtgui_listbox_item_t item;
		rtgui_rect_t rect;
		rt_uint32_t h,count;

		item.name = (char*)string;
		item.image = RT_NULL;
		rtgui_listbox_add_item(cbo->lbox, &item, 1);

		rtgui_widget_get_rect(cbo->lbox, &rect);
		count = rtgui_listbox_get_count(cbo->lbox);

		h = RTGUI_WIDGET_BORDER_SIZE(cbo->lbox)*2+(RTGUI_SEL_H + 2)*count;

		if(h>RC_H(rect))
		{
			if(cbo->lbox->item_per_page<5)/* change when less than five */
			{
				rect.y2 = rect.y1+h;
				rtgui_widget_rect_to_device(cbo->lbox,&rect);
				rtgui_widget_set_rect(cbo->lbox,&rect);/* update listbox extent */
				cbo->lbox->item_per_page = RC_H(rect) / (RTGUI_SEL_H+2);

				if(cbo->lbox->scroll != RT_NULL)/* update scrollbar extent */
				{
					rtgui_widget_get_rect(cbo->lbox->scroll, &rect);
					rect.y2 = rect.y1+h-RTGUI_WIDGET_BORDER_SIZE(cbo->lbox)*2;
					rtgui_widget_rect_to_device(cbo->lbox->scroll,&rect);
					rtgui_widget_set_rect(cbo->lbox->scroll,&rect);

					if(cbo->lbox->item_count > cbo->lbox->item_per_page)
					{
						RTGUI_WIDGET_SHOW(cbo->lbox->scroll);
						rtgui_scrollbar_set_line_step(cbo->lbox->scroll, 1);
						rtgui_scrollbar_set_page_step(cbo->lbox->scroll, cbo->lbox->item_per_page);
						rtgui_scrollbar_set_range(cbo->lbox->scroll, cbo->lbox->item_count);
					}
					else
					{
						RTGUI_WIDGET_HIDE(cbo->lbox->scroll);
					}
					rtgui_widget_update_clip(cbo->lbox);
				}
			}
		}
	}
}

void rtgui_combo_set_items(rtgui_combo_t* cbo, rtgui_listbox_item_t* items, rt_uint32_t count)
{
	if(cbo != RT_NULL && cbo->lbox != RT_NULL)
	{
		rtgui_listbox_set_items(cbo->lbox,items,count);
	}
}

rt_bool_t rtgui_combo_event_handler(pvoid wdt, rtgui_event_t* event)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	rtgui_combo_t* cbo = RTGUI_COMBO(wdt);


	RT_ASSERT(widget != RT_NULL);

	switch(event->type)
	{
	case RTGUI_EVENT_PAINT:
		if(widget->on_draw != RT_NULL)
			widget->on_draw(widget, event);
		else
			rtgui_combo_ondraw(cbo);
		break;

	case RTGUI_EVENT_KBD:
		if(widget->on_key != RT_NULL)
			widget->on_key(widget, event);

		return RT_TRUE;

	case RTGUI_EVENT_MOUSE_BUTTON:
	{
		rtgui_rect_t rect;
		struct rtgui_event_mouse* emouse = (struct rtgui_event_mouse*)event;
		rt_bool_t inclip=RT_EOK;

		if(!RTGUI_WIDGET_IS_ENABLE(cbo)) break;

		if(cbo->tbox->isedit == RT_TRUE)
		{
			/* only detect textbox area */
			inclip = rtgui_region_contains_point(&RTGUI_WIDGET_CLIP(cbo),emouse->x,emouse->y,&rect);
		}
		else
		{
			/* detect all area */
			inclip = (rtgui_region_contains_point(&RTGUI_WIDGET_CLIP(cbo),emouse->x,emouse->y,&rect) &&
			          rtgui_region_contains_point(&RTGUI_WIDGET_CLIP(cbo->tbox),emouse->x,emouse->y,&rect));
		}

		if(inclip == RT_EOK)
		{
			rtgui_combo_get_downarrow_rect(cbo,&rect);
			if(emouse->button & RTGUI_MOUSE_BUTTON_DOWN)
			{
				if(rtgui_rect_contains_point(&rect,emouse->x,emouse->y) == RT_EOK)
				{
					/* on pull-down button */
					cbo->style = RTGUI_COMBO_STYLE_DOWNARROW_DOWN;
					rtgui_combo_draw_downarrow(cbo);
				}

				if(cbo->lbox != RT_NULL)
				{
					if(RTGUI_WIDGET_IS_HIDE(cbo->lbox))
					{
						/* display pupup listbox */
						RTGUI_WIDGET_SHOW(cbo->lbox);
						rtgui_widget_focus(cbo->lbox);
						rtgui_widget_update_clip_pirate(RTGUI_WIDGET_PARENT(cbo->lbox),cbo->lbox);
						/* set listbox location is 0 */
						cbo->lbox->first_item=0;
						cbo->lbox->now_item = 0;
						if(cbo->lbox->scroll != RT_NULL)
						{
							if(!RTGUI_WIDGET_IS_HIDE(cbo->lbox->scroll))
							{
								rtgui_scrollbar_set_value(cbo->lbox->scroll,cbo->lbox->first_item);
							}
						}
						rtgui_widget_update(RTGUI_WIDGET_PARENT(cbo->lbox));
					}
					else
					{
						/* hide it */
						rtgui_widget_hide(cbo->lbox);
					}
				}
			}
			else if(emouse->button & RTGUI_MOUSE_BUTTON_UP)
			{
				if(rtgui_region_contains_point(&RTGUI_WIDGET_CLIP(cbo),emouse->x,emouse->y,&rect) == RT_EOK)
				{
					/* on upriver button */
					cbo->style = RTGUI_COMBO_STYLE_DOWNARROW_UP;
					rtgui_combo_draw_downarrow(cbo);
				}
			}
		}
		else
			rtgui_view_event_handler(widget,event);

		return RT_TRUE;
	}

	default:
		return rtgui_view_event_handler(widget,event);
	}

	return RT_FALSE;
}

