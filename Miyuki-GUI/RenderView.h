#pragma once
#include "MiyukiGUI.h"
#include <QLabel>
class MainWindow;
class RenderView;
class RenderView : public QLabel
{
	Q_OBJECT
private:
	MainWindow * window;
	float scale;
	QPoint pos;
	Miyuki::vec3 dir;
public:
	void setMainWindow(MainWindow*w) { window = w; }
	RenderView(QWidget *parent);
	~RenderView();
	void mousePressEvent(QMouseEvent*e)override;
	void mouseMoveEvent(QMouseEvent*e)override;
	void keyPressEvent(QKeyEvent*w)override;
};
