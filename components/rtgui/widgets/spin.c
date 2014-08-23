/*
 * File      : spin.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * spin(abbr.spin): a change/UpDown subsidiary widget.
 * Change Logs:
 * Date           Author       Notes
 *
 */
#include <rtgui/widgets/spin.h>

static void _rtgui_spin_constructor(rtgui_spin_t *spin)
{
	/* init widget and set event handler */
	rtgui_widget_set_event_handler(spin, rtgui_spin_event_handler);

	RTGUI_WIDGET_FC(spin) = theme.foreground;
	RTGUI_WIDGET_BC(spin) = theme.background;
	RTGUI_WIDGET_TEXTALIGN(spin) = RTGUI_ALIGN_CENTER_VERTICAL;
	/* set field */
	spin->orient = RTGUI_HORIZONTAL;
	spin->flag = PROPEL_FLAG_NONE;
	spin->range_min = 0;
	spin->range_max = 0;
	spin->widget_link = RT_NULL;
	spin->bind = RT_NULL;
}

static void _rtgui_spin_destructor(rtgui_spin_t *spin)
{

}

DEFINE_CLASS_TYPE(spin, "spin",
                  RTGUI_WIDGET_TYPE,
                  _rtgui_spin_constructor,
                  _rtgui_spin_destructor,
                  sizeof(struct rtgui_spin));

//parent必须是一个容器类控件
rtgui_spin_t* rtgui_spin_create(pvoid parent, int left, int top, int w, int h, int orient)
{
	rtgui_container_t *container;
	rtgui_spin_t* spin;

	RT_ASSERT(parent != RT_NULL);
	container = RTGUI_CONTAINER(parent);

	spin = rtgui_widget_create(RTGUI_SPIN_TYPE);
	if(spin != RT_NULL)
	{
		rtgui_rect_t rect;
		rtgui_widget_get_rect(container, &rect);
		rtgui_widget_rect_to_device(container,&rect);
		rect.x1 += left;
		rect.y1 += top;
		rect.x2 = rect.x1+w;
		rect.y2 = rect.y1+h;
		rtgui_widget_set_rect(spin, &rect);
		rtgui_container_add_child(container, spin);
		spin->orient = orient;
	}
	return spin;
}

void rtgui_ppl_destroy(rtgui_spin_t* spin)
{
	rtgui_widget_destroy(spin);
}

const static rt_uint8_t _up_arrow[]    = {0x10, 0x38, 0x7C, 0xFE};
const static rt_uint8_t _down_arrow[]  = {0xFE,0x7C, 0x38, 0x10};
const static rt_uint8_t _left_arrow[]  = {0x10, 0x30, 0x70, 0xF0, 0x70, 0x30, 0x10};
const static rt_uint8_t _right_arrow[] = {0x80, 0xC0, 0xE0, 0xF0, 0xE0, 0xC0, 0x80};

