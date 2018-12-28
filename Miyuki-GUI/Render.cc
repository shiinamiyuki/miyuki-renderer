#include "Render.h"
#include "MainWindow.h"
#include "../Miyuki/ImageFilter.h"
using namespace Miyuki;

Render::Render(MainWindow*_w, int w, int h) : Miyuki::Scene(w, h)
{
	mode = Mode::preview;
	window = _w;
	initLogger();
}


void Render::checkReset()
{
	if (signal.reset) {
		sampleCount = 0;
		signal.reset = false;
		camera = dummy;
	}
}

int Render::maxInteractiveRenderSample()
{
	if (mode == Mode::preview)return 1;
	else if (mode == Mode::renderPathTracing)return 1280;
	else if (mode == Mode::renderPM)return 1280;
	else return 0;
}

void Render::renderPass()
{
	if (mode == Mode::renderPathTracing) {
		render(&tracer);
	}
	else if(mode == Mode::preview){
		preview();
	}
	else {
		render(&sppm);
	}
}

void Render::initLogger()
{
	logger = new WindowLogger(window);
}

Render::~Render()
{
}

void Render::copyFiltered(QPixmap & pixmap)
{
	vec3 *temp = new vec3[w*h];
	concurrency::parallel_for((size_t)0, screen.size(), [&](unsigned int idx) {
		temp[idx] = screen[idx];
	});
	NonLocalMeanFilter f;
	f.filter(temp, w, h);
	pixel.resize(w*h * 4);
	concurrency::parallel_for((size_t)0, screen.size(), [&](unsigned int idx) {
		auto & i = temp[idx];
		pixel[idx * 4] = (static_cast<unsigned char>(clamp(i.x(), 0, 1) * 255));
		pixel[idx * 4 + 1] = (static_cast<unsigned char>(clamp(i.y(), 0, 1) * 255));
		pixel[idx * 4 + 2] = (static_cast<unsigned char>(clamp(i.z(), 0, 1) * 255));
		pixel[idx * 4 + 3] = (255);
	});
	QImage image(pixel.data(), w, h, QImage::Format::Format_RGBA8888);
	pixmap.convertFromImage(image);
	delete[] temp;
}

void Render::copyImage(QPixmap & pixmap)
{
	pixel.resize(w*h*4);
	concurrency::parallel_for((size_t)0,screen.size(),[&](unsigned int idx) {
		auto & i = screen[idx];
		pixel[idx * 4] = (static_cast<unsigned char>(clamp(i.x(), 0, 1) * 255));
		pixel[idx * 4  + 1] = (static_cast<unsigned char>(clamp(i.y(), 0, 1) * 255));
		pixel[idx * 4  + 2] = (static_cast<unsigned char>(clamp(i.z(), 0, 1) * 255));
		pixel[idx * 4  + 3] = (255);
	});
	QImage image(pixel.data(), w, h, QImage::Format::Format_RGBA8888);
	pixmap.convertFromImage(image);
}
std::string format3digits(int x) {
	if (x >= 100) {
		return fmt::format("{}",x);
	}
	else if (x >= 10) {
		return fmt::format("0{}", x);
	}
	else {
		return fmt::format("00{}", x);
	}
}
void Render::interactiveRender()
{
	while (true) {
		checkReset();
		while (sampleCount < maxInteractiveRenderSample()) {
			auto start = std::chrono::system_clock::now();
			checkReset();
			renderPass();
			signal.hasNew = true;
			auto end = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsed_seconds = end - start;
			int rate = (w*h) / elapsed_seconds.count();
			std::string msg;
			if (rate > 1000000) {
				msg = fmt::format("Iteration: {} {}, {}, {} Samples/sec\r",
					sampleCount,
					rate / 1000000,
					format3digits((rate % 1000000) / 1000), 
					format3digits((rate % 1000)));
			}
			else {
				msg = fmt::format("Iteration: {} {}, {} Samples/sec\r",
					sampleCount,
					format3digits((rate % 1000000) / 1000),
					format3digits((rate % 1000)));
			}
			window->ui.status->setText(QString::fromStdString(msg));
			if (option.sleepTime > 0)
				std::this_thread::sleep_for(std::chrono::milliseconds(option.sleepTime));
			window->updateImage();
			do {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
			while (window->saving);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}
