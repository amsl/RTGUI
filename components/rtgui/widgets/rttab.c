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
#include <rtgui/widgets/rttab.h>

static rt_bool_t rtgui_rttab_ppl_onclick(pvoid wdt, rtgui_event_t* event);

static void _rtgui_rttab_constructor(rtgui_rttab_t *tab)
{
	/* init widget and set event handler */
	RTGUI_WIDGET_FLAG(tab) |= RTGUI_WIDGET_FLAG_FOCUSABLE;
	rtgui_widget_set_event_handler(tab, rtgui_rttab_event_handler);

	tab->orient = RTGUI_RTTAB_ORIENT_TOP;
	tab->tag_size = RTGUI_RTTAB_DEFAULT_TITLE_SIZE;
	tab->focus_color = Blue; /* please reset it */
	tab->tag_count = 0;
	tab->first_tag = 0;
	tab->now_tag   = 0;
	tab->tags      = RT_NULL;
	tab->spin       = RT_NULL; /* spin widget */
}

static void _rtgui_rttab_destructor(rtgui_rttab_t *tab)
{
}

DEFINE_CLASS_TYPE(rttab, "rttab",
                  RTGUI_CONTAINER_TYPE,
                  _rtgui_rttab_constructor,
                  _rtgui_rttab_destructor,
                  sizeof(struct rtgui_rttab));

rtgui_rttab_t* rtgui_rttab_create(pvoid parent, int left, int top, int w, int h)
{
	rtgui_container_t *container;
	rtgui_rttab_t* tab;

	RT_ASSERT(parent != RT_NULL);
	container = RTGUI_CONTAINER(parent);

	tab = rtgui_widget_create(RTGUI_RTTAB_TYPE);
	if(tab != RT_NULL)
	{
		rtgui_rect_t rect;
		int height = 0;
		/* set default rect */
		rtgui_widget_get_rect(container, &rect);
		rtgui_widget_rect_to_device(container,&rect);
		rect.x1 += left;
		rect.y1 += top;
		rect.x2 = rect.x1+w;
		rect.y2 = rect.y1+h;
		rtgui_widget_set_rect(tab, &rect);
		rtgui_container_add_child(container, tab);

		height = FONT_H(RTGUI_WIDGET_FONT(tab));
		tab->tag_size = height + RTGUI_MARGIN*2;
	}

	return tab;
}

void rtgui_rttab_destroy(rtgui_rttab_t* tab)
{
	rtgui_widget_destroy(tab);
}

