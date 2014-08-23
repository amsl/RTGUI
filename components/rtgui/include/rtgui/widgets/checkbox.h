#ifndef __RTGUI_CHECKBOX_H__
#define __RTGUI_CHECKBOX_H__

#include <rtgui/widgets/label.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CHECK_BOX_W		13
#define CHECK_BOX_H		13

DECLARE_CLASS_TYPE(checkbox);
#define RTGUI_CHECKBOX_TYPE     (RTGUI_TYPE(checkbox))
#define RTGUI_CHECKBOX(obj)     (RTGUI_OBJECT_CAST((obj), RTGUI_CHECKBOX_TYPE, rtgui_checkbox_t))
#define RTGUI_IS_CHECKBOX(obj)	(RTGUI_OBJECT_CHECK_TYPE((obj), RTGUI_CHECKBOX_TYPE))

struct rtgui_checkbox
{
	/* inherit from label */
	rtgui_label_t parent;

	/* check box status */
	int value;
};
typedef struct rtgui_checkbox rtgui_checkbox_t;

rtgui_checkbox_t* rtgui_checkbox_create(pvoid wdt,const char* text, rt_bool_t checked,int left,int top);
void rtgui_checkbox_destroy(rtgui_checkbox_t* checkbox);

void rtgui_checkbox_set_checked(rtgui_checkbox_t* checkbox, rt_bool_t checked);
rt_bool_t rtgui_checkbox_get_checked(rtgui_checkbox_t* checkbox);
void rtgui_checkbox_ondraw(rtgui_checkbox_t* checkbox);
rt_bool_t rtgui_checkbox_event_handler(pvoid wdt, rtgui_event_t* event);
void rtgui_checkbox_set_text(rtgui_checkbox_t *box, const char* text);

#ifdef __cplusplus
}
#endif

#endif
