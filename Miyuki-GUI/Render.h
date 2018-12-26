#pragma once
#include "MiyukiGUI.h"
#include "../Miyuki/Miyuki.h"
#include "../Miyuki/Scene.h"
#include "WindowLogger.h"
class MainWindow;
class Render : public Miyuki::Scene
{
	std::vector<unsigned char> pixel;
	MainWindow * window;
	friend class MainWindow;
	void checkReset();
	int maxInteractiveRenderSample();
	void renderPass();
public:
	enum class Mode {
		preview,
		renderPathTracing,
		renderPM,
	}mode;
	void initLogger()override;
	Render(MainWindow*,int w,int h);
	~Render();
	void copyFiltered(QPixmap&map);
	void copyImage(QPixmap&map);
	void interactiveRender();
};

