/*
 * Copyright 2008 Blanton Black
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file MainWindow.cpp
 * @brief MainWindow implementation
 */

/* includes {{{*/
#include "MainWindow.moc"
#include "GLSurface.h"

#include <QtCore>
#include <QAction>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QApplication>
/*}}}*/

MainWindow::MainWindow() :/*{{{*/
    QMainWindow(),
    settings("MentalDistortion", "Gazer"),
    open_action(NULL),
    quit_action(NULL),
    menu_bar(NULL),
    surface(NULL)
{
    settings.beginGroup("MainWindow");
    resize(settings.value("size", QSize(400, 400)).toSize());
    move(settings.value("pos", QPoint(200, 200)).toPoint());
    settings.endGroup();

    create_actions();
    create_menus();

    surface = new GLSurface();
    setCentralWidget(surface);

    statusBar()->showMessage("Ready");

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(about_to_quit()));
}/*}}}*/
MainWindow::~MainWindow()/*{{{*/
{
}/*}}}*/

void MainWindow::create_actions(void)/*{{{*/
{
    open_action = new QAction("&Open", this);
    open_action->setShortcut(tr("Ctrl+O"));
    open_action->setStatusTip("Open file");
    connect(open_action, SIGNAL(triggered()), this, SLOT(open()));

    quit_action = new QAction("&Quit", this);
    quit_action->setShortcut(tr("Ctrl+Q"));
    quit_action->setStatusTip("Quit application");
    connect(quit_action, SIGNAL(triggered()), this, SLOT(quit()));
}/*}}}*/
void MainWindow::create_menus(void)/*{{{*/
{
    menu_bar = new QMenuBar(this);
    setMenuBar(menu_bar);

    file_menu = menu_bar->addMenu("&File");
    file_menu->addAction(open_action);
    file_menu->addSeparator();
    file_menu->addAction(quit_action);
}/*}}}*/

void MainWindow::open (void)/*{{{*/
{
    QString fname = QFileDialog::getOpenFileName(this, "Open Media");
    if (fname.isEmpty()) {
        return;
    }
    try {
        surface->load_image(fname);
        setWindowTitle(fname.replace(QRegExp(".*/"), "").append(" - Gazer"));
    }
    catch (...) {
        statusBar()->showMessage("Error");
    }
}/*}}}*/
void MainWindow::quit (void)/*{{{*/
{
    qApp->quit();
}/*}}}*/

void MainWindow::about_to_quit (void)/*{{{*/
{
    settings.beginGroup("MainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();

    qApp->quit();
}/*}}}*/

// vim: sw=4 fdm=marker
