/*
 * 程序清单：checkbox控件演示
 *
 * 这个例子会在创建出的container上添加几个checkbox控件
 */

#include "demo_view.h"
#include <rtgui/widgets/checkbox.h>

/* 创建用于演示checkbox控件的视图 */
rtgui_container_t *demo_view_checkbox(void)
{
    rtgui_container_t *container;
    rtgui_checkbox_t *checkbox;
    rtgui_font_t *font;

    /* 先创建一个演示用的视图 */
    container = demo_view("CheckBox View");

	checkbox = rtgui_checkbox_create(container, "Red", RT_TRUE, 5, 50);
    /* 设置前景色为红色 */
    RTGUI_WIDGET_FOREGROUND(checkbox) = red;

	checkbox = rtgui_checkbox_create(container, "Blue", RT_TRUE, 5, 75);
    /* 设置前景色为蓝色 */
    RTGUI_WIDGET_FOREGROUND(checkbox) = blue;

	checkbox = rtgui_checkbox_create(container, "12 font", RT_TRUE, 5, 100);
    /* 设置字体为12点阵 */
    font = rtgui_font_refer("asc", 12);
    RTGUI_WIDGET_FONT(checkbox) = font;

	checkbox = rtgui_checkbox_create(container, "16 font", RT_TRUE, 5, 125);
    /* 设置字体为16点阵 */
    font = rtgui_font_refer("asc", 16);
    RTGUI_WIDGET_FONT(checkbox) = font;

    return container;
}
