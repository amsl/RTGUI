/*
 * File      : menu.c
 * This file is part of RT-Thread RTOS
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

#include <rtgui/widgets/menu.h>
#include <topwin.h>
#include <rtgui/rtgui_system.h>


/*  */
static rt_bool_t rtgui_menu_active=0;

static rt_bool_t rtgui_menu_entrust_win_deactivate(pvoid wdt, rtgui_event_t* event);
static rt_bool_t rtgui_menu_event_handler(pvoid wdt, rtgui_event_t* event);
static void rtgui_menu_on_down(rtgui_menu_t* menu);
static void rtgui_menu_on_up(rtgui_menu_t* menu);

static void _rtgui_menu_constructor(rtgui_menu_t *menu)
{
	/* init menu */
	rtgui_widget_set_event_handler(menu, rtgui_menu_event_handler);
	menu->now_item = RT_NULL;
	menu->old_item = RT_NULL;
	menu->name = RT_NULL;
	RTGUI_WIDGET_FLAG(menu) |= RTGUI_WIDGET_FLAG_FOCUSABLE;
	menu->type = RTGUI_MENU_NORMAL;
	menu->unfold = 0;

	menu->head = RT_NULL;
	menu->tail = RT_NULL;
	menu->farther = RT_NULL;

	menu->orient = RTGUI_HORIZONTAL;
	menu->item_count = 0;
	menu->updown = RT_NULL;
	menu->entrust_win = RT_NULL;
}

static void _rtgui_menu_destructor(rtgui_menu_t *menu)
{
	if(menu->name != RT_NULL)
	{
		rt_free(menu->name);
		menu->name = RT_NULL;
	}
	/* if entrust window is exist, destroy it together. */
	if(menu->entrust_win != RT_NULL)
	{
		rtgui_win_destroy(menu->entrust_win);
	}
}

DEFINE_CLASS_TYPE(menu, "menu",
                  RTGUI_WIDGET_TYPE,
                  _rtgui_menu_constructor,
                  _rtgui_menu_destructor,
                  sizeof(struct rtgui_menu));

rtgui_menu_t* rtgui_menu_create(pvoid parent, const char* name, int left, int top, int flag)
{
	rtgui_container_t *container;
	rtgui_menu_t* menu;

	RT_ASSERT(parent != RT_NULL);
	container = RTGUI_CONTAINER(parent);

	menu = rtgui_widget_create(RTGUI_MENU_TYPE);
	if(menu != RT_NULL)
	{
		rtgui_rect_t rect;

		rtgui_widget_get_rect(container, &rect);
		rtgui_widget_rect_to_device(parent,&rect);
		rect.x1 += left;
		rect.y1 += top;
		rect.x2 = rect.x1+16;
		rect.y2 = rect.y1+16;
		rtgui_widget_set_rect(menu, &rect);

		menu->name = rt_strdup(name);
		RTGUI_WIDGET_TEXTALIGN(menu) = RTGUI_ALIGN_CENTER_VERTICAL;

		if(flag == RTGUI_MENU_POPUP)
		{
			/* popup menu use vertical style */
			rtgui_rect_t rect= {0,0,10,10};/* make no difference */
			rtgui_widget_set_border_style(menu, RTGUI_BORDER_SIMPLE);
			menu->orient = RTGUI_VERTICAL;
			/* create entrust window */
			menu->type = RTGUI_MENU_POPUP;
			menu->entrust_win = rtgui_win_create(RT_NULL, name, &rect, RTGUI_WIN_NOBORDER); //POPUP²Ëµ¥ÈÝÆ÷´°¿Ú
			if(menu->entrust_win == RT_NULL)
			{
				rt_kprintf("create entrust win failed!\n");
				rtgui_menu_destroy(menu);
				return RT_NULL;
			}
			RTGUI_WIDGET_USER_DATA(menu->entrust_win) = (rt_uint32_t)menu;
			rtgui_win_set_ondeactivate(menu->entrust_win, rtgui_menu_entrust_win_deactivate);
			/* let entrust win takeover popup menu */
			rtgui_container_add_child(menu->entrust_win, menu);
		}
		else
		{
			/* by the way, main menu use horizontal style normal. */
			menu->type = RTGUI_MENU_NORMAL;
			rtgui_widget_set_border_style(menu, RTGUI_BORDER_SIMPLE);
			menu->orient = RTGUI_HORIZONTAL;
			rtgui_container_add_child(container, menu);
		}
	}
	return menu;
}

void rtgui_menu_destroy(rtgui_menu_t* menu)
{
	rtgui_widget_destroy(menu);
}

rtgui_menu_t* rtgui_menu_get_root(rtgui_menu_t *menu)
{
	rtgui_menu_t *menu_root = menu;

	while(menu_root->farther != RT_NULL)
	{
		menu_root = menu_root->farther;
	}

	return menu_root;
}

