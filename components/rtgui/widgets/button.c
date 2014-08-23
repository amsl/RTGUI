/*
 * File      : button.c
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
 */
#include <rtgui/widgets/button.h>

static rt_bool_t rtgui_button_onfocus(pvoid wdt, rtgui_event_t* event);
static rt_bool_t rtgui_button_onunfocus(pvoid wdt, rtgui_event_t* event);

static void _rtgui_button_constructor(rtgui_button_t *button)
{
	/* init widget and set event handler */
	RTGUI_WIDGET_FLAG(button) |= RTGUI_WIDGET_FLAG_FOCUSABLE;
	rtgui_widget_set_event_handler(button, rtgui_button_event_handler);
	rtgui_widget_set_onfocus(button, rtgui_button_onfocus);
	rtgui_widget_set_onunfocus(button, rtgui_button_onunfocus);
	/* un-press button */
	button->flag = 0;

	/* set flag and click event handler */
	button->image = RT_NULL;
	button->click = RT_NULL;

	/* set gc */
	RTGUI_WIDGET_FC(button) = theme.foreground;
	RTGUI_WIDGET_BC(button) = theme.background;
	RTGUI_WIDGET_TEXTALIGN(button) = RTGUI_ALIGN_CENTER;
}

static void _rtgui_button_destructor(rtgui_button_t *button)
{
	if(button->image != RT_NULL)
	{
		rtgui_image_destroy(button->image);
		button->image = RT_NULL;
	}
}

DEFINE_CLASS_TYPE(button, "button",
                  RTGUI_LABEL_TYPE,
                  _rtgui_button_constructor,
                  _rtgui_button_destructor,
                  sizeof(struct rtgui_button));

rtgui_button_t* rtgui_button_create(pvoid parent, const char* text,int left, int top, int w, int h)
{
	rtgui_container_t *container;
	rtgui_button_t* btn;

	RT_ASSERT(parent != RT_NULL);
	container = RTGUI_CONTAINER(parent);

	btn = rtgui_widget_create(RTGUI_BUTTON_TYPE);
	if(btn != RT_NULL)
	{
		rtgui_rect_t rect;
		/* set default rect */
		rtgui_font_get_string_rect(RTGUI_WIDGET_FONT(btn), text, &rect);
	
		rtgui_label_set_text(RTGUI_LABEL(btn), text);
	
		rtgui_widget_get_rect(container, &rect);
		rtgui_widget_rect_to_device(container,&rect);
		rect.x1 += left;
		rect.y1 += top;
		rect.x2 = rect.x1 + w;
		rect.y2 = rect.y1 + h;
		rtgui_widget_set_rect(btn, &rect);
	
		rtgui_container_add_child(container, btn);
	}

	return btn;
}

void rtgui_button_destroy(rtgui_button_t* btn)
{
	rtgui_widget_destroy(btn);
}

