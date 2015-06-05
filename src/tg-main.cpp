/*\
 * tg-main.cpp
 *
 * Copyright (c) 2015 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
\*/

#include <QApplication>
#include "tg-dialog.h"
#include "tg-config.h"

#ifndef APP_SETD
#define APP_SETD "geoffair.org"
#endif
#ifndef APP_SETO
#define APP_SETO "geoffair"
#endif

#ifndef APP_SETN
#define APP_SETN "tidygui2"
#endif
#ifndef APP_SETV
#define APP_SETV "0.0.1"
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// INI File location:
// In Windows: C:\Users\<user>\AppData\Local\geoffair\tidygui2\TidyGUI2.ini
//          or %LOCALAPPDATA%\geoffair\tidygui2\TidyGUI2.ini
// In Linux:   $HOME/.local/share/data/geoffair/tidygui2/TidyGUI2.ini
//////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
    int iret = 0;

    QApplication app(argc, argv);

    QApplication::setOrganizationDomain(APP_SETD);

    // store the UNIX file as $HOME/.config/geoffair/Qt_OSM_Map.conf - nav: . hconf; cd geoffair
    QApplication::setOrganizationName(APP_SETO);
    QApplication::setApplicationName(APP_SETN);
    QApplication::setApplicationVersion(APP_SETV);

    openTidyLib();

    QString fileName;

    if (argc >= 2) {
        fileName = argv[1]; // TODO: For now ASSUME any command is the input file
    }

    TabDialog tabdialog(fileName);
#ifdef Q_OS_SYMBIAN
    tabdialog.showMaximized();
#else
    tabdialog.show();
#endif

    iret = app.exec();
    closeTidyLib();
    return iret;
}
