#include "Logger.h"
#include "MainWindow.h"


void Logger::h(const std::string & s)
{
	if(window)
		window->log(s);
}

Logger::Logger(MainWindow *w)
{
	window = w;
}


Logger::~Logger()
{
}
