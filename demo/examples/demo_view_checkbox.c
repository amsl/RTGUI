/*
 * �����嵥��checkbox�ؼ���ʾ
 *
 * ������ӻ��ڴ�������container����Ӽ���checkbox�ؼ�
 */

#include "demo_view.h"
#include <rtgui/widgets/checkbox.h>

/* ����������ʾcheckbox�ؼ�����ͼ */
rtgui_container_t *demo_view_checkbox(void)
{
    rtgui_container_t *container;
    rtgui_checkbox_t *checkbox;
    rtgui_font_t *font;

    /* �ȴ���һ����ʾ�õ���ͼ */
    container = demo_view("CheckBox View");

	checkbox = rtgui_checkbox_create(container, "Red", RT_TRUE, 5, 50);
    /* ����ǰ��ɫΪ��ɫ */
    RTGUI_WIDGET_FOREGROUND(checkbox) = red;

	checkbox = rtgui_checkbox_create(container, "Blue", RT_TRUE, 5, 75);
    /* ����ǰ��ɫΪ��ɫ */
    RTGUI_WIDGET_FOREGROUND(checkbox) = blue;

	checkbox = rtgui_checkbox_create(container, "12 font", RT_TRUE, 5, 100);
    /* ��������Ϊ12���� */
    font = rtgui_font_refer("asc", 12);
    RTGUI_WIDGET_FONT(checkbox) = font;

	checkbox = rtgui_checkbox_create(container, "16 font", RT_TRUE, 5, 125);
    /* ��������Ϊ16���� */
    font = rtgui_font_refer("asc", 16);
    RTGUI_WIDGET_FONT(checkbox) = font;

    return container;
}
