/*
 * File      : radiobox.c
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
#include <rtgui/widgets/radiobox.h>

static rt_bool_t rtgui_radiobox_unfocus(pvoid wdt, rtgui_event_t* event);

static void _rtgui_radiobox_constructor(rtgui_radiobox_t *rbox)
{
	/* init widget and set event handler */
	RTGUI_WIDGET_FLAG(rbox) |= RTGUI_WIDGET_FLAG_FOCUSABLE;
	RTGUI_WIDGET_TEXTALIGN(rbox) = RTGUI_ALIGN_LEFT | RTGUI_ALIGN_CENTER_VERTICAL;
	rtgui_widget_set_onunfocus(rbox, rtgui_radiobox_unfocus);
	rtgui_widget_set_event_handler(rbox, rtgui_radiobox_event_handler);
	rbox->bond = RT_NULL;
}

static void _rtgui_radiobox_destructor(rtgui_radiobox_t *rbox)
{
	if(rbox->name != RT_NULL)
	{
		rt_free(rbox->name);
		rbox->name = RT_NULL;
	}
	if(rbox->bond != RT_NULL)
	{
		/* in same group, free once only. */
		rbox->bond->item_used--; /* Times cited subtract 1. */
		if(rbox->bond->item_used==0)
		{
			/* when and only when there is no user. */
			if(rbox->bond->rboxs != RT_NULL)
			{
				rt_free(rbox->bond->rboxs);
				rbox->bond->rboxs = RT_NULL;
			}
			rbox->bond->bind_var = RT_NULL;
			rt_free(rbox->bond);
			rbox->bond = RT_NULL;
		}
	}
}

DEFINE_CLASS_TYPE(radiobox, "radiobox",
                  RTGUI_WIDGET_TYPE,
                  _rtgui_radiobox_constructor,
                  _rtgui_radiobox_destructor,
                  sizeof(struct rtgui_radiobox));

/* create a radiobox widget */
rtgui_radiobox_t* rtgui_radiobox_create(pvoid parent, const char* name,
                                        int left, int top, int w, int h, rtgui_rb_bond_t* bond)
{
	rtgui_container_t *container;
	rtgui_radiobox_t* rbox;

	RT_ASSERT(parent != RT_NULL);
	container = RTGUI_CONTAINER(parent);

	rbox = rtgui_widget_create(RTGUI_RADIOBOX_TYPE);
	if(rbox != RT_NULL)
	{
		rtgui_rect_t rect;

		rtgui_widget_get_rect(container,&rect);
		rtgui_widget_rect_to_device(container,&rect);
		rect.x1 += left;
		rect.y1 += top;
		rect.x2 = rect.x1+w;
		rect.y2 = rect.y1+h;
		rtgui_widget_set_rect(rbox,&rect);
		rtgui_container_add_child(container, rbox);

		rbox->name = rt_strdup(name);

		if(bond == RT_NULL)
		{
			/* if radiobox group is null, create it right now. */
			rtgui_rb_bond_t *grp = rt_malloc(sizeof(rtgui_rb_bond_t));
			if(grp != RT_NULL)
			{
				grp->item_used = 0;
				grp->item_count = 0;
				grp->item_sel = 0;
				grp->rboxs = rt_malloc(sizeof(rtgui_radiobox_t*));
				grp->bind_var = RT_NULL;
				grp->on_item = RT_NULL;
				bond = grp;
			}
		}

		if(bond != RT_NULL)
		{
			bond->rboxs = rt_realloc(bond->rboxs,
			                         sizeof(rtgui_radiobox_t*)*(bond->item_count+1));
			*(bond->rboxs+bond->item_count) = rbox;
			bond->item_count++;
			bond->item_used++; /* Times cited add 1. */
			rbox->bond = bond;
		}
	}

	return rbox;
}

void rtgui_radiobox_destroy(rtgui_radiobox_t* rbox)
{
	rtgui_widget_destroy(rbox);
}

static const rt_uint8_t radio_unchecked_byte[] =
{
	0x0f, 0x00, 0x30, 0xc0, 0x40, 0x20,
	0x40, 0x20,	0x80, 0x10,	0x80, 0x10,
	0x80, 0x10,	0x80, 0x10,	0x40, 0x20,
	0x40, 0x20,	0x30, 0xc0,	0x0f, 0x00,
};
static const rt_uint8_t radio_checked_byte[] =
{
	0x0f, 0x00, 0x30, 0xc0, 0x40, 0x20,
	0x40, 0x20, 0x86, 0x10, 0x8f, 0x10,
	0x8f, 0x10, 0x86, 0x10, 0x40, 0x20,
	0x40, 0x20, 0x30, 0xc0, 0x0f, 0x00,
};

