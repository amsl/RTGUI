/*
 * File      : rttab.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * RTTAB = RT-thread TAB widget
 * Change Logs:
 * Date           Author       Notes
 *
 */
#include <rtgui/rtgui.h>
#include <rtgui/dc.h>
#include <rtgui/widgets/rttab.h>

static rt_bool_t rtgui_rttab_spin_onclick(rtgui_object_t *object, rtgui_event_t* event);

static void _rtgui_rttab_constructor(rtgui_rttab_t *tab)
{
	/* init widget and set event handler */
	RTGUI_WIDGET_FLAG(tab) |= RTGUI_WIDGET_FLAG_FOCUSABLE;
	RTGUI_WIDGET_TEXTALIGN(tab) |= RTGUI_ALIGN_CENTER;
	rtgui_object_set_event_handler(RTGUI_OBJECT(tab), rtgui_rttab_event_handler);
	rtgui_widget_set_border(RTGUI_WIDGET(tab),RTGUI_BORDER_UP);
	tab->caption = RT_NULL;
	tab->orient = RTGUI_RTTAB_ORIENT_TOP;
	tab->tag_size = RTGUI_RTTAB_DEFAULT_TITLE_SIZE;
	tab->focus_color = blue; /* please reset it */
	tab->tag_count = 0;
	tab->first_tag = 0;
	tab->now_tag   = 0;
	tab->tags      = RT_NULL;
	tab->spin      = RT_NULL; /* spin widget */
}

static void _rtgui_rttab_destructor(rtgui_rttab_t *tab)
{
	if(tab->caption != RT_NULL)
	{
		rt_free(tab->caption);
		tab->caption = RT_NULL;
	}
}

DEFINE_CLASS_TYPE(rttab, "rttab",
                  RTGUI_CONTAINER_TYPE,
                  _rtgui_rttab_constructor,
                  _rtgui_rttab_destructor,
                  sizeof(struct rtgui_rttab));

rtgui_rttab_t* rtgui_rttab_create(rtgui_container_t *container, char* caption,int left, int top, int w, int h)
{
	rtgui_rttab_t* tab;

	RT_ASSERT(container != RT_NULL);

	tab = (rtgui_rttab_t *) rtgui_widget_create(RTGUI_RTTAB_TYPE);
	if(tab != RT_NULL)
	{
		rtgui_rect_t rect;
		int height = 0;
		/* set default rect */
		rtgui_widget_get_rect(RTGUI_WIDGET(container), &rect);
		rtgui_widget_rect_to_device(RTGUI_WIDGET(container),&rect);
		rect.x1 += left;
		rect.y1 += top;
		rect.x2 = rect.x1+w;
		rect.y2 = rect.y1+h;
		rtgui_widget_set_rect(RTGUI_WIDGET(tab), &rect);
		rtgui_container_add_child(container, RTGUI_WIDGET(tab));

		height = FONT_H(RTGUI_WIDGET_FONT(tab));
		tab->tag_size = height + RTGUI_WIDGET_DEFAULT_MARGIN*2;
		tab->caption = rt_strdup(caption);
	}

	return tab;
}

void rtgui_rttab_destroy(rtgui_rttab_t* tab)
{
	rtgui_widget_destroy(RTGUI_WIDGET(tab));
}

/* draw left/top/right */
void rtgui_rttab_draw_title_rect(struct rtgui_dc* dc, rtgui_rect_t* rect,
                                 rtgui_color_t c1, rtgui_color_t c2, rt_uint32_t flag)
{
	RT_ASSERT(dc != RT_NULL);

	RTGUI_DC_FC(dc) = c1;
	rtgui_dc_draw_vline(dc, rect->x1, rect->y1, rect->y2);
	if(flag == RTGUI_RTTAB_ORIENT_TOP)
	{
		rtgui_dc_draw_hline(dc, rect->x1 + 1, rect->x2, rect->y1);
	}

	RTGUI_DC_FC(dc) = c2;

	rtgui_dc_draw_vline(dc, rect->x2 - 1, rect->y1, rect->y2);
	if(flag == RTGUI_RTTAB_ORIENT_BOTTOM)
	{
		rtgui_dc_draw_hline(dc, rect->x1, rect->x2, rect->y2 - 1);
	}
}

