//
// Created by Shiina Miyuki on 2019/1/12.
//

#include "util.h"

#include "../lib/stb/stb_image.h"
#include <boost/algorithm/string/predicate.hpp>

static RTCDevice rtcDevice = nullptr;

RTCDevice Miyuki::GetEmbreeDevice() {
    if (!rtcDevice) {
        rtcDevice = rtcNewDevice(nullptr);
        assert(rtcGetDeviceError(rtcDevice) == RTC_ERROR_NONE);
    }
    return rtcDevice;
}

void Miyuki::Init() {
    if (!GetEmbreeDevice()) {
        fmt::print(stderr, "Error creating embree device. Exiting...\n");
        exit(-1);
    }

}

void Miyuki::Exit() {
    rtcReleaseDevice(rtcDevice);
}

void Miyuki::readUnderPath(const std::string &filename, std::function<void(const std::string &)> f) {
    cxx::filesystem::path currentPath = cxx::filesystem::current_path();

    cxx::filesystem::path inputFile(filename);
    auto file = inputFile.filename().string();
    auto parent = inputFile.parent_path();
    if (!parent.empty())
        cxx::filesystem::current_path(parent);

    f(file);
    cxx::filesystem::current_path(currentPath);
}


void
Miyuki::loadImage(const std::string &filename, std::vector<unsigned char> &data, unsigned int *w, unsigned int *h) {
    if (boost::algorithm::ends_with(filename, ".png")) {
        lodepng::decode(data, *w, *h, filename);
    } else if (boost::algorithm::ends_with(filename, ".jpg") || boost::algorithm::ends_with(filename, ".jpeg")) {

    } else {
        fmt::print(stderr, "Unrecognize image format {}\n", filename);
    }
}

