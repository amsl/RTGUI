/*
 * File      : listview.c
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
#include <rtgui/widgets/listview.h>

static void rtgui_listview_calc(rtgui_listview_t* view);

static void _rtgui_listview_constructor(rtgui_listview_t *view)
{
	/* default rect */
	rtgui_rect_t rect = {0, 0, 200, 200};

	/* set default widget rect and set event handler */
	rtgui_widget_set_event_handler(view,rtgui_listview_event_handler);
	rtgui_widget_set_rect(view, &rect);

	RTGUI_WIDGET_FLAG(view) |= RTGUI_WIDGET_FLAG_FOCUSABLE;

	view->flag = RTGUI_LISTVIEW_LIST;
	view->item_current = 0;
	view->item_count = 0;
	view->page = 0;

	RTGUI_WIDGET_BC(view) = theme.blankspace;
	RTGUI_WIDGET_TEXTALIGN(view) = RTGUI_ALIGN_CENTER_VERTICAL;
}

static void _rtgui_listview_destructor(rtgui_listview_t *view)
{

}

DEFINE_CLASS_TYPE(listview, "listview",
                  RTGUI_VIEW_TYPE,
                  _rtgui_listview_constructor,
                  _rtgui_listview_destructor,
                  sizeof(struct rtgui_listview));

rtgui_listview_t* rtgui_listview_create(pvoid parent,const rtgui_listview_item_t* items, rt_uint16_t count, int left,int top,int w,int h, rt_uint16_t flag)
{
	rtgui_container_t *container;
	rtgui_listview_t* view = RT_NULL;

	RT_ASSERT(parent != RT_NULL);
	container = RTGUI_CONTAINER(parent);

	view = rtgui_widget_create(RTGUI_LISTVIEW_TYPE);
	if(view != RT_NULL)
	{
		rtgui_rect_t rect;
		view->items = items;
		view->item_count = count;

		view->flag = flag;
		rtgui_widget_get_rect(view, &rect);
		rtgui_widget_rect_to_device(container,&rect);
		rect.x1 += left;
		rect.y1 += top;
		rect.x2 = rect.x1+w;
		rect.y2 = rect.y1+h;
		rtgui_widget_set_rect(view, &rect);

		if(flag == RTGUI_LISTVIEW_LIST)
			view->page = RC_H(rect) / (2 + RTGUI_SEL_H);
		else if((flag == RTGUI_LISTVIEW_ICON) && (count > 0))
		{
			rtgui_listview_calc(view);
		}
		rtgui_container_add_child(container, view);
	}

	return view;
}

void rtgui_listview_destroy(rtgui_listview_t* view)
{
	/* destroy view */
	rtgui_widget_destroy(view);
}

