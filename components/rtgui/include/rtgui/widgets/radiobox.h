#ifndef __RTGUI_RADIOBOX_H__
#define __RTGUI_RADIOBOX_H__

#include <rtgui/widgets/widget.h>
#include <rtgui/widgets/container.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RADIO_BOX_W		12
#define RADIO_BOX_H		12

DECLARE_CLASS_TYPE(radiobox);
/** Gets the type of a rbox */
#define RTGUI_RADIOBOX_TYPE       (RTGUI_TYPE(radiobox))//(rtgui_radiobox_type_get())
/** Casts the object to an rtgui_radiobox_t */
#define RTGUI_RADIOBOX(obj)       (RTGUI_OBJECT_CAST((obj), RTGUI_RADIOBOX_TYPE, rtgui_radiobox_t))
/** Checks if the object is an rtgui_radiobox_t */
#define RTGUI_IS_RADIOBOX(obj)    (RTGUI_OBJECT_CHECK_TYPE((obj), RTGUI_RADIOBOX_TYPE))

typedef struct rtgui_rb_bond rtgui_rb_bond_t;

/* rb bond, rb => radiobox */
struct rtgui_rb_bond
{
	rt_uint32_t item_used;/* record times cited. */
	rt_uint32_t	item_count;
	rt_uint16_t	item_sel;
	rt_uint32_t* bind_var; /* binding user variable */
	rtgui_radiobox_t **rboxs;
	rt_bool_t (*on_item)(pvoid wdt, rtgui_event_t* event);
};

struct rtgui_radiobox
{
	rtgui_widget_t parent;
	char* name;
	rtgui_rb_bond_t *bond;
};

typedef struct rtgui_radiobox rtgui_radiobox_t;

rtgui_radiobox_t* rtgui_radiobox_create(pvoid parent, const char* name, 
			int left, int top, int w, int h, rtgui_rb_bond_t* bond);
void rtgui_radiobox_destroy(rtgui_radiobox_t* rbox);
rtgui_rb_bond_t* rtgui_radiobox_create_group(void);
rtgui_rb_bond_t* rtgui_radiobox_get_bond(rtgui_radiobox_t* rbox);
/* bind a external variable */
void rtgui_rb_bond_bind(rtgui_rb_bond_t *bond, rt_uint32_t *var);
/* terminate binding relation */
void rtgui_rb_bond_unbind(rtgui_rb_bond_t *bond);
/* set selection in group */
void rtgui_rb_bond_set_sel(rtgui_rb_bond_t* bond, int selection);
int rtgui_rb_bond_get_sel(rtgui_rb_bond_t* bond);
void rtgui_rb_bond_set_onitem(rtgui_rb_bond_t* bond, rtgui_event_handler_ptr func);
void rtgui_radiobox_ondraw(rtgui_radiobox_t* rbox);
rt_bool_t rtgui_radiobox_event_handler(pvoid wdt, rtgui_event_t* event);

#ifdef __cplusplus
}
#endif

#endif

