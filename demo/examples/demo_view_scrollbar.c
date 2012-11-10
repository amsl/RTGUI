#include "demo_view.h"
#include <rtgui/rtgui_system.h>
#include <rtgui/widgets/label.h>
#include <rtgui/widgets/scrollbar.h>

rtgui_container_t *demo_view_scrollbar(void)
{
    rtgui_container_t *container;
    rtgui_label_t *label;
    rtgui_scrollbar_t *hbar;
    rtgui_scrollbar_t *vbar;

    /* create a demo container */
    container = demo_view("ScrollBar View");

	label = rtgui_label_create(container, "horizontal bar:", 5, 40, 120, 20);

    hbar = rtgui_scrollbar_create(container, 5, 60, 20, 200, RTGUI_HORIZONTAL);
	rtgui_scrollbar_set_line_step(hbar, 1);
	rtgui_scrollbar_set_page_step(hbar, 3);
	rtgui_scrollbar_set_range(hbar, 20);

	label = rtgui_label_create(container, "vertical bar:", 5, 85, 120, 20);
    
    vbar = rtgui_scrollbar_create(container, 10, 110, 20, 200, RTGUI_VERTICAL);
	rtgui_scrollbar_set_line_step(vbar, 1);
	rtgui_scrollbar_set_page_step(vbar, 5);
	rtgui_scrollbar_set_range(vbar, 20);

    return container;
}
