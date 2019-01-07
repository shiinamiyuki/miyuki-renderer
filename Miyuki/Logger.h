#pragma once
#include "util.h"
class MainWindow;
class Logger
{
	
	void h(const std::string&s);
public:
	MainWindow *window;
	template<typename...Arg>
	void log(Arg... arg) {
		auto msg = fmt::format(arg...);
		h(msg);
	}
	Logger(const Logger&l) { window = l.window; }
	Logger() { window = nullptr; }
	Logger(MainWindow *w);
	~Logger();
};

