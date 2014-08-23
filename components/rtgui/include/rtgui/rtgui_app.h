#ifndef __RTGUI_APP_H__
#define __RTGUI_APP_H__

#include <rtgui/rtgui.h>
#include <rtgui/rtgui_object.h>
#include <rtgui/event.h>

#ifdef __cplusplus
extern "C" {
#endif

struct rtgui_thread
{
	/* the thread id */
	struct rt_thread* tid;
	/* the message queue of thread */
	rt_mq_t mq;
	/* the owner of thread */
	pvoid widget;
	/* event buffer */
	rt_uint8_t event_buffer[RTGUI_EVENT_SIZE];

	/* on idle event handler */
	void (*on_idle)(pvoid wdt, rtgui_event_t *event);
};
typedef struct rtgui_thread rtgui_app_t;


rtgui_app_t* rtgui_app_create(void);
void rtgui_app_destroy(rtgui_app_t *app);
rtgui_app_t* rtgui_app_self(void);
void rtgui_app_set_onidle(rtgui_idle_func_t onidle);
rtgui_idle_func_t rtgui_app_get_onidle(void);

rt_thread_t rtgui_get_server(void);

void rtgui_app_set_widget(pvoid widget);
pvoid rtgui_app_get_widget(void);

#ifdef __cplusplus
}
#endif

#endif
