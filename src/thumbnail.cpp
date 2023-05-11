#include <gst/gst.h>
#include<iostream>
#include <unistd.h>

/* Display thumbnail on host side */
int display_img() {
	GstElement *pipeline, *src, *jpegdec, *videoconvert, *sink;
	GstBus *bus;
	GstMessage *msg;
	GstStateChangeReturn ret;

	/* Initialize GStreamer */
	gst_init(NULL, NULL);

	/* Create elements */
	pipeline = gst_pipeline_new("jpeg-display");
	src = gst_element_factory_make("filesrc", "src");
	//g_object_set(src, "location", "image.jpg", NULL);

	jpegdec = gst_element_factory_make("jpegdec", "jpegdec");
	videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
	sink = gst_element_factory_make("autovideosink", "sink");
	//g_object_set(G_OBJECT(src), "location", "/home/ee213015/d/def1.mp4_output.jpg", NULL);
	g_object_set(G_OBJECT(src), "location",
			"/home/ee212784/Desktop/remote-streaming/output.jpg", NULL);

	/* Check if all elements are created */
	if (!pipeline || !src || !jpegdec || !videoconvert || !sink) {
		g_printerr("Not all elements could be created.\n");
		return -1;
	}

	/* Build the pipeline */
	gst_bin_add_many(GST_BIN(pipeline), src, jpegdec, videoconvert, sink, NULL);
	gst_element_link_many(src, jpegdec, videoconvert, sink, NULL);

	/* Set the pipeline to paused state */
	ret = gst_element_set_state(pipeline, GST_STATE_PAUSED);
	if (ret == GST_STATE_CHANGE_FAILURE) {
		g_printerr("Unable to set the pipeline to the playing state.\n");
		gst_object_unref(pipeline);
		return -1;
	}
	g_usleep(3 * G_USEC_PER_SEC);
	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(pipeline);

	return 0;

}
