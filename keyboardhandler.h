#ifndef KEYBOARDHANDLER_H
#define KEYBOARDHANDLER_H

#include "header.h"
#include <iostream>

typedef struct _Customdata {
	std::string path;
	GstElement *pipeline;
	GMainLoop  *loop;
	
}CustomData;

void handle_menu(CustomData *data, gchar input)
{
	  	
	if(input == 'p'){		
	  	
	  	/* Change State To PLAYING */
	  	
	  	gst_element_set_state(data->pipeline, GST_STATE_PLAYING);
	  		
	}else if(input == 's'){	
	  	
	  	/* Change State to PAUSED */
	  	
	  	gst_element_set_state(data->pipeline, GST_STATE_PAUSED);
		g_printerr("Do not keep the pipeline for pause state more than 5 seconds\n");
	  		
	}else if(input == 't'){	
	
	  	/* Track the current position */
	  	gint64 cur_pos;
		gst_element_query_position(data->pipeline, GST_FORMAT_TIME, &cur_pos);
		g_print("\n Current Position : %ld Min %ld Second. \n",((cur_pos/1000000000)/60), ((cur_pos/1000000000)%60));
		  	
	}else if(input == 'd'){		 
	  	
	  	/* Print the Duration of the Stream */
	  	
	 	gint64 duration;
	  	gst_element_query_duration(data->pipeline, GST_FORMAT_TIME, &duration);
	  	g_print("\n Duration : %ld : %ld Min\n",((duration/1000000000)/60), ((duration/1000000000)%60));
	  				
	}else if(input == 'c'){		
	  	
	  	/* Change the current position of the Stream */
	  	
	  	gint64 cur_pos;
	  	gst_element_query_position(data->pipeline, GST_FORMAT_TIME, &cur_pos);
	  	g_print("\n Current Position : %ld Second\n ",(cur_pos)/1000000000);
	  	gst_element_seek_simple(data->pipeline, GST_FORMAT_TIME, GstSeekFlags(GST_SEEK_FLAG_FLUSH), (cur_pos + 10 * GST_SECOND));
	}else if(input == 'k'){	
	
	  	g_print("\nInteractive mode - keyboard controls:\n\n"
            "     p         :  play\n"
            "     s         :  pause\n"
            "     n         :  play next\n"
            "     k         :  show keyboard shorcuts\n"
            "     d         :  Duration of media\n"
			"     c         :  Seek 10 sec forward\n"
            "     t         :  Current position of media\n"
			"     m         :  Print metadata\n" 
            "     q         :  quit\n");
	  		
	}else if(input == 'n'){
	  	/* Flush the existing Stream Data and Play the next Stream */
	  	gst_element_set_state(data->pipeline, GST_STATE_PAUSED);
	  	gst_event_new_seek (1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, 
		  					0, GST_SEEK_TYPE_SET, 0);
	  	gst_element_set_state(data->pipeline, GST_STATE_NULL);
	  	g_main_loop_quit(data->loop);
	  	
	}else if(input == 'q'){
	  	/* Exit the Program*/
	  	gst_element_set_state(data->pipeline, GST_STATE_NULL);
	  	g_main_loop_quit(data->loop);
		exit(0);
	}else if(input == 'm'){
	  	metadata_fun(data->path);
	}	
	else if (input == 'v') { 
	  	
	}	
}


static gboolean handle_keyboard (GIOChannel * source, GIOCondition cond, CustomData *data)
{
	gchar *str = NULL;

	if (g_io_channel_read_line (source, &str, NULL, NULL, NULL) != G_IO_STATUS_NORMAL) {
	  return TRUE;
	}
	
	handle_menu(data, g_ascii_tolower (str[0]));
	
	return TRUE;
}


#endif
