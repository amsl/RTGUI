/*
 * File      : server.c
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

#include <rtgui/rtgui.h>
#include <topwin.h>
#include <rtgui/event.h>
#include <rtgui/rtgui_app.h>
#include <rtgui/widgets/panel.h>
#include <mouse.h>
#include <rtgui/widgets/widget.h>
#include <rtgui/widgets/window.h>

static rt_thread_t rtgui_server_tid;

extern rtgui_win_t* rtgui_server_focus_win;

void rtgui_server_handle_update(struct rtgui_event_update* event)
{
	if(hw_driver != RT_NULL)
	{
		rtgui_gdev_update(hw_driver, &(event->rect));
	}
}

//鼠标点击事件
void rtgui_server_handle_mouse_btn(struct rtgui_event_mouse* event)
{
	rt_thread_t thread;
	rtgui_win_t* win;
	rtgui_panel_t* panel=rtgui_panel_get();

	/* re-init to server thread */
	RTGUI_EVENT_MOUSE_BUTTON_INIT(event);

#ifdef RTGUI_USING_WINMOVE
	if(rtgui_winrect_is_moved())
	{
		if(event->button & (RTGUI_MOUSE_BUTTON_LEFT|RTGUI_MOUSE_BUTTON_UP))
		{
			/* update window location */
			rtgui_win_t *wid;
			rtgui_rect_t rect;
			
			if (rtgui_winrect_moved_done(&rect, &wid) == RT_TRUE)
			{
				struct rtgui_event_win_move ewin;

				/* move window */
				RTGUI_EVENT_WIN_MOVE_INIT(&ewin);
				ewin.wid = wid;
				ewin.x = rect.x1;
				ewin.y = rect.y1;
				
				/* send to client thread */
				rtgui_send(wid->tid, &(ewin.parent), sizeof(ewin));
				return;
			}
		}
	}
#endif

	if(RTGUI_PANEL_IS_MODAL_MODE(panel))
	{
		if(panel->modal_widget != RT_NULL && RTGUI_IS_WIN(panel->modal_widget))
		{
			win = RTGUI_WIN(panel->modal_widget);

			if(rtgui_rect_contains_point(&(RTGUI_WIDGET_EXTENT(win)), event->x, event->y) != RT_EOK)
			{
				win->status &= ~RTGUI_WIN_STATUS_ACTIVATE;
				rtgui_win_draw_title(win);
				win->status |= RTGUI_WIN_STATUS_ACTIVATE;
				rtgui_win_draw_title(win);
				win->status &= ~RTGUI_WIN_STATUS_ACTIVATE;
				rtgui_win_draw_title(win);
				win->status |= RTGUI_WIN_STATUS_ACTIVATE;
				rtgui_win_draw_title(win);
				return;
			}
		}
	}
	/* the point in a win? */
	win = rtgui_topwin_get_wnd(event->x, event->y);
	if(win != RT_NULL)
	{
		rtgui_rect_t rect;

		event->wid = win;
		if(rtgui_server_focus_win != win)
		{
			/* no focused window, raise it. */
			rtgui_topwin_raise(win);
		}

		rtgui_win_get_title_rect(win, &rect);
		rtgui_widget_rect_to_device(win, &rect);
		if(rtgui_rect_contains_point(&rect, event->x, event->y) == RT_EOK)
		{
			/* in title bar */
			rtgui_topwin_title_onmouse(win, event);
		}
		else
		{
			/* in client area */
			/* send mouse event to thread */
			rtgui_send(win->tid, (rtgui_event_t*)event, sizeof(struct rtgui_event_mouse));
		}
		return;
	}

	/* send event to panel */
	/* deactivate old window */
	if(rtgui_server_focus_win != RT_NULL)
	{
		rtgui_topwin_deactivate(rtgui_server_focus_win);
	}
	rtgui_server_focus_win = RT_NULL;
	event->wid = RT_NULL;

	thread = rtgui_panel_thread_get();
	rtgui_send(thread,(rtgui_event_t*)event,sizeof(struct rtgui_event_mouse));
}

//鼠标手势事件
void rtgui_server_handle_mouse_motion(struct rtgui_event_mouse* event)
{
	rt_thread_t thread;
	rtgui_win_t* win=RT_NULL;

	RTGUI_EVENT_MOUSE_MOTION_INIT(event);

	win = rtgui_topwin_get_wnd(event->x, event->y);
	if(win != RT_NULL)
	{
#ifdef RTGUI_USING_WINMOVE
		if(!rtgui_winrect_is_moved())
#endif
		{
			//如果没有窗口在移动
			event->wid = win;
			/* send mouse event to thread */
			rtgui_send(win->tid, (rtgui_event_t*)event, sizeof(struct rtgui_event_mouse));
		}
	}
	else
	{
#ifdef RTGUI_USING_WINMOVE
		if(!rtgui_winrect_is_moved())
#endif
		{
			/* send event to panel */
			thread = rtgui_panel_thread_get();
			rtgui_send(thread,(rtgui_event_t*)event,sizeof(struct rtgui_event_mouse));
		}
	}

	/* move mouse to (x, y) */
	rtgui_mouse_moveto(event->x, event->y);
}

