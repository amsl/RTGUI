#include <rtgui/dc.h>
#include <rtgui/rtgui_theme.h>
#include <rtgui/widgets/checkbox.h>

static rt_bool_t rtgui_checkbox_onunfocus(struct rtgui_object *object, rtgui_event_t *event);

static void _rtgui_checkbox_constructor(rtgui_checkbox_t *box)
{
    /* init widget and set event handler */
    RTGUI_WIDGET(box)->flag |= RTGUI_WIDGET_FLAG_FOCUSABLE;
    rtgui_object_set_event_handler(RTGUI_OBJECT(box), rtgui_checkbox_event_handler);
	rtgui_widget_set_onunfocus(RTGUI_WIDGET(box), rtgui_checkbox_onunfocus);
    /* set status */
    box->status_down = RTGUI_CHECKBOX_STATUS_UNCHECKED;
    box->on_button = RT_NULL;

    /* set default gc */
    RTGUI_WIDGET_TEXTALIGN(box) = RTGUI_ALIGN_LEFT | RTGUI_ALIGN_CENTER_VERTICAL;
}

DEFINE_CLASS_TYPE(checkbox, "checkbox",
                  RTGUI_LABEL_TYPE,
                  _rtgui_checkbox_constructor,
                  RT_NULL,
                  sizeof(struct rtgui_checkbox));

void rtgui_checkbox_set_onbutton(rtgui_checkbox_t *checkbox, rtgui_onbutton_func_t func)
{
    RT_ASSERT(checkbox != RT_NULL);

    checkbox->on_button = func;
}

static const rt_uint8_t checked_byte[7] = {0x02, 0x06, 0x8E, 0xDC, 0xF8, 0x70, 0x20};
void rtgui_checkbox_ondraw(struct rtgui_checkbox *checkbox)
{
	struct rtgui_dc *dc;
	struct rtgui_rect rect, box_rect;
	rtgui_color_t bc, fc;

	fc = RTGUI_WIDGET_FOREGROUND(checkbox);
	bc = RTGUI_WIDGET_BACKGROUND(checkbox);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(RTGUI_WIDGET(checkbox));
	if (dc == RT_NULL) return;

	/* get rect */
	rtgui_widget_get_rect(RTGUI_WIDGET(checkbox), &rect);

	/* fill rect */
	rtgui_dc_fill_rect(dc, &rect);

	if (RTGUI_WIDGET_IS_FOCUSED(checkbox))
	{
		rtgui_rect_t tmp_rect;
		char *string = rtgui_label_get_text(RTGUI_LABEL(checkbox));
		rtgui_font_get_string_rect(RTGUI_WIDGET_FONT(checkbox), string, &tmp_rect);
		rtgui_rect_moveto(&tmp_rect, rect.x1+CHECK_BOX_W+5, rect.y1);
		tmp_rect.x1 -= 1;
		tmp_rect.x2 += 2;
		tmp_rect.y2 = rect.y2-1;
		rtgui_dc_draw_focus_rect(dc, &tmp_rect);
	}

	/* draw check box */
	box_rect.x1 = 0;
	box_rect.y1 = 0;
	box_rect.x2 = CHECK_BOX_W;
	box_rect.y2 = CHECK_BOX_H;
	rtgui_rect_moveto_align(&rect, &box_rect, RTGUI_ALIGN_CENTER_VERTICAL);
	box_rect.x1 += 2;
	box_rect.x2 += 2;

	rtgui_dc_draw_border(dc, &box_rect, RTGUI_BORDER_BOX);
	rtgui_rect_inflate(&box_rect, -1);
	RTGUI_WIDGET_BACKGROUND(checkbox) = RTGUI_RGB(247, 247, 246);
	rtgui_dc_fill_rect(dc, &box_rect);
	if (checkbox->status_down == RTGUI_CHECKBOX_STATUS_CHECKED)
	{
		RTGUI_WIDGET_FOREGROUND(checkbox) = RTGUI_RGB(33, 161, 33);
		rtgui_dc_draw_byte(dc, box_rect.x1 + 2, box_rect.y1 + 2, 7, checked_byte);
	}

	/* restore saved color */
	RTGUI_WIDGET_BACKGROUND(checkbox) = bc;
	RTGUI_WIDGET_FOREGROUND(checkbox) = fc;

	/* draw text */
	rect.x1 += RC_H(rect) - 4 + 5;
	rtgui_dc_draw_text(dc, rtgui_label_get_text(RTGUI_LABEL(checkbox)), &rect);

	/* end drawing */
	rtgui_dc_end_drawing(dc);

	return;
}

