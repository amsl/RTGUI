/*
 * File      : vernier.c
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
#include <rtgui/widgets/vernier.h>

static void _rtgui_vernier_constructor(rtgui_vernier_t *verr)
{
	verr->nowloc = 0;
	verr->oldloc = 0;
	/* init widget and set event handler */
	rtgui_widget_set_event_handler(verr, rtgui_vernier_event_handler);
}

static void _rtgui_vernier_destructor(rtgui_vernier_t *verr)
{
}

DEFINE_CLASS_TYPE(vernier, "vernier",
                  RTGUI_WIDGET_TYPE,
                  _rtgui_vernier_constructor,
                  _rtgui_vernier_destructor,
                  sizeof(struct rtgui_vernier));

rtgui_vernier_t* rtgui_vernier_create(pvoid parent, int left, int top, int w, int h,
                                      rt_int16_t orient, rt_int16_t dir_motion)
{
	rtgui_container_t *container;
	rtgui_vernier_t* verr;

	RT_ASSERT(parent != RT_NULL);
	container = RTGUI_CONTAINER(parent);

	verr = rtgui_widget_create(RTGUI_VERNIER_TYPE);
	if(verr != RT_NULL)
	{
		rtgui_rect_t rect;
		rtgui_widget_get_rect(container, &rect);
		rtgui_widget_rect_to_device(container,&rect);
		rect.x1 += left;
		rect.y1 += top;
		rect.x2 = rect.x1+w;
		rect.y2 = rect.y1+h;
		rtgui_widget_set_rect(verr, &rect);
		rtgui_container_add_child(container, verr);
		verr->orient = orient;
		if(orient == RTGUI_HORIZONTAL)
		{
			verr->len = RC_W(rect) - RC_H(rect);
			verr->thumb_w = RC_H(rect);
			verr->thumb_h = RC_H(rect);
		}
		else if(orient == RTGUI_VERTICAL)
		{
			verr->len = RC_H(rect) - RC_W(rect);
			verr->thumb_w = RC_W(rect);
			verr->thumb_h = RC_W(rect);
		}
		verr->dir_motion = dir_motion;
	}

	return verr;
}

void rtgui_vernier_destroy(rtgui_vernier_t* verr)
{
	rtgui_widget_destroy(verr);
}

void rtgui_vernier_ondraw(rtgui_vernier_t *verr)
{
	rtgui_rect_t rect;
	rtgui_color_t _fc,_bc;
	rtgui_dc_t *dc;

	if(verr == RT_NULL) return;
	dc = rtgui_dc_begin_drawing(verr);
	if(dc == RT_NULL) return;

	_fc = RTGUI_DC_FC(dc);
	_bc = RTGUI_DC_BC(dc);

	rtgui_widget_get_rect(verr, &rect);
	rtgui_rect_inflate(&rect,-RTGUI_WIDGET_BORDER_SIZE(verr));
	rtgui_dc_fill_rect(dc,&rect);
	rtgui_rect_inflate(&rect, RTGUI_WIDGET_BORDER_SIZE(verr));
	rtgui_dc_draw_border(dc, &rect, RTGUI_WIDGET_BORDER_STYLE(verr));

	if(verr->orient == RTGUI_HORIZONTAL)
	{
		rtgui_rect_t thumb;
		thumb.x1 = thumb.y1 = 0;
		thumb.x2 = verr->thumb_w;
		thumb.y2 = verr->thumb_h;
		rtgui_rect_moveto_align(&rect, &thumb, RTGUI_ALIGN_CENTER_VERTICAL);

		if(verr->dir_motion == 0) /* forward direction */
			rtgui_rect_moveto(&thumb, verr->nowloc, 0);
		else /* reverse direction */
			rtgui_rect_moveto(&thumb, verr->len - verr->nowloc, 0);

		if(verr->nowloc == 0 || verr->nowloc == verr->len)
			RTGUI_DC_BC(dc) = Red;
		else
			RTGUI_DC_BC(dc) = Blue;
		rtgui_rect_inflate(&thumb,-1);
		rtgui_dc_fill_rect(dc, &thumb);
	}
	else if(verr->orient == RTGUI_VERTICAL)
	{
		rtgui_rect_t thumb;
		thumb.x1 = thumb.y1 = 0;
		thumb.x2 = verr->thumb_w;
		thumb.y2 = verr->thumb_h;
		rtgui_rect_moveto_align(&rect, &thumb, RTGUI_ALIGN_CENTER_HORIZONTAL);

		if(verr->dir_motion == 0) /* forward direction */
			rtgui_rect_moveto(&thumb, 0, verr->len - verr->nowloc);
		else /* reverse direction */
			rtgui_rect_moveto(&thumb, 0, verr->nowloc);

		if(verr->nowloc == 0 || verr->nowloc == verr->len)
			RTGUI_DC_BC(dc) = Red;
		else
			RTGUI_DC_BC(dc) = Blue;
		rtgui_rect_inflate(&thumb,-1);
		rtgui_dc_fill_rect(dc, &thumb);
	}

	RTGUI_DC_FC(dc) = _fc;
	RTGUI_DC_BC(dc) = _bc;
	rtgui_dc_end_drawing(dc);
}

