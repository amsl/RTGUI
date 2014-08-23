/*
 * File      : terminal.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2011, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-05-10	  amsl
 */
#ifndef __RTGUI_TEXTVIEW_H__
#define __RTGUI_TEXTVIEW_H__

#include <rtgui/widgets/widget.h>
#include <rtgui/widgets/container.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup rtgui_terminal
 * @{
 */

DECLARE_CLASS_TYPE(terminal);

/** Gets the type of a terminal */
#define RTGUI_TERMINAL_TYPE       (RTGUI_TYPE(terminal))//(rtgui_terminal_type_get())
/** Casts the object to a rtgui_terminal_t */
#define RTGUI_TERMINAL(obj)       (RTGUI_OBJECT_CAST((obj), RTGUI_TERMINAL_TYPE, rtgui_terminal_t))
/** Checks if the object is a rtgui_terminal_t */
#define RTGUI_IS_TERMINAL(obj)    (RTGUI_OBJECT_CHECK_TYPE((obj), RTGUI_TERMINAL_TYPE))

typedef struct rtgui_terminal rtgui_terminal_t;
/*
 * the terminal widget
 */
struct rtgui_terminal
{
	/* inherit from widget */
	rtgui_widget_t parent;

	rt_int16_t item_width;
	rt_int16_t item_per_page;
	rt_int16_t item_count;
	rt_int16_t first_item;
	rt_int16_t now_item;
	rt_int16_t old_item;

	char* lines;
};

rtgui_terminal_t* rtgui_terminal_create(pvoid parent,const char* text, int left,int top,int w,int h);
void rtgui_terminal_destroy(rtgui_terminal_t* tma);

rt_bool_t rtgui_terminal_event_handler(pvoid wdt, rtgui_event_t* event);
void rtgui_terminal_set_text(rtgui_terminal_t* tma, const char* text);

/** @} */

#ifdef __cplusplus
}
#endif

#endif