/* draw left/top/right */
void rtgui_rttab_draw_title_rect(rtgui_dc_t* dc, rtgui_rect_t* rect,
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
	rtgui_dc_t* dc;

	RT_ASSERT(tab != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(tab);
	if(dc == RT_NULL)return;

	rtgui_widget_get_rect(tab, &rect);

	/* draw rttab widget border */
	tmp_rect = rect;
	if(tab->orient==RTGUI_RTTAB_ORIENT_TOP)
	{
		tmp_rect.y2 = tmp_rect.y1 + RTGUI_WIDGET_BORDER_SIZE(tab) + tab->tag_size;
		RTGUI_DC_FC(dc) = RTGUI_WIDGET_BC(tab);
		/* draw tagbar border */
		rtgui_dc_draw_line(dc,tmp_rect.x1, tmp_rect.y1, tmp_rect.x1, tmp_rect.y2);
		rtgui_dc_draw_line(dc,tmp_rect.x2-1, tmp_rect.y1, tmp_rect.x2-1, tmp_rect.y2);
		rtgui_dc_draw_line(dc,tmp_rect.x1, tmp_rect.y1, tmp_rect.x2, tmp_rect.y1);

		tmp_rect.y1 = tmp_rect.y2-1;
		tmp_rect.y2 = rect.y2;
		/* fill client background */
		rtgui_rect_inflate(&tmp_rect,-RTGUI_WIDGET_BORDER_SIZE(tab));
		rtgui_dc_fill_rect(dc, &tmp_rect);
		rtgui_rect_inflate(&tmp_rect,RTGUI_WIDGET_BORDER_SIZE(tab));
		/* draw client border */
		RTGUI_DC_FC(dc) = theme.blankspace;
		rtgui_dc_draw_line(dc,tmp_rect.x1, tmp_rect.y1, tmp_rect.x1, tmp_rect.y2-1);
		RTGUI_DC_FC(dc) = DimGray;
		rtgui_dc_draw_line(dc,tmp_rect.x2-1, tmp_rect.y1, tmp_rect.x2-1, tmp_rect.y2-1);
		rtgui_dc_draw_line(dc,tmp_rect.x1, tmp_rect.y2-1, tmp_rect.x2, tmp_rect.y2-1);
	}
	else if(tab->orient==RTGUI_RTTAB_ORIENT_BOTTOM)
	{
		tmp_rect.y2 = tmp_rect.y1 + (RC_H(rect)-tab->tag_size);
		/* fill client background */
		rtgui_rect_inflate(&tmp_rect,-RTGUI_WIDGET_BORDER_SIZE(tab));
		rtgui_dc_fill_rect(dc, &tmp_rect);
		rtgui_rect_inflate(&tmp_rect,RTGUI_WIDGET_BORDER_SIZE(tab));
		/* draw client border */
		RTGUI_DC_FC(dc) = theme.blankspace;
		rtgui_dc_draw_line(dc,tmp_rect.x1, tmp_rect.y1, tmp_rect.x1, tmp_rect.y2-1);
		rtgui_dc_draw_line(dc,tmp_rect.x1, tmp_rect.y1, tmp_rect.x2, tmp_rect.y1);
		RTGUI_DC_FC(dc) = DimGray;
		rtgui_dc_draw_line(dc,tmp_rect.x2-1, tmp_rect.y1, tmp_rect.x2-1, tmp_rect.y2);

		tmp_rect.y1 = tmp_rect.y2-1;
		tmp_rect.y2 = rect.y2;
		/* draw tagbar border */
		RTGUI_DC_FC(dc) = RTGUI_WIDGET_BC(tab);
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
			tmp_rect.y2 = tmp_rect.y1 + RTGUI_WIDGET_BORDER_SIZE(tab) + tab->tag_size;
			/* fill tagbar background, use same color as client area. */
			RTGUI_DC_BC(dc) = RTGUI_WIDGET_BC(item->tag);
			rtgui_dc_fill_rect(dc, &tmp_rect);
			/* draw tagbutton border */
			rtgui_rttab_draw_title_rect(dc, &tmp_rect, theme.blankspace, DimGray, RTGUI_RTTAB_ORIENT_TOP);
			rtgui_rect_inflate(&tmp_rect, -1);
			rtgui_rttab_draw_title_rect(dc, &tmp_rect, theme.background, Gray, RTGUI_RTTAB_ORIENT_TOP);
			rtgui_rect_inflate(&tmp_rect, 1);

			if(i == tab->now_tag)
			{
				_left = tmp_rect.x1;
				_right = tmp_rect.x2;
				RTGUI_DC_FC(dc) = tab->focus_color;
			}
			else
			{
				RTGUI_DC_FC(dc) = theme.foreground;
			}

			if(item->image != RT_NULL)
			{
				rtgui_rect_t image_rect = {0};
				image_rect.x2 = image_rect.x1+item->image->w;
				image_rect.y2 = image_rect.y1+item->image->h;
				rtgui_rect_moveto_align(&tmp_rect, &image_rect, RTGUI_ALIGN_CENTER_VERTICAL);
				rtgui_image_paste(item->image, dc, &image_rect, theme.blankspace);
				tmp_rect.x1 += item->image->w;
			}
			rtgui_dc_draw_text(dc, item->tag->title, &tmp_rect);
			tmp_rect.x1 = tmp_rect.x2;
		}
		tmp_rect.x2 = rect.x2;
		if(RTGUI_WIDGET_PARENT(tab) != RT_NULL)
			RTGUI_DC_BC(dc) = RTGUI_WIDGET_BC(RTGUI_WIDGET_PARENT(tab));
		else
			RTGUI_DC_BC(dc) = theme.background;
		rtgui_dc_fill_rect(dc, &tmp_rect);

		RTGUI_DC_FC(dc) = theme.blankspace;
		rtgui_dc_draw_line(dc,rect.x1, tmp_rect.y2-1, _left, tmp_rect.y2-1);
		rtgui_dc_draw_line(dc,_right, tmp_rect.y2-1, rect.x2, tmp_rect.y2-1);
	}
	else if(tab->orient == RTGUI_RTTAB_ORIENT_BOTTOM)
	{
		int h;
		h = RC_H(rect)-RTGUI_WIDGET_BORDER_SIZE(tab)*2 - tab->tag_size;
		tmp_rect.y1 += RTGUI_WIDGET_BORDER_SIZE(tab) + h;
		tmp_rect.y2 = tmp_rect.y1 + tab->tag_size;

		for(i=tab->first_tag; i<tab->tag_count; i++)
		{
			item = &(tab->tags[i]);
			tmp_rect.x2 = tmp_rect.x1+item->tag_width;
			tmp_rect.y2 = tmp_rect.y1+tab->tag_size;
			RTGUI_DC_BC(dc) = RTGUI_WIDGET_BC(item->tag);
			rtgui_dc_fill_rect(dc, &tmp_rect);
			/* draw tagbutton border */
			rtgui_rttab_draw_title_rect(dc, &tmp_rect, theme.blankspace, DimGray, RTGUI_RTTAB_ORIENT_BOTTOM);
			rtgui_rect_inflate(&tmp_rect, -1);
			rtgui_rttab_draw_title_rect(dc, &tmp_rect, theme.background, Gray, RTGUI_RTTAB_ORIENT_BOTTOM);
			rtgui_rect_inflate(&tmp_rect, 1);

			if(i == tab->now_tag)
			{
				_left = tmp_rect.x1;
				_right = tmp_rect.x2;
				RTGUI_DC_FC(dc) = tab->focus_color;
			}
			else
			{
				RTGUI_DC_FC(dc) = theme.foreground;
			}

			if(item->image != RT_NULL)
			{
				rtgui_rect_t image_rect = {0};
				image_rect.x2 = image_rect.x1+item->image->w;
				image_rect.y2 = image_rect.y1+item->image->h;
				rtgui_rect_moveto_align(&tmp_rect, &image_rect, RTGUI_ALIGN_CENTER_VERTICAL);
				rtgui_image_paste(item->image, dc, &image_rect, theme.blankspace);
				tmp_rect.x1 += item->image->w;
			}
			rtgui_dc_draw_text(dc, item->tag->title, &tmp_rect);
			tmp_rect.x1 = tmp_rect.x2;
		}
		tmp_rect.x2 = rect.x2;
		if(RTGUI_WIDGET_PARENT(tab) != RT_NULL)
			RTGUI_DC_BC(dc) = RTGUI_WIDGET_BC(RTGUI_WIDGET_PARENT(tab));
		else
			RTGUI_DC_BC(dc) = theme.background;
		rtgui_dc_fill_rect(dc, &tmp_rect);

		RTGUI_DC_FC(dc) = DimGray;
		rtgui_dc_draw_line(dc,rect.x1, tmp_rect.y1, _left, tmp_rect.y1);
		rtgui_dc_draw_line(dc,_right, tmp_rect.y1, rect.x2, tmp_rect.y1);
	}

	rtgui_dc_end_drawing(dc);
}