void rtgui_radiobox_ondraw(rtgui_radiobox_t* rbox)
{
	rtgui_rect_t rect,item_rect;
	rtgui_rb_bond_t *bond;
	rtgui_dc_t* dc;
	rt_base_t len=0,w;

	RT_ASSERT(rbox != RT_NULL);

	bond = rbox->bond;
	
	/* begin drawing */
	dc = rtgui_dc_begin_drawing(rbox);
	if(dc == RT_NULL)return;
	
	/* get widget rect */
	rtgui_widget_get_rect(rbox, &rect);
	rtgui_dc_fill_rect(dc,&rect);

	item_rect = rect;
	/* draw radio */
	item_rect.x2 = item_rect.x1+RADIO_BOX_W;
	item_rect.y2 = item_rect.y1+RADIO_BOX_H;
	rtgui_rect_moveto_align(&rect, &item_rect, RTGUI_WIDGET_TEXTALIGN(rbox));
	rtgui_rect_inflate(&item_rect, -2);
	
	RTGUI_DC_FC(dc) = theme.blankspace;

	w = RC_W(item_rect)/2;
	rtgui_dc_fill_circle(dc, item_rect.x1+w, item_rect.y1+w, w+1);
	rtgui_rect_inflate(&item_rect, 2);
	RTGUI_DC_FC(dc) = theme.foreground;
	if(*(bond->rboxs + bond->item_sel) == rbox)
	{
		rtgui_dc_draw_word(dc, item_rect.x1, item_rect.y1, RADIO_BOX_H, radio_checked_byte);
	}
	else
	{
		rtgui_dc_draw_word(dc, item_rect.x1, item_rect.y1, RADIO_BOX_H, radio_unchecked_byte);
	}

	/* draw text */
	rtgui_font_get_string_rect(RTGUI_WIDGET_FONT(rbox), rbox->name, &item_rect);
	len = RC_W(item_rect);
	rtgui_rect_moveto_align(&rect, &item_rect, RTGUI_WIDGET_TEXTALIGN(rbox));
	item_rect.x1 += RADIO_BOX_H + RTGUI_MARGIN;
	item_rect.x2 = item_rect.x1 + len;
	rtgui_dc_draw_text(dc, rbox->name, &item_rect);

	if(*(bond->rboxs + bond->item_sel) == rbox)
	{
		/* draw focus rect */
		if(RTGUI_WIDGET_IS_FOCUSED(RTGUI_WIDGET(rbox)))
			rtgui_dc_draw_focus_rect(dc, &item_rect);
	}
	rtgui_dc_end_drawing(dc);
}

static void rtgui_radiobox_onmouse(rtgui_radiobox_t* rbox, struct rtgui_event_mouse* event)
{
	RT_ASSERT(rbox != RT_NULL);
	RT_ASSERT(event  != RT_NULL);
	
	//if(!RTGUI_WIDGET_IS_ENABLE(rbox)) return;
		
	if(event->button & RTGUI_MOUSE_BUTTON_DOWN && event->button & RTGUI_MOUSE_BUTTON_LEFT)
	{
		rtgui_rect_t rect;

		/* focus widgets */
		rtgui_widget_focus(rbox);

		/* get widget physical rect */
		rtgui_widget_get_rect(rbox, &rect);
		rtgui_widget_rect_to_device(rbox, &rect);
		if(rtgui_rect_contains_point(&rect, event->x, event->y) == RT_EOK)
		{
			rtgui_rb_bond_t *bond = rbox->bond;
			rtgui_radiobox_t *radio;
			int old_item = bond->item_sel;
			int i;
			for(i=bond->item_count-1; i>=0; i--)
			{
				/* iff one item , it break until 0, it's right */
				radio = *(bond->rboxs+i);
				if(radio == rbox) break;
			}
			bond->item_sel = i;
			if(bond->bind_var != RT_NULL)
				*(bond->bind_var) = bond->item_sel;

			if(bond != RT_NULL && bond->on_item != RT_NULL)
			{
				if(old_item != bond->item_sel)
					bond->on_item(bond, (rtgui_event_t*)event);
			}

			/* update rbox widget */
			rtgui_radiobox_ondraw(*(bond->rboxs + old_item));
			rtgui_radiobox_ondraw(*(bond->rboxs + i));
		}
	}
}

