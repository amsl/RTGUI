/*
 * File      : widget.c
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
#include <rtgui/dc_client.h>
#include <rtgui/widgets/widget.h>
#include <rtgui/widgets/label.h>
#include <rtgui/widgets/window.h>
#include <rtgui/widgets/combobox.h>
#include <rtgui/widgets/view.h>
#include <rtgui/widgets/listbox.h>
#include <rtgui/widgets/panel.h>
#include <topwin.h>


static void _rtgui_widget_constructor(rtgui_widget_t *widget)
{
	if(widget == RT_NULL) return;

	/* set default flag */
	widget->flag = RTGUI_WIDGET_FLAG_DEFAULT;

	/* init list */
	rtgui_list_init(&(widget->sibling));

	/* init gc */
	widget->gc.fc = theme.foreground;
	widget->gc.bc = theme.background;
	widget->gc.font = rtgui_font_default();
	widget->gc.textstyle = RTGUI_TEXTSTYLE_NORMAL;
	widget->gc.textalign = RTGUI_ALIGN_CENTER;
	widget->align = RTGUI_ALIGN_CENTER_VERTICAL;
	rtgui_widget_set_border_style(widget,theme.style);

	widget->tab_index 		= 0;
	widget->tab_stop 		= RT_FALSE;
	/* set parent and toplevel */
	widget->parent			= RT_NULL;
	widget->toplevel		= RT_NULL;

	/* some common event handler */
	widget->on_focus_in		= RT_NULL;
	widget->on_focus_out	= RT_NULL;
	widget->on_draw 		= RT_NULL;
	widget->on_mouseclick 	= RT_NULL;
	widget->on_key 			= RT_NULL;
	widget->on_size 		= RT_NULL;
	widget->on_command 		= RT_NULL;

	/* set default event handler */
	rtgui_widget_set_event_handler(widget,rtgui_widget_event_handler);

	/* init user data private to 0 */
	widget->user_data = 0;

	/* init clip information */
	rtgui_region_init(&(widget->clip));

	/* init hardware dc */
	rtgui_dc_client_init(widget);
}

/* Destroys the widget */
static void _rtgui_widget_destructor(rtgui_widget_t *widget)
{
	if(widget == RT_NULL) return;

	if(widget->parent != RT_NULL)
	{
		/* remove widget from parent's child list */
		rtgui_list_remove(&(RTGUI_CONTAINER(widget->parent)->children), &(widget->sibling));

		widget->parent = RT_NULL;
	}

	/* fini clip region */
	rtgui_region_fini(&(widget->clip));
}

DEFINE_CLASS_TYPE(widget, "widget",
	RTGUI_OBJECT_TYPE,
	_rtgui_widget_constructor,
	_rtgui_widget_destructor,
	sizeof(struct rtgui_widget));

pvoid rtgui_widget_create(rtgui_type_t *widget_type)
{
	pvoid type;

	type = rtgui_object_create(widget_type);

	return type;
}

void rtgui_widget_destroy(pvoid wdt)
{
	rtgui_object_destroy(RTGUI_OBJECT(wdt));
}

void rtgui_widget_set_rect(pvoid wdt, rtgui_rect_t* rect)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);

	if(widget == RT_NULL || rect == RT_NULL) return;

	widget->extent = *rect;

	widget->dc.w = RC_W(widget->extent);
	widget->dc.h = RC_H(widget->extent);

	/* it's not empty, fini it */
	if(rtgui_region_not_empty(&(widget->clip)))
	{
		rtgui_region_fini(&(widget->clip));
	}

	/* reset clip info */
	rtgui_region_init_with_extents(&(widget->clip), rect);
	if((widget->parent != RT_NULL) && (widget->toplevel != RT_NULL))
	{
		/* update widget clip */
		rtgui_widget_update_clip(widget->parent);
	}
}

/* set widget draw style */
void rtgui_widget_set_border_style(pvoid wdt, rt_uint32_t style)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	RT_ASSERT(widget != RT_NULL);

	widget->style = style;
	switch(style)
	{
		case RTGUI_BORDER_NONE:
			widget->border = 0;
			break;

		case RTGUI_BORDER_SIMPLE:
		case RTGUI_BORDER_UP:
		case RTGUI_BORDER_DOWN:
			widget->border = 1;
			break;

		case RTGUI_BORDER_STATIC:
		case RTGUI_BORDER_RAISE:
		case RTGUI_BORDER_SUNKEN:
		case RTGUI_BORDER_BOX:
		case RTGUI_BORDER_EXTRA:
			widget->border = 2;
			break;

		default:
			widget->border = 2;
			break;
	}
}
rt_uint16_t rtgui_widget_get_border(pvoid wdt)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	return widget->border;
}

