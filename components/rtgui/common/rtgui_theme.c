#include <rtgui/rtgui.h>
#include <rtgui/dc.h>
#include <rtgui/rtgui_theme.h>

struct rtgui_theme	theme;

void rtgui_theme_init(void)
{
	theme.style = RTGUI_BORDER_UP; //系统显示外观样式
	theme.foreground = Black;//系统前景色
	theme.background = RTGUI_RGB(225,225,225);//系统背景色
	theme.win_active = Blue;//活动窗口颜色
	theme.win_deactive = Gray;//非活动窗口颜色
	theme.blankspace = White;
}


