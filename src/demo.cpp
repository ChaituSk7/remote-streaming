#include "header.h"

static void pad_added_handler(GstElement *src, GstPad *pad, gpointer *data) {
	GstElement *sink = (GstElement*) data;
	GstCaps *new_pad_caps = NULL;
	GstStructure *new_pad_struct = NULL;
	const gchar *new_pad_type = NULL;
	GstPadLinkReturn ret;
	GstPad *sinkpad;
	g_print("Received new pad '%s' from '%s':\n", GST_PAD_NAME(pad),
			GST_ELEMENT_NAME(src));
	/* Check the new pad's type */
	new_pad_caps = gst_pad_get_current_caps(pad);
	new_pad_struct = gst_caps_get_structure(new_pad_caps, 0);
	new_pad_type = gst_structure_get_name(new_pad_struct);
	if (g_str_has_prefix(new_pad_type, "audio")) {
		//g_print ("It has type '%s'\n", new_pad_type);
		sinkpad = gst_element_get_static_pad(sink, "sink");
		ret = gst_pad_link(pad, sinkpad);
		if (GST_PAD_LINK_FAILED(ret)) {
			//g_print ("Type is '%s' but link failed.\n", new_pad_type);
		} else
			g_print("Link succeeded (type '%s').\n", new_pad_type);
	}
	if (g_str_has_prefix(new_pad_type, "video")) {
		g_print("It has type '%s'\n", new_pad_type);
		sinkpad = gst_element_get_static_pad(sink, "sink");
		ret = gst_pad_link(pad, sinkpad);
		if (GST_PAD_LINK_FAILED(ret)) {
			g_print("Type is '%s' but link failed.\n", new_pad_type);
		} else {
			g_print("Link succeeded (type '%s').\n", new_pad_type);
		}
	}
	gst_object_unref(sinkpad);
}

/* Forward definition of the message processing function */
static void handle_message(ThumbnailData *data, GstMessage *msg);

