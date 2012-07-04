#include <rtgui/dc.h>
#include <rtgui/dc_hw.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/widgets/container.h>
#include "demo_view.h"

static struct rtgui_container *container = RT_NULL;
static int running = 0;
static rt_tick_t tick_bak=0;
static rt_uint32_t frame_rate=0;
static rt_uint8_t shade_r,shade_g,shade_b,shade_sw=0; 

void frame_rate_onidle(struct rtgui_object *object, rtgui_event_t *event)
{
	rtgui_color_t color, bc;
	rtgui_rect_t rect, draw_rect;
	struct rtgui_dc *dc;

	if(rt_tick_get()-tick_bak >= RT_TICK_PER_SECOND)
	{
		rt_kprintf("frame_rate=%d\n",frame_rate);
		frame_rate=0;
		tick_bak = rt_tick_get();
	}
	else frame_rate ++;
	
	dc = rtgui_dc_begin_drawing(RTGUI_WIDGET(container));
	if (dc == RT_NULL)
		return;

	demo_view_get_logic_rect(RTGUI_CONTAINER(container), &rect);
	draw_rect.x1 = rect.x1;
	draw_rect.y1 = rect.y1;
	draw_rect.x2 = draw_rect.x1+240;
	draw_rect.y2 = draw_rect.y1+240;
	
	bc = RTGUI_DC_BC(dc);
	switch(shade_sw)
	{
	case 0:
		if (++shade_r == 0xFF)
			shade_sw = 1;
		break;
	case 1:
		if (++shade_g == 0xFF)
			shade_sw = 2;
		break;
	case 2:
		if (--shade_r == 0x00)
			shade_sw = 3;
		break;
	case 3:
		if (++shade_b == 0xFF)
			shade_sw = 4;
		break;
	case 4:
		if (--shade_g == 0x00)
			shade_sw = 5;
		break;
	case 5:
		if (++shade_r == 0xFF)
			shade_sw = 6;
		break;
	case 6:
		if (--shade_b == 0x00)
			shade_sw = 1;
		break;
	}
	 
    color = RTGUI_RGB(shade_r, shade_g, shade_b);

	RTGUI_DC_BC(dc) = color;

#if (1)
	rtgui_dc_fill_rect(dc, &draw_rect);
#else
	{
		int x,y;
		for(x=draw_rect.x1; x<draw_rect.x2; x++)
			for(y=draw_rect.y1; y<draw_rect.y2; y++)
				rtgui_dc_draw_color_point(dc, x, y, color);
	}
#endif
	RTGUI_DC_BC(dc) = bc;
	/* 绘图完成 */
	rtgui_dc_end_drawing(dc);
}

void frame_rate_draw_default(struct rtgui_object *object, rtgui_event_t* event)
{
	struct rtgui_widget *widget = RTGUI_WIDGET(object);
	struct rtgui_dc* dc;
	rtgui_rect_t rect;

	/* 因为用的是demo container，上面本身有一部分控件，所以在绘图时先要让demo container先绘图 */
	rtgui_container_event_handler(object, event);

	/* 获得控件所属的DC */
	dc = rtgui_dc_begin_drawing(widget);
	if (dc == RT_NULL) /* 如果不能正常获得DC，返回（如果控件或父控件是隐藏状态，DC是获取不成功的） */
		return;

	/* 获得demo container允许绘图的区域 */
	demo_view_get_logic_rect(RTGUI_CONTAINER(widget), &rect);

	/* 擦除所有 */
	RTGUI_WIDGET_BACKGROUND(widget) = default_background;
	rtgui_dc_fill_rect(dc, &rect);

	/* 显示提示 */
	rtgui_dc_draw_text(dc, "按下鼠标测试刷新率...", &rect);

	/* 绘图完成 */
	rtgui_dc_end_drawing(dc);
}

rt_bool_t frame_rate_event_handler(struct rtgui_object *object, rtgui_event_t *event)
{
	if (event->type == RTGUI_EVENT_PAINT)
	{
		frame_rate_draw_default(object, event);
	}
	else if (event->type == RTGUI_EVENT_MOUSE_BUTTON)
    {
        rtgui_rect_t rect;
		struct rtgui_event_mouse *emouse = (struct rtgui_event_mouse *)event;
		
		demo_view_get_logic_rect(RTGUI_CONTAINER(container), &rect);
		if (rtgui_rect_contains_point(&rect, emouse->x, emouse->y) == RT_EOK)
		{
			if (emouse->button & RTGUI_MOUSE_BUTTON_DOWN)
			{
				if (running)
				{
					/* stop */
					rtgui_application_set_onidle(RT_NULL);
					frame_rate_draw_default(object, event);
				}
				else
				{
					/* run */
					rtgui_application_set_onidle(frame_rate_onidle);
				}

				running = !running;
			}
			return RT_TRUE;
		}
        return rtgui_container_event_handler(object, event);
    }
	else
	{
		/* 调用默认的事件处理函数 */
		return rtgui_container_event_handler(object, event);
	}

	return RT_FALSE;
}

rtgui_container_t *demo_view_frame_rate(void)
{
	srand(100);
	container = demo_view("测试刷新率");
	rtgui_object_set_event_handler(RTGUI_OBJECT(container), frame_rate_event_handler);

	return container;
}