void rtgui_vernier_update(rtgui_vernier_t* verr)
{
	rtgui_rect_t rect;
	rtgui_color_t _fc,_bc;
	rtgui_dc_t *dc;

	if(verr == RT_NULL) return;
	dc = rtgui_dc_begin_drawing(verr);
	if(dc == RT_NULL) return;

	_fc = RTGUI_DC_FC(dc);
	_bc = RTGUI_DC_BC(dc);

	rtgui_widget_get_rect(verr, &rect);

	if(verr->orient == RTGUI_HORIZONTAL)
	{
		rtgui_rect_t thumb;
		thumb.x1 = thumb.y1 = 0;
		thumb.x2 = verr->thumb_w;
		thumb.y2 = verr->thumb_h;
		rtgui_rect_moveto_align(&rect, &thumb, RTGUI_ALIGN_CENTER_VERTICAL);

		if(verr->dir_motion == 0) /* forward direction */
			rtgui_rect_moveto(&thumb, verr->oldloc, 0);
		else /* reverse direction */
			rtgui_rect_moveto(&thumb, verr->len-verr->oldloc, 0);

		rtgui_rect_inflate(&thumb,-1);
		rtgui_dc_fill_rect(dc, &thumb);

		thumb.x1 = thumb.y1 = 0;
		thumb.x2 = verr->thumb_w;
		thumb.y2 = verr->thumb_h;
		rtgui_rect_moveto_align(&rect, &thumb, RTGUI_ALIGN_CENTER_VERTICAL);

		if(verr->dir_motion == 0) /* forward direction */
			rtgui_rect_moveto(&thumb, verr->nowloc, 0);
		else /* reverse direction */
			rtgui_rect_moveto(&thumb, verr->len-verr->nowloc, 0);

		if(verr->nowloc == 0 || verr->nowloc == verr->len)
			RTGUI_DC_BC(dc) = Red;
		else
			RTGUI_DC_BC(dc) = _fc;
		rtgui_rect_inflate(&thumb,-1);
		rtgui_dc_fill_rect(dc, &thumb);
	}
	else if(verr->orient == RTGUI_VERTICAL)
	{
		rtgui_rect_t thumb;
		thumb.x1 = thumb.y1 = 0;
		thumb.x2 = verr->thumb_w;
		thumb.y2 = verr->thumb_h;
		rtgui_rect_moveto_align(&rect, &thumb, RTGUI_ALIGN_CENTER_HORIZONTAL);

		if(verr->dir_motion == 0) /* forward direction */
			rtgui_rect_moveto(&thumb, 0, verr->len - verr->oldloc);
		else /* reverse direction */
			rtgui_rect_moveto(&thumb, 0, verr->oldloc);

		rtgui_rect_inflate(&thumb,-1);
		rtgui_dc_fill_rect(dc, &thumb);

		thumb.x1 = thumb.y1 = 0;
		thumb.x2 = verr->thumb_w;
		thumb.y2 = verr->thumb_h;
		rtgui_rect_moveto_align(&rect, &thumb, RTGUI_ALIGN_CENTER_HORIZONTAL);

		if(verr->dir_motion == 0) /* forward direction */
			rtgui_rect_moveto(&thumb, 0, verr->len - verr->nowloc);
		else /* reverse direction */
			rtgui_rect_moveto(&thumb, 0, verr->nowloc);

		if(verr->nowloc == 0 || verr->nowloc == verr->len)
			RTGUI_DC_BC(dc) = Red;
		else
			RTGUI_DC_BC(dc) = _fc;
		rtgui_rect_inflate(&thumb,-1);
		rtgui_dc_fill_rect(dc, &thumb);
	}

	RTGUI_DC_FC(dc) = _fc;
	RTGUI_DC_BC(dc) = _bc;
	rtgui_dc_end_drawing(dc);
}

rt_bool_t rtgui_vernier_event_handler(pvoid wdt, rtgui_event_t* event)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	rtgui_vernier_t *verr = RTGUI_VERNIER(wdt);

	RT_ASSERT(widget != RT_NULL);

	switch(event->type)
	{
		case RTGUI_EVENT_PAINT:
			if(widget->on_draw)
				widget->on_draw(widget, event);
			else
				rtgui_vernier_ondraw(verr);
			return RT_FALSE;
		default:
			return RT_FALSE;
	}
}
