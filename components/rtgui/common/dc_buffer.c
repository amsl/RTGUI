/*
 * File      : dc_buffer.c
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
#include <rtgui/dc_buffer.h>
#include <rtgui/rtgui_theme.h>

static rt_bool_t rtgui_dc_buffer_fini(rtgui_dc_t* dc);
static void rtgui_dc_buffer_draw_point(rtgui_dc_t* dc, int x, int y);
static void rtgui_dc_buffer_draw_color_point(rtgui_dc_t* dc, int x, int y, rtgui_color_t color);
static void rtgui_dc_buffer_draw_vline(rtgui_dc_t* dc, int x, int y1, int y2);
static void rtgui_dc_buffer_draw_hline(rtgui_dc_t* dc, int x1, int x2, int y);
static void rtgui_dc_buffer_fill_rect(rtgui_dc_t* dc, rtgui_rect_t* rect);
static void rtgui_dc_buffer_blit_line(rtgui_dc_t* dc, int x1, int x2, int y, rt_uint8_t* line_data);
static void rtgui_dc_buffer_blit(rtgui_dc_t* src, int sx, int sy,
                                 rtgui_dc_t* dest, int dx, int dy, int w, int h);

static void rtgui_dc_buffer_set_gc(rtgui_dc_t* dc, rtgui_gc_t *gc);
static rtgui_gc_t* rtgui_dc_buffer_get_gc(rtgui_dc_t* dc);

static rt_bool_t rtgui_dc_buffer_get_visible(rtgui_dc_t* dc);
static void rtgui_dc_buffer_get_rect(rtgui_dc_t* dc, rtgui_rect_t* rect);

const static dc_engine_t dc_buffer_engine =
{
	rtgui_dc_buffer_draw_point,
	rtgui_dc_buffer_draw_color_point,
	rtgui_dc_buffer_draw_vline,
	rtgui_dc_buffer_draw_hline,
	rtgui_dc_buffer_fill_rect,
	rtgui_dc_buffer_blit_line,
	rtgui_dc_buffer_blit,

	rtgui_dc_buffer_set_gc,
	rtgui_dc_buffer_get_gc,

	rtgui_dc_buffer_get_visible,
	rtgui_dc_buffer_get_rect,

	rtgui_dc_buffer_fini,
};

rtgui_dc_t* rtgui_dc_buffer_create(int w, int h)
{
	rtgui_dc_buffer_t* dc_buffer;

	dc_buffer = (rtgui_dc_buffer_t*)rt_malloc(sizeof(rtgui_dc_buffer_t));
	dc_buffer->parent.type   = RTGUI_DC_BUFFER;
	dc_buffer->parent.w = w;
	dc_buffer->parent.h = h;
	dc_buffer->parent.engine = &dc_buffer_engine;
	dc_buffer->gc.fc = theme.foreground;
	dc_buffer->gc.bc = theme.background;
	dc_buffer->gc.font = rtgui_font_default();
	dc_buffer->gc.textalign = RTGUI_ALIGN_LEFT | RTGUI_ALIGN_TOP;

	dc_buffer->pitch	= w * sizeof(rtgui_color_t);

	rtgui_region_init(&(dc_buffer->clip));

	dc_buffer->pixel = rt_malloc(h * dc_buffer->pitch);
	rt_memset(dc_buffer->pixel, 0, h * dc_buffer->pitch);

	return &(dc_buffer->parent);
}

rt_uint8_t* rtgui_dc_buffer_get_pixel(rtgui_dc_t* dc)
{
	rtgui_dc_buffer_t* dc_buffer;

	dc_buffer = (rtgui_dc_buffer_t*)dc;

	return dc_buffer->pixel;
}

static rt_bool_t rtgui_dc_buffer_fini(rtgui_dc_t* dc)
{
	rtgui_dc_buffer_t* dc_buffer = (rtgui_dc_buffer_t*)dc;

	if(dc->type != RTGUI_DC_BUFFER) return RT_FALSE;

	rt_free(dc_buffer->pixel);
	dc_buffer->pixel = RT_NULL;

	rt_free(dc_buffer);

	return RT_TRUE;
}

static void rtgui_dc_buffer_draw_point(rtgui_dc_t* dc, int x, int y)
{
	rtgui_color_t* ptr;
	rtgui_dc_buffer_t* dc_buffer = (rtgui_dc_buffer_t*)dc;

	if(x < 0 || x >= dc->w) return;
	if(y < 0 || y >= dc->h) return;

	/* note: there is no parameter check in this function */
	ptr = (rtgui_color_t*)(dc_buffer->pixel + y * dc_buffer->pitch + x * sizeof(rtgui_color_t));

	*ptr = dc_buffer->gc.fc;
}

