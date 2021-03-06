/*
 * File      : container.c
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
#include <rtgui/rtgui_system.h>
#include <rtgui/widgets/container.h>
#include <rtgui/widgets/window.h>
#include <rtgui/widgets/menu.h>
#include <rtgui/widgets/listbox.h>
#include <rtgui/widgets/view.h>

static void _rtgui_container_constructor(rtgui_container_t *container)
{
	/* set event handler and init field */
	rtgui_widget_set_event_handler(container, rtgui_container_event_handler);
	rtgui_list_init(&(container->children));

	/* set focused widget to itself */
	container->focused = RTGUI_WIDGET(container);
	/* set box as focusable widget */
	//RTGUI_WIDGET_FLAG(container) |= RTGUI_WIDGET_FLAG_FOCUSABLE;
}

static void _rtgui_container_destructor(rtgui_container_t *container)
{
	/* destroy child of box */
	rtgui_container_destroy_children(container);
}

static void _rtgui_container_update_toplevel(rtgui_container_t* container)
{
	rtgui_list_t* node;

	rtgui_list_foreach(node, &(container->children))
	{
		rtgui_widget_t* child = rtgui_list_entry(node, rtgui_widget_t, sibling);
		/* set child top */
		child->toplevel = RTGUI_WIDGET(rtgui_widget_get_toplevel(container));

		if(RTGUI_IS_CONTAINER(child))
		{
			_rtgui_container_update_toplevel(RTGUI_CONTAINER(child));
		}
	}
}

DEFINE_CLASS_TYPE(container, "container",
                  RTGUI_WIDGET_TYPE,
                  _rtgui_container_constructor,
                  _rtgui_container_destructor,
                  sizeof(struct rtgui_container));

rt_bool_t rtgui_container_dispatch_event(pvoid wdt, rtgui_event_t* event)
{
	rtgui_container_t *container = RTGUI_CONTAINER(wdt);
	/* handle in child widget */
	rtgui_list_t* node;

	rtgui_list_foreach(node, &(container->children))
	{
		rtgui_widget_t* w;
		w = rtgui_list_entry(node, rtgui_widget_t, sibling);
		if(RTGUI_WIDGET_IS_HIDE(w)) continue; /* it's hide, respond no to request */
		if(RTGUI_IS_WIN(w)) continue; /* ignore window. */
		if(RTGUI_WIDGET_EVENT_HANDLE(w) != RT_NULL)
		{
			if(RTGUI_WIDGET_EVENT_CALL(w, event) == RT_TRUE) return RT_TRUE;
		}
	}
	return RT_FALSE;
}

rt_bool_t rtgui_container_dispatch_mouse_event(pvoid wdt, struct rtgui_event_mouse* event)
{
	rtgui_container_t *container;
	/* handle in child widget */
	rtgui_list_t* node;

	RT_ASSERT(wdt != RT_NULL);

	container = RTGUI_CONTAINER(wdt);
	rtgui_list_foreach(node, &(container->children))
	{
		rtgui_widget_t* w;
		w = rtgui_list_entry(node, rtgui_widget_t, sibling);
		if(RTGUI_WIDGET_IS_HIDE(w))continue;/* it's hide, respond no to request */
		if(rtgui_rect_contains_point(&(w->extent), event->x, event->y) == RT_EOK)
		{
			/* it maybe nesting, pass on in family from parent to chid  */
			if(RTGUI_WIDGET_EVENT_HANDLE(w) != RT_NULL)
			{
				if(RTGUI_WIDGET_EVENT_CALL(w,(rtgui_event_t*)event) == RT_TRUE) return RT_TRUE;
			}
		}
		/*else if(RTGUI_WIDGET_IS_FOCUSABLE(w))
		{	//焦点控件之外的对鼠标点击事件感兴趣的控件
			if(RTGUI_WIDGET_EVENT_HANDLE(w) != RT_NULL)
			{
				RTGUI_WIDGET_EVENT_CALL(w,(rtgui_event_t*)event);
			}
		}*/
	}
	return RT_FALSE;
}

