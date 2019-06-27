//
// Created by Shiina Miyuki on 2019/3/24.
//

#include <utils/log.h>
#include <ui/logwindow.h>
#include <core/profile.h>

namespace Miyuki {
    static std::atomic<Log::Level> curLevel(Log::verbose);

    void Log::SetLogLevel(Log::Level level) {
        curLevel = level;
    }

    Log::Level Log::CurrentLogLevel() {
        return curLevel;
    }
	static std::mutex mutex;
	static Timer timer;
	void Log::_LogInternal(const std::string& s) {
		std::lock_guard<std::mutex> lockGuard(mutex);
		auto s2 = fmt::format("[{:.3}]  {}", timer.elapsedSeconds(), s);
		std::cout << s2 << std::endl;
		GUI::LogWindowContent::GetInstance()->append(s2);
	}
}