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

#include <assert.h>

#include <QtCore>
#include <QAction>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QApplication>
#include <QDockWidget>
#include <QTreeWidget>
/*}}}*/

MainWindow::MainWindow() :/*{{{*/
    QMainWindow(),
    settings("MentalDistortion", "Gazer"),
    open_action(NULL),
    quit_action(NULL),
    menu_bar(NULL),
    surface(NULL),
    file_index(-1)
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

    QDockWidget* list_dock = new QDockWidget(this);
    addDockWidget(Qt::LeftDockWidgetArea, list_dock);
    tree_widget = new QTreeWidget();
    list_dock->setWidget(tree_widget);
    tree_widget->setHeaderLabels(QStringList("File"));

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(about_to_quit()));
    connect(
        tree_widget,
        SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
        this, SLOT(current_item_changed(QTreeWidgetItem*,QTreeWidgetItem*)));
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

    QFileInfo info (fname);
    QDir dir (info.absolutePath());
    QDir::setCurrent(dir.absolutePath());
    QStringList list = dir.entryList(QDir::Files, QDir::Name);
    set_file_list(list, list.indexOf(info.fileName()));
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

void MainWindow::set_file_list(const QStringList& list, int new_index)/*{{{*/
{
    this->file_list.clear();
    tree_widget->clear();
    this->file_list = list;

    if (list.size() == 1) {
        QFileInfo finfo (list.first());
        if (finfo.isDir()) {
            QDir dir (list.first());
            QDir::setCurrent(dir.absolutePath());
            set_file_list(dir.entryList(QDir::Files, QDir::Name));
            return;
        }
    }

    QList<QTreeWidgetItem*> items;
    foreach (QString f, file_list) {
        items.append(new QTreeWidgetItem(tree_widget, QStringList(f)));
    }
    tree_widget->insertTopLevelItems(0, items);

    go(new_index);
}/*}}}*/

void MainWindow::current_item_changed (QTreeWidgetItem* item,QTreeWidgetItem* prev)/*{{{*/
{
    if (item == NULL) {
        return;
    }
    file_index = file_list.indexOf(item->data(0, Qt::DisplayRole).toString());
    assert(file_index >= 0);
    assert(file_index < file_list.size());
    surface->load_image(file_list[file_index]);
}/*}}}*/

void MainWindow::next (int direction)/*{{{*/
{
    if (file_list.isEmpty()) {
        file_index = -1;
        return;
    }
    file_index += direction;
    file_index = file_index < 0 ?
        file_index + file_list.size() : file_index % file_list.size();
    tree_widget->setCurrentItem(tree_widget->topLevelItem(file_index));
}/*}}}*/
void MainWindow::go (int index)/*{{{*/
{
    if (file_list.isEmpty()) {
        file_index = -1;
        return;
    }
    file_index = index;
    file_index = file_index < 0 ?
        file_index + file_list.size() : file_index % file_list.size();
    tree_widget->setCurrentItem(tree_widget->topLevelItem(file_index));
}/*}}}*/

// vim: sw=4 fdm=marker
