#ifndef HEADER_H
#define HEADER_H
#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>
#include <bits/stdc++.h>
#include <string>
using namespace std;

// Structure of Elements for Mp4 file
typedef struct _HostMP4Data {
	GstElement *pipeline;
	GstElement *source;
	GstElement *demuxer;
	GstElement *video_decoder;
	GstElement *video_queue;
	GstElement *video_convert;
	GstElement *video_encoder;
	GstElement *rtp_payload;
	GstElement *udp_sink_video;

	GstElement *audio_decoder;
	GstElement *audio_volume;
	GstElement *audio_queue;
	GstElement *audio_convert;
	GstElement *audio_resample;
	GstElement *audio_encoder;
	GstElement *rtp_audio_payload;
	GstElement *udp_sink_audio;
	GMainLoop *loop;
} HostMP4Data;

// Structure of Elements for Mp3 file
typedef struct _HostMP3Data {
	GstElement *pipeline;
	GstElement *filesrc;
	GstElement *audio_parse;
	GstElement *audio_decoder;
	GstElement *audio_queue;
	GstElement *audio_volume;
	GstElement *audio_convert;
	GstElement *audio_encoder;
	GstElement *audio_payloader;
	GstElement *audio_udp_sink;
	GMainLoop *loop;
} HostMP3Data;

// Structure of Elements for Webm file
typedef struct _HostWEBMData {
	GstElement *pipeline;
	GstElement *source;
	GstElement *demux;
	GstElement *video_queue;
	GstElement *video_decoder;
	GstElement *video_convert;
	GstElement *video_encoder;
	GstElement *video_payload;
	GstElement *udp_video_sink;
	GstElement *audio_queue;
	GstElement *audio_decoder;
	GstElement *audio_volume;
	GstElement *audio_convert;
	GstElement *audio_encoder;
	GstElement *audio_payload;
	GstElement *udp_audio_sink;
	GMainLoop *loop;
} HostWEBMData;

// Structure of Elements for Avi file
typedef struct _HostAVIData {
	GstElement *pipeline;
	GstElement *source;
	GstElement *demux;
	GstElement *video_queue;
	GstElement *video_parser;
	GstElement *video_decoder;
	GstElement *video_convert;
	GstElement *video_encoder;
	GstElement *video_payload;
	GstElement *udp_video_sink;
	GstElement *audio_queue;
	GstElement *audio_parser;
	GstElement *audio_decoder;
	GstElement *audio_volume;
	GstElement *audio_convert;
	GstElement *audio_encoder;
	GstElement *audio_payload;
	GstElement *udp_audio_sink;
	GMainLoop *loop;
} HostAVIData;

// Structure of Elements for creating thumbnail image
typedef struct _ThumbnailData {
	GstElement *pipeline;
	GstElement *src;
	GstElement *demuxer;
	GstElement *video_decoder;
	GstElement *video_queue;
	GstElement *convert;
	GstElement *rate;
	GstElement *scale;
	GstElement *capsfilter;
	GstElement *encode;
	GstElement *sink;
	gboolean playing;
	gboolean terminate;
	gboolean seek_enabled;
	gboolean seek_done;
	gint64 duration;
} ThumbnailData;

// Structure of Elements for sending thumbnail file to client
typedef struct _ImageData {
	GstElement *pipeline;
	GstElement *source;
	GstElement *jpegdec;
	GstElement *video_convert;
	GstElement *video_scale;
	GstElement *capsfilter;
	GstElement *img_freeze;
	GstElement *video_encoder;
	GstElement *rtp_payload;
	GstElement *udp_sink;
} ImageData;

// Function declaration
extern int localhost_pipeline(const char*);

extern int hostmp3_pipeline(const char*);

extern int hostwebm_pipeline(const char*);

extern int hostavi_pipeline(const char*);

extern int metadata_fun (std::string);

extern int directory_set(std::string);

extern int display_img();

extern int image_display();

extern GstPadProbeReturn my_probe_callback(GstPad*, GstPadProbeInfo*, gpointer);

#endif
