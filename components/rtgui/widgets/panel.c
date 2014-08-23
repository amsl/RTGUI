/*
 * File      : panel.c
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
#include <rtgui/event.h>
#include <rtgui/rtgui_app.h>
#include <rtgui/widgets/panel.h>
#include <rtgui/widgets/view.h>
#include <rtgui/widgets/window.h>
#include <mouse.h>

rtgui_rect_t *external_clip_rect;
rt_uint32_t	external_clip_size;

/* the global parameter */
rtgui_panel_t *rtgui_panel = RT_NULL;
rt_thread_t *__panel_thread=RT_NULL;

rt_thread_t* rtgui_panel_thread_get(void)
{
	return __panel_thread;
}

static void _rtgui_panel_constructor(rtgui_panel_t *panel)
{
	panel->status = 0;
	RTGUI_WIDGET_FLAG(panel) |= RTGUI_WIDGET_FLAG_FOCUSABLE;
	panel->image = RT_NULL;
	rtgui_widget_set_event_handler(panel, rtgui_panel_event_handler);
	panel->modal_widget = RT_NULL;
}

static void _rtgui_panel_destructor(rtgui_panel_t *panel)
{
	if(external_clip_size > 0)
	{
		rt_free(external_clip_rect);
		external_clip_rect = RT_NULL;
		external_clip_size = 0;
	}
}

DEFINE_CLASS_TYPE(panel, "panel",
                  RTGUI_CONTAINER_TYPE,
                  _rtgui_panel_constructor,
                  _rtgui_panel_destructor,
                  sizeof(struct rtgui_panel));

rtgui_panel_t* rtgui_panel_create(int left, int top, int w, int h)
{
	rtgui_rect_t rect;
	rtgui_panel_t *panel;

	panel = rtgui_widget_create(RTGUI_PANEL_TYPE);
	if(panel == RT_NULL) return RT_NULL;
	rtgui_panel_set(panel);

	RTGUI_WIDGET(panel)->toplevel = RTGUI_WIDGET(panel);

	rect.x1 = left;
	rect.y1 = top;
	rect.x2 = rect.x1+w;
	rect.y2 = rect.y1+h;

	rtgui_widget_set_rect(panel,&rect);
	rtgui_widget_update_clip(panel);

	rtgui_app_set_widget(panel);

	return panel;
}

void rtgui_panel_destroy(char* name)
{
	/* panel need lingering in memory forever */
}

void rtgui_panel_set(rtgui_panel_t *panel)
{
	rtgui_panel = panel;
	__panel_thread = rt_thread_self();
}

rtgui_panel_t* rtgui_panel_get(void)
{
	return rtgui_panel;
}

void rtgui_panel_show(rtgui_panel_t *panel)
{
	rtgui_widget_update(panel);
}

void rtgui_panel_ondraw(rtgui_panel_t* panel)
{
	rtgui_rect_t rect;
	rtgui_dc_t* dc;

	RT_ASSERT(panel != RT_NULL);

	if(panel->image == RT_NULL)
	{
		rtgui_widget_get_rect(panel, &rect);
		/* begin drawing */
		dc = rtgui_dc_begin_drawing(panel);
		if(dc == RT_NULL)return;
		rtgui_dc_fill_rect(dc,&rect);
		rtgui_dc_end_drawing(dc);
	}
}

void rtgui_panel_draw_by_rect(rtgui_panel_t* panel, rtgui_rect_t *rect)
{
	rtgui_container_t *container = RTGUI_CONTAINER(panel);
	rtgui_list_t* node;
	struct rtgui_event_paint event;
	rtgui_dc_t* dc;
	
	if(panel->image == RT_NULL)
	{
		/* begin drawing */
		dc = rtgui_dc_begin_drawing(panel);
		if(dc == RT_NULL)return;
		rtgui_dc_fill_rect(dc, rect);
		rtgui_dc_end_drawing(dc);
	}
	
	RTGUI_EVENT_PAINT_INIT(&event);

	rtgui_list_foreach(node, &(container->children))
	{
		rtgui_widget_t* w;
		w = rtgui_list_entry(node, rtgui_widget_t, sibling);
		if(RTGUI_WIDGET_IS_HIDE(w)) continue; /* it's hide, respond no to request */
		if(RTGUI_IS_WIN(w)) continue; /* ignore window. */
		/* only draw intersect widget */
		if(rtgui_rect_is_intersect(&w->extent, rect) == RT_EOK)
		{
			if(RTGUI_WIDGET_EVENT_HANDLE(w) != RT_NULL)
			{
				RTGUI_WIDGET_EVENT_CALL(w, (rtgui_event_t*)&event);
			}
		}
	}
}

