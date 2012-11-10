/*
 * �����嵥��������ʾ
 *
 * ������ӻ��ȴ�����һ����ʾ�õ�container�����������İ�ťʱ�᲻ͬ��ģʽ��������
 */

#include <rtgui/rtgui.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/widgets/window.h>
#include <rtgui/widgets/label.h>
#include <rtgui/widgets/button.h>
#include "demo_view.h"
#include <string.h>

static struct rtgui_timer *timer;
static struct rtgui_label *label;
static struct rtgui_win *autowin = RT_NULL;
static char label_text[80];
static rt_uint8_t cnt = 5;

rtgui_win_t *normal_window;
rtgui_label_t *normal_window_label;
static char normal_window_label_text[16];
static unsigned char normal_window_show_count = 1;
extern struct rtgui_win *main_win;

static rt_bool_t normal_window_onclose(struct rtgui_object *win,
                                       struct rtgui_event *event)
{
    normal_window_show_count++;

    return RT_TRUE;
}

static void create_normal_win(void)
{
    rtgui_rect_t rect = {30, 40, 150, 80};

    normal_window = rtgui_win_create(RT_NULL, "��ͨ����",
                                     &rect, RTGUI_WIN_STYLE_DEFAULT);

    rect.x1 += 20;
    rect.x2 -= 5;
    rect.y1 += 5;
    rect.y2 = rect.y1 + 20;

    /* ���һ���ı���ǩ */
    rt_sprintf(normal_window_label_text,
               "�� %d ����ʾ", normal_window_show_count);
	normal_window_label = rtgui_label_create(RTGUI_CONTAINER(normal_window), 
								normal_window_label_text, 5, 5, 120, 20);

    rtgui_win_set_onclose(normal_window,
                          normal_window_onclose);
}

/* ��������������ʾ */
static void demo_normal_window_onbutton(struct rtgui_object *object, rtgui_event_t *event)
{
    rt_sprintf(normal_window_label_text,
               "�� %d ����ʾ", normal_window_show_count);
    rtgui_label_set_text(normal_window_label,
                         normal_window_label_text);
    if (RTGUI_WIDGET_IS_HIDE(normal_window))
        rtgui_win_show(normal_window, RT_FALSE);
    else
        rtgui_win_activate(normal_window);
}

/* ��ȡһ�������Ĵ��ڱ��� */
static char *get_win_title()
{
    static rt_uint8_t win_no = 0;
    static char win_title[16];

    rt_sprintf(win_title, "���� %d", ++win_no);
    return win_title;
}

/* �رնԻ���ʱ�Ļص����� */
void diag_close(struct rtgui_timer *timer, void *parameter)
{
    cnt --;
    rt_sprintf(label_text, "closed then %d second!", cnt);

    /* ���ñ�ǩ�ı������¿ؼ� */
    rtgui_label_set_text(label, label_text);
    rtgui_widget_update(RTGUI_WIDGET(label));

    if (cnt == 0)
    {
        /* ��ʱ���رնԻ��� */
        rtgui_win_destroy(autowin);
    }
}

/* AUTO���ڹر�ʱ���¼����� */
rt_bool_t auto_window_close(struct rtgui_object *object, struct rtgui_event *event)
{
    if (timer != RT_NULL)
    {
        /* ֹͣ��ɾ����ʱ�� */
        rtgui_timer_stop(timer);
        rtgui_timer_destory(timer);

        timer = RT_NULL;
    }
    autowin = RT_NULL;

    return RT_TRUE;
}

static rt_uint16_t delta_x = 20;
static rt_uint16_t delta_y = 40;

/* �����Զ�������ʾ */
static void demo_autowin_onbutton(struct rtgui_object *object, rtgui_event_t *event)
{
    struct rtgui_rect rect = {50, 50, 200, 200};

    /* don't create the window twice */
    if (autowin)
        return;

    autowin = rtgui_win_create(main_win, "Information",
                              &rect, RTGUI_WIN_STYLE_DEFAULT | RTGUI_WIN_STYLE_DESTROY_ON_CLOSE);
    if (autowin == RT_NULL)
        return;

    cnt = 5;
    rt_sprintf(label_text, "closed then %d second!", cnt);
	label = rtgui_label_create(RTGUI_CONTAINER(autowin), label_text, 5, 5, 120, 20);

    /* ���ùرմ���ʱ�Ķ��� */
    rtgui_win_set_onclose(autowin, auto_window_close);

    rtgui_win_show(autowin, RT_FALSE);
    /* ����һ����ʱ�� */
    timer = rtgui_timer_create(100, RT_TIMER_FLAG_PERIODIC, diag_close, RT_NULL);
    rtgui_timer_start(timer);
}

