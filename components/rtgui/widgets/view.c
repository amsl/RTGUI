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
#include <rtgui/dc.h>
#include <rtgui/widgets/view.h>

static void _rtgui_view_constructor(rtgui_view_t *view)
{
	/* init view */
	rtgui_object_set_event_handler(RTGUI_OBJECT(view),rtgui_view_event_handler);

	rtgui_widget_set_border(RTGUI_WIDGET(view), RTGUI_BORDER_NONE);
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

rtgui_view_t* rtgui_view_create(rtgui_container_t *container,const char* title,int left,int top,int w,int h)
{
	rtgui_view_t* view;

	RT_ASSERT(container != RT_NULL);

	/* allocate view */
	view = (rtgui_view_t *) rtgui_widget_create(RTGUI_VIEW_TYPE);
	if(view != RT_NULL)
	{
		rtgui_rect_t rect;
		rtgui_widget_get_rect(RTGUI_WIDGET(container),&rect);
		rtgui_widget_rect_to_device(RTGUI_WIDGET(container), &rect);
		rect.x1 += left;
		rect.y1 += top;
		rect.x2 = rect.x1+w;
		rect.y2 = rect.y1+h;
		rtgui_widget_set_rect(RTGUI_WIDGET(view),&rect);

		if(title != RT_NULL)
			view->title = rt_strdup(title);

		rtgui_container_add_child(container, RTGUI_WIDGET(view));
	}

	return view;
}

void rtgui_view_destroy(rtgui_view_t* view)
{
	rtgui_widget_destroy(RTGUI_WIDGET(view));
}

void rtgui_view_ondraw(rtgui_view_t* view)
{
	rtgui_rect_t rect;
	struct rtgui_dc* dc;

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(RTGUI_WIDGET(view));
	if(dc == RT_NULL)return;

	rtgui_widget_get_rect(RTGUI_WIDGET(view), &rect);
	rtgui_rect_inflate(&rect, -RTGUI_WIDGET_BORDER(view));
	/* fill view with background */
	rtgui_dc_fill_rect(dc, &rect);
	rtgui_rect_inflate(&rect, RTGUI_WIDGET_BORDER(view));
	rtgui_dc_draw_border(dc, &rect, RTGUI_WIDGET_BORDER_STYLE(view));

	rtgui_dc_end_drawing(dc);	
}

rt_bool_t rtgui_view_event_handler(rtgui_object_t *object, rtgui_event_t* event)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(object);
	rtgui_view_t* view = RTGUI_VIEW(object);

	RT_ASSERT(object != RT_NULL);

	switch(event->type)
	{
	case RTGUI_EVENT_PAINT:
#ifndef RTGUI_USING_SMALL_SIZE
		if(widget->on_draw != RT_NULL)
			widget->on_draw(widget, event);
		else
#endif
			rtgui_view_ondraw(view);
		
		/* paint on each child */
		rtgui_container_dispatch_event(RTGUI_CONTAINER(view), event);
		break;

	default:
		return rtgui_container_event_handler(object, event);
	}

	return RT_FALSE;
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