/* widget drawing */
void rtgui_button_ondraw(rtgui_button_t* btn)
{
	/* draw button */
	rtgui_rect_t rect;
	rtgui_dc_t* dc;

	RT_ASSERT(btn != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(btn);
	if(dc == RT_NULL)return;

	/* get widget rect */
	rtgui_widget_get_rect(btn, &rect);
	rtgui_dc_fill_rect(dc,&rect);
	
	if(RTGUI_WIDGET_IS_ENABLE(btn))
	{
		if(btn->flag & RTGUI_BUTTON_FLAG_PRESS)
		{
			if(RTGUI_WIDGET_BORDER_SIZE(btn)>0)	
				rtgui_dc_draw_border(dc, &rect,RTGUI_BORDER_DOWN);

			if(btn->image != RT_NULL)
			{
				rtgui_rect_t image_rect;
				image_rect.x1 = 1;
				image_rect.y1 = 1;
				image_rect.x2 = btn->image->w;
				image_rect.y2 = btn->image->h;
				rtgui_rect_moveto_align(&rect, &image_rect, RTGUI_ALIGN_CENTER);
				rtgui_image_paste(btn->image, dc, &image_rect,White);
			}
		}
		else
		{
			if(RTGUI_WIDGET_BORDER_SIZE(btn)>0)
				rtgui_dc_draw_border(dc, &rect,RTGUI_WIDGET_BORDER_STYLE(btn));

			if(btn->image != RT_NULL)
			{
				rtgui_rect_t image_rect;
				image_rect.x1 = 0;
				image_rect.y1 = 0;
				image_rect.x2 = btn->image->w;
				image_rect.y2 = btn->image->h;
				rtgui_rect_moveto_align(&rect, &image_rect, RTGUI_ALIGN_CENTER);
				rtgui_image_paste(btn->image, dc, &image_rect,White);
			}
		}
	}
	else
	{
		if(RTGUI_WIDGET_BORDER_SIZE(btn)>0)
			rtgui_dc_draw_border(dc, &rect,RTGUI_WIDGET_BORDER_STYLE(btn));
		if(btn->image != RT_NULL)
		{
			rtgui_rect_t image_rect;
			image_rect.x1 = 0;
			image_rect.y1 = 0;
			image_rect.x2 = btn->image->w;
			image_rect.y2 = btn->image->h;
			rtgui_rect_moveto_align(&rect, &image_rect, RTGUI_ALIGN_CENTER);
			rtgui_image_paste(btn->image, dc, &image_rect,White);
		}
	}

	if(btn->image == RT_NULL)
	{
		/* re-set foreground and get default rect */
		rtgui_widget_get_rect(btn, &rect);
		/* remove border */
		rtgui_rect_inflate(&rect, -2);

		if(RTGUI_WIDGET_IS_ENABLE(btn))
		{
			if(btn->flag & RTGUI_BUTTON_FLAG_PRESS)
			{
				rtgui_color_t fc;
				fc = RTGUI_DC_FC(dc);
				RTGUI_DC_FC(dc) = Blue;
				rect.x1 += 1;
				rect.y1 += 1;
				rect.x2 += 1;
				rect.y2 += 1;
				if(RTGUI_WIDGET_FLAG(btn) & RTGUI_WIDGET_FLAG_FONT_STROKE)
					rtgui_dc_draw_text_stroke(dc, rtgui_label_get_text(RTGUI_LABEL(btn)), &rect, Black, White);
				else
					rtgui_dc_draw_text(dc, rtgui_label_get_text(RTGUI_LABEL(btn)), &rect);
				RTGUI_DC_FC(dc) = fc;
			}
			else
			{
				if(RTGUI_WIDGET_FLAG(btn) & RTGUI_WIDGET_FLAG_FONT_STROKE)
					rtgui_dc_draw_text_stroke(dc, rtgui_label_get_text(RTGUI_LABEL(btn)), &rect, Black, White);
				else
					rtgui_dc_draw_text(dc, rtgui_label_get_text(RTGUI_LABEL(btn)), &rect);
			}
		}
		else
		{
			rtgui_color_t fc = RTGUI_DC_FC(dc);
			RTGUI_DC_FC(dc) = White;
			rtgui_rect_moveto(&rect, 1,0);
			rtgui_dc_draw_text(dc, rtgui_label_get_text(RTGUI_LABEL(btn)), &rect);
			rtgui_rect_moveto(&rect, 0,1);
			rtgui_dc_draw_text(dc, rtgui_label_get_text(RTGUI_LABEL(btn)), &rect);
			RTGUI_DC_FC(dc) = Gray;
			rtgui_rect_moveto(&rect, -1,-1);
			rtgui_dc_draw_text(dc, rtgui_label_get_text(RTGUI_LABEL(btn)), &rect);
			RTGUI_DC_FC(dc) = fc;
		}
	}

	if(RTGUI_WIDGET_IS_ENABLE(btn))
	{
		if(RTGUI_WIDGET_IS_FOCUSED(btn) && RTGUI_WIDGET(btn)->on_focus_in != RT_NULL)
		{
			/* re-set foreground and get default rect */
			rtgui_widget_get_rect(btn, &rect);
			rtgui_rect_inflate(&rect, -2);
			rtgui_dc_draw_focus_rect(dc,&rect);
		}
	}

	rtgui_dc_end_drawing(dc);
}

rt_bool_t rtgui_button_event_handler(pvoid wdt, rtgui_event_t* event)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	rtgui_button_t* btn = RTGUI_BUTTON(wdt);

	if(btn == RT_NULL)return RT_FALSE;

	switch(event->type)
	{
		case RTGUI_EVENT_PAINT:
			if(widget->on_draw != RT_NULL)
				widget->on_draw(widget, event);
			else
				rtgui_button_ondraw(btn);
			break;

		case RTGUI_EVENT_KBD:
		{
			struct rtgui_event_kbd *ekbd = (struct rtgui_event_kbd*) event;

			if(!RTGUI_WIDGET_IS_ENABLE(btn)) return RT_FALSE;

			if(ekbd->key == RTGUIK_SPACE)
			{
				if(RTGUI_KBD_IS_DOWN(ekbd))
					btn->flag |= RTGUI_BUTTON_FLAG_PRESS;
				else
					btn->flag &= ~RTGUI_BUTTON_FLAG_PRESS;
				/* draw button */
				rtgui_widget_update(btn);

				if((btn->flag & RTGUI_BUTTON_FLAG_PRESS) && (btn->click != RT_NULL))
				{
					/* call on button handler */
					btn->click(widget, event);
				}

				if(!RTGUI_KBD_IS_DOWN(ekbd))
					btn->flag |= RTGUI_BUTTON_FLAG_PRESS;
				else
					btn->flag &= ~RTGUI_BUTTON_FLAG_PRESS;

				/* draw button */
				rtgui_widget_update(btn);
			}
			return RT_TRUE;
		}
		case RTGUI_EVENT_MOUSE_BUTTON:
		{
			struct rtgui_event_mouse* emouse = (struct rtgui_event_mouse*)event;

			if(!RTGUI_WIDGET_IS_ENABLE(btn)) return RT_FALSE;
			if(emouse->button & RTGUI_MOUSE_BUTTON_LEFT)
			{
				rtgui_widget_focus(widget);
				
				/* it's a normal button */
				if(emouse->button & RTGUI_MOUSE_BUTTON_DOWN)
					btn->flag |= RTGUI_BUTTON_FLAG_PRESS;
				else if(emouse->button & RTGUI_MOUSE_BUTTON_UP)
					btn->flag &= ~RTGUI_BUTTON_FLAG_PRESS;

				/* draw button */
				rtgui_widget_update(btn);

				/* invokes call back */
				if(widget->on_mouseclick != RT_NULL && emouse->button & RTGUI_MOUSE_BUTTON_UP)
					return widget->on_mouseclick(widget, event);

				if(!(btn->flag & RTGUI_BUTTON_FLAG_PRESS) && (btn->click != RT_NULL))
				{
					/* call on button handler */
					btn->click(widget, event);
				}
			}
				
			return RT_FALSE;
		}
		default:
			return RT_TRUE;
	}

	return RT_FALSE;
}