rtgui_menu_t* rtgui_menu_get_last(rtgui_menu_t *menu)
{
	rtgui_menu_t *tmpmenu = menu;
	do
	{
		if(tmpmenu->now_item->flag == RTGUI_MENU_POPUP &&
		        tmpmenu->now_item->submenu != RT_NULL &&
		        !RTGUI_WIDGET_IS_HIDE(tmpmenu->now_item->submenu))
		{
			tmpmenu = tmpmenu->now_item->submenu;
		}
		else
			break;

	}
	while(tmpmenu != RT_NULL);

	return tmpmenu;
}

rtgui_menu_t* rtgui_menu_hide_popup(rtgui_menu_t *menu)
{
	while(menu->farther != RT_NULL)
	{
		if(menu->type == RTGUI_MENU_POPUP &&menu->entrust_win != RT_NULL
		        && !RTGUI_WIDGET_IS_HIDE(menu->entrust_win))
		{
			if(menu->farther != RT_NULL)
			{
				menu->farther->unfold = 0;
				rtgui_topwin_hide(menu->entrust_win);
				rtgui_widget_focus(menu->farther);
			}
		}
		menu = menu->farther;
	}
	return menu;
}

/*
 * get menu item width when menu orient is horizontal
 * otherwise get menu item height.
 */
int rtgui_menu_get_item_size(rtgui_menu_t *menu, rtgui_menu_item_t *item)
{
	rtgui_menu_item_t *menu_node = menu->head;
	int size = 0;

	while(menu_node != item)
	{
		if(menu->orient == RTGUI_HORIZONTAL)
			size += menu_node->item_width;
		else
			size += menu_node->item_height;
		menu_node = menu_node->next;
	}

	return size;
}

/* calculate popup menu display area */
/* popup window is floating around screen */
void rtgui_menu_calc_popup_area(rtgui_menu_t *menu)
{
	if(menu->now_item->flag == RTGUI_MENU_POPUP && menu->now_item->submenu != RT_NULL)
	{
		/* goto sub popup menu */
		rtgui_menu_t *submenu = menu->now_item->submenu;
		rtgui_rect_t rect;
		int w, h;
		/* get parent menu area */
		rtgui_widget_get_rect(menu, &rect);
		rtgui_widget_rect_to_device(menu, &rect);

		w = rtgui_widget_get_width(submenu);
		h = rtgui_widget_get_height(submenu);
		/* get display position */
		if(menu->type == RTGUI_MENU_NORMAL)
		{
			rect.x1 += rtgui_menu_get_item_size(menu, menu->now_item);
			if((rect.x1 + w) > hw_driver->width)
				rect.x1 = hw_driver->width - w - 2;
			rect.y1 += RC_H(rect);
			rect.x2 = rect.x1+w;
			rect.y2 = rect.y1+h;
		}
		else
		{
			int sub_w = RC_W(rect);
			rect.x1 += rtgui_widget_get_width(menu);
			if((rect.x1 + sub_w) > hw_driver->width)
				rect.x1 = hw_driver->width - sub_w - 2;
			rect.y1	+= rtgui_menu_get_item_size(menu, menu->now_item);
			rect.x2 = rect.x1+w;
			rect.y2 = rect.y1+h;
		}

		if(rtgui_rect_is_equal(&rect, &RTGUI_WIDGET_EXTENT(submenu)) != RT_EOK)
		{
			rtgui_widget_set_rect(submenu, &rect);
			if(submenu->entrust_win != RT_NULL)
			{
				rtgui_widget_set_rect(submenu->entrust_win, &rect);
			}
		}
		/* entrust window is exist, show it. */
		if(menu->unfold && submenu->entrust_win != RT_NULL)
		{
			if(RTGUI_WIDGET_IS_HIDE(submenu->entrust_win))
			{
				rtgui_win_show(submenu->entrust_win, RT_FALSE);
				rtgui_widget_focus(submenu);
				submenu->now_item = submenu->head;
				rtgui_menu_draw_item(submenu, submenu->now_item);
			}
		}
	}
}

void rtgui_menu_on_item(rtgui_menu_t* menu)
{
	if(menu->now_item->flag == RTGUI_MENU_NORMAL)
	{
		/* normal menu item */
		rtgui_menu_hide_popup(menu);
	}

	if(menu->now_item->func_enter)
	{
		menu->now_item->func_enter(menu);
	}
}

