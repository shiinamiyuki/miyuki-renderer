#pragma once
#include "util.h"
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
	QVector3D dir;
public:
	void setMainWindow(MainWindow*w) { window = w; }
	RenderView(QWidget *parent);
	~RenderView();
	void mousePressEvent(QMouseEvent*e)override;
	void mouseMoveEvent(QMouseEvent*e)override;
	void keyPressEvent(QKeyEvent*w)override;
};
