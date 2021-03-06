

#include "Camera.h"

Camera::Camera(string device, Quality q) : device_path(device), current_quality(q)
{}

bool Camera::set_bitrate(guint32 _bitrate)
{
    return dynamic_bitrate;
}

string Camera::get_name()
{
    return camera_name;
}

int Camera::get_supported_qualities()
{
    return supported_qualities;
}

bool Camera::set_quality(Quality q)
{
    return dynamic_res;
}

Quality Camera::get_quality()
{
    return current_quality;
}

string Camera::get_device_path()
{
    return device_path;
}

void Camera::set_bitrates_constants(bool congested)
{
    if (congested) {
        min_bitrate = congested_state_min;
        max_bitrate = congested_state_max;
        increment_bitrate = congested_state_increment;
        decrement_bitrate = congested_state_decrement;
    } else {
        min_bitrate = steady_state_min;
        max_bitrate = steady_state_max;
        increment_bitrate = steady_state_increment;
        decrement_bitrate = steady_state_decrement;
    }
}

bool Camera::read_configuration(Setting& camera_config, Setting& quality_config)
{
    steady_state_min = quality_config.lookup("quality.steady_state.min");
    steady_state_max = quality_config.lookup("quality.steady_state.max");
    steady_state_increment = quality_config.lookup("quality.steady_state.increment");
    steady_state_decrement = quality_config.lookup("quality.steady_state.decrement");

    congested_state_min = quality_config.lookup("quality.congested_state.min");
    congested_state_max = quality_config.lookup("quality.congested_state.max");
    congested_state_increment = quality_config.lookup("quality.congested_state.increment");
    congested_state_decrement = quality_config.lookup("quality.congested_state.decrement");

    low_bitrate = quality_config.lookup("quality.bitrate.low");
    medium_bitrate = quality_config.lookup("quality.bitrate.med");
    high_bitrate = quality_config.lookup("quality.bitrate.high");

    launch_string = static_cast<const char *>(camera_config.lookup("camera.properties.launch_string"));
    capsfilter = static_cast<const char *>(camera_config.lookup("camera.properties.capsfilter"));
    dynamic_res = camera_config.lookup("camera.properties.dynamic_res");
    dynamic_bitrate = camera_config.lookup("camera.properties.dynamic_bitrate");
    supported_qualities = camera_config.lookup("camera.properties.supported_qualities");

    const Setting& low_res = camera_config.lookup("camera.resolutions.low");
    resolutions[Quality::Level::LOW].first = low_res[0];
    resolutions[Quality::Level::LOW].second = low_res[1];

    const Setting& med_res = camera_config.lookup("camera.resolutions.medium");
    resolutions[Quality::Level::MEDIUM].first = med_res[0];
    resolutions[Quality::Level::MEDIUM].second = med_res[1];

    const Setting& high_res = camera_config.lookup("camera.resolutions.high");
    resolutions[Quality::Level::HIGH].first = high_res[0];
    resolutions[Quality::Level::HIGH].second = high_res[1];

    const Setting& framerate_config = camera_config.lookup("camera.framerates");
    framerates[Quality::Level::LOW] = framerate_config[0];
    framerates[Quality::Level::MEDIUM] = framerate_config[1];
    framerates[Quality::Level::HIGH] = framerate_config[2];
    return true;
}

string Camera::generate_capsfilter() const
{
    regex w("%width");
    regex h("%height");
    regex f("%framerate");
    string caps;
    caps = capsfilter;

    pair<int, int> res;
    res = resolutions.at(current_quality.getResolution());
    int framerate;
    framerate = framerates.at(current_quality.getFramerate());

    caps = regex_replace(caps, w, to_string(res.first));
    caps = regex_replace(caps, h, to_string(res.second));
    caps = regex_replace(caps, f, to_string(framerate));
    return caps;
}

bool Camera::dynamic_res_capability() const
{
    return dynamic_res;
}

bool Camera::dynamic_bitrate_capability() const
{
    return dynamic_bitrate;
}

void Camera::improve_quality(bool congested)
{
    set_bitrates_constants(congested);
    if (dynamic_bitrate_capability()) {
        set_bitrate(bitrate + increment_bitrate);
    }
    if (dynamic_res_capability()) {
        if (current_quality.getResolution() == Quality::Level::LOW && bitrate > medium_bitrate) {
            Quality mediumQuality(Quality::Level::MEDIUM, Quality::Level::MEDIUM);
            set_quality(mediumQuality);
        } else if (current_quality.getResolution() == Quality::Level::MEDIUM && bitrate > high_bitrate) {
            Quality highQuality(Quality::Level::HIGH, Quality::Level::MEDIUM);
            set_quality(highQuality);
        }
    }
}

void Camera::degrade_quality(bool congested)
{
    set_bitrates_constants(congested);
    if (dynamic_bitrate_capability()) {
        set_bitrate(bitrate - decrement_bitrate);
    }
    if (dynamic_res_capability()) {
        if (current_quality.getResolution() == Quality::Level::MEDIUM && bitrate < medium_bitrate) {
            Quality lowQuality(Quality::Level::LOW, Quality::Level::MEDIUM);
            set_quality(lowQuality);
        } else if (current_quality.getResolution() == Quality::Level::HIGH && bitrate < high_bitrate) {
            Quality mediumQuality(Quality::Level::MEDIUM, Quality::Level::MEDIUM);
            set_quality(mediumQuality);
        }
    }
}