void rtgui_menu_update_selected(rtgui_menu_t* menu)
{
	if(menu->old_item == menu->now_item && menu->now_item->flag == RTGUI_MENU_POPUP)
	{
		if(menu->unfold)
			menu->unfold = 0;
		else
			menu->unfold = 1;
	}

	/* location is changed, and old menu item sense popup menu, hide it. */
	if(menu->old_item->flag == RTGUI_MENU_POPUP)
	{
		rtgui_menu_t *submenu = menu->old_item->submenu;
		rtgui_menu_t *tmpmenu = RT_NULL;

		do
		{
			if(submenu->now_item->flag == RTGUI_MENU_POPUP &&
			        submenu->now_item->submenu != RT_NULL)
			{
				tmpmenu = submenu->now_item->submenu;
			}
			else
				tmpmenu = RT_NULL;

			if(submenu != RT_NULL && submenu->entrust_win != RT_NULL
			        && !RTGUI_WIDGET_IS_HIDE(submenu->entrust_win))
			{
				submenu->unfold = 0;
				rtgui_topwin_hide(submenu->entrust_win);
			}
			submenu = tmpmenu;

		}
		while(submenu != RT_NULL);

		rtgui_widget_focus(menu);
	}

	/* update rbox widget */
	rtgui_menu_draw_item(menu, menu->old_item);
	rtgui_menu_draw_item(menu, menu->now_item);
	/* re-calculate display area. */
	rtgui_menu_calc_popup_area(menu);
}

/* to do something after selected */
void rtgui_menu_set_selected(rtgui_menu_t* menu, int selected)
{
	RT_ASSERT(menu != RT_NULL);

	if(selected >= menu->item_count) return;

	if(selected >= 0)
	{
		rtgui_menu_item_t* menu_node = menu->head;
		while(selected--)
		{
			/* set location is selected item */
			menu_node = menu_node->next;
		}
		menu->old_item = menu->now_item;
		menu->now_item = menu_node;
	}
	rtgui_menu_update_selected(menu);
}

static void rtgui_menu_onmouse(rtgui_menu_t* menu, struct rtgui_event_mouse* event)
{
	int sel=0;
	rtgui_menu_item_t *menu_node = menu->head;

	RT_ASSERT(menu != RT_NULL);
	RT_ASSERT(event  != RT_NULL);

	if(event->button & RTGUI_MOUSE_BUTTON_LEFT)
	{
		rtgui_rect_t rect;

		rtgui_widget_get_rect(menu, &rect);
		rtgui_widget_rect_to_device(menu, &rect);

		rtgui_rect_inflate(&rect, -RTGUI_WIDGET_BORDER_SIZE(menu));
		if(rtgui_rect_contains_point(&rect, event->x, event->y) == RT_EOK)
		{
			if(menu->orient == RTGUI_HORIZONTAL)
			{
				while(menu_node)
				{
					rect.x2 = rect.x1 + menu_node->item_width;
					if(rtgui_rect_contains_point(&rect, event->x, event->y) == RT_EOK)
					{
						rtgui_menu_active = 1;
						if(event->button & RTGUI_MOUSE_BUTTON_DOWN)
						{
							/* down key touch off popup action */
							if(menu_node->flag == RTGUI_MENU_POPUP)
							{
								rtgui_menu_set_selected(menu, sel);
								rtgui_menu_on_item(menu);
							}
						}
						else if(event->button & RTGUI_MOUSE_BUTTON_UP)
						{
							/* up key touch off normal action */
							if(menu_node->flag == RTGUI_MENU_POPUP)
							{
								if(menu_node->submenu != RT_NULL && menu->unfold &&
								        !RTGUI_WIDGET_IS_HIDE(menu_node->submenu->entrust_win))
								{
									rtgui_topwin_activate(menu_node->submenu->entrust_win);
									rtgui_widget_focus(menu_node->submenu);
									rtgui_menu_draw_item(menu_node->submenu, menu_node->submenu->now_item);
								}
							}
							else if(menu_node->flag == RTGUI_MENU_NORMAL)
							{
								rtgui_menu_set_selected(menu, sel);
								rtgui_menu_on_item(menu);
							}
						}
						break;
					}
					rect.x1 = rect.x2;
					sel++;
					menu_node = menu_node->next;
				}
				return;
			}
			else
			{
				while(menu_node)
				{
					rect.y2 = rect.y1 + menu_node->item_height;
					if(rtgui_rect_contains_point(&rect, event->x, event->y) == RT_EOK)
					{
						rtgui_menu_active = 1;
						if(event->button & RTGUI_MOUSE_BUTTON_DOWN)
						{
							/* down key touch off popup action */
							if(menu_node->flag == RTGUI_MENU_POPUP)
							{
								rtgui_menu_set_selected(menu, sel);
								rtgui_menu_on_item(menu);
							}
						}
						else if(event->button & RTGUI_MOUSE_BUTTON_UP)
						{
							/* up key touch off normal action */
							if(menu_node->flag == RTGUI_MENU_POPUP)
							{
								if(menu_node->submenu != RT_NULL && menu->unfold &&
								        !RTGUI_WIDGET_IS_HIDE(menu_node->submenu->entrust_win))
								{
									rtgui_topwin_activate(menu_node->submenu->entrust_win);
									rtgui_widget_focus(menu_node->submenu);
									rtgui_menu_draw_item(menu_node->submenu, menu_node->submenu->now_item);
								}
							}
							else if(menu_node->flag == RTGUI_MENU_NORMAL)
							{
								rtgui_menu_set_selected(menu, sel);
								rtgui_menu_on_item(menu);
							}
						}
						break;
					}
					rect.y1 = rect.y2;
					sel++;
					menu_node = menu_node->next;
				}
				return;
			}
		}
		else
		{
			rtgui_menu_active = 0;

			if(menu->now_item->flag == RTGUI_MENU_POPUP &&
			        menu->now_item->submenu != RT_NULL && menu->unfold &&
			        !RTGUI_WIDGET_IS_HIDE(menu->now_item->submenu))
			{
				rtgui_menu_t *submenu;

				submenu = rtgui_menu_get_last(menu);
				rtgui_menu_hide_popup(submenu);
			}
		}
	}
}

