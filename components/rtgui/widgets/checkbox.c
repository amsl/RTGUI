/*
 * File      : checkbox.c
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
#include <rtgui/widgets/checkbox.h>

static rt_bool_t rtgui_checkbox_onunfocus(pvoid wdt, rtgui_event_t* event);

static void _rtgui_checkbox_constructor(rtgui_checkbox_t *checkbox)
{
	/* init widget and set event handler */
	RTGUI_WIDGET_FLAG(checkbox) |= RTGUI_WIDGET_FLAG_FOCUSABLE;
	if(theme.style == RTGUI_BORDER_UP)
		rtgui_widget_set_border_style(checkbox,RTGUI_BORDER_DOWN);
	else if(theme.style == RTGUI_BORDER_EXTRA)
		rtgui_widget_set_border_style(checkbox,RTGUI_BORDER_SUNKEN);
	rtgui_widget_set_event_handler(checkbox, rtgui_checkbox_event_handler);
	rtgui_widget_set_onunfocus(checkbox, rtgui_checkbox_onunfocus);
	/* set status */
	checkbox->value = 0;

	/* set default gc */
	RTGUI_WIDGET_TEXTALIGN(checkbox) = RTGUI_ALIGN_LEFT | RTGUI_ALIGN_CENTER_VERTICAL;
}

static void _rtgui_checkbox_destructor(rtgui_checkbox_t *checkbox)
{

}

DEFINE_CLASS_TYPE(checkbox, "checkbox",
                  RTGUI_LABEL_TYPE,
                  _rtgui_checkbox_constructor,
                  _rtgui_checkbox_destructor,
                  sizeof(struct rtgui_checkbox));

rtgui_checkbox_t* rtgui_checkbox_create(pvoid parent,const char* text, rt_bool_t checked, int left,int top)
{
	rtgui_container_t *container;
	rtgui_checkbox_t* box;

	RT_ASSERT(parent != RT_NULL);
	container = RTGUI_CONTAINER(parent);

	box = rtgui_widget_create(RTGUI_CHECKBOX_TYPE);
	if(box != RT_NULL)
	{
		rtgui_rect_t rect,text_rect;

		rtgui_widget_get_rect(container, &rect);
		rtgui_widget_rect_to_device(container,&rect);
		/* set default rect */
		rtgui_font_get_string_rect(RTGUI_WIDGET_FONT(box), text, &text_rect);
		text_rect.x2 += RTGUI_WIDGET_BORDER_SIZE(box) + 5 + (RTGUI_WIDGET_BORDER_SIZE(box) << 1);
		text_rect.y2 += (RTGUI_WIDGET_BORDER_SIZE(box) << 1);

		rect.x1 += left;
		rect.y1 += top;
		rect.x2 = rect.x1 + CHECK_BOX_W + RC_W(text_rect)+5;
		rect.y2 = rect.y1 + RC_H(text_rect);

		rtgui_widget_set_rect(box, &rect);

		rtgui_checkbox_set_text(box, text);

		if(checked == RT_TRUE)
			box->value = 1;
		else
			box->value = 0;
		rtgui_container_add_child(container, box);
	}

	return box;
}

void rtgui_checkbox_destroy(rtgui_checkbox_t* box)
{
	rtgui_widget_destroy(box);
}

static const rt_uint8_t checked_byte[7] = {0x02, 0x06, 0x8E, 0xDC, 0xF8, 0x70, 0x20};

