/*
 * 程序清单：button控件演示
 *
 * 这个例子会在创建出的container上添加几个不同类型的button控件
 */

#include "demo_view.h"
#include <rtgui/widgets/button.h>

/* 创建用于演示button控件的视图 */
rtgui_container_t *demo_view_button(void)
{
    rtgui_container_t *container;
    rtgui_button_t *button;
    rtgui_font_t *font;

    /* 先创建一个演示用的视图 */
    container = demo_view("Button View");

	button = rtgui_button_create(container, "Red", 5, 50, 100, 20);
    /* 设置label控件的前景色为红色 */
    RTGUI_WIDGET_FOREGROUND(button) = red;

	button = rtgui_button_create(container, "Blue", 5, 75, 100, 20);
    /* 设置label控件的前景色为蓝色 */
    RTGUI_WIDGET_FOREGROUND(button) = blue;

	button = rtgui_button_create(container, "12 font", 5, 100, 100, 20);
    /* 设置字体为12点阵的asc字体 */
    font = rtgui_font_refer("asc", 12);
    RTGUI_WIDGET_FONT(button) = font;

	button = rtgui_button_create(container, "16 font", 5, 125, 100, 20);
    /* 设置字体为16点阵的asc字体 */
    font = rtgui_font_refer("asc", 16);
    RTGUI_WIDGET_FONT(button) = font;

    return container;
}
