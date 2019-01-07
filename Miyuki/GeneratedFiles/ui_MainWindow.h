/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.11.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>
#include "RenderView.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindowClass
{
public:
    QAction *actionTo_Clipboard;
    QAction *actionPath_tracing;
    QAction *actionBDPT;
    QAction *actionAmbient_Occlusion;
    QAction *actionStart_Interactive_Rendering;
    QAction *actionStart_Final_Rendering;
    QAction *actionStop_Rendering;
    QAction *actionImport;
    QAction *actionModel_View;
    QAction *actionExperimental;
    QWidget *centralWidget;
    RenderView *renderView;
    QTextEdit *log;
    QLabel *status;
    QPushButton *capture;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuAction;
    QMenu *menuRender;
    QMenu *menuIntegrator;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindowClass)
    {
        if (MainWindowClass->objectName().isEmpty())
            MainWindowClass->setObjectName(QStringLiteral("MainWindowClass"));
        MainWindowClass->resize(587, 400);
        QPalette palette;
        QBrush brush(QColor(255, 255, 255, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
        palette.setBrush(QPalette::Active, QPalette::Text, brush);
        QBrush brush1(QColor(59, 59, 59, 255));
        brush1.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Base, brush1);
        QBrush brush2(QColor(60, 60, 60, 255));
        brush2.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Window, brush2);
        palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Text, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Base, brush1);
        palette.setBrush(QPalette::Inactive, QPalette::Window, brush2);
        QBrush brush3(QColor(120, 120, 120, 255));
        brush3.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Disabled, QPalette::WindowText, brush3);
        palette.setBrush(QPalette::Disabled, QPalette::Text, brush3);
        palette.setBrush(QPalette::Disabled, QPalette::Base, brush2);
        palette.setBrush(QPalette::Disabled, QPalette::Window, brush2);
        MainWindowClass->setPalette(palette);
        MainWindowClass->setWindowTitle(QStringLiteral("Miyuki Renderer"));
        actionTo_Clipboard = new QAction(MainWindowClass);
        actionTo_Clipboard->setObjectName(QStringLiteral("actionTo_Clipboard"));
        actionPath_tracing = new QAction(MainWindowClass);
        actionPath_tracing->setObjectName(QStringLiteral("actionPath_tracing"));
        actionBDPT = new QAction(MainWindowClass);
        actionBDPT->setObjectName(QStringLiteral("actionBDPT"));
        actionAmbient_Occlusion = new QAction(MainWindowClass);
        actionAmbient_Occlusion->setObjectName(QStringLiteral("actionAmbient_Occlusion"));
        actionStart_Interactive_Rendering = new QAction(MainWindowClass);
        actionStart_Interactive_Rendering->setObjectName(QStringLiteral("actionStart_Interactive_Rendering"));
        actionStart_Final_Rendering = new QAction(MainWindowClass);
        actionStart_Final_Rendering->setObjectName(QStringLiteral("actionStart_Final_Rendering"));
        actionStop_Rendering = new QAction(MainWindowClass);
        actionStop_Rendering->setObjectName(QStringLiteral("actionStop_Rendering"));
        actionImport = new QAction(MainWindowClass);
        actionImport->setObjectName(QStringLiteral("actionImport"));
        actionModel_View = new QAction(MainWindowClass);
        actionModel_View->setObjectName(QStringLiteral("actionModel_View"));
        actionExperimental = new QAction(MainWindowClass);
        actionExperimental->setObjectName(QStringLiteral("actionExperimental"));
        centralWidget = new QWidget(MainWindowClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        QPalette palette1;
        QBrush brush4(QColor(241, 241, 241, 255));
        brush4.setStyle(Qt::SolidPattern);
        palette1.setBrush(QPalette::Active, QPalette::ButtonText, brush4);
        palette1.setBrush(QPalette::Inactive, QPalette::ButtonText, brush4);
        palette1.setBrush(QPalette::Disabled, QPalette::ButtonText, brush3);
        centralWidget->setPalette(palette1);
        renderView = new RenderView(centralWidget);
        renderView->setObjectName(QStringLiteral("renderView"));
        renderView->setGeometry(QRect(200, 30, 401, 331));
        QPalette palette2;
        QBrush brush5(QColor(48, 48, 48, 255));
        brush5.setStyle(Qt::SolidPattern);
        palette2.setBrush(QPalette::Active, QPalette::Button, brush5);
        palette2.setBrush(QPalette::Inactive, QPalette::Button, brush5);
        palette2.setBrush(QPalette::Disabled, QPalette::Button, brush5);
        renderView->setPalette(palette2);
        renderView->setFocusPolicy(Qt::StrongFocus);
        log = new QTextEdit(centralWidget);
        log->setObjectName(QStringLiteral("log"));
        log->setEnabled(true);
        log->setGeometry(QRect(0, 30, 191, 351));
        log->setFocusPolicy(Qt::ClickFocus);
        status = new QLabel(centralWidget);
        status->setObjectName(QStringLiteral("status"));
        status->setGeometry(QRect(10, 0, 311, 21));
        capture = new QPushButton(centralWidget);
        capture->setObjectName(QStringLiteral("capture"));
        capture->setGeometry(QRect(480, 0, 121, 31));
        QPalette palette3;
        palette3.setBrush(QPalette::Active, QPalette::WindowText, brush);
        QBrush brush6(QColor(52, 52, 52, 255));
        brush6.setStyle(Qt::SolidPattern);
        palette3.setBrush(QPalette::Active, QPalette::Button, brush6);
        palette3.setBrush(QPalette::Active, QPalette::Dark, brush1);
        palette3.setBrush(QPalette::Active, QPalette::Text, brush);
        palette3.setBrush(QPalette::Active, QPalette::ButtonText, brush4);
        QBrush brush7(QColor(55, 55, 55, 255));
        brush7.setStyle(Qt::SolidPattern);
        palette3.setBrush(QPalette::Active, QPalette::Base, brush7);
        palette3.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
        palette3.setBrush(QPalette::Inactive, QPalette::Button, brush6);
        palette3.setBrush(QPalette::Inactive, QPalette::Dark, brush1);
        palette3.setBrush(QPalette::Inactive, QPalette::Text, brush);
        palette3.setBrush(QPalette::Inactive, QPalette::ButtonText, brush4);
        palette3.setBrush(QPalette::Inactive, QPalette::Base, brush7);
        palette3.setBrush(QPalette::Disabled, QPalette::WindowText, brush1);
        palette3.setBrush(QPalette::Disabled, QPalette::Button, brush6);
        palette3.setBrush(QPalette::Disabled, QPalette::Dark, brush1);
        palette3.setBrush(QPalette::Disabled, QPalette::Text, brush1);
        palette3.setBrush(QPalette::Disabled, QPalette::ButtonText, brush1);
        palette3.setBrush(QPalette::Disabled, QPalette::Base, brush2);
        capture->setPalette(palette3);
        capture->setAutoFillBackground(true);
        capture->setStyleSheet(QStringLiteral(""));
        MainWindowClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindowClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 587, 18));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        menuAction = new QMenu(menuBar);
        menuAction->setObjectName(QStringLiteral("menuAction"));
        menuRender = new QMenu(menuBar);
        menuRender->setObjectName(QStringLiteral("menuRender"));
        menuIntegrator = new QMenu(menuBar);
        menuIntegrator->setObjectName(QStringLiteral("menuIntegrator"));
        MainWindowClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindowClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindowClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindowClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindowClass->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuAction->menuAction());
        menuBar->addAction(menuRender->menuAction());
        menuBar->addAction(menuIntegrator->menuAction());
        menuFile->addAction(actionImport);
        menuAction->addAction(actionTo_Clipboard);
        menuRender->addAction(actionStart_Interactive_Rendering);
        menuRender->addAction(actionStart_Final_Rendering);
        menuRender->addAction(actionStop_Rendering);
        menuIntegrator->addAction(actionModel_View);
        menuIntegrator->addAction(actionPath_tracing);
        menuIntegrator->addAction(actionAmbient_Occlusion);
        menuIntegrator->addAction(actionBDPT);

        retranslateUi(MainWindowClass);

        QMetaObject::connectSlotsByName(MainWindowClass);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindowClass)
    {
        actionTo_Clipboard->setText(QApplication::translate("MainWindowClass", "To Clipboard", nullptr));
        actionPath_tracing->setText(QApplication::translate("MainWindowClass", "Path tracing", nullptr));
        actionBDPT->setText(QApplication::translate("MainWindowClass", "BDPT", nullptr));
        actionAmbient_Occlusion->setText(QApplication::translate("MainWindowClass", "Ambient Occlusion", nullptr));
        actionStart_Interactive_Rendering->setText(QApplication::translate("MainWindowClass", "Start Interactive Rendering", nullptr));
        actionStart_Final_Rendering->setText(QApplication::translate("MainWindowClass", "Start Final Rendering", nullptr));
        actionStop_Rendering->setText(QApplication::translate("MainWindowClass", "Stop Rendering", nullptr));
        actionImport->setText(QApplication::translate("MainWindowClass", "Import", nullptr));
        actionModel_View->setText(QApplication::translate("MainWindowClass", "Model View", nullptr));
        actionExperimental->setText(QApplication::translate("MainWindowClass", "Experimental", nullptr));
        renderView->setText(QString());
        status->setText(QString());
        capture->setText(QApplication::translate("MainWindowClass", "Capture", nullptr));
        menuFile->setTitle(QApplication::translate("MainWindowClass", "File", nullptr));
        menuAction->setTitle(QApplication::translate("MainWindowClass", "Action", nullptr));
        menuRender->setTitle(QApplication::translate("MainWindowClass", "Render", nullptr));
        menuIntegrator->setTitle(QApplication::translate("MainWindowClass", "Integrator", nullptr));
        Q_UNUSED(MainWindowClass);
    } // retranslateUi

};

namespace Ui {
    class MainWindowClass: public Ui_MainWindowClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