rt_bool_t rtgui_panel_event_handler(pvoid wdt,rtgui_event_t *event)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	rtgui_panel_t *panel = RTGUI_PANEL(wdt);

	RT_ASSERT((wdt != RT_NULL) && (event != RT_NULL));

	switch(event->type)
	{
	case RTGUI_EVENT_WIN_CLOSE:
	case RTGUI_EVENT_WIN_ACTIVATE:
	case RTGUI_EVENT_WIN_DEACTIVATE:
	{
		struct rtgui_event_win* wevent = (struct rtgui_event_win*)event;
		rtgui_win_t* win = wevent->wid;
		if(win != RT_NULL && RTGUI_WIDGET_EVENT_HANDLE(win) != RT_NULL)
		{
			RTGUI_WIDGET_EVENT_CALL(win, event);
		}
		return RT_FALSE;
	}
	case RTGUI_EVENT_WIN_MOVE:
	{
		struct rtgui_event_win_move* wevent = (struct rtgui_event_win_move*)event;
		rtgui_win_t* win = wevent->wid;
		if(win != RT_NULL && RTGUI_WIDGET_EVENT_HANDLE(win) != RT_NULL)
		{
			RTGUI_WIDGET_EVENT_CALL(win, event);
		}
		return RT_FALSE;
	}
	case RTGUI_EVENT_PAINT:
		if(widget->on_draw != RT_NULL)
			widget->on_draw(widget, event);
		else
			rtgui_panel_ondraw(panel);
		rtgui_container_dispatch_event(panel, event);
		return RT_FALSE;

	case RTGUI_EVENT_MOUSE_BUTTON:
	{
		struct rtgui_event_mouse* emouse = (struct rtgui_event_mouse*)event;
		rtgui_win_t* win = emouse->wid;

		/* check the destination window */
		if(win != RT_NULL && RTGUI_WIDGET_EVENT_HANDLE(win) != RT_NULL)
		{
			RTGUI_WIDGET_EVENT_CALL(win, event);
		}
		else
		{
			if(RTGUI_PANEL_IS_MODAL_MODE(panel))
			{
				/* let modal widget to handle it */
				if(panel->modal_widget != RT_NULL && RTGUI_WIDGET_EVENT_HANDLE(panel->modal_widget) != RT_NULL)
				{
					RTGUI_WIDGET_EVENT_CALL(panel->modal_widget, event);
				}
			}
			else
			{
				return rtgui_container_dispatch_mouse_event(panel,(struct rtgui_event_mouse*)event);
			}
		}
		return RT_TRUE;
	}

	case RTGUI_EVENT_MOUSE_MOTION:
		return rtgui_container_dispatch_mouse_event(panel,(struct rtgui_event_mouse*)event);

	case RTGUI_EVENT_KBD:
	{
		struct rtgui_event_kbd *kbd = (struct rtgui_event_kbd*)event;
		rtgui_win_t* win = kbd->wid;

		/* check the destination window */
		if(win != RT_NULL && RTGUI_WIDGET_EVENT_HANDLE(win) != RT_NULL)
		{
			RTGUI_WIDGET_EVENT_CALL(win, event);
		}
		else
		{
			if(RTGUI_PANEL_IS_MODAL_MODE(panel))
			{
				/* let modal widget to handle it */
				if(panel->modal_widget != RT_NULL && RTGUI_WIDGET_EVENT_HANDLE(panel->modal_widget) != RT_NULL)
				{
					RTGUI_WIDGET_EVENT_CALL(panel->modal_widget, event);
				}
			}
			else if(RTGUI_CONTAINER(panel)->focused != RT_NULL)
			{
				if(RTGUI_CONTAINER(panel)->focused != widget)
				{
					if(RTGUI_WIDGET_EVENT_HANDLE(RTGUI_CONTAINER(panel)->focused) != RT_NULL)
						RTGUI_WIDGET_EVENT_CALL(RTGUI_CONTAINER(panel)->focused, event);
				}
			}
		}
		return RT_TRUE;
	}

	default:
		return rtgui_container_event_handler(widget, event);
	}
}

void rtgui_panel_event_loop(rtgui_panel_t *panel)
{
	rt_err_t result;
	rtgui_app_t* app;
	rtgui_event_t* event;

	app = rtgui_app_self();
	RT_ASSERT(app != RT_NULL);
	event = (rtgui_event_t*)app->event_buffer;

	if(RTGUI_PANEL_IS_MODAL_MODE(panel))
	{
		/* event loop for modal mode shown view */
		while(RTGUI_PANEL_IS_MODAL_MODE(panel))
		{
			if(app->on_idle != RT_NULL)
			{
				result = rtgui_recv_nosuspend();
				if(result == RT_EOK)
				{
					if(RTGUI_WIDGET_EVENT_HANDLE(panel) != RT_NULL)
						RTGUI_WIDGET_EVENT_CALL(panel, event);
				}
				else if(result == -RT_ETIMEOUT)
					app->on_idle(panel, event);
			}
			else
			{
				result = rtgui_recv();
				if(result == RT_EOK)
				{
					if(RTGUI_WIDGET_EVENT_HANDLE(panel) != RT_NULL)
						RTGUI_WIDGET_EVENT_CALL(panel, event);
				}
			}
		}
	}
	else
	{
		while(1)
		{
			if(app->on_idle != RT_NULL)
			{
				result = rtgui_recv_nosuspend();
				if(result == RT_EOK)
				{
					if(RTGUI_WIDGET_EVENT_HANDLE(panel) != RT_NULL)
						RTGUI_WIDGET_EVENT_CALL(panel, event);
				}
				else if(result == -RT_ETIMEOUT)
					app->on_idle(panel, event);
			}
			else
			{
				result = rtgui_recv();
				if(result == RT_EOK)
				{
					/* perform event handler */
					if(RTGUI_WIDGET_EVENT_HANDLE(panel) != RT_NULL)
						RTGUI_WIDGET_EVENT_CALL(panel, event);
				}
			}
		}
	}
}



