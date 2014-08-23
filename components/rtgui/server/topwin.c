/*
 * File      : win_server.c
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
 * 2011-04-04     amsl         let topwin regard as window kits.
 *                             by the way,they meaning is different.
 */
#include <rtgui/event.h>
#include <rtgui/widgets/panel.h>
#include <rtgui/widgets/window.h>
#include <topwin.h>
#include <mouse.h>

rtgui_win_t* rtgui_server_focus_win = RT_NULL;

static rtgui_list_t _rtgui_win_show_list;
static rtgui_list_t _rtgui_win_hide_list;

void rtgui_toplevel_redraw(rtgui_rect_t* rect);

void rtgui_topwin_init(void)
{
	/* initizlize window list */
	rtgui_list_init(&_rtgui_win_show_list);
	rtgui_list_init(&_rtgui_win_hide_list);
}

void rtgui_win_insert(rtgui_list_t *list, rtgui_win_t *win)
{
	rtgui_list_insert(list, &(win->list));
}

static rt_bool_t rtgui_win_search_in_list(rtgui_win_t* win, rtgui_list_t *list)
{
	rtgui_list_t *node;
	rtgui_win_t* wid;

	/* search in list */
	rtgui_list_foreach(node, list)
	{
		wid = rtgui_list_entry(node, rtgui_win_t, list);
		if(wid == win) return RT_TRUE;
	}

	return RT_FALSE;
}

/* add a window to window list[hide] */
rt_err_t rtgui_topwin_add(rtgui_win_t* win)
{
	if(win != RT_NULL)
	{
		/* update clip info */
		rtgui_widget_update_clip(win);
		rtgui_list_insert(&(_rtgui_win_hide_list), &(win->list));
		return RT_EOK;
	}
	return RT_ERROR;
}

/* remove a window from window list */
rt_err_t rtgui_topwin_remove(rtgui_win_t* win)
{
	RT_ASSERT(win != RT_NULL);
	/* first hide when it in show list */
	if(rtgui_win_search_in_list(win,&_rtgui_win_show_list))
	{
		rtgui_topwin_hide(win);
	}

	if(rtgui_win_search_in_list(win, &_rtgui_win_hide_list))
	{
		/* in hide window list */
		/* remove node from list */
		rtgui_list_remove(&_rtgui_win_hide_list, &(win->list));
		return RT_EOK;
	}

	return RT_ERROR;
}

/* activate a win
 */
rt_err_t rtgui_topwin_activate(rtgui_win_t* win)
{
	struct rtgui_event_win event;

	if((rtgui_server_focus_win != RT_NULL) && (rtgui_server_focus_win != win))
	{
		/* deactivate the old focus win  */
		rtgui_topwin_deactivate(rtgui_server_focus_win);
	}

	win->status |= RTGUI_WIN_STATUS_ACTIVATE;
	rtgui_server_focus_win = win;
	rtgui_widget_update_clip(win);
	rtgui_widget_focus(win);
	rtgui_widget_update(win);

	/* activate the raised window */
	RTGUI_EVENT_WIN_ACTIVATE_INIT(&event);
	event.wid = win;

	if(win->on_activate != RT_NULL)
	{
		win->on_activate(win, &(event.parent));
	}

	return RT_EOK;
}

/*
 * deactivate a win
 */
rt_err_t rtgui_topwin_deactivate(rtgui_win_t* win)
{
	struct rtgui_event_win event;

	win->status &= ~RTGUI_WIN_STATUS_ACTIVATE;
	/* it is acitvate before draw window title */
	rtgui_widget_update_clip(win);
	rtgui_win_draw_title(win);

	if(rtgui_server_focus_win == win)
	{
		rtgui_server_focus_win = RT_NULL;
	}

	RTGUI_EVENT_WIN_DEACTIVATE_INIT(&event);
	event.wid = win;

	if(win->on_deactivate != RT_NULL)
	{
		win->on_deactivate(win, &(event.parent));
	}

	return RT_EOK;
}

