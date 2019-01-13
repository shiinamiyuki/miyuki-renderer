#include "RenderView.h"
#include "MainWindow.h"
RenderView::RenderView(QWidget *parent)
    : QLabel(parent)
{
	scale = 1;
}

RenderView::~RenderView()
{
}

void RenderView::mousePressEvent(QMouseEvent * e)
{
	pos = e->pos();
	dir = window->getRender()->getCameraDir();
}

void RenderView::mouseMoveEvent(QMouseEvent * e)
{
	auto p = e->pos();
	QVector3D v(p.x() - pos.x(), p.y() - pos.y(),0);
	v /= window->height();
	window->getRender()->setCameraDir(dir + v);
	window->getRender()->resetSample();
}

void RenderView::keyPressEvent(QKeyEvent * e)
{
	bool f = false;
	float dx =0, dy=0, dz=0;
	if (e->key() == Qt::Key::Key_Space) {
		scale = 1.0f;
		window->getRender()->resetSample();
		QVector3D offset(dx, dy, dz);
		window->getRender()->moveCameraTo(QVector3D(0,0,0));
		window->getRender()->setCameraDir(QVector3D(0, 0, 0)); return;
	}
	if (e->key() == Qt::Key::Key_Z) {
		scale /= 10.0f;
	}
	if (e->key() == Qt::Key::Key_X) {
		scale *= 10.0f;
	}
	if (e->key() == Qt::Key::Key_R) {
		dy += 1; f = true;
	}
	if (e->key() == Qt::Key::Key_F) {
		dy -= 1; f = true;
	}
	if (e->key() == Qt::Key::Key_W) {
		dz += 1; f = true;
	}
	if (e->key() == Qt::Key::Key_S) {
		dz -= 1; f = true;
	}
	if (e->key() == Qt::Key::Key_A) {
		dx -= 1; f = true;
	}
	if (e->key() == Qt::Key::Key_D) {
		dx += 1; f = true;
	}
	if (f) {
		window->getRender()->resetSample();
		auto offset = vec3(dx, dy, dz);
		offset = vecRotate(offset, vec3(1, 0, 0), window->getRender()->getCameraDir().y());
		offset = vecRotate(offset, vec3(0, 1, 0), window->getRender()->getCameraDir().x());
		window->getRender()->moveCameraBy(fromVec3(offset) * scale);
	}
}
