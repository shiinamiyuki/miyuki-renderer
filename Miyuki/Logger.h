#pragma once
#include "Miyuki.h"
class Logger
{

	
public:
	virtual void h(const std::string&s);
	template<typename...Arg>
	void log(Arg... arg) {
		auto msg = fmt::format(arg...);
		h(msg);
	}
	Logger() {}
	~Logger();
};

