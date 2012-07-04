/*
 * File      : image_bmp.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2010-08-10     Bernard      first version
 */
#ifndef __RTGUI_IMAGE_BMP_H__
#define __RTGUI_IMAGE_BMP_H__

#pragma  pack(2)

/* Compression encodings for BMP files */
#ifndef BI_RGB
#define BI_RGB			0
#define BI_RLE8			1
#define BI_RLE4			2
#define BI_BITFIELDS	3
#endif

#define hw_driver		(rtgui_graphic_driver_get_default())

struct rtgui_image_bmp
{
	rt_bool_t is_loaded;

	rt_uint32_t Rmask;
	rt_uint32_t Gmask;
	rt_uint32_t Bmask;

	rt_size_t   pixel_offset;

	rt_uint8_t  byte_per_pixel;
	rt_uint8_t  pad;
	rt_uint8_t  ExpandBMP;

	rt_uint8_t *pixels;
	rt_uint8_t *line_pixels;
	rt_uint32_t pitch;

	struct rtgui_filerw* filerw;
};

struct rtgui_image_bmp_header
{
	/* The Win32 BMP file header (14 bytes) */
	char   magic[2];
	rt_uint32_t bfSize;
	rt_uint16_t bfReserved1;
	rt_uint16_t bfReserved2;
	rt_uint32_t bfOffBits;

	/* The Win32 BITMAPINFOHEADER struct (40 bytes) */
	rt_uint32_t biSize;
	rt_int32_t  biWidth;
	rt_int32_t  biHeight;
	rt_uint16_t biPlanes;
	rt_uint16_t biBitCount;
	rt_uint32_t biCompression;
	rt_uint32_t biSizeImage;
	rt_int32_t  biXPelsPerMeter;
	rt_int32_t  biYPelsPerMeter;
	rt_uint32_t biClrUsed;
	rt_uint32_t biClrImportant;
};

void rtgui_image_bmp_init(void);
void rtgui_image_bmp_header_cfg(struct rtgui_image_bmp_header *bhr, rt_int32_t w, rt_int32_t h, rt_uint16_t bits_per_pixel);
void bmp_create(const char *filename);

#endif
