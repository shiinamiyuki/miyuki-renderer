#ifndef MIYUKI_FUTURE_HPP
#define MIYUKI_FUTURE_HPP
#include <miyuki.h>

namespace Miyuki {
	template<class T>
	struct Future : std::future<T> {
		using std::future<T>::future;
		Future(std::future<T> f):future(std::move(f)) {}


		template<class Function>
		Future<std::invoke_result_t<Function, T>> then(Function f) {
			return std::async(std::launch::async, [](Future<T> future, Function f) {
				return f(future.get());
			}, std::move(*this), std::move(f));
		}
	};
	template<class Function, class... Args>
	Future<std::invoke_result_t<Function, Args...>>
		Async(Function f, Args... args) {
		return std::async(std::launch::async, f, args...);
	}
	template<class Function, class... Args>
	Future<std::invoke_result_t<Function, Args...>>
		Async(std::launch policy, Function f, Args... args) {
		return std::async(policy, f, args...);
	}
}

#endif