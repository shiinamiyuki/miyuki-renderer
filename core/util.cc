//
// Created by Shiina Miyuki on 2019/1/12.
//

#include "util.h"
static RTCDevice rtcDevice = nullptr;
RTCDevice Miyuki::GetEmbreeDevice() {
    if(!rtcDevice) {
        rtcDevice = rtcNewDevice(nullptr);
        assert(rtcGetDeviceError(rtcDevice) == RTC_ERROR_NONE);
    }
    return rtcDevice;
}

void Miyuki::Init() {
    if(!GetEmbreeDevice()){
        fmt::print(stderr, "Error creating embree device. Exiting...\n");
        exit(-1);
    }

}

void Miyuki::Exit() {
    rtcReleaseDevice(rtcDevice);
}

