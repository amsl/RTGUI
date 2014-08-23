#ifndef __RTGUI_PROGRESSBAR_H__
#define __RTGUI_PROGRESSBAR_H__

#include <rtgui/widgets/widget.h>
#include <rtgui/widgets/container.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rtgui_progressbar rtgui_progressbar_t;

DECLARE_CLASS_TYPE(progressbar);
/** Gets the type of a progressbar */
#define RTGUI_PROGRESSBAR_TYPE       (RTGUI_TYPE(progressbar))
/** Casts the object to a rtgui_progressbar */
#define RTGUI_PROGRESSBAR(obj)       (RTGUI_OBJECT_CAST((obj), RTGUI_PROGRESSBAR_TYPE, rtgui_progressbar_t))
/** Checks if the object is a rtgui_progressbar */
#define RTGUI_IS_PROGRESSBAR(obj)    (RTGUI_OBJECT_CHECK_TYPE((obj), RTGUI_PROGRESSBAR_TYPE))

#define DEFAULT_WIDTH           100
#define DEFAULT_HEIGHT          20

struct rtgui_progressbar
{
	rtgui_widget_t parent;

	int orient;

    int range;
    int position;
};

rtgui_progressbar_t* rtgui_progressbar_create(pvoid parent, int orient, int range, int left, int top, int w, int h);
void rtgui_progressbar_destroy(rtgui_progressbar_t* p_bar);
void rtgui_progressbar_ondraw(rtgui_progressbar_t* bar);
rt_bool_t rtgui_progressbar_event_handler(pvoid wdt, rtgui_event_t* event);

void rtgui_progressbar_set_value(rtgui_progressbar_t *p_bar, int value);
int rtgui_progressbar_get_value(rtgui_progressbar_t *p_bar);
void rtgui_progressbar_set_range(rtgui_progressbar_t *p_bar, int range);
int rtgui_progressbar_get_range(rtgui_progressbar_t *p_bar);

#ifdef __cplusplus
}
#endif

#endif
