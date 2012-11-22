/*
 * �����嵥��edit�ؼ���ʾ
 *
 * ������ӻ���conatiner�Ͻ���edit�ؼ�����ʾ
 */

#include "demo_view.h"
#include <rtgui/dc.h>
#include <rtgui/filerw.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/widgets/edit.h>
#include <rtgui/widgets/label.h>
#include <rtgui/widgets/button.h>

#ifdef RTGUI_USING_DFS_FILERW
void demo_edit_readin_file(struct rtgui_object *object, struct rtgui_event *event)
{
    rtgui_button_t *button;
    struct rtgui_edit *edit;
    const char *filename = "/test_readin.txt";
    struct rtgui_filerw *file;

    RT_ASSERT(object != RT_NULL);
    button = RTGUI_BUTTON(object);
    /* ȡ��editָ�� */
    edit = RTGUI_EDIT(RTGUI_WIDGET(button)->user_data);

    /* �ж��ļ��Ƿ���� */
    file = rtgui_filerw_create_file(filename, "rb");
    if (file == RT_NULL)
    {
        /* �����ڴ���,�򴴽��� */
        rt_kprintf("file:\"%s\" does not exist!\n", filename);

        return;
    }
    rtgui_filerw_close(file);

    rt_kprintf("read-in file:\"%s\"\n", filename);
    rtgui_edit_readin_file(edit, filename);
}

void demo_edit_saveas_file(struct rtgui_object *object, struct rtgui_event *event)
{
    rtgui_button_t *button;
    struct rtgui_edit *edit;
    const char* filename = "/test_saveas.txt";
    struct rtgui_filerw *file;

    RT_ASSERT(object != RT_NULL);
    button = RTGUI_BUTTON(object);
    /* ȡ��editָ�� */
    edit = RTGUI_EDIT(RTGUI_WIDGET(button)->user_data);

    /* �ж��ļ��Ƿ����, ���������ɾ��֮ */
    file = rtgui_filerw_create_file(filename, "rb");
    if (file != RT_NULL)
    {
        rtgui_filerw_close(file);
        /* �������win32�е���, ���ֹ�ɾ�����ļ���, NT���ļ���ֻ����,unlinkɾ������ */
        if (rtgui_filerw_unlink(filename) == -1)
            rt_kprintf("Could not delete %s\n", filename);
    }

    rt_kprintf("save as file:\"%s\"\n", filename);
    rtgui_edit_saveas_file(edit, filename);
}

void demo_edit_get_mem(struct rtgui_object* object, struct rtgui_event* event)
{
	rtgui_button_t *button;
	struct rtgui_edit *edit;

	RT_ASSERT(object != RT_NULL);

	button = RTGUI_BUTTON(object);
	edit = RTGUI_EDIT( RTGUI_WIDGET(button)->user_data );
	rt_kprintf("edit mem consume: %d\n", rtgui_edit_get_mem_consume(edit));
}

rt_bool_t demo_edit_event_handler(struct rtgui_object* object, struct rtgui_event *event)
{
	rt_bool_t result;
	char buf[32];
	rtgui_point_t p;
	struct rtgui_edit *edit = RTGUI_EDIT(object);
	struct rtgui_label *label = (struct rtgui_label*)RTGUI_WIDGET(edit)->user_data;

	result = rtgui_edit_event_handler(object, event);

	p = rtgui_edit_get_current_point(edit);
	rt_sprintf(buf, "TRACK: line:%d, col:%d", p.y, p.x);
	rtgui_label_set_text(label, buf);
	
	return result;
}

/* ����������ʾedit�ؼ�����ͼ */
rtgui_container_t *demo_view_edit(void)
{
    rtgui_container_t *container;
    struct rtgui_edit *edit;
	struct rtgui_label *label;
    struct rtgui_button *button;

    /* �ȴ���һ����ʾ�õ���ͼ */
    container = demo_view("Edit View");

    edit = rtgui_edit_create(container, 10, 35, 220, 200);
    rtgui_edit_set_text(edit, 
                        "Edit beta v0.2\n"
                        "Hello RT-Thread\n"
                        "this's edit demo\n"
                        "rtgui_edit_create\n"
                        "rtgui_edit_append_line\n"
                        "rtgui_edit_insert_line\n"
                        "rtgui_edit_delete_line\n"
                        "rtgui_edit_connect_line\n"
						"˫�ֽڲ���\n"
						"this line text can be \n"
                        "a\n"
                        "b\n"
                        "c\n"
                        "d\n"
						"e\n"
						"f\n"
                        "1234567890\n");

    /* ����һ����ť, ��ȡĳ���ļ� */
	button = rtgui_button_create(container, "ReadIn File", 10, 245, 100, 20);
    rtgui_button_set_onbutton(button, demo_edit_readin_file);
    /* ʹ��user_data����editָ�� */
    RTGUI_WIDGET(button)->user_data = (rt_uint32_t)edit;

    /* ����һ����ť, ����Ϊĳ���ļ� */
	button = rtgui_button_create(container, "SaveAs File", 130, 245, 100, 20);
    rtgui_button_set_onbutton(button, demo_edit_saveas_file);
    /* ʹ��user_data����editָ�� */
    RTGUI_WIDGET(button)->user_data = (rt_uint32_t)edit;

	/* ����һ����ǩ, ��ʾEDIT����Ҫ���� */
	label = rtgui_label_create(container, "TRACK:", 10, 270, 220, 18);
	RTGUI_WIDGET_FOREGROUND(label) = blue;

	RTGUI_WIDGET(edit)->user_data = (rt_uint32_t)label;
	rtgui_object_set_event_handler(RTGUI_OBJECT(edit), demo_edit_event_handler);
	
	/* ����һ����ť, ��ȡEDIT���ڴ����� */
	button = rtgui_button_create(container, "Get Mem", 130, 3, 100, 20);
	rtgui_button_set_onbutton(button, demo_edit_get_mem);
	RTGUI_WIDGET(button)->user_data = (rt_uint32_t)edit;

    return container;
}
#endif