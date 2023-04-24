#include "header.h"

static void callback_message (GstBus *bus, GstMessage *msg, HostMP3Data *data) {

  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR: {
        GError *err;
        gchar *debug;
        gst_message_parse_error (msg, &err, &debug);
        g_print ("Error: %s\n", err->message);
        g_error_free (err);
        g_free (debug);
        g_main_loop_quit (data->loop);
    }
    break;
    case GST_MESSAGE_EOS: {
        g_print("Reached End of Stream.\n");
        g_main_loop_quit (data->loop);
    }
     break;
    case GST_MESSAGE_STATE_CHANGED: {
      if (GST_MESSAGE_SRC(msg) == GST_OBJECT(data->pipeline)) {
        GstState old_state, new_state, pending_state;
        gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
        g_print("Pipeline state changed from '%s' to '%s'\n", gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));
      }
    }
    break;
    default: 
    break;
  }
}

int hostmp3_pipeline (int argc, char *argv[]) {
    GstStateChangeReturn ret;
    GstBus *bus;
    HostMP3Data mp3;

    /* Initialize Gstreamer */
    gst_init (NULL, NULL);
    
    /* Initialize elements */
    mp3.pipeline = gst_pipeline_new("mp3-pipeline");
    mp3.filesrc = gst_element_factory_make("filesrc", NULL);
    mp3.audio_parse = gst_element_factory_make("mpegaudioparse", NULL);
    mp3.audio_decoder = gst_element_factory_make("avdec_mp3", NULL);
    mp3.audio_queue = gst_element_factory_make("queue", NULL);
    mp3.audio_convert = gst_element_factory_make("audioconvert", NULL);
    mp3.audio_encoder = gst_element_factory_make("lamemp3enc", NULL);
    mp3.audio_payloader = gst_element_factory_make("rtpmpapay", NULL);
    mp3.audio_udp_sink = gst_element_factory_make("udpsink", NULL);

    /* Check the elements are created or not */
    if (!mp3.pipeline || !mp3.filesrc || !mp3.audio_parse || !mp3.audio_decoder || !mp3.audio_queue ||
        !mp3.audio_convert || !mp3.audio_encoder || !mp3.audio_payloader || !mp3.audio_udp_sink) {
            g_printerr("Not all the elements could be created.\n");
            exit(EXIT_FAILURE);
        }
    
    /* Add all the elements to the Bin */
    gst_bin_add_many(GST_BIN(mp3.pipeline), mp3.filesrc, mp3.audio_parse, mp3.audio_decoder, mp3.audio_queue,
                        mp3.audio_convert, mp3.audio_encoder, mp3.audio_payloader, mp3.audio_udp_sink, NULL);

    /* Set the element properties */        
    g_object_set(G_OBJECT(mp3.filesrc), "location", "/home/ee212798/Downloads/sample.mp3", NULL);

    g_object_set(G_OBJECT(mp3.audio_udp_sink), "host", "10.1.137.49",
                                                "port", 5000,
                                                "clients", "10.1.138.194:5000,10.1.136.123:5000", NULL);

    /* Link the elements */
    if (gst_element_link_many(mp3.filesrc, mp3.audio_parse, mp3.audio_decoder, mp3.audio_queue, 
                            mp3.audio_convert, mp3.audio_encoder, mp3.audio_payloader, 
                            mp3.audio_udp_sink, NULL) != TRUE) {
                                g_printerr("Elements are not linked.\n");
                                exit(EXIT_FAILURE);
                            }
    
    /* Set the pipeline for playing State */
    ret = gst_element_set_state(mp3.pipeline, GST_STATE_PLAYING);
    
    if (ret = GST_STATE_CHANGE_FAILURE) {
        g_printerr("Could not set the pipeline to playing.\n");
        exit(EXIT_FAILURE);
    }

    bus = gst_element_get_bus(mp3.pipeline);
    gst_bus_add_signal_watch(bus);

    /* Connect signal messages that came from bus */
    g_signal_connect(bus, "message", G_CALLBACK(callback_message), &mp3);

    /* Start the Main Loop event */
    mp3.loop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (mp3.loop);

    gst_element_set_state(mp3.pipeline, GST_STATE_NULL);
    gst_object_unref(mp3.pipeline);
    gst_object_unref(bus);
    g_main_loop_unref(mp3.loop);

    return 0;
}