/*
 * File      : event.h
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
#ifndef __RTGUI_EVENT_H__
#define __RTGUI_EVENT_H__

#include <rtgui/rtgui.h>
#include <rtgui/kbddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Use new name & namespace */
enum _rtgui_event_type
{
	/* window event */
	RTGUI_EVENT_WIN_CREATE=0,       /* create a window 		*/
	RTGUI_EVENT_WIN_DESTROY,        /* destroy a window 	*/
	RTGUI_EVENT_WIN_SHOW,           /* show a window 		*/
	RTGUI_EVENT_WIN_HIDE,           /* hide a window 		*/
	RTGUI_EVENT_WIN_ACTIVATE,       /* activate a window 	*/
	RTGUI_EVENT_WIN_DEACTIVATE,     /* deactivate a window 	*/
	RTGUI_EVENT_WIN_CLOSE,          /* close a window 		*/
	RTGUI_EVENT_WIN_MAX,            /* max a window 		*/
	RTGUI_EVENT_WIN_MIN,            /* min a window 		*/
	RTGUI_EVENT_WIN_MOVE,           /* move a window 		*/
	RTGUI_EVENT_WIN_RESIZE,         /* resize a window 		*/

	RTGUI_EVENT_UPDATE,             /* update a rect 		*/
	RTGUI_EVENT_PAINT,              /* paint on screen 		*/
	RTGUI_EVENT_TIMER,              /* timer 0x17			*/
	/* mouse and keyboard event */
	RTGUI_EVENT_MOUSE_MOTION,       /* mouse motion         */
	RTGUI_EVENT_MOUSE_BUTTON,       /* mouse button info 	*/
	RTGUI_EVENT_KBD,                /* keyboard info	    */
	/* user command event */
	RTGUI_EVENT_COMMAND,            /* user command 		*/
	/* widget event */
	RTGUI_EVENT_RESIZE,             /* widget resize 		*/
	/* custom event */
	USER_EVENT_CURRENT_POS,			/* 光栅尺当前点 */
	USER_EVENT_BELL,				/* 蜂鸣器 */
	USER_EVENT_PRINT_CURRENT,
	USER_EVENT_PRINT_GRAPH,
	USER_EVENT_SD_HOTPLUG,
	USER_EVENT_RTC,
	USER_EVENT_FINDRI,
}; 
typedef enum _rtgui_event_type rtgui_event_type;

enum {
	RTGUI_STATUS_OK = 0,		/* status ok 		*/
	RTGUI_STATUS_ERROR,			/* generic error 	*/
};

struct rtgui_event
{
	/* the event type */
	rt_uint16_t type;
	/* user field of event */
	rt_uint16_t user;

	/* the event sender */
	rt_thread_t sender;

	/* mailbox to acknowledge request */
	rt_mailbox_t ack;
};

typedef struct rtgui_event	rtgui_event_t;
#define RTGUI_EVENT(e)	((struct rtgui_event*)(e))

#define RTGUI_EVENT_INIT(e, t)	do		\
{										\
	(e)->type = (t);					\
	(e)->user = 0;						\
	(e)->sender = rt_thread_self();		\
	(e)->ack = RT_NULL;					\
} while(0)

/*
 * RTGUI Window Event
 */
struct rtgui_event_win
{
	rtgui_event_t parent;

	/* the window id */
	rtgui_win_t* wid;
};

struct rtgui_event_win_move
{
	rtgui_event_t parent;

	/* the window id */
	rtgui_win_t* wid;

	rt_int16_t x, y;
};

struct rtgui_event_win_resize
{
	rtgui_event_t parent;

	/* the window id */
	rtgui_win_t* wid;

	rtgui_rect_t rect;
};

