/*
 * File      : listbox.h
 * This file is part of RTGUI in RT-Thread RTOS
 * COPYRIGHT (C) 2010, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2010-01-06     Bernard      first version
 */

#ifndef __RTGUI_LISTBOX_H__
#define __RTGUI_LISTBOX_H__

#include <rtgui/widgets/container.h>

#ifdef __cplusplus
extern "C" {
#endif

struct rtgui_listbox_item
{
    char *name;
	rtgui_image_t *image;
	volatile rt_bool_t	ext_flag;
};

typedef struct rtgui_listbox_item	rtgui_listbox_item_t;

DECLARE_CLASS_TYPE(listbox);
/** Gets the type of a list box */
#define RTGUI_LISTBOX_TYPE		(RTGUI_TYPE(listbox))
/** Casts the object to a list box */
#define RTGUI_LISTBOX(obj)		(RTGUI_OBJECT_CAST((obj), RTGUI_LISTBOX_TYPE, rtgui_listbox_t))
/** Checks if the object is a list box */
#define RTGUI_IS_LISTBOX(obj)	(RTGUI_OBJECT_CHECK_TYPE((obj), RTGUI_LISTBOX_TYPE))

typedef struct rtgui_listbox rtgui_listbox_t;

struct rtgui_listbox
{
	rtgui_container_t parent;

	/* widget private data */
	rt_int16_t  		item_count;	    /* total number of items */
	rt_int16_t  		item_per_page;  /* the number of item in a page */
	rt_int16_t			item_size;		/* item size */
    rt_int16_t  		first_item;     /* first item */
	rt_int16_t  		now_item;       /* now item */
	rt_int16_t  		old_item;       /* old item */
	rtgui_color_t		select_fc;		/* select fc color */
	rtgui_color_t		select_bc;	    /* select bc color */

	rt_bool_t			ispopup;	    /* is popup widget */
	rt_bool_t			vindex;  		/* display index */
	pvoid 				widget_link;	/* link widget */
	rtgui_scrollbar_t  	*scroll;
	volatile rt_bool_t	multi_sel;		/* multi selected flag */
	rt_bool_t			multi_valid;
    rtgui_listbox_item_t *items;	    /* items array */
	
	/* item event handler */
	rt_bool_t (*on_item)(pvoid wdt, rtgui_event_t* event);
	rt_bool_t (*updown)(pvoid wdt, rtgui_event_t* event);
};

rtgui_listbox_t* rtgui_listbox_create(pvoid wdt, int left,int top,int w,int h);
void rtgui_listbox_destroy(rtgui_listbox_t* box);

void rtgui_listbox_update(rtgui_listbox_t* box);
rt_bool_t rtgui_listbox_event_handler(pvoid wdt, rtgui_event_t* event);
void rtgui_listbox_set_onitem(rtgui_listbox_t* box, rtgui_event_handler_ptr func);
void rtgui_listbox_set_updown(rtgui_listbox_t* box, rtgui_event_handler_ptr func);
void rtgui_listbox_set_items(rtgui_listbox_t* box, rtgui_listbox_item_t* items, rt_int16_t count);
void rtgui_listbox_update_loc(rtgui_listbox_t* box, rt_int16_t loc);
void rtgui_listbox_add_item(rtgui_listbox_t* box, rtgui_listbox_item_t* item, rt_bool_t update);
void rtgui_listbox_insert_item(rtgui_listbox_t* box, rtgui_listbox_item_t* item, rt_int16_t item_num);
void rtgui_listbox_del_item(rtgui_listbox_t* box, rt_int16_t item_num);
rt_uint32_t rtgui_listbox_get_count(rtgui_listbox_t* box);
void rtgui_listbox_clear_items(rtgui_listbox_t* box);
rt_bool_t rtgui_listbox_unfocus(pvoid wdt, rtgui_event_t* event);

#ifdef __cplusplus
}
#endif

#endif

