/*
 * File      : image.h
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
#ifndef __RTGUI_IMAGE_H__
#define __RTGUI_IMAGE_H__

#include <rtgui/dc.h>
#include <rtgui/filerw.h>

#ifdef __cplusplus
extern "C" {
#endif

enum rtgui_img_zoom
{  
	RTGUI_IMG_ZOOM_NEAREST,
	RTGUI_IMG_ZOOM_BILINEAR
};

typedef struct rtgui_image rtgui_image_t;

struct rtgui_image_engine
{
	const char* name; 
	rtgui_list_t list;

	/* image engine function  */
	rt_bool_t (*image_check)(struct rtgui_filerw* file);

	rt_bool_t (*image_load)(rtgui_image_t* image, struct rtgui_filerw* file, rt_bool_t load);
	void (*image_unload)(rtgui_image_t* image);

	void (*image_blit)(rtgui_image_t *image, rtgui_dc_t *dc, rtgui_rect_t *rect); 
	void (*image_paste)(rtgui_image_t *image, rtgui_dc_t *dc, rtgui_rect_t *rect, rtgui_color_t shield_color);
	struct rtgui_image* (*image_zoom)(struct rtgui_image* image, float scalew, float scaleh, rt_uint32_t mode);
	struct rtgui_image* (*image_rotate)(struct rtgui_image* image, float angle);
};

struct rtgui_image_palette
{
	rt_uint32_t* colors;
	rt_uint32_t ncolors;
};
typedef struct rtgui_image_palette rtgui_image_palette_t;

struct rtgui_image
{
	/* image metrics */
	rt_uint16_t w, h;

	/* image engine */
	const struct rtgui_image_engine* engine;

	/* image palette */
	rtgui_image_palette_t* palette;

	/* image private data */
	void* data;
};

/* init rtgui image system */
void rtgui_system_image_init(void);

rtgui_image_t* rtgui_image_create_from_file(const char* type, const char* filename, rt_bool_t load);
rtgui_image_t* rtgui_image_create_from_mem(const char* type, const rt_uint8_t* data, rt_size_t length, rt_bool_t load);
void rtgui_image_destroy(rtgui_image_t* image);

/* register an image engine */
void rtgui_image_register_engine(struct rtgui_image_engine* engine);

/* blit an image */
void rtgui_image_blit(rtgui_image_t* image, rtgui_dc_t *dc, rtgui_rect_t* rect);
void rtgui_image_paste(rtgui_image_t *image, rtgui_dc_t *dc, rtgui_rect_t *rect, rtgui_color_t shield_color);
struct rtgui_image_palette* rtgui_image_palette_create(rt_uint32_t ncolors);
rtgui_image_t* rtgui_image_zoom(rtgui_image_t* image, float scalew, float scaleh, rt_uint32_t mode);
rtgui_image_t* rtgui_image_rotate(rtgui_image_t* image, float angle);

#ifdef __cplusplus
}
#endif

#endif
