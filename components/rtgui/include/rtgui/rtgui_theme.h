#ifndef __RTGUI_THEME_H__
#define __RTGUI_THEME_H__

#include <rtgui/color.h>

struct rtgui_theme
{
	rt_uint8_t	style;
	rtgui_color_t foreground;	//系统前景色
	rtgui_color_t background;	//系统背景色
	rtgui_color_t win_active;	//活动窗口颜色
	rtgui_color_t win_deactive; //非活动窗口颜色
	rtgui_color_t blankspace;   //空白区颜色,如编辑框,列表框的底色
};

void rtgui_theme_init(void);

extern struct rtgui_theme	theme;



#endif

