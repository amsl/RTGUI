#ifndef __RTGUI_FILEVIEW_H__
#define __RTGUI_FILEVIEW_H__

#include <rtgui/widgets/container.h>
#include <rtgui/widgets/scrollbar.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RTGUI_FITEM_FILE      0x0
#define RTGUI_FITEM_DIR       0x1

typedef struct rtgui_fileview rtgui_fileview_t;

DECLARE_CLASS_TYPE(fileview);

/** Gets the type of a fileview */
#define RTGUI_FILEVIEW_TYPE       (RTGUI_TYPE(fileview))
/** Casts the object to a fileview */
#define RTGUI_FILEVIEW(obj)       (RTGUI_OBJECT_CAST((obj), RTGUI_FILEVIEW_TYPE, rtgui_fileview_t))
/** Checks if the object is a fileview */
#define RTGUI_IS_FILEVIEW(obj)    (RTGUI_OBJECT_CHECK_TYPE((obj), RTGUI_FILEVIEW_TYPE))

typedef struct dialog_st rtgui_dialog_t;
/* dialog access struct */
struct dialog_st
{
	rtgui_fileview_t* fview;

	char* path;		 /* search path	*/
	rtgui_textbox_t* tbox_path;	

	char* filename;	 /* dir/file name */
	rtgui_textbox_t* tbox_filename;

	char* pattern;   /* search type */
	rtgui_textbox_t*  tbox_pattern;
};

typedef struct rtgui_fileview_item
{
	char* name;
	rt_uint32_t type;
	rt_uint32_t size;
}rtgui_fileview_item_t;

struct rtgui_fileview
{
	rtgui_container_t parent;

	/* widget private data */
    char* current_dir;	   
    char* pattern;         

    rt_int16_t item_per_page; 
	rt_int16_t item_count;	   

	rt_int16_t first_item;/* first item */
	rt_int16_t now_item;  /* now item */
	rt_int16_t old_item;  /* old item */

	rt_bool_t (*on_item)(pvoid wdt, rtgui_event_t* event);
	rtgui_scrollbar_t *sbar;      
	
	rtgui_fileview_item_t *items; /* items array */
	
	/* dialog access entry */
	rtgui_dialog_t *dlg;
};

rtgui_fileview_t* rtgui_fileview_create(pvoid parent, const char* directory, const char* pattern,  int left, int top, int w, int h);
void rtgui_fileview_destroy(rtgui_fileview_t* view);

rt_bool_t rtgui_fileview_event_handler(pvoid wdt, rtgui_event_t* event);
void rtgui_fileview_set_directory(rtgui_fileview_t* view, const char* directory);
void rtgui_fileview_on_enter(rtgui_fileview_t* fview, rtgui_event_t* event);
void rtgui_fileview_get_fullpath(rtgui_fileview_t* view, char* path, rt_size_t len);
void rtgui_fileview_goto_topfolder(rtgui_fileview_t* fview);
void dfs_get_fullpath(char *fullpath, const char *path, const char *file);

#ifdef __cplusplus
}
#endif

#endif