rt_bool_t rtgui_container_event_handler(pvoid wdt, rtgui_event_t* event)
{

	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	rtgui_container_t *container = RTGUI_CONTAINER(widget);

	switch(event->type)
	{
	case RTGUI_EVENT_KBD:
		if(widget->on_key != RT_NULL)
		{
			return widget->on_key(widget, event);
		}
		else
		{
			/* let parent to handle keyboard event */
			if(widget->parent != RT_NULL && widget->parent != widget->toplevel)
			{
				if(RTGUI_WIDGET_EVENT_HANDLE(widget->parent) != RT_NULL)
					return RTGUI_WIDGET_EVENT_CALL(widget->parent, event);
			}
		}
		return RT_TRUE;

	case RTGUI_EVENT_MOUSE_BUTTON:
		/* handle in child widget */
		if(rtgui_container_dispatch_mouse_event(container,(struct rtgui_event_mouse*)event) == RT_FALSE)
		{
			/* handle event in current widget */
			if(widget->on_mouseclick != RT_NULL)
			{
				return widget->on_mouseclick(widget, event);
			}
		}
		return RT_TRUE;

	case RTGUI_EVENT_MOUSE_MOTION:
		if(rtgui_container_dispatch_mouse_event(container,(struct rtgui_event_mouse*)event) == RT_FALSE)
		{
#if 0
			/* handle event in current widget */
			if(widget->on_mousemotion != RT_NULL)
			{
				return widget->on_mousemotion(widget, event);
			}
#endif
		}
		return RT_TRUE;

	case RTGUI_EVENT_TIMER:
	{
		rtgui_timer_t* timer;
		struct rtgui_event_timer* etimer = (struct rtgui_event_timer*) event;

		timer = etimer->timer;
		if(timer->timeout != RT_NULL)
		{
			/* call timeout function */
			timer->timeout(timer, timer->user_data);
		}
		return RT_TRUE;
	}

	case RTGUI_EVENT_COMMAND:
		if(rtgui_container_dispatch_event(container, event) == RT_FALSE)
		{
			if(widget->on_command != RT_NULL)
			{
				return widget->on_command(widget, event);
			}
		}
		return RT_TRUE;

	case RTGUI_EVENT_RESIZE:
		if(rtgui_container_dispatch_event(container, event) == RT_FALSE)
		{
			if(widget->on_size != RT_NULL)
				return widget->on_size(widget, event);
		}
		return RT_TRUE;
	case USER_EVENT_FINDRI:
		return rtgui_container_dispatch_event(container, event);
	
	default:
		/* call parent widget event handler */
		return rtgui_widget_event_handler(widget, event);
	}
}

/*
 * This function will add a child to a box widget
 * Note: this function will not change the widget layout
 * the layout is the responsibility of layout widget, such as box.
 */
void rtgui_container_add_child(pvoid cbox, pvoid wdt)
{
	rtgui_container_t *container;
	rtgui_widget_t *child;

	RT_ASSERT(cbox != RT_NULL);
	RT_ASSERT(wdt != RT_NULL);

	container = RTGUI_CONTAINER(cbox);
	child = RTGUI_WIDGET(wdt);

	/* set parent and toplevel widget */
	child->parent = RTGUI_WIDGET(container);
	/* put widget to parent's child list */
	rtgui_list_append(&(container->children), &(child->sibling));

	/* update child toplevel */
	if(RTGUI_WIDGET(container)->toplevel != RT_NULL && RTGUI_IS_CONTAINER(RTGUI_WIDGET(container)->toplevel))
	{
		child->toplevel = RTGUI_WIDGET(rtgui_widget_get_toplevel(container));

		/* update all child toplevel */
		if(RTGUI_IS_CONTAINER(child))
		{
			_rtgui_container_update_toplevel(RTGUI_CONTAINER(child));
		}
	}

	rtgui_widget_update_clip(child);
}

/* remove a child to widget */
void rtgui_container_remove_child(rtgui_container_t *container, pvoid wdt)
{
	rtgui_widget_t *child = RTGUI_WIDGET(wdt);

	RT_ASSERT(container != RT_NULL);
	RT_ASSERT(child != RT_NULL);

	if(child == container->focused)
	{
		/* set focused to itself */
		container->focused = RTGUI_WIDGET(container);
		rtgui_widget_focus(container);
	}

	/* remove widget from parent's child list */
	rtgui_list_remove(&(container->children), &(child->sibling));

	/* set parent and top widget */
	child->parent = RT_NULL;
	child->toplevel = RT_NULL;
}

/* destroy all child of box */
void rtgui_container_destroy_children(rtgui_container_t *container)
{
	rtgui_list_t* node;

	if(container == RT_NULL) return;

	node = container->children.next;
	while(node != RT_NULL)
	{
		rtgui_widget_t* child = rtgui_list_entry(node, rtgui_widget_t, sibling);

		if(RTGUI_IS_CONTAINER(child))
		{
			/* break parent firstly */
			child->parent = RT_NULL;

			/* destroy child of child */
			rtgui_container_destroy_children(RTGUI_CONTAINER(child));
		}

		/* remove widget from parent's child list */
		rtgui_list_remove(&(container->children), &(child->sibling));

		/* set parent and top widget */
		child->parent = RT_NULL;

		/* destroy object and remove from parent */
		rtgui_object_destroy(RTGUI_OBJECT(child));

		node = node->next;
	}

	container->children.next = RT_NULL;
	container->focused = RTGUI_WIDGET(container);
	if(RTGUI_WIDGET_PARENT(container) != RT_NULL)
		rtgui_widget_focus(container);

	/* update widget clip */
	rtgui_widget_update_clip(container);
}

rtgui_widget_t* rtgui_container_get_first_child(rtgui_container_t* container)
{
	rtgui_widget_t* child = RT_NULL;

	if(container->children.next != RT_NULL)
	{
		child = rtgui_list_entry(container->children.next, rtgui_widget_t, sibling);
	}
	return child;
}

