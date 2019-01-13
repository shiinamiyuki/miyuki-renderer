//
// Created by xiaoc on 2019/1/12.
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
    GetEmbreeDevice();
}

void Miyuki::Exit() {
    rtcReleaseDevice(rtcDevice);
}