rt_bool_t rtgui_checkbox_event_handler(struct rtgui_object *object, struct rtgui_event *event)
{
    struct rtgui_checkbox *box;

    RTGUI_WIDGET_EVENT_HANDLER_PREPARE

    box = RTGUI_CHECKBOX(object);

    switch (event->type)
    {
    case RTGUI_EVENT_PAINT:
        if (widget->on_draw != RT_NULL)
            return widget->on_draw(RTGUI_OBJECT(widget), event);
        else
            rtgui_checkbox_ondraw(box);
        break;

    case RTGUI_EVENT_MOUSE_BUTTON:
    {
        if (RTGUI_WIDGET_IS_ENABLE(widget) && !RTGUI_WIDGET_IS_HIDE(widget))
        {
            struct rtgui_event_mouse *emouse = (struct rtgui_event_mouse *)event;
            if (emouse->button & RTGUI_MOUSE_BUTTON_LEFT &&
                    emouse->button & RTGUI_MOUSE_BUTTON_UP)
            {
                /* set focus */
                rtgui_widget_focus(widget);

                if (box->status_down & RTGUI_CHECKBOX_STATUS_UNCHECKED)
                {
                    /* check it */
                    box->status_down = RTGUI_CHECKBOX_STATUS_CHECKED;
                }
                else
                {
                    /* un-check it */
                    box->status_down = RTGUI_CHECKBOX_STATUS_UNCHECKED;
                }
            }

            /* draw checkbox */
            rtgui_checkbox_ondraw(box);

            /* call user callback */
            if (widget->on_mouseclick != RT_NULL)
            {
                return widget->on_mouseclick(RTGUI_OBJECT(widget), event);
            }
            if (box->on_button != RT_NULL)
            {
                box->on_button(RTGUI_OBJECT(widget), event);
                return RT_TRUE;
            }
        }

        return RT_TRUE;
    }
    default:
        return rtgui_widget_event_handler(object, event);
    }

    return RT_FALSE;
}

struct rtgui_checkbox *rtgui_checkbox_create(rtgui_container_t *container, const char *text, rt_bool_t checked, int left, int top)
{
    struct rtgui_checkbox *box;
	RT_ASSERT(container != RT_NULL);

    box = (struct rtgui_checkbox *) rtgui_widget_create(RTGUI_CHECKBOX_TYPE);
    if (box != RT_NULL)
    {
        rtgui_rect_t rect,text_rect;
		rtgui_widget_get_rect(RTGUI_WIDGET(container), &rect);
		rtgui_widget_rect_to_device(RTGUI_WIDGET(container), &rect);
        /* set default rect */
        rtgui_font_get_metrics(rtgui_font_default(), text, &text_rect);
        text_rect.x2 += RTGUI_BORDER_DEFAULT_WIDTH + 5 + (RTGUI_BORDER_DEFAULT_WIDTH << 1);
        text_rect.y2 += (RTGUI_BORDER_DEFAULT_WIDTH << 1);
		
		rect.x1 += left;
		rect.y1 += top;
		rect.x2 = rect.x1 + 16 + RC_W(text_rect);
		rect.y2 = rect.y1 + RC_H(text_rect);

        rtgui_widget_set_rect(RTGUI_WIDGET(box), &rect);
        rtgui_label_set_text(RTGUI_LABEL(box), text);

        if (checked == RT_TRUE)
            box->status_down = RTGUI_CHECKBOX_STATUS_CHECKED;
        else
            box->status_down = RTGUI_CHECKBOX_STATUS_UNCHECKED;
		rtgui_container_add_child(container, RTGUI_WIDGET(box));
    }

    return box;
}

void rtgui_checkbox_destroy(rtgui_checkbox_t *box)
{
    rtgui_widget_destroy(RTGUI_WIDGET(box));
}

void rtgui_checkbox_set_checked(rtgui_checkbox_t *checkbox, rt_bool_t checked)
{
    RT_ASSERT(checkbox != RT_NULL);
    if (checked == RT_TRUE)
        checkbox->status_down = RTGUI_CHECKBOX_STATUS_CHECKED;
    else
        checkbox->status_down = RTGUI_CHECKBOX_STATUS_UNCHECKED;

}

rt_bool_t rtgui_checkbox_get_checked(rtgui_checkbox_t *checkbox)
{
    RT_ASSERT(checkbox != RT_NULL);

    if (checkbox->status_down == RTGUI_CHECKBOX_STATUS_CHECKED)
        return RT_TRUE;

    return RT_FALSE;
}

static rt_bool_t rtgui_checkbox_onunfocus(struct rtgui_object *object, rtgui_event_t *event)
{
	rtgui_rect_t rect;
	rtgui_checkbox_t *box = RTGUI_CHECKBOX(object);
	struct rtgui_dc *dc;

	RT_ASSERT(box != RT_NULL);

	dc = rtgui_dc_begin_drawing(RTGUI_WIDGET(box));
	if (dc == RT_NULL) return RT_FALSE;

	rtgui_widget_get_rect(RTGUI_WIDGET(box), &rect);

	if (!RTGUI_WIDGET_IS_FOCUSED(box))
	{
		rtgui_rect_t tmp_rect;
		rtgui_color_t color;
		rtgui_font_get_string_rect(RTGUI_WIDGET_FONT(box), rtgui_label_get_text(RTGUI_LABEL(box)), &tmp_rect);
		rtgui_rect_moveto(&tmp_rect, rect.x1+CHECK_BOX_W+5, rect.y1);
		tmp_rect.x1 -= 1;
		tmp_rect.x2 += 2;
		tmp_rect.y2 = rect.y2-1;

		color = RTGUI_DC_FC(dc);
		RTGUI_DC_FC(dc) = RTGUI_DC_BC(dc);
		rtgui_dc_draw_focus_rect(dc, &tmp_rect);
		RTGUI_DC_FC(dc) = color;
	}

	rtgui_dc_end_drawing(dc);
	return RT_TRUE;
}
