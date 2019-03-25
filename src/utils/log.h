//
// Created by Shiina Miyuki on 2019/3/24.
//

#ifndef MIYUKI_LOG_H
#define MIYUKI_LOG_H

#include <miyuki.h>

namespace Miyuki {
    namespace Log {
        enum Level {
            verbose,
            silent
        };
        enum Type {
            debug,
            info,
            warning,
            error,
            fatal
        };

        void SetLogLevel(Level level);

        Level CurrentLogLevel();

        template<typename... Args>
        void log(const char *fmt, Args...args) {
            if (CurrentLogLevel() == verbose)
                fmt::print(fmt, args...);
        }

        template<typename... Args>
        void log(Type type, const char *fmt, Args...args) {
            if (type == error || type == fatal)
                fmt::print(stderr, fmt, args...);
            else if (CurrentLogLevel() == verbose)
                fmt::print(fmt, args...);
        }
    }
}
#endif //MIYUKI_LOG_H
