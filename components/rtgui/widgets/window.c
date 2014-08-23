/*
 * File      : window.c
 * This file is part of RTGUI in RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 *
 */
#include <rtgui/rtgui_app.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/widgets/window.h>
#include <rtgui/widgets/panel.h>
#include <rtgui/widgets/button.h>

static void _rtgui_win_constructor(rtgui_win_t *win)
{
	/* init window attribute */
	win->title			= RT_NULL;
	win->image			= RT_NULL;
	win->on_activate	= RT_NULL;
	win->on_deactivate  = RT_NULL;
	win->on_close		= RT_NULL;

	win->modal_widget	= RT_NULL;
	win->tid			= RT_NULL;

	/* set window hide */
	RTGUI_WIDGET_HIDE(win);

	RTGUI_WIDGET_FLAG(win) |= RTGUI_WIDGET_FLAG_FOCUSABLE;
	RTGUI_WIDGET_BC(win) = theme.background;

	/* set window status and style */
	win->status  = 0;
	win->style = RTGUI_WIN_DEFAULT;
	rtgui_widget_set_event_handler(win, rtgui_win_event_handler);
}

static void _rtgui_win_destructor(rtgui_win_t* win)
{
	struct rtgui_event_win ewin;

	/* remove win from parent container */
	if(RTGUI_WIDGET_PARENT(win) != RT_NULL)
	{
		rtgui_container_t *box;
		
		box = RTGUI_CONTAINER(RTGUI_WIDGET_PARENT(win));
		rtgui_container_remove_child(box, win);
	}
	
	/* destroy in server */
	RTGUI_EVENT_WIN_DESTROY_INIT(&ewin);
	ewin.wid = win;
	if(rtgui_send_sync(rtgui_get_server(), RTGUI_EVENT(&ewin),sizeof(struct rtgui_event_win)) != RT_EOK)
	{
		return;	 /* destroy in server failed */
	}

	/* release field */
	rt_free(win->title);
	rt_free(win->image);
}

DEFINE_CLASS_TYPE(win, "win",
                  RTGUI_CONTAINER_TYPE,
                  _rtgui_win_constructor,
                  _rtgui_win_destructor,
                  sizeof(struct rtgui_win));

rtgui_win_t* rtgui_win_create(pvoid parent, const char* title, rtgui_rect_t *rect, rt_uint32_t style)
{
	struct rtgui_event_win ewin;
	rtgui_container_t *container=RT_NULL;
	rtgui_win_t* win;
	/* get GUI server thread */
	struct rt_thread* server = rtgui_get_server();
	rtgui_app_t* app;

	if(parent == RT_NULL) container = RTGUI_CONTAINER(rtgui_panel_get());
	else container = RTGUI_CONTAINER(parent);

	if(server == RT_NULL) return RT_NULL;
	win = rtgui_widget_create(RTGUI_WIN_TYPE);

	if(win != RT_NULL)
	{
		/* set window title */
		if(title != RT_NULL)
			win->title = rt_strdup(title);
		else
			win->title = RT_NULL;

		win->style = style;

		/* set default size */
		win->title_height = RTGUI_WIN_TITLE_HEIGHT;
		win->status_height= RTGUI_WIN_STATUS_HEIGHT;
		win->menu_height  = RTGUI_WIN_MENU_HEIGHT;

		RTGUI_WIDGET(win)->toplevel = RTGUI_WIDGET(win); /* window is a toplevel widget */
		/* set extent of win */
		rtgui_widget_set_rect(win, rect);
		rtgui_container_add_child(container, win);

		app = rtgui_app_self();
		win->tid = app->tid;/* use current thread */
		rtgui_app_set_widget(win);

		/* send a create window event. */
		RTGUI_EVENT_WIN_CREATE_INIT(&ewin);
		ewin.wid = win;

		if(rtgui_send_sync(server, RTGUI_EVENT(&ewin),sizeof(struct rtgui_event_win)) != RT_EOK)
		{
			rt_kprintf("create win: %s failed\n", win->title);
			return RT_NULL;
		}
	}

	return win;
}

