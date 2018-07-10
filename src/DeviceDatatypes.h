#ifndef DEVICE_DATATYPES_H
#define DEVICE_DATATYPES_H

#include <string>

using namespace std;

enum CameraType {RAW_CAM, RAW_H264_CAM, H264_CAM, MANUAL_CAM};
enum ResolutionPresets {LOW, MED, HIGH};
enum RTSPMessageType {GET_DEVICE_PROPS, TMP, ERR, COUNT};
enum FramePresets {FRAME_320x240, FRAME_640x480, FRAME_1280x720, FRAME_COUNT};
enum VideoPresets {VIDEO_320x240x15, VIDEO_640x480x15, VIDEO_1280x720x15,
                   VIDEO_320x240x30, VIDEO_640x480x30, VIDEO_1280x720x30,
                   VIDEO_320x240x60, VIDEO_640x480x60, VIDEO_1280x720x60
                  };
const static int AUTO_PRESET = 1024;
const static vector<string> RTSPMessageHeader = {
    "GDP", "TMP", "RES"
};

struct v4l2_info {
    string camera_name;
    string mount_point;
    CameraType camera_type;
    guint32 current_quality;
    guint64 frame_property_bitmask;
};

#endif