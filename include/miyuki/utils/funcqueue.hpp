#ifndef MIYUKI_FUNCQUEUE_H
#define MIYUKI_FUNCQUEUE_H

#include <miyuki.h>
#include <utils/thread.h>

namespace Miyuki {
	template<class F>
	class FuncQueue {
		std::unique_ptr<Thread::ThreadPool> pool;
		std::atomic<bool> executable;
	public:
		FuncQueue() :executable(true) {
			pool = std::make_unique<Thread::ThreadPool>(1);
		}
		void enqueue(F&& f) {
			_queue.emplace_back(f);
		}

		void exec() {
			while (executable) {

			}
		}
	};
}


#endif