
/**
 * @file GazerWindow.h
 * @brief GazerWindow definition
 */

#pragma once

#include <QMainWindow>
#include <QSettings>

class QAction;
class GLSurface;

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:

    QSettings settings;

    QAction *open_action;
    QAction *quit_action;

    QMenuBar *menu_bar;
    QMenu *file_menu;

    GLSurface* surface;

public:
    MainWindow();
    virtual ~MainWindow();

private:
    void create_actions(void);
    void create_menus(void);

private slots:
    void open ();
    void quit ();
    void about_to_quit ();
};

// vim: sw=4 fdm=marker
