#ifndef __RTGUI_PROPEL_H__
#define __RTGUI_PROPEL_H__

#include <rtgui/widgets/widget.h>
#include <rtgui/widgets/container.h>

#ifdef __cplusplus
extern "C" {
#endif

DECLARE_CLASS_TYPE(spin);

/** Gets the type of a spin */
#define RTGUI_SPIN_TYPE       (RTGUI_TYPE(spin))
/** Casts the object to an rtgui_spin_t */
#define RTGUI_SPIN(obj)       (RTGUI_OBJECT_CAST((obj), RTGUI_SPIN_TYPE, rtgui_spin_t))
/** Checks if the object is an rtgui_label_t */
#define RTGUI_IS_SPIN(obj)    (RTGUI_OBJECT_CHECK_TYPE((obj), RTGUI_SPIN_TYPE))

#define PROPEL_FLAG_MASK		0x000F
#define PROPEL_FLAG_NONE    	0x0000
#define PROPEL_FLAG_LEFT  		0x0001
#define PROPEL_FLAG_RIGHT		0x0002
#define PROPEL_FLAG_UP			0x0004
#define PROPEL_FLAG_DOWN		0x0008

#define PROPEL_UNVISIBLE_MASK	0x00F0
#define PROPEL_UNVISIBLE_LEFT 	0x0010
#define PROPEL_UNVISIBLE_RIGHT	0x0020
#define PROPEL_UNVISIBLE_UP		0x0040
#define PROPEL_UNVISIBLE_DOWN	0x0080

/*
 * the spin widget
 */
struct rtgui_spin
{
	rtgui_widget_t 	parent;
	rt_uint32_t 	orient;
	rt_uint32_t     flag;
	rt_int16_t		range_min;
	rt_int16_t		range_max;
	rt_uint32_t*	bind;
	pvoid 			widget_link;
	rt_bool_t (*on_click) (pvoid wdt, rtgui_event_t* event);
};
typedef struct rtgui_spin rtgui_spin_t;

rtgui_spin_t* rtgui_spin_create(pvoid parent, int left, int top, int w, int h, int orient);
void rtgui_spin_destroy(rtgui_spin_t* spin);
rt_bool_t rtgui_spin_event_handler(pvoid wdt, rtgui_event_t* event);
void rtgui_spin_bind(rtgui_spin_t *spin, rt_uint32_t *var);
void rtgui_spin_unbind(rtgui_spin_t *spin);

#ifdef __cplusplus
}
#endif

#endif