void rtgui_win_destroy(rtgui_win_t* win)
{
	if(RTGUI_WIN_IS_MODAL(win))
	{
		/* end modal */
		rtgui_win_end_modal(win);
	}

	rtgui_widget_destroy(win);
}

/* get widget's container window */
rtgui_win_t* rtgui_win_get_win_by_widget(pvoid wdt)
{
	rtgui_widget_t *parent=RTGUI_WIDGET(wdt);

	if(RTGUI_IS_WIN(parent))
		return (rtgui_win_t*)parent;

	while(parent->parent != RT_NULL)
	{
		if(RTGUI_IS_WIN(parent))break;
		parent = parent->parent;
	}
	if(RTGUI_IS_WIN(parent))
		return (rtgui_win_t*)parent;

	return RT_NULL;
}

/* window close event handler */
void rtgui_win_close(pvoid wdt, rtgui_event_t *event)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	rtgui_win_t* win = RT_NULL;

	if(wdt==RT_NULL)return;
	win = rtgui_win_get_win_by_widget(widget);

	if(win != RT_NULL)
	{
		rtgui_panel_t *panel=rtgui_panel_get();
		/* set window status is closed. */
		win->status |= RTGUI_WIN_STATUS_CLOSED;
		RTGUI_CONTAINER(panel)->focused	= RT_NULL;

		rtgui_win_destroy(win);
	}
}

void rtgui_win_show(rtgui_win_t* win, rt_bool_t modal)
{

	RT_ASSERT(win != RT_NULL);

	if(RTGUI_WIDGET_IS_HIDE(win))
	{
		/* send show message to server */
		struct rtgui_event_win ewin;
		RTGUI_EVENT_WIN_SHOW_INIT(&ewin);
		ewin.wid = win;

		if(rtgui_send_sync(rtgui_get_server(), RTGUI_EVENT(&ewin),sizeof(struct rtgui_event_win)) != RT_EOK)
		{
			return;
		}
	}
	else rtgui_widget_update(win);

	if(modal == RT_TRUE)
	{
		/* display by modal. */
		rtgui_widget_t *parent_widget = RTGUI_WIDGET_PARENT(win);

		if(parent_widget != RT_NULL)
		{
			/* set style */
			win->status |= RTGUI_WIN_STATUS_MODAL;

			if(RTGUI_IS_PANEL(parent_widget))
			{
				rtgui_panel_t* panel;
				panel = RTGUI_PANEL(parent_widget);
				panel->status |= RTGUI_PANEL_STATUS_MODAL;
				panel->modal_widget = win;
			}
			else if(RTGUI_IS_WIN(parent_widget))
			{
				rtgui_win_t* pwin;
				pwin = RTGUI_WIN(parent_widget);
				pwin->status |= RTGUI_WIN_STATUS_MODAL;
				pwin->modal_widget = win;
			}
		}
		else
		{
			/* which is a top window */
			win->status |= RTGUI_WIN_STATUS_MODAL;
			rtgui_win_event_loop(win);

			win->status &= ~RTGUI_WIN_STATUS_MODAL;
		}
	}
}

void rtgui_win_end_modal(rtgui_win_t* win)
{
	rtgui_widget_t* parent_widget;

	RT_ASSERT(win != RT_NULL);

	parent_widget = RTGUI_WIDGET_PARENT(win);

	if(parent_widget != RT_NULL)
	{
		if(RTGUI_IS_PANEL(parent_widget))
		{
			rtgui_panel_t* panel;

			/* which is shown under workbench */
			panel = RTGUI_PANEL(parent_widget);
			panel->status &= ~RTGUI_PANEL_STATUS_MODAL;
			panel->modal_widget = RT_NULL;
		}
		else if(RTGUI_IS_WIN(parent_widget))
		{
			rtgui_win_t* pwin;

			/* which is shown under win */
			pwin = RTGUI_WIN(parent_widget);
			pwin->status &= ~RTGUI_WIN_STATUS_MODAL;
			pwin->modal_widget = RT_NULL;
		}
	}
	else
	{
		/* which is a stand alone window */
	}

	/* remove modal mode */
	win->status &= ~RTGUI_WIN_STATUS_MODAL;
}

