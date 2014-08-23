/*
 * File      : staticline.c
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
#include <rtgui/widgets/staticline.h>

static void _rtgui_static_constructor(rtgui_staticline_t *sline)
{
	/* init widget and set event handler */
	rtgui_rect_t rect = {0, 0, 100, 2};

	rtgui_widget_set_rect(sline, &rect);
	sline->orient= RTGUI_HORIZONTAL;

	rtgui_widget_set_event_handler(sline, rtgui_staticline_event_handler);
}

static void _rtgui_static_destructor(rtgui_staticline_t *sline)
{

}

DEFINE_CLASS_TYPE(staticline, "staticline",
                  RTGUI_WIDGET_TYPE,
                  _rtgui_static_constructor,
                  _rtgui_static_destructor,
                  sizeof(struct rtgui_staticline));

void rtgui_staticline_ondraw(rtgui_staticline_t* sline)
{
	rtgui_rect_t rect;
	rtgui_dc_t* dc;

	RT_ASSERT(sline != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(sline);
	if(dc == RT_NULL)return;

	rtgui_widget_get_rect(sline, &rect);
	rtgui_dc_fill_rect(dc,&rect);

	if(sline->orient == RTGUI_HORIZONTAL)
	{
		rtgui_dc_draw_horizontal_line(dc, rect.x1, rect.x2, rect.y1);
	}
	else
	{
		rtgui_dc_draw_vertical_line(dc, rect.x1, rect.y1, rect.y2);
	}

	rtgui_dc_end_drawing(dc);
}

rt_bool_t rtgui_staticline_event_handler(pvoid wdt, rtgui_event_t* event)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	rtgui_staticline_t* sline = RTGUI_STATICLINE(wdt);
	RT_ASSERT(wdt != RT_NULL);

	switch(event->type)
	{
		case RTGUI_EVENT_PAINT:
			if(widget->on_draw != RT_NULL) widget->on_draw(widget, event);
			else
				rtgui_staticline_ondraw(sline);
			return RT_FALSE;
		default:
			return RT_FALSE;
	}
}

rtgui_staticline_t * rtgui_staticline_create(pvoid parent,int left,int top,int w,int len,int orient)
{
	rtgui_container_t *container;
	rtgui_staticline_t* sline;

	RT_ASSERT(parent != RT_NULL);
	container = RTGUI_CONTAINER(parent);

	sline = rtgui_widget_create(RTGUI_STATICLINE_TYPE);
	if(sline!= RT_NULL)
	{
		rtgui_rect_t rect;

		rtgui_widget_get_rect(container, &rect);
		rtgui_widget_rect_to_device(container,&rect);
		rect.x1 += left;
		rect.y1 += top;

		sline->orient = orient;

		if(orient == RTGUI_HORIZONTAL)
		{
			rect.x2 = rect.x1+len;
			rect.y2 = rect.y1+w;
		}
		else
		{
			rect.x2 = rect.x1+w;
			rect.y2 = rect.y1+len;
		}

		rtgui_widget_set_rect(sline,&rect);

		rtgui_container_add_child(container, sline);
	}

	return sline;
}

void rtgui_staticline_destroy(rtgui_staticline_t* sline)
{
	rtgui_widget_destroy(sline);
}

