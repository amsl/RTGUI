/*
 * File      : fileview.c
 * This file is part of RTGUI in RT-Thread RTOS
 * COPYRIGHT (C) 2010, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 *
 */

#include <rtgui/widgets/fileview.h>
#include <rtgui/widgets/view.h>
#include <rtgui/widgets/textbox.h>
#include <rtgui/widgets/listbox.h>
#include <rtgui/widgets/window.h>

#if defined(RTGUI_USING_DFS_FILERW)
#ifdef _WIN32
#include <io.h>
#include <dirent.h>
#include <sys/stat.h>
#define PATH_SEPARATOR		'\\'
#else
#include <dfs_posix.h>
#define PATH_SEPARATOR		'/'
#endif

#include <string.h>

const static char * file_xpm[] =
{
	"16 16 21 1",
	" 	c None",
	".	c #999999",
	"+	c #818181",
	"@	c #FFFFFF",
	"#	c #ECECEC",
	"$	c #EAEAEA",
	"%	c #EBEBEB",
	"&	c #EDEDED",
	"*	c #F0F0F0",
	"=	c #C4C4C4",
	"-	c #C5C5C5",
	";	c #C6C6C6",
	">	c #C7C7C7",
	",	c #EEEEEE",
	"'	c #EDEDE5",
	")	c #EDEDE6",
	"!	c #EFEFEF",
	"~	c #C8C8C8",
	"{	c #F1F1F1",
	"]	c #F2F2F2",
	"^	c #959595",
	".++++++++++++   ",
	"+@@@@@@@@@@@@+  ",
	"+@#$$%%%##&*@+  ",
	"+@$=--;;;;>*@+  ",
	"+@$%%###&&,*@+  ",
	"+@%-;;;;;;>*@+  ",
	"+@%%##&&'#,*@+  ",
	"+@%;;;;,,),*@+  ",
	"+@##&&,,!!!*@+  ",
	"+@#;;;>>~~~*@+  ",
	"+@#&,,!!*{{{@+  ",
	"+@&;>>~~~{{]@+  ",
	"+@&&,!!**{]]@+  ",
	"+@@@@@@@@@@@@+  ",
	"^++++++++++++^  ",
	"                "
};

