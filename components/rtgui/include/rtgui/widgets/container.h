/*
 * File      : container.h
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
#ifndef __RTGUI_CONTAINER_H__
#define __RTGUI_CONTAINER_H__

#include <rtgui/widgets/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

DECLARE_CLASS_TYPE(container);
/** Gets the type of a container */
#define RTGUI_CONTAINER_TYPE       (RTGUI_TYPE(container))
/** Casts the object to an rtgui_container */
#define RTGUI_CONTAINER(obj)       (RTGUI_OBJECT_CAST((obj), RTGUI_CONTAINER_TYPE, rtgui_container_t))
/** Checks if the object is an rtgui_container */
#define RTGUI_IS_CONTAINER(obj)    (RTGUI_OBJECT_CHECK_TYPE((obj), RTGUI_CONTAINER_TYPE))

/*
 * the container widget
 */
struct rtgui_container
{
	struct rtgui_widget parent;

	rtgui_list_t children;
};
typedef struct rtgui_container rtgui_container_t;

rtgui_container_t* rtgui_container_create(void);
void rtgui_container_destroy(rtgui_container_t* container);

rt_bool_t rtgui_container_event_handler(void* object, struct rtgui_event* event);

#ifndef RTGUI_USING_SMALL_SIZE
struct rtgui_box;
void rtgui_container_set_box(struct rtgui_container* container, struct rtgui_box* box);
#endif

void rtgui_container_hide(rtgui_container_t* container);

void rtgui_container_add_child(void *cbox, void* wdt);
void rtgui_container_remove_child(rtgui_container_t *container, void* wdt);
void rtgui_container_destroy_children(rtgui_container_t *container);
rtgui_widget_t* rtgui_container_get_first_child(rtgui_container_t* container);

rt_bool_t rtgui_container_event_handler(void* widget, rtgui_event_t* event);

rt_bool_t rtgui_container_dispatch_event(void *wdt, rtgui_event_t* event);
rt_bool_t rtgui_container_dispatch_mouse_event(void *wdt, struct rtgui_event_mouse* event);

#ifdef __cplusplus
}
#endif

#endif
