#ifndef __RTGUI_SLIDER_H__
#define __RTGUI_SLIDER_H__

#include <rtgui/widgets/widget.h>
#include <rtgui/widgets/container.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rtgui_slider rtgui_slider_t;

DECLARE_CLASS_TYPE(slider);
/** Gets the type of a slider */
#define RTGUI_SLIDER_TYPE       (RTGUI_TYPE(slider))
/** Casts the object to an rtgui_slider */
#define RTGUI_SLIDER(obj)       (RTGUI_OBJECT_CAST((obj), RTGUI_SLIDER_TYPE, rtgui_slider_t))
/** Checks if the object is an rtgui_slider */
#define RTGUI_IS_SLIDER(obj)    (RTGUI_OBJECT_CHECK_TYPE((obj), RTGUI_SLIDER_TYPE))

struct rtgui_slider
{
	rtgui_widget_t parent;

	/* widget private data */
	int min, max, value, ticks;
	int thumb_width;
	int orient;
	void (*on_change)(pvoid wdt, rtgui_event_t *event);
};

rtgui_slider_t* rtgui_slider_create(pvoid parent, int left, int top, int w, int h, int orient, int min, int max);
void rtgui_slider_destroy(rtgui_slider_t* slider);
void rtgui_slider_ondraw(rtgui_slider_t* slider);
rt_bool_t rtgui_slider_event_handler(pvoid wdt, rtgui_event_t* event);

void rtgui_slider_set_range(rtgui_slider_t* slider, rt_size_t min, rt_size_t max);
void rtgui_slider_set_value(rtgui_slider_t* slider, rt_size_t value);
void rtgui_slider_set_orient(rtgui_slider_t* slider, int orient);

rt_size_t rtgui_slider_get_value(rtgui_slider_t* slider);

#ifdef __cplusplus
}
#endif

#endif