void rtgui_server_handle_kbd(struct rtgui_event_kbd* event)
{
	rtgui_win_t *win;
	rt_thread_t thread;

	/* re-init to server thread */
	RTGUI_EVENT_KBD_INIT(event);

	win = rtgui_server_focus_win;
	if(win != RT_NULL && win->status & RTGUI_WIN_STATUS_ACTIVATE)
	{
		event->wid = win;
		rtgui_send(win->tid, (rtgui_event_t*)event, sizeof(struct rtgui_event_kbd));
	}
	else
	{
		/* send event to panel */
		event->wid = RT_NULL;
		thread = rtgui_panel_thread_get();
		rtgui_send(thread,(rtgui_event_t*)event,sizeof(struct rtgui_event_kbd));
	}
}

/**
 * rtgui server thread's entry
 */
static void rtgui_server_entry(void* parameter)
{
	rtgui_app_t* app;
	rtgui_event_t* event;
	rt_err_t result;
	
	app = rtgui_app_create();
	event = (rtgui_event_t*)app->event_buffer;
	
	/* init mouse and show */
	rtgui_mouse_init();
#ifdef RTGUI_USING_MOUSE_CURSOR
	rtgui_mouse_show_cursor();
#endif

	while(1)
	{
		/* the buffer uses to receive event */
		result = rtgui_recv();
		if(result == RT_EOK)
		{
			/* dispatch event */
			switch(event->type)
			{
			case RTGUI_EVENT_WIN_CREATE:
				if(rtgui_topwin_add(((struct rtgui_event_win*)event)->wid) == RT_EOK)
				{
					rtgui_ack(event, RTGUI_STATUS_OK);
				}
				else
				{
					rtgui_ack(event, RTGUI_STATUS_ERROR);
				}
				break;

			case RTGUI_EVENT_WIN_DESTROY:
				if(rtgui_topwin_remove(((struct rtgui_event_win*)event)->wid) == RT_EOK)
					rtgui_ack(event, RTGUI_STATUS_OK);
				else
					rtgui_ack(event, RTGUI_STATUS_ERROR);
				break;

			case RTGUI_EVENT_WIN_SHOW:
				if(rtgui_topwin_show(((struct rtgui_event_win*)event)->wid) == RT_EOK)
					rtgui_ack(event, RTGUI_STATUS_OK);
				else
					rtgui_ack(event, RTGUI_STATUS_ERROR);
				break;

			case RTGUI_EVENT_WIN_HIDE:
				if(rtgui_topwin_hide(((struct rtgui_event_win*)event)->wid) == RT_EOK)
					rtgui_ack(event, RTGUI_STATUS_OK);
				else
					rtgui_ack(event, RTGUI_STATUS_ERROR);
				break;

			case RTGUI_EVENT_WIN_MOVE:
				if(rtgui_topwin_move(((struct rtgui_event_win_move*)event)->wid,
				                     ((struct rtgui_event_win_move*)event)->x,
				                     ((struct rtgui_event_win_move*)event)->y))
					rtgui_ack(event, RTGUI_STATUS_OK);
				else
					rtgui_ack(event, RTGUI_STATUS_ERROR);
				break;

			case RTGUI_EVENT_WIN_RESIZE:
				rtgui_topwin_resize(((struct rtgui_event_win_resize*)event)->wid,
				                    &(((struct rtgui_event_win_resize*)event)->rect));
				break;

			case RTGUI_EVENT_UPDATE:
				/* handle screen update */
				rtgui_server_handle_update((struct rtgui_event_update*)event);
#ifdef RTGUI_USING_MOUSE_CURSOR
				/* show cursor */
				rtgui_mouse_show_cursor();
#endif
				break;

			case RTGUI_EVENT_MOUSE_MOTION:
				rtgui_server_handle_mouse_motion((struct rtgui_event_mouse*)event);
				break;

			case RTGUI_EVENT_MOUSE_BUTTON:
				rtgui_server_handle_mouse_btn((struct rtgui_event_mouse*)event);
				break;

			case RTGUI_EVENT_KBD:
				rtgui_server_handle_kbd((struct rtgui_event_kbd*)event);
				break;

			case RTGUI_EVENT_COMMAND:
				break;
			}
		}
	}
}

void rtgui_server_post_event(rtgui_event_t* event, rt_size_t size)
{
	rt_thread_t thread = rtgui_get_server();
	rtgui_app_t *app = (rtgui_app_t*)thread->user_data;
	
	if(app == RT_NULL) return;
	
	rt_mq_send(app->mq, event, size);
}

void rtgui_server_init(void)
{
	rtgui_server_tid = rt_thread_create("rtgui", rtgui_server_entry, RT_NULL, 8192, 2, 5);

	/* start rtgui server thread */
	if(rtgui_server_tid != RT_NULL)
		rt_thread_startup(rtgui_server_tid);
}

rt_thread_t rtgui_get_server(void)
{
	return rtgui_server_tid;
}
