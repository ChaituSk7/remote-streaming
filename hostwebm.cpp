#include "header.h"

static void callback_message (GstBus *bus, GstMessage *msg, HostWEBMData *data) {

  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR: {
        GError *err;
        gchar *debug;
        gst_message_parse_error (msg, &err, &debug);
        g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
        g_printerr ("Debugging information: %s\n", debug ? debug : "none");
        g_clear_error (&err);
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


static void host_pad_handler (GstElement *src, GstPad *pad, HostWEBMData *data) {

    GstPad *video_sink_pad = gst_element_get_static_pad(data->video_queue, "sink");
    GstPad *audio_sink_pad = gst_element_get_static_pad(data->audio_queue, "sink");

    GstPadLinkReturn ret;
    GstCaps *new_pad_caps = NULL;
    GstStructure *new_pad_struct = NULL;
    const gchar *new_pad_type = NULL;

    g_print("\nReceived new pad '%s' from '%s'\n", GST_PAD_NAME(pad), GST_ELEMENT_NAME(src));
    
    new_pad_caps = gst_pad_get_current_caps (pad);
    new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
    new_pad_type = gst_structure_get_name (new_pad_struct);

    if (g_str_has_prefix (new_pad_type, "video/")) {
        
        ret = gst_pad_link (pad, video_sink_pad);
        if (GST_PAD_LINK_FAILED (ret)) {
            g_print ("Type is '%s' but link failed.\n", new_pad_type);
        } 
        else {
            g_print ("Link succeeded (type '%s').\n", new_pad_type);
        }
    }   
    if (g_str_has_prefix (new_pad_type, "audio/")) {  
        ret = gst_pad_link (pad, audio_sink_pad);
        if (GST_PAD_LINK_FAILED (ret)) {
            g_print ("Type is '%s' but link failed.\n", new_pad_type);
        } 
        else {
            g_print ("Link succeeded (type '%s').\n", new_pad_type);
        }
    }
    if (new_pad_caps != NULL)
        gst_caps_unref (new_pad_caps);
    
    gst_object_unref (video_sink_pad);
    gst_object_unref (audio_sink_pad);
} 

int hostwebm_pipeline (int argc, char *argv[]) {
    GstStateChangeReturn ret;
    GstBus *bus;
    HostWEBMData webm;
    
    /* Initialize the structure members with 0 */
    memset(&webm, 0, sizeof(webm));

    /* Initialze the gstreamer */
    gst_init (NULL, NULL);
    
    /* Initialize the elements */
    webm.pipeline = gst_pipeline_new("WEMB-pipeline");
    webm.source = gst_element_factory_make("filesrc", NULL);
    webm.demux = gst_element_factory_make("matroskademux", NULL);
    webm.video_queue = gst_element_factory_make("queue", NULL);
    webm.video_decoder = gst_element_factory_make("vp8dec", NULL);
    webm.video_convert = gst_element_factory_make("videoconvert", NULL);
    webm.video_encoder = gst_element_factory_make("vp8enc", NULL);
    webm.video_payload = gst_element_factory_make("rtpvp8pay", NULL);
    webm.udp_video_sink = gst_element_factory_make("udpsink", NULL);
    webm.audio_queue = gst_element_factory_make("queue", NULL);
    webm.audio_decoder = gst_element_factory_make("vorbisdec", NULL);
    webm.audio_convert = gst_element_factory_make("audioconvert", NULL);
    webm.audio_encoder = gst_element_factory_make("opusenc", NULL);
    webm.audio_payload = gst_element_factory_make("rtpopuspay", NULL);
    webm.udp_audio_sink = gst_element_factory_make("udpsink", NULL);

    /* Check the elements are created on not */
    if (!webm.pipeline || !webm.source || !webm.demux || !webm.video_queue || !webm.video_decoder ||
        !webm.video_convert || !webm.video_encoder || !webm.video_payload || !webm.video_payload ||
        !webm.udp_video_sink || !webm.audio_queue || !webm.audio_decoder || !webm.audio_convert ||
        !webm.audio_encoder || !webm.audio_payload || !webm.udp_audio_sink) {
            g_printerr("Not all elements could be created\n");
        }

    /* Add elements to the Bin */
    gst_bin_add_many(GST_BIN(webm.pipeline), webm.source, webm.demux, webm.video_queue, webm.video_decoder,
                    webm.video_convert, webm.video_encoder, webm.video_payload, webm.udp_video_sink,
                    webm.audio_queue, webm.audio_decoder, webm.audio_convert, webm.audio_encoder,
                    webm.audio_payload, webm.udp_audio_sink, NULL);
    
    /* Setting the element properties */
    g_object_set(G_OBJECT(webm.source), "location", "/home/ee212798/Downloads/sample1.webm", NULL);
    g_object_set(G_OBJECT(webm.video_encoder), "deadline", 1, NULL);
    g_object_set(G_OBJECT(webm.udp_video_sink), "host", "10.1.137.49",
                                                "port", 5000,
                                                "clients", "10.1.138.194:5000,10.1.136.123:5000", NULL);
    g_object_set(G_OBJECT(webm.udp_audio_sink), "host", "10.1.137.49",
                                                "port", 5001,
                                                "clients", "10.1.138.194:5001,10.1.136.123:5001", NULL);


    /* Linking the elements */
    if (gst_element_link(webm.source, webm.demux) != TRUE) {
        g_printerr("Source and demuxer not linked.\n");
        exit(EXIT_FAILURE);
    } 

    if (gst_element_link_many(webm.video_queue, webm.video_decoder,webm.video_convert, 
                webm.video_encoder, webm.video_payload, webm.udp_video_sink, NULL) != TRUE) {
                    g_printerr("video elements are not linked.\n");
                    exit(EXIT_FAILURE);
                }
    
    if (gst_element_link_many(webm.audio_queue, webm.audio_decoder, webm.audio_convert,
                webm.audio_encoder, webm.audio_payload, webm.udp_audio_sink, NULL) != TRUE) {
                    g_printerr("Audio elements are not linked.\n");
                    exit(EXIT_FAILURE);
                }

    /* Connect the pad-added signal */
    g_signal_connect(webm.demux, "pad-added", G_CALLBACK(host_pad_handler), &webm);

    /* Set the Pipeline to playing state */
    ret = gst_element_set_state(webm.pipeline, GST_STATE_PLAYING);

    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Could not set the pipeline for playing state.\n");
        exit(EXIT_FAILURE);
    }

    bus = gst_element_get_bus(webm.pipeline);
    gst_bus_add_signal_watch(bus);

    /* Connect signal messages that came from bus */
    g_signal_connect(bus, "message", G_CALLBACK(callback_message), &webm);

    /* Start the Main Loop event */
    webm.loop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (webm.loop);

    /* Unreference the resources */
    gst_element_set_state(webm.pipeline, GST_STATE_NULL);
    gst_object_unref(webm.pipeline);
    gst_object_unref(bus);
    g_main_loop_unref(webm.loop);

    return 0;
}