/*
 * 这个函数移动控件和它的子控件到一个逻辑点
 */
void rtgui_widget_move_to_logic(pvoid wdt, int dx, int dy)
{
	rtgui_list_t* node;
	rtgui_widget_t* child;
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);

	if(widget == RT_NULL) return;

	rtgui_rect_moveto(&(widget->extent), dx, dy);

	/* move each child */
	if(RTGUI_IS_CONTAINER(widget))
	{
		rtgui_list_foreach(node, &(RTGUI_CONTAINER(widget)->children))
		{
			child = rtgui_list_entry(node, rtgui_widget_t, sibling);
			rtgui_widget_move_to_logic(child, dx, dy);
		}
	}
}

void rtgui_widget_set_event_handler(pvoid wdt, rtgui_event_handler_ptr handler)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	RT_ASSERT(widget != RT_NULL);

	RTGUI_WIDGET_EVENT_HANDLE(widget) = handler;
}

void rtgui_widget_get_rect(pvoid wdt, rtgui_rect_t *rect)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	RT_ASSERT(widget != RT_NULL);

	if(rect != RT_NULL)
	{
		rect->x1 = rect->y1 = 0;
		rect->x2 = widget->extent.x2 - widget->extent.x1;
		rect->y2 = widget->extent.y2 - widget->extent.y1;
	}
}

void rtgui_widget_get_position(pvoid wdt, rtgui_point_t *p)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	rtgui_rect_t rect,parent_rect;
	RT_ASSERT(widget != RT_NULL && RTGUI_WIDGET_PARENT(widget) != RT_NULL);

	rtgui_widget_get_rect(widget, &rect);
	rtgui_widget_rect_to_device(widget,&rect);
	rtgui_widget_get_rect(RTGUI_WIDGET_PARENT(widget), &parent_rect);
	rtgui_widget_rect_to_device(RTGUI_WIDGET_PARENT(widget),&parent_rect);

	p->x = rect.x1 - parent_rect.x1;
	p->y = rect.y1 - parent_rect.y1;
}

rt_uint16_t rtgui_widget_get_width(pvoid wdt)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	RT_ASSERT(widget != RT_NULL);

	return (widget->extent.x2 - widget->extent.x1);
}

rt_uint16_t rtgui_widget_get_height(pvoid wdt)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	RT_ASSERT(widget != RT_NULL);

	return (widget->extent.y2 - widget->extent.y1);
}

void rtgui_widget_set_onfocus(pvoid wdt, rtgui_event_handler_ptr handler)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	RT_ASSERT(widget != RT_NULL);

	widget->on_focus_in = handler;
}

void rtgui_widget_set_onunfocus(pvoid wdt, rtgui_event_handler_ptr handler)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	RT_ASSERT(widget != RT_NULL);

	widget->on_focus_out = handler;
}

void rtgui_widget_set_ondraw(pvoid wdt, rtgui_event_handler_ptr handler)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	RT_ASSERT(widget != RT_NULL);

	widget->on_draw = handler;
}

void rtgui_widget_set_onclick(pvoid wdt, rtgui_event_handler_ptr handler)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	RT_ASSERT(widget != RT_NULL);

	widget->on_mouseclick = handler;
}

void rtgui_widget_set_onkey(pvoid wdt, rtgui_event_handler_ptr handler)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	RT_ASSERT(widget != RT_NULL);

	widget->on_key = handler;
}

void rtgui_widget_set_onsize(pvoid wdt, rtgui_event_handler_ptr handler)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	RT_ASSERT(widget != RT_NULL);

	widget->on_size = handler;
}

void rtgui_widget_set_oncommand(pvoid wdt, rtgui_event_handler_ptr handler)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	RT_ASSERT(widget != RT_NULL);

	widget->on_command = handler;
}

/**
 * find wdt's toplevel widget and return toplevel focused
 */
pvoid rtgui_widget_get_focus(pvoid wdt)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	rtgui_widget_t *top;

	if(widget == RT_NULL)return RT_NULL;

	top = rtgui_widget_get_toplevel(widget);

	if(RTGUI_IS_CONTAINER(top))
		return RTGUI_CONTAINER(top)->focused;

	return RT_NULL;
}

/**
 * @brief Focuses the widget. The focused widget is the widget which can receive the keyboard events
 * @param widget a widget
 * @note The widget has to be attached to a top widget, otherwise it will have no effect
 */