void rtgui_spin_ondraw(rtgui_spin_t* spin)
{
	rtgui_rect_t rect, tmp_rect, arrow_rect;
	rtgui_dc_t* dc;

	RT_ASSERT(spin != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(spin);
	if(dc == RT_NULL)return;

	rtgui_widget_get_rect(spin, &rect);

	tmp_rect = rect;
	if(spin->orient == RTGUI_HORIZONTAL)
	{
		tmp_rect.x2 = tmp_rect.x2/2;
		rtgui_rect_inflate(&tmp_rect, -RTGUI_WIDGET_BORDER_SIZE(spin));
		rtgui_dc_fill_rect(dc, &tmp_rect);
		rtgui_rect_inflate(&tmp_rect, RTGUI_WIDGET_BORDER_SIZE(spin));
		if(spin->flag & PROPEL_FLAG_LEFT)
			rtgui_dc_draw_border(dc, &tmp_rect, RTGUI_BORDER_DOWN);
		else
			rtgui_dc_draw_border(dc, &tmp_rect, RTGUI_WIDGET_BORDER_STYLE(spin));

		arrow_rect.x1 = 0;
		arrow_rect.y1 = 0;
		arrow_rect.x2 = 4;
		arrow_rect.y2 = 7;
		rtgui_rect_moveto_align(&tmp_rect, &arrow_rect, RTGUI_ALIGN_CENTER);
		if(spin->flag & PROPEL_UNVISIBLE_LEFT)
			RTGUI_DC_FC(dc) = Gray;
		else
			RTGUI_DC_FC(dc) = theme.foreground;
		rtgui_dc_draw_byte(dc, arrow_rect.x1, arrow_rect.y1, RC_H(arrow_rect), _left_arrow);

		tmp_rect.x1 = tmp_rect.x2;
		tmp_rect.x2 = rect.x2;
		rtgui_rect_inflate(&tmp_rect, -RTGUI_WIDGET_BORDER_SIZE(spin));
		rtgui_dc_fill_rect(dc, &tmp_rect);
		rtgui_rect_inflate(&tmp_rect, RTGUI_WIDGET_BORDER_SIZE(spin));
		if(spin->flag & PROPEL_FLAG_RIGHT)
			rtgui_dc_draw_border(dc, &tmp_rect, RTGUI_BORDER_DOWN);
		else
			rtgui_dc_draw_border(dc, &tmp_rect, RTGUI_WIDGET_BORDER_STYLE(spin));

		arrow_rect.x1 = 0;
		arrow_rect.y1 = 0;
		arrow_rect.x2 = 4;
		arrow_rect.y2 = 7;
		rtgui_rect_moveto_align(&tmp_rect, &arrow_rect, RTGUI_ALIGN_CENTER);
		if(spin->flag & PROPEL_UNVISIBLE_RIGHT)
			RTGUI_DC_FC(dc) = Gray;
		else
			RTGUI_DC_FC(dc) = theme.foreground;
		rtgui_dc_draw_byte(dc, arrow_rect.x1, arrow_rect.y1, RC_H(arrow_rect), _right_arrow);
	}
	else if(spin->orient == RTGUI_VERTICAL)
	{
		tmp_rect.y2 = tmp_rect.y2/2;
		rtgui_rect_inflate(&tmp_rect, -RTGUI_WIDGET_BORDER_SIZE(spin));
		rtgui_dc_fill_rect(dc, &tmp_rect);
		rtgui_rect_inflate(&tmp_rect, RTGUI_WIDGET_BORDER_SIZE(spin));
		if(spin->flag & PROPEL_FLAG_UP)
			rtgui_dc_draw_border(dc, &tmp_rect, RTGUI_BORDER_DOWN);
		else
			rtgui_dc_draw_border(dc, &tmp_rect, RTGUI_WIDGET_BORDER_STYLE(spin));

		arrow_rect.x1 = 0;
		arrow_rect.y1 = 0;
		arrow_rect.x2 = 7;
		arrow_rect.y2 = 4;
		rtgui_rect_moveto_align(&tmp_rect, &arrow_rect, RTGUI_ALIGN_CENTER);
		if(spin->flag & PROPEL_UNVISIBLE_UP)
			RTGUI_DC_FC(dc) = Gray;
		else
			RTGUI_DC_FC(dc) = theme.foreground;
		rtgui_dc_draw_byte(dc, arrow_rect.x1, arrow_rect.y1, RC_H(arrow_rect), _up_arrow);

		tmp_rect.y1 = tmp_rect.y2;
		tmp_rect.y2 = rect.y2;
		rtgui_rect_inflate(&tmp_rect, -RTGUI_WIDGET_BORDER_SIZE(spin));
		rtgui_dc_fill_rect(dc, &tmp_rect);
		rtgui_rect_inflate(&tmp_rect, RTGUI_WIDGET_BORDER_SIZE(spin));
		if(spin->flag & PROPEL_FLAG_DOWN)
			rtgui_dc_draw_border(dc, &tmp_rect, RTGUI_BORDER_DOWN);
		else
			rtgui_dc_draw_border(dc, &tmp_rect, RTGUI_WIDGET_BORDER_STYLE(spin));

		arrow_rect.x1 = 0;
		arrow_rect.y1 = 0;
		arrow_rect.x2 = 7;
		arrow_rect.y2 = 4;
		rtgui_rect_moveto_align(&tmp_rect, &arrow_rect, RTGUI_ALIGN_CENTER);
		if(spin->flag & PROPEL_UNVISIBLE_DOWN)
			RTGUI_DC_FC(dc) = Gray;
		else
			RTGUI_DC_FC(dc) = theme.foreground;
		rtgui_dc_draw_byte(dc, arrow_rect.x1, arrow_rect.y1, RC_H(arrow_rect), _down_arrow);
	}

	rtgui_dc_end_drawing(dc);
}

static void rtgui_prople_onclick(rtgui_spin_t* spin, rtgui_event_t *event)
{
	if(spin == RT_NULL) return;

	if(spin->widget_link != RT_NULL && spin->on_click != RT_NULL)
	{
		spin->on_click(spin->widget_link, event);
	}
}

static void rtgui_spin_onmouse(rtgui_spin_t* spin, struct rtgui_event_mouse* emouse)
{
	rtgui_rect_t rect, tmp_rect;

	/* get physical extent information */
	rtgui_widget_get_rect(spin, &rect);
	rtgui_widget_rect_to_device(spin, &rect);

	if(rtgui_rect_contains_point(&rect, emouse->x, emouse->y) == RT_EOK)
	{
		tmp_rect = rect;
		if(emouse->button & RTGUI_MOUSE_BUTTON_DOWN)
		{
			if(spin->orient == RTGUI_HORIZONTAL)
			{
				tmp_rect.x2 = tmp_rect.x1 + RC_W(tmp_rect)/2;
				if(rtgui_rect_contains_point(&tmp_rect, emouse->x, emouse->y) == RT_EOK)
				{
					if(spin->flag & PROPEL_UNVISIBLE_LEFT) return;
					spin->flag |= PROPEL_FLAG_LEFT;
					if(spin->bind != RT_NULL)
					{
						if(*(spin->bind) > spin->range_min)(*(spin->bind))--;
						if(*(spin->bind) <= spin->range_min)
						{
							spin->flag |= PROPEL_UNVISIBLE_LEFT;
						}
						if(spin->flag & PROPEL_UNVISIBLE_RIGHT)
						{
							if(*(spin->bind) < spin->range_max)
							{
								spin->flag &= ~PROPEL_UNVISIBLE_RIGHT;
							}
						}
					}
					rtgui_spin_ondraw(spin);
					rtgui_prople_onclick(spin, (rtgui_event_t*)emouse);
					return;
				}
				tmp_rect.x1 = tmp_rect.x2;
				tmp_rect.x2 = rect.x2;
				if(rtgui_rect_contains_point(&tmp_rect, emouse->x, emouse->y) == RT_EOK)
				{
					if(spin->flag & PROPEL_UNVISIBLE_RIGHT) return;
					spin->flag |= PROPEL_FLAG_RIGHT;
					if(spin->bind != RT_NULL)
					{
						if(*(spin->bind) < spin->range_max)(*(spin->bind))++;
						if(*(spin->bind) >= spin->range_max)
						{
							spin->flag |= PROPEL_UNVISIBLE_RIGHT;
						}
						if(spin->flag & PROPEL_UNVISIBLE_LEFT)
						{
							if(*(spin->bind) > spin->range_min)
							{
								spin->flag &= ~PROPEL_UNVISIBLE_LEFT;
							}
						}
					}
					rtgui_spin_ondraw(spin);
					rtgui_prople_onclick(spin, (rtgui_event_t*)emouse);
					return;
				}
			}
			else if(spin->orient == RTGUI_VERTICAL)
			{
				tmp_rect.y2 = tmp_rect.y1 + RC_H(tmp_rect)/2;
				if(rtgui_rect_contains_point(&tmp_rect, emouse->x, emouse->y) == RT_EOK)
				{
					if(spin->flag & PROPEL_UNVISIBLE_UP) return;
					spin->flag |= PROPEL_FLAG_UP;
					if(spin->bind != RT_NULL)
					{
						if(*(spin->bind) < spin->range_max)(*(spin->bind))++;
						if(*(spin->bind) >= spin->range_max)
						{
							spin->flag |= PROPEL_UNVISIBLE_UP;
						}
						if(spin->flag & PROPEL_UNVISIBLE_DOWN)
						{
							if(*(spin->bind) > spin->range_min)
							{
								spin->flag &= ~PROPEL_UNVISIBLE_DOWN;
							}
						}
					}
					rtgui_spin_ondraw(spin);
					rtgui_prople_onclick(spin, (rtgui_event_t*)emouse);
					return;
				}
				tmp_rect.y1 = tmp_rect.y2;
				tmp_rect.y2 = rect.y2;
				if(rtgui_rect_contains_point(&tmp_rect, emouse->x, emouse->y) == RT_EOK)
				{
					if(spin->flag & PROPEL_UNVISIBLE_DOWN) return;
					spin->flag |= PROPEL_FLAG_DOWN;
					if(spin->bind != RT_NULL)
					{
						if(*(spin->bind) > spin->range_min)(*(spin->bind))--;
						if(*(spin->bind) <= spin->range_min)
						{
							spin->flag |= PROPEL_UNVISIBLE_DOWN;
						}
						if(spin->flag & PROPEL_UNVISIBLE_UP)
						{
							if(*(spin->bind) < spin->range_max)
							{
								spin->flag &= ~PROPEL_UNVISIBLE_UP;
							}
						}
					}
					rtgui_spin_ondraw(spin);
					rtgui_prople_onclick(spin, (rtgui_event_t*)emouse);
					return;
				}
			}
		}
		if(emouse->button & RTGUI_MOUSE_BUTTON_UP)
		{
			spin->flag &= ~PROPEL_FLAG_MASK;
			rtgui_spin_ondraw(spin);
		}
	}
}

rt_bool_t rtgui_spin_event_handler(pvoid wdt, rtgui_event_t* event)
{
	rtgui_widget_t* widget = RTGUI_WIDGET(wdt);
	rtgui_spin_t* spin = RTGUI_SPIN(wdt);

	RT_ASSERT(wdt != RT_NULL);

	switch(event->type)
	{
	case RTGUI_EVENT_PAINT:
		rtgui_spin_ondraw(spin);
		return RT_FALSE;

	case RTGUI_EVENT_MOUSE_BUTTON:
		if(widget->on_mouseclick != RT_NULL)
		{
			widget->on_mouseclick(widget, event);
		}
		else
		{
			rtgui_spin_onmouse(spin, (struct rtgui_event_mouse*)event);
		}
		return RT_TRUE;

	default:
		return RT_FALSE;
	}
}

/* bind a external variable */
void rtgui_spin_bind(rtgui_spin_t *spin, rt_uint32_t *var)
{
	if(spin != RT_NULL)
	{
		spin->bind = var;
	}
}

/* terminate binding relation */
void rtgui_spin_unbind(rtgui_spin_t *spin)
{
	if(spin != RT_NULL)
	{
		spin->bind = RT_NULL;
	}
}
