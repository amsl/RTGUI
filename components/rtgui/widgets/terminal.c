/*
 * File      : terminal.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2011, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-05-10     amsl         first version
 */
#include <rtgui/dc.h>
#include <rtgui/widgets/terminal.h>

static void rtgui_terminal_calc_line(rtgui_terminal_t *tma, const char* text);
static void rtgui_terminal_calc_width(rtgui_terminal_t *tma);
static void rtgui_terminal_ondraw(rtgui_terminal_t *tma);

static void _rtgui_terminal_constructor(rtgui_terminal_t *tma)
{
	/* init widget and set event handler */
	rtgui_widget_set_event_handler(tma, rtgui_terminal_event_handler);
	RTGUI_WIDGET_TEXTALIGN(tma) = RTGUI_ALIGN_LEFT;
	RTGUI_WIDGET_FLAG(tma) |= RTGUI_WIDGET_FLAG_FOCUSABLE;
	/* set field */
	tma->item_count = 0;
	tma->first_item = 0;
	tma->now_item = 0;
	tma->old_item = 0;
	tma->item_per_page = 0;

	tma->lines = RT_NULL;
}

static void _rtgui_terminal_destructor(rtgui_terminal_t *tma)
{
	/* release line memory */
	rt_free(tma->lines);
	tma->lines = RT_NULL;
}

DEFINE_CLASS_TYPE(terminal, "terminal",
                  RTGUI_WIDGET_TYPE,
                  _rtgui_terminal_constructor,
                  _rtgui_terminal_destructor,
                  sizeof(struct rtgui_terminal));

rtgui_terminal_t* rtgui_terminal_create(pvoid parent,const char* text, int left,int top,int w,int h)
{
	rtgui_container_t *container;
	rtgui_terminal_t* tma;

	RT_ASSERT(parent != RT_NULL);
	container = RTGUI_CONTAINER(parent);

	tma = rtgui_widget_create(RTGUI_TERMINAL_TYPE);
	if(tma != RT_NULL)
	{
		rtgui_rect_t rect;
		rtgui_widget_get_rect(container, &rect);
		rtgui_widget_rect_to_device(container,&rect);
		rect.x1 += left;
		rect.y1 += top;
		rect.x2 = rect.x1+w;
		rect.y2 = rect.y1+h;
		rtgui_widget_set_rect(tma,&rect);
		rtgui_container_add_child(container, tma);

		/* calculate line width and line page count */
		rtgui_terminal_calc_width(tma);
		/* set text */
		rtgui_terminal_calc_line(tma, text);
	}

	return tma;
}

void rtgui_terminal_destroy(rtgui_terminal_t* tma)
{
	rtgui_widget_destroy(tma);
}

rt_bool_t rtgui_terminal_event_handler(pvoid wdt, rtgui_event_t* event)
{
	rtgui_widget_t *widget;
	rtgui_terminal_t* tma;

	RT_ASSERT(wdt != RT_NULL);

	widget = RTGUI_WIDGET(wdt);
	tma = RTGUI_TERMINAL(widget);

	switch(event->type)
	{
	case RTGUI_EVENT_PAINT:
		if(widget->on_draw != RT_NULL)
			widget->on_draw(wdt, event);
		else
			rtgui_terminal_ondraw(tma);
		return RT_FALSE;

	case RTGUI_EVENT_KBD:
	{
		struct rtgui_event_kbd* ekbd = (struct rtgui_event_kbd*)event;
		if(ekbd->type == RTGUI_KEYDOWN)
		{
			rt_int16_t line_current_update;
			line_current_update = tma->now_item;
			if(ekbd->key == RTGUIK_LEFT)
			{
				if(tma->now_item > tma->item_per_page)
				{
					line_current_update -= tma->item_per_page;
				}
				else if(tma->now_item > 0)
				{
					line_current_update = 0;
				}
			}
			else if(ekbd->key == RTGUIK_RIGHT)
			{
				if(tma->now_item + tma->item_per_page < tma->item_count - 1)
				{
					line_current_update += tma->item_per_page;
				}
			}
			else if(ekbd->key == RTGUIK_UP)
			{
				if(tma->now_item > 0)
				{
					line_current_update --;
				}
			}
			else if(ekbd->key == RTGUIK_DOWN)
			{
				if(tma->now_item + tma->item_per_page < tma->item_count - 1)
				{
					line_current_update ++;
				}
			}

			if(tma->now_item != line_current_update)
			{
				tma->now_item = line_current_update;
				rtgui_widget_update(wdt);
				return RT_TRUE;
			}
		}
		return RT_TRUE;
	}
	default:
		return rtgui_widget_event_handler(widget,event);
	}
}

rt_inline char* rtgui_terminal_getline(rtgui_terminal_t *tma, rt_uint16_t index)
{
	char* string;
	if(index < tma->item_count)
	{
		string = tma->lines + (index * tma->item_width);
		return string;
	}

	return RT_NULL;
}

