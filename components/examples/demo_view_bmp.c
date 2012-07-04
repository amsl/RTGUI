#include <rtthread.h>
#include "rtgui/image_bmp.h"
#include "demo_view.h"
#include <rtgui/widgets/button.h>

void screen_grap_onbutton(rtgui_object_t *object, rtgui_event_t *event)
{
	bmp_create("/screen.bmp");	
}

rtgui_container_t* demo_view_screen_grap(void)
{
	rtgui_rect_t rect;
	rtgui_container_t* container;
	rtgui_button_t* button;
	rtgui_font_t* font;

	/* 先创建一个演示用的视图 */
	container = demo_view("Save screen as bmp");

	/* 获得视图的位置信息 */
	demo_view_get_rect(container, &rect);
	rect.x1 += 5;
	rect.x2 = rect.x1 + 100;
	rect.y1 += 5;
	rect.y2 = rect.y1 + 24;

	button = rtgui_button_create("screen grap");
	rtgui_widget_set_rect(RTGUI_WIDGET(button), &rect);
	rtgui_container_add_child(container, RTGUI_WIDGET(button));
	rtgui_button_set_onbutton(button, screen_grap_onbutton);

	return container;
}

