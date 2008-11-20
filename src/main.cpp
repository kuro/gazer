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

    QStringList file_list;
    for (int i = 1; i < argc; i++) {
        file_list << argv[i];
    }
    win.set_file_list(file_list);

    return app.exec();
}/*}}}*/

// vim: sw=4 fdm=marker
