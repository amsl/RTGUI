#ifndef __RTGUI_STATICLINE__H__
#define __RTGUI_STATICLINE__H__

#include <rtgui/widgets/widget.h>
#include <rtgui/widgets/container.h>

#ifdef __cplusplus
extern "C" {
#endif

DECLARE_CLASS_TYPE(staticline);

/** Gets the type of a sline */
#define RTGUI_STATICLINE_TYPE       (RTGUI_TYPE(staticline))//(rtgui_staticline_type_get())
/** Casts the object to an rtgui_staticline_t */
#define RTGUI_STATICLINE(obj)       (RTGUI_OBJECT_CAST((obj), RTGUI_STATICLINE_TYPE, rtgui_staticline_t))
/** Checks if the object is an rtgui_staticline_t */
#define RTGUI_IS_STATICLINE(obj)    (RTGUI_OBJECT_CHECK_TYPE((obj), RTGUI_STATICLINE_TYPE))

struct rtgui_staticline
{
	/* inherit from widget */
	rtgui_widget_t parent;

	int orient;
};
typedef struct rtgui_staticline rtgui_staticline_t;

rtgui_staticline_t *rtgui_staticline_create(pvoid wdt,int left,int top,int w,int len,int orient);
void rtgui_staticline_destroy(rtgui_staticline_t* sline);
void rtgui_staticline_ondraw(rtgui_staticline_t* sline);
rt_bool_t rtgui_staticline_event_handler(pvoid wdt, rtgui_event_t* event);

#ifdef __cplusplus
}
#endif

#endif

