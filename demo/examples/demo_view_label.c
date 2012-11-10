/*
 * 程序清单：label控件演示
 *
 * 这个例子会在创建出的container上添加几个不同类型的label控件
 */
#include "demo_view.h"
#include <rtgui/widgets/label.h>

/* 创建用于演示label控件的视图 */
rtgui_container_t *demo_view_label(void)
{
    rtgui_rect_t rect;
    rtgui_container_t *container;
    rtgui_label_t *label;
    rtgui_font_t *font;

    /* 先创建一个演示用的视图 */
    container = demo_view("Label View");

    /* 获得视图的位置信息 */
    rtgui_widget_get_rect(RTGUI_WIDGET(container), &rect);

	label = rtgui_label_create(container, "Red Left", 5, 50, RC_W(rect)-10, 20);
    /* 设置label控件上的文本对齐方式为：左对齐 */
    RTGUI_WIDGET_TEXTALIGN(label) = RTGUI_ALIGN_LEFT;
    /* 设置label控件的前景色为红色 */
    RTGUI_WIDGET_FOREGROUND(label) = red;

	label = rtgui_label_create(container, "Blue Right", 5, 75, RC_W(rect)-10, 20);
    /* 设置label控件上的文本对齐方式为：右对齐 */
    RTGUI_WIDGET_TEXTALIGN(label) = RTGUI_ALIGN_RIGHT;
    /* 设置label控件的前景色为蓝色 */
    RTGUI_WIDGET_FOREGROUND(label) = blue;

	label = rtgui_label_create(container, "Green Center", 5, 100, RC_W(rect)-10, 20);
    /* 设置label控件的前景色为绿色 */
    RTGUI_WIDGET_FOREGROUND(label) = green;
    /* 设置label控件上的文本对齐方式为：右对齐 */
    RTGUI_WIDGET_TEXTALIGN(label) = RTGUI_ALIGN_CENTER_HORIZONTAL;

	label = rtgui_label_create(container, "12 font", 5, 125, RC_W(rect)-10, 20);
    /* 设置字体为12点阵的asc字体 */
    font = rtgui_font_refer("asc", 12);
    RTGUI_WIDGET_FONT(label) = font;

	label = rtgui_label_create(container, "16 font", 5, 150, RC_W(rect)-10, 20);
    /* 设置字体为16点阵的asc字体 */
    font = rtgui_font_refer("asc", 16);
    RTGUI_WIDGET_FONT(label) = font;

    return container;
}

