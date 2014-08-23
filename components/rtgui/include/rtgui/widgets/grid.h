/*
 * File      : grid.h
 * This file is part of RTGUI in RT-Thread RTOS
 * COPYRIGHT (C) 2010, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-08-11	  amsl
 */

#ifndef __RTGUI_GRID_H__
#define __RTGUI_GRID_H__

#include <rtgui/widgets/container.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rtgui_grid_item	rtgui_grid_item_t;

struct rtgui_grid_item
{
    char *name;
};

DECLARE_CLASS_TYPE(grid);

/** Gets the type of a grid */
#define RTGUI_GRID_TYPE	(RTGUI_TYPE(grid))
/** Casts the object to a grid */
#define RTGUI_GRID(obj)	(RTGUI_OBJECT_CAST((obj), RTGUI_GRID_TYPE, rtgui_grid_t))
/** Checks if the object is a grid */
#define RTGUI_IS_GRID(obj)	(RTGUI_OBJECT_CHECK_TYPE((obj), RTGUI_GRID_TYPE))

typedef struct rtgui_grid rtgui_grid_t;

struct rtgui_grid
{
	rtgui_container_t parent;
	/* widget private data */
	rt_uint16_t	 	rows,cols;		/* rows & cols */
	rt_uint16_t  	row_per_page,col_per_page;
	rt_uint16_t	 	item_height;		/* item height */
    rt_uint16_t  	first_row,frist_col; /* first row */
	rt_uint16_t  	now_row,old_row;/* now item */
	rt_uint16_t  	now_col,old_col;/* old item */
	rt_uint16_t	 	*colw;	 /* col width */
	rt_bool_t		grid_head; /* boolean,reserve */
	rt_bool_t    	grid_line; /* boolean */
	rtgui_color_t 	grid_color;
	pvoid 		 	widget_link;		/* link widget */
	rtgui_scrollbar_t  	*vbar;			
    rtgui_grid_item_t  *items;	/* items array */
	/* item event handler */
	rt_bool_t (*on_item)(pvoid wdt, rtgui_event_t* event);
};

rtgui_grid_t* rtgui_grid_create(pvoid wdt, int left,int top,int w,int h,int rows,int cols);
void rtgui_grid_destroy(rtgui_grid_t* grid);

void rtgui_grid_update(rtgui_grid_t* grid);
rt_bool_t rtgui_grid_event_handler(pvoid wdt, rtgui_event_t* event);
void rtgui_grid_set_onitem(rtgui_grid_t* grid, rtgui_event_handler_ptr func);
void rtgui_grid_set_grid(rtgui_grid_t *grid, int row, int col, char *text, int update);
void rtgui_grid_set_colw(rtgui_grid_t* grid, int col, int width);

int rtgui_grid_get_rows(rtgui_grid_t* grid);
int rtgui_grid_get_cols(rtgui_grid_t* grid);
void rtgui_grid_append_row(rtgui_grid_t* grid, int update);
void rtgui_grid_delete_row(rtgui_grid_t* grid, int update);
void rtgui_grid_insert_row(rtgui_grid_t* grid, int update);
void rtgui_grid_update_grid(rtgui_grid_t* grid, int row, int col);
void rtgui_grid_clear_grid(rtgui_grid_t *grid, int row, int col);

void rtgui_grid_clear_items(rtgui_grid_t* grid);
rt_bool_t rtgui_grid_unfocus(pvoid wdt, rtgui_event_t* event);

#ifdef __cplusplus
}
#endif

#endif

