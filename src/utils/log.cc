//
// Created by Shiina Miyuki on 2019/3/24.
//

#include "log.h"

namespace Miyuki {
    static std::atomic<Log::Level> curLevel(Log::verbose);

    void Log::SetLogLevel(Log::Level level) {
        curLevel = level;
    }

    Log::Level Log::CurrentLogLevel() {
        return curLevel;
    }
}