rt_bool_t rtgui_menu_check_sub_loop(rtgui_menu_t *menu)
{
	if(RTGUI_WIDGET_IS_FOCUSED(menu))
		return RT_TRUE;
	if(menu->now_item->flag == RTGUI_MENU_POPUP &&
	        menu->now_item->submenu != RT_NULL && menu->unfold)
	{
		rtgui_menu_check_sub_loop(menu->now_item->submenu);
	}
	return RT_FALSE;
}

/*
 * append a menu item
 * Input: menu   -- menu handle
 * 	      flag   -- RTGUI_MENU_POPUP:popup menu item,0:normal menu item
 *        ID     -- if flags is MENU_FLAG_POPUP ,it is POPUP menu handle,else it is menu ID
 *        name   -- menu item name
 * return : TRUE -- succ.
*/
rt_bool_t rtgui_menu_append(rtgui_menu_t *menu,rt_uint32_t flag,rt_uint32_t ID,char* caption,
                            menu_callback_t func_enter)
{
	rtgui_rect_t item_rect;
	rtgui_menu_item_t *item,*node,*old;

	if(menu == RT_NULL) return RT_FALSE;

	item = rt_malloc(sizeof(rtgui_menu_item_t));
	if(item == RT_NULL) return RT_FALSE;
	rt_memset(item, 0, sizeof(rtgui_menu_item_t));

	item->flag = flag;

	item->caption = rt_strdup(caption);
	rtgui_font_get_string_rect(RTGUI_WIDGET_FONT(menu), item->caption, &item_rect);

	item->shortcut = 0;
	item->bexit = RT_FALSE;
	item->func_enter = func_enter;

	item->item_width = RC_W(item_rect) + RTGUI_MENU_MARGIN_W*2;
	item->item_height = RC_H(item_rect) + RTGUI_MENU_MARGIN_H*2;

	if(item->flag == RTGUI_MENU_POPUP)
	{
		/* popup menu */
		item->item_id = 0;
		item->submenu = (rtgui_menu_t*)ID;
		/* farther menu */
		item->submenu->farther = menu;
	}
	else
	{
		/* normal menu */
		item->item_id = ID;
		item->submenu = 0;
	}
	item->next = RT_NULL;
	menu->item_count++;

	node = menu->head;
	if(!node)
	{
		/* first item */
		menu->head = item;
		menu->tail = item;
		item->prev = RT_NULL;
		return RT_TRUE;
	}

	while(node)
	{
		old = node;
		node = node->next;/* next item */
	}

	/* to tail item on to queue */
	old->next = item;
	item->prev = old;

	/* re-fixed position tail */
	menu->tail = item;

	if(menu->now_item == RT_NULL)
	{
		/* init now item */
		menu->now_item = menu->head;
	}

	if(menu->item_count > 0)
	{
		/* adjust menu extent */
		rtgui_rect_t rect= {0};
		rtgui_menu_item_t *menu_node = menu->head;
		int size=0,max=0;

		if(menu->orient == RTGUI_HORIZONTAL)
		{
			/* horizontal style */
			while(menu_node)
			{
				/* to deed the maximum size to widget width */
				size += menu_node->item_width;
				menu_node = menu_node->next;
			}
			rect.x2 = size;
			rect.y2 = item->item_height;
		}
		else
		{
			/* vertical style,it's popup menu */
			while(menu_node)
			{
				/* to deed the maximum size to widget width */
				if(max < menu_node->item_width)
					max	= menu_node->item_width;
				size += menu_node->item_height;
				menu_node = menu_node->next;
			}
			rect.x2 = max;
			rect.y2 = size;
		}
		rect.x2 += RTGUI_WIDGET_BORDER_SIZE(menu)*2;
		rect.y2 += RTGUI_WIDGET_BORDER_SIZE(menu)*2;
		rtgui_widget_rect_to_device(menu, &rect);

		if(menu->entrust_win != RT_NULL)
		{
			rtgui_widget_set_rect(menu->entrust_win, &rect);
		}

		rtgui_widget_set_rect(menu, &rect);
	}

	return RT_TRUE;
}

