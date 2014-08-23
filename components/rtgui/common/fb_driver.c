#include <rtgui/rtgui_system.h>
#include <rtgui/driver.h>

#define GET_PIXEL(dst, x, y, type)	\
	(type *)((rt_uint8_t*)((dst)->framebuffer) + (y)*(dst)->pitch + (x)*((dst)->bits_per_pixel/8))

static void _rgb565_set_pixel(rtgui_color_t *c, int x, int y)
{
	*GET_PIXEL(rtgui_gdev_get(), x, y, rt_uint16_t) = (*c);
}

static void _rgb565_get_pixel(rtgui_color_t *c, int x, int y)
{
	rt_uint16_t pixel;

	pixel = *GET_PIXEL(rtgui_gdev_get(), x, y, rt_uint16_t);

	/* get pixel from color */
	*c = pixel;
}

static void _rgb565_draw_hline(rtgui_color_t *c, int x1, int x2, int y)
{
	rt_ubase_t index;
	rt_uint16_t pixel;
	rt_uint16_t *pixel_ptr;

	/* get pixel from color */
	pixel = (*c);

	/* get pixel pointer in framebuffer */
	pixel_ptr = GET_PIXEL(rtgui_gdev_get(), x1, y, rt_uint16_t);

	for(index = x1; index < x2; index ++)
	{
		*pixel_ptr = pixel;
		pixel_ptr ++;
	}
}

static void _rgb565_draw_vline(rtgui_color_t *c, int x , int y1, int y2)
{
	rt_uint8_t *dst;
	rt_uint16_t pixel;
	rt_ubase_t index;

	pixel = (*c);
	dst = GET_PIXEL(rtgui_gdev_get(), x, y1, rt_uint8_t);
	for(index = y1; index < y2; index ++)
	{
		*(rt_uint16_t*)dst = pixel;
		dst += rtgui_gdev_get()->pitch;
	}
}

/* draw raw hline */
static void framebuffer_draw_raw_hline(rt_uint8_t *pixels, int x1, int x2, int y)
{
	rt_uint8_t *dst;

	dst = GET_PIXEL(rtgui_gdev_get(), x1, y, rt_uint8_t);
	rt_memcpy(dst, pixels, (x2 - x1) * (rtgui_gdev_get()->bits_per_pixel/8));
}

const struct rtgui_graphic_driver_ops _framebuffer_rgb565_ops =
{
	_rgb565_set_pixel,
	_rgb565_get_pixel,
	_rgb565_draw_hline,
	_rgb565_draw_vline,
	framebuffer_draw_raw_hline,
};

const struct rtgui_graphic_driver_ops *rtgui_fb_get_ops(void)
{
	return &_framebuffer_rgb565_ops;
}


