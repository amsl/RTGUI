#include "demo_view.h"
#include <rtgui/widgets/box.h>
#include <rtgui/widgets/panel.h>
#include <rtgui/widgets/label.h>
#include <rtgui/widgets/button.h>

rtgui_container_t *demo_view_box(void)
{
    rtgui_rect_t  rect;
    rtgui_container_t *view;
    struct rtgui_panel *panel;
    struct rtgui_box *box;

    struct rtgui_button *button;

    view = demo_view("Box View");
    rtgui_widget_get_rect(RTGUI_WIDGET(view), &rect);

    panel = rtgui_panel_create(RTGUI_BORDER_NONE);
    rtgui_widget_set_rect(RTGUI_WIDGET(panel), &rect);
    rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(panel));

    box = rtgui_box_create(RTGUI_VERTICAL, 5);
    rtgui_container_set_box(RTGUI_CONTAINER(panel), box);

    rtgui_label_create(RTGUI_CONTAINER(panel), "label 1",5,40,0,20);
    rtgui_label_create(RTGUI_CONTAINER(panel), "label 2",5,70,0,20);
    rtgui_button_create(RTGUI_CONTAINER(panel), "button 1",5,100,0,20);

    button = rtgui_button_create(RTGUI_CONTAINER(panel), "button 2",5,0,130,20);
    rtgui_widget_set_miniheight(RTGUI_WIDGET(button), 25);
    RTGUI_WIDGET_ALIGN(button) = RTGUI_ALIGN_EXPAND;

    rtgui_container_layout(RTGUI_CONTAINER(panel));

    return view;
}