/* window event init */
#define RTGUI_EVENT_WIN_CREATE_INIT(e)		RTGUI_EVENT_INIT(&((e)->parent), RTGUI_EVENT_WIN_CREATE)
#define RTGUI_EVENT_WIN_DESTROY_INIT(e)		RTGUI_EVENT_INIT(&((e)->parent), RTGUI_EVENT_WIN_DESTROY)
#define RTGUI_EVENT_WIN_SHOW_INIT(e)		RTGUI_EVENT_INIT(&((e)->parent), RTGUI_EVENT_WIN_SHOW)
#define RTGUI_EVENT_WIN_HIDE_INIT(e)		RTGUI_EVENT_INIT(&((e)->parent), RTGUI_EVENT_WIN_HIDE)
#define RTGUI_EVENT_WIN_ACTIVATE_INIT(e)	RTGUI_EVENT_INIT(&((e)->parent), RTGUI_EVENT_WIN_ACTIVATE)
#define RTGUI_EVENT_WIN_DEACTIVATE_INIT(e)	RTGUI_EVENT_INIT(&((e)->parent), RTGUI_EVENT_WIN_DEACTIVATE)
#define RTGUI_EVENT_WIN_CLOSE_INIT(e)		RTGUI_EVENT_INIT(&((e)->parent), RTGUI_EVENT_WIN_CLOSE)
#define RTGUI_EVENT_WIN_MAX_INIT(e)			RTGUI_EVENT_INIT(&((e)->parent), RTGUI_EVENT_WIN_MAX)
#define RTGUI_EVENT_WIN_MIN_INIT(e)			RTGUI_EVENT_INIT(&((e)->parent), RTGUI_EVENT_WIN_MIN)
#define RTGUI_EVENT_WIN_MOVE_INIT(e)		RTGUI_EVENT_INIT(&((e)->parent), RTGUI_EVENT_WIN_MOVE)
#define RTGUI_EVENT_WIN_RESIZE_INIT(e)		RTGUI_EVENT_INIT(&((e)->parent), RTGUI_EVENT_WIN_RESIZE)

/*
 * RTGUI Other Event
 */
struct rtgui_event_update
{
	rtgui_event_t parent;

	/* the update rect */
	rtgui_rect_t rect;
};

struct rtgui_event_paint
{
	rtgui_event_t parent;
	/* destination window */
	rtgui_win_t* wid;
};

struct rtgui_event_timer
{
	rtgui_event_t parent;

	struct rtgui_timer *timer;
};

#define RTGUI_EVENT_UPDATE_INIT(e)			RTGUI_EVENT_INIT(&((e)->parent), RTGUI_EVENT_UPDATE)
#define RTGUI_EVENT_PAINT_INIT(e)				RTGUI_EVENT_INIT(&((e)->parent), RTGUI_EVENT_PAINT)
#define RTGUI_EVENT_TIMER_INIT(e)				RTGUI_EVENT_INIT(&((e)->parent), RTGUI_EVENT_TIMER)

/*
 * RTGUI Mouse and Keyboard Event
 */
struct rtgui_event_mouse
{
	rtgui_event_t parent;

	rtgui_win_t* wid;		/* destination window */

	rt_int16_t x, y;
	rt_uint16_t button;
};

#define RTGUI_MOUSE_BUTTON_RIGHT		0x01
#define RTGUI_MOUSE_BUTTON_LEFT			0x02
#define RTGUI_MOUSE_BUTTON_MIDDLE		0x03
#define RTGUI_MOUSE_BUTTON_WHEELUP		0x04
#define RTGUI_MOUSE_BUTTON_WHEELDOWN	0x08

#define RTGUI_MOUSE_BUTTON_DOWN			0x10
#define RTGUI_MOUSE_BUTTON_UP			0x20

struct rtgui_event_kbd
{
	rtgui_event_t parent;

	rtgui_win_t* wid;		/* destination window */

	rt_uint16_t type;		/* key down or up */
	rt_uint16_t key;		/* current key */
	rt_uint16_t mod;		/* current key modifiers */
	rt_uint16_t unicode;	/* translated character */
};

struct user_event_current_pos
{
	rtgui_event_t parent;
	double dx,dy,dz;//usr->P.PD
};

struct user_event_bell
{
	rtgui_event_t parent;
	rt_uint16_t delay;
};

struct user_event_print_current
{
	rtgui_event_t parent;
	double x,y,z;	
};

struct user_event_print_graph
{
	rtgui_event_t parent;
	void *ug;	
	void *item;
};

#define SD_HOTPLUG_IN	0x01
#define SD_HOTPLUG_OUT	0x02
struct user_event_sd_hotplug
{
	rtgui_event_t parent;
	rt_uint16_t type;
};

struct user_event_rtc
{
	rtgui_event_t parent;
	int hour,min,sec;
};