/* raise window to front */
rt_err_t rtgui_topwin_raise(rtgui_win_t* win)
{
	RT_ASSERT(win != RT_NULL);

	/* find the twin node */
	if(rtgui_win_search_in_list(win, &_rtgui_win_show_list))
	{
		/* in show window list */
		rtgui_list_t *node;
		rtgui_win_t *wid;
		rtgui_rect_t rect = RTGUI_WIDGET_EXTENT(win);
		int front_num = 0;

		/* the window is already placed in front */
		if(&(win->list) == _rtgui_win_show_list.next)
		{
			/* show already */
			rtgui_server_focus_win = RT_NULL;
			rtgui_topwin_activate(win);
			return RT_EOK;
		}

		if(rtgui_server_focus_win != RT_NULL && rtgui_server_focus_win != win)
		{
			/* let the shall unfocused window to deactivate */
			rtgui_topwin_deactivate(rtgui_server_focus_win);
		}

		rtgui_list_foreach(node, &_rtgui_win_show_list)
		{
			/* update window list clip */
			wid = rtgui_list_entry(node, rtgui_win_t, list);
			if(wid == win)
			{
				front_num++;
				continue;
			}
			/* update clip from other window to current window */
			rtgui_toplevel_update_clip_by_rect(wid, &rect, front_num);
			rtgui_win_ondraw(wid);
			front_num++;
		}

		/* remove node from list */
		rtgui_list_remove(&_rtgui_win_show_list, &(win->list));
		/* add to front */
		rtgui_win_insert(&_rtgui_win_show_list, win);

		/* set current window on activate */
		rtgui_topwin_activate(win);

		return RT_EOK;
	}

	return RT_ERROR;
}

/* show a window */
rt_err_t rtgui_topwin_show(rtgui_win_t* win)
{
	rtgui_panel_t *panel = rtgui_panel_get();

	/* find it */
	if(win != RT_NULL)
	{
		if(rtgui_win_search_in_list(win, &_rtgui_win_hide_list))
		{
			/* remove node from hidden list */
			rtgui_list_remove(&_rtgui_win_hide_list, &(win->list));

			/* add node to show list */
			rtgui_win_insert(&_rtgui_win_show_list, win);
			rtgui_update_external_clip_info();
			rtgui_toplevel_update_clip(panel);

			RTGUI_WIDGET_SHOW(win);

			/* activate this window */
			rtgui_topwin_activate(win);
		}
		else if(rtgui_win_search_in_list(win, &_rtgui_win_show_list))
		{
			/* just raise it */
			rtgui_topwin_raise(win);
		}

		return RT_EOK;
	}

	return RT_ERROR;
}

/* hide a window */
rt_err_t rtgui_topwin_hide(rtgui_win_t* win)
{
	rtgui_panel_t *panel = rtgui_panel_get();

	/* found it */
	if(win != RT_NULL)
	{
		rtgui_list_t *node;
		rtgui_win_t *wid;
		rtgui_rect_t rect = RTGUI_WIDGET_EXTENT(win);
		int front_num=0;

		/* remove node from show list */
		rtgui_list_remove(&_rtgui_win_show_list, &(win->list));
		/* insert node to hide list */
		rtgui_list_insert(&_rtgui_win_hide_list, &(win->list));
		RTGUI_WIDGET_HIDE(win);
		win->status &= ~RTGUI_WIN_STATUS_ACTIVATE;
		/* update external clip informations */
		rtgui_update_external_clip_info();

		rtgui_list_foreach(node, &_rtgui_win_show_list)
		{
			wid = rtgui_list_entry(node, rtgui_win_t, list);
			/* update overlap area */
			rtgui_toplevel_update_clip_by_rect(wid, &rect, front_num);
			rtgui_win_ondraw(wid);
			rtgui_widget_update_clip(wid);
			front_num++;
		}

		/* activate next window */
		if(_rtgui_win_show_list.next != RT_NULL)
		{
			rtgui_win_t* wnd = rtgui_list_entry(_rtgui_win_show_list.next,rtgui_win_t, list);

			rtgui_topwin_raise(wnd);
		}
		else
		{
			/* no focus window */
			rtgui_server_focus_win = RT_NULL;
		}

		rtgui_toplevel_redraw(&rect);
		return RT_EOK;
	}

	return RT_ERROR;
}

