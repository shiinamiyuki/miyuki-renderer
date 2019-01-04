#include "WindowLogger.h"
#include "MainWindow.h"
class MainWindow;
WindowLogger::WindowLogger(MainWindow * _window)
{
	window = _window;
}

void WindowLogger::h(const std::string & s)
{
	fmt::print("{}", s);
	window->log(s);
}


WindowLogger::~WindowLogger()
{
}