static void rtgui_terminal_calc_line(rtgui_terminal_t *tma, const char* text)
{
	char* string;
	const unsigned char* ptr;
	rt_ubase_t line_index, line_position;

	if(tma->lines != RT_NULL)
	{
		rt_free(tma->lines);
		tma->lines = RT_NULL;
		tma->item_count = 0;
	}

	/* get line count */
	line_index = 0;
	line_position = 0;
	ptr = (const unsigned char*)text;
	if(*ptr == 0) return;

	while(*ptr != '\0')
	{
		if(*ptr == '\n')
		{
			line_index ++;
			line_position = 0;
		}
		else if(*ptr == '\r')
		{
			ptr ++;
			continue;
		}
		else if(*ptr == '\t')
		{
			line_position += 4;
			if(line_position >= tma->item_width - 1)
			{
				line_index ++;
				line_position = 0;
			}
		}
		else
		{
			if((*ptr) >= 0x80)
			{
				/* fill cjk character */
				if(line_position + 1 >= (tma->item_width - 1))
				{
					/* split to next line */
					line_index ++;
					line_position = 0;
				}

				line_position ++;
				line_position ++;
			}
			else
			{
				line_position ++;
			}

			if(line_position >= tma->item_width - 1)
			{
				line_index ++;
				line_position = 0;
			}
		}

		ptr ++;
	}

	/* set line count */
	tma->item_count = line_index + 1;

	/* allocate lines */
	tma->lines = rt_malloc(tma->item_count * tma->item_width);
	rt_memset(tma->lines, 0, (tma->item_count * tma->item_width));

	/* fill lines */
	line_index = 0;
	line_position = 0;
	ptr = (const unsigned char*)text;
	string = rtgui_terminal_getline(tma, line_index);
	while(*ptr)
	{
		if(*ptr == '\n')
		{
			line_index ++;
			line_position = 0;
			string = rtgui_terminal_getline(tma, line_index);
		}
		else if(*ptr == '\r')
		{
			/* ignore '\r' */
			ptr ++;
			continue;
		}
		else if(*ptr == '\t')
		{
			string[line_position++] = ' ';
			string[line_position++] = ' ';
			string[line_position++] = ' ';
			string[line_position++] = ' ';
			if(line_position >= tma->item_width - 1)
			{
				line_index ++;
				line_position = 0;
				string = rtgui_terminal_getline(tma, line_index);
			}
		}
		else
		{
			if((*ptr) >= 0x80)
			{
				/* fill cjk character */
				if(line_position + 1 >= (tma->item_width - 1))
				{
					/* split to next line */
					line_index ++;
					line_position = 0;
					string = rtgui_terminal_getline(tma, line_index);
				}

				string[line_position ++] = *ptr ++;
				string[line_position ++] = *ptr;
			}
			else
			{
				string[line_position ++] = *ptr;
			}

			if(line_position >= tma->item_width - 1)
			{
				line_index ++;
				line_position = 0;
				string = rtgui_terminal_getline(tma, line_index);
			}
		}

		ptr ++;
	}

	tma->now_item = 0;
}

static void rtgui_terminal_calc_width(rtgui_terminal_t *tma)
{
	rt_uint16_t w, h, fw, fh;

	w = rtgui_widget_get_width(tma)-RTGUI_WIDGET_BORDER_SIZE(tma)-RTGUI_MARGIN*2;
	h = rtgui_widget_get_height(tma);
	fw = FONT_W(RTGUI_WIDGET_FONT(tma));
	fh = FONT_H(RTGUI_WIDGET_FONT(tma));

	tma->item_width = w/fw + 1;
	tma->item_per_page = h / (fh);

	/* set minimal value */
	if(tma->item_per_page == 0) tma->item_per_page = 1;
}

static void rtgui_terminal_ondraw(rtgui_terminal_t *tma)
{
	rtgui_dc_t* dc;
	rtgui_rect_t rect;
	char* str;
	rt_ubase_t i, item_height;

	item_height = FONT_H(RTGUI_WIDGET_FONT(tma));

	dc = rtgui_dc_begin_drawing(tma);
	if(dc == RT_NULL) return ;

	/* get widget rect */
	rtgui_widget_get_rect(tma, &rect);

	rtgui_rect_inflate(&rect,-RTGUI_WIDGET_BORDER_SIZE(tma));
	rtgui_dc_fill_rect(dc,&rect);
	rtgui_rect_inflate(&rect,RTGUI_WIDGET_BORDER_SIZE(tma));
	/* draw border */
	rtgui_dc_draw_border(dc,&rect,RTGUI_WIDGET_BORDER_STYLE(tma));

	rect.x1 += RTGUI_MARGIN+RTGUI_WIDGET_BORDER_SIZE(tma);
	rect.x2 -= RTGUI_MARGIN;
	rect.y1 += RTGUI_MARGIN;
	rect.y2 = rect.y1+item_height;

	for(i = tma->now_item;
	        (i < tma->now_item + tma->item_per_page) &&
	        (i < tma->item_count); i++)
	{
		str = (char*)rtgui_terminal_getline(tma, i);
		rtgui_dc_draw_text(dc, str, &rect);

		rect.y1 += item_height;
		rect.y2 = rect.y1+item_height;
	}

	rtgui_dc_end_drawing(dc);
}

void rtgui_terminal_set_text(rtgui_terminal_t* tma, const char* text)
{
	RT_ASSERT(tma != RT_NULL);

	/* calculate line width and line page count */
	rtgui_terminal_calc_width(tma);

	/* set text */
	rtgui_terminal_calc_line(tma, text);

	/* update widget */
	rtgui_widget_update(RTGUI_WIDGET(tma));
}