void rtgui_button_set_image(rtgui_button_t* btn, rtgui_image_t* image)
{
	RT_ASSERT(btn != RT_NULL);

	btn->image = image;
}

void rtgui_button_set_click(rtgui_button_t* btn, rtgui_click_func_t func)
{
	RT_ASSERT(btn != RT_NULL);

	btn->click = func;
}


static rt_bool_t rtgui_button_onfocus(pvoid wdt, rtgui_event_t* event)
{
	/* nothing */
	return RT_TRUE;
}

static rt_bool_t rtgui_button_onunfocus(pvoid wdt, rtgui_event_t* event)
{
	rtgui_rect_t rect;
	rtgui_button_t *btn = RTGUI_BUTTON(wdt);
	rtgui_dc_t *dc;

	RT_ASSERT(wdt != RT_NULL);

	dc = rtgui_dc_begin_drawing(wdt);
	if(dc == RT_NULL)return RT_FALSE;

	rtgui_widget_get_rect(btn, &rect);

	if(!RTGUI_WIDGET_IS_FOCUSED(btn))
	{
		/* clear focus rect */
		rtgui_color_t color;
		rtgui_rect_inflate(&rect, -2);
		color = RTGUI_DC_FC(dc);
		RTGUI_DC_FC(dc) = RTGUI_DC_BC(dc);
		rtgui_dc_draw_focus_rect(dc,&rect);
		RTGUI_DC_FC(dc) = color;
	}

	rtgui_dc_end_drawing(dc);

	return RT_TRUE;
}

void rtgui_button_set_text(rtgui_button_t* btn, const char* text)
{
	if(btn == RT_NULL)return;

	rtgui_label_set_text(RTGUI_LABEL(btn), text);

	/* update widget */
	rtgui_button_ondraw(btn);
}
