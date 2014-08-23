/*
 * File      : slider.c
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
#include <rtgui/widgets/slider.h>

#define RTGUI_SLIDER_DEFAULT_WIDTH	100
#define RTGUI_SLIDER_DEFAULT_HEIGHT	20
#define RTGUI_SLIDER_DEFAULT_MIN	0
#define RTGUI_SLIDER_DEFAULT_MAX	100

static void _rtgui_slider_constructor(rtgui_slider_t *slider)
{
	/* init widget and set event handler */
	RTGUI_WIDGET_FLAG(slider) |= RTGUI_WIDGET_FLAG_FOCUSABLE;
	rtgui_widget_set_event_handler(slider, rtgui_slider_event_handler);

	/* set proper of control */
	slider->min = RTGUI_SLIDER_DEFAULT_MIN;
	slider->max = RTGUI_SLIDER_DEFAULT_MAX;
	slider->value = RTGUI_SLIDER_DEFAULT_MIN;
	slider->orient = RTGUI_HORIZONTAL;

	slider->ticks = 10;
	slider->thumb_width = 6;
	slider->on_change = RT_NULL;
}

static void _rtgui_slider_destructor(rtgui_slider_t *slider)
{
}

DEFINE_CLASS_TYPE(slider, "slider",
                  RTGUI_WIDGET_TYPE,
                  _rtgui_slider_constructor,
                  _rtgui_slider_destructor,
                  sizeof(struct rtgui_slider));

/* create a slider */
rtgui_slider_t* rtgui_slider_create(pvoid parent, int left, int top, int w, int h, int orient, int min, int max)
{
	rtgui_container_t *container;
	rtgui_slider_t* slider;

	RT_ASSERT(parent != RT_NULL);
	container = RTGUI_CONTAINER(parent);

	slider = rtgui_widget_create(RTGUI_SLIDER_TYPE);
	if(slider != RT_NULL)
	{
		rtgui_rect_t rect;

		/* set proper of control */
		slider->min = min;
		slider->max = max;
		slider->value = min;

		slider->ticks = max-min;
		slider->thumb_width = 6;/* thumb size */

		rtgui_slider_set_orient(slider, orient);

		rtgui_widget_get_rect(container, &rect);
		rtgui_widget_rect_to_device(container,&rect);
		rect.x1 += left;
		rect.y1 += top;
		rect.x2 = rect.x1+w;
		rect.y2 = rect.y1+h;
		rtgui_widget_set_rect(slider, &rect);

		rtgui_container_add_child(container, slider);
	}

	return slider;
}

void rtgui_slider_destroy(rtgui_slider_t* slider)
{
	rtgui_widget_destroy(slider);
}

static void rtgui_slider_onmouse(rtgui_slider_t* slider, struct rtgui_event_mouse* event)
{
	RT_ASSERT(slider != RT_NULL);
	RT_ASSERT(event  != RT_NULL);

	if(event->button & RTGUI_MOUSE_BUTTON_DOWN && event->button & RTGUI_MOUSE_BUTTON_LEFT)
	{
		int sel;
		int range = slider->max - slider->min;
		int x0, xsize;
		int x;
		x0 = 1 + slider->thumb_width/2;

		if(slider->orient == RTGUI_VERTICAL)
		{
			x = event->y - RTGUI_WIDGET(slider)->extent.y1;
			x -= x0;
			xsize = rtgui_widget_get_height(slider) - 2 * x0;
		}
		else
		{
			x = event->x - RTGUI_WIDGET(slider)->extent.x1;
			x -= x0;
			xsize = rtgui_widget_get_width(slider) - 2 * x0;
		}

		if(x <= 0)
		{
			sel = slider->min;
		}
		else if(x >= xsize)
		{
			sel = slider->max;
		}
		else
		{
			sel = ((range * x) + xsize/2) / xsize;
			sel += slider->min;
		}

		rtgui_widget_focus(slider);
		rtgui_slider_set_value(slider, sel);
		
		if(slider->on_change != RT_NULL) 
			slider->on_change(slider,(rtgui_event_t*)event);
	}
}

static void rtgui_slider_onkey(rtgui_slider_t* slider, struct rtgui_event_kbd *event)
{
	RT_ASSERT(slider != RT_NULL);
	RT_ASSERT(event != RT_NULL);

	if(!(RTGUI_KBD_IS_UP(event))) return;

	if(event->key == RTGUIK_LEFT)
	{
		if(slider->value > slider->min)
			slider->value ++;
	}

	if(event->key == RTGUIK_RIGHT)
	{
		if(slider->value < slider->max)
			slider->value --;
	}

	/* update widget */
	rtgui_widget_update(slider);
}

