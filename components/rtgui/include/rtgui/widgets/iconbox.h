/*
 * File      : iconbox.h
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
#ifndef __RTGUI_ICONBOX_H__
#define __RTGUI_ICONBOX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <rtgui/widgets/widget.h>
#include <rtgui/widgets/container.h>

DECLARE_CLASS_TYPE(iconbox);
/** Gets the type of a iconbox */
#define RTGUI_ICONBOX_TYPE       (RTGUI_TYPE(iconbox))
/** Casts the object to a rtgui_iconbox_t */
#define RTGUI_ICONBOX(obj)       (RTGUI_OBJECT_CAST((obj), RTGUI_ICONBOX_TYPE, rtgui_iconbox_t))
/** Checks if the object is a rtgui_iconbox_t */
#define RTGUI_IS_ICONBOX(obj)    (RTGUI_OBJECT_CHECK_TYPE((obj), RTGUI_ICONBOX_TYPE))

#define RTGUI_ICONBOX_NOTEXT		0x00
#define RTGUI_ICONBOX_TEXT_RIGHT	0x01
#define RTGUI_ICONBOX_TEXT_BELOW	0x02

#define RTGUI_ICONBOX_AREA		76

struct rtgui_iconbox
{
	rtgui_widget_t parent;	  /* inherit from widget */

	rtgui_image_t* image;	  /* widget private data */
	char *text;
	void (*func)(void);
	rt_uint32_t text_position;

	rt_bool_t selected;
};
typedef struct rtgui_iconbox rtgui_iconbox_t;

rtgui_iconbox_t* rtgui_iconbox_create(pvoid parent, rtgui_image_t* image, const char* text, int position);
void rtgui_iconbox_destroy(rtgui_iconbox_t* iconbox);

rt_bool_t rtgui_iconbox_event_handler(pvoid wdt, rtgui_event_t* event);

void rtgui_iconbox_set_text_position(rtgui_iconbox_t* iconbox, int position);

#ifdef __cplusplus
}
#endif

#endif