void rtgui_widget_focus(pvoid wdt)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	rtgui_container_t *top;

	RT_ASSERT(widget != RT_NULL);

	if(!widget->parent || !widget->toplevel) return;

	if(!RTGUI_WIDGET_IS_FOCUSABLE(widget) || !RTGUI_WIDGET_IS_ENABLE(widget))
		return;

	/* gets root widget */
	top = RTGUI_CONTAINER(widget->toplevel);
	if(top->focused == widget)
		return; /* it's the same focused widget */

	/* unfocused the old widget */
	if(top->focused != RT_NULL)
		rtgui_widget_unfocus(top->focused);

	/* set widget as focused widget in parent link */
	top = RTGUI_CONTAINER(RTGUI_WIDGET_PARENT(widget));
	do{
		top->focused = widget;
		top = RTGUI_CONTAINER(RTGUI_WIDGET_PARENT(top));
	}while((top != RT_NULL) && !RTGUI_WIDGET_IS_HIDE(top));
	
	/* set widget as focused */
	widget->flag |= RTGUI_WIDGET_FLAG_FOCUS;
	
	/* invoke on focus in call back */
	if(widget->on_focus_in != RT_NULL)
		widget->on_focus_in(widget, RT_NULL);
}

/**
 * @brief Unfocused the widget
 * @param widget a widget
 */
void rtgui_widget_unfocus(pvoid wdt)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	if(widget == RT_NULL)return;

	if(!widget->toplevel || !RTGUI_WIDGET_IS_FOCUSED(widget))
		return;

	widget->flag &= ~RTGUI_WIDGET_FLAG_FOCUS;

	if(widget->on_focus_out != RT_NULL)
		widget->on_focus_out(widget, RT_NULL);

	RTGUI_CONTAINER(widget->toplevel)->focused = RT_NULL;

	if(RTGUI_IS_CONTAINER(widget))
	{
		rtgui_list_t *node;
		rtgui_list_foreach(node,&(RTGUI_CONTAINER(widget)->children))
		{
			rtgui_widget_t *child = rtgui_list_entry(node, rtgui_widget_t, sibling);
			if(RTGUI_WIDGET_IS_HIDE(child))continue;
			rtgui_widget_unfocus(child);
		}
	}
}

rt_bool_t rtgui_widget_contains_point(pvoid wdt, int x, int y)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	if(rtgui_rect_contains_point(&(widget->extent), x, y) == RT_EOK)
		return RT_TRUE;
	return RT_FALSE;
}

void rtgui_widget_point_to_device(pvoid wdt, rtgui_point_t* point)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	RT_ASSERT(widget != RT_NULL);

	if(point != RT_NULL)
	{
		point->x += widget->extent.x1;
		point->y += widget->extent.y1;
	}
}

void rtgui_widget_rect_to_device(pvoid wdt, rtgui_rect_t* rect)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	RT_ASSERT(widget != RT_NULL);

	if(rect != RT_NULL)
	{
		rect->x1 += widget->extent.x1;
		rect->x2 += widget->extent.x1;

		rect->y1 += widget->extent.y1;
		rect->y2 += widget->extent.y1;
	}
}

void rtgui_widget_point_to_logic(pvoid wdt, rtgui_point_t* point)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	RT_ASSERT(widget != RT_NULL);

	if(point != RT_NULL)
	{
		point->x -= widget->extent.x1;
		point->y -= widget->extent.y1;
	}
}

void rtgui_widget_rect_to_logic(pvoid wdt, rtgui_rect_t* rect)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	RT_ASSERT(widget != RT_NULL);

	if(rect != RT_NULL)
	{
		rect->x1 -= widget->extent.x1;
		rect->x2 -= widget->extent.x1;

		rect->y1 -= widget->extent.y1;
		rect->y2 -= widget->extent.y1;
	}
}

pvoid rtgui_widget_get_toplevel(pvoid wdt)
{
	rtgui_widget_t *top;
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);

	RT_ASSERT(widget != RT_NULL);

	if(widget->toplevel)
		return (pvoid)(widget->toplevel);

	top = widget;
	/* get the top widget */
	while(top->parent != RT_NULL) top = top->parent;

	/* set top */
	widget->toplevel = top;

	return top;
}

