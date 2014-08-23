#ifndef __RTGUI_DC_BUFFER_H__
#define __RTGUI_DC_BUFFER_H__

#include <rtgui/dc_client.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rtgui_dc_buffer rtgui_dc_buffer_t;

struct rtgui_dc_buffer
{
	rtgui_dc_t parent;

	/* graphic context */
	rtgui_gc_t gc;

	/* width and height */
	rt_uint16_t pitch;

	/* blit info */
	rtgui_region_t clip;

	/* pixel data */
	rt_uint8_t* pixel;
};

#ifdef __cplusplus
}
#endif

#endif

