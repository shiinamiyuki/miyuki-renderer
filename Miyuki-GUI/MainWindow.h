#pragma once
#include "MiyukiGUI.h"
#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"
#include "Render.h"
#include "WindowLogger.h"
class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	friend class Render;
	MainWindow(QWidget *parent = Q_NULLPTR);
	void log(const QString&s) {
		ui.log->append(s);
	}
	void log(const std::string&s) {
		ui.log->append(QString::fromStdString(s));
	}
	void log(const char * s) {
		ui.log->append(QString::fromLatin1(s));
	}
	void start();
private:
	Ui::MainWindowClass ui;
	volatile bool saving;
	QPixmap image;
	Render *render;
	void resizeEvent(QResizeEvent*e)override;
	void timerEvent(QTimerEvent*e)override;
	void save();
	void switchMode();
	void copyToClipboard();
	void updateImage();
	void startInteractive();
	void startFinal();
	void stop();
	void usePT();
	void useRT();
	void useSPPM();
public:
	Render* getRender()const { return render; }
};
