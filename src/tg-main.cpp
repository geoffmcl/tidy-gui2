/*\
 * tg-main.cpp
 *
 * Copyright (c) 2015 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
\*/

#include <QApplication>
#include <QMessageBox>
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

PINFOSTR m_pinfo;

static const char *help =
    APP_NAME " - " APP_VERSION " - Help Commands\n"
    "Usage: tidy-gui2 [options] [inputFile]\n"
    "Options:\n"
    " -h or -? This help and exit(2)\n"
    " -o outputFile\n"
    " -c configFile\n"
    " -f errorFile\n"
    " This will only set the items, but not perform any action\n";

int parseArgs( PINFOSTR pinfo, int argc, char **argv )
{
    int i, i2, c, iret;
    char *arg, *sarg;
    iret = 0;
    for (i = 1; i < argc; i++) {
        arg = argv[i];
        i2 = i + 1;
        if (*arg == '-') {
            sarg = &arg[1];
            while (*sarg == '-')
                sarg++;
            c = *sarg;
            switch (c)
            {
            case 'h':
            case '?':
                pinfo->errorStr = "Request for help\n";
                iret = 2;
                goto exit;
            case 'o':
                if (i2 < argc) {
                    i++;
                    pinfo->outputStr = arg[i];
                } else {
                    pinfo->errorStr = QString("Expected file name to follow %1\n").arg(arg);
                    iret = 1;
                    goto exit;
                }
                break;
            case 'c':
                if (i2 < argc) {
                    i++;
                    pinfo->configStr = arg[i];
                } else {
                    pinfo->errorStr = QString("Expected file name to follow %1\n").arg(arg);
                    iret = 1;
                    goto exit;
                }
                break;
                break;
            case 'f':
                if (i2 < argc) {
                    i++;
                    pinfo->errorStr = arg[i];
                } else {
                    pinfo->errorStr = QString("Expected file name to follow %1\n").arg(arg);
                    iret = 1;
                    goto exit;
                }
                break;
                break;
            default:
                break;
            }

        } else {
            pinfo->inputStr = arg; // For now ASSUME any bear command is the input file
        }
    }
exit:
    if (iret) {
        pinfo->errorStr.append(help);
    }
    return iret;
}

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

    m_pinfo = new INFOSTR;

    iret = parseArgs( m_pinfo, argc, argv );
    if (iret) {
        QMessageBox::warning(NULL, "Command Line Error!",
            m_pinfo->errorStr,QMessageBox::Ok);
        goto exit;
    }

    m_tabDialog = new TabDialog(m_pinfo);
#ifdef Q_OS_SYMBIAN
    m_tabDialog->showMaximized();
#else
    m_tabDialog->show();
#endif

    iret = app.exec();

exit:
    if (m_tabDialog)
        delete m_tabDialog;
    m_tabDialog = 0;
    if (m_pinfo)
        delete m_pinfo;
    m_pinfo = 0;
    closeTidyLib();
    return iret;
}