/*
 * delete a menu item, return menu handle when he link the submenu.
 */
rtgui_menu_t* rtgui_menu_item_delete(rtgui_menu_t* menu,rtgui_menu_item_t* item)
{
	rtgui_menu_t* ret;

	if(item->prev == RT_NULL)
	{
		if(item->next == RT_NULL)
		{
			/* only one item */
			menu->head = RT_NULL;
			menu->tail = RT_NULL;
		}
		else
		{
			/* delete first item */
			item->next->prev = RT_NULL;
			menu->head = item->next;
		}
	}
	else
	{
		if(item->next == RT_NULL)
		{
			/* delete last item */
			item->prev->next = RT_NULL;
			menu->tail = item->prev;
		}
		else
		{
			/* middle item */
			item->prev->next = item->next;
			item->next->prev = item->prev;
		}
	}

	if(menu->item_count > 0) menu->item_count--;

	if(item->flag == RTGUI_MENU_POPUP)
		ret = item->submenu;	/* submenu handle */
	else
		ret = RT_NULL;

	rt_free(item->caption);
	rt_free(item);

	return ret;
}

/*
 * detect menu whether null
 */
rt_bool_t rtgui_menu_is_null(rtgui_menu_t* hMenu)
{
	if(hMenu->head == RT_NULL)
		return RT_TRUE;
	return RT_FALSE;
}

/* delete popup menu */
void rtgui_menu_popup_delete(rtgui_menu_t* menu)
{
	rtgui_menu_item_t* pItem;
	rtgui_menu_t* submenu;

	if(rtgui_menu_is_null(menu))
	{
		rt_free(menu);
		return;
	}

	pItem = menu->head;
	while(pItem)
	{
		submenu = rtgui_menu_item_delete(menu,pItem);
		if(submenu != RT_NULL)
		{
			/* recursion, delete submenu */
			rtgui_menu_popup_delete(submenu);
		}
		pItem = menu->head;
	}
	rt_free(menu);
}

/* move down arrow */
static void rtgui_menu_on_down(rtgui_menu_t* menu)
{
	RT_ASSERT(menu != RT_NULL);

	if(menu->now_item==RT_NULL)
	{
		return ;
	}

	menu->old_item = menu->now_item;
	/* it's last item? */
	if(menu->now_item->next==RT_NULL)
	{
		/* yes, to skip first item */
		menu->now_item = menu->head;
	}
	else
	{
		/* next */
		menu->now_item = menu->now_item->next;
	}

	if(menu->updown != RT_NULL)
	{
		menu->updown(menu);
	}

	if(menu->type == RTGUI_MENU_POPUP &&
	        menu->entrust_win != RT_NULL &&
	        !rtgui_win_is_activated(menu->entrust_win))
	{
		rtgui_topwin_activate(menu->entrust_win);
	}
	rtgui_menu_update_selected(menu);
}

/* move up arrow */
static void rtgui_menu_on_up(rtgui_menu_t* menu)
{
	RT_ASSERT(menu != RT_NULL);

	if(menu->now_item==RT_NULL)
	{
		return ;
	}

	menu->old_item = menu->now_item;
	/* it's first item? */
	if(menu->now_item->prev==RT_NULL)
	{
		/* yes, continue move */
		menu->now_item = menu->tail;
	}
	else
	{
		/* prev */
		menu->now_item = menu->now_item->prev;
	}

	if(menu->updown != RT_NULL)
	{
		menu->updown(menu);
	}

	if(menu->type == RTGUI_MENU_POPUP &&
	        menu->entrust_win != RT_NULL &&
	        !rtgui_win_is_activated(menu->entrust_win))
	{
		rtgui_topwin_activate(menu->entrust_win);
	}
	rtgui_menu_update_selected(menu);
}

static rt_bool_t rtgui_menu_entrust_win_deactivate(pvoid wdt, rtgui_event_t* event)
{
	rtgui_win_t *win = RTGUI_WIN(wdt);
	rtgui_menu_t *menu;

	menu = (rtgui_menu_t*)RTGUI_WIDGET_USER_DATA(win);

	/* rt_kprintf("menu->entrust_win:%s, menu=%x, rtgui_menu_active=%x\n",
		menu->entrust_win->title, menu, rtgui_menu_active); */

	if(rtgui_menu_active) return RT_TRUE;

	if(menu->now_item->flag == RTGUI_MENU_POPUP &&
	        menu->now_item->submenu != RT_NULL &&
	        !RTGUI_WIDGET_IS_HIDE(menu->now_item->submenu))
	{
		menu->unfold = 0;
		rtgui_topwin_hide(menu->now_item->submenu->entrust_win);
	}

	rtgui_menu_hide_popup(menu);

	return RT_TRUE;
}

