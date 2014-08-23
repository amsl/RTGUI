/*
 * File      : dc_hw.c
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

#include <rtgui/dc_hw.h>
#include <rtgui/widgets/widget.h>

static void rtgui_dc_hw_draw_point(rtgui_dc_t* dc, int x, int y);
static void rtgui_dc_hw_draw_color_point(rtgui_dc_t* dc, int x, int y, rtgui_color_t color);
static void rtgui_dc_hw_draw_hline(rtgui_dc_t* dc, int x1, int x2, int y);
static void rtgui_dc_hw_draw_vline(rtgui_dc_t* dc, int x, int y1, int y2);
static void rtgui_dc_hw_fill_rect(rtgui_dc_t* dc, rtgui_rect_t* rect);
static void rtgui_dc_hw_blit_line(rtgui_dc_t* dc, int x1, int x2, int y, rt_uint8_t* line_data);
static void rtgui_dc_hw_blit(rtgui_dc_t* src, int sx, int sy, rtgui_dc_t* dest, int dx, int dy, int w, int h);
static void rtgui_dc_hw_set_gc(rtgui_dc_t* dc, rtgui_gc_t *gc);
static rtgui_gc_t *rtgui_dc_hw_get_gc(rtgui_dc_t* dc);
static rt_bool_t rtgui_dc_hw_fini(rtgui_dc_t* dc);
static rt_bool_t rtgui_dc_hw_get_visible(rtgui_dc_t* dc);
static void rtgui_dc_hw_get_rect(rtgui_dc_t* dc, rtgui_rect_t* rect);

const dc_engine_t dc_hw_engine =
{
	rtgui_dc_hw_draw_point,
	rtgui_dc_hw_draw_color_point,
	rtgui_dc_hw_draw_vline,
	rtgui_dc_hw_draw_hline,
	rtgui_dc_hw_fill_rect,
	rtgui_dc_hw_blit_line,
	rtgui_dc_hw_blit,

	rtgui_dc_hw_set_gc,
	rtgui_dc_hw_get_gc,

	rtgui_dc_hw_get_visible,
	rtgui_dc_hw_get_rect,

	rtgui_dc_hw_fini,
};

extern struct rt_mutex cursor_mutex;
extern void rtgui_mouse_show_cursor(void);
extern void rtgui_mouse_hide_cursor(void);
rtgui_dc_t* rtgui_dc_hw_create(rtgui_widget_t* owner)
{
	rtgui_dc_hw_t* dc_hw;
	rtgui_widget_t* widget;

	/* adjudge owner */
	if(owner == RT_NULL || owner->toplevel == RT_NULL) return RT_NULL;

	/* set init visible as true */
	RTGUI_WIDGET_DC_SET_VISIBLE(owner);
	
	/* check widget visible */
	widget = owner;
	while(widget != RT_NULL)
	{
		if(RTGUI_WIDGET_IS_HIDE(widget))
		{
			RTGUI_WIDGET_DC_SET_UNVISIBLE(owner);
			return RT_NULL;
		}

		widget = widget->parent;
	}

	if(!RTGUI_WIDGET_IS_DC_VISIBLE(owner)) return RT_NULL;

	/* create DC */
	dc_hw = (rtgui_dc_hw_t*) rt_malloc(sizeof(rtgui_dc_hw_t));
	if(dc_hw == RT_NULL)return RT_NULL;

	dc_hw->parent.type = RTGUI_DC_HW;
	dc_hw->parent.w = RC_W(owner->extent);
	dc_hw->parent.h = RC_H(owner->extent);
	dc_hw->parent.engine = &dc_hw_engine;
	dc_hw->owner = owner;
	dc_hw->hw_drv = hw_driver;

	return &(dc_hw->parent);
}

static rt_bool_t rtgui_dc_hw_fini(rtgui_dc_t* dc)
{
	rtgui_dc_hw_t* dc_hw;

	if(dc == RT_NULL || dc->type != RTGUI_DC_HW) return RT_FALSE;

	dc_hw = (rtgui_dc_hw_t*)dc;

	/* release hardware dc */
	rt_free(dc_hw);

	return RT_TRUE;
}

/*
 * draw a logic point on device
 */
static void rtgui_dc_hw_draw_point(rtgui_dc_t* dc, int x, int y)
{
	rtgui_dc_hw_t* dc_hw;

	RT_ASSERT(dc != RT_NULL);
	dc_hw = (rtgui_dc_hw_t*) dc;

	x = x + dc_hw->owner->extent.x1;
	if(x < 0 || x >= dc_hw->owner->extent.x2) return;
	y = y + dc_hw->owner->extent.y1;
	if(y < 0 || y >= dc_hw->owner->extent.y2) return;

	/* draw this point */
	dc_hw->hw_drv->ops->set_pixel(&(dc_hw->owner->gc.fc), x, y);
}