const static char * folder_xpm[] =
{
	"16 16 121 2",
	"  	c None",
	". 	c #D9B434",
	"+ 	c #E1C25E",
	"@ 	c #E2C360",
	"# 	c #E2C35F",
	"$ 	c #DBB63C",
	"% 	c #DAB336",
	"& 	c #FEFEFD",
	"* 	c #FFFFFE",
	"= 	c #FFFEFE",
	"- 	c #FFFEFD",
	"; 	c #FBF7EA",
	"> 	c #E4C76B",
	", 	c #E3C76B",
	"' 	c #E6CD79",
	") 	c #E5CA74",
	"! 	c #DAAF35",
	"~ 	c #FEFCF7",
	"{ 	c #F8E48E",
	"] 	c #F5DE91",
	"^ 	c #F5E09F",
	"/ 	c #F6E1AC",
	"( 	c #FEFBEF",
	"_ 	c #FEFDF4",
	": 	c #FEFCF3",
	"< 	c #FEFCF1",
	"[ 	c #FEFBEE",
	"} 	c #FFFDFA",
	"| 	c #DAAF36",
	"1 	c #DAAA36",
	"2 	c #FDFAF1",
	"3 	c #F5DE94",
	"4 	c #F4DC93",
	"5 	c #F2D581",
	"6 	c #EDCA6A",
	"7 	c #EACB6C",
	"8 	c #EFD385",
	"9 	c #EFD280",
	"0 	c #EFD07A",
	"a 	c #EECF76",
	"b 	c #EECF72",
	"c 	c #FBF7E9",
	"d 	c #DAAE34",
	"e 	c #DAAB35",
	"f 	c #FBF6E8",
	"g 	c #EFD494",
	"h 	c #EECE88",
	"i 	c #E9C173",
	"j 	c #F6E9C9",
	"k 	c #FEFCF2",
	"l 	c #FEFCF0",
	"m 	c #DAAB36",
	"n 	c #DAA637",
	"o 	c #FFFDF8",
	"p 	c #FFFDF6",
	"q 	c #FFFCF5",
	"r 	c #FCF6D8",
	"s 	c #F8E694",
	"t 	c #F7E385",
	"u 	c #F6DF76",
	"v 	c #F5DB68",
	"w 	c #F4D85C",
	"x 	c #FCF4D7",
	"y 	c #DAA435",
	"z 	c #DAA136",
	"A 	c #FEFCF6",
	"B 	c #FCF2C8",
	"C 	c #FBEFB9",
	"D 	c #FAECAC",
	"E 	c #F9E89C",
	"F 	c #F7E38B",
	"G 	c #F6E07C",
	"H 	c #F6DC6C",
	"I 	c #F5D95D",
	"J 	c #F4D64F",
	"K 	c #F3D344",
	"L 	c #FCF3D0",
	"M 	c #DA9F35",
	"N 	c #DA9A36",
	"O 	c #FDFAF2",
	"P 	c #FAEDB3",
	"Q 	c #F9E9A4",
	"R 	c #F8E695",
	"S 	c #F7E285",
	"T 	c #F6DE76",
	"U 	c #F5DB65",
	"V 	c #F4D757",
	"W 	c #F3D449",
	"X 	c #F2D13B",
	"Y 	c #F1CE30",
	"Z 	c #FBF2CC",
	"` 	c #DA9835",
	" .	c #DA9435",
	"..	c #FEFAEF",
	"+.	c #F9E9A1",
	"@.	c #F8E591",
	"#.	c #F7E181",
	"$.	c #F6DE72",
	"%.	c #F5DA63",
	"&.	c #F4D754",
	"*.	c #F3D347",
	"=.	c #F2D039",
	"-.	c #F1CD2E",
	";.	c #F0CB26",
	">.	c #FBF2CA",
	",.	c #D98E33",
	"'.	c #FAF0DC",
	").	c #F4DDA7",
	"!.	c #F4DB9E",
	"~.	c #F3DA96",
	"{.	c #F3D88E",
	"].	c #F3D786",
	"^.	c #F2D47F",
	"/.	c #F2D379",
	"(.	c #F1D272",
	"_.	c #F1D06C",
	":.	c #F1CF69",
	"<.	c #F8EAC2",
	"[.	c #D8882D",
	"}.	c #D8872D",
	"|.	c #D8862C",
	"                                ",
	"                                ",
	"                                ",
	"  . + @ @ @ # $                 ",
	"  % & * = - * ; > , , , ' )     ",
	"  ! ~ { ] ^ / ( _ : < ( [ } |   ",
	"  1 2 3 4 5 6 7 8 9 0 a b c d   ",
	"  e f g h i j k : k l ( [ * m   ",
	"  n * o p q : r s t u v w x y   ",
	"  z A B C D E F G H I J K L M   ",
	"  N O P Q R S T U V W X Y Z `   ",
	"   ...+.@.#.$.%.&.*.=.-.;.>. .  ",
	"  ,.'.).!.~.{.].^./.(._.:.<.,.  ",
	"    [.}.[.[.[.[.[.[.[.[.}.[.|.  ",
	"                                ",
	"                                "
};

/* image for file and folder */
static rtgui_image_t *file_image   = RT_NULL;
static rtgui_image_t *folder_image = RT_NULL;

static rt_bool_t rtgui_fileview_onunfocus(pvoid wdt, rtgui_event_t* event);
static rt_bool_t rtgui_fileview_sbar_handle(pvoid wdt, rtgui_event_t* event);

static void _rtgui_fileview_constructor(rtgui_fileview_t *fview)
{
	/* set default widget rect and set event handler */
	rtgui_widget_set_event_handler(fview, rtgui_fileview_event_handler);
	rtgui_widget_set_onunfocus(fview, rtgui_fileview_onunfocus);

	RTGUI_WIDGET_FLAG(fview) |= RTGUI_WIDGET_FLAG_FOCUSABLE;

	fview->first_item = 0;
	fview->now_item = 0;
	fview->item_count = 0;
	fview->item_per_page = 0;

	fview->current_dir = RT_NULL;
	fview->pattern = RT_NULL;
	RTGUI_WIDGET_BC(fview) = theme.blankspace;
	RTGUI_WIDGET_TEXTALIGN(fview) = RTGUI_ALIGN_CENTER_VERTICAL;
	
	if(theme.style == RTGUI_BORDER_UP)
		rtgui_widget_set_border_style(fview,RTGUI_BORDER_DOWN);
	else if(theme.style == RTGUI_BORDER_EXTRA)
		rtgui_widget_set_border_style(fview,RTGUI_BORDER_SUNKEN);

	fview->on_item = RT_NULL;
	fview->dlg = RT_NULL;

	if(file_image==RT_NULL)
		file_image = rtgui_image_create_from_mem("xpm",(rt_uint8_t*)file_xpm, sizeof(file_xpm), RT_TRUE);
	if(folder_image==RT_NULL)
		folder_image = rtgui_image_create_from_mem("xpm",(rt_uint8_t*)folder_xpm, sizeof(folder_xpm), RT_TRUE);
}

