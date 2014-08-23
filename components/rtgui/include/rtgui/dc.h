/*
 * File      : dc.h
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
#ifndef __RTGUI_DC_H__
#define __RTGUI_DC_H__

#include <stdlib.h>
#include <rtthread.h>
#include <rtgui/font.h>
#include <rtgui/driver.h>
#include <rtgui/region.h>
#include <rtgui/blit.h>
#include <rtgui/rtgui_theme.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

#define _int_swap(x, y)			do {x ^= y; y ^= x; x ^= y;} while (0)

enum rtgui_dc_type
{
	RTGUI_DC_HW,
	RTGUI_DC_CLIENT,
	RTGUI_DC_BUFFER,
	RTGUI_DC_IMLIB2,
};

typedef struct rtgui_dc_engine
{
	/* interface */
	void (*draw_point)(rtgui_dc_t* dc, int x, int y);
	void (*draw_color_point)(rtgui_dc_t* dc, int x, int y, rtgui_color_t color);
	void (*draw_vline)(rtgui_dc_t* dc, int x, int y1, int y2);
	void (*draw_hline)(rtgui_dc_t* dc, int x1, int x2, int y);
	void (*fill_rect )(rtgui_dc_t* dc, rtgui_rect_t* rect);
	void (*blit_line) (rtgui_dc_t* dc, int x1, int x2, int y, rt_uint8_t* line_data);
	void (*blit)(rtgui_dc_t* src, int sx, int sy, rtgui_dc_t* dest, int dx, int dy, int w, int h);

	/* set and get graphic context */
	void (*set_gc)(rtgui_dc_t* dc, rtgui_gc_t *gc);
	rtgui_gc_t* (*get_gc)(rtgui_dc_t* dc);

	/* get dc visible */
	rt_bool_t (*get_visible)(rtgui_dc_t* dc);
	/* get dc rect */
	void (*get_rect)(rtgui_dc_t* dc, rtgui_rect_t* rect);

	rt_bool_t (*fini)(rtgui_dc_t* dc);
}dc_engine_t;

/* the abstract device context */
struct rtgui_dc
{
	/* type of device context */
	rt_uint32_t type;
	rt_int16_t w;
	rt_int16_t h;
	/* dc engine */
	const dc_engine_t* engine;
};

#define RTGUI_DC_FC(dc)			(rtgui_dc_get_gc(dc)->fc)
#define RTGUI_DC_BC(dc)			(rtgui_dc_get_gc(dc)->bc)
#define RTGUI_DC_FONT(dc)		(rtgui_dc_get_gc(dc)->font)
#define RTGUI_DC_TEXTALIGN(dc)	(rtgui_dc_get_gc(dc)->textalign)

/* create a buffer dc */
rtgui_dc_t* rtgui_dc_buffer_create(int width, int height);
rt_uint8_t* rtgui_dc_buffer_get_pixel(rtgui_dc_t* dc);

/* begin and end a drawing */
rtgui_dc_t* rtgui_dc_begin_drawing(pvoid wdt);
void rtgui_dc_end_drawing(rtgui_dc_t* dc);

/* destroy a dc */
void rtgui_dc_destory(rtgui_dc_t* dc);

void rtgui_dc_draw_line (rtgui_dc_t* dc, int x1, int y1, int x2, int y2);
void rtgui_dc_draw_rect (rtgui_dc_t* dc, rtgui_rect_t* rect);
void rtgui_dc_fill_rect_forecolor(rtgui_dc_t* dc, rtgui_rect_t* rect);
void rtgui_dc_draw_round_rect(rtgui_dc_t* dc, rtgui_rect_t* rect, int r);
void rtgui_dc_fill_round_rect(rtgui_dc_t* dc, rtgui_rect_t* rect, int r);
void rtgui_dc_draw_annulus(rtgui_dc_t *dc, rt_int16_t x, rt_int16_t y, rt_int16_t r1, rt_int16_t r2, rt_int16_t start, rt_int16_t end);
void rtgui_dc_draw_sector(rtgui_dc_t *dc, rt_int16_t x, rt_int16_t y, rt_int16_t r, rt_int16_t start, rt_int16_t end);
void rtgui_dc_fill_sector(rtgui_dc_t *dc, rt_int16_t x, rt_int16_t y, rt_int16_t r, rt_int16_t start, rt_int16_t end);

void rtgui_dc_draw_text (rtgui_dc_t* dc, const char* text, rtgui_rect_t* rect);
void rtgui_dc_draw_text_stroke(rtgui_dc_t *dc, const char *text, rtgui_rect_t *rect, 
	rtgui_color_t color_stroke, rtgui_color_t color_core);

void rtgui_dc_draw_mono_bmp(rtgui_dc_t* dc, int x, int y, int w, int h, const rt_uint8_t* data);
void rtgui_dc_draw_byte(rtgui_dc_t*dc, int x, int y, int h, const rt_uint8_t* data);
void rtgui_dc_draw_word(rtgui_dc_t*dc, int x, int y, int h, const rt_uint8_t* data);

