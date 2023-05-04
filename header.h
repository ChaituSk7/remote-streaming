#ifndef HEADER_H
#define HEADER_H
#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>
#include <string>

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
    GstElement *audio_queue;
    GstElement *audio_convert;
    GstElement *audio_resample;
    GstElement *audio_encoder;
    GstElement *rtp_audio_payload;
    GstElement *udp_sink_audio;
    GMainLoop *loop;
}HostMP4Data;

typedef struct _HostMP3Data {
    GstElement *pipeline;
    GstElement *filesrc;
    GstElement *audio_parse;
    GstElement *audio_decoder;
    GstElement *audio_queue;
    GstElement *audio_convert;
    GstElement *audio_encoder;
    GstElement *audio_payloader;
    GstElement *audio_udp_sink;
    GMainLoop *loop;
}HostMP3Data;

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
    GstElement *audio_convert;
    GstElement *audio_encoder;
    GstElement *audio_payload;
    GstElement *udp_audio_sink;
    GMainLoop *loop;
}HostWEBMData;

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
    GstElement *audio_convert;
    GstElement *audio_encoder;
    GstElement *audio_payload;
    GstElement *udp_audio_sink;
    GMainLoop *loop;
}HostAVIData;

typedef struct _RemoteMP4Data {
    GstElement *pipeline;
    GstElement *udp_source;
    GstElement *caps_filter;
    GstElement *rtp_depay;
    GstElement *video_queue;
    GstElement *video_decoder;
    GstElement *video_sink;
    GMainLoop *loop;
}RemoteMP4Data;


extern int localhost_pipeline (char *);

extern int hostmp3_pipeline (char *);

extern int hostwebm_pipeline (char *);

extern int hostavi_pipeline (char *);

extern int remotehost_pipeline (char *);

extern int metadata_fun (std::string);

#endif