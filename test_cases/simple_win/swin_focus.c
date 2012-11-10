#include <rtthread.h>

#include <rtgui/rtgui.h>
#include <rtgui/region.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/widgets/window.h>
#include <rtgui/widgets/label.h>
#include <rtgui/widgets/button.h>

struct rtgui_win *main_win;

rt_bool_t picture_win_onpaint(struct rtgui_object* object, struct rtgui_event* event)
{
	struct rtgui_widget *widget = RTGUI_WIDGET(object);

	if (event->type == RTGUI_EVENT_PAINT)
	{
		struct rtgui_dc* dc;
		struct rtgui_rect rect;
		struct rtgui_event_paint event;

		rt_kprintf("handle custom paint event\n");

		/* begin drawing */
		dc = rtgui_dc_begin_drawing(RTGUI_WIDGET(widget));
		if (dc == RT_NULL)
		{
			rt_kprintf("dc init failed\n");
			return RT_FALSE;
		}

		/* get window rect */
		rtgui_widget_get_rect(RTGUI_WIDGET(widget), &rect);

		RTGUI_DC_BC(dc) = white;
		rtgui_dc_fill_rect(dc, &rect);

		rtgui_dc_end_drawing(dc);

		return RT_FALSE;
	}
	else
	{
		return rtgui_win_event_handler(object, event);
	}
}

rt_bool_t show_modal_info(struct rtgui_widget *object, struct rtgui_event *event)
{
	rtgui_label_t *label;
	struct rtgui_win *win;
	rtgui_rect_t rect = {0, 60, 150, 100};
	win = rtgui_win_create(main_win,
						   "Info",
						   &rect,
						   RTGUI_WIN_STYLE_DEFAULT);

	label = rtgui_label_create(RTGUI_CONTAINER(win), "modal mode info", 20, 5, 100, 20);

	rtgui_win_show(win, RT_TRUE);

	rtgui_win_destroy(win);

	return RT_TRUE;
}

rt_bool_t echo_btn_pressed(struct rtgui_widget *object, struct rtgui_event *event)
{
	rt_kprintf("object %p pressed\n", object);

	return RT_TRUE;
}

rt_bool_t window_focus(void)
{
	rtgui_label_t *label;

	rtgui_button_t* start_btn;
	rtgui_button_t* stop_btn;
    rtgui_win_t *picture_win;
	rtgui_rect_t rect = {0, 20, 240, 320};

	/* ����һ������ */
	main_win = rtgui_win_create(RT_NULL,
			"������",
			&rect,
			RTGUI_WIN_STYLE_DEFAULT | RTGUI_WIN_STYLE_DESTROY_ON_CLOSE);
	/* �����ͼ��λ����Ϣ */
	rtgui_widget_get_rect(RTGUI_WIDGET(main_win), &rect);
	rtgui_widget_rect_to_device(RTGUI_WIDGET(main_win), &rect);

	/* ���������õı�ǩ */
	label = rtgui_label_create(RTGUI_CONTAINER(main_win), "������", 10, 20, 100, 20);

	/* ����"����"��ť */
	start_btn = rtgui_button_create(RTGUI_CONTAINER(main_win), "��ť1", 10, 50, 50, 25);
	rtgui_button_set_onbutton(start_btn, show_modal_info);

	/* ����"ֹͣ"��ť */
	stop_btn = rtgui_button_create(RTGUI_CONTAINER(main_win), "��ť2", 10, 50, 50, 25);
	rtgui_button_set_onbutton(stop_btn, echo_btn_pressed);

	/*����һ����ͼWindows�ؼ�*/
	rtgui_widget_get_rect(RTGUI_WIDGET(main_win), &rect);
	rtgui_widget_rect_to_device(RTGUI_WIDGET(main_win), &rect);
	rect.x1 += 10;
	rect.y1 += 20;
	rect.x2 = rect.x1 + 200;
	rect.y2 = rect.y1 + 150;
	picture_win = rtgui_win_create(main_win, "��ͼ����", &rect,
            RTGUI_WIN_STYLE_NO_TITLE|RTGUI_WIN_STYLE_NO_BORDER|RTGUI_WIN_STYLE_NO_FOCUS);
    //�������ڣ�û�б�������û����С�����ڣ�Ҳ���ܻ�ȡ����
	/* ���windows���¼�*/
	/* all of the windows are managed by topwin. Never set a window as other
	 * window's child.
	 *
	 *rtgui_container_add_child(RTGUI_CONTAINER(main_win),
	 *        RTGUI_WIDGET(picture_win));
     */
	rtgui_object_set_event_handler(RTGUI_OBJECT(picture_win),
			picture_win_onpaint);

	/* ��ģ̬��ʾ���� */
	rtgui_widget_focus(RTGUI_WIDGET(main_win));//�趨�������ȡ����
	rtgui_win_show(main_win, RT_FALSE);
	rtgui_win_show(picture_win,RT_FALSE);

	return RT_TRUE;
}

