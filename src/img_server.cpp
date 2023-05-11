#include "header.h"

// Handle messages received by Bus 
static void message_cb(GstElement *bus, GstMessage *msg, GMainLoop *loop) {
	if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS) {
		g_print("\nStream End\n");
		g_main_loop_quit(loop);

	} else if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
		GError *error = NULL;
		gchar *debug = NULL;
		gst_message_parse_error(msg, &error, &debug);
		g_print("\n Error received from %s : %s", GST_OBJECT_NAME(msg->src),
				error->message);
		g_print("\n Debug Info : %s\n", (debug) ? debug : "None");
		g_error_free(error);
		g_free(debug);
		g_print("\n Error in playing stream\n");
		g_main_loop_quit(loop);
	} else {

	}
}

// Send thumbnail image to client
int image_display() {
	ImageData data;
	GstBus *bus;
	GstStateChangeReturn ret;
	GstMessage *msg;
	// Initialize gstreamer
	gst_init(NULL, NULL);

	// Initilize elements
	data.pipeline = gst_pipeline_new("host-pipeline");
	data.source = gst_element_factory_make("filesrc", NULL);
	data.jpegdec = gst_element_factory_make("jpegdec", NULL);
	data.video_convert = gst_element_factory_make("videoconvert", NULL);
	data.video_scale = gst_element_factory_make("videoscale", NULL);
	data.img_freeze = gst_element_factory_make("imagefreeze", NULL);
	data.video_encoder = gst_element_factory_make("jpegenc", NULL);
	data.capsfilter = gst_element_factory_make("capsfilter", "capsfilter");
	data.rtp_payload = gst_element_factory_make("rtpjpegpay", NULL);
	data.udp_sink = gst_element_factory_make("udpsink", NULL);

	// Check the video elements are created
	if (!data.pipeline || !data.source || !data.jpegdec || !data.video_convert
			|| !data.video_scale || !data.capsfilter || !data.img_freeze
			|| !data.video_encoder || !data.rtp_payload || !data.udp_sink) {
		g_printerr("Not all the elements could be created.\n");
		exit (EXIT_FAILURE);
	}

	/* Check the audio elements are created */

	// Add elements to bin
	gst_bin_add_many(GST_BIN(data.pipeline), data.source, data.jpegdec,
			data.video_scale, data.video_convert, data.img_freeze,
			data.capsfilter, data.video_encoder, data.rtp_payload,
			data.udp_sink, NULL);

	// Set the element properties
	g_object_set(G_OBJECT(data.source), "location",
			"/home/ee212784/Desktop/remote-streaming/output.jpg", NULL);

	GstCaps *caps = gst_caps_new_simple("video/x-raw", "framerate",
			GST_TYPE_FRACTION, 1, 10, "width", G_TYPE_INT, 579, "height",
			G_TYPE_INT, 386, NULL);

	g_object_set(G_OBJECT(data.capsfilter), "caps", caps, NULL);

	//Set IP address and port number
	g_object_set(G_OBJECT(data.udp_sink), "host", "10.1.139.244", NULL);
	g_object_set(G_OBJECT(data.udp_sink), "port", 5003, NULL);
	g_object_set(G_OBJECT(data.udp_sink), "clients",
			"10.1.139.244:5003,192.168.2.16:5003", NULL);
	g_object_set(G_OBJECT(data.img_freeze), "num-buffers", 2, NULL);

	// Link Elements
	if (gst_element_link_many(data.source, data.jpegdec, data.video_convert,
			data.img_freeze, data.video_scale, data.capsfilter,
			data.video_encoder, data.rtp_payload, data.udp_sink, NULL)
			!= TRUE) {
		g_printerr("Sorce to qtdemux not linked.\n");
		exit (EXIT_FAILURE);
	}

	/* Set the pipeline for playing state */
	ret = gst_element_set_state(data.pipeline, GST_STATE_PLAYING);

	if (ret == GST_STATE_CHANGE_FAILURE) {
		g_printerr("Could not set the pipeline for playing.\n");
		exit (EXIT_FAILURE);
	} else {
		g_print("\nPipeline is Playing\n");
	}
	/* Start the Main Loop event */
	GMainLoop *loop = g_main_loop_new(NULL, FALSE);

	/* Create bus to keep watch for message */
	bus = gst_pipeline_get_bus(GST_PIPELINE(data.pipeline));
	gst_bus_add_signal_watch(bus);

	/* Connect signal messages that came from bus */
	g_signal_connect(bus, "message", G_CALLBACK(message_cb), loop);

	g_main_loop_run(loop);
	gst_element_set_state(data.pipeline, GST_STATE_NULL);
	gst_object_unref(data.pipeline);
	gst_object_unref(bus);
	return 0;
}
