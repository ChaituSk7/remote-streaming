#ifndef MSGHANDLER_H
#define MSGHANDLER_H

#include "header.h"
#include "keyboardhandler.h"


gboolean msg_handle(GstBus *bus, GstMessage *msg, CustomData *data) {
	switch(GST_MESSAGE_TYPE(msg))
	{
		case GST_MESSAGE_UNKNOWN:
					g_print("\n Unknown Message Received.\n");
					break;
					
		case GST_MESSAGE_EOS:
					g_print("\n End of Stream Reached.\n");
					gst_element_set_state(data->pipeline, GST_STATE_NULL);
					g_main_loop_quit(data->loop);
					break;
					
		case GST_MESSAGE_ERROR:	{
						GError *error = NULL;
						gchar *debug = NULL;
						gst_message_parse_error(msg, &error, &debug);
						g_print("\n Error received from %s : %s", GST_OBJECT_NAME(msg->src), error->message);
						g_print("\n Debug Info : %s\n",(debug)? debug : "None");
						g_error_free(error);
						g_free(debug);
						
						gst_element_set_state(data->pipeline, GST_STATE_NULL);
						g_main_loop_quit(data->loop);
					}
					break;
					
		case GST_MESSAGE_WARNING:{
						GError *error = NULL;
						gchar *debug = NULL;
						gst_message_parse_warning(msg, &error, &debug);
						g_print("\n Error received from %s : %s", GST_OBJECT_NAME(msg->src), error->message);
						g_print("\n Debug Info : %s\n",(debug)? debug : "None");
						g_error_free(error);
						g_free(debug);
					}
					break;
					
		case GST_MESSAGE_INFO:	{
						GError *error = NULL;
						gchar *debug = NULL;
						gst_message_parse_info(msg, &error, &debug);
						g_print("\n Error received from %s : %s", GST_OBJECT_NAME(msg->src), error->message);
						g_print("\n Debug Info : %s\n",(debug)? debug : "None");
						g_error_free(error);
						g_free(debug);
					}
					break;
					
		case GST_MESSAGE_STATE_CHANGED:
					if (GST_MESSAGE_SRC(msg) == GST_OBJECT(data->pipeline)) {
						GstState old_state, new_state, pending_state;
						gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
						g_print("Pipeline state changed from '%s' to '%s'\n", gst_element_state_get_name(old_state),
								gst_element_state_get_name(new_state));
					}
					break;
					
		default:
					break;
	}
	
	return true;
}


#endif
