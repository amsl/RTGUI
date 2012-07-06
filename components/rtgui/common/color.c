/*
 * File      : color.c
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
#include <rtgui/color.h>

#define RTGUI_RGB_R(c)	((c) & 0xff)
#define RTGUI_RGB_G(c)	(((c) >> 8)  & 0xff)
#define RTGUI_RGB_B(c)	(((c) >> 16) & 0xff)
#define RTGUI_RGB_A(c)	(((c) >> 24) & 0xff)

