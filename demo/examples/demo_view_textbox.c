/*
 * �����嵥��texbox�ؼ���ʾ
 *
 * ������ӻ��ڴ�������container����Ӽ�����ͬ���͵�textbox�ؼ�
 */
#include "demo_view.h"
#include <rtgui/widgets/label.h>
#include <rtgui/widgets/textbox.h>

/* ����������ʾtextbox�ؼ�����ͼ */
rtgui_container_t *demo_view_textbox(void)
{
    rtgui_container_t *container;
    rtgui_label_t *label;
    rtgui_textbox_t *text;

    /* �ȴ���һ����ʾ�õ���ͼ */
    container = demo_view("TextBox View");

	label = rtgui_label_create(container, "����: ", 5, 50, 30, 20);
    text = rtgui_textbox_create(container, "bernard", 40, 50, 160, 20, RTGUI_TEXTBOX_SINGLE);

	label = rtgui_label_create(container, "�ʼ�: ", 5, 73, 30, 20);
    text = rtgui_textbox_create(container, "bernard.xiong@gmail.com", 40, 73, 160, 20, RTGUI_TEXTBOX_SINGLE);

	label = rtgui_label_create(container, "����: ", 5, 96, 30, 20);
    text = rtgui_textbox_create(container, "rt-thread", 40, 96, 160, 20, RTGUI_TEXTBOX_SINGLE);
    /* ����textbox��ʾ�ı�Ϊ������ʽ(����ʾΪ*�ţ��ʺ�����ʾ��������) */
    text->flag |= RTGUI_TEXTBOX_MASK;

	label = rtgui_label_create(container, "��ҳ: ", 5, 119, 30, 20);
    text = rtgui_textbox_create(container, "http://www.rt-thread.org", 40, 119, 160, 20, RTGUI_TEXTBOX_SINGLE);

    return container;
}
