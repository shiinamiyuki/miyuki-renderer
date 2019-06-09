#pragma once

#include "error.hpp"
#include <variant>
namespace Miyuki {
	template<class T>
	using Optional = std::optional<T>;

	template<class T, class E = Error>
	class Result {
		std::variant<T, E> data;
	public:
		Result(const T& value) :data(value) {}
		Result(T&& value) :data(value) {}
		Result(const E& err) :data(err) {}
		Result(E&& err) :data(err) {}
		operator bool()const noexcept {
			return hasValue();
		}
		bool hasValue()const noexcept {
			return std::get_if<T>(&data) != nullptr;
		}
		bool hasError()const noexcept {
			return !hasValue();
		}

		const T& value()const noexcept {
			if (hasValue())
				return *std::get_if<T>(&data);
			std::abort();
		}
		T& value() noexcept {
			if(hasValue())
				return *std::get_if<T>(&data);
			std::abort();
		}
		const E& error()const noexcept {
			if (hasError())
				return *std::get_if<E>(&data);
			std::abort();
		}
		E& error() noexcept {
			if (hasError())
				return *std::get_if<E>(&data);
			std::abort();
		}
	private:
		struct Match {
			Result& result;
			std::function<void(const T&)> fT;
			std::function<void(const E&)> fE;
			Match(Result& result, const std::function<void(const T&)>& f) :fT(f),result(result) {}
			Match(Result& result, const std::function<void(const E&)>& f) :fE(f), result(result) {}
			void match()noexcept {
				if (result.hasValue() && fT) {
					fT(result.value());
				}
				else if(fE){
					fE(result.error());
				}
			}
			Match& with(const std::function<void(const T&)>& f)noexcept {
				fT = f;
				return *this;
			}
			Match& with(const std::function<void(const E&)>& f)noexcept {
				fE = f;
				return *this;
			}
		};
	public:
		template<class F>
		Match with(const F& f) { return {*this, f }; }

	};
}