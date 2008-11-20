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
 * @file MainWindow.h
 * @brief MainWindow definition
 */

#pragma once

#include <QMainWindow>
#include <QSettings>

class QAction;
class QTreeWidget;
class QTreeWidgetItem;
class GLSurface;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    friend class GLSurface;

private:

    QSettings settings;

    QAction *open_action;
    QAction *quit_action;

    QMenuBar *menu_bar;
    QMenu *file_menu;

    GLSurface* surface;

    QTreeWidget* tree_widget;
    QStringList file_list;
    int file_index;

public:
    MainWindow();
    virtual ~MainWindow();

    void set_file_list(const QStringList& list, int new_index = 0);
    void go (int index);
    void next (int direction = 1);

private:
    void create_actions(void);
    void create_menus(void);

private slots:
    void open ();
    void quit ();
    void about_to_quit ();

    void current_item_changed (QTreeWidgetItem* item, QTreeWidgetItem* prev);
};

// vim: sw=4 fdm=marker
