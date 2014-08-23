/*
 * File      : label.c
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
#include <string.h>
#include <rtgui/widgets/label.h>

static void _rtgui_label_constructor(rtgui_label_t *label)
{
	/* init widget and set event handler */
	rtgui_widget_set_event_handler(label, rtgui_label_event_handler);

	RTGUI_WIDGET_FC(label) = theme.foreground;
	RTGUI_WIDGET_BC(label) = theme.background;
	RTGUI_WIDGET_TEXTALIGN(label) = RTGUI_ALIGN_CENTER_VERTICAL;
	/* set field */
	label->text = RT_NULL;
}

static void _rtgui_label_destructor(rtgui_label_t *label)
{
	/* release text memory */
	if(label->text != RT_NULL)
	{
		rt_free(label->text);
		label->text = RT_NULL;
	}
}

DEFINE_CLASS_TYPE(label, "label",
                  RTGUI_WIDGET_TYPE,
                  _rtgui_label_constructor,
                  _rtgui_label_destructor,
                  sizeof(struct rtgui_label));

void rtgui_label_ondraw(rtgui_label_t* label)
{
	/* draw label */
	rtgui_rect_t rect;
	rtgui_dc_t* dc;

	RT_ASSERT(label != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(label);
	if(dc == RT_NULL)return;

	rtgui_widget_get_rect(label, &rect);
	rtgui_dc_fill_rect(dc,&rect);

	/* default left and center draw */
	rect.x1 += 1;
	if(RTGUI_WIDGET_FLAG(label) & RTGUI_WIDGET_FLAG_FONT_STROKE)
		rtgui_dc_draw_text_stroke(dc, label->text, &rect, theme.foreground, theme.blankspace);
	else
		rtgui_dc_draw_text(dc, label->text, &rect);

	rtgui_dc_end_drawing(dc);
}

rt_bool_t rtgui_label_event_handler(pvoid wdt, rtgui_event_t* event)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	rtgui_label_t* label = RTGUI_LABEL(wdt);

	RT_ASSERT(widget != RT_NULL);

	switch(event->type)
	{
	case RTGUI_EVENT_PAINT:
		if(widget->on_draw)
			widget->on_draw(widget, event);
		else
			rtgui_label_ondraw(label);
		break;
	default:
		break;
	}

	return RT_FALSE;
}

//parent必须是一个容器类控件
rtgui_label_t* rtgui_label_create(pvoid parent, const char* text,int left, int top, int w, int h)
{
	rtgui_container_t *container;
	rtgui_label_t* label;

	RT_ASSERT(parent != RT_NULL);
	container = RTGUI_CONTAINER(parent);

	label = rtgui_widget_create(RTGUI_LABEL_TYPE);
	if(label != RT_NULL)
	{
		rtgui_rect_t rect;
		
		rtgui_widget_get_rect(container, &rect);
		rtgui_widget_rect_to_device(container,&rect);
		rect.x1 += left;
		rect.y1 += top;
		rect.x2 = rect.x1 + w;
		rect.y2 = rect.y1 + h;
		rtgui_widget_set_rect(label, &rect);

		label->text = rt_strdup(text);

		rtgui_container_add_child(container, label);
	}

	return label;
}

void rtgui_label_destroy(rtgui_label_t* label)
{
	rtgui_widget_destroy(label);
}

char* rtgui_label_get_text(rtgui_label_t* label)
{
	if(label == RT_NULL)return RT_NULL;

	return label->text;
}

void rtgui_label_set_text(rtgui_label_t* label, const char* text)
{
	RT_ASSERT(label != RT_NULL);

	if(label->text != RT_NULL)
	{
		if (rt_strcmp(text, label->text) == 0) return;
		
		/* release old text memory */
		rt_free(label->text);
	}

	if(text != RT_NULL)
		label->text = rt_strdup(text);
	else
		label->text = RT_NULL;

	/* update widget */
	rtgui_label_ondraw(label);
}