/* add a new page tag */
void rtgui_rttab_add_tag(rtgui_rttab_t* tab, const char* name, rtgui_image_t* image)
{
	rtgui_rect_t rect;
	rtgui_rttab_item_t item= {0}, *_tags;
	int w, h, _top;

	RT_ASSERT(tab != RT_NULL);

	rtgui_widget_get_rect(tab, &rect);

	w = RC_W(rect)-RTGUI_WIDGET_BORDER_SIZE(tab)*2;
	h = RC_H(rect)-RTGUI_WIDGET_BORDER_SIZE(tab)*2 - tab->tag_size;

	if(tab->orient==RTGUI_RTTAB_ORIENT_TOP)
		_top = RTGUI_WIDGET_BORDER_SIZE(tab) + tab->tag_size;
	else
		_top = RTGUI_WIDGET_BORDER_SIZE(tab);

	item.tag = rtgui_view_create(tab, name, RTGUI_WIDGET_BORDER_SIZE(tab), _top, w, h);
	if(item.tag == RT_NULL) return;
	RTGUI_WIDGET_HIDE(item.tag);
	
	item.image = image;
	item.tag_width = rtgui_font_get_string_width(RTGUI_WIDGET_FONT(tab), name);
	item.tag_width += RTGUI_MARGIN*2;
	if(item.image != RT_NULL) 
		item.tag_width += item.image->w+RTGUI_MARGIN;

	if(tab->tag_count == 0)
		_tags = rt_malloc(sizeof(rtgui_rttab_item_t));
	else
		_tags = rt_realloc(tab->tags,sizeof(rtgui_rttab_item_t)*(tab->tag_count+1));

	_tags[tab->tag_count].tag = item.tag;
	_tags[tab->tag_count].image = item.image;
	_tags[tab->tag_count].tag_width = item.tag_width;
	tab->tag_count++;
	tab->tags = _tags;

	RTGUI_WIDGET_SHOW(_tags[tab->now_tag].tag);

	{
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
					rtgui_widget_get_rect(tab, &rect);
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
					ppl_top = h+RTGUI_WIDGET_BORDER_SIZE(tab)*2;
				tab->spin = rtgui_spin_create(tab, RC_W(rect)-ppl_h*2,
				                               ppl_top, ppl_h*2, ppl_h, RTGUI_HORIZONTAL);
				rtgui_spin_bind(tab->spin, (rt_uint32_t*)&(tab->first_tag));
				tab->spin->widget_link = tab;
				tab->spin->on_click = rtgui_rttab_ppl_onclick;
				tab->spin->range_max = hide_num;
			}
			else
			{
				tab->spin->range_max = hide_num;
			}
		}
	}
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