void rtgui_win_hide(rtgui_win_t* win)
{
	RT_ASSERT(win != RT_NULL);

	if(!RTGUI_WIDGET_IS_HIDE(win) && rtgui_get_server() != RT_NULL)
	{
		/* send hidden message to server */
		struct rtgui_event_win ewin;
		RTGUI_EVENT_WIN_HIDE_INIT(&ewin);
		ewin.wid = win;

		if(rtgui_send_sync(rtgui_get_server(), RTGUI_EVENT(&ewin),sizeof(struct rtgui_event_win)) != RT_EOK)
		{
			rt_kprintf("hide win: %s failed\n", win->title);
			return;
		}

		/* set window hide and deactivated */
		RTGUI_WIDGET_HIDE(win);
		win->status &= ~RTGUI_WIN_STATUS_ACTIVATE;
	}
}

rt_bool_t rtgui_win_is_activated(rtgui_win_t* win)
{
	RT_ASSERT(win != RT_NULL);

	return win->status & RTGUI_WIN_STATUS_ACTIVATE;
}

void rtgui_win_move(rtgui_win_t* win, int x, int y)
{
	rt_thread_t thread;
	struct rtgui_event_win_move emove;
	RTGUI_EVENT_WIN_MOVE_INIT(&emove);

	if(win == RT_NULL) return;

	emove.wid 	= win;
	emove.x		= x;
	emove.y		= y;

	thread = rtgui_get_server();
	if(thread == RT_NULL) return;

	if(rtgui_send_sync(rtgui_get_server(), RTGUI_EVENT(&emove),
	                          sizeof(struct rtgui_event_win_move)) != RT_EOK)
	{
		return;
	}
}

rt_bool_t rtgui_win_ondraw(rtgui_win_t* win)
{
	struct rtgui_event_paint event;

	rtgui_win_draw(win);

	/* paint each widget */
	RTGUI_EVENT_PAINT_INIT(&event);
	event.wid = RT_NULL;
	rtgui_container_dispatch_event(win, (rtgui_event_t*)&event);

	return RT_FALSE;
}

rt_bool_t rtgui_win_event_handler(pvoid wdt, rtgui_event_t* event)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	rtgui_win_t* win = RTGUI_WIN(wdt);

	RT_ASSERT((win != RT_NULL) && (event != RT_NULL));

	switch(event->type)
	{
	case RTGUI_EVENT_WIN_CLOSE:
		if(RTGUI_WIN_IS_MODAL(win))
		{
			rtgui_win_destroy(win);
		}
		else
		{
			rtgui_win_close(win,RT_NULL);
		}

		if(win->on_close != RT_NULL)
		{
			if(win->on_close(widget, event) == RT_FALSE) return RT_TRUE;
		}
		/* exit event loop */
		return RT_TRUE;

	case RTGUI_EVENT_WIN_MAX:
		/* TODO: Place code here. */
		return RT_TRUE;

	case RTGUI_EVENT_WIN_MIN:
		/* TODO: Place code here. */
		return RT_TRUE;

	case RTGUI_EVENT_WIN_MOVE:
	{
		struct rtgui_event_win_move* emove = (struct rtgui_event_win_move*)event;
		/* move window */
		rtgui_win_move(win, emove->x, emove->y);
		return RT_TRUE;
	}

	case RTGUI_EVENT_WIN_ACTIVATE:
		return RT_TRUE;

	case RTGUI_EVENT_WIN_DEACTIVATE:
		return RT_TRUE;
	case RTGUI_EVENT_PAINT:
		if(widget->on_draw != RT_NULL)
			widget->on_draw(widget, event);
		else
			rtgui_win_ondraw(win);
		return RT_FALSE;

	case RTGUI_EVENT_MOUSE_BUTTON:
		if(RTGUI_WIN_IS_MODAL(win))
		{
			if(win->modal_widget != RT_NULL && RTGUI_WIDGET_EVENT_HANDLE(win->modal_widget)!=RT_NULL)
				return RTGUI_WIDGET_EVENT_CALL(win->modal_widget, event);
		}

		if(rtgui_container_dispatch_mouse_event(win,(struct rtgui_event_mouse*)event) == RT_FALSE)
		{
			/* dispatch event */
			if(widget->on_mouseclick != RT_NULL)
			{
				return widget->on_mouseclick(widget, event);
			}
		}
		return RT_TRUE;

	case RTGUI_EVENT_MOUSE_MOTION:
		if(RTGUI_WIN_IS_MODAL(win))
		{
			if(win->modal_widget != RT_NULL && RTGUI_WIDGET_EVENT_HANDLE(win->modal_widget) != RT_NULL)
				return RTGUI_WIDGET_EVENT_CALL(win->modal_widget, event);
		}
		rtgui_container_dispatch_mouse_event(win,(struct rtgui_event_mouse*)event);
		return RT_TRUE;

	case RTGUI_EVENT_KBD:
		if(RTGUI_WIN_IS_MODAL(win))
		{
			if(win->modal_widget != RT_NULL && RTGUI_WIDGET_EVENT_HANDLE(win->modal_widget) != RT_NULL)
				return RTGUI_WIDGET_EVENT_CALL(win->modal_widget, event);
		}

		if(RTGUI_CONTAINER(win)->focused != widget)
		{
			if(RTGUI_WIDGET_EVENT_HANDLE(RTGUI_CONTAINER(win)->focused) != RT_NULL)
				RTGUI_WIDGET_EVENT_CALL(RTGUI_CONTAINER(win)->focused, event);
		}
		return RT_TRUE;

	default:
		return rtgui_container_event_handler(wdt,event);
	}
}