static void rtgui_dc_buffer_draw_color_point(rtgui_dc_t* dc, int x, int y, rtgui_color_t color)
{
	rtgui_color_t* ptr;
	rtgui_dc_buffer_t* dc_buffer = (rtgui_dc_buffer_t*)dc;

	if(x < 0 || x >= dc->w) return;
	if(y < 0 || y >= dc->h) return;

	/* note: there is no parameter check in this function */
	ptr = (rtgui_color_t*)(dc_buffer->pixel + y * dc_buffer->pitch + x * sizeof(rtgui_color_t));

	*ptr = color;
}

static void rtgui_dc_buffer_draw_vline(rtgui_dc_t* dc, int x, int y1, int y2)
{
	rtgui_color_t* ptr;
	register rt_base_t index;
	rtgui_dc_buffer_t* dc_buffer = (rtgui_dc_buffer_t*)dc;

	if(x  >= dc->w) return;
	if(y1 > dc->h) y1 = dc->h;
	if(y2 > dc->h) y2 = dc->h;

	ptr = (rtgui_color_t*)(dc_buffer->pixel + y1 * dc_buffer->pitch + x * sizeof(rtgui_color_t));
	for(index = y1; index < y2; index ++)
	{
		/* draw this point */
		*ptr = dc_buffer->gc.fc;
		ptr += dc->w;
	}
}

static void rtgui_dc_buffer_draw_hline(rtgui_dc_t* dc, int x1, int x2, int y)
{
	rtgui_color_t* ptr;
	register rt_base_t index;
	rtgui_dc_buffer_t* dc_buffer;

	dc_buffer = (rtgui_dc_buffer_t*)dc;
	if(y >= dc->h) return;
	if(x1 > dc->w) x1 = dc->w;
	if(x2 > dc->w) x2 = dc->w;

	ptr = (rtgui_color_t*)(dc_buffer->pixel + y * dc_buffer->pitch + x1 * sizeof(rtgui_color_t));
	for(index = x1; index < x2; index ++)
	{
		/* draw this point */
		*ptr++ = dc_buffer->gc.fc;
	}
}

static void rtgui_dc_buffer_fill_rect(rtgui_dc_t* dc, rtgui_rect_t* rect)
{
	rtgui_color_t fc;
	rtgui_rect_t r;
	rtgui_dc_buffer_t* dc_buffer;

	r = *rect;
	dc_buffer = (rtgui_dc_buffer_t*)dc;
	if(r.x1 > dc->w) r.x1 = dc->w;
	if(r.x2 > dc->w) r.x2 = dc->w;
	if(r.y1 > dc->h) r.y1 = dc->h;
	if(r.y2 > dc->h) r.y2 = dc->h;

	/* save foreground color */
	fc = RTGUI_DC_FC(dc);

	/* set background color as foreground color */
	RTGUI_DC_FC(dc) = RTGUI_DC_BC(dc);

	/* fill first line */
	rtgui_dc_buffer_draw_hline(dc, r.x1, r.x2, r.y1);

	/* memory copy other lines */
	if(r.y2 > r.y1)
	{
		register rt_base_t index;
		for(index = r.y1 + 1; index < r.y2; index ++)
		{
			rt_memcpy(dc_buffer->pixel + index * dc_buffer->pitch,
			          dc_buffer->pixel + r.y1 * dc_buffer->pitch,
			          (r.x2 - r.x1) * sizeof(rtgui_color_t));
		}
	}

	/* restore foreground color */
	RTGUI_DC_FC(dc) = fc;
}

