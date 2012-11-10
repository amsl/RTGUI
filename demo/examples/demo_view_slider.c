#include "demo_view.h"
#include <rtgui/rtgui_system.h>
#include <rtgui/widgets/label.h>
#include <rtgui/widgets/slider.h>

rtgui_container_t *demo_view_slider(void)
{
    rtgui_container_t *container;
    rtgui_rect_t rect;
    rtgui_label_t *label;
    rtgui_slider_t *slider;

    /* create a demo container */
    container = demo_view("Slider View");
	rtgui_widget_get_rect(RTGUI_WIDGET(container), &rect);

    /* get demo container rect */
	label = rtgui_label_create(container, "horizontal slider:", 5, 50, 100, 18);
    slider = rtgui_slider_create(container, 0, 100, 5, 70, RC_W(rect)-10, 18, RTGUI_HORIZONTAL);

    /* get demo container rect */
	label = rtgui_label_create(container, "vertical slider:", 5, 95, 100, 18);
    slider = rtgui_slider_create(container, 0, 100, 110, 120, 18, 150, RTGUI_VERTICAL);
    return container;
}