/* windows event loop */
void rtgui_win_event_loop(rtgui_win_t* win)
{
	rt_err_t result;
	rtgui_app_t* app;
	rtgui_event_t* event;

	app = rtgui_app_self();
	RT_ASSERT(app != RT_NULL);
	event = (rtgui_event_t*)app->event_buffer;

	if(RTGUI_WIN_IS_MODAL(win))
	{
		while(RTGUI_WIN_IS_MODAL(win))
		{
			if(app->on_idle != RT_NULL)
			{
				result = rtgui_recv_nosuspend();
				if(result == RT_EOK)
				{
					/* perform event handler */
					if(RTGUI_WIDGET_EVENT_HANDLE(win) != RT_NULL)
						RTGUI_WIDGET_EVENT_CALL(win, event);
				}
				else if(result == -RT_ETIMEOUT)
				{
					app->on_idle(win, event);
				}
			}
			else
			{
				result = rtgui_recv();
				if(result == RT_EOK)
				{
					/* perform event handler */
					if(RTGUI_WIDGET_EVENT_HANDLE(win) != RT_NULL)
						RTGUI_WIDGET_EVENT_CALL(win, event);
				}
			}
		}
	}
	else
	{
		while(!(win->status & RTGUI_WIN_STATUS_CLOSED))
		{
			if(app->on_idle != RT_NULL)
			{
				result = rtgui_recv_nosuspend();
				if(result == RT_EOK)
				{
					/* perform event handler */
					if(RTGUI_WIDGET_EVENT_HANDLE(win) != RT_NULL)
						RTGUI_WIDGET_EVENT_CALL(win, event);
				}
				else if(result == -RT_ETIMEOUT)
				{
					app->on_idle(win, event);
				}
			}
			else
			{
				result = rtgui_recv();
				if(result == RT_EOK)
				{
					/* perform event handler */
					if(RTGUI_WIDGET_EVENT_HANDLE(win) != RT_NULL)
						RTGUI_WIDGET_EVENT_CALL(win, event);
				}
			}
		}
	}
}

void rtgui_win_set_rect(rtgui_win_t* win, rtgui_rect_t* rect)
{
	rt_thread_t thread;
	struct rtgui_event_win_resize event;

	if(win == RT_NULL || rect == RT_NULL) return;

	RTGUI_WIDGET(win)->extent = *rect;

	thread = rtgui_get_server();
	if(thread != RT_NULL)
	{
		/* set window resize event to server */
		RTGUI_EVENT_WIN_RESIZE_INIT(&event);
		event.wid = win;
		event.rect = *rect;

		rtgui_send(thread, &(event.parent), sizeof(struct rtgui_event_win_resize));
	}
}