/* blit a dc to a hardware dc */
static void rtgui_dc_buffer_blit(rtgui_dc_t* src,  int sx, int sy,
                                 rtgui_dc_t* dest, int dx, int dy, int w, int h)
{
	if(rtgui_dc_get_visible(dest) == RT_FALSE) return;

	if((dest->type == RTGUI_DC_HW) || (dest->type == RTGUI_DC_CLIENT))
	{
		rt_uint8_t *line_ptr, *pixels;
		rt_uint16_t index, pitch;
		rtgui_blit_line_func blit_line;
		rtgui_dc_buffer_t* dc_src = (rtgui_dc_buffer_t*)src;

		/* prepare pixel line */
		pixels = dc_src->pixel + sy * dc_src->pitch + sx * sizeof(rtgui_color_t);

		if(hw_driver->bits_per_pixel == sizeof(rtgui_color_t)*8)
		{
			/* it's the same byte per pixel, draw it directly */
			for(index = dy; index < dy + h; index++)
			{
				dest->engine->blit_line(dest, dx, dx + w, index, pixels);
				pixels += src->w * sizeof(rtgui_color_t);
			}
		}
		else
		{
			/* get blit line function */
			blit_line = rtgui_blit_line_get(hw_driver->bits_per_pixel/8, 4);
			/* calculate pitch */
			pitch = w * sizeof(rtgui_color_t);

			/* create line buffer */
			line_ptr = (rt_uint8_t*) rt_malloc(w * hw_driver->bits_per_pixel/8);
			if(line_ptr == RT_NULL) return;
			/* draw each line */
			for(index = dy; index < dy + h; index ++)
			{
				/* blit on line buffer */
				blit_line(line_ptr, (rt_uint8_t*)pixels, pitch);
				pixels += src->w * sizeof(rtgui_color_t);

				/* draw on hardware dc */
				dest->engine->blit_line(dest, dx, dx + w, index, line_ptr);
			}

			/* release line buffer */
			rt_free(line_ptr);
		}
	}
	else if(dest->type == RTGUI_DC_BUFFER)
	{
		/* From the buffer is copied to the buffer */
		rt_uint8_t *pixels_src, *pixels_dst;
		rt_uint16_t y, x;
		rtgui_dc_buffer_t *dc_src, *dc_dst;

		dc_src = (rtgui_dc_buffer_t*)src;
		dc_dst = (rtgui_dc_buffer_t*)dest;

		/* it's the same byte per pixel, draw it directly */
		for(y = 0; y < h; y++)
		{
			pixels_src = dc_src->pixel + (sy + y)*dc_src->pitch + sx*sizeof(rtgui_color_t);
			pixels_dst = dc_dst->pixel + (dy + y)*dc_dst->pitch + dx*sizeof(rtgui_color_t);

			for(x = 0; x < (w * sizeof(rtgui_color_t)); x++)
			{
				*pixels_dst++ = *pixels_src++;
			}
		}
	}
}

static void rtgui_dc_buffer_blit_line(rtgui_dc_t* dc, int x1, int x2, int y, rt_uint8_t* line_data)
{
	rtgui_color_t* color_ptr;
	rtgui_dc_buffer_t* dc_buffer = (rtgui_dc_buffer_t*)dc;

	RT_ASSERT(dc_buffer != RT_NULL);
	RT_ASSERT(line_data != RT_NULL);

	/* out of range */
	if((x1 > dc->w) || (y > dc->h)) return;
	/* check range */
	if(x2 > dc->w) x2 = dc->w;

	if(x1 > x2)_int_swap(x1, x2);

	color_ptr = (rtgui_color_t*)(dc_buffer->pixel + y * dc_buffer->pitch + x1 * sizeof(rtgui_color_t));
	rt_memcpy(color_ptr, line_data, (x2 - x1) * sizeof(rtgui_color_t));
}

static void rtgui_dc_buffer_set_gc(rtgui_dc_t* dc, rtgui_gc_t *gc)
{
	rtgui_dc_buffer_t* dc_buffer = (rtgui_dc_buffer_t*)dc;

	dc_buffer->gc = *gc;
}

static rtgui_gc_t *rtgui_dc_buffer_get_gc(rtgui_dc_t* dc)
{
	rtgui_dc_buffer_t* dc_buffer = (rtgui_dc_buffer_t*)dc;

	return &dc_buffer->gc;
}

static rt_bool_t rtgui_dc_buffer_get_visible(rtgui_dc_t* dc)
{
	return RT_TRUE;
}

static void rtgui_dc_buffer_get_rect(rtgui_dc_t* dc, rtgui_rect_t* rect)
{
	rect->x1 = rect->y1 = 0;

	rect->x2 = dc->w;
	rect->y2 = dc->h;
}