static int set_pipelinemp4(string input_path, string output_path) {
	ThumbnailData data;
	GstBus *bus;
	GstMessage *msg;
	GstStateChangeReturn ret;

	data.playing = FALSE;
	data.terminate = FALSE;
	data.seek_enabled = FALSE;
	data.seek_done = FALSE;
	data.duration = GST_CLOCK_TIME_NONE;

	/* Initialize GStreamer */
	gst_init(NULL, NULL);

	/* Create the elements */
	data.pipeline = gst_pipeline_new("thumbpipe");
	data.src = gst_element_factory_make("filesrc", "src");
	data.demuxer = gst_element_factory_make("qtdemux", NULL);
	data.video_decoder = gst_element_factory_make("avdec_h264", NULL);
	data.video_queue = gst_element_factory_make("queue", NULL);
	data.convert = gst_element_factory_make("videoconvert", "convert");
	data.rate = gst_element_factory_make("videorate", "rate");
	data.scale = gst_element_factory_make("videoscale", "scale");
	data.capsfilter = gst_element_factory_make("capsfilter", "capsfilter");
	data.encode = gst_element_factory_make("jpegenc", "encode");
	data.sink = gst_element_factory_make("filesink", "sink");
	/* Check if all elements were created successfully */
	if (!data.pipeline || !data.src || !data.demuxer || !data.video_decoder
			|| !data.video_queue || !data.convert || !data.rate || !data.scale
			|| !data.capsfilter || !data.encode || !data.sink) {
		g_printerr("Failed to create elements\n");
		return 1;
	}

	gst_bin_add_many(GST_BIN(data.pipeline), data.src, data.demuxer,
			data.video_decoder, data.video_queue, data.convert, data.rate,
			data.scale, data.capsfilter, data.encode, data.sink, NULL);

	g_object_set(G_OBJECT(data.src), "location", input_path.c_str(), NULL);
	g_object_set(G_OBJECT(data.sink), "location", output_path.c_str(), NULL);
	// g_object_set(G_OBJECT(data.encode), "snapshot", TRUE, NULL);
	GstCaps *caps = gst_caps_new_simple("video/x-raw", "framerate",
			GST_TYPE_FRACTION, 1, 10, "width", G_TYPE_INT, 900, "height",
			G_TYPE_INT, 500, NULL);

	g_object_set(G_OBJECT(data.capsfilter), "caps", caps, NULL);
	g_signal_connect(data.demuxer, "pad-added", G_CALLBACK(pad_added_handler),
			data.video_queue);
	if (!gst_element_link_many(data.src, data.demuxer, NULL)
			|| !gst_element_link_many(data.video_queue, data.video_decoder,
					data.convert, data.rate, data.scale, data.capsfilter,
					data.encode, data.sink, NULL)) {
		g_printerr("Failed to link elements\n");
		return 1;
	}

	/* Start playing */
	ret = gst_element_set_state(data.pipeline, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE) {
		g_printerr("Unable to set the pipeline to the playing state.\n");
		gst_object_unref(data.pipeline);
		return -1;
	}

	/* Listen to the bus */
	bus = gst_element_get_bus(data.pipeline);
	do {
		msg = gst_bus_timed_pop_filtered(bus, 100 * GST_MSECOND,
				(GstMessageType)(
						GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR
								| GST_MESSAGE_EOS | GST_MESSAGE_DURATION));

		/* Parse message */
		if (msg != NULL) {
			handle_message(&data, msg);
		} else {
			/* We got no message, this means the timeout expired */
			if (data.playing) {
				gint64 current = -1;

				/* Query the current position of the stream */
				if (!gst_element_query_position(data.pipeline, GST_FORMAT_TIME,
						&current)) {
					g_printerr("Could not query current position.\n");
				}

				/* If we didn't know it yet, query the stream duration */
				if (!GST_CLOCK_TIME_IS_VALID(data.duration)) {
					if (!gst_element_query_duration(data.pipeline,
							GST_FORMAT_TIME, &data.duration)) {
						g_printerr("Could not query current duration.\n");
					}
				}

				/* If seeking is enabled, we have not done it yet, and the time is right, seek */
				if (data.seek_enabled && !data.seek_done && current > 0) {
					g_print("\nperforming seek...\n");
					gst_element_seek_simple(data.pipeline, GST_FORMAT_TIME,
							(GstSeekFlags)(
									GST_SEEK_FLAG_FLUSH
											| GST_SEEK_FLAG_ACCURATE),
							30 * GST_SECOND);
					data.seek_done = TRUE;

				}
			}
		}
	} while (!data.terminate);

	/* Free resources */
	gst_object_unref(bus);
	gst_element_set_state(data.pipeline, GST_STATE_NULL);
	gst_object_unref(data.pipeline);

	return 0;
}
static int set_pipelineavi(string input_path, string output_path) {
	ThumbnailData data;
	GstBus *bus;
	GstMessage *msg;
	GstStateChangeReturn ret;

	data.playing = FALSE;
	data.terminate = FALSE;
	data.seek_enabled = FALSE;
	data.seek_done = FALSE;
	data.duration = GST_CLOCK_TIME_NONE;

	/* Initialize GStreamer */
	gst_init(NULL, NULL);

	/* Create the elements */
	data.pipeline = gst_pipeline_new("thumbpipe");
	data.src = gst_element_factory_make("filesrc", "src");
	data.demuxer = gst_element_factory_make("avidemux", NULL);
	data.video_decoder = gst_element_factory_make("avdec_mpeg4", NULL);
	data.video_queue = gst_element_factory_make("queue", NULL);
	data.convert = gst_element_factory_make("videoconvert", "convert");
	data.rate = gst_element_factory_make("videorate", "rate");
	data.scale = gst_element_factory_make("videoscale", "scale");
	data.capsfilter = gst_element_factory_make("capsfilter", "capsfilter");
	data.encode = gst_element_factory_make("jpegenc", "encode");
	data.sink = gst_element_factory_make("filesink", "sink");
	/* Check if all elements were created successfully */
	if (!data.pipeline || !data.src || !data.demuxer || !data.video_decoder
			|| !data.video_queue || !data.convert || !data.rate || !data.scale
			|| !data.capsfilter || !data.encode || !data.sink) {
		g_printerr("Failed to create elements\n");
		return 1;
	}

	gst_bin_add_many(GST_BIN(data.pipeline), data.src, data.demuxer,
			data.video_decoder, data.video_queue, data.convert, data.rate,
			data.scale, data.capsfilter, data.encode, data.sink, NULL);

	g_object_set(G_OBJECT(data.src), "location", input_path.c_str(), NULL);
	g_object_set(G_OBJECT(data.sink), "location", output_path.c_str(), NULL);
	// g_object_set(G_OBJECT(data.encode), "snapshot", TRUE, NULL);
	GstCaps *caps = gst_caps_new_simple("video/x-raw", "framerate",
			GST_TYPE_FRACTION, 1, 10, "width", G_TYPE_INT, 900, "height",
			G_TYPE_INT, 500, NULL);

	g_object_set(G_OBJECT(data.capsfilter), "caps", caps, NULL);
	g_signal_connect(data.demuxer, "pad-added", G_CALLBACK(pad_added_handler),
			data.video_queue);
	if (!gst_element_link_many(data.src, data.demuxer, NULL)
			|| !gst_element_link_many(data.video_queue, data.video_decoder,
					data.convert, data.rate, data.scale, data.capsfilter,
					data.encode, data.sink, NULL)) {
		g_printerr("Failed to link elements\n");
		return 1;
	}

	/* Start playing */
	ret = gst_element_set_state(data.pipeline, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE) {
		g_printerr("Unable to set the pipeline to the playing state.\n");
		gst_object_unref(data.pipeline);
		return -1;
	}

	/* Listen to the bus */
	bus = gst_element_get_bus(data.pipeline);
	do {
		msg = gst_bus_timed_pop_filtered(bus, 100 * GST_MSECOND,
				(GstMessageType)(
						GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR
								| GST_MESSAGE_EOS | GST_MESSAGE_DURATION));

		/* Parse message */
		if (msg != NULL) {
			handle_message(&data, msg);
		} else {
			/* We got no message, this means the timeout expired */
			if (data.playing) {
				gint64 current = -1;

				/* Query the current position of the stream */
				if (!gst_element_query_position(data.pipeline, GST_FORMAT_TIME,
						&current)) {
					g_printerr("Could not query current position.\n");
				}

				/* If we didn't know it yet, query the stream duration */
				if (!GST_CLOCK_TIME_IS_VALID(data.duration)) {
					if (!gst_element_query_duration(data.pipeline,
							GST_FORMAT_TIME, &data.duration)) {
						g_printerr("Could not query current duration.\n");
					}
				}

				/* If seeking is enabled, we have not done it yet, and the time is right, seek */
				if (data.seek_enabled && !data.seek_done && current > 0) {
					g_print("\nperforming seek...\n");
					gst_element_seek_simple(data.pipeline, GST_FORMAT_TIME,
							(GstSeekFlags)(
									GST_SEEK_FLAG_FLUSH
											| GST_SEEK_FLAG_ACCURATE),
							30 * GST_SECOND);
					data.seek_done = TRUE;
				}
			}
		}
	} while (!data.terminate);

	/* Free resources */
	gst_object_unref(bus);
	gst_element_set_state(data.pipeline, GST_STATE_NULL);
	gst_object_unref(data.pipeline);

	return 0;
}
static int set_pipelinewebm(string input_path, string output_path) {
	ThumbnailData data;
	GstBus *bus;
	GstMessage *msg;
	GstStateChangeReturn ret;

	data.playing = FALSE;
	data.terminate = FALSE;
	data.seek_enabled = FALSE;
	data.seek_done = FALSE;
	data.duration = GST_CLOCK_TIME_NONE;

	/* Initialize GStreamer */
	gst_init(NULL, NULL);

	/* Create the elements */
	data.pipeline = gst_pipeline_new("thumbpipe");
	data.src = gst_element_factory_make("filesrc", "src");
	data.demuxer = gst_element_factory_make("matroskademux", NULL);
	data.video_decoder = gst_element_factory_make("vp8dec", NULL);
	data.video_queue = gst_element_factory_make("queue", NULL);
	data.convert = gst_element_factory_make("videoconvert", "convert");
	data.rate = gst_element_factory_make("videorate", "rate");
	data.scale = gst_element_factory_make("videoscale", "scale");
	data.capsfilter = gst_element_factory_make("capsfilter", "capsfilter");
	data.encode = gst_element_factory_make("jpegenc", "encode");
	data.sink = gst_element_factory_make("filesink", "sink");
	/* Check if all elements were created successfully */
	if (!data.pipeline || !data.src || !data.demuxer || !data.video_decoder
			|| !data.video_queue || !data.convert || !data.rate || !data.scale
			|| !data.capsfilter || !data.encode || !data.sink) {
		g_printerr("Failed to create elements\n");
		return 1;
	}

	gst_bin_add_many(GST_BIN(data.pipeline), data.src, data.demuxer,
			data.video_decoder, data.video_queue, data.convert, data.rate,
			data.scale, data.capsfilter, data.encode, data.sink, NULL);

	g_object_set(G_OBJECT(data.src), "location", input_path.c_str(), NULL);
	g_object_set(G_OBJECT(data.sink), "location", output_path.c_str(), NULL);
	// g_object_set(G_OBJECT(data.encode), "snapshot", TRUE, NULL);
	GstCaps *caps = gst_caps_new_simple("video/x-raw", "framerate",
			GST_TYPE_FRACTION, 1, 10, "width", G_TYPE_INT, 900, "height",
			G_TYPE_INT, 500, NULL);

	g_object_set(G_OBJECT(data.capsfilter), "caps", caps, NULL);
	g_signal_connect(data.demuxer, "pad-added", G_CALLBACK(pad_added_handler),
			data.video_queue);
	if (!gst_element_link_many(data.src, data.demuxer, NULL)
			|| !gst_element_link_many(data.video_queue, data.video_decoder,
					data.convert, data.rate, data.scale, data.capsfilter,
					data.encode, data.sink, NULL)) {
		g_printerr("Failed to link elements\n");
		return 1;
	}

	/* Start playing */
	ret = gst_element_set_state(data.pipeline, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE) {
		g_printerr("Unable to set the pipeline to the playing state.\n");
		gst_object_unref(data.pipeline);
		return -1;
	}

	/* Listen to the bus */
	bus = gst_element_get_bus(data.pipeline);
	do {
		msg = gst_bus_timed_pop_filtered(bus, 100 * GST_MSECOND,
				(GstMessageType)(
						GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR
								| GST_MESSAGE_EOS | GST_MESSAGE_DURATION));

		/* Parse message */
		if (msg != NULL) {
			handle_message(&data, msg);
		} else {
			/* We got no message, this means the timeout expired */
			if (data.playing) {
				gint64 current = -1;

				/* Query the current position of the stream */
				if (!gst_element_query_position(data.pipeline, GST_FORMAT_TIME,
						&current)) {
					g_printerr("Could not query current position.\n");
				}

				/* If we didn't know it yet, query the stream duration */
				if (!GST_CLOCK_TIME_IS_VALID(data.duration)) {
					if (!gst_element_query_duration(data.pipeline,
							GST_FORMAT_TIME, &data.duration)) {
						g_printerr("Could not query current duration.\n");
					}
				}

				/* If seeking is enabled, we have not done it yet, and the time is right, seek */
				if (data.seek_enabled && !data.seek_done && current > 0) {
					g_print("\nperforming seek...\n");
					gst_element_seek_simple(data.pipeline, GST_FORMAT_TIME,
							(GstSeekFlags)(
									GST_SEEK_FLAG_FLUSH
											| GST_SEEK_FLAG_ACCURATE),
							30 * GST_SECOND);
					data.seek_done = TRUE;

				}
			}
		}
	} while (!data.terminate);

	/* Free resources */
	gst_object_unref(bus);
	gst_element_set_state(data.pipeline, GST_STATE_NULL);
	gst_object_unref(data.pipeline);

	return 0;
}
/* Function to store image in local system */
int directory_set(string path) {
	string thumbnail_path = "/home/ee212784/Desktop/remote-streaming/";
	gst_init(NULL, NULL);
	GError *error = NULL;

	//Check suffix of given path and call function to create image  
	if (g_str_has_suffix((gchar*) path.c_str(), ".mp4")) {
		string output_path = thumbnail_path + "output.jpg";
		set_pipelinemp4(path, output_path);
	} else if (g_str_has_suffix((gchar*) path.c_str(), ".avi")) {
		string output_path = thumbnail_path + "output.jpg";
		set_pipelineavi(path, output_path);
	} else if (g_str_has_suffix((gchar*) path.c_str(), ".webm")) {
		string output_path = thumbnail_path + "output.jpg";
		set_pipelinewebm(path, output_path);

	} else
		;
	return 0;
}