void rtgui_rttab_ondraw(rtgui_rttab_t* tab)
{
	int i, _left=0, _right=0;
	rtgui_rect_t rect, tmp_rect;
	rtgui_rttab_item_t* item;
	struct rtgui_dc* dc;

	RT_ASSERT(tab != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(RTGUI_WIDGET(tab));
	if(dc == RT_NULL)return;

	rtgui_widget_get_rect(RTGUI_WIDGET(tab), &rect);

	/* draw rttab widget border */
	tmp_rect = rect;
	if(tab->orient==RTGUI_RTTAB_ORIENT_TOP)
	{
		tmp_rect.y2 = tmp_rect.y1 + RTGUI_WIDGET_BORDER(tab) + tab->tag_size;
		RTGUI_DC_FC(dc) = RTGUI_WIDGET_BACKGROUND(tab);
		/* draw tagbar border */
		rtgui_dc_draw_line(dc,tmp_rect.x1, tmp_rect.y1, tmp_rect.x1, tmp_rect.y2);
		rtgui_dc_draw_line(dc,tmp_rect.x2-1, tmp_rect.y1, tmp_rect.x2-1, tmp_rect.y2);
		rtgui_dc_draw_line(dc,tmp_rect.x1, tmp_rect.y1, tmp_rect.x2, tmp_rect.y1);

		tmp_rect.y1 = tmp_rect.y2-1;
		tmp_rect.y2 = rect.y2;
		/* fill client background */
		rtgui_rect_inflate(&tmp_rect,-RTGUI_WIDGET_BORDER(tab));
		rtgui_dc_fill_rect(dc, &tmp_rect);
		rtgui_rect_inflate(&tmp_rect,RTGUI_WIDGET_BORDER(tab));
		/* draw client border */
		RTGUI_DC_FC(dc) = white;
		rtgui_dc_draw_line(dc,tmp_rect.x1, tmp_rect.y1, tmp_rect.x1, tmp_rect.y2-1);
		RTGUI_DC_FC(dc) = dark_grey;
		rtgui_dc_draw_line(dc,tmp_rect.x2-1, tmp_rect.y1, tmp_rect.x2-1, tmp_rect.y2-1);
		rtgui_dc_draw_line(dc,tmp_rect.x1, tmp_rect.y2-1, tmp_rect.x2, tmp_rect.y2-1);
	}
	else if(tab->orient==RTGUI_RTTAB_ORIENT_BOTTOM)
	{
		tmp_rect.y2 = tmp_rect.y1 + (RC_H(rect)-tab->tag_size);
		/* fill client background */
		rtgui_rect_inflate(&tmp_rect,-RTGUI_WIDGET_BORDER(tab));
		rtgui_dc_fill_rect(dc, &tmp_rect);
		rtgui_rect_inflate(&tmp_rect,RTGUI_WIDGET_BORDER(tab));
		/* draw client border */
		RTGUI_DC_FC(dc) = white;
		rtgui_dc_draw_line(dc,tmp_rect.x1, tmp_rect.y1, tmp_rect.x1, tmp_rect.y2-1);
		rtgui_dc_draw_line(dc,tmp_rect.x1, tmp_rect.y1, tmp_rect.x2, tmp_rect.y1);
		RTGUI_DC_FC(dc) = dark_grey;
		rtgui_dc_draw_line(dc,tmp_rect.x2-1, tmp_rect.y1, tmp_rect.x2-1, tmp_rect.y2);

		tmp_rect.y1 = tmp_rect.y2-1;
		tmp_rect.y2 = rect.y2;
		/* draw tagbar border */
		RTGUI_DC_FC(dc) = RTGUI_WIDGET_BACKGROUND(tab);
		rtgui_dc_draw_line(dc,tmp_rect.x1, tmp_rect.y1, tmp_rect.x1, tmp_rect.y2-1);
		rtgui_dc_draw_line(dc,tmp_rect.x2-1, tmp_rect.y1+1, tmp_rect.x2-1, tmp_rect.y2-1);
		rtgui_dc_draw_line(dc,tmp_rect.x1, tmp_rect.y2-1, tmp_rect.x2, tmp_rect.y2-1);
	}

	tmp_rect = rect;

	if(tab->orient==RTGUI_RTTAB_ORIENT_TOP)
	{
		for(i=tab->first_tag; i<tab->tag_count; i++)
		{
			item = &(tab->tags[i]);
			tmp_rect.x2 = tmp_rect.x1 + item->tag_width;
			tmp_rect.y2 = tmp_rect.y1 + RTGUI_WIDGET_BORDER(tab) + tab->tag_size;
			/* fill tagbar background, use same color as client area. */
			RTGUI_DC_BC(dc) = RTGUI_WIDGET_BACKGROUND(item->tag);
			rtgui_dc_fill_rect(dc, &tmp_rect);
			/* draw tagbutton border */
			rtgui_rttab_draw_title_rect(dc, &tmp_rect, white, dark_grey, RTGUI_RTTAB_ORIENT_TOP);
			rtgui_rect_inflate(&tmp_rect, -1);
			rtgui_rttab_draw_title_rect(dc, &tmp_rect, default_background, light_grey, RTGUI_RTTAB_ORIENT_TOP);
			rtgui_rect_inflate(&tmp_rect, 1);

			if(i == tab->now_tag)
			{
				_left = tmp_rect.x1;
				_right = tmp_rect.x2;
				RTGUI_DC_FC(dc) = tab->focus_color;
			}
			else
			{
				RTGUI_DC_FC(dc) = black;
			}

			if(item->image != RT_NULL)
			{
				rtgui_rect_t image_rect = {0};
				image_rect.x2 = image_rect.x1+item->image->w;
				image_rect.y2 = image_rect.y1+item->image->h;
				rtgui_rect_moveto_align(&tmp_rect, &image_rect, RTGUI_ALIGN_CENTER_VERTICAL);
				rtgui_image_blit(item->image, dc, &image_rect);
				tmp_rect.x1 += item->image->w;
			}
			rtgui_dc_draw_text(dc, item->tag->title, &tmp_rect);
			tmp_rect.x1 = tmp_rect.x2;
		}
		tmp_rect.x2 = rect.x2;
		if(RTGUI_WIDGET(tab)->parent != RT_NULL)
			RTGUI_DC_BC(dc) = RTGUI_WIDGET_BACKGROUND(RTGUI_WIDGET(tab)->parent);
		else
			RTGUI_DC_BC(dc) = default_background;
		rtgui_dc_fill_rect(dc, &tmp_rect);

		RTGUI_DC_FC(dc) = white;
		rtgui_dc_draw_line(dc,rect.x1, tmp_rect.y2-1, _left, tmp_rect.y2-1);
		rtgui_dc_draw_line(dc,_right, tmp_rect.y2-1, rect.x2, tmp_rect.y2-1);
	}
	else if(tab->orient == RTGUI_RTTAB_ORIENT_BOTTOM)
	{
		int h;
		h = RC_H(rect)-RTGUI_WIDGET_BORDER(tab)*2 - tab->tag_size;
		tmp_rect.y1 += RTGUI_WIDGET_BORDER(tab) + h;
		tmp_rect.y2 = tmp_rect.y1 + tab->tag_size;

		for(i=tab->first_tag; i<tab->tag_count; i++)
		{
			item = &(tab->tags[i]);
			tmp_rect.x2 = tmp_rect.x1+item->tag_width;
			tmp_rect.y2 = tmp_rect.y1+tab->tag_size;
			RTGUI_DC_BC(dc) = RTGUI_WIDGET_BACKGROUND(item->tag);
			rtgui_dc_fill_rect(dc, &tmp_rect);
			/* draw tagbutton border */
			rtgui_rttab_draw_title_rect(dc, &tmp_rect, white, dark_grey, RTGUI_RTTAB_ORIENT_BOTTOM);
			rtgui_rect_inflate(&tmp_rect, -1);
			rtgui_rttab_draw_title_rect(dc, &tmp_rect, default_background, light_grey, RTGUI_RTTAB_ORIENT_BOTTOM);
			rtgui_rect_inflate(&tmp_rect, 1);

			if(i == tab->now_tag)
			{
				_left = tmp_rect.x1;
				_right = tmp_rect.x2;
				RTGUI_DC_FC(dc) = tab->focus_color;
			}
			else
			{
				RTGUI_DC_FC(dc) = black;
			}

			if(item->image != RT_NULL)
			{
				rtgui_rect_t image_rect = {0};
				image_rect.x2 = image_rect.x1+item->image->w;
				image_rect.y2 = image_rect.y1+item->image->h;
				rtgui_rect_moveto_align(&tmp_rect, &image_rect, RTGUI_ALIGN_CENTER_VERTICAL);
				rtgui_image_blit(item->image, dc, &image_rect);
				tmp_rect.x1 += item->image->w;
			}
			rtgui_dc_draw_text(dc, item->tag->title, &tmp_rect);
			tmp_rect.x1 = tmp_rect.x2;
		}
		tmp_rect.x2 = rect.x2;
		if(RTGUI_WIDGET(tab)->parent != RT_NULL)
			RTGUI_DC_BC(dc) = RTGUI_WIDGET_BACKGROUND(RTGUI_WIDGET(tab)->parent);
		else
			RTGUI_DC_BC(dc) = default_background;
		rtgui_dc_fill_rect(dc, &tmp_rect);

		RTGUI_DC_FC(dc) = dark_grey;
		rtgui_dc_draw_line(dc,rect.x1, tmp_rect.y1, _left, tmp_rect.y1);
		rtgui_dc_draw_line(dc,_right, tmp_rect.y1, rect.x2, tmp_rect.y1);
	}

	rtgui_dc_end_drawing(dc);
}

/* add a new page tag */
rtgui_container_t* rtgui_rttab_add_tag(rtgui_rttab_t* tab, const char* name, rtgui_image_t* image)
{
	rtgui_rect_t rect;
	rtgui_rttab_item_t item= {0}, *_tags;
	int w, h, _top;

	RT_ASSERT(tab != RT_NULL);

	rtgui_widget_get_rect(RTGUI_WIDGET(tab), &rect);

	w = RC_W(rect)-RTGUI_WIDGET_BORDER(tab)*2;
	h = RC_H(rect)-RTGUI_WIDGET_BORDER(tab)*2 - tab->tag_size;

	if(tab->orient==RTGUI_RTTAB_ORIENT_TOP)
		_top = RTGUI_WIDGET_BORDER(tab) + tab->tag_size;
	else
		_top = RTGUI_WIDGET_BORDER(tab);

	item.tag = rtgui_view_create(RTGUI_CONTAINER(tab), name, RTGUI_WIDGET_BORDER(tab), _top, w, h);
	if(item.tag == RT_NULL) return RT_NULL;

	RTGUI_WIDGET_HIDE(item.tag);

	item.image = image;
	item.tag_width = rtgui_font_get_string_width(RTGUI_WIDGET_FONT(tab), name);
	item.tag_width += RTGUI_WIDGET_DEFAULT_MARGIN*2;
	if(item.image != RT_NULL) 
		item.tag_width += item.image->w+RTGUI_WIDGET_DEFAULT_MARGIN;

	if(tab->tag_count == 0)
		_tags = rt_malloc(sizeof(rtgui_rttab_item_t));
	else
		_tags = rt_realloc(tab->tags,sizeof(rtgui_rttab_item_t)*(tab->tag_count+1));

	_tags[tab->tag_count].tag = item.tag;
	_tags[tab->tag_count].image = item.image;
	_tags[tab->tag_count].tag_width = item.tag_width;
	tab->tag_count++;
	tab->tags = _tags;

	RTGUI_WIDGET_UNHIDE(_tags[tab->now_tag].tag);

	{	/* Calculation hidden tag number */
		int i, uw=0, hide_num=0;
		rtgui_rttab_item_t* tag;

		for(i=0; i<tab->tag_count; i++)
		{
			tag = &(tab->tags[i]);
			uw += tag->tag_width;
			if(uw >= w)
			{
				hide_num = tab->tag_count-i;

				if(tab->spin != RT_NULL)
				{
					rtgui_widget_get_rect(RTGUI_WIDGET(tab), &rect);
					if((uw-tag->tag_width) >= (w-RC_W(rect)))
						hide_num += 1;
				}
				break;
			}
		}
		if(hide_num>0)
		{
			if(tab->spin == RT_NULL)
			{
				int ppl_top=0;
				int ppl_h = tab->tag_size-2;
				if(tab->orient==RTGUI_RTTAB_ORIENT_TOP)
					ppl_top = tab->tag_size-ppl_h;
				else if(tab->orient==RTGUI_RTTAB_ORIENT_BOTTOM)
					ppl_top = h+RTGUI_WIDGET_BORDER(tab)*2;
				tab->spin = rtgui_spin_create(RTGUI_CONTAINER(tab), RC_W(rect)-ppl_h*2,
				                               ppl_top, ppl_h*2, ppl_h, RTGUI_HORIZONTAL);
				rtgui_spin_bind(tab->spin, (rt_uint32_t*)&(tab->first_tag));
				tab->spin->widget_link = RTGUI_WIDGET(tab);
				tab->spin->on_click = rtgui_rttab_spin_onclick;
				tab->spin->range_max = hide_num;
			}
			else
			{
				tab->spin->range_max = hide_num;
			}
		}
	}
	return RTGUI_CONTAINER(item.tag);
}

rtgui_container_t* rtgui_rttab_get_container_by_index(rtgui_rttab_t* tab, rt_int16_t index)
{
	rtgui_rttab_item_t* item;

	if(tab == RT_NULL) return RT_NULL;
	if(tab->tag_count <= 0) return RT_NULL;
	if(index >= tab->tag_count) return RT_NULL;

	item = &(tab->tags[index]);
	return RTGUI_CONTAINER(item->tag);
}

rtgui_container_t* rtgui_rttab_get_container_by_title(rtgui_rttab_t* tab, const char* title)
{
	int i;
	rtgui_rttab_item_t* item;

	if(tab == RT_NULL) return RT_NULL;
	if(tab->tag_count <= 0) return RT_NULL;
	if(title == RT_NULL) return RT_NULL;

	for(i=0; i<tab->tag_count; i++)
	{
		item = &(tab->tags[i]);
		if(rt_strstr(title, item->tag->title) != RT_NULL)
		{
			return RTGUI_CONTAINER(item->tag);
		}
	}

	return RT_NULL;
}

rt_bool_t rtgui_rttab_switchto_next(rtgui_rttab_t* tab)
{
	if(tab == RT_NULL) return RT_FALSE;
	if(tab->tag_count <= 0) return RT_FALSE;

	if(tab->now_tag < tab->tag_count-1)
	{
		rtgui_widget_t* top;
		rtgui_rttab_item_t* item;

		item = &(tab->tags[tab->now_tag]);
		/* hide old item */
		RTGUI_WIDGET_HIDE(item->tag);

		tab->now_tag = tab->now_tag + 1;
		item = &(tab->tags[tab->now_tag]);
		RTGUI_WIDGET_UNHIDE(item->tag);
		top = (rtgui_widget_t*)rtgui_widget_get_toplevel(RTGUI_WIDGET(tab));
		rtgui_widget_update_clip(top);
		rtgui_widget_update(RTGUI_WIDGET(tab));
		return RT_TRUE;
	}
	return RT_FALSE;
}

rt_bool_t rtgui_rttab_switchto_prev(rtgui_rttab_t* tab)
{
	if(tab == RT_NULL) return RT_FALSE;
	if(tab->tag_count <= 0) return RT_FALSE;

	if(tab->now_tag > 0)
	{
		rtgui_widget_t* top;
		rtgui_rttab_item_t* item;

		item = &(tab->tags[tab->now_tag]);
		/* hide old item */
		RTGUI_WIDGET_HIDE(item->tag);

		tab->now_tag = tab->now_tag - 1;
		item = &(tab->tags[tab->now_tag]);
		RTGUI_WIDGET_UNHIDE(item->tag);
		top = (rtgui_widget_t*)rtgui_widget_get_toplevel(RTGUI_WIDGET(tab));
		rtgui_widget_update_clip(top);
		rtgui_widget_update(RTGUI_WIDGET(tab));
		return RT_TRUE;
	}
	return RT_FALSE;
}

static void rtgui_rttab_onmouse(rtgui_rttab_t* tab, struct rtgui_event_mouse* emouse)
{
	rtgui_rect_t rect, tagbar_rect;

	/* get physical extent information */
	rtgui_widget_get_rect(RTGUI_WIDGET(tab), &rect);
	rtgui_widget_rect_to_device(RTGUI_WIDGET(tab), &rect);

	tagbar_rect = rect;
	if(tab->orient==RTGUI_RTTAB_ORIENT_TOP)
	{
		tagbar_rect.y2 = tagbar_rect.y1 + RTGUI_WIDGET_BORDER(tab) + tab->tag_size;
	}
	else if(tab->orient==RTGUI_RTTAB_ORIENT_BOTTOM)
	{
		tagbar_rect.y1 = tagbar_rect.y1 + (RC_H(rect)-tab->tag_size);
		tagbar_rect.y2 = rect.y2;
	}

	if(tab->spin != RT_NULL)
	{
		/* cut off spin from tagbar area */
		rtgui_rect_t ppl_rect;
		rtgui_widget_get_rect(RTGUI_WIDGET(tab->spin), &ppl_rect);
		tagbar_rect.x2 -= RC_W(ppl_rect);
	}

	if(rtgui_rect_contains_point(&tagbar_rect, emouse->x, emouse->y) == RT_EOK)
	{
		/* on tag bar */
		int i;
		rtgui_rect_t tmp_rect = tagbar_rect;
		rtgui_rttab_item_t* item;

		for(i=tab->first_tag; i<tab->tag_count; i++)
		{
			item = &(tab->tags[i]);
			tmp_rect.x2 = tmp_rect.x1+item->tag_width;
			if(rtgui_rect_contains_point(&tmp_rect, emouse->x, emouse->y) == RT_EOK)
			{
				/* inside a tag */
				if(emouse->button & RTGUI_MOUSE_BUTTON_DOWN)
				{
					if(i != tab->now_tag)
					{
						rtgui_widget_t* top;
						item = &(tab->tags[tab->now_tag]);
						/* hide old item */
						RTGUI_WIDGET_HIDE(item->tag);

						tab->now_tag = i;
						item = &(tab->tags[tab->now_tag]);
						RTGUI_WIDGET_UNHIDE(item->tag);
						top = (rtgui_widget_t*)rtgui_widget_get_toplevel(RTGUI_WIDGET(tab));
						rtgui_widget_update_clip(top);
						rtgui_widget_update(RTGUI_WIDGET(tab));
					}
				}
				return;
			}
			tmp_rect.x1 = tmp_rect.x2;
		}
	}
	else
	{
		/* other child widget */
		rtgui_container_dispatch_mouse_event(RTGUI_CONTAINER(tab), emouse);
	}
}

rt_bool_t rtgui_rttab_event_handler(rtgui_object_t *object, rtgui_event_t* event)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(object);
	rtgui_rttab_t* tab = RTGUI_RTTAB(object);

	if(tab == RT_NULL)return RT_FALSE;

	switch(event->type)
	{
	case RTGUI_EVENT_PAINT:
#ifndef RTGUI_USING_SMALL_SIZE
		if(widget->on_draw != RT_NULL)
			widget->on_draw(widget, event);
		else
#endif
		{
			rtgui_rttab_ondraw(tab);
			/* paint on each child */
			rtgui_container_dispatch_event(RTGUI_CONTAINER(tab), event);
		}
		break;

	case RTGUI_EVENT_MOUSE_BUTTON:
#ifndef RTGUI_USING_SMALL_SIZE
		if(widget->on_mouseclick != RT_NULL)
		{
			widget->on_mouseclick(widget, event);
		}
		else
#endif
		{
			rtgui_rttab_onmouse(tab, (struct rtgui_event_mouse*)event);
		}
		break;
	case RTGUI_EVENT_SHOW:
		rtgui_widget_onshow(object, event);
		break;
	case RTGUI_EVENT_HIDE:
		rtgui_widget_onhide(object, event);
		break;
	default:
		return rtgui_container_event_handler(object, event);
	}

	return RT_FALSE;
}

/* bind widget: spin on click event */
static rt_bool_t rtgui_rttab_spin_onclick(rtgui_object_t *object, rtgui_event_t* event)
{
	int i;
	rtgui_rttab_t* tab = RTGUI_RTTAB(object);
	rtgui_rttab_item_t* item;
	rtgui_widget_t* top;

	for(i=0; i<tab->tag_count; i++)
	{
		item = &(tab->tags[i]);
		if(!RTGUI_WIDGET_IS_HIDE(item->tag))
		{
			RTGUI_WIDGET_HIDE(item->tag);
			item = &(tab->tags[tab->now_tag]);
			RTGUI_WIDGET_UNHIDE(item->tag);
			top = (rtgui_widget_t*)rtgui_widget_get_toplevel(RTGUI_WIDGET(tab));
			rtgui_widget_update_clip(top);
			rtgui_widget_update(RTGUI_WIDGET(tab));
		}
	}

	return RT_TRUE;
}
