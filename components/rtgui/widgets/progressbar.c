/*
 * File      : progressbar.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 *
 *
 */
#include <rtgui/widgets/progressbar.h>

#define RTGUI_PROGRESSBAR_DEFAULT_RANGE	100

static void _rtgui_progressbar_constructor(rtgui_progressbar_t *bar)
{
	rtgui_rect_t rect = {0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT};

	rtgui_widget_set_event_handler(bar, rtgui_progressbar_event_handler);
	rtgui_widget_set_rect(bar, &rect);

	bar->orient = RTGUI_HORIZONTAL;
	bar->range = RTGUI_PROGRESSBAR_DEFAULT_RANGE;
	bar->position = 0;

	/* set gc */
	RTGUI_WIDGET_TEXTALIGN(bar) = RTGUI_ALIGN_CENTER;
}

static void _rtgui_progressbar_destructor(rtgui_progressbar_t *bar)
{

}

DEFINE_CLASS_TYPE(progressbar, "progressbar",
                  RTGUI_WIDGET_TYPE,
                  _rtgui_progressbar_constructor,
                  _rtgui_progressbar_destructor,
                  sizeof(struct rtgui_progressbar));

rtgui_progressbar_t* rtgui_progressbar_create(pvoid parent, int orient, int range, int left, int top, int w, int h)
{
	rtgui_container_t *container;
	rtgui_progressbar_t* bar;

	RT_ASSERT(parent != RT_NULL);
	container = RTGUI_CONTAINER(parent);

	bar = rtgui_widget_create(RTGUI_PROGRESSBAR_TYPE);
	if(bar != RT_NULL)
	{
		rtgui_rect_t rect;
		rtgui_widget_get_rect(bar, &rect);
		rtgui_widget_rect_to_device(container,&rect);
		rect.x1 += left;
		rect.y1 += top;
		rect.x2 = rect.x1+w;
		rect.y2 = rect.y1+h;
		rtgui_widget_set_rect(bar, &rect);

		bar->orient = orient;
		bar->range = range;

		rtgui_container_add_child(container, bar);
	}

	return bar;
}

void rtgui_progressbar_destroy(rtgui_progressbar_t* bar)
{
	rtgui_widget_destroy(bar);
}

void rtgui_progressbar_ondraw(rtgui_progressbar_t* bar)
{
	/* draw progress bar */
	rtgui_dc_t* dc;
	rtgui_rect_t rect;
	int max = bar->range;
	int pos = bar->position;
	int left;
	rtgui_color_t bc;

	RT_ASSERT(bar != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(bar);
	if(dc == RT_NULL)return;

	bc = RTGUI_DC_BC(dc);
	rtgui_widget_get_rect(&(bar->parent), &rect);

	/* fill button rect with background color */
	RTGUI_DC_BC(dc) = RTGUI_RGB(212, 208, 200);

	/* draw border */
	rect.x2 --;
	rect.y2 --;
	rtgui_dc_draw_border(dc, &rect, RTGUI_WIDGET_BORDER_STYLE(bar));

	/* Nothing to draw */
	if(max == 0)
	{
		return;
	}

	rect.x2 ++;
	rect.y2 ++;
	left = max - pos;
	rtgui_rect_inflate(&rect, -2);
	RTGUI_DC_BC(dc) = Blue;

	if(bar->orient == RTGUI_VERTICAL)
	{
		/* Vertical bar grows from bottom to top */
		int dy = (RC_H(rect) * left) / max;
		rect.y1 += dy;
		rtgui_dc_fill_rect(dc,&rect);

		rect.y1 -= dy;
		rect.y2 = dy;
		RTGUI_DC_BC(dc) = theme.blankspace;
		rtgui_dc_fill_rect(dc,&rect);
	}
	else
	{
		/* Horizontal bar grows from left to right */
		int dx = (RC_W(rect) * left) / max;
		rect.x2 -= dx;
		rtgui_dc_fill_rect(dc,&rect);

		rect.x1 = rect.x2;
		rect.x2 += dx;
		RTGUI_DC_BC(dc) = theme.blankspace;
		rtgui_dc_fill_rect(dc, &rect);
	}
	RTGUI_DC_BC(dc) = bc;

	rtgui_dc_end_drawing(dc);
}

rt_bool_t rtgui_progressbar_event_handler(pvoid wdt, rtgui_event_t* event)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	rtgui_progressbar_t *bar = RTGUI_PROGRESSBAR(wdt);

	switch(event->type)
	{
		case RTGUI_EVENT_PAINT:
			if(widget->on_draw != RT_NULL)
				widget->on_draw(widget, event);
			else
			{
				rtgui_progressbar_ondraw(bar);
			}
			return RT_FALSE;
		default:
			return RT_FALSE;
	}
}

void rtgui_progressbar_set_value(rtgui_progressbar_t *bar, int value)
{
	RT_ASSERT(bar != RT_NULL);

	bar->position = value;

	rtgui_progressbar_ondraw(bar);
	return;
}

int rtgui_progressbar_get_value(rtgui_progressbar_t *bar)
{
	RT_ASSERT(bar != RT_NULL);

	return bar->position;
}

void rtgui_progressbar_set_range(rtgui_progressbar_t *bar, int range)
{
	RT_ASSERT(bar != RT_NULL);

	bar->range = range;

	rtgui_progressbar_ondraw(bar);
	return;
}

int rtgui_progressbar_get_range(rtgui_progressbar_t *bar)
{
	RT_ASSERT(bar != RT_NULL);

	return bar->range;
}

