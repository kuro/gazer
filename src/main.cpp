
/**
 * @file main.cpp
 * @brief main implementation
 */

/* includes {{{*/
#include "MainWindow.h"

#include <QApplication>
#include <apr_getopt.h>
/*}}}*/

int main (int argc, char **argv)/*{{{*/
{
    QApplication app (argc, argv);

    apr_initialize();
    atexit(apr_terminate);

    MainWindow win;
    win.show();

    return app.exec();
}/*}}}*/

// vim: sw=4 fdm=marker