rt_bool_t rtgui_widget_event_handler(pvoid wdt, rtgui_event_t* event)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);

	switch(event->type)
	{
		case RTGUI_EVENT_PAINT:
			if(widget->on_draw != RT_NULL) return widget->on_draw(widget, event);
			return RT_FALSE;

		case RTGUI_EVENT_KBD:
			if(widget->on_key != RT_NULL) return widget->on_key(widget, event);
			return RT_TRUE;

		case RTGUI_EVENT_MOUSE_BUTTON:
			if(widget->on_mouseclick != RT_NULL) return widget->on_mouseclick(widget, event);
			return RT_TRUE;

		case RTGUI_EVENT_COMMAND:
			if(widget->on_command != RT_NULL) return widget->on_command(widget, event);
			return RT_TRUE;

		case RTGUI_EVENT_RESIZE:
			if(widget->on_size != RT_NULL) return widget->on_size(widget, event);
			return RT_TRUE;
		default:
			return RT_FALSE;
	}
}

/*
 * This function updates the clip info of widget
 */
void rtgui_widget_update_clip(pvoid wdt)
{
	rtgui_list_t *node;
	rtgui_widget_t *parent;
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);

	/* no widget or widget is hide, no update clip */
	if(widget == RT_NULL || RTGUI_WIDGET_IS_HIDE(widget)) return;

	parent = widget->parent;

	/* if there is no parent, do not update clip (please use toplevel widget API) */
	if(parent == RT_NULL)
	{
		rtgui_toplevel_update_clip(widget);
		return;
	}

	/* reset clip to extent */
	rtgui_region_reset(&(widget->clip), &(widget->extent));

	/* limit widget extent in screen extent */
	{
		rtgui_rect_t scrn_rc;
		rtgui_gdev_get_rect(hw_driver,&scrn_rc);
		rtgui_region_intersect_rect(&(widget->clip), &(widget->clip), &scrn_rc);
	}

	/* get the no transparent parent */
	while(parent != RT_NULL && parent->flag & RTGUI_WIDGET_FLAG_TRANSPARENT)
	{
		parent = parent->parent;
	}

	if(parent != RT_NULL)
	{
		/* subtract widget clip in parent clip */
		if(!(widget->flag & RTGUI_WIDGET_FLAG_TRANSPARENT))
		{
			rtgui_region_subtract_rect(&(parent->clip), &(parent->clip),
			                           &(widget->extent));
		}
	}

	/*
	 * note: since the layout widget introduction, the sibling widget will not
	 * intersect.
	 */

	/* if it's a container object, update the clip info of children */
	if(RTGUI_IS_CONTAINER(widget))
	{
		rtgui_widget_t* child;
		rtgui_list_foreach(node, &(RTGUI_CONTAINER(widget)->children))
		{
			child = rtgui_list_entry(node, rtgui_widget_t, sibling);
			if(RTGUI_WIDGET_IS_HIDE(child)) continue;
			rtgui_widget_update_clip(child);
		}
	}
}

/*
 * 更新多个控件重叠时的clip剪切情况,将子控件topwdt作为最顶端控件,
 * 它的兄弟控件则相应的剪切掉topwdt区域,使它们从位置上被topwdt覆盖
 * 调用该函数时,应该保证topwdt控件是wdt控件下的一个子控件.
 */
void rtgui_widget_update_clip_pirate(pvoid wdt,pvoid topwdt)
{
	rtgui_list_t *node;
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	rtgui_panel_t *panel = rtgui_panel_get();
	rtgui_rect_t wdt_rect;

	if(widget == RT_NULL || RTGUI_WIDGET_IS_HIDE(widget)) return;

	wdt_rect = widget->extent;

	if(wdt == topwdt)
	{
		/* the same as oneself */
		rtgui_widget_update_clip(widget);
		return;
	}
	else
	{
		/* different widget */
		if(rtgui_rect_is_intersect(&RTGUI_WIDGET_EXTENT(topwdt), &wdt_rect) == RT_EOK)
		{
			rtgui_rect_intersect(&RTGUI_WIDGET_EXTENT(topwdt), &wdt_rect);/* calculate overlap */
			rtgui_region_reset(&(widget->clip), &(widget->extent));
			rtgui_region_subtract_rect(&(widget->clip), &(widget->clip),&wdt_rect);/* subtract overlap area */
		}
		else
		{
			rtgui_region_reset(&(widget->clip), &(widget->extent));
		}
	}

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

	{
		rtgui_rect_t scrn_rc;
		rtgui_gdev_get_rect(hw_driver,&scrn_rc);
		rtgui_region_intersect_rect(&(widget->clip), &(widget->clip), &scrn_rc);
	}

	if(RTGUI_IS_CONTAINER(widget))
	{
		/* update all child widget */
		rtgui_list_foreach(node, &(RTGUI_CONTAINER(widget)->children))
		{
			rtgui_widget_t *child = rtgui_list_entry(node, rtgui_widget_t, sibling);
			if(RTGUI_WIDGET_IS_HIDE(child))continue;

			rtgui_region_subtract_rect(&(widget->clip), &(widget->clip),&(child->extent));
			rtgui_widget_update_clip_pirate(child,topwdt);
		}
	}
}