static void rtgui_menu_to_submenu(rtgui_menu_t* menu)
{
	rtgui_menu_t *submenu;
	RT_ASSERT(menu != RT_NULL);

	if(menu->now_item->flag == RTGUI_MENU_POPUP)
	{
		submenu = menu->now_item->submenu;
		if(!menu->unfold)
		{
			/* if it is unfold, so entrust_win was hide, show it first. */
			menu->unfold = 1;
			rtgui_win_show(submenu->entrust_win, RT_FALSE);
		}
		if(!rtgui_win_is_activated(submenu->entrust_win))
		{
			rtgui_topwin_activate(submenu->entrust_win);
		}
		rtgui_widget_focus(submenu);

		/* set now item ,it's very improtant */
		submenu->now_item = submenu->head;
		rtgui_menu_draw_item(submenu, submenu->now_item);
	}
}

static const rt_uint8_t menu_popup_byte[7] = {0x40,0x60,0x70,0x78,0x70,0x60,0x40};

void rtgui_menu_draw_item(rtgui_menu_t* menu, rtgui_menu_item_t *item)
{
	rtgui_rect_t rect, item_rect;
	rtgui_dc_t* dc;

	RT_ASSERT(menu != RT_NULL);
	if(item == RT_NULL) return;

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(menu);
	if(dc == RT_NULL)return;

	rtgui_widget_get_rect(menu, &rect);

	rtgui_rect_inflate(&rect, -RTGUI_WIDGET_BORDER_SIZE(menu));
	item_rect = rect;

	if(menu->orient == RTGUI_HORIZONTAL)
	{
		int size = 0;
		rtgui_menu_item_t *menu_node = menu->head;
		while(menu_node != item)
		{
			/* calculate menu item display position */
			size += menu_node->item_width;
			menu_node = menu_node->next;
		}
		item_rect.x1 += size;
		item_rect.x2 = item_rect.x1 + menu_node->item_width;
	}
	else
	{
		int size = 0;
		rtgui_menu_item_t *menu_node = menu->head;

		while(menu_node != item)
		{
			/* calculate menu item display position */
			size += menu_node->item_height;
			menu_node = menu_node->next;
		}
		/* set the text rect */
		item_rect.y1 += size;
		item_rect.y2 = item_rect.y1 + menu_node->item_height;
	}

	/* draw menu item */
	if(menu->now_item == item)
	{
		/* current item */
		RTGUI_DC_BC(dc) = DarkBlue;
		RTGUI_DC_FC(dc) = theme.blankspace;

		rtgui_dc_fill_rect(dc, &item_rect);
		if(item->image)
		{
			item_rect.x1 += item->image->w + RTGUI_WIDGET_BORDER_SIZE(menu);
		}

		item_rect.x1 += RTGUI_MENU_MARGIN_W;
		rtgui_dc_draw_text(dc, item->caption, &item_rect);
		if((item->flag == RTGUI_MENU_POPUP) && (menu->orient==RTGUI_VERTICAL))
		{
			RTGUI_DC_FC(dc) = theme.blankspace;
			rtgui_dc_draw_byte(dc,rect.x2-8, item_rect.y1+7, 7, menu_popup_byte);
		}
		if(item->image)
		{
			item_rect.x1 -= item->image->w + RTGUI_WIDGET_BORDER_SIZE(menu);
		}
		item_rect.x1 -= RTGUI_MENU_MARGIN_W;
	}
	else
	{
		/* other item */
		RTGUI_DC_BC(dc) = theme.background;
		RTGUI_DC_FC(dc) = Black;
		rtgui_dc_fill_rect(dc,&item_rect);
		if(item->image)
		{
			item_rect.x1 += item->image->w + RTGUI_WIDGET_BORDER_SIZE(menu);
		}

		item_rect.x1 += RTGUI_MENU_MARGIN_W;
		rtgui_dc_draw_text(dc, item->caption, &item_rect);
		if((item->flag == RTGUI_MENU_POPUP) && (menu->orient==RTGUI_VERTICAL))
		{
			rtgui_dc_draw_byte(dc,rect.x2-8, item_rect.y1+7, 7, menu_popup_byte);
		}
		if(item->image)
		{
			item_rect.x1 -= item->image->w + RTGUI_WIDGET_BORDER_SIZE(menu);
		}
		item_rect.x1 -= RTGUI_MENU_MARGIN_W;
	}

	rtgui_dc_end_drawing(dc);
}