/* move top window */
rt_err_t rtgui_topwin_move(rtgui_win_t* win, int x, int y)
{
	rtgui_panel_t *panel = rtgui_panel_get();

	if(win != RT_NULL)
	{
		int dx, dy;
		rtgui_list_t *node;
		rtgui_win_t *wid;
		rtgui_rect_t rect = RTGUI_WIDGET_EXTENT(win);
		int front_num = 0;

		/* get the delta move x, y */
		dx = x - RTGUI_WIDGET(win)->extent.x1;
		dy = y - RTGUI_WIDGET(win)->extent.y1;

		/* move window rect */
		rtgui_widget_move_to_logic(RTGUI_WIDGET(win), dx, dy);

		/* it's moved, update external info */
		rtgui_update_external_clip_info();

		rtgui_list_foreach(node, &_rtgui_win_show_list)
		{
			wid = rtgui_list_entry(node, rtgui_win_t, list);
			if(wid == win)
			{
				front_num++;
				continue;
			}
			rtgui_toplevel_update_clip_by_rect(wid, &rect, front_num);
			rtgui_win_ondraw(wid);
			front_num++;
		}

		/* direct update itself on account of active. */
		rtgui_widget_update_clip(win);
		rtgui_widget_update(win);

		/* update old window coverage area */
		rtgui_toplevel_redraw(&rect);

		rtgui_topwin_activate(win);
		return RT_EOK;
	}

	return RT_ERROR;
}

/*
 * resize a top win
 * Note: currently, only support resize hidden window
 */
rt_err_t rtgui_topwin_resize(rtgui_win_t* win, rtgui_rect_t* r)
{
	if(win != RT_NULL)
	{
		RTGUI_WIDGET_EXTENT(win) = *r;

		/* update title & border clip info */
		rtgui_widget_update_clip(win);

		return RT_EOK;
	}

	return RT_ERROR;
}

rtgui_win_t* rtgui_topwin_get_wnd(int x, int y)
{
	rtgui_list_t* node;
	rtgui_win_t* win;

	/* search in show win list */
	if(_rtgui_win_show_list.next != RT_NULL)
	{
		rtgui_list_foreach(node, &(_rtgui_win_show_list))
		{
			win = rtgui_list_entry(node, rtgui_win_t, list);

			/* is this window? */
			if(rtgui_rect_contains_point(&(RTGUI_WIDGET_EXTENT(win)), x, y) == RT_EOK)
			{
				return win;
			}
		}
	}
	return RT_NULL;
}

/* front_num: Z-order number above the current window */
void rtgui_toplevel_update_clip_by_rect(pvoid wdt, rtgui_rect_t *rect, int front_num)
{
	rtgui_panel_t *panel = rtgui_panel_get();
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	rtgui_widget_t *child;
	rtgui_list_t *node;
	rtgui_rect_t new_rect;
	rtgui_rect_t scrn_rc;

	new_rect = widget->extent;

	if(rtgui_rect_is_intersect(rect, &new_rect) == RT_EOK)
	{	
		/* two box rect is intersect */
		rtgui_rect_intersect(rect, &new_rect);/* calculate overlapped area */
		rtgui_region_reset(&(widget->clip), &new_rect);/* to install overlap area in the clip */
	
		if(front_num > 0)
		{
			rt_int32_t i;
			rtgui_rect_t *ext_rect;

			ext_rect = external_clip_rect;
			for(i=0; i<front_num; i++)
			{
				rtgui_region_subtract_rect(&(widget->clip), &(widget->clip),ext_rect);
				ext_rect++;
			}
		}
	}
	else
	{
		rtgui_region_reset(&(widget->clip), &rtgui_empty_rect);
	}

	/* widget clip limited to screen area */
	rtgui_gdev_get_rect(hw_driver,&scrn_rc);
	rtgui_region_intersect_rect(&(widget->clip), &(widget->clip), &scrn_rc);

	if(RTGUI_IS_CONTAINER(widget))
	{
		/* is container ,update children */
		rtgui_list_foreach(node, &(RTGUI_CONTAINER(widget)->children))
		{
			child = rtgui_list_entry(node, rtgui_widget_t, sibling);
			if(RTGUI_WIDGET_IS_HIDE(child))continue;
			rtgui_region_subtract_rect(&(widget->clip), &(widget->clip),&(child->extent));
			rtgui_toplevel_update_clip_by_rect(child, rect, front_num);
		}
	}
}

