/*
 * 程序清单：texbox控件演示
 *
 * 这个例子会在创建出的container上添加几个不同类型的textbox控件
 */
#include "demo_view.h"
#include <rtgui/widgets/label.h>
#include <rtgui/widgets/textbox.h>

/* 创建用于演示textbox控件的视图 */
rtgui_container_t *demo_view_textbox(void)
{
    rtgui_container_t *container;
    rtgui_label_t *label;
    rtgui_textbox_t *text;

    /* 先创建一个演示用的视图 */
    container = demo_view("TextBox View");

	label = rtgui_label_create(container, "名字: ", 5, 50, 30, 20);
    text = rtgui_textbox_create(container, "bernard", 40, 50, 160, 20, RTGUI_TEXTBOX_SINGLE);

	label = rtgui_label_create(container, "邮件: ", 5, 73, 30, 20);
    text = rtgui_textbox_create(container, "bernard.xiong@gmail.com", 40, 73, 160, 20, RTGUI_TEXTBOX_SINGLE);

	label = rtgui_label_create(container, "密码: ", 5, 96, 30, 20);
    text = rtgui_textbox_create(container, "rt-thread", 40, 96, 160, 20, RTGUI_TEXTBOX_SINGLE);
    /* 设置textbox显示文本为掩码形式(即显示为*号，适合于显示密码的情况) */
    text->flag |= RTGUI_TEXTBOX_MASK;

	label = rtgui_label_create(container, "主页: ", 5, 119, 30, 20);
    text = rtgui_textbox_create(container, "http://www.rt-thread.org", 40, 119, 160, 20, RTGUI_TEXTBOX_SINGLE);

    return container;
}
