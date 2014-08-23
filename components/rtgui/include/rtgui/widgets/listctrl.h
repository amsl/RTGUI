/*
 * File      : listctrl.h
 * This file is part of RTGUI in RT-Thread RTOS
 * COPYRIGHT (C) 2010, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2010-12-06     Bernard      first version
 */

#ifndef __RTGUI_LISTCTRL_H__
#define __RTGUI_LISTCTRL_H__

#include <rtgui/widgets/widget.h>
#include <rtgui/widgets/container.h>

/** Gets the type of a listctrl */
#define RTGUI_LISTCTRL_TYPE		(rtgui_listctrl_type_get())
/** Casts the object to a listctrl */
#define RTGUI_LISTCTRL(obj)		(RTGUI_OBJECT_CAST((obj), RTGUI_LISTCTRL_TYPE, rtgui_listctrl_t))
/** Checks if the object is a listctrl */
#define RTGUI_IS_LISTCTRL(obj)	(RTGUI_OBJECT_CHECK_TYPE((obj), RTGUI_LISTCTRL_TYPE))

typedef struct rtgui_listctrl rtgui_listctrl_t;
struct rtgui_listctrl
{
	rtgui_widget_t parent;

	/* widget private data */
	/* listctrl items */
    rt_uint32_t items;

	/* total number of items */
	rt_uint16_t items_count;
    /* the number of item in a page */
    rt_uint16_t page_items;
    /* current item */
    rt_int16_t current_item;

	/* item event handler */
	void (*on_item)(rtgui_widget_t *widget, rtgui_event_t* event);
	void (*on_item_draw)(rtgui_listctrl_t *list, rtgui_dc_t* dc, rtgui_rect_t* rect, rt_uint16_t index);
};

typedef void (*rtgui_onitem_func_t)(rtgui_widget_t* widget, rtgui_event_t *event);
typedef void (*rtgui_onitem_draw_t)(rtgui_listctrl_t *list, rtgui_dc_t* dc, rtgui_rect_t* rect, rt_uint16_t index);

rtgui_type_t *rtgui_listctrl_type_get(void);

rtgui_listctrl_t* rtgui_listctrl_create(pvoid parent, rt_uint32_t items, rt_uint16_t count,
    int left, int top, int w, int h, rtgui_onitem_draw_t ondraw);
void rtgui_listctrl_destroy(rtgui_listctrl_t* ctrl);
void rtgui_draw_selected(rtgui_dc_t *dc, rtgui_rect_t *rect);
rt_bool_t rtgui_listctrl_event_handler(pvoid wdt, rtgui_event_t* event);
void rtgui_listctrl_set_onitem(rtgui_listctrl_t* ctrl, rtgui_onitem_func_t func);
void rtgui_listctrl_set_items(rtgui_listctrl_t* ctrl, rt_uint32_t items, rt_uint16_t count);
rt_bool_t rtgui_listctrl_get_item_rect(rtgui_listctrl_t* ctrl, rt_uint16_t item, rtgui_rect_t* item_rect);

#endif