// Handle messages receive by Bus
static void handle_message(ThumbnailData *data, GstMessage *msg) {
	GError *err;
	gchar *debug_info;

	switch (GST_MESSAGE_TYPE(msg)) {
	case GST_MESSAGE_ERROR:
		gst_message_parse_error(msg, &err, &debug_info);
		g_printerr("Error received from element %s: %s\n",
				GST_OBJECT_NAME(msg->src), err->message);
		g_printerr("Debugging information: %s\n",
				debug_info ? debug_info : "none");
		g_clear_error(&err);
		g_free(debug_info);
		data->terminate = TRUE;
		break;
	case GST_MESSAGE_EOS:
		g_print("\nEnd-Of-Stream reached in demo.\n");
		data->terminate = TRUE;
		break;
	case GST_MESSAGE_DURATION:
		/* The duration has changed, mark the current one as invalid */
		data->duration = GST_CLOCK_TIME_NONE;
		break;
	case GST_MESSAGE_STATE_CHANGED: {
		GstState old_state, new_state, pending_state;
		gst_message_parse_state_changed(msg, &old_state, &new_state,
				&pending_state);
		if (GST_MESSAGE_SRC(msg) == GST_OBJECT(data->pipeline)) {
			g_print("Pipeline state changed from %s to %s:\n",
					gst_element_state_get_name(old_state),
					gst_element_state_get_name(new_state));

			/* Remember whether we are in the PLAYING state or not */
			data->playing = (new_state == GST_STATE_PLAYING);

			if (data->playing) {
				/* We just moved to PLAYING. Check if seeking is possible */
				GstQuery *query;
				gint64 start, end;
				query = gst_query_new_seeking(GST_FORMAT_TIME);
				if (gst_element_query(data->pipeline, query)) {
					gst_query_parse_seeking(query, NULL, &data->seek_enabled,
							&start, &end);
					if (data->seek_enabled) {
					g_print(
							"Seeking is ENABLED from %" GST_TIME_FORMAT " to %" GST_TIME_FORMAT "\n",
							GST_TIME_ARGS(start), GST_TIME_ARGS(end));
				} else {
					g_print("Seeking is DISABLED for this stream.\n");
				}
			} else {
				g_printerr("Seeking query failed.");
			}
			gst_query_unref(query);
		}
	}
}
	break;
default:
	/* We should not reach here */
	g_printerr("Unexpected message received.\n");
	break;
	}
	gst_message_unref(msg);
}
