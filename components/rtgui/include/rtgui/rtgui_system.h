/*
 * File      : rtgui_system.h
 * This file is part of RTGUI in RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-10-04     Bernard      first version
 */
#ifndef __RTGUI_SYSTEM_H__
#define __RTGUI_SYSTEM_H__

#include <rtthread.h>
#include <rtgui/rtgui.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rtgui_timer rtgui_timer_t;
typedef void (*rtgui_timeout_func)(rtgui_timer_t* timer, void* parameter);
typedef void (*rtgui_idle_func_t)(pvoid wdt, rtgui_event_t *event);

struct rtgui_timer
{
	/* context thread id */
	rt_thread_t tid;
	/* rt timer */
	struct rt_timer timer;
	rt_bool_t active;

	/* timeout function and user data */
	rtgui_timeout_func timeout;
	void* user_data;
};


rtgui_timer_t* rtgui_timer_create(const char *name, rt_int32_t time, rt_int32_t flag, rtgui_timeout_func timeout, void* parameter);
void rtgui_timer_destory(rtgui_timer_t* timer);

void rtgui_timer_start(rtgui_timer_t* timer);
void rtgui_timer_stop(rtgui_timer_t* timer);

void rtgui_screen_lock(rt_int32_t timeout);
void rtgui_screen_unlock(void);

rt_err_t rtgui_send(struct rt_thread* tid, rtgui_event_t* event, rt_size_t event_size);
rt_err_t rtgui_send_urgent(struct rt_thread* tid, rtgui_event_t* event, rt_size_t event_size);
rt_err_t rtgui_send_sync(struct rt_thread* tid, rtgui_event_t* event, rt_size_t event_size);
rt_err_t rtgui_recv(void);
rt_err_t rtgui_recv_nosuspend(void);
rt_err_t rtgui_recv_filter(rt_uint32_t type);
rt_err_t rtgui_ack(rtgui_event_t* event, rt_err_t errcode);

/* rtgui system initialization function */
void rtgui_system_server_init(void);

void* rtgui_malloc(rt_size_t size);
void rtgui_free(void* ptr);
void* rtgui_realloc(void* ptr, rt_size_t size);

#ifdef __cplusplus
}
#endif

#endif