/* ����ģ̬������ʾ */
static void demo_modalwin_onbutton(struct rtgui_object *object, rtgui_event_t *event)
{
    rtgui_win_t *win;
    rtgui_label_t *label;
    rtgui_rect_t rect = {0, 0, 150, 80};

    rtgui_rect_moveto(&rect, delta_x, delta_y);
    delta_x += 20;
    delta_y += 20;

    /* ����һ������ */
    win = rtgui_win_create(main_win,
                           get_win_title(), &rect, RTGUI_WIN_STYLE_DEFAULT);

    rect.x1 += 20;
    rect.x2 -= 5;
    rect.y1 += 5;
    rect.y2 = rect.y1 + 20;

	label = rtgui_label_create(RTGUI_CONTAINER(win), "����һ��ģʽ����", 20, 5, 120, 20);

    /* ģ̬��ʾ���� */
    rtgui_win_show(win, RT_TRUE);

    /* ɾ�����Զ�ɾ������ */
    rtgui_win_destroy(win);
}

static void demo_close_ntitle_window(struct rtgui_object *object, rtgui_event_t *event)
{
    rtgui_win_t *win;

    /* ������ؼ� */
    win = RTGUI_WIN(rtgui_widget_get_toplevel(RTGUI_WIDGET(object)));

    /* ���ٴ��� */
    rtgui_win_destroy(win);
}

/* �����ޱ��ⴰ����ʾ */
static void demo_ntitlewin_onbutton(struct rtgui_object *object, rtgui_event_t *event)
{
    rtgui_win_t *win;
    rtgui_label_t *label;
    rtgui_button_t *button;
    rtgui_rect_t rect = {0, 0, 150, 80};

    rtgui_rect_moveto(&rect, delta_x, delta_y);
    delta_x += 20;
    delta_y += 20;

    /* ����һ�����ڣ����Ϊ�ޱ��⼰�ޱ߿� */
    win = rtgui_win_create(main_win,
                           "no title", &rect, RTGUI_WIN_STYLE_NO_TITLE |
                           RTGUI_WIN_STYLE_NO_BORDER |
                           RTGUI_WIN_STYLE_DESTROY_ON_CLOSE);
    RTGUI_WIDGET_BACKGROUND(win) = white;

    /* ����һ���ı���ǩ */
	label = rtgui_label_create(RTGUI_CONTAINER(win), "�ޱ߿򴰿�", (rtgui_rect_width(rect)-100)/2, 20, 100, 20);
    RTGUI_WIDGET_BACKGROUND(label) = white;

    /* ����һ���رհ�ť */
	button = rtgui_button_create(RTGUI_CONTAINER(win), "�ر�", 5, 25, 40, 20);
    rtgui_button_set_onbutton(button, demo_close_ntitle_window);

    rtgui_win_show(win, RT_FALSE);
}

rtgui_container_t *demo_view_window(void)
{
    rtgui_container_t *container;
    rtgui_button_t *button;

    /* ����һ����ʾ�õ���ͼ */
    container = demo_view("Window Demo");

    create_normal_win();

	button = rtgui_button_create(container, "Normal Win", 5, 50, 100, 20);
    /* ����onbuttonΪdemo_win_onbutton���� */
    rtgui_button_set_onbutton(button, demo_normal_window_onbutton);

	button = rtgui_button_create(container, "Auto Win", 5, 75, 100, 20);
    /* ����onbuttonΪdemo_autowin_onbutton���� */
    rtgui_button_set_onbutton(button, demo_autowin_onbutton);

	button = rtgui_button_create(container, "Modal Win", 5, 100, 100, 20);
    /* ����onbuttonΪdemo_modalwin_onbutton���� */
    rtgui_button_set_onbutton(button, demo_modalwin_onbutton);

	button = rtgui_button_create(container, "Normal Win", 5, 125, 100, 20);
    /* ����onbuttonΪdemo_ntitlewin_onbutton���� */
    rtgui_button_set_onbutton(button, demo_ntitlewin_onbutton);

    return container;
}
