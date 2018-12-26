#pragma once
#include "../Miyuki/Logger.h"
class MainWindow;
class WindowLogger :
	public Logger
{
	
	MainWindow * window;
public:
	WindowLogger(MainWindow * window);
	void h(const std::string&s)override;
	~WindowLogger();
};

