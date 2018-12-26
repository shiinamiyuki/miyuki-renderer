#include "MainWindow.h"
#include "../Miyuki/Primitive.h"
using namespace Miyuki;
MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	ui.renderView->setMainWindow(this);
	saving = false;

	render = new Render(this,1000,1000);
	startTimer(1000 / 30);

	connect(ui.capture, &QPushButton::pressed, this, &MainWindow::save);
	connect(ui.checkBox, &QCheckBox::stateChanged, this, &MainWindow::switchMode);
	connect(ui.actionTo_Clipboard, &QAction::triggered, this, &MainWindow::copyToClipboard);
}
/*
std::thread load([&]() {
		render->openSession("default");
		//render->moveCameraTo(vec3(250, 250, -550));
	//	render->moveCameraTo(vec3(0, 0.8, 3));
	//	render->setCameraDir(vec3(pi, 0, 0));
		render->moveCameraTo(vec3(0, 100, -480));
		render->setWorldColor(vec3(100, 149, 237)/255.0);
		render->loadObj("cornell_box.obj", vec3(-160, 0, 30), vec3(0, 0, 0),6*60/500.0);
		render->loadObj("xmas.obj",vec3(0,0,0),vec3(-pi/2,0,0));
		//render->addObject(new Sphere(vec3(130, 350, 350), 80, Material::makeRefr(vec3(1,1,1),1.5)));
		render->addObject(new Sphere(vec3(250,500,-250), 80, Material::makeEmission(vec3(60,60,60))));
		//render->addObject(makeSphere(vec3(-100000, 0, 250), 100000, makeDiffuse(vec3(0.75,0.25,0.25))));
		//render->addObject(makeSphere(vec3(100000+500, 0, 250), 100000, makeDiffuse(vec3(0.25, 0.25, 0.75))));
		render->addObject(new Sphere(vec3(250,-100000+1, 250), 100000, Material::makeDiffuse(vec3(1,1,1))));
		//render->addObject(makeSphere(vec3(250, 100000+600, 250), 100000, makeDiffuse(vec3(1, 1, 1))));
		//render->addObject(new Sphere(vec3(250,  250,10000 + 100), 10000, Material::makeDiffuse(vec3(1, 1, 1))));
		render->prepare();
		render->interactiveRender();
	});
*/
void MainWindow::start()	
{
	log(fmt::format("SIMD width={}bit\n", 8*sizeof(Float)*simdVec::width()));
	std::thread load([&]() {
		render->openSession("default");
		render->moveCameraTo(vec3(280, 250, -550));
		render->loadObj("cornell_box.obj");
		//render->addObject(new Sphere(vec3(130, 350, 350), 80, Material::makeRefr(vec3(1, 1, 1), 1.5)));
		render->prepare();
		render->interactiveRender();
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
	system(fmt::format("py scripts/filter.py {}", name.toStdString()).c_str());
}

void MainWindow::switchMode()
{
	render->resetSample();
	if (ui.checkBox->isChecked()) {
		render->mode = Render::Mode::renderPathTracing;
	}
	else {
		render->mode = Render::Mode::preview;
	}
}

void MainWindow::copyToClipboard()
{
	saving = true;
	auto cb = QApplication::clipboard();
	cb->setPixmap(image);
	saving = false;
}

void MainWindow::updateImage()
{
	render->copyImage(image);
	ui.renderView->setPixmap(image);
}

void MainWindow::startInteractive()
{
}

void MainWindow::startFinal()
{
}

void MainWindow::stop()
{
}