void rtgui_widget_show(pvoid wdt)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	/* there is no parent or the parent is hide, no show at all */
	if(RTGUI_WIDGET_PARENT(widget) == RT_NULL || RTGUI_WIDGET_IS_HIDE(RTGUI_WIDGET_PARENT(widget))) return;

	/* update the clip info of widget */
	RTGUI_WIDGET_SHOW(widget);

	rtgui_widget_update_clip(RTGUI_WIDGET_PARENT(widget));
	rtgui_widget_update(widget);
}

void rtgui_widget_hide(pvoid wdt)
{
	rtgui_rect_t rect;
	rtgui_widget_t  *widget = RTGUI_WIDGET(wdt);

	/* hide this widget */
	RTGUI_WIDGET_HIDE(widget);

	if(widget->parent != RT_NULL)
	{
		rtgui_widget_t *parent;

		rect = widget->extent;
		parent = widget->parent;
		/* get the no transparent parent */
		while(parent != RT_NULL && parent->flag & RTGUI_WIDGET_FLAG_TRANSPARENT)
		{
			parent = parent->parent;
		}

		/* union widget rect */
		rtgui_region_union_rect(&(parent->clip), &(parent->clip), &rect);
		/* rtgui_widget_update_clip(parent); */

		rtgui_widget_update(parent);
	}
}

rtgui_color_t rtgui_widget_get_parent_fc(pvoid wdt)
{
	rtgui_widget_t* parent;
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);

	/* get parent widget */
	parent = widget->parent;
	while(parent->parent != RT_NULL && (RTGUI_WIDGET_FLAG(parent) & RTGUI_WIDGET_FLAG_TRANSPARENT))
		parent = parent->parent;

	/* get parent's color */
	if(parent != RT_NULL)
		return RTGUI_WIDGET_FC(parent);

	return RTGUI_WIDGET_FC(widget);
}

rtgui_color_t rtgui_widget_get_parent_bc(pvoid wdt)
{
	rtgui_widget_t* parent;
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);

	/* get parent widget */
	parent = widget->parent;
	while(parent->parent != RT_NULL && (RTGUI_WIDGET_FLAG(parent) & RTGUI_WIDGET_FLAG_TRANSPARENT))
		parent = parent->parent;

	/* get parent's color */
	if(parent != RT_NULL)
		return RTGUI_WIDGET_BC(parent);

	return RTGUI_WIDGET_BC(widget);
}

void rtgui_widget_update(pvoid wdt)
{
	struct rtgui_event_paint paint;
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);

	if(RTGUI_WIDGET_IS_HIDE(widget)) return;
	
	RTGUI_EVENT_PAINT_INIT(&paint);
	paint.wid = RT_NULL;

	RT_ASSERT(widget != RT_NULL);

	if(RTGUI_WIDGET_EVENT_HANDLE(widget) != RT_NULL)
	{
		RTGUI_WIDGET_EVENT_CALL(widget, &paint.parent);
	}
}

/* get the next sibling of widget */
rtgui_widget_t* rtgui_widget_get_next_sibling(pvoid wdt)
{
	rtgui_widget_t* sibling = RT_NULL;
	rtgui_widget_t* widget = RTGUI_WIDGET(wdt);

	if(widget->sibling.next != RT_NULL)
	{
		sibling = rtgui_list_entry(widget->sibling.next, rtgui_widget_t, sibling);
	}

	return sibling;
}

/* get the prev sibling of widget */
rtgui_widget_t* rtgui_widget_get_prev_sibling(pvoid wdt)
{
	rtgui_list_t* node;
	rtgui_widget_t *sibling, *parent;
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);

	node = RT_NULL;
	sibling = RT_NULL;
	parent = widget->parent;
	if(parent != RT_NULL)
	{
		rtgui_list_foreach(node, &(RTGUI_CONTAINER(parent)->children))
		{
			if(node->next == &(widget->sibling))
				break;
		}
	}

	if(node != RT_NULL)
		sibling = rtgui_list_entry(node, rtgui_widget_t, sibling);

	return sibling;
}
