#ifndef __MY_WIDGET_H__
#define __MY_WIDGET_H__

#include <rtgui/rtgui.h>
#include <rtgui/widgets/widget.h>

#define MYWIDGET_STATUS_ON  1
#define MYWIDGET_STATUS_OFF 0

DECLARE_CLASS_TYPE(mywidget);
#define RTGUI_MYWIDGET_TYPE       (RTGUI_TYPE(mywidget))
#define RTGUI_MYWIDGET(obj)       (RTGUI_OBJECT_CAST((obj), RTGUI_MYWIDGET_TYPE, rtgui_mywidget_t))
#define RTGUI_IS_MYWIDGET(obj)    (RTGUI_OBJECT_CHECK_TYPE((obj), RTGUI_MYWIDGET_TYPE))

struct rtgui_mywidget
{
    struct rtgui_widget parent;

    rt_uint8_t status;
};
typedef struct rtgui_mywidget rtgui_mywidget_t;

struct rtgui_mywidget *rtgui_mywidget_create(rtgui_rect_t *r);
void rtgui_mywidget_destroy(struct rtgui_mywidget *me);

rt_bool_t rtgui_mywidget_event_handler(struct rtgui_object *object, struct rtgui_event *event);

#endif
