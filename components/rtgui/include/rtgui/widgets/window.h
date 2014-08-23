/*
 * File      : window.h
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
 * 2010-05-03     Bernard      add win close function
 */
#ifndef __RTGUI_WINDOW_H__
#define __RTGUI_WINDOW_H__

#include <rtgui/widgets/container.h>

#ifdef __cplusplus
extern "C" {
#endif

DECLARE_CLASS_TYPE(win);

/** Gets the type of a win */
#define RTGUI_WIN_TYPE       (RTGUI_TYPE(win))
/** Casts the object to an rtgui_win_t */
#define RTGUI_WIN(obj)       (RTGUI_OBJECT_CAST((obj), RTGUI_WIN_TYPE, rtgui_win_t))
/** Checks if the object is an rtgui_win_t */
#define RTGUI_IS_WIN(obj)    (RTGUI_OBJECT_CHECK_TYPE((obj), RTGUI_WIN_TYPE))

#define RTGUI_WIN_CLOSEBOX_WIDTH	16
#define RTGUI_WIN_CLOSEBOX_HEIGHT  	16

#define RTGUI_WIN_TITLE_HEIGHT	 	20	/* title height */
#define RTGUI_WIN_STATUS_HEIGHT	 	20	/* status bar height */
#define RTGUI_WIN_MENU_HEIGHT		20  /* menu bar height */

/* window status */
#define RTGUI_WIN_STATUS_MODAL	 	(1ul << 0)	/* modal status */
#define RTGUI_WIN_STATUS_MIN		(1ul << 1)	/* mninmux status */
#define RTGUI_WIN_STATUS_MAX		(1ul << 2)	/* maximum status */
#define RTGUI_WIN_STATUS_CLOSED	 	(1ul << 3)	/* closed status */
#define RTGUI_WIN_STATUS_ACTIVATE	(1ul << 4)	/* activate status */
#define RTGUI_WIN_STATUS_FOCUS	 	(1ul << 5)	/* focused status */

#define RTGUI_WIN_IS_MODAL(w)		(w->status & RTGUI_WIN_STATUS_MODAL)
/* window style */
#define RTGUI_WIN_TITLE			 	(1ul << 1)	/* use title bar */
#define RTGUI_WIN_STATUS			(1ul << 2)	/* use status bar */
#define RTGUI_WIN_MENU			 	(1ul << 3)	/* use status menu bar */
#define RTGUI_WIN_BORDER			(1ul << 4)	/* use border	 */
#define RTGUI_WIN_MINBOX			(1ul << 5)	/* use minimum button */
#define RTGUI_WIN_MAXBOX			(1ul << 6)	/* use maximum button */
#define RTGUI_WIN_CLOSEBOX		 	(1ul << 7)	/* use close button	*/
#define RTGUI_WIN_MINBOX_PRESSED	(1ul << 8)	/* minimum button pressed */
#define RTGUI_WIN_MAXBOX_PRESSED	(1ul << 9)	/* maximum button pressed */
#define RTGUI_WIN_CLOSEBOX_PRESSED 	(1ul << 10) /* close button pressed */

#define RTGUI_WIN_NOBORDER	(0)
#define RTGUI_WIN_NOTITLE	(RTGUI_WIN_BORDER)
#define RTGUI_WIN_ONLYTITLE	(RTGUI_WIN_BORDER|RTGUI_WIN_TITLE)
#define RTGUI_WIN_DIALOG	(RTGUI_WIN_BORDER|RTGUI_WIN_TITLE|RTGUI_WIN_CLOSEBOX)
#define RTGUI_WIN_DEFAULT	(RTGUI_WIN_BORDER|RTGUI_WIN_TITLE|RTGUI_WIN_CLOSEBOX|RTGUI_WIN_MINBOX)
#define RTGUI_WIN_NORMAL	(RTGUI_WIN_BORDER|RTGUI_WIN_TITLE|RTGUI_WIN_CLOSEBOX|RTGUI_WIN_MINBOX|RTGUI_WIN_MAXBOX)

struct rtgui_win
{
	/* inherit from top */
	rtgui_container_t 	parent;
	char*               title;			/* window title */
	rtgui_image_t*		image;			/* title bar icon */
	pvoid               modal_widget;   /* parent top */
	rtgui_list_t        list;           /* window list */
	/* the thread id */
	rt_thread_t			tid;            /* binding thread */
	
	rt_uint32_t style;                  /* window style */
	rt_uint32_t status;                 /* window status */
	rt_uint16_t	title_height;           /* title bar height */
	rt_uint16_t	status_height;          /* status bar height */
	rt_uint16_t	menu_height;            /* menu bar height */

	/* call back */
	rt_bool_t (*on_activate)(pvoid wdt, rtgui_event_t* event);
	rt_bool_t (*on_deactivate)(pvoid wdt, rtgui_event_t* event);
	rt_bool_t (*on_close)(pvoid wdt, rtgui_event_t* event);
};
typedef struct rtgui_win rtgui_win_t;

rtgui_win_t* rtgui_win_create(pvoid parent, const char* title, rtgui_rect_t *rect, rt_uint32_t flag);
void rtgui_win_destroy(rtgui_win_t* win);
rtgui_win_t* rtgui_win_get_win_by_widget(pvoid wdt);
void rtgui_win_close(pvoid wdt, rtgui_event_t *event);
rt_bool_t rtgui_win_ondraw(rtgui_win_t* win);

void rtgui_win_show(rtgui_win_t* win, rt_bool_t modal);
void rtgui_win_hide(rtgui_win_t* win);
void rtgui_win_end_modal(rtgui_win_t* win);

rt_bool_t rtgui_win_is_activated(rtgui_win_t* win);

void rtgui_win_move(rtgui_win_t* win, int x, int y);

/* reset extent of window */
void rtgui_win_set_rect(rtgui_win_t* win, rtgui_rect_t* rect);
void rtgui_win_set_onactivate(rtgui_win_t* win, rtgui_event_handler_ptr handler);
void rtgui_win_set_ondeactivate(rtgui_win_t* win, rtgui_event_handler_ptr handler);
void rtgui_win_set_onclose(rtgui_win_t* win, rtgui_event_handler_ptr handler);

rt_bool_t rtgui_win_event_handler(pvoid wdt, rtgui_event_t* event);
void rtgui_win_event_loop(rtgui_win_t* win);

void rtgui_win_set_title(rtgui_win_t* win, const char *title);
char* rtgui_win_get_title(rtgui_win_t* win);

rtgui_point_t rtgui_win_get_client_zero(rtgui_win_t *win);
void rtgui_win_get_client_rect(rtgui_win_t *win, rtgui_rect_t *rect);
void rtgui_win_get_title_rect(rtgui_win_t *win, rtgui_rect_t *rect);
rt_uint16_t rtgui_win_get_title_height(rtgui_win_t *win);
void rtgui_win_get_closebox_rect(rtgui_win_t *win, rtgui_rect_t *rect);
void rtgui_win_get_maxbox_rect(rtgui_win_t *win, rtgui_rect_t *rect);
void rtgui_win_get_minbox_rect(rtgui_win_t *win, rtgui_rect_t *rect);

void rtgui_win_draw_closebox(rtgui_win_t *win);
void rtgui_win_draw_maxbox(rtgui_win_t *win);
void rtgui_win_draw_minbox(rtgui_win_t *win);
void rtgui_win_draw_title(rtgui_win_t *win);
void rtgui_win_draw(rtgui_win_t* win);

#ifdef __cplusplus
}
#endif

#endif
