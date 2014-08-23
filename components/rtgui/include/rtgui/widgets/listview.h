/*
 * File      : listview.h
 * This file is part of RTGUI in RT-Thread RTOS
 * COPYRIGHT (C) 2010, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2010-01-06     Bernard      first version
 */

#ifndef __RTGUI_LISTVIEW_H__
#define __RTGUI_LISTVIEW_H__

#include <rtgui/widgets/view.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*item_action)(void* parameter);
typedef struct rtgui_listview_item	rtgui_listview_item_t;
struct rtgui_listview_item
{
    char* name;
	rtgui_image_t *image;
    item_action action;
    void *parameter;
};

DECLARE_CLASS_TYPE(listview);

/** Gets the type of a listview */
#define RTGUI_LISTVIEW_TYPE       (RTGUI_TYPE(listview))
/** Casts the object to a rtgui_listview_t */
#define RTGUI_LISTVIEW(obj)       (RTGUI_OBJECT_CAST((obj), RTGUI_LISTVIEW_TYPE, rtgui_listview_t))
/** Checks if the object is a rtgui_listview_t */
#define RTGUI_IS_LISTVIEW(obj)    (RTGUI_OBJECT_CHECK_TYPE((obj), RTGUI_LISTVIEW_TYPE))

#define RTGUI_LISTVIEW_LIST		0x00
#define RTGUI_LISTVIEW_ICON		0x01
#define RTGUI_LISTVIEW_REPORT	0x02

struct rtgui_listview
{
	rtgui_view_t parent;

	/* widget private data */
	/* list item */
    const rtgui_listview_item_t* items;

	/* layout flag */
	rt_uint16_t flag;

	/* total number of items */
	rt_uint16_t item_count;
    /* the number of item in a page */
    rt_uint16_t page;
	/* current item */
    rt_int16_t item_current;

	/* icon layout */
	rt_uint32_t row_items, col_items;
};
typedef struct rtgui_listview rtgui_listview_t;

rtgui_listview_t* rtgui_listview_create(pvoid parent,const rtgui_listview_item_t* items, rt_uint16_t count, int left,int top,int w,int h, rt_uint16_t flag);
void rtgui_listview_destroy(rtgui_listview_t* view);

rt_bool_t rtgui_listview_event_handler(pvoid wdt, rtgui_event_t* event);

#ifdef __cplusplus
}
#endif

#endif

