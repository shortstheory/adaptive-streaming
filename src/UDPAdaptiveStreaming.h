#ifndef UDP_ADAPTIVE_STREAMING_H
#define UDP_ADAPTIVE_STREAMING_H

#include "GenericAdaptiveStreaming.h"

using namespace std;

class UDPAdaptiveStreaming : public GenericAdaptiveStreaming {
private:
    const gint video_sink_port;
    const gint rtcp_port;
    const string receiver_ip_addr;

    GstElement* rtpbin;
    GstElement* rtp_identity;
    GstElement* rr_rtcp_identity;
    GstElement* sr_rtcp_identity;
    GstElement* video_udp_sink;
    GstElement* rtcp_udp_sink;
    GstElement* rtcp_udp_src;
    string rtcp_caps_string;

    bool init_rtp_elements();
    bool link_all_elements() override;

    void init_rtp_element_properties();
    void pipeline_add_rtp_elements();
    void rtcp_callback(GstElement* src, GstBuffer *buf);
    void rtp_callback(GstElement* src, GstBuffer* buf);

    static void static_callback(GstElement* src, GstBuffer* buf, gpointer data);
    static void static_rtp_callback(GstElement* src, GstBuffer* buf, gpointer data);
    // static void 

public:
    UDPAdaptiveStreaming(string _device = "/dev/video0", CameraType type = CameraType::RAW_CAM,
                        string _ip_addr = "127.0.0.1", gint _video_port = 5000,
                        gint _rtcp_port = 5001);
    ~UDPAdaptiveStreaming() override;
};

#endif