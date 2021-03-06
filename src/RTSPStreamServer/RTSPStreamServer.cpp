#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/videodev2.h>
#include <dirent.h>
#include <string>
#include <string.h>
#include <iostream>
#include <fcntl.h>
#include <algorithm>
#include <experimental/filesystem>
#include <libconfig.h++>

#include "RTSPStreamServer.h"
namespace fs = std::experimental::filesystem;

RTSPStreamServer::RTSPStreamServer(string _ip_addr, string _port) : ip_addr(_ip_addr), port(_port)
{
    server = gst_rtsp_server_new();
    gst_rtsp_server_set_address(server, ip_addr.c_str());
    gst_rtsp_server_set_service(server, port.c_str());

    get_v4l2_devices_paths();
    cerr << "***APStreamline***\nAccess the following video streams using VLC or gst-launch following "
    "the instructions here: https://github.com/shortstheory/adaptive-streaming#usage"
    "\n==============================\n";
    setup_streams();
}

RTSPStreamServer::~RTSPStreamServer()
{
}

vector<string> RTSPStreamServer::get_v4l2_devices_paths()
{
    vector<string> device_list;
    for (const auto & entry : fs::directory_iterator("/dev")) {
        string s = entry.path();
        if (s.find("video") != std::string::npos) {
            // Add device path to list
            device_list.push_back(s);
        }
    }
    // To make mount points and the device names in the same order
    reverse(device_list.begin(), device_list.end());
    return device_list;
}

bool RTSPStreamServer::check_h264_ioctls(int fd)
{
    v4l2_queryctrl bitrate_ctrl;
    bitrate_ctrl.id = V4L2_CID_MPEG_VIDEO_BITRATE;

    if (ioctl(fd, VIDIOC_S_CTRL, &bitrate_ctrl) == -1) {
        return false;
    }
    return true;
}

pair<CameraType, string> RTSPStreamServer::get_camera_type(const string &device_path)
{
    int fd = open(device_path.c_str(), O_RDONLY);
    if (fd != -1) {
        v4l2_capability caps;
        ioctl(fd, VIDIOC_QUERYCAP, &caps);
        // V4L2 annoyingly lists each camera twice, so we need to filter the ones
        // which don't support the VIDEO_CAPTURE capability
        if (!(caps.device_caps & V4L2_CAP_VIDEO_CAPTURE)) {
            return make_pair(CameraType::NOT_SUPPORTED, "");
        }

        string camera_name;
        string driver_name;
        camera_name = string(caps.card, caps.card + sizeof(caps.card)/sizeof(caps.card[0]));
        driver_name = string(caps.driver, caps.driver+sizeof(caps.driver)/sizeof(caps.card[0]));

        // FIXME: add more camera IDs
        pair<CameraType, string> cameraPair;
        cameraPair = make_pair(CameraType::NOT_SUPPORTED, "");
        if (camera_name.find("mmal service 16.1") != string::npos) {
            cameraPair = make_pair(CameraType::RPI_CAM, camera_name);
        } else if (camera_name.find("ar0521") != string::npos) {
            cameraPair = make_pair(CameraType::AR0521_CAM, camera_name);
        } else if (camera_name.find("HD Pro Webcam C920") != string::npos) {
            cameraPair = make_pair(CameraType::C920_CAM, camera_name);
        } else if (camera_name.find("ZED") != string::npos) {
            cameraPair = make_pair(CameraType::ZED_CAM, camera_name);
        } else {
            // MJPG is our fallback option
            v4l2_fmtdesc fmt;
            v4l2_buf_type type;
            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            memset(&fmt, 0, sizeof(fmt));
            fmt.type = type;
            fmt.index = 0;
            while (ioctl(fd, VIDIOC_ENUM_FMT, &fmt) >= 0) {
                string description((char*)fmt.description);
                if (description.find("Motion-JPEG") != string::npos
                || description.find("MJPG") != string::npos
                || description.find("M.JPG") != string::npos) {
                    cameraPair = make_pair(CameraType::MJPG_CAM, camera_name);
                    break;
                }
                fmt.index++;
            }
        }
        close(fd);
        return cameraPair;
    }
    return make_pair(CameraType::NOT_SUPPORTED, "");
}

void RTSPStreamServer::setup_streams()
{
    int i = 0;
    for (string device : get_v4l2_devices_paths()) {
        // why is this a pointer?
        pair<CameraType, string> camera_info = get_camera_type(device);
        CameraType type;
        type = camera_info.first;

        if (type == CameraType::NOT_SUPPORTED) {
            continue;
        }

        i++;
        string mount_point;
        mount_point = "/cam" + to_string(i);

        string camera_name;
        camera_name = camera_info.second;

        adaptive_streams_map.insert(pair<string, shared_ptr<RTSPAdaptiveStreaming>>(device,
                                    new RTSPAdaptiveStreaming(device, type, mount_point, camera_name, server)));
        string camera_description;
        camera_description = device + " (" + camera_name + "): rtsp://" + ip_addr + ":"+ port + mount_point;
        cerr << camera_description << endl;
    }
}

void RTSPStreamServer::set_service_id(guint id)
{
    service_id = id;
}

map<string, shared_ptr<RTSPAdaptiveStreaming>> RTSPStreamServer::get_stream_map()
{
    return adaptive_streams_map;
}

GstRTSPServer* RTSPStreamServer::get_server()
{
    return server;
}

string RTSPStreamServer::get_ip_address()
{
    return ip_addr;
}

string RTSPStreamServer::get_port()
{
    return port;
}