void rtgui_dc_draw_border(rtgui_dc_t* dc, rtgui_rect_t* rect, int flag);
void rtgui_dc_draw_horizontal_line(rtgui_dc_t* dc, int x1, int x2, int y);
void rtgui_dc_draw_vertical_line(rtgui_dc_t* dc, int x, int y1, int y2);
void rtgui_dc_draw_focus_rect(rtgui_dc_t* dc, rtgui_rect_t* rect);

void rtgui_dc_draw_polygon(rtgui_dc_t* dc, const int *vx, const int *vy, int count);
void rtgui_dc_fill_polygon(rtgui_dc_t* dc, const int* vx, const int* vy, int count);

void rtgui_dc_draw_circle(rtgui_dc_t* dc, int x, int y, int r);
void rtgui_dc_fill_circle(rtgui_dc_t* dc, int x, int y, int r);
void rtgui_dc_draw_arc(rtgui_dc_t *dc, int x, int y, int r, double start, double end);

void rtgui_dc_draw_ellipse(rtgui_dc_t* dc, rt_int16_t x, rt_int16_t y, rt_int16_t rx, rt_int16_t ry);
void rtgui_dc_fill_ellipse(rtgui_dc_t *dc, rt_int16_t x, rt_int16_t y, rt_int16_t rx, rt_int16_t ry);
void rtgui_dc_draw_ellipse_bre(rtgui_dc_t *dc,float xc ,float yc,float fla,float flb, double angle);
/*
 * dc inline function
 *
 * Note:
 * In order to improve drawing speed, put most of common function of dc to inline
 */

/*
 * get rect of dc
 */
rt_inline void rtgui_dc_get_rect(rtgui_dc_t*dc, rtgui_rect_t* rect)
{
	dc->engine->get_rect(dc, rect);
}

/*
 * draw a point on dc
 */
rt_inline void rtgui_dc_draw_point(rtgui_dc_t* dc, int x, int y)
{
	dc->engine->draw_point(dc, x, y);
}

/*
 * draw a color point on dc
 */
rt_inline void rtgui_dc_draw_color_point(rtgui_dc_t* dc, int x, int y, rtgui_color_t color)
{
	dc->engine->draw_color_point(dc, x, y, color);
}

/*
 * draw a vertical line on dc
 */
rt_inline void rtgui_dc_draw_vline(rtgui_dc_t* dc, int x, int y1, int y2)
{
	if(y1 > y2) _int_swap(y1, y2);
	if(y1 < 0) 
	{
		y1 = 0;
		if(y2 < 0) return;
	}
	if(x < 0) return;
	
	dc->engine->draw_vline(dc, x, y1, y2);
}

/*
 * draw a horizontal line on dc
 */
rt_inline void rtgui_dc_draw_hline(rtgui_dc_t* dc, int x1, int x2, int y)
{
	if(x1 > x2) _int_swap(x1, x2);
	if(x1 < 0) 
	{
		x1 = 0;
		if(x2 < 0) return;
	}
	if(y < 0) return;
	
	dc->engine->draw_hline(dc, x1, x2, y);
}

/*
 * fill a rect with background color 
 */
rt_inline void rtgui_dc_fill_rect (rtgui_dc_t* dc, rtgui_rect_t* rect)
{
	dc->engine->fill_rect(dc, rect);
}

/*
 * blit a dc on hardware dc
 */
rt_inline void rtgui_dc_blit(rtgui_dc_t* src, int sx, int sy, rtgui_dc_t* dest, int dx, int dy, int w, int h)
{
	int dif;

	/* check left value */
	if(sx < 0)
	{
		dif = abs(sx);
		sx = 0;
		w -= dif;
		dx += dif;
	}

	if(sy < 0)
	{
		dif = abs(sy);
		sy = 0;
		h -= dif;
		dy += dif;
	}

	/* check right value */
	if(w>(src->w-sx))  w = src->w - sx;
	if(h>(src->h-sy))  h = src->h - sy;
	if(w>(dest->w-dx)) w = dest->w - dx;
	if(h>(dest->h-dy)) h = dest->h - dy;
	
	src->engine->blit(src, sx, sy, dest, dx, dy, w, h);
}

/*
 * set gc of dc
 */
rt_inline void rtgui_dc_set_gc(rtgui_dc_t* dc, rtgui_gc_t* gc)
{
	dc->engine->set_gc(dc, gc);
}

/*
 * get gc of dc
 */
rt_inline rtgui_gc_t *rtgui_dc_get_gc(rtgui_dc_t* dc)
{
	return dc->engine->get_gc(dc);
}

/*
 * get visible status of dc 
 */
rt_inline rt_bool_t rtgui_dc_get_visible(rtgui_dc_t* dc)
{
	return dc->engine->get_visible(dc);
}

#ifdef __cplusplus
}
#endif

#endif