void rtgui_slider_ondraw(rtgui_slider_t* slider)
{
	/* draw button */
	int i, xsize, x0;
	rtgui_rect_t rect, focus_rect, slider_rect, slot_rect;
	rtgui_dc_t* dc;

	RT_ASSERT(slider != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(slider);
	if(dc == RT_NULL)return;

	/* get widget rect */
	rtgui_widget_get_rect(slider, &focus_rect);
	/* fill widget rect with background color */
	rtgui_dc_fill_rect(dc,&focus_rect);
	rect = focus_rect;

	if(slider->orient == RTGUI_VERTICAL)
	{
		rtgui_rect_inflate(&rect, -1);
		xsize = rect.y2 - rect.y1 - slider->thumb_width;
		x0 = rect.y1 + slider->thumb_width / 2;

		/* calculate thumb position */
		slider_rect = rect;
		slider_rect.x1 += 1;
		slider_rect.x2 -= 1;
		slider_rect.y1 = x0 + xsize * (slider->value - slider->min) / (slider->max - slider->min) - slider->thumb_width/2;
		slider_rect.y2  = slider_rect.y1 + slider->thumb_width;

		/* calculate slot position */
		slot_rect.y1 = x0;
		slot_rect.y2 = x0 + xsize;
		slot_rect.x1 = (slider_rect.x1 + slider_rect.x2) /2 -1;
		slot_rect.x2 = slot_rect.x1 +3;
		RTGUI_DC_FC(dc) = theme.foreground;
		/* draw slot */
		rtgui_dc_draw_border(dc, &slot_rect, RTGUI_WIDGET_BORDER_STYLE(slider));

		/* draw the ticks */
		for(i = 0; i <= slider->ticks; i++)
		{
			int k = x0 + xsize * i / slider->ticks;
			rtgui_dc_draw_hline(dc,rect.x1+1, rect.x1+4, k);
		}

		/* draw the thumb */
		rtgui_dc_fill_rect(dc,&slider_rect);
		rtgui_dc_draw_border(dc, &slider_rect, RTGUI_WIDGET_BORDER_STYLE(slider));
	}
	else
	{
		rtgui_rect_inflate(&rect, -1);
		xsize = rect.x2 - rect.x1 - slider->thumb_width;
		x0 = rect.x1 + slider->thumb_width / 2;

		/* calculate thumb position */
		slider_rect = rect;
		slider_rect.x1 = x0 + xsize*(slider->value - slider->min)/(slider->max - slider->min) - slider->thumb_width/2;
		slider_rect.x2  = slider_rect.x1 + slider->thumb_width;
		slider_rect.y1 += 1;
		slider_rect.y2 -= 1;

		/* calculate slot position */
		slot_rect.x1 = x0;
		slot_rect.x2 = x0 + xsize;
		slot_rect.y1 = (slider_rect.y1 + slider_rect.y2) /2 -1;
		slot_rect.y2 = slot_rect.y1 +3;
		/* draw slot */
		rtgui_dc_draw_border(dc, &slot_rect, RTGUI_WIDGET_BORDER_STYLE(slider));

		/* draw the ticks */
		for(i = 0; i <= slider->ticks; i++)
		{
			int x = x0 + xsize * i / slider->ticks;
			rtgui_dc_draw_vline(dc,x, rect.y1+1, rect.y1+4);
		}

		/* draw the thumb */
		rtgui_dc_fill_rect(dc,&slider_rect);
		rtgui_dc_draw_border(dc, &slider_rect, RTGUI_WIDGET_BORDER_STYLE(slider));
	}

	/* draw focus */
	if(RTGUI_WIDGET_IS_FOCUSED(slider))
	{
		RTGUI_DC_FC(dc) = theme.foreground;
		rtgui_dc_draw_focus_rect(dc, &focus_rect);
	}

	rtgui_dc_end_drawing(dc);
}

rt_bool_t rtgui_slider_event_handler(pvoid wdt, rtgui_event_t* event)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	rtgui_slider_t *slider = RTGUI_SLIDER(wdt);

	switch(event->type)
	{
		case RTGUI_EVENT_PAINT:
			if(widget->on_draw != RT_NULL)
				widget->on_draw(widget, event);
			else
				rtgui_slider_ondraw(slider);

			return RT_FALSE;

		case RTGUI_EVENT_KBD:
			if(widget->on_key != RT_NULL)
				widget->on_key(widget, event);
			else
				rtgui_slider_onkey(slider, (struct rtgui_event_kbd *)event);
			return RT_TRUE;

		case RTGUI_EVENT_MOUSE_BUTTON:
			if(widget->on_mouseclick != RT_NULL)
				widget->on_mouseclick(widget, event);
			else
				rtgui_slider_onmouse(slider, (struct rtgui_event_mouse*)event);
			return RT_TRUE;
		default:
			return RT_FALSE;
	}
}

void rtgui_slider_set_range(rtgui_slider_t* slider, rt_size_t min, rt_size_t max)
{
	RT_ASSERT(slider != RT_NULL);

	slider->max = max;
	slider->min = min;
}

void rtgui_slider_set_value(rtgui_slider_t* slider, rt_size_t value)
{
	RT_ASSERT(slider != RT_NULL);

	if(value < slider->min) value = slider->min;
	if(value > slider->max) value = slider->max;

	if(slider->value != value)
	{
		slider->value = value;
		rtgui_slider_ondraw(slider);
	}
}

/* set slider draw orientation */
void rtgui_slider_set_orient(rtgui_slider_t* slider, int orient)
{
	RT_ASSERT(slider != RT_NULL);

	/* set orient */
	slider->orient = orient;
}

rt_size_t rtgui_slider_get_value(rtgui_slider_t* slider)
{
	RT_ASSERT(slider != RT_NULL);

	return slider->value;
}
