/*
 * �����嵥��button�ؼ���ʾ
 *
 * ������ӻ��ڴ�������container����Ӽ�����ͬ���͵�button�ؼ�
 */

#include "demo_view.h"
#include <rtgui/widgets/button.h>

/* ����������ʾbutton�ؼ�����ͼ */
rtgui_container_t *demo_view_button(void)
{
    rtgui_container_t *container;
    rtgui_button_t *button;
    rtgui_font_t *font;

    /* �ȴ���һ����ʾ�õ���ͼ */
    container = demo_view("Button View");

	button = rtgui_button_create(container, "Red", 5, 50, 100, 20);
    /* ����label�ؼ���ǰ��ɫΪ��ɫ */
    RTGUI_WIDGET_FOREGROUND(button) = red;

	button = rtgui_button_create(container, "Blue", 5, 75, 100, 20);
    /* ����label�ؼ���ǰ��ɫΪ��ɫ */
    RTGUI_WIDGET_FOREGROUND(button) = blue;

	button = rtgui_button_create(container, "12 font", 5, 100, 100, 20);
    /* ��������Ϊ12�����asc���� */
    font = rtgui_font_refer("asc", 12);
    RTGUI_WIDGET_FONT(button) = font;

	button = rtgui_button_create(container, "16 font", 5, 125, 100, 20);
    /* ��������Ϊ16�����asc���� */
    font = rtgui_font_refer("asc", 16);
    RTGUI_WIDGET_FONT(button) = font;

    return container;
}