void rtgui_checkbox_ondraw(rtgui_checkbox_t* checkbox)
{
	rtgui_rect_t rect, box_rect;
	rtgui_dc_t* dc;
	rtgui_color_t color;

	RT_ASSERT(checkbox != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(checkbox);
	if(dc == RT_NULL)return;

	/* get rect */
	rtgui_widget_get_rect(checkbox, &rect);
	RTGUI_DC_BC(dc) = theme.background;
	/* fill rect */
	rtgui_dc_fill_rect(dc,&rect);

	if(RTGUI_WIDGET_IS_FOCUSED(checkbox))
	{
		/* only draw focus rect surround string */
		rtgui_rect_t tmp_rect;
		char *string = rtgui_label_get_text(RTGUI_LABEL(checkbox));
		rtgui_font_get_string_rect(RTGUI_WIDGET_FONT(checkbox), string, &tmp_rect);

		rtgui_rect_moveto(&tmp_rect,rect.x1+CHECK_BOX_W+7, rect.y1);
		tmp_rect.x1 -= 1;
		tmp_rect.x2 += 2;
		tmp_rect.y2 = rect.y2-1;
		rtgui_dc_draw_focus_rect(dc,&tmp_rect);
	}

	/* draw check box */
	box_rect.x1 = 0;
	box_rect.y1 = 0;
	box_rect.x2 = CHECK_BOX_W;
	box_rect.y2 = CHECK_BOX_H;
	rtgui_rect_moveto_align(&rect, &box_rect, RTGUI_ALIGN_CENTER_VERTICAL);
	box_rect.x1 += 2;
	box_rect.x2 += 2;

	rtgui_dc_draw_border(dc, &box_rect, RTGUI_WIDGET_BORDER_STYLE(checkbox));
	rtgui_rect_inflate(&box_rect, -2);

	color = RTGUI_DC_BC(dc);
	RTGUI_DC_BC(dc) = theme.blankspace;
	rtgui_dc_fill_rect(dc, &box_rect);
	RTGUI_DC_BC(dc) = color;

	if(checkbox->value)
	{
		color = RTGUI_DC_FC(dc);
		RTGUI_DC_FC(dc) = ForestGreen;
		rtgui_dc_draw_byte(dc,box_rect.x1 + 1, box_rect.y1 + 1, 7, checked_byte);
		RTGUI_DC_FC(dc) = color;
	}
	/* draw text */
	rect.x1 += RC_H(rect) - 4 + 5;
	rtgui_dc_draw_text(dc, rtgui_label_get_text(RTGUI_LABEL(checkbox)), &rect);

	rtgui_dc_end_drawing(dc);
}

rt_bool_t rtgui_checkbox_event_handler(pvoid wdt, rtgui_event_t* event)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	rtgui_checkbox_t* box = RTGUI_CHECKBOX(wdt);

	switch(event->type)
	{
	case RTGUI_EVENT_PAINT:
		if(widget->on_draw != RT_NULL)
		{
			return widget->on_draw(widget, event);
		}
		else
			rtgui_checkbox_ondraw(box);
		break;

	case RTGUI_EVENT_MOUSE_BUTTON:
	{
		struct rtgui_event_mouse* emouse = (struct rtgui_event_mouse*)event;
		if(RTGUI_WIDGET_IS_ENABLE(widget) && !RTGUI_WIDGET_IS_HIDE(widget))
		{
			if(emouse->button & RTGUI_MOUSE_BUTTON_LEFT && emouse->button & RTGUI_MOUSE_BUTTON_UP)
			{
				if(box->value)
				{
					/* check it */
					box->value = 0;
				}
				else
				{
					/* un-check it */
					box->value = 1;
				}
			}
			else if(emouse->button & RTGUI_MOUSE_BUTTON_LEFT && emouse->button & RTGUI_MOUSE_BUTTON_DOWN)
			{
				/* set focus */
				rtgui_widget_focus(widget);
			}
			/* draw checkbox */
			rtgui_checkbox_ondraw(box);

			/* call user callback */
			if(widget->on_mouseclick != RT_NULL)
			{
				return widget->on_mouseclick(widget, event);
			}
		}

		return RT_TRUE;
	}
	default:
		return RT_TRUE;
	}

	return RT_FALSE;
}

void rtgui_checkbox_set_checked(rtgui_checkbox_t* checkbox, rt_bool_t checked)
{
	RT_ASSERT(checkbox != RT_NULL);
	if(checked == RT_TRUE)
		checkbox->value = 1;
	else
		checkbox->value = 0;

}

rt_bool_t rtgui_checkbox_get_checked(rtgui_checkbox_t* checkbox)
{
	RT_ASSERT(checkbox != RT_NULL);

	if(checkbox->value)
		return RT_TRUE;

	return RT_FALSE;
}

static rt_bool_t rtgui_checkbox_onunfocus(pvoid wdt, rtgui_event_t* event)
{
	rtgui_rect_t rect;
	rtgui_checkbox_t* box = RTGUI_CHECKBOX(wdt);
	rtgui_dc_t *dc;

	RT_ASSERT(box != RT_NULL);

	dc = rtgui_dc_begin_drawing(box);
	if(dc == RT_NULL)return RT_FALSE;

	rtgui_widget_get_rect(box, &rect);

	if(!RTGUI_WIDGET_IS_FOCUSED(box))
	{
		/* clear focus rect */
		rtgui_rect_t tmp_rect;
		rtgui_color_t color;
		rtgui_font_get_string_rect(RTGUI_WIDGET_FONT(box), rtgui_label_get_text(RTGUI_LABEL(box)), &tmp_rect);

		rtgui_rect_moveto(&tmp_rect,rect.x1+CHECK_BOX_W+7, rect.y1);
		tmp_rect.x1 -= 1;
		tmp_rect.x2 += 2;
		tmp_rect.y2 = rect.y2-1;

		color = RTGUI_DC_FC(dc);
		RTGUI_DC_FC(dc) = RTGUI_DC_BC(dc);
		rtgui_dc_draw_focus_rect(dc,&tmp_rect);
		RTGUI_DC_FC(dc) = color;
	}

	rtgui_dc_end_drawing(dc);

	return RT_TRUE;
}

void rtgui_checkbox_set_text(rtgui_checkbox_t *box, const char* text)
{
	RT_ASSERT(box != RT_NULL);

	if(RTGUI_LABEL(box)->text != RT_NULL)
	{
		/* release old text memory */
		rt_free(RTGUI_LABEL(box)->text);
		RTGUI_LABEL(box)->text = RT_NULL;
	}

	if(text != RT_NULL)
		RTGUI_LABEL(box)->text = rt_strdup(text);
	else
		RTGUI_LABEL(box)->text = RT_NULL;
}