static void _rtgui_fileview_destructor(rtgui_fileview_t *fview)
{
	rtgui_fileview_destroy(fview);
}

DEFINE_CLASS_TYPE(fileview, "fileview",
                  RTGUI_CONTAINER_TYPE,
                  _rtgui_fileview_constructor,
                  _rtgui_fileview_destructor,
                  sizeof(struct rtgui_fileview));

rtgui_fileview_t* rtgui_fileview_create(pvoid parent, const char* directory, const char* pattern, int left, int top, int w, int h)
{
	rtgui_container_t *container;
	rtgui_fileview_t* fview = RT_NULL;

	RT_ASSERT(parent != RT_NULL);
	container = RTGUI_CONTAINER(parent);

	/* create a new view */
	fview = rtgui_widget_create(RTGUI_FILEVIEW_TYPE);

	if(fview != RT_NULL)
	{
		rtgui_rect_t rect;

		rtgui_widget_get_rect(container, &rect);
		rtgui_widget_rect_to_device(container,&rect);
		rect.x1 += left;
		rect.y1 += top;
		rect.x2 = rect.x1+w;
		rect.y2 = rect.y1+h;

		fview->items = RT_NULL;
		fview->pattern = rt_strdup(pattern);
		fview->item_per_page = RC_H(rect) / (1 + RTGUI_SEL_H);

		rtgui_widget_set_rect(fview,&rect);
		rtgui_container_add_child(container, fview);

		{
			/* create scrollbar */
			rt_uint32_t sLeft,sTop,sw=RTGUI_DEFAULT_SB_WIDTH,sLen;
			sLeft = RC_W(rect)-RTGUI_WIDGET_BORDER_SIZE(fview)-sw;
			sTop = RTGUI_WIDGET_BORDER_SIZE(fview);

			sLen = rect.y2-rect.y1-RTGUI_WIDGET_BORDER_SIZE(fview)*2;
			fview->sbar = rtgui_scrollbar_create(fview,sLeft,sTop,sw,sLen,RTGUI_VERTICAL);
			if(fview->sbar != RT_NULL)
			{
				fview->sbar->widget_link = fview;
				fview->sbar->on_scroll = rtgui_fileview_sbar_handle;
				RTGUI_WIDGET_HIDE(fview->sbar);/* default hid scrollbar */
			}
		}

		rtgui_fileview_set_directory(fview, directory);
	}

	return fview;
}

static void rtgui_fileview_clear(rtgui_fileview_t* view);

void rtgui_fileview_destroy(rtgui_fileview_t* fview)
{
	/* delete all file items */
	rtgui_fileview_clear(fview);

	/* delete current directory and pattern */
	if(fview->current_dir != RT_NULL)
	{
		rt_free(fview->current_dir);
		fview->current_dir = RT_NULL;
	}

	if(fview->pattern != RT_NULL)
	{
		rt_free(fview->pattern);
		fview->pattern = RT_NULL;
	}
}

/* set fview directory on top folder */
void rtgui_fileview_goto_topfolder(rtgui_fileview_t* fview)
{
	char* dirstr = fview->current_dir;

	if(strlen(dirstr) > 1)
	{
		char new_path[256];
		char* ptr = strrchr(dirstr,PATH_SEPARATOR);/* last char '/' */

		if(ptr == dirstr)
		{
			/* It's root dir */
			new_path[0] = PATH_SEPARATOR;
			new_path[1] = '\0';
		}
		else
		{
			strncpy(new_path, dirstr, ptr - dirstr + 1);
			new_path[ptr - dirstr] = '\0';
		}
		dirstr = new_path;

		rtgui_fileview_set_directory(fview, dirstr);
	}
}

static void rtgui_fileview_on_folder_item(rtgui_fileview_t *fview, rtgui_event_t* event)
{
	char* dir_ptr;

	dir_ptr = (char*)rt_malloc(256);
	if(dir_ptr==RT_NULL)
	{
		return;
	}
	/* no file, exit */
	if(fview->items==RT_NULL)
	{
		rt_free(dir_ptr);
		return;
	}

	rtgui_fileview_get_fullpath(fview, dir_ptr, 256);

	rtgui_fileview_set_directory(fview, dir_ptr);
	rt_free(dir_ptr);
}

