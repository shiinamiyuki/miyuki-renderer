#ifndef MIYUKI_FUTURE_HPP
#define MIYUKI_FUTURE_HPP
#include <miyuki.h>

namespace Miyuki {
	template<class T>
	class Future : public std::future<T> {
	public:
		using std::future<T>::future;
		Future(std::future<T> f):future(std::move(f)) {}


		template<class Function>
		Future<std::invoke_result_t<Function, T>> then(Function f) {
			return std::async(std::launch::async, [](Future<T> future, Function f) {
				return f(future.get());
			}, std::move(*this), std::move(f));
		}

		bool isReady() {
			return wait_for(std::chrono::seconds(0)) == std::future_status::ready;
		}
	};

	template<>
	class Future<void> : public std::future<void> {
		using T = void;
	public:
		using std::future<T>::future;
		Future(std::future<T> f) :future(std::move(f)) {}


		template<class Function>
		Future<std::invoke_result_t<Function>> then(Function f) {
			return std::async(std::launch::async, [](Future<T> future, Function f) {
				future.wait(); return f();
			}, std::move(*this), std::move(f));
		}

		bool isReady() {
			return wait_for(std::chrono::seconds(0)) == std::future_status::ready;
		}
	};

	template<class Function, class... Args>
	Future<std::invoke_result_t<Function, Args...>>
		async(Function f, Args... args) {
		return std::async(std::launch::async, f, args...);
	}
	template<class Function, class... Args>
	Future<std::invoke_result_t<Function, Args...>>
		async(std::launch policy, Function f, Args... args) {
		return std::async(policy, f, args...);
	}
}

#endif