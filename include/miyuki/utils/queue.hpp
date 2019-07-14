#ifndef MIYUKI_QUEUE_HPP
#define MIYUKI_QUEUE_HPP

#include <miyuki.h>

namespace Miyuki {
	namespace Thread {
		template<class T>
		class BlockingQueue {
			std::deque<T> queue;
			std::mutex mutex;
			std::condition_variable cv;
			void notify() {
				cv.notify_one();
			}
		public:
			void push(T&& value) {
				std::unique_lock<std::mutex> lock(mutex);
				queue.emplace_front(value);
				notify();

			}
			void push(const T& value) {
				std::unique_lock<std::mutex> lock(mutex);
				queue.push_front(value);
				notify();
			}
			T pop() {
				std::unique_lock<std::mutex> lock(mutex);
				cv.wait(lock, [=]() {
					return !queue.empty();
				});
				auto value = std::move(queue.back());
				queue.pop_back();
				return value;
			}
			template< class Rep, class Period >
			std::optional<T> pop(const std::chrono::duration<Rep, Period>& rel_time) {
				std::unique_lock<std::mutex> lock(mutex);
				bool has = cv.wait_for(lock, rel_time, [=]() {
					return !queue.empty();
				});
				if (has) {
					auto value = std::move(queue.back());
					queue.pop_back();
					return value;
				}
				else {
					return {};
				}
			}
		};
	}
}
#endif