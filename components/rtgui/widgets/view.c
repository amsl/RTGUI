/*
 * File      : view.c
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
#include <rtgui/widgets/view.h>

static void _rtgui_view_constructor(rtgui_view_t *view)
{
	/* init view */
	rtgui_widget_set_event_handler(view,rtgui_view_event_handler);

	rtgui_widget_set_border_style(view, RTGUI_BORDER_NONE);
	view->title = RT_NULL;
}

static void _rtgui_view_destructor(rtgui_view_t *view)
{
	if(view->title != RT_NULL)
	{
		rt_free(view->title);
		view->title = RT_NULL;
	}
}

DEFINE_CLASS_TYPE(view, "view",
                  RTGUI_CONTAINER_TYPE,
                  _rtgui_view_constructor,
                  _rtgui_view_destructor,
                  sizeof(struct rtgui_view));

rtgui_view_t* rtgui_view_create(pvoid parent,const char* title,int left,int top,int w,int h)
{
	rtgui_container_t *container;
	rtgui_view_t* view;

	RT_ASSERT(parent != RT_NULL);
	container = RTGUI_CONTAINER(parent);

	/* allocate view */
	view = rtgui_widget_create(RTGUI_VIEW_TYPE);
	if(view != RT_NULL)
	{
		rtgui_rect_t rect;
		rtgui_widget_get_rect(container,&rect);
		rtgui_widget_rect_to_device(container, &rect);
		rect.x1 += left;
		rect.y1 += top;
		rect.x2 = rect.x1+w;
		rect.y2 = rect.y1+h;
		rtgui_widget_set_rect(view,&rect);

		if(title != RT_NULL)
			view->title = rt_strdup(title);

		rtgui_container_add_child(container, view);
	}

	return view;
}

void rtgui_view_destroy(rtgui_view_t* view)
{
	rtgui_widget_destroy(view);
}

void rtgui_view_ondraw(rtgui_view_t* view)
{
	rtgui_rect_t rect;
	rtgui_dc_t* dc;

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(view);
	if(dc == RT_NULL)return;

	rtgui_widget_get_rect(view, &rect);
	rtgui_rect_inflate(&rect, -RTGUI_WIDGET_BORDER_SIZE(view));
	/* fill view with background */
	rtgui_dc_fill_rect(dc, &rect);
	rtgui_rect_inflate(&rect, RTGUI_WIDGET_BORDER_SIZE(view));
	rtgui_dc_draw_border(dc, &rect, RTGUI_WIDGET_BORDER_STYLE(view));

	rtgui_dc_end_drawing(dc);	
}

rt_bool_t rtgui_view_event_handler(pvoid wdt, rtgui_event_t* event)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	rtgui_view_t* view = RTGUI_VIEW(wdt);

	RT_ASSERT(wdt != RT_NULL);

	switch(event->type)
	{
		case RTGUI_EVENT_PAINT:
			if(widget->on_draw != RT_NULL)
				widget->on_draw(widget, event);
			else
				rtgui_view_ondraw(view);
			
			/* paint on each child */
			rtgui_container_dispatch_event(view, event);
			return RT_FALSE;

		default:
			return rtgui_container_event_handler(widget, event);
	}
}

rt_bool_t rtgui_view_show(rtgui_view_t* view)
{
	/* parameter check */
	if(view == RT_NULL) return RT_FALSE;

	rtgui_widget_show(view);

	return RT_TRUE;
}


void rtgui_view_hide(rtgui_view_t* view)
{
	if(view == RT_NULL) return;

	if(RTGUI_WIDGET_PARENT(view) == RT_NULL)
	{
		RTGUI_WIDGET_HIDE(view);
		return;
	}

	rtgui_widget_hide(view);
}

char* rtgui_view_get_title(rtgui_view_t* view)
{
	RT_ASSERT(view != RT_NULL);

	return view->title;
}

void rtgui_view_set_title(rtgui_view_t* view, const char *title)
{
	RT_ASSERT(view != RT_NULL);

	rt_free(view->title);

	if(title != RT_NULL)
		view->title = rt_strdup(title);
	else
		view->title = RT_NULL;
}