rt_bool_t rtgui_radiobox_event_handler(pvoid wdt, rtgui_event_t* event)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	rtgui_radiobox_t *rbox = RTGUI_RADIOBOX(wdt);
	rtgui_rb_bond_t *bond;

	RT_ASSERT(rbox != RT_NULL);

	bond = rbox->bond;

	switch(event->type)
	{
		case RTGUI_EVENT_PAINT:
			if(widget->on_draw != RT_NULL)
				widget->on_draw(widget, event);
			else
			{
				rtgui_radiobox_ondraw(rbox);
			}
			return RT_FALSE;

		case RTGUI_EVENT_KBD:
			if(widget->on_key != RT_NULL)
				widget->on_key(widget, event);
			else
			{
				struct rtgui_event_kbd *e = (struct rtgui_event_kbd*)event;

				/* set focused */
				rtgui_widget_focus(rbox);
				if(!(RTGUI_KBD_IS_UP(e))) return RT_FALSE;

				if(e->key == RTGUIK_UP || e->key == RTGUIK_LEFT)
				{
					if(bond->item_sel > 0)
					{
						if(bond->bind_var != RT_NULL)
							*(bond->bind_var) = bond->item_sel - 1;
						rtgui_rb_bond_set_sel(bond, bond->item_sel - 1);
					}
				}
				else if(e->key == RTGUIK_DOWN || e->key == RTGUIK_RIGHT)
				{
					if(bond->item_sel < bond->item_count - 1)
					{
						if(bond->bind_var != RT_NULL)
							*(bond->bind_var) = bond->item_sel +1;
						rtgui_rb_bond_set_sel(bond, bond->item_sel + 1);
					}
				}
			}
			return RT_TRUE;

		case RTGUI_EVENT_MOUSE_BUTTON:
			if(widget->on_mouseclick != RT_NULL)
				widget->on_mouseclick(widget, event);
			else
			{
				rtgui_radiobox_onmouse(rbox, (struct rtgui_event_mouse*)event);
			}
			return RT_TRUE;
		default:
			return RT_FALSE;
	}
}

rtgui_rb_bond_t* rtgui_radiobox_create_group(void)
{
	rtgui_rb_bond_t *bond=RT_NULL;

	bond = rt_malloc(sizeof(rtgui_rb_bond_t));
	if(bond != RT_NULL)
	{
		bond->item_used = 0;
		bond->item_count = 0;
		bond->item_sel = 0;
		bond->rboxs = rt_malloc(sizeof(rtgui_radiobox_t*));
		bond->bind_var = RT_NULL;
		bond->on_item = RT_NULL;
	}

	return bond;
}

rtgui_rb_bond_t* rtgui_radiobox_get_bond(rtgui_radiobox_t* rbox)
{
	RT_ASSERT(rbox != RT_NULL);

	return rbox->bond;
}

/* bind a external variable */
void rtgui_rb_bond_bind(rtgui_rb_bond_t *bond, rt_uint32_t *var)
{
	if(bond != RT_NULL)
	{
		bond->bind_var = var;
	}
}

/* terminate binding relation */
void rtgui_rb_bond_unbind(rtgui_rb_bond_t *bond)
{
	if(bond != RT_NULL)
	{
		bond->bind_var = RT_NULL;
	}
}

/* set selection in group */
void rtgui_rb_bond_set_sel(rtgui_rb_bond_t* bond, int sel)
{
	rt_uint16_t old;

	if(sel == bond->item_sel) return;

	old = bond->item_sel;
	if(sel >= 0 && sel < bond->item_count)
		bond->item_sel = sel;
	else
		bond->item_sel = 0;

	if(bond->bind_var != RT_NULL)
		*(bond->bind_var) = bond->item_sel;

	/* update rbox widget */
	rtgui_radiobox_ondraw(*(bond->rboxs + old));
	rtgui_radiobox_ondraw(*(bond->rboxs + sel));
}

int rtgui_rb_bond_get_sel(rtgui_rb_bond_t* bond)
{
	if(bond != RT_NULL)
	{
		return bond->item_sel;
	}

	return 0;
}

void rtgui_rb_bond_set_onitem(rtgui_rb_bond_t* bond, rtgui_event_handler_ptr func)
{
	if(bond != RT_NULL)
	{
		bond->on_item = func;
	}
}

static rt_bool_t rtgui_radiobox_unfocus(pvoid wdt, rtgui_event_t* event)
{
	rtgui_rect_t rect, item_rect;
	rtgui_radiobox_t *rbox = RTGUI_RADIOBOX(wdt);
	rtgui_dc_t *dc;
	rt_base_t len=0;

	RT_ASSERT(wdt != RT_NULL);

	dc = rtgui_dc_begin_drawing(wdt);
	if(dc == RT_NULL)return RT_FALSE;

	rtgui_widget_get_rect(rbox, &rect);

	item_rect = rect;
	rtgui_rect_moveto_align(&rect, &item_rect, RTGUI_WIDGET_TEXTALIGN(rbox));

	/* draw text */
	rtgui_font_get_string_rect(RTGUI_WIDGET_FONT(rbox), rbox->name, &item_rect);
	len = RC_W(item_rect);
	rtgui_rect_moveto_align(&rect, &item_rect, RTGUI_WIDGET_TEXTALIGN(rbox));
	item_rect.x1 += RADIO_BOX_H + RTGUI_MARGIN;
	item_rect.x2 = item_rect.x1 + len;

	if(!RTGUI_WIDGET_IS_FOCUSED(rbox))
	{
		/* clear fouce rect */
		rtgui_color_t color;
		color = RTGUI_DC_FC(dc);
		RTGUI_DC_FC(dc) = theme.background;
		rtgui_dc_draw_focus_rect(dc,&item_rect);
		RTGUI_DC_FC(dc) = color;
	}

	rtgui_dc_end_drawing(dc);

	return RT_EOK;
}
