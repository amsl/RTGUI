#include <rtgui/rtgui.h>
#include <rtgui/rtgui_app.h>

#include <rtgui/widgets/container.h>
#include <rtgui/widgets/notebook.h>
#include <rtgui/widgets/button.h>
#include <rtgui/widgets/staticline.h>
#include <rtgui/widgets/box.h>

extern struct rtgui_notebook *the_notebook;

void demo_view_next(struct rtgui_object *object, struct rtgui_event *event)
{
	rtgui_notebook_set_current_by_index(the_notebook,
		(rtgui_notebook_get_current_index(the_notebook) + 1) %
		rtgui_notebook_get_count(the_notebook));
}

void demo_view_prev(struct rtgui_object *object, struct rtgui_event *event)
{
	rt_int16_t cur_idx = rtgui_notebook_get_current_index(the_notebook);

	if (cur_idx == 0)
		rtgui_notebook_set_current_by_index(the_notebook,
		rtgui_notebook_get_count(the_notebook) - 1);
	else
		rtgui_notebook_set_current_by_index(the_notebook,
		--cur_idx);
}

rtgui_container_t *demo_view(const char *title)
{
	struct rtgui_container  *container;
	struct rtgui_button     *button;
	struct rtgui_rect       rect;

	container = rtgui_container_create();
	if (container == RT_NULL)
		return RT_NULL;

	rtgui_notebook_add(the_notebook, title, RTGUI_WIDGET(container));

	/* 获得视图的位置信息(在加入到 notebook 中时，notebook 会自动调整 container
	* 的大小) */
	rtgui_widget_get_rect(RTGUI_WIDGET(container), &rect);

	/* 创建标题用的标签 */
	rtgui_label_create(container, title, 5, 5, rt_strlen(title)*8, 20);
	/* 创建一个水平的 staticline 线 */
	rtgui_staticline_create(container, 5, 30, 2, RC_W(rect)-10, RTGUI_HORIZONTAL);

	/* 创建"下一个"按钮 */
	
	button = rtgui_button_create(container, "下一个", RC_W(rect)-105, RC_H(rect)-35, 100, 25);
	/* 设置onbutton动作到demo_view_next函数 */
	rtgui_button_set_onbutton(button, demo_view_next);
//rt_kprintf("button = %08X, %s\n", button, RTGUI_LABEL(button)->text);//DEBUG
	/* 创建"上一个"按钮 */
	button = rtgui_button_create(container, "上一个", 5, RC_H(rect)-35, 100, 25);
	/* 设置onbutton动作到demo_view_prev函数 */
	rtgui_button_set_onbutton(button, demo_view_prev);

	/* 返回创建的视图 */
	return container;
}

/* 这个函数用于返回演示视图的对外可用区域 */
void demo_view_get_rect(rtgui_container_t *container, rtgui_rect_t *rect)
{
	RT_ASSERT(container != RT_NULL);
	RT_ASSERT(rect != RT_NULL);

	rtgui_widget_get_rect(RTGUI_WIDGET(container), rect);
	rtgui_widget_rect_to_device(RTGUI_WIDGET(container), rect);
	/* 去除演示标题和下方按钮的区域 */
	rect->y1 += 45;
	rect->y2 -= 35;
}

void demo_view_get_logic_rect(rtgui_container_t *container, rtgui_rect_t *rect)
{
	RT_ASSERT(container != RT_NULL);
	RT_ASSERT(rect != RT_NULL);

	rtgui_widget_get_rect(RTGUI_WIDGET(container), rect);
	/* 去除演示标题和下方按钮的区域 */
	rect->y1 += 45;
	rect->y2 -= 35;
}
