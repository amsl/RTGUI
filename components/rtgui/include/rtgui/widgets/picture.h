/*
 * File      : picture.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-10-11     amsl         first version
 */
#ifndef __RTGUI_PICTURE_H__
#define __RTGUI_PICTURE_H__

#include <rtgui/widgets/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

DECLARE_CLASS_TYPE(picture);

/** Gets the type of a picture */
#define RTGUI_PICTURE_TYPE       (RTGUI_TYPE(picture))
/** Casts the object to an rtgui_button_t */
#define RTGUI_PICTURE(obj)       (RTGUI_OBJECT_CAST((obj), RTGUI_PICTURE_TYPE, rtgui_picture_t))
/** Checks if the object is an rtgui_button_t */
#define RTGUI_IS_PICTURE(obj)    (RTGUI_OBJECT_CHECK_TYPE((obj), RTGUI_PICTURE_TYPE))

/*
 * the picture widget
 */
struct rtgui_picture
{
	/* inherit from widget */
	rtgui_widget_t parent;
	/* image */
	rtgui_image_t *image;
};
typedef struct rtgui_picture rtgui_picture_t;


rtgui_picture_t* rtgui_picture_create(pvoid parent, int left,int top,int w,int h);
void rtgui_picture_destroy(rtgui_picture_t* pic);
void rtgui_picture_ondraw(rtgui_picture_t* pic);
void rtgui_picture_set_image(rtgui_picture_t* pic, rtgui_image_t* image);
rt_bool_t rtgui_picture_event_handler(pvoid wdt, rtgui_event_t* event);

/** @} */
#ifdef __cplusplus
}
#endif

#endif

