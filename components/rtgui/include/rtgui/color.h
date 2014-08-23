/*
 * File      : color.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-10-16     Bernard      first version
 */
#ifndef __RTGUI_COLOR_H__
#define __RTGUI_COLOR_H__

//#include <stdlib.h>
#include <rtgui/rtgui.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RGB_EXCHANGE_16

#define RTGUI_ARGB(r, g, b, a)	\
		((rt_uint32_t)(((rt_uint8_t)(r)|\
		(((unsigned)(rt_uint8_t)(g))<<8))|\
		(((unsigned long)(rt_uint8_t)(b))<<16)|\
		(((unsigned long)(rt_uint8_t)(a))<<24)))

#define RTGUI_RGB(r, g, b) \
		((((b) >> 3) << 11)|\
		(((g) >> 2) << 5) |\
		((r)>> 3))
//#define RTGUI_RGB(r, g, b)	RTGUI_ARGB(255, (r), (g), (b))

#define RAND_COLOR 		RTGUI_RGB(rand()%255, rand()%255, rand()%255)

#define RTGUI_RGB_R(c)	((c) & 0xff)
#define RTGUI_RGB_G(c)	(((c) >> 8)  & 0xff)
#define RTGUI_RGB_B(c)	(((c) >> 16) & 0xff)
#define RTGUI_RGB_A(c)	(((c) >> 24) & 0xff)

/*
 * RTGUI default color format
 * BBBBB GGGGGG RRRRR
 */

/* convert rtgui 32 to 16 (RGB=5:6:5) */
rt_inline rt_uint16_t rtgui_color_to_565(rt_uint32_t c)
{
	rt_uint16_t pixel;

#ifdef RGB_EXCHANGE_16
	pixel = (rt_uint16_t)(((RTGUI_RGB_B(c)>> 3) << 11) | ((RTGUI_RGB_G(c) >> 2) << 5) | (RTGUI_RGB_R(c) >> 3));
#else
	pixel = (rt_uint16_t)(((RTGUI_RGB_R(c) >> 3) << 11) | ((RTGUI_RGB_G(c) >> 2) << 5) | (RTGUI_RGB_B(c)>> 3));
#endif

	return pixel;
}

rt_inline rt_uint32_t rtgui_color_from_565(rt_uint16_t pixel)
{
	rt_uint8_t r, g, b;
	rt_uint32_t color;
	
	r = (pixel >> 11) & 0x1f;
	g = (pixel >> 5)  & 0x3f;
	b = pixel & 0x1f;
	
#ifdef RGB_EXCHANGE_16
 	color = b * 255 / 31 + ((g * 255 / 63) << 8) + ((r * 255 / 31) << 16);
#else
	color = r * 255 / 31 + ((g * 255 / 63) << 8) + ((b * 255 / 31) << 16);
#endif

	return color;
}

rt_inline rt_uint16_t rgb_exchange_16(rt_uint16_t c)
{
	rt_uint16_t color;
	color =  c&0x07E0;
	color += c<<11;
	color += c>>11;
	return color;
}

