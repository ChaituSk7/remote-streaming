#ifndef KEYBOARDHANDLER_H
#define KEYBOARDHANDLER_H
#include "header.h"

typedef struct _Customdata {
	std::string path;
	GstElement *pipeline;
	GstElement *volume;
	GMainLoop  *loop;
}CustomData;

extern gboolean handle_keyboard (GIOChannel *, GIOCondition, CustomData *);

extern gboolean msg_handle(GstBus *, GstMessage *, CustomData *);

#endif
