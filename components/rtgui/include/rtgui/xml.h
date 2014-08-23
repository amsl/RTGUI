#ifndef __RTGUI_XML_H__
#define __RTGUI_XML_H__

#include <rtgui/rtgui.h>

/* Types of events: start element, end element, text, attr name, attr
   val and start/end document. Other events can be ignored! */
enum {
	EVENT_START = 0, /* Start tag */
	EVENT_END,       /* End tag */
	EVENT_TEXT,      /* Text */
	EVENT_NAME,      /* Attribute name */
	EVENT_VAL,       /* Attribute value */
	EVENT_END_DOC,   /* End of document */
	EVENT_COPY,      /* Internal only; copies to internal buffer */
	EVENT_NONE       /* Internal only; should never see this event */
};

/* xml structure typedef */
typedef struct xml xml_t;
typedef int (*XML_handle)(rt_uint8_t event, const char* text, rt_size_t len, void* user);

/* create a xml parser context */
xml_t* xml_create(rt_size_t buffer_size, XML_handle handler, void* user);
/* destroy a xml parser context */
void xml_destroy(xml_t* xml);

/* parse xml buffer */
int xml_parse(xml_t* xml, const char* buf, rt_size_t len);

/* event string */
const char* xml_event_str(rt_uint8_t event);

#endif
