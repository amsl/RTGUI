#include <rtgui/rtgui.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/rtgui_app.h>
#include <rtgui/widgets/button.h>
#include <rtgui/widgets/window.h>
#include <rtgui/widgets/rttab.h>
#include <rtgui/widgets/panel.h>
#include <rtgui/widgets/label.h>

struct rtgui_win *main_win;
static void demo_test_entry(void *parameter)
{
    struct rtgui_app *app;
	rtgui_panel_t *panel;
	rtgui_label_t *label;
    struct rtgui_rect rect;

    app = rtgui_app_create();

    /* create a full screen window */
    rtgui_gdev_get_rect(rtgui_gdev_get(), &rect);
	panel = rtgui_panel_create(0,0,RC_W(rect),RC_H(rect));

	///{{{
	label = rtgui_label_create(panel, "Hello World!", 10, 10, 150, 20);
	RTGUI_WIDGET_BC(label) = Blue;
	label = rtgui_label_create(panel, "Demo Code Test!", 10, 40, 150, 20);
	RTGUI_WIDGET_BC(label) = Blue;

	
	///}}}

    rtgui_panel_show(panel);

    /* 执行工作台事件循环 */
    rtgui_panel_event_loop(panel);

    rtgui_app_destroy(app);
}

void demo_test_init(void)
{
    static rt_bool_t inited = RT_FALSE;

    if (inited == RT_FALSE) /* 避免重复初始化而做的保护 */
    {
        rt_thread_t tid;

        tid = rt_thread_create("demo",
                               demo_test_entry, RT_NULL,
                               2048 * 2, 25, 10);

        if (tid != RT_NULL)
            rt_thread_startup(tid);

        inited = RT_TRUE;
    }
}
