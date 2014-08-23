#include <rtgui/rtgui_system.h>
#include <rtgui/rtgui_app.h>
#include <rtgui/widgets/window.h>


rtgui_app_t* rtgui_app_create(void)
{
	rt_thread_t tid = rt_thread_self();
	rtgui_app_t* app = (rtgui_app_t*)rt_malloc(sizeof(rtgui_app_t));

	if(app != RT_NULL)
	{
		/* set tid and mq */
		app->tid		= tid;
		app->mq			= rt_mq_create("app_mq", 256, 32, RT_IPC_FLAG_FIFO); //mq;
		app->widget		= RT_NULL;
		app->on_idle    = RT_NULL;
		/* set user thread */
		tid->user_data = (rt_uint32_t)app;
		rt_memset(app->event_buffer, 0, RTGUI_EVENT_SIZE);
	}
	return app;
}

void rtgui_app_destroy(rtgui_app_t *app)
{
	RT_ASSERT(app != RT_NULL);
	
	rt_mq_delete(app->mq);
	app->tid->user_data = 0;
	rt_free(app);
}

/* get current gui thread */
rtgui_app_t* rtgui_app_self()
{
	rtgui_app_t* app;
	struct rt_thread* self;

	/* get current thread */
	self = rt_thread_self();
	app = (rtgui_app_t*)(self->user_data);

	return app;
}

void rtgui_app_set_onidle(rtgui_idle_func_t onidle)
{
	rtgui_app_t* app;

	app = rtgui_app_self();
	RT_ASSERT(app != RT_NULL);

	app->on_idle = onidle;
}

rtgui_idle_func_t rtgui_app_get_onidle(void)
{
	rtgui_app_t* app;

	app = rtgui_app_self();
	RT_ASSERT(app != RT_NULL);

	return app->on_idle;
}

void rtgui_app_set_widget(pvoid widget)
{
	rtgui_app_t* app;

	/* get thread */
	app = (rtgui_app_t*)(rt_thread_self()->user_data);

	if(app != RT_NULL) app->widget = widget;
}

pvoid rtgui_app_get_widget(void)
{
	rtgui_app_t* app;

	/* get rtgui_app_t */
	app = (rtgui_app_t*)(rt_thread_self()->user_data);

	return app == RT_NULL? RT_NULL : app->widget;
}

