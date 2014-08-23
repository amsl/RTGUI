/*
 * File      : driver.c
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
#include <rtthread.h>
#include <rtgui/driver.h>

struct rtgui_gdev _driver;

extern const struct rtgui_graphic_driver_ops *rtgui_fb_get_ops(void);

/* get default driver */
struct rtgui_gdev* rtgui_gdev_get()
{
	return &_driver;
}

void rtgui_gdev_get_rect(const struct rtgui_gdev *driver, rtgui_rect_t *rect)
{
	RT_ASSERT(rect != RT_NULL);
	RT_ASSERT(driver != RT_NULL);

	rect->x1 = rect->y1 = 0;
	rect->x2 = driver->width;
	rect->y2 = driver->height;
}

rt_err_t rtgui_gdev_set(rt_device_t device)
{
	rt_err_t result;
	struct rt_device_graphic_info info;

	/* get framebuffer address */
	result = rt_device_control(device, RTGRAPHIC_CTRL_GET_INFO, &info);
	if(result != RT_EOK)
	{
		/* get device information failed */
		return -RT_ERROR;
	}

	/* initialize framebuffer driver */
	_driver.device = device;
	_driver.bits_per_pixel = info.bits_per_pixel;
	_driver.width = info.width;
	_driver.height = info.height;
	_driver.pitch = _driver.width * _driver.bits_per_pixel/8;
	_driver.framebuffer = info.framebuffer;

	/* is a frame buffer device */
	_driver.ops = rtgui_fb_get_ops();

	return RT_EOK;
}

/* screen update */
void rtgui_gdev_update(const struct rtgui_gdev* driver, rtgui_rect_t *rect)
{
	struct rt_device_rect_info rect_info;

	rect_info.x = rect->x1;
	rect_info.y = rect->y1;
	rect_info.w = rect->x2 - rect->x1;
	rect_info.h = rect->y2 - rect->y1;
	rt_device_control(driver->device, RTGRAPHIC_CTRL_RECT_UPDATE, &rect_info);
}

/* get video frame buffer */
rt_uint8_t* rtgui_gdev_get_framebuffer(const struct rtgui_gdev* driver)
{
	return (rt_uint8_t*)driver->framebuffer;
}