void rtgui_toplevel_redraw(rtgui_rect_t* rect)
{
	rtgui_panel_t *panel = rtgui_panel_get();

	rtgui_toplevel_update_clip_by_rect(panel,rect,external_clip_size);
	rtgui_panel_draw_by_rect(panel, rect);
	/* reset panel clip */
	rtgui_update_external_clip_info();
	rtgui_toplevel_update_clip(panel);
}

void rtgui_topwin_title_onmouse(rtgui_win_t* win, struct rtgui_event_mouse* event)
{
	rtgui_rect_t rect;

	if(event->button & RTGUI_MOUSE_BUTTON_LEFT)
	{
		if(event->button & RTGUI_MOUSE_BUTTON_DOWN)
		{
			if(win->style & RTGUI_WIN_CLOSEBOX)
			{
				rtgui_win_get_closebox_rect(win, &rect);
				rtgui_widget_rect_to_device(win, &rect);
				if(rtgui_rect_contains_point(&rect, event->x, event->y) == RT_EOK)
				{
					win->style |= RTGUI_WIN_CLOSEBOX_PRESSED;
					rtgui_win_draw_closebox(win);
					return;
				}
			}
			if(win->style & RTGUI_WIN_MAXBOX)
			{
				/* use maximum box */
				rtgui_win_get_maxbox_rect(win, &rect);
				rtgui_widget_rect_to_device(win, &rect);
				if(rtgui_rect_contains_point(&rect, event->x, event->y) == RT_EOK)
				{
					win->style |= RTGUI_WIN_MAXBOX_PRESSED;
					rtgui_win_draw_maxbox(win);
					return;
				}
			}
			if(win->style & RTGUI_WIN_MINBOX)
			{
				/* use minimum box */
				rtgui_win_get_minbox_rect(win, &rect);
				rtgui_widget_rect_to_device(win, &rect);
				if(rtgui_rect_contains_point(&rect, event->x, event->y) == RT_EOK)
				{
					win->style |= RTGUI_WIN_MINBOX_PRESSED;
					rtgui_win_draw_minbox(win);
					return;
				}
			}
#ifdef RTGUI_USING_WINMOVE
			/* maybe move window */
			rtgui_cursor_set_location(event->x, event->y);
			rtgui_winrect_set(win);
#endif
		}
		else if(event->button & RTGUI_MOUSE_BUTTON_UP)
		{
			if(win->style & RTGUI_WIN_CLOSEBOX)
			{
				rtgui_win_get_closebox_rect(win, &rect);
				rtgui_widget_rect_to_device(win, &rect);
				if(rtgui_rect_contains_point(&rect, event->x, event->y) == RT_EOK)
				{
					struct rtgui_event_win event;

					win->style &= ~RTGUI_WIN_CLOSEBOX_PRESSED;
					rtgui_win_draw_closebox(win);

					/* send "close window" event to server */
					RTGUI_EVENT_WIN_CLOSE_INIT(&event);
					event.wid = win;

					rtgui_send(win->tid, RTGUI_EVENT(&event), sizeof(struct rtgui_event_win));

					return;
				}
			}
			if(win->style & RTGUI_WIN_MAXBOX)
			{
				rtgui_win_get_maxbox_rect(win, &rect);
				rtgui_widget_rect_to_device(win, &rect);
				if(rtgui_rect_contains_point(&rect, event->x, event->y) == RT_EOK)
				{
					struct rtgui_event_win ewin;

					win->style &= ~RTGUI_WIN_MAXBOX_PRESSED;
					rtgui_win_draw_maxbox(win);

					/* send close event to window */
					RTGUI_EVENT_WIN_MAX_INIT(&ewin);
					ewin.wid = win;
					rtgui_send(win->tid, &(ewin.parent), sizeof(struct rtgui_event_win));
					return;
				}
			}
			if(win->style & RTGUI_WIN_MINBOX)
			{
				rtgui_win_get_minbox_rect(win, &rect);
				rtgui_widget_rect_to_device(win, &rect);
				if(rtgui_rect_contains_point(&rect, event->x, event->y) == RT_EOK)
				{
					struct rtgui_event_win ewin;

					win->style &= ~RTGUI_WIN_MINBOX_PRESSED;
					rtgui_win_draw_minbox(win);

					/* send close event to window */
					RTGUI_EVENT_WIN_MIN_INIT(&ewin);
					ewin.wid = win;
					rtgui_send(win->tid, &(ewin.parent), sizeof(struct rtgui_event_win));
					return;
				}
			}
		}
	}
}

