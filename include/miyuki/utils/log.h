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

		void _LogInternal(const std::string& s);
		void _LogInternal(FILE *, const std::string& s);

        template<typename... Args>
        void log(const char *fmt, Args...args) {
            if (CurrentLogLevel() == verbose)
				_LogInternal(fmt::format(fmt, args...));
        }

        template<typename... Args>
        void log(Type type, const char *fmt, Args...args) {
            if (type == error || type == fatal)
				_LogInternal(fmt::format(fmt, args...));
            else if (CurrentLogLevel() == verbose)
				_LogInternal(fmt::format(fmt, args...));
        }

		void addHandler(std::function<void(const std::string&)> handler);
    }
}
#endif //MIYUKI_LOG_H
