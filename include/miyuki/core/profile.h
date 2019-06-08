//
// Created by Shiina Miyuki on 2019/3/3.
//

#ifndef MIYUKI_PROFILE_H
#define MIYUKI_PROFILE_H

#include "miyuki.h"

namespace Miyuki {
    class Timer {
	protected:
        std::chrono::time_point<std::chrono::system_clock> start;
    public:
		Timer();

        double elapsedSeconds()const;

        ~Timer();
    };
	class Profiler : public Timer {

	};
}
#endif //MIYUKI_PROFILE_H
