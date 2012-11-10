#include "demo_view.h"
#include <rtgui/rtgui_system.h>
#include <rtgui/widgets/label.h>
#include <rtgui/widgets/progressbar.h>

static rtgui_progressbar_t *hbar;
static rtgui_progressbar_t *vbar;
static rtgui_timer_t *bar_timer = RT_NULL;

void progressbar_timeout(struct rtgui_timer *timer, void *parameter)
{
    static rt_uint32_t value = 0;

    value++;
    if (value == 100)
        value = 0;

    rtgui_progressbar_set_value(hbar, value);
    rtgui_progressbar_set_value(vbar, value);
}

static rt_bool_t start_timer(struct rtgui_object *object, struct rtgui_event *event)
{
    if (bar_timer != RT_NULL)
        rtgui_timer_start(bar_timer);
    return RT_TRUE;
}

static rt_bool_t stop_timer(struct rtgui_object *object, struct rtgui_event *event)
{
    if (bar_timer != RT_NULL)
        rtgui_timer_stop(bar_timer);
    return RT_TRUE;
}

static rt_bool_t progressbar_event_handler(struct rtgui_object *object, rtgui_event_t *event)
{
    struct rtgui_widget *widget = RTGUI_WIDGET(object);

    if (event->type == RTGUI_EVENT_SHOW)
    {
        rtgui_container_event_handler(object, event);
        start_timer(object, event);
    }
    else if (event->type == RTGUI_EVENT_HIDE)
    {
        rtgui_container_event_handler(object, event);
        stop_timer(object, event);
    }
    else
    {
        /* 调用默认的事件处理函数 */
        return rtgui_progressbar_event_handler(object, event);
    }

    return RT_FALSE;
}

rtgui_container_t *demo_view_progressbar(void)
{
    rtgui_container_t *container;

    /* create a demo container */
    container = demo_view("ProgressBar View");

    /* get demo container rect */
	rtgui_label_create(container, "水平进度条:", 5, 50, 120, 20);
    hbar = rtgui_progressbar_create(container, RTGUI_HORIZONTAL, 100, 5, 75, 150, 18);

	rtgui_label_create(container, "垂直进度条:", 5, 100, 120, 20);
    vbar = rtgui_progressbar_create(container, RTGUI_VERTICAL, 100, 20, 125, 18, 150);

    bar_timer = rtgui_timer_create(50, RT_TIMER_FLAG_PERIODIC,
                                   progressbar_timeout, RT_NULL);

    return container;
}

