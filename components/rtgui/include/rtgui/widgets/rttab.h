/*
 * File      : rttab.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 */
#ifndef __RTGUI_RTTAB_H__
#define __RTGUI_RTTAB_H__

#include <rtgui/widgets/view.h>	
#include <rtgui/widgets/spin.h>

#ifdef __cplusplus
extern "C" {
#endif

DECLARE_CLASS_TYPE(rttab);
/** Gets the type of a rttab */
#define RTGUI_RTTAB_TYPE       (RTGUI_TYPE(rttab))//(rtgui_rttab_type_get())
/** Casts the object to an rtgui_rttab_t */
#define RTGUI_RTTAB(obj)       (RTGUI_OBJECT_CAST((obj), RTGUI_RTTAB_TYPE, rtgui_rttab_t))
/** Checks if the object is an rtgui_rttab_t */
#define RTGUI_IS_RTTAB(obj)    (RTGUI_OBJECT_CHECK_TYPE((obj), RTGUI_RTTAB_TYPE))

enum
{
	RTGUI_RTTAB_ORIENT_TOP    = 0x01,
	RTGUI_RTTAB_ORIENT_BOTTOM = 0x02,
};

#define RTGUI_RTTAB_DEFAULT_TITLE_SIZE	22

typedef struct rtgui_rttab_item rtgui_rttab_item_t;
struct rtgui_rttab_item
{
	rtgui_view_t*  tag;
	rtgui_image_t* image;
	rt_int16_t     tag_width;
};

/* the rttab widget */
struct rtgui_rttab
{
	/* inherit from container */
	rtgui_container_t   parent;
	rt_uint8_t 	        orient;
	rt_int16_t          tag_size;
	rtgui_color_t		focus_color;
	rt_uint32_t         tag_count;
	rt_uint32_t			first_tag;
	rt_uint32_t			now_tag;
	rtgui_rttab_item_t* tags;
	rtgui_spin_t*		spin;
};
typedef struct rtgui_rttab rtgui_rttab_t;

rtgui_rttab_t* rtgui_rttab_create(pvoid parent, int left, int top, int w, int h);
void rtgui_rttab_add_tag(rtgui_rttab_t* tab, const char* name, rtgui_image_t* image);
rtgui_container_t* rtgui_rttab_get_container_by_index(rtgui_rttab_t* tab, rt_int16_t index);
rtgui_container_t* rtgui_rttab_get_container_by_title(rtgui_rttab_t* tab, char* title);
rt_bool_t rtgui_rttab_switchto_next(rtgui_rttab_t* tab);
rt_bool_t rtgui_rttab_switchto_prev(rtgui_rttab_t* tab);
rt_bool_t rtgui_rttab_event_handler(pvoid wdt, rtgui_event_t* event);

/** @} */

#ifdef __cplusplus
}
#endif

#endif