static void rtgui_dc_hw_draw_color_point(rtgui_dc_t* dc, int x, int y, rtgui_color_t color)
{
	rtgui_dc_hw_t* dc_hw;

	RT_ASSERT(dc != RT_NULL);
	dc_hw = (rtgui_dc_hw_t*) dc;

	x = x + dc_hw->owner->extent.x1;
	if(x < 0 || x >= dc_hw->owner->extent.x2) return;
	y = y + dc_hw->owner->extent.y1;
	if(y < 0 || y >= dc_hw->owner->extent.y2) return;

	/* draw this point */
	dc_hw->hw_drv->ops->set_pixel(&color, x, y);
}

/*
 * draw a logic vertical line on device
 */
static void rtgui_dc_hw_draw_vline(rtgui_dc_t* dc, int x, int y1, int y2)
{
	rtgui_dc_hw_t* dc_hw;

	RT_ASSERT(dc != RT_NULL);
	dc_hw = (rtgui_dc_hw_t*) dc;

	x = x + dc_hw->owner->extent.x1;
	y1 = y1 + dc_hw->owner->extent.y1;
	y2 = y2 + dc_hw->owner->extent.y1;
	if (y1 > y2) _int_swap(y1, y2);

	/* draw vline */
	dc_hw->hw_drv->ops->draw_vline(&(dc_hw->owner->gc.fc), x, y1, y2);
}

/*
 * draw a logic horizontal line on device
 */
static void rtgui_dc_hw_draw_hline(rtgui_dc_t* dc, int x1, int x2, int y)
{
	rtgui_dc_hw_t* dc_hw;

	RT_ASSERT(dc != RT_NULL);
	dc_hw = (rtgui_dc_hw_t*) dc;

	/* convert logic to device */
	x1 = x1 + dc_hw->owner->extent.x1;
	x2 = x2 + dc_hw->owner->extent.x1;
	if (x1 > x2) _int_swap(x1, x2);
	y  = y + dc_hw->owner->extent.y1;

	/* draw hline */
	dc_hw->hw_drv->ops->draw_hline(&(dc_hw->owner->gc.fc), x1, x2, y);
}

static void rtgui_dc_hw_fill_rect(rtgui_dc_t* dc, rtgui_rect_t* rect)
{
	rtgui_color_t color;
	register rt_base_t index, x1, x2;
	rtgui_dc_hw_t* dc_hw;

	RT_ASSERT(dc != RT_NULL);
	dc_hw = (rtgui_dc_hw_t*) dc;

	/* get background color */
	color = dc_hw->owner->gc.bc;
	/* convert logic to device */
	x1 = rect->x1 + dc_hw->owner->extent.x1;
	x2 = rect->x2 + dc_hw->owner->extent.x1;

	/* fill rect */
	for(index = dc_hw->owner->extent.y1 + rect->y1; index < dc_hw->owner->extent.y1 + rect->y2; index ++)
	{
		dc_hw->hw_drv->ops->draw_hline(&color, x1, x2, index);
	}
}

static void rtgui_dc_hw_blit_line(rtgui_dc_t* dc, int x1, int x2, int y, rt_uint8_t* line_data)
{
	rtgui_dc_hw_t* dc_hw;

	RT_ASSERT(dc != RT_NULL);
	dc_hw = (rtgui_dc_hw_t*) dc;

	/* convert logic to device */
	x1 = x1 + dc_hw->owner->extent.x1;
	x2 = x2 + dc_hw->owner->extent.x1;
	if(x1 > x2) _int_swap(x1, x2);
	y  = y  + dc_hw->owner->extent.y1;

	dc_hw->hw_drv->ops->draw_raw_hline(line_data, x1, x2, y);
}

static void rtgui_dc_hw_blit(rtgui_dc_t* src, int sx, int sy, rtgui_dc_t* dest, int dx, int dy, int w, int h)
{
	/* not blit in hardware dc */
	return ;
}

static void rtgui_dc_hw_set_gc(rtgui_dc_t* dc, rtgui_gc_t *gc)
{
	rtgui_dc_hw_t* dc_hw;

	RT_ASSERT(dc != RT_NULL);
	dc_hw = (rtgui_dc_hw_t*) dc;

	/* set gc */
	dc_hw->owner->gc = *gc;
}

static rtgui_gc_t* rtgui_dc_hw_get_gc(rtgui_dc_t* dc)
{
	rtgui_dc_hw_t* dc_hw;

	RT_ASSERT(dc != RT_NULL);
	dc_hw = (rtgui_dc_hw_t*) dc;

	return &(dc_hw->owner->gc);
}

static rt_bool_t rtgui_dc_hw_get_visible(rtgui_dc_t* dc)
{
	rtgui_dc_hw_t* dc_hw;

	RT_ASSERT(dc != RT_NULL);
	dc_hw = (rtgui_dc_hw_t*) dc;

	if(!RTGUI_WIDGET_IS_DC_VISIBLE(dc_hw->owner)) return RT_FALSE;

	return RT_TRUE;
}

static void rtgui_dc_hw_get_rect(rtgui_dc_t* dc, rtgui_rect_t* rect)
{
	rtgui_dc_hw_t* dc_hw;

	RT_ASSERT(dc != RT_NULL);
	dc_hw = (rtgui_dc_hw_t*) dc;

	/* get owner */
	rtgui_widget_get_rect(dc_hw->owner, rect);
}