static void rtgui_fileview_on_file_item(rtgui_fileview_t *fview, rtgui_event_t* event)
{
	if(fview == RT_NULL) return;

	if(fview->on_item != RT_NULL)
		fview->on_item(fview, event);
}

void rtgui_fileview_ondraw(rtgui_fileview_t* fview)
{
	rt_uint16_t first, i,rx2;
	rtgui_fileview_item_t* item;
	rtgui_rect_t rect, item_rect, image_rect;
	rtgui_dc_t* dc;

	RT_ASSERT(fview != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(fview);
	if(dc == RT_NULL)return;

	rtgui_widget_get_rect(fview, &rect);
	rtgui_dc_draw_border(dc, &rect,RTGUI_WIDGET_BORDER_STYLE(fview));
	rtgui_rect_inflate(&rect,-RTGUI_WIDGET_BORDER_SIZE(fview));
	RTGUI_DC_BC(dc) = theme.blankspace;
	rtgui_dc_fill_rect(dc,&rect);
	rtgui_rect_inflate(&rect,RTGUI_WIDGET_BORDER_SIZE(fview));
	if(fview->sbar && !RTGUI_WIDGET_IS_HIDE(fview->sbar))
		rect.x2 -= RC_W(fview->sbar->parent.extent);

	rect.x2 -=1;
	rect.y2 -= 1;

	/* get item base rect */
	item_rect = rect;
	item_rect.x1 += RTGUI_WIDGET_BORDER_SIZE(fview);
	item_rect.x2 -= RTGUI_WIDGET_BORDER_SIZE(fview);
	rx2 = item_rect.x2;
	item_rect.y1 += RTGUI_WIDGET_BORDER_SIZE(fview);
	item_rect.y2 = item_rect.y1 + (1 + RTGUI_SEL_H);

	/* get image base rect */
	image_rect.x1 = RTGUI_MARGIN;
	image_rect.y1 = 0;
	image_rect.x2 = RTGUI_MARGIN + file_image->w;
	image_rect.y2 = file_image->h;
	rtgui_rect_moveto_align(&item_rect, &image_rect, RTGUI_ALIGN_CENTER_VERTICAL);

	/* get current page */
	first = fview->first_item;
	for(i = 0; i < fview->item_per_page; i ++)
	{
		char str_size[32];

		if(first + i >= fview->item_count) break;

		item = &(fview->items[first + i]);

		/* draw item image */
		if(item->type == RTGUI_FITEM_FILE)
			rtgui_image_paste(file_image, dc, &image_rect, Black);
		else
			rtgui_image_paste(folder_image, dc, &image_rect, Black);

		/* draw text */
		item_rect.x1 += RTGUI_MARGIN + file_image->w + 2;
		item_rect.x2 = item_rect.x1 + rtgui_font_get_string_width(RTGUI_DC_FONT(dc), item->name);
		if(first + i == fview->now_item)
		{
			if(RTGUI_WIDGET_IS_FOCUSED(fview))
			{
				RTGUI_DC_BC(dc) = DarkBlue;
				RTGUI_DC_FC(dc) = theme.blankspace;
			}
			else
			{
				RTGUI_DC_BC(dc) = Gray;
				RTGUI_DC_FC(dc) = theme.foreground;
			}
			rtgui_dc_fill_rect(dc, &item_rect);
			rtgui_dc_draw_text(dc, item->name, &item_rect);
		}
		else
		{
			/* draw background */
			RTGUI_DC_BC(dc) = theme.blankspace;
			RTGUI_DC_FC(dc) = theme.foreground;
			rtgui_dc_fill_rect(dc,&item_rect);
			rtgui_dc_draw_text(dc, item->name, &item_rect);
		}

#if (1) /* please turn off it when need. */
		if(item->type == RTGUI_FITEM_FILE)
		{
			rtgui_rect_t rect=item_rect;
			/* print file information */
			rt_snprintf(str_size, 16, "(%dB)",item->size);
			rect.x1 = rect.x2 + RTGUI_MARGIN;
			rect.x2 = rect.x1 + rt_strlen(str_size) * FONT_W(RTGUI_WIDGET_FONT(fview));
			RTGUI_DC_FC(dc) = theme.foreground;
			rtgui_dc_draw_text(dc, str_size, &rect);
		}
#endif
		item_rect.x1 -= RTGUI_MARGIN + file_image->w + 2;
		item_rect.x2 = rx2;
		/* move to next item position */
		item_rect.y1 += (RTGUI_SEL_H + 1);
		item_rect.y2 += (RTGUI_SEL_H + 1);

		image_rect.y1 += (RTGUI_SEL_H + 1);
		image_rect.y2 += (RTGUI_SEL_H + 1);
	}

	if(fview->sbar && !RTGUI_WIDGET_IS_HIDE(fview->sbar))
	{
		rtgui_scrollbar_ondraw(fview->sbar);
	}

	rtgui_dc_end_drawing(dc);
}

/* update fileview */
void rtgui_fileview_update_current(rtgui_fileview_t* fview)
{
	rtgui_fileview_item_t *item;
	rtgui_rect_t rect, item_rect, image_rect;
	rtgui_dc_t *dc;

	RT_ASSERT(fview != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(fview);
	if(dc == RT_NULL)return;

	/* if directory is null, no dispost */
	if(fview->items==RT_NULL)return;

	rtgui_widget_get_rect(fview, &rect);
	if(fview->sbar && !RTGUI_WIDGET_IS_HIDE(fview->sbar))
		rect.x2 -= RC_W(fview->sbar->parent.extent);

	if((fview->old_item >= fview->first_item) &&
	        (fview->old_item < fview->first_item+fview->item_per_page) &&
	        (fview->old_item != fview->now_item))
	{
		/* these condition dispell blinked when drawed */
		/* get old item rect */
		item_rect = rect;
		item_rect.x1 += RTGUI_WIDGET_BORDER_SIZE(fview);
		item_rect.x2 -= RTGUI_WIDGET_BORDER_SIZE(fview);
		item_rect.y1 += RTGUI_WIDGET_BORDER_SIZE(fview);
		item_rect.y1 += ((fview->old_item-fview->first_item) % fview->item_per_page) * (1 + RTGUI_SEL_H);
		item_rect.y2 = item_rect.y1 + (1 + RTGUI_SEL_H);

		/* get image rect */
		image_rect.x1 = RTGUI_MARGIN;
		image_rect.y1 = 0;
		image_rect.x2 = RTGUI_MARGIN + file_image->w;
		image_rect.y2 = file_image->h;
		rtgui_rect_moveto_align(&item_rect, &image_rect, RTGUI_ALIGN_CENTER_VERTICAL);

		/* draw old item */
		item = &(fview->items[fview->old_item]);
		if(item->type == RTGUI_FITEM_FILE) /* draw item image */
			rtgui_image_paste(file_image, dc, &image_rect, Black);
		else
			rtgui_image_paste(folder_image, dc, &image_rect,Black);

		item_rect.x1 += RTGUI_MARGIN + file_image->w + 2;
		item_rect.x2 = item_rect.x1 + rtgui_font_get_string_width(RTGUI_DC_FONT(dc), item->name);
		RTGUI_DC_BC(dc) = theme.blankspace;
		RTGUI_DC_FC(dc) = theme.foreground;
		rtgui_dc_fill_rect(dc,&item_rect);
		rtgui_dc_draw_text(dc, item->name, &item_rect);
	}
	/* draw current item */
	item_rect = rect;
	item_rect.x1 += RTGUI_WIDGET_BORDER_SIZE(fview);
	item_rect.x2 -= RTGUI_WIDGET_BORDER_SIZE(fview);
	item_rect.y1 += RTGUI_WIDGET_BORDER_SIZE(fview);
	item_rect.y1 += ((fview->now_item-fview->first_item) % fview->item_per_page) * (1 + RTGUI_SEL_H);
	item_rect.y2 = item_rect.y1 + (1 + RTGUI_SEL_H);

	/* get image base rect */
	image_rect.x1 = RTGUI_MARGIN;
	image_rect.y1 = 0;
	image_rect.x2 = RTGUI_MARGIN + file_image->w;
	image_rect.y2 = file_image->h;
	rtgui_rect_moveto_align(&item_rect, &image_rect, RTGUI_ALIGN_CENTER_VERTICAL);

	item = &(fview->items[fview->now_item]);
	if(item->type == RTGUI_FITEM_FILE) /* draw item image */
		rtgui_image_paste(file_image, dc, &image_rect, Black);
	else
		rtgui_image_paste(folder_image, dc, &image_rect, Black);

	if(fview->dlg != RT_NULL)
	{
		if(fview->dlg->filename != RT_NULL)
		{
			rt_free(fview->dlg->filename);
			fview->dlg->filename = RT_NULL;
		}
		fview->dlg->filename = rt_strdup(item->name);
	}

	item_rect.x1 += RTGUI_MARGIN + file_image->w + 2;
	item_rect.x2 = item_rect.x1 + rtgui_font_get_string_width(RTGUI_DC_FONT(dc), item->name);

	{
		if(RTGUI_WIDGET_IS_FOCUSED(fview))
		{
			RTGUI_DC_BC(dc) = DarkBlue;
			RTGUI_DC_FC(dc) = theme.blankspace;
		}
		else
		{
			RTGUI_DC_BC(dc) = Gray;
			RTGUI_DC_FC(dc) = theme.foreground;
		}
		rtgui_dc_fill_rect(dc, &item_rect);
		rtgui_dc_draw_text(dc, item->name, &item_rect);
	}

	if(fview->dlg != RT_NULL)
	{
		if(item->type == RTGUI_FITEM_FILE)
		{
			if(fview->dlg->tbox_filename != RT_NULL)
			{
				rtgui_textbox_set_value(fview->dlg->tbox_filename,fview->dlg->filename);
				RTGUI_DC_FC(dc) = theme.foreground;
				rtgui_textbox_ondraw(fview->dlg->tbox_filename);
			}
		}
	}

	rtgui_dc_end_drawing(dc);
}

void rtgui_fileview_on_enter(rtgui_fileview_t* fview, rtgui_event_t* event)
{
	if(fview->items==RT_NULL)return;
	if(fview->item_count==0)return;

	if(fview->items[fview->now_item].type == RTGUI_FITEM_DIR)
	{
		/* directory */
		rtgui_fileview_on_folder_item(fview, event);
	}
	else
	{
		/* file */
		rtgui_fileview_on_file_item(fview, event);
	}
}

rt_bool_t rtgui_fileview_event_handler(pvoid wdt, rtgui_event_t* event)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(wdt);
	rtgui_fileview_t* fview = RTGUI_FILEVIEW(wdt);

	switch(event->type)
	{
	case RTGUI_EVENT_PAINT:
		rtgui_fileview_ondraw(fview);
		return RT_FALSE;

	case RTGUI_EVENT_MOUSE_BUTTON:
	{
		rtgui_rect_t rect;
		struct rtgui_event_mouse* emouse;

		emouse = (struct rtgui_event_mouse*)event;

		rtgui_widget_focus(fview);
		/* get physical extent information */
		rtgui_widget_get_rect(fview, &rect);
		rtgui_widget_rect_to_device(fview, &rect);

		if(fview->sbar && !RTGUI_WIDGET_IS_HIDE(fview->sbar))
			rect.x2 -= RC_W(fview->sbar->parent.extent);

		if((rtgui_rect_contains_point(&rect, emouse->x, emouse->y) == RT_EOK) && fview->item_count>0)
		{
			rt_uint16_t i;

			/* set focus */
			rtgui_widget_focus(fview);

			i = (emouse->y - rect.y1) / (2 + RTGUI_SEL_H);

			if((i < fview->item_count) && (i < fview->item_per_page))
			{
				if(emouse->button & RTGUI_MOUSE_BUTTON_DOWN)
				{
					fview->old_item = fview->now_item;
					fview->now_item = fview->first_item + i;
					rtgui_fileview_update_current(fview);
				}
				else if(emouse->button & RTGUI_MOUSE_BUTTON_UP)
				{
					if(fview->now_item==fview->old_item) return RT_FALSE;

					rtgui_fileview_update_current(fview);
				}
				if(fview->sbar && !RTGUI_WIDGET_IS_HIDE(fview))
				{
					if(!RTGUI_WIDGET_IS_HIDE(fview->sbar))
						rtgui_scrollbar_set_value(fview->sbar,fview->first_item);
				}
			}
		}
		rtgui_container_dispatch_mouse_event(fview, emouse);
		return RT_TRUE;
	}

	case RTGUI_EVENT_KBD:
	{
		struct rtgui_event_kbd *ekbd = (struct rtgui_event_kbd*)event;
		if(RTGUI_KBD_IS_DOWN(ekbd))
		{
			switch(ekbd->key)
			{
			case RTGUIK_UP:
				if(fview->now_item > 0)
				{
					fview->old_item = fview->now_item;
					fview->now_item --;

					if(fview->now_item < fview->first_item)
					{
						/* turn up page */
						fview->first_item = fview->now_item;
						rtgui_fileview_ondraw(fview);
					}
					else
					{
						/* current page */
						rtgui_fileview_update_current(fview);
					}

					if(fview->sbar && !RTGUI_WIDGET_IS_HIDE(fview))
					{
						if(!RTGUI_WIDGET_IS_HIDE(fview->sbar))
							rtgui_scrollbar_set_value(fview->sbar,fview->first_item);
					}
				}
				return RT_TRUE;

			case RTGUIK_DOWN:
				if(fview->now_item < fview->item_count-1)
				{
					fview->old_item = fview->now_item;
					fview->now_item ++;

					if(fview->now_item >= fview->first_item+fview->item_per_page)
					{
						/* turn down page */
						fview->first_item++;
						rtgui_fileview_ondraw(fview);
					}
					else
					{
						/* in current page */
						rtgui_fileview_update_current(fview);
					}
					if(fview->sbar && !RTGUI_WIDGET_IS_HIDE(fview))
					{
						if(!RTGUI_WIDGET_IS_HIDE(fview->sbar))
							rtgui_scrollbar_set_value(fview->sbar,fview->first_item);
					}
				}
				return RT_TRUE;

			case RTGUIK_LEFT:
				if(fview->item_count==0)return RT_FALSE;
				fview->old_item = fview->now_item;
				fview->now_item -= fview->item_per_page;

				if(fview->now_item < 0)
					fview->now_item = 0;

				if(fview->now_item < fview->first_item)
				{
					fview->first_item = fview->now_item;
					rtgui_fileview_ondraw(fview);
				}
				else
				{
					rtgui_fileview_update_current(fview);
				}
				if(fview->sbar && !RTGUI_WIDGET_IS_HIDE(fview))
				{
					if(!RTGUI_WIDGET_IS_HIDE(fview->sbar))
						rtgui_scrollbar_set_value(fview->sbar,fview->first_item);
				}
				return RT_TRUE;

			case RTGUIK_RIGHT:
				if(fview->item_count==0)return RT_FALSE;
				fview->old_item = fview->now_item;
				fview->now_item += fview->item_per_page;

				if(fview->now_item > fview->item_count-1)
					fview->now_item = fview->item_count-1;

				if(fview->now_item >= fview->first_item+fview->item_per_page)
				{
					fview->first_item += fview->item_per_page;
					if(fview->first_item >fview->item_count-fview->item_per_page)
					{
						fview->first_item = fview->item_count-fview->item_per_page;
						fview->now_item = fview->first_item;
					}
					rtgui_fileview_ondraw(fview);
				}
				else
				{
					rtgui_fileview_update_current(fview);
				}
				if(fview->sbar && !RTGUI_WIDGET_IS_HIDE(fview))
				{
					if(!RTGUI_WIDGET_IS_HIDE(fview->sbar))
						rtgui_scrollbar_set_value(fview->sbar,fview->first_item);
				}
				return RT_TRUE;

			case RTGUIK_RETURN:
				rtgui_fileview_on_enter(fview, event);
				return RT_TRUE;
			case RTGUIK_BACKSPACE:
				rtgui_fileview_goto_topfolder(fview);
				return RT_TRUE;
			default:
				break;
			}
		}
	}
	return RT_FALSE;
	default:
		return rtgui_container_event_handler(widget, event);
	}
}

/* clear all file items */
static void rtgui_fileview_clear(rtgui_fileview_t* fview)
{
	rt_uint32_t index;
	rtgui_fileview_item_t* item;

	if(fview->items == RT_NULL) return;

	for(index = 0; index < fview->item_count; index ++)
	{
		item = &(fview->items[index]);
		/* release item name */
		if(item->name != RT_NULL)
		{
			rt_free(item->name);
			item->name = RT_NULL;
		}
	}
	/* release items */
	rt_free(fview->items);
	fview->items = RT_NULL;
	fview->item_count = 0;
	fview->now_item = 0;
}

void rtgui_fileview_set_directory(rtgui_fileview_t* fview, const char* directory)
{
	char fullpath[256];
	rtgui_fileview_item_t *item;

	fview->first_item = 0;
	fview->item_count = 0;

	/* clear file information */
	rtgui_fileview_clear(fview);

	if(directory != RT_NULL)
	{
		DIR* dir;
		struct stat s;
		rt_uint32_t i;
		struct dirent* dirent;

		fview->item_count = 0;
		/* open directory */
		dir = opendir(directory);
		if(dir == RT_NULL) return;

		/* set current directory */
		if(fview->current_dir != RT_NULL)
		{
			rt_free(fview->current_dir);
			fview->current_dir = RT_NULL;
		}
		fview->current_dir = rt_strdup(directory);

		if(fview->dlg != RT_NULL)
		{
			if(fview->dlg->path != RT_NULL)
				rt_free(fview->dlg->path);
			fview->dlg->path = rt_strdup(fview->current_dir);

			if(fview->dlg->tbox_path != RT_NULL)
			{
				rtgui_textbox_set_value(fview->dlg->tbox_path,fview->dlg->path);
				rtgui_textbox_ondraw(fview->dlg->tbox_path);
			}
		}

		do
		{
			dirent = readdir(dir);
			if(dirent == RT_NULL) break;
			fview->item_count ++;
		}
		while(dirent != RT_NULL);
		closedir(dir);

		if((fview->item_count > fview->item_per_page) && fview->sbar!=RT_NULL)
		{
			RTGUI_WIDGET_SHOW(fview->sbar);
			rtgui_scrollbar_set_line_step(fview->sbar,1);
			rtgui_scrollbar_set_page_step(fview->sbar, fview->item_per_page);
			rtgui_scrollbar_set_range(fview->sbar, fview->item_count);
		}
		else
		{
			RTGUI_WIDGET_HIDE(fview->sbar);
		}
		rtgui_widget_update_clip(fview);

		/* apply to memory for store all items. */
		fview->items = (rtgui_fileview_item_t*) rt_malloc(sizeof(rtgui_fileview_item_t) * fview->item_count);

		if(fview->items == RT_NULL) goto __return; /* under the folder has not sub files. */

		/* reopen directory */
		dir = opendir(directory);
		if(dir == RT_NULL)  goto __return;

		for(i=0; i < fview->item_count; i ++)
		{
			dirent = readdir(dir);
			if(dirent == RT_NULL) break;

			item = &(fview->items[i]);
			item->name = rt_strdup(dirent->d_name);

			rt_memset(&s, 0, sizeof(struct stat));

			/* get fullpath of file */
			dfs_get_fullpath(fullpath, directory, dirent->d_name);

			stat(fullpath, &s);
			if(s.st_mode & S_IFDIR)
			{
				item->type = RTGUI_FITEM_DIR;
				item->size = 0;
			}
			else
			{
				item->type = RTGUI_FITEM_FILE;
				item->size = s.st_size;
			}
		}

		closedir(dir);
	}

	fview->now_item = 0;

__return:
	/* update view */
	rtgui_widget_update(fview);
}

void rtgui_fileview_get_fullpath(rtgui_fileview_t* view, char* path, rt_size_t len)
{
	RT_ASSERT(view != RT_NULL);

	if(view->current_dir[strlen(view->current_dir) - 1] != PATH_SEPARATOR)
		rt_snprintf(path, len, "%s%c%s",view->current_dir, PATH_SEPARATOR,
		            view->items[view->now_item].name);
	else
		rt_snprintf(path, len, "%s%s",view->current_dir,
		            view->items[view->now_item].name);
}

static rt_bool_t rtgui_fileview_onunfocus(pvoid wdt, rtgui_event_t* event)
{
	rtgui_fileview_t *fview = RTGUI_FILEVIEW(wdt);

	if(fview == RT_NULL) return RT_FALSE;

	if(!RTGUI_WIDGET_IS_FOCUSED(fview))
	{
		/* clear focus rect */
		rtgui_fileview_update_current(fview);
	}

	return RT_TRUE;
}

static rt_bool_t rtgui_fileview_sbar_handle(pvoid wdt, rtgui_event_t* event)
{
	rtgui_fileview_t *fview = RTGUI_FILEVIEW(wdt);

	if (fview->first_item == fview->sbar->value) return RT_FALSE;
	
	fview->first_item = fview->sbar->value;
	rtgui_fileview_ondraw(fview);

	return RT_TRUE;
}

void dfs_get_fullpath(char *fullpath, const char *path, const char *file)
{
	if(path[strlen(path) - 1] != '/')
		rt_sprintf(fullpath, "%s%c%s", path, '/', file);
	else
		rt_sprintf(fullpath, "%s%s", path, file);
}

#endif