rtgui_container_t* rtgui_rttab_get_container_by_title(rtgui_rttab_t* tab, char* title)
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
		pvoid top;
		rtgui_rttab_item_t* item;
		
		item = &(tab->tags[tab->now_tag]);
		RTGUI_WIDGET_HIDE(item->tag);
		
		tab->now_tag = tab->now_tag + 1;
		item = &(tab->tags[tab->now_tag]);
		RTGUI_WIDGET_SHOW(item->tag);
		top = rtgui_widget_get_toplevel(tab);
		rtgui_widget_update_clip(top);
		rtgui_widget_update(tab);
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
		pvoid top;
		rtgui_rttab_item_t* item;
		
		item = &(tab->tags[tab->now_tag]);
		RTGUI_WIDGET_HIDE(item->tag);
		
		tab->now_tag = tab->now_tag - 1;
		item = &(tab->tags[tab->now_tag]);
		RTGUI_WIDGET_SHOW(item->tag);
		top = rtgui_widget_get_toplevel(tab);
		rtgui_widget_update_clip(top);
		rtgui_widget_update(tab);
		return RT_TRUE;
	}
	return RT_FALSE;
}

static void rtgui_rttab_onmouse(rtgui_rttab_t* tab, struct rtgui_event_mouse* emouse)
{
	rtgui_rect_t rect, tagbar_rect;

	/* get physical extent information */
	rtgui_widget_get_rect(tab, &rect);
	rtgui_widget_rect_to_device(tab, &rect);

	tagbar_rect = rect;
	if(tab->orient==RTGUI_RTTAB_ORIENT_TOP)
	{
		tagbar_rect.y2 = tagbar_rect.y1 + RTGUI_WIDGET_BORDER_SIZE(tab) + tab->tag_size;
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
		rtgui_widget_get_rect(tab->spin, &ppl_rect);
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
						RTGUI_WIDGET_SHOW(item->tag);
						top = rtgui_widget_get_toplevel(tab);
						rtgui_widget_update_clip(top);
						rtgui_widget_update(tab);
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
		rtgui_container_dispatch_mouse_event(tab, emouse);
	}
}

rt_bool_t rtgui_rttab_event_handler(pvoid wdt, rtgui_event_t* event)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	rtgui_rttab_t* tab = RTGUI_RTTAB(wdt);

	if(tab == RT_NULL)return RT_FALSE;

	switch(event->type)
	{
	case RTGUI_EVENT_PAINT:
		if(widget->on_draw != RT_NULL)
			widget->on_draw(widget, event);
		else
		{
			rtgui_rttab_ondraw(tab);
			/* paint on each child */
			rtgui_container_dispatch_event(tab, event);
		}
		return RT_FALSE;

	case RTGUI_EVENT_MOUSE_BUTTON:
		if(widget->on_mouseclick != RT_NULL)
		{
			widget->on_mouseclick(widget, event);
		}
		else
		{
			rtgui_rttab_onmouse(tab, (struct rtgui_event_mouse*)event);
		}
		rtgui_container_dispatch_event(tab, event);
		return RT_TRUE;

	default:
		return rtgui_container_event_handler(tab, event);
	}
}

/* bind widget: spin on click event */
static rt_bool_t rtgui_rttab_ppl_onclick(pvoid wdt, rtgui_event_t* event)
{
	int i;
	rtgui_rttab_t* tab = RTGUI_RTTAB(wdt);
	rtgui_rttab_item_t* item;
	rtgui_widget_t* top;

	for(i=0; i<tab->tag_count; i++)
	{
		item = &(tab->tags[i]);
		if(!RTGUI_WIDGET_IS_HIDE(item->tag))
		{
			RTGUI_WIDGET_HIDE(item->tag);
			item = &(tab->tags[tab->now_tag]);
			RTGUI_WIDGET_SHOW(item->tag);
			top = rtgui_widget_get_toplevel(tab);
			rtgui_widget_update_clip(top);
			rtgui_widget_update(tab);
		}
	}

	return RT_TRUE;
}
