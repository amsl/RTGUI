/*
 * File      : picture.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-10-11     amsl         first version
 */
#include <rtgui/widgets/picture.h>
#include <rtgui/widgets/container.h>

static void _rtgui_picture_constructor(rtgui_picture_t *pic)
{
	/* init widget and set event handler */
	RTGUI_WIDGET_FLAG(pic) |= RTGUI_WIDGET_FLAG_FOCUSABLE;

	rtgui_widget_set_event_handler(pic, rtgui_picture_event_handler);
	rtgui_widget_set_border_style(pic,RTGUI_BORDER_NONE);
	pic->image = RT_NULL;
}

static void _rtgui_picture_destructor(rtgui_picture_t *pic)
{
	if(pic->image != RT_NULL)
	{
		rtgui_image_destroy(pic->image);
		pic->image = RT_NULL;
	}
}

DEFINE_CLASS_TYPE(picture, "picture",
                  RTGUI_WIDGET_TYPE,
                  _rtgui_picture_constructor,
                  _rtgui_picture_destructor,
                  sizeof(struct rtgui_picture));

rtgui_picture_t* rtgui_picture_create(pvoid parent, int left, int top, int w, int h)
{
	rtgui_container_t *container;
	rtgui_picture_t* pic;

	RT_ASSERT(parent != RT_NULL);
	container = RTGUI_CONTAINER(parent);

	pic = rtgui_widget_create(RTGUI_PICTURE_TYPE);
	if(pic != RT_NULL)
	{
		rtgui_rect_t rect;
		/* set default rect */
		rtgui_widget_get_rect(container, &rect);
		rtgui_widget_rect_to_device(container,&rect);
		rect.x1 += left;
		rect.y1 += top;
		rect.x2 = rect.x1+w;
		rect.y2 = rect.y1+h;
		rtgui_widget_set_rect(pic, &rect);

		rtgui_container_add_child(container, pic);
	}

	return pic;
}

void rtgui_picture_destroy(rtgui_picture_t* pic)
{
	rtgui_widget_destroy(pic);
}

/* widget drawing */
void rtgui_picture_ondraw(rtgui_picture_t* pic)
{
	/* draw picture */
	rtgui_rect_t rect;
	rtgui_dc_t* dc;

	RT_ASSERT(pic != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(pic);
	if(dc == RT_NULL)return;

	/* get widget rect */
	rtgui_widget_get_rect(pic, &rect);
	rtgui_dc_fill_rect(dc,&rect);

	if(RTGUI_WIDGET_BORDER_SIZE(pic)>0)
		rtgui_dc_draw_border(dc, &rect,RTGUI_WIDGET_BORDER_STYLE(pic));

	if(pic->image != RT_NULL)
	{
		rtgui_rect_t image_rect;
		image_rect.x1 = 0;
		image_rect.y1 = 0;
		image_rect.x2 = pic->image->w;
		image_rect.y2 = pic->image->h;
		rtgui_rect_moveto_align(&rect, &image_rect, RTGUI_ALIGN_CENTER);
		rtgui_image_paste(pic->image, dc, &image_rect, theme.blankspace);
		//rtgui_image_blit(pic->image, dc, &image_rect);
	}

	rtgui_dc_end_drawing(dc);
}

rt_bool_t rtgui_picture_event_handler(pvoid wdt, rtgui_event_t* event)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	rtgui_picture_t* pic = RTGUI_PICTURE(wdt);

	if(pic == RT_NULL)return RT_FALSE;

	switch(event->type)
	{
		case RTGUI_EVENT_PAINT:
			if(widget->on_draw != RT_NULL)
				widget->on_draw(widget, event);
			else
				rtgui_picture_ondraw(pic);
			return RT_FALSE;

		default:
			return RT_FALSE;
	}
}

void rtgui_picture_set_image(rtgui_picture_t* pic, rtgui_image_t* image)
{
	if(pic == RT_NULL)return;

	pic->image = image;
}

