#include "RenderView.h"
#include "MainWindow.h"
using namespace Miyuki;
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
	vec3 v(p.x() - pos.x(), p.y() - pos.y(),0);
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
		vec3 offset(dx, dy, dz);
		window->getRender()->moveCameraTo(vec3(0,0,0));
		window->getRender()->setCameraDir(vec3(0, 0, 0)); return;
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
		offset = rotate(offset, vec3(1, 0, 0), window->getRender()->getCameraDir().y());
		offset = rotate(offset, vec3(0, 1, 0), window->getRender()->getCameraDir().x());
		window->getRender()->moveCameraBy(offset * scale);
	}
}
