/*
 * File      : panel.h
 * This file is part of RTGUI in RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-10-04     Bernard      first version
 */

#ifndef __RT_PANEL_H__
#define __RT_PANEL_H__

#include <rtgui/widgets/container.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RTGUI_PANEL_STATUS_MODAL	(1ul << 0)	/* panel is modal mode showing */	

#define RTGUI_PANEL_IS_MODAL_MODE(panel)	(panel->status & RTGUI_PANEL_STATUS_MODAL)

typedef struct rtgui_panel rtgui_panel_t;

DECLARE_CLASS_TYPE(panel);

#define RTGUI_PANEL_TYPE       (RTGUI_TYPE(panel))
#define RTGUI_PANEL(obj)       (RTGUI_OBJECT_CAST((obj), RTGUI_PANEL_TYPE, rtgui_panel_t))
#define RTGUI_IS_PANEL(obj)    (RTGUI_OBJECT_CHECK_TYPE((obj), RTGUI_PANEL_TYPE))

struct rtgui_panel
{
	rtgui_container_t   parent;		
	pvoid				modal_widget;
	rt_uint32_t 		status;
	/* gui manage toplevel window */		
	rtgui_image_t*		image;		/* background image */
};

extern rtgui_rect_t *external_clip_rect;
extern rt_uint32_t	external_clip_size;

rtgui_panel_t* rtgui_panel_create(int left, int top, int w, int h);

rt_thread_t* rtgui_panel_thread_get(void);
void rtgui_panel_set(rtgui_panel_t *panel);
rtgui_panel_t* rtgui_panel_get(void);
void rtgui_panel_ondraw(rtgui_panel_t* panel);
void rtgui_panel_draw_by_rect(rtgui_panel_t* panel, rtgui_rect_t *rect);
rt_bool_t rtgui_panel_event_handler(pvoid wdt,rtgui_event_t *event);
void rtgui_panel_show(rtgui_panel_t *panel);
void rtgui_panel_event_loop(rtgui_panel_t *panel);

extern rtgui_panel_t *rtgui_panel;

#ifdef __cplusplus
}
#endif

#endif
