#pragma once

#include <miyuki.h>
namespace Miyuki {
	class Error {
		std::string message;
	public:
		Error(const char* msg) :message(msg) {}
		Error(const std::string& msg) :message(msg) {}
		const char* what()const noexcept { return message.c_str(); }
	};
}