/*
 * File      : vernier.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-07-05     iamyhw       first version
 */
#ifndef __RTGUI_VERNIER_H__
#define __RTGUI_VERNIER_H__

#include <rtgui/widgets/widget.h>
#include <rtgui/widgets/container.h>

#ifdef __cplusplus
extern "C" {
#endif

DECLARE_CLASS_TYPE(vernier);

/** Gets the type of a vernier */
#define RTGUI_VERNIER_TYPE       (RTGUI_TYPE(vernier))
/** Casts the object to an rtgui_label_t */
#define RTGUI_VERNIER(obj)       (RTGUI_OBJECT_CAST((obj), RTGUI_VERNIER_TYPE, rtgui_vernier_t))
/** Checks if the object is an rtgui_label_t */
#define RTGUI_IS_VERNIER(obj)    (RTGUI_OBJECT_CHECK_TYPE((obj), RTGUI_VERNIER_TYPE))

typedef struct rtgui_vernier rtgui_vernier_t;
/*
 * the vernier widget
 */
struct rtgui_vernier
{
	rtgui_widget_t parent;
	rt_int16_t orient,dir_motion;
	rt_int16_t nowloc,oldloc,len;
	rt_int16_t thumb_w,thumb_h;
};

rtgui_vernier_t* rtgui_vernier_create(pvoid parent, int left, int top, int w, int h, 
									rt_int16_t orient, rt_int16_t dir_motion);
void rtgui_vernier_destroy(rtgui_vernier_t* verr);
void rtgui_vernier_ondraw(rtgui_vernier_t *verr);
void rtgui_vernier_update(rtgui_vernier_t* verr);
rt_bool_t rtgui_vernier_event_handler(pvoid wdt, rtgui_event_t* event);

#ifdef __cplusplus
}
#endif

#endif
