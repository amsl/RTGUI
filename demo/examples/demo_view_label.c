/*
 * �����嵥��label�ؼ���ʾ
 *
 * ������ӻ��ڴ�������container����Ӽ�����ͬ���͵�label�ؼ�
 */
#include "demo_view.h"
#include <rtgui/widgets/label.h>

/* ����������ʾlabel�ؼ�����ͼ */
rtgui_container_t *demo_view_label(void)
{
    rtgui_rect_t rect;
    rtgui_container_t *container;
    rtgui_label_t *label;
    rtgui_font_t *font;

    /* �ȴ���һ����ʾ�õ���ͼ */
    container = demo_view("Label View");

    /* �����ͼ��λ����Ϣ */
    rtgui_widget_get_rect(RTGUI_WIDGET(container), &rect);

	label = rtgui_label_create(container, "Red Left", 5, 50, RC_W(rect)-10, 20);
    /* ����label�ؼ��ϵ��ı����뷽ʽΪ������� */
    RTGUI_WIDGET_TEXTALIGN(label) = RTGUI_ALIGN_LEFT;
    /* ����label�ؼ���ǰ��ɫΪ��ɫ */
    RTGUI_WIDGET_FOREGROUND(label) = red;

	label = rtgui_label_create(container, "Blue Right", 5, 75, RC_W(rect)-10, 20);
    /* ����label�ؼ��ϵ��ı����뷽ʽΪ���Ҷ��� */
    RTGUI_WIDGET_TEXTALIGN(label) = RTGUI_ALIGN_RIGHT;
    /* ����label�ؼ���ǰ��ɫΪ��ɫ */
    RTGUI_WIDGET_FOREGROUND(label) = blue;

	label = rtgui_label_create(container, "Green Center", 5, 100, RC_W(rect)-10, 20);
    /* ����label�ؼ���ǰ��ɫΪ��ɫ */
    RTGUI_WIDGET_FOREGROUND(label) = green;
    /* ����label�ؼ��ϵ��ı����뷽ʽΪ���Ҷ��� */
    RTGUI_WIDGET_TEXTALIGN(label) = RTGUI_ALIGN_CENTER_HORIZONTAL;

	label = rtgui_label_create(container, "12 font", 5, 125, RC_W(rect)-10, 20);
    /* ��������Ϊ12�����asc���� */
    font = rtgui_font_refer("asc", 12);
    RTGUI_WIDGET_FONT(label) = font;

	label = rtgui_label_create(container, "16 font", 5, 150, RC_W(rect)-10, 20);
    /* ��������Ϊ16�����asc���� */
    font = rtgui_font_refer("asc", 16);
    RTGUI_WIDGET_FONT(label) = font;

    return container;
}

