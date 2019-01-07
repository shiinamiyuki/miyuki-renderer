#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	ui.renderView->setMainWindow(this);
	saving = false;
	logger = new Logger(this);

	render = new Render(this);
	
	startTimer(1000 / 30);

	connect(ui.capture, &QPushButton::pressed, this, &MainWindow::save);
	connect(ui.actionAmbient_Occlusion, &QAction::triggered, this, &MainWindow::selectAO);
//	connect(ui.actionBDPT, &QAction::triggered, this, &MainWindow::selectBDPT);
	connect(ui.actionPath_tracing, &QAction::triggered, this, &MainWindow::selectPT);
	connect(ui.actionModel_View, &QAction::triggered, this, &MainWindow::modelView);
	connect(ui.actionTo_Clipboard, &QAction::triggered, this, &MainWindow::copyToClipboard);
}

void MainWindow::start()
{
	std::thread load([&]() {
		//render->loadObj("cornell_box.obj");
		render->addMaterial("light", makeEmission(vec3(4,4,4) * 8));
		render->addMaterial("white", makeDiffuse(vec3(1, 1, 1)));
		render->addMaterial("glossy", makeSpec(vec3(1, 1, 1),0.1));
		render->addObject(makeSphere(vec3(0, 16, 4), 2,  render->getMaterial("light")));
		render->addObject(makeSphere(vec3(0,-10000,0), 10000, render->getMaterial("white")));
		render->addObject(makeSphere(vec3(0,2,4), 2, render->getMaterial("white")));
		render->addObject(makeSphere(vec3(-6, 2, 4), 2, render->getMaterial("glossy")));
		render->moveCameraTo(QVector3D(0,2,-5));
		render->render();
	});

	load.detach();
}

void MainWindow::resizeEvent(QResizeEvent * e)
{
	auto size = e->size();
	QPoint pos(250, 50);
	ui.renderView->move(pos);
	ui.log->move(QPoint(0, pos.y()));
	ui.log->resize(pos.x(), this->height() - pos.y());
	ui.renderView->resize(this->width() - pos.x(), this->height() - pos.y());
}

void MainWindow::timerEvent(QTimerEvent * e)
{
	QMainWindow::timerEvent(e); 
	if (render->signal.hasNew ) {
		render->signal.hasNew = false;
		render->copyImage(this->image);
		ui.renderView->setPixmap(image);
	}
	
}

void MainWindow::save()
{
	saving = true;
	if (!QDir("snapshots").exists())
		QDir().mkdir("snapshots");
	auto t = time(0);
	QString name = QString("snapshots/snapshots-").append(std::ctime(&t)).append(".png");
	name.replace(' ', "-");
	name.replace('\n', "-");
	name.replace(':', "-");
	if (!image.save(name)) {
		log(QString("cannot save ").append(name));
	}
	else {
		log(QString("saved ").append(name));
	}
	saving = false;
}

void MainWindow::switchMode()
{
	render->resetSample();

}

void MainWindow::copyToClipboard()
{
	saving = true;
	auto cb = QApplication::clipboard();
	cb->setPixmap(image);
	saving = false;
}

void MainWindow::resetIntegratorType(Render::Integrator::Type t)
{
	render->resetSample();
	render->integrator.setIntegrator(t);
}

void MainWindow::setRenderOptionEnabled()
{

}

void MainWindow::modelView()
{
	resetIntegratorType(Render::Integrator::Type::raycast);
}

void MainWindow::selectPT()
{
	resetIntegratorType(Render::Integrator::Type::pt);
}

void MainWindow::selectBDPT()
{
	resetIntegratorType(Render::Integrator::Type::bdpt);
}

void MainWindow::selectAO()
{
	resetIntegratorType(Render::Integrator::Type::ao);
}

void MainWindow::startInteractive()
{
	setRenderOptionEnabled();
}