void rtgui_win_set_onactivate(rtgui_win_t* win, rtgui_event_handler_ptr handler)
{
	if(win != RT_NULL)
	{
		win->on_activate = handler;
	}
}

void rtgui_win_set_ondeactivate(rtgui_win_t* win, rtgui_event_handler_ptr handler)
{
	if(win != RT_NULL)
	{
		win->on_deactivate = handler;
	}
}

void rtgui_win_set_onclose(rtgui_win_t* win, rtgui_event_handler_ptr handler)
{
	if(win != RT_NULL)
	{
		win->on_close = handler;
	}
}

void rtgui_win_set_title(rtgui_win_t* win, const char *title)
{
	/* modify in local side */
	rt_free(win->title);

	if(title != RT_NULL)
		win->title = rt_strdup(title);
	else
		win->title = RT_NULL;
}

char* rtgui_win_get_title(rtgui_win_t* win)
{
	RT_ASSERT(win != RT_NULL);

	return win->title;
}

rtgui_point_t rtgui_win_get_client_zero(rtgui_win_t *win)
{
	rtgui_point_t p= {0};
	rtgui_rect_t r1,r2;

	rtgui_win_get_client_rect(win, &r1);
	rtgui_widget_get_rect(win, &r2);
	p.x = r1.x1-r2.x1;
	p.y = r1.y1-r2.y1;

	return p;
}

/* return window's client area */
void rtgui_win_get_client_rect(rtgui_win_t *win, rtgui_rect_t *rect)
{
	rtgui_widget_get_rect(win, rect);

	if(win->style & RTGUI_WIN_BORDER)
		rtgui_rect_inflate(rect, -RTGUI_WIDGET_BORDER_SIZE(win));

	if(win->style & RTGUI_WIN_TITLE)
		rect->y1 += win->title_height;

	if(win->style & RTGUI_WIN_MENU)
		rect->y1 += win->menu_height;

	if(win->style & RTGUI_WIN_STATUS)
		rect->y2 -= win->status_height;

}

/* return winodw's title bar area */
void rtgui_win_get_title_rect(rtgui_win_t *win, rtgui_rect_t *rect)
{
	rtgui_widget_get_rect(win, rect);

	if(win->style & RTGUI_WIN_TITLE)
	{
		if(win->style & RTGUI_WIN_BORDER)
		{
			rtgui_rect_inflate(rect, -RTGUI_WIDGET_BORDER_SIZE(win));
		}
		rect->y2 = rect->y1 + win->title_height;
	}
	else
	{
		rect->x1=rect->x2=rect->y1=rect->y2 = 0;
	}
}

rt_uint16_t rtgui_win_get_title_height(rtgui_win_t *win)
{
	return win->title_height;
}

/* return window's closebox area */
void rtgui_win_get_closebox_rect(rtgui_win_t *win, rtgui_rect_t *rect)
{
	rtgui_win_get_title_rect(win,rect);
	if(win->style & RTGUI_WIN_CLOSEBOX)
	{
		/* get close button rect (device value) */
		rect->x2 = rect->x2 - 2;
		rect->x1 = rect->x2 - RTGUI_WIN_CLOSEBOX_WIDTH;
		rect->y1 = rect->y1 + (RC_H(*rect)-RTGUI_WIN_CLOSEBOX_HEIGHT)/2;
		rect->y2 = rect->y1 + RTGUI_WIN_CLOSEBOX_HEIGHT;
	}
}

/* return window's maximum box area */
void rtgui_win_get_maxbox_rect(rtgui_win_t *win, rtgui_rect_t *rect)
{
	rtgui_win_get_closebox_rect(win,rect);
	if(win->style & RTGUI_WIN_CLOSEBOX)
	{
		rtgui_rect_moveto(rect,-2-RTGUI_WIN_CLOSEBOX_WIDTH,0);
	}
}

/* return window's minimum box area */
void rtgui_win_get_minbox_rect(rtgui_win_t *win, rtgui_rect_t *rect)
{
	rtgui_win_get_closebox_rect(win,rect);
	if(win->style & RTGUI_WIN_CLOSEBOX)
	{
		rtgui_rect_moveto(rect,-2-RTGUI_WIN_CLOSEBOX_WIDTH,0);
	}
	if(win->style &RTGUI_WIN_MAXBOX)
	{
		rtgui_rect_moveto(rect,-2-RTGUI_WIN_CLOSEBOX_WIDTH,0);
	}
}

