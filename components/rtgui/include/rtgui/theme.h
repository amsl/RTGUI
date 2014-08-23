/*
 * File      : theme.h
 * This file is part of RTGUI in RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-10-04     Bernard      first version
 */
#ifndef __RTGUI_THEME_H__
#define __RTGUI_THEME_H__

#include <rtgui/rtgui.h>

#define CHECK_BOX_W		13
#define CHECK_BOX_H		13

#define RADIO_BOX_W		12
#define RADIO_BOX_H		12

#include <rtgui/widgets/widget.h>
#include <rtgui/widgets/container.h>
#include <rtgui/widgets/label.h>
#include <rtgui/widgets/button.h>
#include <rtgui/widgets/textbox.h>
#include <rtgui/widgets/iconbox.h>
#include <rtgui/widgets/checkbox.h>
#include <rtgui/widgets/radiobox.h>
#include <rtgui/widgets/listbox.h>
#include <rtgui/widgets/scrollbar.h>
#include <rtgui/widgets/slider.h>
#include <rtgui/widgets/progressbar.h>
#include <rtgui/widgets/staticline.h>
#include <rtgui/widgets/window.h>
#include <rtgui/widgets/view.h>
#include <rtgui/widgets/menu.h>
#include <rtgui/widgets/combobox.h>

#ifdef __cplusplus
extern "C" {
#endif

void system_theme_init(void);

void theme_draw_win_closebox(win_t *win);
void theme_draw_win_maxbox(win_t *win);
void theme_draw_win_minbox(win_t *win);
void theme_draw_win_title(win_t *win);
void theme_draw_win(win_t* win);
void theme_draw_button(button_t* btn);
void theme_draw_label(label_t* label);
void theme_draw_textbox(textbox_t* box);
void theme_draw_iconbox(iconbox_t* iconbox);
void theme_draw_checkbox(checkbox_t* checkbox);
void theme_draw_radiobox(radiobox_t* rbox);
void theme_draw_menu(menu_t* menu);
void theme_draw_menu_item(menu_t* menu, menu_item_t *item);
void theme_draw_slider(slider_t* slider);
void theme_draw_scrollbar(scrollbar_t* bar);
void theme_draw_progressbar(progressbar_t* bar);
void theme_draw_staticline(staticline_t* sline);

void theme_draw_selected(dc_t *dc, rect_t *rect);

color_t theme_default_bc(void);//default black ground color
color_t theme_default_fc(void);
void theme_draw_panel(panel_t *panel);
void theme_draw_combo(combo_t *cbo);
void theme_draw_combo_downarrow(combo_t *cbo);

#ifdef __cplusplus
}
#endif

#endif