void rtgui_toplevel_update_clip(pvoid wdt)
{
	rtgui_list_t *node;
	rtgui_widget_t *child=RT_NULL, *widget;
	rtgui_panel_t *panel = rtgui_panel_get();

	widget = RTGUI_WIDGET(wdt);

	rtgui_region_reset(&(widget->clip), &(widget->extent));

	if(external_clip_size > 0)
	{
		rt_int32_t i;
		rtgui_rect_t *rect;
		rect = external_clip_rect;
		for(i=0; i<external_clip_size; i++)
		{
			rtgui_region_subtract_rect(&(widget->clip), &(widget->clip),rect);
			rect++;
		}
	}

	if(RTGUI_IS_CONTAINER(widget))
	{
		/* reset child clip under the panel */
		rtgui_list_foreach(node, &(RTGUI_CONTAINER(widget)->children))
		{
			child = rtgui_list_entry(node, rtgui_widget_t, sibling);
			if(RTGUI_WIDGET_IS_HIDE(child))continue;
			rtgui_region_subtract_rect(&(widget->clip), &(widget->clip),&(child->extent));
			rtgui_toplevel_update_clip(child);
		}
	}

	if(rtgui_server_focus_win != RT_NULL && rtgui_win_is_activated(rtgui_server_focus_win))
	{
		/* regain raw clip of focus win. because it is possible by change. */
		rtgui_widget_update_clip(rtgui_server_focus_win);
	}
}

/* update external clip info */
void rtgui_update_external_clip_info(void)
{
	rtgui_rect_t *rect;
	rtgui_win_t *win;
	rtgui_list_t *node;
	rt_uint32_t count=0;
	rtgui_panel_t *panel=rtgui_panel_get();

	RT_ASSERT(panel != RT_NULL);

	if(external_clip_size > 0)
	{
		/* clear old data */
		rt_free(external_clip_rect);
		external_clip_rect = RT_NULL;
		external_clip_size = 0;
	}

	rtgui_list_foreach(node, &_rtgui_win_show_list)
	{
		/* window count */
		count ++;
	}

	external_clip_rect = (rtgui_rect_t*)rt_malloc(sizeof(rtgui_rect_t)*count);
	external_clip_size = count;

	rect = external_clip_rect;

	rtgui_list_foreach(node, &_rtgui_win_show_list)
	{
		win = rtgui_list_entry(node, rtgui_win_t, list);
		*rect = RTGUI_WIDGET_EXTENT(win);
		rect ++;
	}
}

#ifdef RT_USING_FINSH
#include <finsh.h>
/* print window list info */
rt_bool_t list_win(void)
{
	rtgui_list_t *node;
	rtgui_win_t* win;

	rt_kprintf("thread'name  modal    window'id  state status   level window'name         \n");
	rt_kprintf("------------ -------- ---------- ----- -------- ----- --------------------\n");
	/* search in show list */
	rtgui_list_foreach(node, &_rtgui_win_show_list)
	{
		win = rtgui_list_entry(node, rtgui_win_t, list);

		if(win != RT_NULL)
		{
			rt_kprintf("%-12s %-8s 0x%08x %-5s %08x %-20s\n",
			           win->tid->name,
			           ((RTGUI_WIN_IS_MODAL(win))?"OK":"CANCEL"),
			           win,
			           "SHOW",
			           win->status,
			           win->title);
		}
	}
	/* search in hide list */
	rtgui_list_foreach(node, &_rtgui_win_hide_list)
	{
		win = rtgui_list_entry(node, rtgui_win_t, list);

		if(win != RT_NULL)
		{
			rt_kprintf("%-12s %-8s 0x%08x %-5s %08x %05d %-20s\n",
			           win->tid->name,
			           ((RTGUI_WIN_IS_MODAL(win))?"OK":"CANCEL"),
			           win,
			           "HIDE",
			           win->status,
			           win->title);
		}
	}
	if(rtgui_server_focus_win != RT_NULL)
	{
		win = rtgui_server_focus_win;
		rt_kprintf("focus win: %-s,ID=0x%08x\n",win->title,win);
	}
	else
	{
		rt_kprintf("focus win=NULL\n");
	}

	return RT_TRUE;
}
FINSH_FUNCTION_EXPORT(list_win, list show window);
#endif