static void rtgui_listview_onicondraw(rtgui_listview_t* view)
{
	rtgui_rect_t rect, item_rect, drawing_rect;
	rt_uint32_t c, r, i; /* col and row index */
	rt_uint32_t item_width, item_height;
	rtgui_image_t* image;
	rtgui_dc_t* dc;

	RT_ASSERT(view != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(view);
	if(dc == RT_NULL)return;

	if(view->item_count == 0) return;

	rtgui_widget_get_rect(view, &rect);
	i = (view->item_current / view->page) * view->page;

	item_width = (RC_W(rect) - 2 * RTGUI_MARGIN)/view->col_items;
	item_height = (RC_H(rect) - 4)/view->row_items;
	image = view->items[0].image;

	for(r = 0; r < view->row_items; r ++)
	{
		for(c = 0; c < view->col_items; c ++)
		{
			if(i < view->item_count)
			{
				item_rect.y1 = rect.y1 + RTGUI_MARGIN + r * item_height;
				item_rect.x1 = rect.x1 + RTGUI_MARGIN + c * item_width;
				item_rect.x2 = item_rect.x1 + item_width;
				item_rect.y2 = item_rect.y1 + item_height;

				if(i == view->item_current)
				{
					RTGUI_DC_BC(dc) = DarkBlue;
					rtgui_dc_fill_rect(dc, &item_rect);
				}

				drawing_rect.x1 = drawing_rect.y1 = 0;
				drawing_rect.x2 = image->w;
				drawing_rect.y2 = image->h;
				rtgui_rect_moveto_align(&item_rect, &drawing_rect, RTGUI_ALIGN_CENTER_HORIZONTAL);
				drawing_rect.y1 += 3;
				drawing_rect.y2 += 3;
				rtgui_image_blit(view->items[i].image, dc, &drawing_rect);

				item_rect.y1 = drawing_rect.y2 + RTGUI_MARGIN;
				item_rect.x1 += 3;
				item_rect.x2 -=3;
				rtgui_font_get_string_rect(RTGUI_WIDGET_FONT(view), view->items[i].name,
				                           &drawing_rect);
				rtgui_rect_moveto_align(&item_rect, &drawing_rect, RTGUI_ALIGN_CENTER_HORIZONTAL);
				rtgui_dc_draw_text(dc, view->items[i].name, &drawing_rect);

				i ++;
			}
			else break;
		}
	}
	rtgui_dc_end_drawing(dc);
}

static void rtgui_listview_update_icon(rtgui_listview_t* view, rt_uint16_t old_item)
{
	rtgui_rect_t rect, item_rect, drawing_rect;
	rt_uint32_t c, r; /* col and row index */
	rt_uint32_t item_width, item_height;
	rtgui_image_t* image;
	rtgui_dc_t* dc;

	RT_ASSERT(view != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(view);
	if(dc == RT_NULL)return;

	if((view->item_count == 0) ||
	        (old_item == view->item_current))
		return;

	if(old_item/view->page != view->item_current/view->page)
	{
		/* it's not a same page, update all */
		rtgui_widget_update(view);
		return;
	}

	rtgui_widget_get_rect(view, &rect);

	item_width = (RC_W(rect) - 2 * RTGUI_MARGIN)/view->col_items;
	item_height = (RC_H(rect) - 4)/view->row_items;
	image = view->items[0].image;

	/* update old item */
	r = (old_item % view->page)/ view->col_items;
	c = (old_item % view->page)% view->col_items;
	item_rect.y1 = rect.y1 + RTGUI_MARGIN + r * item_height;
	item_rect.x1 = rect.x1 + RTGUI_MARGIN + c * item_width;
	item_rect.x2 = item_rect.x1 + item_width;
	item_rect.y2 = item_rect.y1 + item_height;
	rtgui_dc_fill_rect(dc,&item_rect);

	/* draw image */
	drawing_rect.x1 = drawing_rect.y1 = 0;
	drawing_rect.x2 = image->w;
	drawing_rect.y2 = image->h;
	rtgui_rect_moveto_align(&item_rect, &drawing_rect, RTGUI_ALIGN_CENTER_HORIZONTAL);
	drawing_rect.y1 += 3;
	drawing_rect.y2 += 3;
	rtgui_image_blit(view->items[old_item].image, dc,&drawing_rect);

	/* draw text */
	item_rect.y1 = drawing_rect.y2 + RTGUI_MARGIN;
	item_rect.x1 += 3;
	item_rect.x2 -=3;
	rtgui_font_get_string_rect(RTGUI_WIDGET_FONT(view), view->items[old_item].name,
	                           &drawing_rect);
	rtgui_rect_moveto_align(&item_rect, &drawing_rect, RTGUI_ALIGN_CENTER_HORIZONTAL);
	rtgui_dc_draw_text(dc, view->items[old_item].name, &drawing_rect);

	/* update new item as selected */
	r = (view->item_current % view->page) / view->col_items;
	c = (view->item_current % view->page) % view->col_items;
	item_rect.y1 = rect.y1 + RTGUI_MARGIN + r * item_height;
	item_rect.x1 = rect.x1 + RTGUI_MARGIN + c * item_width;
	item_rect.x2 = item_rect.x1 + item_width;
	item_rect.y2 = item_rect.y1 + item_height;
	RTGUI_DC_BC(dc) = DarkBlue;
	rtgui_dc_fill_rect(dc, &item_rect);

	/* draw image */
	drawing_rect.x1 = 0;
	drawing_rect.y1 = 3;
	drawing_rect.x2 = image->w;
	drawing_rect.y2 = 3 + image->h;

	rtgui_rect_moveto_align(&item_rect, &drawing_rect, RTGUI_ALIGN_CENTER_HORIZONTAL);
	rtgui_image_blit(view->items[view->item_current].image, dc, &drawing_rect);

	/* draw text */
	item_rect.y1 = drawing_rect.y2 + RTGUI_MARGIN;
	item_rect.x1 += 3;
	item_rect.x2 -=3;
	rtgui_font_get_string_rect(RTGUI_WIDGET_FONT(view),
	                           view->items[view->item_current].name,
	                           &drawing_rect);
	rtgui_rect_moveto_align(&item_rect, &drawing_rect, RTGUI_ALIGN_CENTER_HORIZONTAL);
	rtgui_dc_draw_text(dc, view->items[view->item_current].name, &drawing_rect);

	rtgui_dc_end_drawing(dc);
}

static void rtgui_listview_onlistdraw(rtgui_listview_t* view)
{
	rt_uint32_t i, page;
	rtgui_rect_t rect, item_rect, image_rect;
	const rtgui_listview_item_t* item;
	rtgui_dc_t* dc;

	RT_ASSERT(view != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(view);
	if(dc == RT_NULL)return;

	rtgui_widget_get_rect(view, &rect);

	/* get item base rect */
	item_rect = rect;
	item_rect.x1 += 2;
	item_rect.x2 -= 2;
	item_rect.y1 += 2;
	item_rect.y2 = item_rect.y1 + (2 + RTGUI_SEL_H);

	/* get current page */
	page = (view->item_current / view->page) * view->page;
	for(i = 0; i < view->page; i ++)
	{
		if(page + i >= view->item_count) break;

		item = &(view->items[page + i]);

		if(page + i == view->item_current)
		{
			RTGUI_DC_BC(dc) = DarkBlue;
			rtgui_dc_fill_rect(dc, &item_rect);
		}
		item_rect.x1 += RTGUI_MARGIN;

		if(item->image != RT_NULL)
		{
			/* blit on center */
			image_rect.x1 = 0;
			image_rect.y1 = 0;
			image_rect.x2 = item->image->w;
			image_rect.y2 = item->image->h;

			rtgui_rect_moveto_align(&item_rect, &image_rect, RTGUI_ALIGN_CENTER_VERTICAL);
			rtgui_image_blit(item->image, dc, &image_rect);
			item_rect.x1 += item->image->w + 2;
		}
		/* draw text */
		rtgui_dc_draw_text(dc, item->name, &item_rect);

		if(item->image != RT_NULL)
			item_rect.x1 -= (item->image->w + 2);
		item_rect.x1 -= RTGUI_MARGIN;

		/* move to next item position */
		item_rect.y1 += (RTGUI_SEL_H + 2);
		item_rect.y2 += (RTGUI_SEL_H + 2);
	}

	rtgui_dc_end_drawing(dc);
}

void rtgui_listview_update_list(rtgui_listview_t* view, rt_uint16_t old_item)
{
	const rtgui_listview_item_t* item;
	rtgui_rect_t rect, item_rect;
	rtgui_dc_t* dc;

	RT_ASSERT(view != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(view);
	if(dc == RT_NULL)return;

	if(old_item/view->page != view->item_current/view->page)
	{
		/* it's not a same page, update all */
		rtgui_widget_update(view);
		return;
	}

	/* draw old item */
	rtgui_widget_get_rect(view, &rect);
	item_rect = rect;
	item_rect.x1 += 2;
	item_rect.x2 -= 2;
	item_rect.y1 += 2;
	item_rect.y1 += (old_item % view->page) * (2 + RTGUI_SEL_H);
	item_rect.y2 = item_rect.y1 + (2 + RTGUI_SEL_H);

	rtgui_dc_fill_rect(dc,&item_rect);

	item_rect.x1 += RTGUI_MARGIN;

	item = &(view->items[old_item]);
	if(item->image != RT_NULL)
	{
		rtgui_rect_t image_rect;

		/* blit on center */
		image_rect.x1 = 0;
		image_rect.y1 = 0;
		image_rect.x2 = item->image->w;
		image_rect.y2 = item->image->h;

		rtgui_rect_moveto_align(&item_rect, &image_rect, RTGUI_ALIGN_CENTER_VERTICAL);
		rtgui_image_blit(item->image, dc, &image_rect);
		item_rect.x1 += item->image->w + 2;
	}
	rtgui_dc_draw_text(dc, item->name, &item_rect);

	/* draw current item */
	item_rect = rect;
	/* get current item's rect */
	item_rect.x1 += 2;
	item_rect.x2 -= 2;
	item_rect.y1 += 2;
	item_rect.y1 += (view->item_current % view->page) * (2 + RTGUI_SEL_H);
	item_rect.y2 = item_rect.y1 + (2 + RTGUI_SEL_H);

	/* draw current item */
	RTGUI_DC_BC(dc) = DarkBlue;
	rtgui_dc_fill_rect(dc, &item_rect);

	item_rect.x1 += RTGUI_MARGIN;

	item = &(view->items[view->item_current]);
	if(item->image != RT_NULL)
	{
		rtgui_rect_t image_rect;

		/* blit on center */
		image_rect.x1 = 0;
		image_rect.y1 = 0;
		image_rect.x2 = item->image->w;
		image_rect.y2 = item->image->h;

		rtgui_rect_moveto_align(&item_rect, &image_rect, RTGUI_ALIGN_CENTER_VERTICAL);
		rtgui_image_blit(item->image, dc, &image_rect);
		item_rect.x1 += (item->image->w + 2);
	}

	RTGUI_DC_FC(dc) = theme.blankspace;
	rtgui_dc_draw_text(dc, item->name, &item_rect);

	rtgui_dc_end_drawing(dc);
}

void rtgui_listview_ondraw(rtgui_listview_t* view)
{
	rtgui_rect_t rect;
	rtgui_dc_t* dc;

	RT_ASSERT(view != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(view);
	if(dc == RT_NULL)return;

	rtgui_widget_get_rect(view, &rect);
	rtgui_dc_fill_rect(dc,&rect);

	switch(view->flag)
	{
	case RTGUI_LISTVIEW_LIST:
		rtgui_listview_onlistdraw(view);
		break;

	case RTGUI_LISTVIEW_ICON:
		rtgui_listview_onicondraw(view);
		break;
	}

	rtgui_dc_end_drawing(dc);
}

static rt_bool_t rtgui_listview_onmouse(rtgui_listview_t* view, struct rtgui_event_mouse* emouse)
{
	rtgui_rect_t rect;

	/* calculate selected item */

	/* get physical extent information */
	rtgui_widget_get_rect(view, &rect);

	if(rtgui_rect_contains_point(&rect, emouse->x, emouse->y) == RT_EOK)
	{
		rt_uint16_t index;
		rt_uint16_t old_item;

		/* get old item */
		old_item = view->item_current;

		switch(view->flag)
		{
		case RTGUI_LISTVIEW_LIST:
			index = (emouse->y - rect.y1) / (2 + RTGUI_SEL_H);

			if((index < view->item_count) && (index < view->page))
			{
				/* set selected item */
				view->item_current = (view->item_current/view->page) * view->page + index;
				if(emouse->button & RTGUI_MOUSE_BUTTON_DOWN)
				{
					/* down event */
					rtgui_listview_update_list(view, old_item);
				}
				else
				{
					/* up event */
					if(view->items[view->item_current].action != RT_NULL)
					{
						view->items[view->item_current].action(view->items[view->item_current].parameter);
					}
				}
			}
			break;

		case RTGUI_LISTVIEW_ICON:
		{
			rt_uint16_t x, y;
			rt_uint16_t item_height, item_width;
			rt_uint32_t current_page;

			item_width = (RC_W(rect) - 2 * RTGUI_MARGIN)/view->col_items;
			item_height = (RC_H(rect) - 4)/view->row_items;
			x = emouse->x - rect.x1;
			y = emouse->y - rect.y1;

			index = (y / item_height * view->col_items) + x / item_width;
			current_page = view->item_current / view->page;

			if((index + (current_page * view->page) < view->item_count))
			{
				if(emouse->button & RTGUI_MOUSE_BUTTON_DOWN)
				{
					view->item_current = index + (current_page * view->page);

					/* down event */
					rtgui_listview_update_icon(view, old_item);
				}
				else
				{
					/* up event */
					if(view->items[view->item_current].action != RT_NULL)
					{
						view->items[view->item_current].action(view->items[view->item_current].parameter);
					}
				}
			}
		}
		break;

		case RTGUI_LISTVIEW_REPORT:
			break;
		}

		return RT_TRUE;
	}

	return RT_FALSE;
}

rt_bool_t rtgui_listview_event_handler(pvoid wdt, rtgui_event_t* event)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	rtgui_listview_t* view = RTGUI_LISTVIEW(widget);

	switch(event->type)
	{
	case RTGUI_EVENT_PAINT:
		rtgui_listview_ondraw(view);
		return RT_FALSE;

	case RTGUI_EVENT_MOUSE_BUTTON:
	{
		struct rtgui_event_mouse* emouse;

		emouse = (struct rtgui_event_mouse*)event;
		return rtgui_listview_onmouse(view, emouse);
	}

	case RTGUI_EVENT_KBD:
	{
		struct rtgui_event_kbd *ekbd = (struct rtgui_event_kbd*)event;
		if(RTGUI_KBD_IS_DOWN(ekbd))
		{
			rt_uint16_t old_item;

			old_item = view->item_current;
			switch(ekbd->key)
			{
			case RTGUIK_LEFT:
				if(view->flag == RTGUI_LISTVIEW_LIST)
				{
					if(view->item_current - view->page >= 0)
						view->item_current -= view->page;

					rtgui_listview_update_list(view, old_item);
				}
				else if(view->flag == RTGUI_LISTVIEW_ICON)
				{
					if(view->item_current > 0)
						view->item_current --;
					rtgui_listview_update_icon(view, old_item);
				}
				return RT_FALSE;

			case RTGUIK_UP:
				if(view->flag == RTGUI_LISTVIEW_LIST)
				{
					if(view->item_current > 0)
						view->item_current --;
					rtgui_listview_update_list(view, old_item);
				}
				else if(view->flag == RTGUI_LISTVIEW_ICON)
				{
					if(view->item_current >= view->col_items)
						view->item_current -= view->col_items;
					else
						view->item_current = 0;

					rtgui_listview_update_icon(view, old_item);
				}
				return RT_FALSE;

			case RTGUIK_RIGHT:
				if(view->flag == RTGUI_LISTVIEW_LIST)
				{
					if(view->item_current + view->page < view->item_count - 1)
						view->item_current += view->page;

					rtgui_listview_update_list(view, old_item);
				}
				else if(view->flag == RTGUI_LISTVIEW_ICON)
				{
					if(view->item_current < view->item_count - 1)
						view->item_current ++;

					rtgui_listview_update_icon(view, old_item);
				}
				return RT_FALSE;

			case RTGUIK_DOWN:
				if(view->flag == RTGUI_LISTVIEW_LIST)
				{
					if(view->item_current < view->item_count - 1)
						view->item_current ++;

					rtgui_listview_update_list(view, old_item);
				}
				else if(view->flag == RTGUI_LISTVIEW_ICON)
				{
					if(view->item_current + view->col_items <= (view->item_count - 1))
						view->item_current += view->col_items;
					else
						view->item_current = view->item_count - 1;

					rtgui_listview_update_icon(view, old_item);
				}
				return RT_FALSE;

			case RTGUIK_RETURN:
				if(view->items[view->item_current].action != RT_NULL)
				{
					view->items[view->item_current].action(view->items[view->item_current].parameter);
				}
				return RT_FALSE;

			default:
				break;
			}
		}
		return RT_TRUE;
	}

	default:
		return rtgui_view_event_handler(widget, event);
	}
}

static void rtgui_listview_calc(rtgui_listview_t* view)
{
	/* get image of first item*/
	rtgui_image_t *image;
	rtgui_rect_t rect;
	rt_uint32_t text_width, text_height;
	rt_uint32_t item_width, item_height;

	if(view->item_count == 0) return;

	image = view->items[0].image;
	rtgui_font_get_string_rect(RTGUI_WIDGET_FONT(view), "HHHHHH", &rect);

	text_height = RC_H(rect);
	text_width = RC_W(rect);

	rtgui_widget_get_rect(view, &rect);

	item_width = image->w + RTGUI_MARGIN;
	if(item_width < (text_width + RTGUI_MARGIN)) item_width = text_width + RTGUI_MARGIN;
	item_height = image->h + 3 + text_height + RTGUI_MARGIN;

	view->row_items = (RC_H(rect) - 2 * RTGUI_MARGIN) / item_height;
	view->col_items = (RC_W(rect) - 2 * RTGUI_MARGIN) / item_width;
	view->page = view->row_items * view->col_items;
}

