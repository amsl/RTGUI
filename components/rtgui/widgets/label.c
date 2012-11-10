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
 * 2009-10-16     Bernard      first version
 */
#include <rtgui/dc.h>
#include <rtgui/widgets/label.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/rtgui_theme.h>

static void _rtgui_label_constructor(rtgui_label_t *label)
{
    RTGUI_WIDGET_TEXTALIGN(label) = RTGUI_ALIGN_LEFT | RTGUI_ALIGN_CENTER_VERTICAL;
    /* init widget and set event handler */
    rtgui_object_set_event_handler(RTGUI_OBJECT(label), rtgui_label_event_handler);

    /* set field */
    label->text = RT_NULL;
}

static void _rtgui_label_destructor(rtgui_label_t *label)
{
    /* release text memory */
    if (label->text)
        rt_free(label->text);
    label->text = RT_NULL;
}

DEFINE_CLASS_TYPE(label, "label",
                  RTGUI_WIDGET_TYPE,
                  _rtgui_label_constructor,
                  _rtgui_label_destructor,
                  sizeof(struct rtgui_label));

rt_bool_t rtgui_label_event_handler(struct rtgui_object *object, struct rtgui_event *event)
{
    struct rtgui_label *label;
    RTGUI_WIDGET_EVENT_HANDLER_PREPARE

    label = RTGUI_LABEL(object);
    switch (event->type)
    {
    case RTGUI_EVENT_PAINT:
        rtgui_theme_draw_label(label);
        break;
    default:
        return rtgui_widget_event_handler(object, event);
    }

    return RT_FALSE;
}
RTM_EXPORT(rtgui_label_event_handler);

rtgui_label_t *rtgui_label_create(rtgui_container_t *container, const char *text, int left, int top, int w, int h)
{
    struct rtgui_label *label;
	
	RT_ASSERT(container != RT_NULL);

    label = (struct rtgui_label *) rtgui_widget_create(RTGUI_LABEL_TYPE);
    if (label != RT_NULL)
    {
        rtgui_rect_t rect;

		rtgui_widget_get_rect(RTGUI_WIDGET(container), &rect);
		rtgui_widget_rect_to_device(RTGUI_WIDGET(container), &rect);
		rect.x1 += left;
		rect.y1 += top;
		rect.x2 = rect.x1 + w;
		rect.y2 = rect.y1 + h;
        rtgui_widget_set_rect(RTGUI_WIDGET(label), &rect);
        /* set text */
        label->text = (char *)rt_strdup((const char *)text);
		rtgui_container_add_child(container, RTGUI_WIDGET(label));
    }

    return label;
}
RTM_EXPORT(rtgui_label_create);

void rtgui_label_destroy(rtgui_label_t *label)
{
    rtgui_widget_destroy(RTGUI_WIDGET(label));
}
RTM_EXPORT(rtgui_label_destroy);

char *rtgui_label_get_text(rtgui_label_t *label)
{
    RT_ASSERT(label != RT_NULL);

    return label->text;
}
RTM_EXPORT(rtgui_label_get_text);

void rtgui_label_set_text(rtgui_label_t *label, const char *text)
{
    RT_ASSERT(label != RT_NULL);

    if (label->text != RT_NULL)
    {
        /* it's a same text string */
        if (rt_strcmp(text, label->text) == 0)
            return;

        /* release old text memory */
        rt_free(label->text);
    }

    if (text != RT_NULL)
        label->text = (char *)rt_strdup((const char *)text);
    else
        label->text = RT_NULL;

    /* update widget */
    rtgui_theme_draw_label(label);
}
RTM_EXPORT(rtgui_label_set_text);