enum
{
	LightPink				= RTGUI_RGB(255,182,193),//浅粉色
	Pink					= RTGUI_RGB(255,192,203),//粉红
	Crimson					= RTGUI_RGB(220,20, 60 ),//猩红
	LavenderBlush			= RTGUI_RGB(255,240,245),//脸红的淡紫色
	PaleVioletRed			= RTGUI_RGB(219,112,147),//苍白的紫罗兰红色
	HotPink					= RTGUI_RGB(255,105,180),//热情的粉红
	DeepPink				= RTGUI_RGB(255,20, 147),//深粉色
	MediumVioletRed			= RTGUI_RGB(199,21, 133),//适中的紫罗兰红色
	Orchid					= RTGUI_RGB(218,112,214),//兰花的紫色
	Thistle					= RTGUI_RGB(216,191,216),//蓟
	plum					= RTGUI_RGB(221,160,221),//李子
	Violet					= RTGUI_RGB(238,130,238),//紫罗兰
	Magenta					= RTGUI_RGB(255,0,  255),//洋红
	Fuchsia					= RTGUI_RGB(255,0,  255),//灯笼海棠(紫红色)
	DarkMagenta				= RTGUI_RGB(139,0,  139),//深洋红色
	Purple					= RTGUI_RGB(128,0,  128),//紫色
	MediumOrchid			= RTGUI_RGB(186,85, 211),//适中的兰花紫
	DarkVoilet				= RTGUI_RGB(148,0,  211),//深紫罗兰色
	DarkOrchid				= RTGUI_RGB(153,50, 204),//深兰花紫
	Indigo					= RTGUI_RGB(75, 0,  130),//靛青
	BlueViolet				= RTGUI_RGB(138,43, 226),//深紫罗兰的蓝色
	MediumPurple			= RTGUI_RGB(147,112,219),//适中的紫色
	MediumSlateBlue			= RTGUI_RGB(123,104,238),//适中的板岩暗蓝灰色
	SlateBlue				= RTGUI_RGB(106,90, 205),//板岩暗蓝灰色
	DarkSlateBlue			= RTGUI_RGB(72, 61, 139),//深岩暗蓝灰色
	Lavender				= RTGUI_RGB(230,230,250),//薰衣草花的淡紫色
	GhostWhite				= RTGUI_RGB(248,248,255),//幽灵的白色
	Blue					= RTGUI_RGB(0,  0,  255),//纯蓝
	MediumBlue				= RTGUI_RGB(0,  0,  205),//适中的蓝色
	MidnightBlue			= RTGUI_RGB(25, 25, 112),//午夜的蓝色
	DarkBlue				= RTGUI_RGB(0,  0,  139),//深蓝色
	Navy					= RTGUI_RGB(0,  0,  128),//海军蓝
	RoyalBlue				= RTGUI_RGB(65, 105,225),//皇军蓝
	CornflowerBlue			= RTGUI_RGB(100,149,237),//矢车菊的蓝色
	LightSteelBlue			= RTGUI_RGB(176,196,222),//淡钢蓝
	LightSlateGray			= RTGUI_RGB(119,136,153),//浅石板灰
	SlateGray				= RTGUI_RGB(112,128,144),//石板灰
	DoderBlue				= RTGUI_RGB(30, 144,255),//道奇蓝
	AliceBlue				= RTGUI_RGB(240,248,255),//爱丽丝蓝
	SteelBlue				= RTGUI_RGB(70, 130,180),//钢蓝
	LightSkyBlue			= RTGUI_RGB(135,206,250),//淡蓝色
	SkyBlue					= RTGUI_RGB(90,130,235),//天蓝色
	DeepSkyBlue				= RTGUI_RGB(0,  191,255),//深天蓝
	LightBLue				= RTGUI_RGB(173,216,230),//淡蓝
	PowDerBlue				= RTGUI_RGB(176,224,230),//火药蓝
	CadetBlue				= RTGUI_RGB(95, 158,160),//军校蓝
	Azure					= RTGUI_RGB(240,255,255),//蔚蓝色
	LightCyan				= RTGUI_RGB(225,255,255),//淡青色
	PaleTurquoise			= RTGUI_RGB(175,238,238),//苍白的绿宝石
	Cyan					= RTGUI_RGB(0,  255,255),//青色
	Aqua					= RTGUI_RGB(0,  255,255),//水绿色
	DarkTurquoise			= RTGUI_RGB(0,  206,209),//深绿宝石
	DarkSlateGray			= RTGUI_RGB(47, 79, 79 ),//深石板灰
	DarkCyan				= RTGUI_RGB(0,  139,139),//深青色
	Teal					= RTGUI_RGB(0,  128,128),//水鸭色
	MediumTurquoise			= RTGUI_RGB(72, 209,204),//适中的绿宝石
	LightSeaGreen			= RTGUI_RGB(32, 178,170),//浅海洋绿
	Turquoise				= RTGUI_RGB(64, 224,208),//绿宝石
	Auqamarin				= RTGUI_RGB(127,255,170),//绿玉\碧绿色
	MediumAquamarine		= RTGUI_RGB(0,  250,154),//适中的碧绿色
	MediumSpringGreen		= RTGUI_RGB(245,255,250),//适中的春天的绿色
	MintCream				= RTGUI_RGB(0,  255,127),//薄荷奶油
	SpringGreen				= RTGUI_RGB(60, 179,113),//春天的绿色
	SeaGreen				= RTGUI_RGB(46, 139,87 ),//海洋绿
	Honeydew				= RTGUI_RGB(240,255,240),//蜂蜜
	LightGreen				= RTGUI_RGB(144,238,144),//淡绿色
	PaleGreen				= RTGUI_RGB(152,251,152),//苍白的绿色
	DarkSeaGreen			= RTGUI_RGB(143,188,143),//深海洋绿
	LimeGreen				= RTGUI_RGB(50, 205,50 ),//酸橙绿
	Lime					= RTGUI_RGB(0,  255,0  ),//酸橙色
	ForestGreen				= RTGUI_RGB(34, 139,34 ),//森林绿
	Green					= RTGUI_RGB(0,  128,0  ),//纯绿
	DarkGreen				= RTGUI_RGB(0,  100,0  ),//深绿色
	Chartreuse				= RTGUI_RGB(127,255,0  ),//查特酒绿
	LawnGreen				= RTGUI_RGB(124,252,0  ),//草坪绿
	GreenYellow				= RTGUI_RGB(173,255,47 ),//绿黄色
	OliveDrab				= RTGUI_RGB(85, 107,47 ),//橄榄土褐色
	Beige					= RTGUI_RGB(107,142,35 ),//米色(浅褐色)
	LightGoldenrodYellow	= RTGUI_RGB(250,250,210),//浅秋麒麟黄
	Ivory					= RTGUI_RGB(255,255,240),//象牙
	LightYellow				= RTGUI_RGB(255,255,224),//浅黄色
	Yellow					= RTGUI_RGB(255,255,0  ),//纯黄
	Olive					= RTGUI_RGB(128,128,0  ),//橄榄
	DarkKhaki				= RTGUI_RGB(189,183,107),//深卡其布
	LemonChiffon			= RTGUI_RGB(255,250,205),//柠檬薄纱
	PaleGodenrod			= RTGUI_RGB(238,232,170),//灰秋麒麟
	Khaki					= RTGUI_RGB(240,230,140),//卡其布
	Gold					= RTGUI_RGB(255,215,0  ),//金
	Cornislk				= RTGUI_RGB(255,248,220),//玉米色
	GoldEnrod				= RTGUI_RGB(218,165,32 ),//秋麒麟
	FloralWhite				= RTGUI_RGB(255,250,240),//花的白色
	OldLace					= RTGUI_RGB(253,245,230),//老饰带
	Wheat					= RTGUI_RGB(245,222,179),//小麦色
	Moccasin				= RTGUI_RGB(255,228,181),//鹿皮鞋
	Orange					= RTGUI_RGB(255,165,0  ),//橙色
	PapayaWhip				= RTGUI_RGB(255,239,213),//番木瓜
	BlanchedAlmond			= RTGUI_RGB(255,235,205),//漂白的杏仁
	NavajoWhite				= RTGUI_RGB(255,222,173),//
	AntiqueWhite			= RTGUI_RGB(250,235,215),//古代的白色
	Tan						= RTGUI_RGB(210,180,140),//晒黑
	BrulyWood				= RTGUI_RGB(222,184,135),//结实的树
	Bisque					= RTGUI_RGB(255,228,196),//(浓汤)乳脂,番茄等
	DarkOrange				= RTGUI_RGB(255,140,0  ),//深橙色
	Linen					= RTGUI_RGB(250,240,230),//亚麻布
	Peru					= RTGUI_RGB(205,133,63 ),//秘鲁
	PeachPuff				= RTGUI_RGB(255,218,185),//桃色
	SandyBrown				= RTGUI_RGB(244,164,96 ),//沙棕色
	Chocolate				= RTGUI_RGB(210,105,30 ),//巧克力
	SaddleBrown				= RTGUI_RGB(139,69, 19),//马鞍棕色
	SeaShell				= RTGUI_RGB(255,245,238),//海贝壳
	Sienna					= RTGUI_RGB(160,82, 45 ),//黄土赭色
	LightSalmon				= RTGUI_RGB(255,160,122),//浅鲜肉(鲑鱼)色
	Coral					= RTGUI_RGB(255,127,80 ),//珊瑚
	OrangeRed				= RTGUI_RGB(255,69, 0  ),//橙红色
	DarkSalmon				= RTGUI_RGB(233,150,122),//深鲜肉(鲑鱼)色
	Tomato					= RTGUI_RGB(255,99, 71 ),//番茄
	MistyRose				= RTGUI_RGB(255,228,225),//薄雾玫瑰
	Salmon					= RTGUI_RGB(250,128,114),//鲜肉(鲑鱼)色
	Snow					= RTGUI_RGB(255,250,250),//雪
	LightCoral				= RTGUI_RGB(240,128,128),//淡珊瑚色
	RosyBrown				= RTGUI_RGB(188,143,143),//玫瑰棕色
	IndianRed				= RTGUI_RGB(205,92, 92 ),//印度红
	Red						= RTGUI_RGB(255,0,  0  ),//纯红
	Brown					= RTGUI_RGB(165,42, 42 ),//棕色
	FireBrick				= RTGUI_RGB(178,34, 34 ),//耐火砖
	DarkRed					= RTGUI_RGB(139,0,  0  ),//深红色
	Maroon					= RTGUI_RGB(128,0,  0  ),//栗色
	White					= RTGUI_RGB(255,255,255),//纯白
	WhiteSmoke				= RTGUI_RGB(245,245,245),//白烟
	Gainsboro				= RTGUI_RGB(220,220,220),//Gainsboro
	LightGrey				= RTGUI_RGB(211,211,211),//浅灰色
	Silver					= RTGUI_RGB(192,192,192),//银白色
	DarkGray				= RTGUI_RGB(169,169,169),//深灰色
	Gray					= RTGUI_RGB(128,128,128),//灰色
	DimGray					= RTGUI_RGB(105,105,105),//暗淡的灰色
	Black					= RTGUI_RGB(0,  0,  0  ),//纯黑
	DefaultBackground		= RTGUI_RGB(226,223,226),//默认背景色
};

#define default_foreground	Black
#define	default_background	DefaultBackground

#ifdef __cplusplus
}
#endif

#endif

