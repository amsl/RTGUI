#include <rtthread.h>

#include <rtgui/rtgui.h>
#include <rtgui/image.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/rtgui_app.h>
#include <rtgui/widgets/window.h>
#include <rtgui/widgets/widget.h>
#include <rtgui/widgets/button.h>
#include <rtgui/widgets/notebook.h>
#include <rtgui/widgets/label.h>
#include <rtgui/driver.h>
#include <rtgui/region.h>

static struct rtgui_notebook* notebook;

static void remove_myself(struct rtgui_object *self, struct rtgui_event *event)
{
    rt_kprintf("remove the first tab from the notebook\n");
    rtgui_notebook_remove(notebook, 0);
}

static struct rtgui_win *win_main;
static void app_lcd(void *parameter)
{
    /* create application */
	struct rtgui_app *app;
    struct rtgui_rect rect1 = {0, 0, 240, 320};
    struct rtgui_button* btn;
    struct rtgui_label *lb;

	app = rtgui_app_create(rt_thread_self(), "lcd_app");

    if (!app)
    {
        rt_kprintf("Create application \"lcd_app\" failed!\n");
        return;
    }

    /* create main window */
    win_main = rtgui_win_create(RT_NULL, "main",
                    &rect1,
                    RTGUI_WIN_STYLE_NO_BORDER | RTGUI_WIN_STYLE_NO_TITLE);
    if (win_main == RT_NULL)
    {
        rt_kprintf("Create window \"main\" failed!\n");
        rtgui_app_destroy(app);
        return;
    }
    RTGUI_WIDGET_BACKGROUND(RTGUI_WIDGET(win_main)) = green;

    rect1.x1 = rect1.y1 = 50;
    rect1.x2 -= 50;
    rect1.y2 -= 50;
    lb = rtgui_label_create("I am a transparent label!!!!!!!!!");
    rtgui_widget_set_rect(RTGUI_WIDGET(lb), &rect1);
    RTGUI_WIDGET_FLAG(RTGUI_WIDGET(lb)) |= RTGUI_WIDGET_FLAG_TRANSPARENT;

    rect1.x1 += 20;
    rect1.y1 += 20;
	notebook = rtgui_notebook_create(&rect1, 0);
    /* create lable in main container */
    btn = rtgui_button_create("Here I am.");
    rtgui_notebook_add(notebook, "btn A", RTGUI_WIDGET(btn));
    rtgui_button_set_onbutton(btn, remove_myself);
    btn = rtgui_button_create("There I am.");
    rtgui_notebook_add(notebook, "btn B", RTGUI_WIDGET(btn));

    rtgui_container_add_child(RTGUI_CONTAINER(win_main), RTGUI_WIDGET(lb));
	rtgui_container_add_child(RTGUI_CONTAINER(win_main), RTGUI_WIDGET(notebook));

    rtgui_win_show(win_main, RT_FALSE);

    rtgui_app_run(app);
    rtgui_app_destroy(app);
}

int rt_application_init()
{
	rt_thread_t init_thread;

#if (RT_THREAD_PRIORITY_MAX == 32)
	init_thread = rt_thread_create("init",
								app_lcd, RT_NULL,
								2048, 20, 20);
#else
	init_thread = rt_thread_create("init",
								app_lcd, RT_NULL,
								2048, 80, 20);
#endif

	if (init_thread != RT_NULL)
		rt_thread_startup(init_thread);

	return 0;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
void dump_tee()
{
	rtgui_region_dump(&RTGUI_WIDGET(win_main)->clip);
}
FINSH_FUNCTION_EXPORT(dump_tee, dump rtgui topwin tree)
#endif