#define FIND_RI_SUCC_X	0x01
#define FIND_RI_SUCC_Y	0x02
#define FIND_RI_SUCC_Z	0x03
struct user_event_findri
{
	rtgui_event_t parent;
	rt_uint16_t type;
};

#define RTGUI_KBD_IS_SET_CTRL(e)	((e)->mod & (RTGUI_KMOD_LCTRL | RTGUI_KMOD_RCTRL))
#define RTGUI_KBD_IS_SET_ALT(e)		((e)->mod & (RTGUI_KMOD_LALT  | RTGUI_KMOD_RALT))
#define RTGUI_KBD_IS_SET_SHIFT(e)	((e)->mod & (RTGUI_KMOD_LSHIFT| RTGUI_KMOD_RSHIFT))
#define RTGUI_KBD_IS_UP(e)			((e)->type == RTGUI_KEYUP)
#define RTGUI_KBD_IS_DOWN(e)		((e)->type == RTGUI_KEYDOWN)

#define RTGUI_EVENT_MOUSE_MOTION_INIT(e)	RTGUI_EVENT_INIT(&((e)->parent), RTGUI_EVENT_MOUSE_MOTION)
#define RTGUI_EVENT_MOUSE_BUTTON_INIT(e)	RTGUI_EVENT_INIT(&((e)->parent), RTGUI_EVENT_MOUSE_BUTTON)
#define RTGUI_EVENT_KBD_INIT(e)				RTGUI_EVENT_INIT(&((e)->parent), RTGUI_EVENT_KBD)
#define USER_EVENT_CURRENT_POS_INIT(e)		RTGUI_EVENT_INIT(&((e)->parent), USER_EVENT_CURRENT_POS)
#define USER_EVENT_BELL_INIT(e)				RTGUI_EVENT_INIT(&((e)->parent), USER_EVENT_BELL)
#define USER_EVENT_PRINT_CURRENT_INIT(e)	RTGUI_EVENT_INIT(&((e)->parent), USER_EVENT_PRINT_CURRENT)
#define USER_EVENT_PRINT_GRAPH_INIT(e)		RTGUI_EVENT_INIT(&((e)->parent), USER_EVENT_PRINT_GRAPH)
#define USER_EVENT_SD_HOTPLUG_INIT(e)		RTGUI_EVENT_INIT(&((e)->parent), USER_EVENT_SD_HOTPLUG)
#define USER_EVENT_RTC_INIT(e)				RTGUI_EVENT_INIT(&((e)->parent), USER_EVENT_RTC)
#define USER_EVENT_FINDRI_INIT(e)			RTGUI_EVENT_INIT(&((e)->parent), USER_EVENT_FINDRI)

struct rtgui_event_command
{
	rtgui_event_t parent;

	/* command type */
	rt_int32_t type;

	/* command id */
	rt_int32_t command;

	/* command string */
	char command_string[RTGUI_NAME_MAX];
};
#define RTGUI_EVENT_COMMAND_INIT(e)	RTGUI_EVENT_INIT(&((e)->parent), RTGUI_EVENT_COMMAND)

#define RTGUI_CMD_UNKNOWN		0x00
#define RTGUI_CMD_WM_CLOSE		0x10

#define RTGUI_CMD_USER_INT		0x20
#define RTGUI_CMD_USER_STRING	0x21

union rtgui_event_generic
{
	struct rtgui_event base;
	struct rtgui_event_win win_base;
	struct rtgui_event_win_move win_move;
	struct rtgui_event_win_resize win_resize;
	struct rtgui_event_update update;
	struct rtgui_event_paint paint;
	struct rtgui_event_timer timer;
	struct rtgui_event_mouse mouse;
	struct rtgui_event_kbd kbd;
	struct user_event_current_pos current_pos;
	struct user_event_bell bell;
	struct user_event_print_current print_current;
	struct user_event_print_graph print_graph;
	struct user_event_sd_hotplug sd_hotplug;
	struct user_event_rtc rtc;
	struct user_event_findri findri;
	struct rtgui_event_command command;
};

#define RTGUI_EVENT_SIZE (sizeof(union rtgui_event_generic))

#include <rtgui/rtgui_system.h>

#ifdef __cplusplus
}
#endif

#endif