static const rt_uint8_t minbox_bmp[]=
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xF0,0x07,0xF0};
static const rt_uint8_t maxbox_bmp[]=
{0x0F,0xF8,0x0F,0xF8,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x0F,0xF8};
static const rt_uint8_t normal_bmp[]=
{0x03,0xF8,0x03,0xF8,0x02,0x08,0x0F,0xE8,0x0F,0xE8,0x08,0x38,0x08,0x20,0x08,0x20,0x0F,0xE0};
static const rt_uint8_t close_bmp[] =
{0x00,0x00,0x06,0x18,0x03,0x30,0x01,0xE0,0x00,0xC0,0x01,0xE0,0x03,0x30,0x06,0x18,0x00,0x00};


void rtgui_win_draw_closebox(rtgui_win_t *win)
{
	rtgui_dc_t* dc;

	RT_ASSERT(win != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(win);
	if(dc == RT_NULL)return;

	if(win->style & RTGUI_WIN_CLOSEBOX)
	{
		/* draw close box */
		rtgui_rect_t rect;
		/* get close button rect */
		rtgui_win_get_closebox_rect(win, &rect);
		rtgui_dc_fill_rect(dc, &rect);

		/* draw close box */
		if(win->style & RTGUI_WIN_CLOSEBOX_PRESSED)
		{
			rtgui_dc_draw_border(dc, &rect, RTGUI_BORDER_SUNKEN);
			RTGUI_DC_FC(dc) = Blue;
			rtgui_dc_draw_word(dc,rect.x1-0, rect.y1+4, 9, close_bmp);
		}
		else
		{
			rtgui_dc_draw_border(dc, &rect, RTGUI_BORDER_RAISE);
			RTGUI_DC_FC(dc) = theme.foreground;
			rtgui_dc_draw_word(dc,rect.x1-1, rect.y1+3, 9, close_bmp);
		}
	}
	rtgui_dc_end_drawing(dc);
}

void rtgui_win_draw_maxbox(rtgui_win_t *win)
{
	rtgui_dc_t* dc;

	RT_ASSERT(win != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(win);
	if(dc == RT_NULL)return;

	if(win->style & RTGUI_WIN_MAXBOX)
	{
		/* maximum button */
		rtgui_rect_t rect;
		rtgui_win_get_maxbox_rect(win, &rect);
		rtgui_dc_fill_rect(dc, &rect);

		/* draw close box */
		if(win->style & RTGUI_WIN_MAXBOX_PRESSED)
		{
			rtgui_dc_draw_border(dc, &rect, RTGUI_BORDER_SUNKEN);
			RTGUI_DC_FC(dc) = Blue;
			if(!(win->status & RTGUI_WIN_STATUS_MAX))
				rtgui_dc_draw_word(dc,rect.x1-0, rect.y1+4, 9, maxbox_bmp);
			else
				rtgui_dc_draw_word(dc,rect.x1-1, rect.y1+3, 9, normal_bmp);
		}
		else
		{
			rtgui_dc_draw_border(dc, &rect, RTGUI_BORDER_RAISE);
			RTGUI_DC_FC(dc) = theme.foreground;
			if(!(win->status & RTGUI_WIN_STATUS_MAX))
				rtgui_dc_draw_word(dc,rect.x1-1, rect.y1+3, 9, maxbox_bmp);
			else
				rtgui_dc_draw_word(dc,rect.x1-1, rect.y1+3, 9, normal_bmp);
		}
	}
	rtgui_dc_end_drawing(dc);
}

void rtgui_win_draw_minbox(rtgui_win_t *win)
{
	rtgui_dc_t* dc;

	RT_ASSERT(win != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(win);
	if(dc == RT_NULL)return;

	if(win->style & RTGUI_WIN_MINBOX)
	{
		rtgui_rect_t rect;
		rtgui_win_get_minbox_rect(win, &rect);
		rtgui_dc_fill_rect(dc, &rect);

		/* draw close box */
		if(win->style & RTGUI_WIN_MINBOX_PRESSED)
		{
			rtgui_dc_draw_border(dc, &rect, RTGUI_BORDER_SUNKEN);
			RTGUI_DC_FC(dc) = Blue;
			rtgui_dc_draw_word(dc, rect.x1-0, rect.y1+4, 9, minbox_bmp);
		}
		else
		{
			rtgui_dc_draw_border(dc, &rect, RTGUI_BORDER_RAISE);
			RTGUI_DC_FC(dc) = theme.foreground;
			rtgui_dc_draw_word(dc,rect.x1-1, rect.y1+3, 9, minbox_bmp);
		}
	}
	rtgui_dc_end_drawing(dc);
}

// unsigned char VARY_COLOR_TABLE[24][3]={
// {22,66,254},{22,65,251},{21,63,248},{20,61,245},{19,60,242},{19,58,239},{18,56,235},{17,55,233},
// {17,53,230},{16,51,226},{15,50,224},{14,48,220},{14,47,217},{13,45,215},{12,43,211},{11,42,206},
// {11,40,206},{10,38,202},{9, 37,199},{8, 35,197},{7, 34,195},{6, 33,193},{5, 31,191},{4, 29,197}
// }; 

void rtgui_win_draw_title(rtgui_win_t *win)
{
	rtgui_dc_t* dc;

	RT_ASSERT(win != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(win);
	if(dc == RT_NULL)return;

	/* draw title */
	if(win->style & RTGUI_WIN_TITLE)
	{
		rt_uint32_t i,j;
		rtgui_rect_t rect;

		rtgui_win_get_title_rect(win, &rect);

		if(rtgui_win_is_activated(win))
		{
			RTGUI_DC_FC(dc) = theme.win_active;
			for(i = rect.y1,j=0; i < rect.y2; i ++,j++)
			{
				rtgui_dc_draw_hline(dc,rect.x1, rect.x2, i);
			}
		}
		else
		{
			RTGUI_DC_FC(dc) = theme.win_deactive;
			for(i = rect.y1,j=0; i < rect.y2; i ++,j++)
			{
				rtgui_dc_draw_hline(dc,rect.x1, rect.x2, i);
			}

		}

		if(win->status & RTGUI_WIN_STATUS_ACTIVATE)
			RTGUI_DC_FC(dc) = theme.blankspace;
		else
			RTGUI_DC_FC(dc) = RTGUI_RGB(212, 208, 200);

		rect.x1 += 4;
		rect.y1 += 2;
		rect.y2 = rect.y1 + RTGUI_WIN_CLOSEBOX_HEIGHT;

		if(win->image != RT_NULL)
		{
			rtgui_image_paste(win->image, dc, &rect, theme.blankspace);
			rect.x1 += win->image->w+4;
		}

		RTGUI_DC_TEXTALIGN(dc) = RTGUI_ALIGN_LEFT;
		rtgui_dc_draw_text(dc,win->title, &rect);

		if(win->style & RTGUI_WIN_BORDER)
			rtgui_rect_inflate(&rect, RTGUI_WIDGET_BORDER_SIZE(win));

		rtgui_win_draw_closebox(win);
		rtgui_win_draw_maxbox(win);
		rtgui_win_draw_minbox(win);
	}

	rtgui_dc_end_drawing(dc);
}

/* window drawing */
void rtgui_win_draw(rtgui_win_t* win)
{
	rtgui_rect_t rect;
	rtgui_dc_t* dc;

	RT_ASSERT(win != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(win);
	if(dc == RT_NULL)return;

	/* get rect */
	rtgui_widget_get_rect(win, &rect);
	/* draw border */
	if(win->style & RTGUI_WIN_BORDER)
	{
		rtgui_dc_draw_border(dc, &rect, RTGUI_WIDGET_BORDER_STYLE(win));
		/* shrink border */
		rtgui_rect_inflate(&rect, -RTGUI_WIDGET_BORDER_SIZE(win));
	}

	rtgui_win_draw_title(win);
	/* fill client rect background color */
	rtgui_win_get_client_rect(win,&rect);
	rtgui_dc_fill_rect(dc,&rect);

	rtgui_dc_end_drawing(dc);
}