void rtgui_menu_ondraw(rtgui_menu_t* menu)
{
	rtgui_rect_t rect, item_rect;
	rtgui_dc_t* dc;

	RT_ASSERT(menu != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(menu);
	if(dc == RT_NULL)return;

	rtgui_widget_get_rect(menu, &rect);

	rtgui_rect_inflate(&rect,-RTGUI_WIDGET_BORDER_SIZE(menu));
	RTGUI_DC_BC(dc) = theme.background;
	rtgui_dc_fill_rect(dc,&rect);
	rtgui_rect_inflate(&rect, RTGUI_WIDGET_BORDER_SIZE(menu));
	rtgui_dc_draw_border(dc, &rect, RTGUI_WIDGET_BORDER_STYLE(menu));

	rtgui_rect_inflate(&rect, -RTGUI_WIDGET_BORDER_SIZE(menu));
	item_rect = rect;

	if(menu->orient == RTGUI_HORIZONTAL)
	{
		/* horizontal style */
		rtgui_menu_item_t* menu_node = menu->head;

		/* draw each menu item */
		while(menu_node)
		{
			/* set the text rect */
			item_rect.x2 = item_rect.x1 + menu_node->item_width;
			/* draw text */
			if(menu_node == menu->now_item)
			{
				/* draw current item */
				if(menu->unfold)
				{
					RTGUI_DC_BC(dc) = DarkBlue;
					RTGUI_DC_FC(dc) = theme.blankspace;
				}
				else
				{
					RTGUI_DC_BC(dc) = theme.background;
					RTGUI_DC_FC(dc) = Black;
				}

				rtgui_dc_fill_rect(dc, &item_rect);
				if(menu_node->image)
				{
					item_rect.x1 += menu_node->image->w;
				}

				item_rect.x1 += RTGUI_MENU_MARGIN_W;
				rtgui_dc_draw_text(dc, menu_node->caption, &item_rect);
				if((menu_node->flag == RTGUI_MENU_POPUP) && (menu->orient==RTGUI_VERTICAL))
				{
					rtgui_dc_draw_byte(dc,rect.x2-8, item_rect.y1+7, 7, menu_popup_byte);
				}
				if(menu_node->image)
				{
					item_rect.x1 -= menu_node->image->w;
				}
				item_rect.x1 -= RTGUI_MENU_MARGIN_W;
			}
			else
			{
				RTGUI_DC_BC(dc) = theme.background;
				RTGUI_DC_FC(dc) = Black;
				rtgui_dc_fill_rect(dc,&item_rect);
				if(menu_node->image)
				{
					item_rect.x1 += menu_node->image->w;
				}

				item_rect.x1 += RTGUI_MENU_MARGIN_W;
				rtgui_dc_draw_text(dc, menu_node->caption, &item_rect);
				if(menu_node->image)
				{
					item_rect.x1 -= menu_node->image->w;
				}
				item_rect.x1 -= RTGUI_MENU_MARGIN_W;
			}
			menu_node = menu_node->next;

			item_rect.x1 = item_rect.x2;
		}
	}
	else
	{
		/* vertical style */
		rtgui_menu_item_t* menu_node = menu->head;

		/* draw each menu item */
		while(menu_node)
		{
			/* set the text rect */
			item_rect.y2 = item_rect.y1 + menu_node->item_height;
			/* draw text */
			if(menu_node == menu->now_item)
			{
				/* draw current item */
				if(menu->unfold)
				{
					RTGUI_DC_BC(dc) = DarkBlue;
					RTGUI_DC_FC(dc) = theme.blankspace;
				}
				else
				{
					RTGUI_DC_BC(dc) = theme.background;
					RTGUI_DC_FC(dc) = Black;
				}

				rtgui_dc_fill_rect(dc, &item_rect);
				if(menu_node->image)
				{
					item_rect.x1 += menu_node->image->w;
				}

				item_rect.x1 += RTGUI_MENU_MARGIN_W;
				rtgui_dc_draw_text(dc, menu_node->caption, &item_rect);
				if((menu_node->flag == RTGUI_MENU_POPUP) && (menu->orient==RTGUI_VERTICAL))
				{
					rtgui_dc_draw_byte(dc,rect.x2-8, item_rect.y1+7, 7, menu_popup_byte);
				}
				if(menu_node->image)
				{
					item_rect.x1 -= menu_node->image->w;
				}
				item_rect.x1 -= RTGUI_MENU_MARGIN_W;
			}
			else
			{
				RTGUI_DC_BC(dc) = theme.background;
				RTGUI_DC_FC(dc) = Black;
				rtgui_dc_fill_rect(dc,&item_rect);
				if(menu_node->image)
				{
					item_rect.x1 += menu_node->image->w;
				}

				item_rect.x1 += RTGUI_MENU_MARGIN_W;
				rtgui_dc_draw_text(dc, menu_node->caption, &item_rect);
				if(menu_node->flag == RTGUI_MENU_POPUP)
					rtgui_dc_draw_byte(dc,rect.x2-8, item_rect.y1+7, 7, menu_popup_byte);
				if(menu_node->image)
				{
					item_rect.x1 -= menu_node->image->w;;
				}
				item_rect.x1 -= RTGUI_MENU_MARGIN_W;
			}
			menu_node = menu_node->next;

			item_rect.y1 = item_rect.y2;
		}
	}

	rtgui_dc_end_drawing(dc);
}

static rt_bool_t rtgui_menu_event_handler(pvoid wdt, rtgui_event_t* event)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	rtgui_menu_t *menu = RTGUI_MENU(wdt);

	RT_ASSERT(widget != RT_NULL);

	switch(event->type)
	{
	case RTGUI_EVENT_PAINT:
		if(widget->on_draw != RT_NULL)
		{
			widget->on_draw(widget, event);
		}
		else
		{
			rtgui_menu_ondraw(menu);
		}
		return RT_FALSE;

	case RTGUI_EVENT_MOUSE_BUTTON:
		if(widget->on_mouseclick != RT_NULL)
		{
			widget->on_mouseclick(widget, event);
		}
		else
		{
			rtgui_menu_onmouse(menu, (struct rtgui_event_mouse*)event);
		}
		return RT_TRUE;

	case RTGUI_EVENT_KBD:
	{
		struct rtgui_event_kbd *ekbd = (struct rtgui_event_kbd*)event;

		if(RTGUI_KBD_IS_DOWN(ekbd))
		{
			rt_uint16_t key = ekbd->key;

			switch(key)
			{
			case RTGUIK_UP:
				if(!rtgui_menu_active) rtgui_menu_active = 1;
				/* goto prev menu item */
				if(menu->type == RTGUI_MENU_POPUP)
				{
					rtgui_menu_on_up(menu);
				}
				else
				{
					/* goto submenu */
					rtgui_menu_to_submenu(menu);
				}
				break;

			case RTGUIK_DOWN:
				if(!rtgui_menu_active) rtgui_menu_active = 1;
				/* goto next menu item */
				if(menu->type == RTGUI_MENU_POPUP)
				{
					rtgui_menu_on_down(menu);
				}
				else
				{
					/* goto submenu */
					rtgui_menu_to_submenu(menu);
				}
				break;

			case RTGUIK_LEFT:
				if(!rtgui_menu_active) rtgui_menu_active = 1;
				/* goto prev menu item */
				if(menu->type == RTGUI_MENU_POPUP)
				{
					/* change submenu into parent menu */
					if(menu->farther->type == RTGUI_MENU_NORMAL)
					{
						/* first order popup menu. */
						menu->farther->unfold = 1;
						rtgui_widget_focus(menu->farther);
						/* re-dispose event, turn it to "else" branch */
						rtgui_menu_event_handler(menu->farther, event);
					}
					else
					{
						/* more level submenu */
						menu->farther->unfold = 0;
						rtgui_topwin_hide(menu->entrust_win);
						rtgui_widget_focus(menu->farther);
						rtgui_menu_draw_item(menu->farther, menu->farther->now_item);
					}
				}
				else
				{
					/* "else" branch */
					rtgui_menu_on_up(menu);
					rtgui_widget_focus(menu);
				}
				break;

			case RTGUIK_RIGHT:
				if(!rtgui_menu_active) rtgui_menu_active = 1;
				/* goto next menu item */
				if(menu->type == RTGUI_MENU_POPUP)
				{
					/* change popup menu into normal menu */
					if(menu->now_item->flag == RTGUI_MENU_POPUP)
					{
						/* goto submenu */
						rtgui_menu_to_submenu(menu);
					}
					else
					{
						/* farther menu is normal. */
						menu = rtgui_menu_hide_popup(menu);
						menu->unfold = 1;
						/* re-dispose event, turn it to "else" branch */
						rtgui_menu_event_handler(menu, event);
					}
				}
				else
				{
					/* "else" branch */
					rtgui_menu_on_down(menu);
					rtgui_widget_focus(menu);
				}
				break;

			case RTGUIK_RETURN:
				if(menu->now_item->flag == RTGUI_MENU_POPUP)
				{
					/* goto event:kbd,RTGUIK_RIGHT */
					ekbd->key = RTGUIK_RIGHT;
					rtgui_menu_event_handler(menu, event);
					break;
				}
				rtgui_menu_on_item(menu);
				rtgui_menu_active = 0;
				break;

			default:
				break;
			}
		}
		return RT_TRUE;
	}
	default:
		return RT_FALSE;
	}
}

