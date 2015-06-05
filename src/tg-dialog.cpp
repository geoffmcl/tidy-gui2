/*\
 * tg-dialog.cpp
 *
 * Copyright (c) 2015 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
\*/

#include <QtGui>
#include <QXmlSimpleReader>
#include <QIODevice>
#include <QWidget>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QLabel>
#include <QFrame>
#include <QPlainTextEdit>
#include <QTimer>
#include <QDateTime>
#include <QString>
#include <QSettings>
#include <QDir>
#include <QFile>
#include <QDesktopServices>
#include <QCloseEvent>
#include <QMessageBox>
#include <QStringList>

#include "tg-dialog.h"
#include "tg-config.h"

// #define USE_HTML_EDITOR
#ifndef APP_NAME
#define APP_NAME "Tidy GUI2"
#endif
#ifndef APP_VERSION
#define APP_VERSION "4.9.30"
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// INI File location:
// In Windows: C:\Users\<user>\AppData\Local\geoffair\tidygui2\TidyGUI2.ini
//          or %LOCALAPPDATA%\geoffair\tidygui2\TidyGUI2.ini
// In Linux:   $HOME/.local/share/data/geoffair/tidygui2/TidyGUI2.ini
//////////////////////////////////////////////////////////////////////////////////////////
#ifndef DEF_INI_FILE
#define DEF_INI_FILE    "TidyGUI2.ini"
#endif
#ifndef DEF_CFG_FILE
#define DEF_CFG_FILE    "tg2default.cfg"    // save and restore the last config
#endif

// Order of TAB PAGES
#define M_GEN_TAB  0
#define M_CFG_TAB  1
#define M_OUT_TAB  2
#define M_DIAG_TAB 3
#define M_ENC_TAB  4
#define M_MARK_TAB 5
#define M_MISC_TAB 6
#define M_PRT_TAB  7

#define lcfo_silent 1
#define lcfo_noerr  2

#define S_INPUT "inputfile"
#define S_LASTTAB "lasttab"
#define S_CONFIG "configfile"
#define S_OUTPUT "outputfile"

// single sole 'settings'
QSettings *m_settings;    // = new QSettings(tmp,QSettings::IniFormat,this);

PINFOSTR m_pinfo;

QPushButton *buttonTidy;
#ifdef USE_HTML_EDITOR  // declare editor variable
QTextEdit *errEditor = 0;
QTextEdit *bigEditor = 0;
#else
QPlainTextEdit *errEditor = 0;
QPlainTextEdit *bigEditor = 0;
#endif
QTextEdit *cfgEditor;

QLineEdit *fileNameEdit;
QToolButton *fileNameBrowse;

QLineEdit *outputNameEdit;
QToolButton *outputNameBrowse;

QLineEdit *configNameEdit;
QToolButton *configNameBrowse;

QLineEdit *outNameEdit = 0;

void set_outNameEdit(QString msg)
{
    if (outNameEdit)
        outNameEdit->setText(msg);
}

// check if file exists and if yes: Is it really a file and not a directory?
bool m_fileExists(QString path) 
{
    QFileInfo checkFile(path);
    if (path.size() && checkFile.exists() && checkFile.isFile()) {
        return true;
    }
    return false;
}

void set_bigEdit( const char *text )
{
    if (bigEditor) {
#ifdef USE_HTML_EDITOR  // set_bigEdit - set the text setText or setPlainText
        bigEditor->setText(text);
#else
        bigEditor->setPlainText(text);
#endif
    }
}

void append_bigEdit( const char *text )
{
    if (bigEditor) {
#ifdef USE_HTML_EDITOR  // append_bigEdit - append or appendPlainText
        bigEditor->append(text);
#else
        bigEditor->appendPlainText(text);
#endif
    }
}

void set_errEdit( const char *text )
{
    if (errEditor) {
#ifdef USE_HTML_EDITOR  // set_errEdit - errEditor - setText or setPlainText
        errEditor->setText(text);
#else
        errEditor->setPlainText(text);
#endif
    }
}

void append_errEdit( const char *text )
{
    if (errEditor) {
#ifdef USE_HTML_EDITOR  // append_errEdit - errEditor - append ot appendPlainText
        errEditor->append(text);
#else
        errEditor->appendPlainText(text);
#endif
    }
}

// maybe really do not need these, but...
static GeneralTab *GeneralTabPtr = 0;
static DiagnosticsTab *DiagnosticsTabPtr = 0;
static EncodingTab *EncodingTabPtr = 0;
static MarkupTab *MarkupTabPtr = 0;
static MiscTab *MiscTabPtr = 0;
static PrintTab *PrintTabPtr = 0;
static OutputTab *OutputTabPtr = 0;
static ConfigTab *ConfigTabPtr = 0;

static QString cfg_file;

//////////////////////////////////////////////////////////////////////////////////////////
// INI File location:
// In Windows: C:\Users\<user>\AppData\Local\geoffair\tidygui2\TidyGUI2.ini
//          or %LOCALAPPDATA%\geoffair\tidygui2\TidyGUI2.ini
// In Linux:   $HOME/.local/share/data/geoffair/tidygui2/TidyGUI2.ini
//////////////////////////////////////////////////////////////////////////////////////////
TabDialog::TabDialog(const QString &fileName, QWidget *parent)
    : QDialog(parent)
{
    // QFileInfo fileInfo(fileName);
    m_pinfo = new INFOSTR;
    if (fileName.size() && m_fileExists(fileName)) {
        m_pinfo->input = fileName;
    }

    QString tmp = QDir(QDesktopServices::storageLocation(
                    QDesktopServices::DataLocation)).absolutePath();
    // create path is not exist
    if(!QFile::exists(tmp)){
        QDir dir;
        dir.mkpath(tmp);
    }
    tmp.append("/");
    cfg_file = tmp;
    cfg_file.append(DEF_CFG_FILE);

    tmp.append(DEF_INI_FILE);
    m_settings = new QSettings(tmp,QSettings::IniFormat,this);

    restoreGeometry(m_settings->value("mainWindowGeometry").toByteArray());

    ////////////////////////////////////////////////////////////////////////
    buttonBox = new QDialogButtonBox(this);

	QPushButton *buttonQuit = new QPushButton();
    // buttonQuit->setIcon(QIcon(":/icons/black"));
	buttonQuit->setText("Quit");

    buttonTidy = new QPushButton();
	buttonTidy->setText("Tidy!");
    if (!m_fileExists(fileName)) {
        buttonTidy->setEnabled(false);
    }

    //QPushButton *buttonShow = new QPushButton();
	//buttonShow->setText("Config");

    QPushButton *buttonAbout = new QPushButton();
	buttonAbout->setText("About");

	buttonBox->addButton(buttonQuit, QDialogButtonBox::ActionRole);
	buttonBox->addButton(buttonAbout, QDialogButtonBox::ActionRole);
	//buttonBox->addButton(buttonShow, QDialogButtonBox::ActionRole);
	buttonBox->addButton(buttonTidy, QDialogButtonBox::ActionRole);
	//buttonBox->addButton(buttonQuit,QDialogButtonBox::RejectRole);
    // TODO: Move buttons

    //buttonBox->setCenterButtons(true); // got it see below - prefer LEFT, but can could not find out how
    buttonBox->setLayoutDirection( Qt::RightToLeft );

	//connect(buttonQuit, SIGNAL(clicked()), this, SLOT(reject()));
	connect(buttonQuit, SIGNAL(clicked()), this, SLOT(onQuit()));
	//connect(buttonShow, SIGNAL(clicked()), this, SLOT(onShow()));
	connect(buttonTidy, SIGNAL(clicked()), this, SLOT(on_buttonTidy()));
	connect(buttonAbout, SIGNAL(clicked()), this, SLOT(on_about()));

    // set up the TABS
    tabWidget = new QTabWidget;

    GeneralTabPtr = new GeneralTab( m_pinfo );
    DiagnosticsTabPtr = new DiagnosticsTab( m_pinfo );
    EncodingTabPtr = new EncodingTab( m_pinfo );
    MarkupTabPtr = new MarkupTab( m_pinfo );
    MiscTabPtr = new MiscTab( m_pinfo );
    PrintTabPtr = new PrintTab( m_pinfo );
    OutputTabPtr = new OutputTab( m_pinfo );
    ConfigTabPtr = new ConfigTab( m_pinfo );

    // NOTE: Fix above "Order ot TAB PAGES" if order is changed
    tabWidget->addTab(GeneralTabPtr, tr("General"));
    tabWidget->addTab(ConfigTabPtr, tr("Config"));  // 20150601: try this in 2nd position
    tabWidget->addTab(OutputTabPtr, tr("Output"));  // 20150605: and this in 3rd position

    tabWidget->addTab(DiagnosticsTabPtr, tr("Diagnostics"));
    tabWidget->addTab(EncodingTabPtr, tr("Encoding"));
    tabWidget->addTab(MarkupTabPtr, tr("Markup"));
    tabWidget->addTab(MiscTabPtr, tr("Misc"));
    tabWidget->addTab(PrintTabPtr, tr("Print"));

    int index = m_settings->value(S_LASTTAB,"0").toInt();
    if ((index == M_CFG_TAB)||(index == M_OUT_TAB)) {
        index = M_GEN_TAB; // do not go to these BLANK pages on startup
    }
    tabWidget->setCurrentIndex(index); // Select Tab Here
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(on_tab_changed()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QLayout::SetNoConstraint);
    mainLayout->addWidget(buttonBox);
    mainLayout->addWidget(tabWidget);
    setLayout(mainLayout);
    setWindowTitle(tr(APP_NAME " - " APP_VERSION));
    setWindowIcon(QIcon(":/icon/tidyicon"));

    // load any default config file
    if (m_fileExists(cfg_file)) {
        ConfigTabPtr->loadConfig(cfg_file, lcfo_silent | lcfo_noerr); // silently restore config to saved default
    }
}

void TabDialog::closeEvent(QCloseEvent *event)
{
    m_settings->setValue("mainWindowGeometry", saveGeometry());
    //outLog(util_getDateTimestg()+" - Application close",0x8001);
    const char *ccp = get_all_options(false,false);
    if (strncmp("Oops! ", ccp, 6) == 0) {
        ccp = "";
    }
    ConfigTabPtr->saveConfig(cfg_file, ccp, lcfo_silent | lcfo_noerr ); // silently rest config to default saved
    event->accept();
}

void TabDialog::onQuit()
{
    //settings->setValue("mainWindowGeometry", saveGeometry());
    close();
}

void TabDialog::onShow()
{
    //showConfig();
}

static const char *about =
    "<h1>This is TidyGUI2 - v.2.0.0</h1>\n\n"
    "<p>This Qt GUI provides an interface to HTML Tidy library.</p>\n\n"
    "<p>HTML Tidy library and console app <a href=\"http://www.html-tidy.org/\">Home Page</a></p> \n\n"
    "<p>Full free open <a href=\"https://github.com/htacg/tidy-html5\"><b>source</b></a> available.</p>\n\n"
    "<p>HTML Tidy library is\n"
    "Copyright <a href=\"http://www.w3.org/\">World Wide Web Consortium</a>,<br>"
    "and this GUI uses <a href=\"http://www.qt.io/developers/\">Qt</a> runtime libaries</p>\n\n"
    "<p>Original based on Dave Raggett's HTML Tidy<br>\n"
    "but now uses <a href=\"https://www.w3.org/community/htacg/\">HTACG</a> Tidy, ver 5, circa June 2015</p>"
    "<p>HTML Tidy Original "
    "<a href=\"http://www.w3.org/People/Raggett/tidy\">Home</a> page<br> \n\n";

void TabDialog::on_about()
{
    QString msg = about;
    QMessageBox::about(this, tr("About Tidy GUI2"), msg);
}


void TabDialog::on_buttonTidy()
{
    // This is the REAL WORK 
    // How to pass the file to tidy
    QString file = fileNameEdit->text();
    file = file.trimmed();
    if (!m_fileExists(file)) {
        QString msg;
        if (file.size()) {
            msg = QString("Error: File '%1' does NOT exist!\nChoose another file.").arg(file);
        } else {
            msg = "Error: NO input file!\nChoose a new file.";
        }
        QMessageBox::warning(this, tr("File Not Found"),msg,QMessageBox::Ok);
        return;
    }

    QFileInfo fi(file);
    QString ff = fi.absoluteFilePath();
    if (file != ff) {
        fileNameEdit->setText(ff);
        file = ff;
    }

    m_settings->setValue( S_INPUT, file );  // save the last tidied file name
    runTidyLib( file.toStdString().c_str() );
    tabWidget->setCurrentIndex(M_OUT_TAB); // Select Tab Here

}

void TabDialog::on_tab_changed()
{
    int ind = tabWidget->currentIndex();
    m_settings->setValue( S_LASTTAB, ind );  // save the last tab index
}

static void check_me(QWidget *w)
{
    int i;
    i = 1;

}

////////////////////////////////////////////////////////////////
//// TABS

GeneralTab::GeneralTab( PINFOSTR pinf, QWidget *parent)
    : QWidget(parent)
{
    fileNameEdit = new QLineEdit("");
    fileNameBrowse = new QToolButton();
    fileNameBrowse->setToolTip("Browse for input file");
    fileNameBrowse->setIcon(QIcon(":/icon/open"));
    fileNameBrowse->setToolButtonStyle(Qt::ToolButtonIconOnly);

    outputNameEdit = new QLineEdit("");
    outputNameBrowse = new QToolButton();
    outputNameBrowse->setToolTip("Browse for output file");
    outputNameBrowse->setIcon(QIcon(":/icon/save"));
    outputNameBrowse->setToolButtonStyle(Qt::ToolButtonIconOnly);

    configNameEdit = new QLineEdit("");
    configNameBrowse = new QToolButton();
    configNameBrowse->setToolTip("Browse for config file");
    configNameBrowse->setIcon(QIcon(":/icon/save"));
    configNameBrowse->setToolButtonStyle(Qt::ToolButtonIconOnly);

#ifdef USE_HTML_EDITOR  // errEditor - new QTextEdit or QPlainTextEdit
    errEditor = new QTextEdit; // this display html
#else
    errEditor = new QPlainTextEdit;
#endif
    errEditor->setReadOnly(true);
    //bigEditor->setTextFormat(Qt::PlainText);

    QSizePolicy sp = errEditor->sizePolicy();
    sp.setVerticalStretch(1);
    errEditor->setSizePolicy(sp);

    QVBoxLayout *mainLayout = new QVBoxLayout;

    QGroupBox *inputfileGroup = new QGroupBox("Input File Name");
    QHBoxLayout *inputfileLay = new QHBoxLayout;

    inputfileLay->addWidget(fileNameEdit);
    inputfileLay->addWidget(fileNameBrowse);
    connect(fileNameEdit,SIGNAL(editingFinished()),this,SLOT(on_fileNameEdit()));
    connect(fileNameBrowse, SIGNAL(clicked()),this,SLOT(on_fileNameBrowse()));

    inputfileGroup->setLayout(inputfileLay);
    mainLayout->addWidget(inputfileGroup);

    QGroupBox *outputfileGroup = new QGroupBox("Output File Name");
    QHBoxLayout *outputfileLay = new QHBoxLayout;
    outputfileLay->addWidget(outputNameEdit);
    outputfileLay->addWidget(outputNameBrowse);
    connect(outputNameBrowse, SIGNAL(clicked()),this,SLOT(on_outputNameBrowse()));
    connect(outputNameEdit,SIGNAL(editingFinished()),this,SLOT(on_outputNameEdit()));

    outputfileGroup->setLayout(outputfileLay);
    mainLayout->addWidget(outputfileGroup);

    QGroupBox *configfileGroup = new QGroupBox("Config File Name");
    QHBoxLayout *configfileLay = new QHBoxLayout;
    configfileLay->addWidget(configNameEdit);
    configfileLay->addWidget(configNameBrowse);
    connect(configNameBrowse, SIGNAL(clicked()),this,SLOT(on_configNameBrowse()));
    connect(configNameEdit,SIGNAL(editingFinished()),this,SLOT(on_configNameEdit()));

    configfileGroup->setLayout(configfileLay);
    mainLayout->addWidget(configfileGroup);

    mainLayout->addWidget(errEditor);
    mainLayout->addStretch(1);
    setLayout(mainLayout);

    if (!pinf->input.size() || !m_fileExists( pinf->input )) {
        pinf->input = m_settings->value(S_INPUT,"").toString();
    }
    if (pinf->input.size() && m_fileExists( pinf->input )) {
        fileNameEdit->setText( pinf->input );
    }

    QString file;
    file = m_settings->value(S_OUTPUT,"").toString();
    outputNameEdit->setText(file);
    file = m_settings->value(S_CONFIG,"").toString();
    configNameEdit->setText(file);

    on_fileNameEdit();
    on_outputNameEdit();
    on_configNameEdit();

}

void GeneralTab::on_fileNameEdit()
{
    QString file = fileNameEdit->text();
    if (m_fileExists(file)) {
        buttonTidy->setEnabled(true);
        fileNameEdit->setStyleSheet("border: 1px solid green");
    } else {
        buttonTidy->setEnabled(false);
        fileNameEdit->setStyleSheet("border: 2px solid red");
    }
}

void GeneralTab::on_outputNameEdit()
{
    QString file = outputNameEdit->text();
    if (m_fileExists(file)) {
        outputNameEdit->setStyleSheet("border: 1px solid red"); // red - will overwrite
    } else {
        outputNameEdit->setStyleSheet("border: 1px solid green");
    }
}

void GeneralTab::on_configNameEdit()
{
    QString file = configNameEdit->text();
    if (m_fileExists(file)) {
        configNameEdit->setStyleSheet("border: 1px solid green");
    } else {
        configNameEdit->setStyleSheet("border: 1px solid red");
    }
}


// example multiple filter: "Images (*.png *.xpm *.jpg);;Text files (*.txt);;XML files (*.xml)"
static const char* filterSpec =
    "HTML Files (*.htm *.html);;"
	"XML Files (*.xml *.xsl);;"
    "All Files (*)";
static const char* filterSpec2 =
    "Cfg Files (*.cfg);;"
    "Text Files (*.txt);;"
    "All Files (*)";

void GeneralTab::on_fileNameBrowse()
{
    QString title = "Get input file name";
    QString filters = filterSpec;
    QString inputFile = QFileDialog::getOpenFileName(this, title, fileNameEdit->text(), filters);
    if(inputFile.length() > 0) {
        fileNameEdit->setText(inputFile);
        on_fileNameEdit();
    }
}

void GeneralTab::on_outputNameBrowse()
{
    QString title = "Choose output name";
    QString filters = filterSpec;
    QString outputFile = QFileDialog::getSaveFileName(this, title, outputNameEdit->text(), filters);
    if(outputFile.length() > 0) {
        outputNameEdit->setText(outputFile);
        m_settings->setValue( S_OUTPUT, outputFile );  // save the output file name
        on_outputNameEdit();
    }
}

void GeneralTab::on_configNameBrowse()
{
    QString title = "Choose config file";
    QString filters = filterSpec2;
    QString outputFile = QFileDialog::getSaveFileName(this, title, configNameEdit->text(), filters);
    if(outputFile.length() > 0) {
        configNameEdit->setText(outputFile);
        m_settings->setValue( S_CONFIG, outputFile );  // save the config file name
        on_configNameEdit();
    }
}


OutputTab::OutputTab( PINFOSTR pinf, QWidget *parent )
{

#ifdef USE_HTML_EDITOR  // OutputTab::OutputTab - bigEditor - new QTextEdit or QPlainTextEdit
    bigEditor = new QTextEdit; // this display html
#else
    bigEditor = new QPlainTextEdit;
#endif
    bigEditor->setReadOnly(true);

    // try to fiddle with the FONT
    QTextDocument *doc = bigEditor->document();
    if (doc) {
        QFont font = doc->defaultFont();
        font.setFamily("Courier New");
        font.setStyleHint(QFont::Monospace);
        font.setFixedPitch(true);
        font.setPointSize(10);    
        doc->setDefaultFont(font);
    }

    QVBoxLayout *outputLayout = new QVBoxLayout;
    // Buttons
    // button box
    QDialogButtonBox *outbuttonBox = new QDialogButtonBox(this);
    // button for button box
	QPushButton *butSaveAs = new QPushButton();
    // buttonQuit->setIcon(QIcon(":/icons/black"));
	butSaveAs->setText("Save As...");
    outbuttonBox->addButton(butSaveAs, QDialogButtonBox::ActionRole);

    outNameEdit = new QLineEdit("");

    outputLayout->addWidget(outbuttonBox);
    outputLayout->addWidget(outNameEdit);
    outputLayout->addWidget(bigEditor);
    setLayout(outputLayout);

    // connect up buttons
    connect(butSaveAs, SIGNAL(clicked()), this, SLOT(on_butSaveAs()));

}

void OutputTab::on_butSaveAs()
{
    QString text = bigEditor->toPlainText(); // get this save html
    if (text.size() == 0) {
        // TODO: Dialog no text to save
        return;
    }
    QString name = outputNameEdit->text();
    name = name.trimmed();
    QString title = "Choose output name";
    QString filters = filterSpec;
    QString outputFile = QFileDialog::getSaveFileName(this, title, name, filters);
    if(outputFile.length() > 0) {
        QFile file( outputFile );
        if ( file.open(QIODevice::ReadWrite |  QIODevice::Text) )
        {
            QTextStream stream( &file );
            stream << text;
            file.close();
            outputNameEdit->setText(outputFile);
            m_settings->setValue( S_OUTPUT, outputFile );  // save the output file name
        } else {
            // TODO: Dialog can NOT open file
        }

    }
}


#define S_DETAILED "show_detailed"
#define S_READONLY "read_only"
#define S_SHOWALL "show_all"

static QCheckBox *show_detailed;
static QCheckBox *read_only;
static QCheckBox *show_all;
ConfigTab::ConfigTab( PINFOSTR pinf, QWidget *parent )
{

    ////////////////////////////////////////////////////////////////
    // button box
    cfgbuttonBox = new QDialogButtonBox(this);
    // button for button box
	QPushButton *buttonSaveAs = new QPushButton();
    // buttonQuit->setIcon(QIcon(":/icons/black"));
	buttonSaveAs->setText("Save As...");
	QPushButton *buttonLoad = new QPushButton();
	buttonLoad->setText("Load...");
	QPushButton *buttonView = new QPushButton();
	buttonView->setText("View");
    // add button to the button box
    cfgbuttonBox->addButton(buttonSaveAs, QDialogButtonBox::ActionRole);
    cfgbuttonBox->addButton(buttonLoad, QDialogButtonBox::ActionRole);
    cfgbuttonBox->addButton(buttonView, QDialogButtonBox::ActionRole);
    // connect up buttons
    connect(buttonSaveAs, SIGNAL(clicked()), this, SLOT(on_buttonSaveAs()));
    connect(buttonLoad, SIGNAL(clicked()), this, SLOT(on_buttonLoad()));
    connect(buttonView, SIGNAL(clicked()), this, SLOT(on_buttonView()));

    ////////////////////////////////////////////////////////////////
    // editor to hold view
    cfgEditor = new QTextEdit;
    cfgEditor->setReadOnly(true);
    // fiddle with the FONT
    QTextDocument *doc = cfgEditor->document();
    if (doc) {
        QFont font = doc->defaultFont();
        font.setFamily("Courier New");
        font.setStyleHint(QFont::Monospace);
        font.setFixedPitch(true);
        font.setPointSize(10);    
        doc->setDefaultFont(font);
    }

    ////////////////////////////////////////////////////////////////
    // check boxes for options
    bool b;
    show_all = new QCheckBox("Show All");
    b = m_settings->value(S_SHOWALL,"0").toBool();
    show_all->setChecked(b);
    show_all->setToolTip("Show ALL config items");
    connect(show_all,SIGNAL(clicked()),this,SLOT(on_show_all()));

    show_detailed = new QCheckBox("Detailed");
    b = m_settings->value(S_DETAILED,"0").toBool();
    show_detailed->setChecked(b);
    show_detailed->setToolTip("Add type, if Show All");
    connect(show_detailed,SIGNAL(clicked()),this,SLOT(on_show_detailed()));

    // TODO: Not yet implmented allowing manual adjustment of shown config
    read_only = new QCheckBox("Read Only");
    b = m_settings->value(S_READONLY,"1").toBool();
    read_only->setChecked(b);
    read_only->setToolTip("NOT YET IMPLEMENTED");
    connect(read_only,SIGNAL(clicked()),this,SLOT(on_read_only()));

    QHBoxLayout *configButLay = new QHBoxLayout;
    configButLay->addWidget(show_detailed);
    configButLay->addWidget(read_only);
    configButLay->addWidget(show_all);
    //configButLay->addSpacerItem(1);
    //configButLay->addSpacing(1);
    configButLay->addStretch(1);    // do not space out - collect to left

    ////////////////////////////////////////////////////////////////
    // setup the page - vertical layout
    QVBoxLayout *configLayout = new QVBoxLayout;

    configLayout->addWidget(cfgbuttonBox);
    configLayout->addLayout(configButLay);
    configLayout->addWidget(cfgEditor);
    setLayout(configLayout);

}

void ConfigTab::on_show_all()
{
    bool b = show_all->isChecked();
    m_settings->setValue( S_SHOWALL, b );  // save the show_all checkbox
}

void ConfigTab::on_show_detailed()
{
    bool b = show_detailed->isChecked();
    m_settings->setValue( S_DETAILED, b );  // save the show_detailed checkbox
}

void ConfigTab::on_read_only()
{
    bool b = read_only->isChecked();
    m_settings->setValue( S_READONLY, b );  // save the read_only checkbox
}

bool ConfigTab::saveConfig(QString outputFile, const char *ccp, int options)
{
    bool done = false;
    bool show_err = (options & lcfo_silent) ? false : true;
    QFile file( outputFile );
    if ( file.open(QIODevice::ReadWrite|  QIODevice::Text) ) {
        QTextStream stream( &file );
        stream << ccp << endl;
        file.close();
        done = true;
    } else if (show_err) {
        QString msg = QString("Oops! Unable to open/create file %1!").arg(outputFile);
        QMessageBox::warning(this, tr("Open File FAILED!"),msg,QMessageBox::Ok);
    }
    return done;
}


void ConfigTab::on_buttonSaveAs()
{
    bool b = false; // show_detailed->isChecked();
    bool b2 = show_all->isChecked();
    const char *ccp = get_all_options(b2,b);
    cfgEditor->setText(ccp);
    QString msg;
    //////////////////////////////////////////////////////
    // what about if NONE
    if (!b2 && (strncmp("Oops! ", ccp, 6) == 0) ) {
        msg = ccp;
        msg.append("Really unable to save NOTHING!" MEOL);
        msg.append("Check 'Save All' if you want a FULL save" MEOL);
        QMessageBox::warning(this, tr("Nothing to SAVE!"),msg,QMessageBox::Ok);
        return;
    }
    QString title = "Get output config file name";
    QString filters = filterSpec2;
    QString outputFile = QFileDialog::getSaveFileName(this, title, configNameEdit->text(), filters);
    if(outputFile.length() > 0) {
        configNameEdit->setText(outputFile);
        // ok, we are going to write to a file...
        if (saveConfig(outputFile,ccp) )
            m_settings->setValue( S_CONFIG, outputFile );  // save the last tidied file name
    }
}

void ConfigTab::loadConfig(QString name, int options )
{
    int i;
    QString msg;
    bool show_err = (options & lcfo_silent) ? false : true;
    if (name.size() == 0) {
        if ( show_err ) {
            msg = QString("No config file given\nChoose new config file name!");
            QMessageBox::warning(this, tr("No Config file"),msg,QMessageBox::Ok);
        }
        return;
    }
    if (!m_fileExists(name)) {
        if ( show_err ) {
            msg = QString("Unable to 'stat' file '%1'!\nChoose new config file name!").arg(name);
            QMessageBox::warning(this, tr("File Not Found"),msg,QMessageBox::Ok);
        }
        return;
    }
    QFile file(name);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        if ( show_err ) {
            msg = QString("Unable to 'open' file '%1'!\nChoose new config file name!").arg(name);
            QMessageBox::warning(this, tr("File Not Found"),msg,QMessageBox::Ok);
        }
        return;
    }
    QTextStream ReadFile(&file);
    QString text = ReadFile.readAll();
    text = text.trimmed();
    file.close();

    if (text.size() == 0) {
        if ( show_err ) {
            msg = QString("File '%1' is BLANK!\nChoose new config file name!").arg(name);
            cfgEditor->setText(msg);
            QMessageBox::warning(this, tr("File is Empty"),msg,QMessageBox::Ok);
        }
        return;
    }

    if ( !(options & lcfo_noerr) ) {
        cfgEditor->setText(text);
    }

    // now get Tidy Library to parse CONFIG
    // tidyLoadConfig( tdoc, argv[2] );
    clearBuffers(1);    // release any previous error message
    i = load_config( name.toStdString().c_str() );
    if (i) {
        if ( show_err ) {
            QString s = QString::number(i);
            // what to do if an ERROR???
            msg = QString("File '%1' had an error %2!\nFix or maybe choose new config file name!").arg(name,s);
            //cfgEditor->setText(msg);
            char *cp = getErrBufText();
            if (cp) {
                s = cp;
                s = s.trimmed();
                msg.append( QString("\nErrs: %1!").arg(s) );
            }
            clearBuffers(1);    // release any error after shown
            cfgEditor->append(msg);
            QMessageBox::warning(this, tr("File is Empty"),msg,QMessageBox::Ok);
        }
    }

    do_configUpdate();

}

void ConfigTab::on_buttonLoad()
{
    QString name = configNameEdit->text();
    loadConfig(name);
}

void ConfigTab::on_buttonView()
{
    bool b = show_detailed->isChecked();
    bool b2 = show_all->isChecked();
    const char *ccp = get_all_options(b2,b);
    cfgEditor->setText(ccp);
}

/////////////////////////////////////////////////////////////////////////////////////
// **********************************************************************************
// *** WARNING *** Code below here is GENERATED by tidyxmlcfg.pl
// Any modification may be overwritten when the next generation is done
// And do NOT change the 2 'INSERT' lines, which act as edit markers!
// **********************************************************************************
////////////////////////////////////////////////////////////////////////////////////

// INSERT HERE //////////////////////////////////////////////////////////////////////

// Generated by tidyxmlcfg.pl 2015/06/01 12:42:02 UTC, 14:42:02 local

// code to be added

void DiagnosticsTab::on_show_info()
{
    const char *label = "show-info";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void DiagnosticsTab::on_show_warnings()
{
    const char *label = "show-warnings";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void DiagnosticsTab::on_show_errorsEd()
{
    const char *label = "show-errors";
    QLineEdit *w = qobject_cast<QLineEdit *>(sender());
    if (w) {
        QString s = w->text();
        int i = s.toInt();
        setConfigInt( label, i );
    }
}


// combobox slot change handler
void DiagnosticsTab::on_accessibility_checkComb()
{
    const char *label = "accessibility-check";
    QComboBox *w = qobject_cast<QComboBox *>(sender());
    if (w) {
        int i = w->currentIndex();
        QVariant qv = w->itemData(i);
        QString qs = qv.toString();
        setConfigEnum( label, qs.toStdString().c_str() );
    }
}


// combobox slot change handler
void EncodingTab::on_char_encodingComb()
{
    const char *label = "char-encoding";
    QComboBox *w = qobject_cast<QComboBox *>(sender());
    if (w) {
        int i = w->currentIndex();
        QVariant qv = w->itemData(i);
        QString qs = qv.toString();
        setConfigEnc( label, qs.toStdString().c_str() );
    }
}


// combobox slot change handler
void EncodingTab::on_input_encodingComb()
{
    const char *label = "input-encoding";
    QComboBox *w = qobject_cast<QComboBox *>(sender());
    if (w) {
        int i = w->currentIndex();
        QVariant qv = w->itemData(i);
        QString qs = qv.toString();
        setConfigEnc( label, qs.toStdString().c_str() );
    }
}


// combobox slot change handler
void EncodingTab::on_output_encodingComb()
{
    const char *label = "output-encoding";
    QComboBox *w = qobject_cast<QComboBox *>(sender());
    if (w) {
        int i = w->currentIndex();
        QVariant qv = w->itemData(i);
        QString qs = qv.toString();
        setConfigEnc( label, qs.toStdString().c_str() );
    }
}


// combobox slot change handler
void EncodingTab::on_newlineComb()
{
    const char *label = "newline";
    QComboBox *w = qobject_cast<QComboBox *>(sender());
    if (w) {
        int i = w->currentIndex();
        QVariant qv = w->itemData(i);
        QString qs = qv.toString();
        setConfigEnum( label, qs.toStdString().c_str() );
    }
}


// combobox slot change handler
void EncodingTab::on_doctypeComb()
{
    const char *label = "doctype";
    QComboBox *w = qobject_cast<QComboBox *>(sender());
    if (w) {
        int i = w->currentIndex();
        QVariant qv = w->itemData(i);
        QString qs = qv.toString();
        setConfigEnum( label, qs.toStdString().c_str() );
    }
}


// combobox slot change handler
void EncodingTab::on_repeated_attributesComb()
{
    const char *label = "repeated-attributes";
    QComboBox *w = qobject_cast<QComboBox *>(sender());
    if (w) {
        int i = w->currentIndex();
        QVariant qv = w->itemData(i);
        QString qs = qv.toString();
        setConfigEnum( label, qs.toStdString().c_str() );
    }
}


void EncodingTab::on_ascii_chars()
{
    const char *label = "ascii-chars";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void EncodingTab::on_languageEd()
{
    const char *label = "language";
    QLineEdit *w = qobject_cast<QLineEdit *>(sender());
    if (w) {
        QString s = w->text();
        s = s.trimmed();
        setConfigStg( label, s.toStdString().c_str() );
    }
}


void EncodingTab::on_output_bom()
{
    const char *label = "output-bom";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b;
        Qt::CheckState cs = w->checkState();
        if (cs ==  Qt::Checked)
            b = yes;
        else if (cs == Qt::Unchecked)
            b = no;
        else
            b = (Bool)2;
        setConfigABool( label, b );
    }
}


void MarkupTab::on_coerce_endtags()
{
    const char *label = "coerce-endtags";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_omit_optional_tags()
{
    const char *label = "omit-optional-tags";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_hide_endtags()
{
    const char *label = "hide-endtags";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_input_xml()
{
    const char *label = "input-xml";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_output_xml()
{
    const char *label = "output-xml";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_output_xhtml()
{
    const char *label = "output-xhtml";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_output_html()
{
    const char *label = "output-html";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_add_xml_decl()
{
    const char *label = "add-xml-decl";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_uppercase_tags()
{
    const char *label = "uppercase-tags";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_uppercase_attributes()
{
    const char *label = "uppercase-attributes";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_bare()
{
    const char *label = "bare";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_clean()
{
    const char *label = "clean";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_gdoc()
{
    const char *label = "gdoc";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_logical_emphasis()
{
    const char *label = "logical-emphasis";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_drop_proprietary_attributes()
{
    const char *label = "drop-proprietary-attributes";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_drop_font_tags()
{
    const char *label = "drop-font-tags";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_drop_empty_elements()
{
    const char *label = "drop-empty-elements";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_drop_empty_paras()
{
    const char *label = "drop-empty-paras";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_fix_bad_comments()
{
    const char *label = "fix-bad-comments";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_numeric_entities()
{
    const char *label = "numeric-entities";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_quote_marks()
{
    const char *label = "quote-marks";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_quote_nbsp()
{
    const char *label = "quote-nbsp";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_quote_ampersand()
{
    const char *label = "quote-ampersand";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_fix_backslash()
{
    const char *label = "fix-backslash";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_assume_xml_procins()
{
    const char *label = "assume-xml-procins";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_add_xml_space()
{
    const char *label = "add-xml-space";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_enclose_text()
{
    const char *label = "enclose-text";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_enclose_block_text()
{
    const char *label = "enclose-block-text";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_word_2000()
{
    const char *label = "word-2000";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_literal_attributes()
{
    const char *label = "literal-attributes";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_show_body_only()
{
    const char *label = "show-body-only";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b;
        Qt::CheckState cs = w->checkState();
        if (cs ==  Qt::Checked)
            b = yes;
        else if (cs == Qt::Unchecked)
            b = no;
        else
            b = (Bool)2;
        setConfigABool( label, b );
    }
}


void MarkupTab::on_fix_uri()
{
    const char *label = "fix-uri";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_lower_literals()
{
    const char *label = "lower-literals";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_hide_comments()
{
    const char *label = "hide-comments";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_indent_cdata()
{
    const char *label = "indent-cdata";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_join_classes()
{
    const char *label = "join-classes";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_join_styles()
{
    const char *label = "join-styles";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_escape_cdata()
{
    const char *label = "escape-cdata";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_ncr()
{
    const char *label = "ncr";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_replace_color()
{
    const char *label = "replace-color";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_merge_emphasis()
{
    const char *label = "merge-emphasis";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_merge_divs()
{
    const char *label = "merge-divs";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b;
        Qt::CheckState cs = w->checkState();
        if (cs ==  Qt::Checked)
            b = yes;
        else if (cs == Qt::Unchecked)
            b = no;
        else
            b = (Bool)2;
        setConfigABool( label, b );
    }
}


void MarkupTab::on_decorate_inferred_ul()
{
    const char *label = "decorate-inferred-ul";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_preserve_entities()
{
    const char *label = "preserve-entities";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MarkupTab::on_merge_spans()
{
    const char *label = "merge-spans";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b;
        Qt::CheckState cs = w->checkState();
        if (cs ==  Qt::Checked)
            b = yes;
        else if (cs == Qt::Unchecked)
            b = no;
        else
            b = (Bool)2;
        setConfigABool( label, b );
    }
}


void MarkupTab::on_anchor_as_name()
{
    const char *label = "anchor-as-name";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MiscTab::on_alt_textEd()
{
    const char *label = "alt-text";
    QLineEdit *w = qobject_cast<QLineEdit *>(sender());
    if (w) {
        QString s = w->text();
        s = s.trimmed();
        setConfigStg( label, s.toStdString().c_str() );
    }
}


void MiscTab::on_slide_styleEd()
{
    const char *label = "slide-style";
    QLineEdit *w = qobject_cast<QLineEdit *>(sender());
    if (w) {
        QString s = w->text();
        s = s.trimmed();
        setConfigStg( label, s.toStdString().c_str() );
    }
}


void MiscTab::on_error_fileEd()
{
    const char *label = "error-file";
    QLineEdit *w = qobject_cast<QLineEdit *>(sender());
    if (w) {
        QString s = w->text();
        s = s.trimmed();
        setConfigStg( label, s.toStdString().c_str() );
    }
}


void MiscTab::on_write_back()
{
    const char *label = "write-back";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MiscTab::on_quiet()
{
    const char *label = "quiet";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MiscTab::on_keep_time()
{
    const char *label = "keep-time";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MiscTab::on_tidy_mark()
{
    const char *label = "tidy-mark";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MiscTab::on_gnu_emacs()
{
    const char *label = "gnu-emacs";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MiscTab::on_gnu_emacs_fileEd()
{
    const char *label = "gnu-emacs-file";
    QLineEdit *w = qobject_cast<QLineEdit *>(sender());
    if (w) {
        QString s = w->text();
        s = s.trimmed();
        setConfigStg( label, s.toStdString().c_str() );
    }
}


void MiscTab::on_force_output()
{
    const char *label = "force-output";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void MiscTab::on_css_prefixEd()
{
    const char *label = "css-prefix";
    QLineEdit *w = qobject_cast<QLineEdit *>(sender());
    if (w) {
        QString s = w->text();
        s = s.trimmed();
        setConfigStg( label, s.toStdString().c_str() );
    }
}


void MiscTab::on_new_inline_tagsEd()
{
    const char *label = "new-inline-tags";
    QLineEdit *w = qobject_cast<QLineEdit *>(sender());
    if (w) {
        QString s = w->text();
        s = s.trimmed();
        setConfigStg( label, s.toStdString().c_str() );
    }
}


void MiscTab::on_new_blocklevel_tagsEd()
{
    const char *label = "new-blocklevel-tags";
    QLineEdit *w = qobject_cast<QLineEdit *>(sender());
    if (w) {
        QString s = w->text();
        s = s.trimmed();
        setConfigStg( label, s.toStdString().c_str() );
    }
}


void MiscTab::on_new_empty_tagsEd()
{
    const char *label = "new-empty-tags";
    QLineEdit *w = qobject_cast<QLineEdit *>(sender());
    if (w) {
        QString s = w->text();
        s = s.trimmed();
        setConfigStg( label, s.toStdString().c_str() );
    }
}


void MiscTab::on_new_pre_tagsEd()
{
    const char *label = "new-pre-tags";
    QLineEdit *w = qobject_cast<QLineEdit *>(sender());
    if (w) {
        QString s = w->text();
        s = s.trimmed();
        setConfigStg( label, s.toStdString().c_str() );
    }
}


void PrintTab::on_indent_spacesEd()
{
    const char *label = "indent-spaces";
    QLineEdit *w = qobject_cast<QLineEdit *>(sender());
    if (w) {
        QString s = w->text();
        int i = s.toInt();
        setConfigInt( label, i );
    }
}


void PrintTab::on_wrapEd()
{
    const char *label = "wrap";
    QLineEdit *w = qobject_cast<QLineEdit *>(sender());
    if (w) {
        QString s = w->text();
        int i = s.toInt();
        setConfigInt( label, i );
    }
}


void PrintTab::on_tab_sizeEd()
{
    const char *label = "tab-size";
    QLineEdit *w = qobject_cast<QLineEdit *>(sender());
    if (w) {
        QString s = w->text();
        int i = s.toInt();
        setConfigInt( label, i );
    }
}


void PrintTab::on_markup()
{
    const char *label = "markup";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void PrintTab::on_indent()
{
    const char *label = "indent";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b;
        Qt::CheckState cs = w->checkState();
        if (cs ==  Qt::Checked)
            b = yes;
        else if (cs == Qt::Unchecked)
            b = no;
        else
            b = (Bool)2;
        setConfigABool( label, b );
    }
}


void PrintTab::on_break_before_br()
{
    const char *label = "break-before-br";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void PrintTab::on_split()
{
    const char *label = "split";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void PrintTab::on_wrap_attributes()
{
    const char *label = "wrap-attributes";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void PrintTab::on_wrap_script_literals()
{
    const char *label = "wrap-script-literals";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void PrintTab::on_wrap_sections()
{
    const char *label = "wrap-sections";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void PrintTab::on_wrap_asp()
{
    const char *label = "wrap-asp";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void PrintTab::on_wrap_jste()
{
    const char *label = "wrap-jste";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void PrintTab::on_wrap_php()
{
    const char *label = "wrap-php";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void PrintTab::on_indent_attributes()
{
    const char *label = "indent-attributes";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void PrintTab::on_vertical_space()
{
    const char *label = "vertical-space";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


void PrintTab::on_punctuation_wrap()
{
    const char *label = "punctuation-wrap";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


// combobox slot change handler
void PrintTab::on_sort_attributesComb()
{
    const char *label = "sort-attributes";
    QComboBox *w = qobject_cast<QComboBox *>(sender());
    if (w) {
        int i = w->currentIndex();
        QVariant qv = w->itemData(i);
        QString qs = qv.toString();
        setConfigEnum( label, qs.toStdString().c_str() );
    }
}


void PrintTab::on_indent_with_tabs()
{
    const char *label = "indent-with-tabs";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}


// structures for objects
typedef struct tagCHKBXS {
    QCheckBox **p;  // pointer to pointer
    const char *lab;
    const char *typ;
    const char *tab;
}CHKBXS, *PCHKBXS;

typedef struct tagLABEL {
    QLabel **p;  // pointer to pointer
    const char *lab;
    const char *typ;
    const char *tab;
}LABEL, *PLABEL;

typedef struct tagLINED {
    QLineEdit **p;   // pointer to pointer
    const char *lab;
    const char *typ;
    const char *tab;
}LINED, *PLINED;

typedef struct tagCOMBOS {
    QComboBox **p;  // pointer to pointer
    const char *lab;
    const char *typ;
    const char *tab;
}COMBOS, *PCOMBOS;

// static objects
static QCheckBox *show_info;
static QCheckBox *show_warnings;
static QLabel *show_errors;
static QLineEdit *show_errorsEd;
static QComboBox *accessibility_checkComb;
static QComboBox *char_encodingComb;
static QComboBox *input_encodingComb;
static QComboBox *output_encodingComb;
static QComboBox *newlineComb;
static QComboBox *doctypeComb;
static QComboBox *repeated_attributesComb;
static QCheckBox *ascii_chars;
static QLabel *language;
static QLineEdit *languageEd;
static QCheckBox *output_bom;
static QCheckBox *coerce_endtags;
static QCheckBox *omit_optional_tags;
static QCheckBox *hide_endtags;
static QCheckBox *input_xml;
static QCheckBox *output_xml;
static QCheckBox *output_xhtml;
static QCheckBox *output_html;
static QCheckBox *add_xml_decl;
static QCheckBox *uppercase_tags;
static QCheckBox *uppercase_attributes;
static QCheckBox *bare;
static QCheckBox *clean;
static QCheckBox *gdoc;
static QCheckBox *logical_emphasis;
static QCheckBox *drop_proprietary_attributes;
static QCheckBox *drop_font_tags;
static QCheckBox *drop_empty_elements;
static QCheckBox *drop_empty_paras;
static QCheckBox *fix_bad_comments;
static QCheckBox *numeric_entities;
static QCheckBox *quote_marks;
static QCheckBox *quote_nbsp;
static QCheckBox *quote_ampersand;
static QCheckBox *fix_backslash;
static QCheckBox *assume_xml_procins;
static QCheckBox *add_xml_space;
static QCheckBox *enclose_text;
static QCheckBox *enclose_block_text;
static QCheckBox *word_2000;
static QCheckBox *literal_attributes;
static QCheckBox *show_body_only;
static QCheckBox *fix_uri;
static QCheckBox *lower_literals;
static QCheckBox *hide_comments;
static QCheckBox *indent_cdata;
static QCheckBox *join_classes;
static QCheckBox *join_styles;
static QCheckBox *escape_cdata;
static QCheckBox *ncr;
static QCheckBox *replace_color;
static QCheckBox *merge_emphasis;
static QCheckBox *merge_divs;
static QCheckBox *decorate_inferred_ul;
static QCheckBox *preserve_entities;
static QCheckBox *merge_spans;
static QCheckBox *anchor_as_name;
static QLabel *alt_text;
static QLineEdit *alt_textEd;
static QLabel *slide_style;
static QLineEdit *slide_styleEd;
static QLabel *error_file;
static QLineEdit *error_fileEd;
static QCheckBox *write_back;
static QCheckBox *quiet;
static QCheckBox *keep_time;
static QCheckBox *tidy_mark;
static QCheckBox *gnu_emacs;
static QLabel *gnu_emacs_file;
static QLineEdit *gnu_emacs_fileEd;
static QCheckBox *force_output;
static QLabel *css_prefix;
static QLineEdit *css_prefixEd;
static QLabel *new_inline_tags;
static QLineEdit *new_inline_tagsEd;
static QLabel *new_blocklevel_tags;
static QLineEdit *new_blocklevel_tagsEd;
static QLabel *new_empty_tags;
static QLineEdit *new_empty_tagsEd;
static QLabel *new_pre_tags;
static QLineEdit *new_pre_tagsEd;
static QLabel *indent_spaces;
static QLineEdit *indent_spacesEd;
static QLabel *wrap;
static QLineEdit *wrapEd;
static QLabel *tab_size;
static QLineEdit *tab_sizeEd;
static QCheckBox *markup;
static QCheckBox *indent;
static QCheckBox *break_before_br;
static QCheckBox *split;
static QCheckBox *wrap_attributes;
static QCheckBox *wrap_script_literals;
static QCheckBox *wrap_sections;
static QCheckBox *wrap_asp;
static QCheckBox *wrap_jste;
static QCheckBox *wrap_php;
static QCheckBox *indent_attributes;
static QCheckBox *vertical_space;
static QCheckBox *punctuation_wrap;
static QComboBox *sort_attributesComb;
static QCheckBox *indent_with_tabs;

// CheckBox types: AutoBool Boolean
static CHKBXS chkbxs[] = {
    { &show_info, "show-info", "Boolean", "diagnostics" },
    { &show_warnings, "show-warnings", "Boolean", "diagnostics" },
    { &ascii_chars, "ascii-chars", "Boolean", "encoding" },
    { &output_bom, "output-bom", "AutoBool", "encoding" },
    { &coerce_endtags, "coerce-endtags", "Boolean", "markup" },
    { &omit_optional_tags, "omit-optional-tags", "Boolean", "markup" },
    { &hide_endtags, "hide-endtags", "Boolean", "markup" },
    { &input_xml, "input-xml", "Boolean", "markup" },
    { &output_xml, "output-xml", "Boolean", "markup" },
    { &output_xhtml, "output-xhtml", "Boolean", "markup" },
    { &output_html, "output-html", "Boolean", "markup" },
    { &add_xml_decl, "add-xml-decl", "Boolean", "markup" },
    { &uppercase_tags, "uppercase-tags", "Boolean", "markup" },
    { &uppercase_attributes, "uppercase-attributes", "Boolean", "markup" },
    { &bare, "bare", "Boolean", "markup" },
    { &clean, "clean", "Boolean", "markup" },
    { &gdoc, "gdoc", "Boolean", "markup" },
    { &logical_emphasis, "logical-emphasis", "Boolean", "markup" },
    { &drop_proprietary_attributes, "drop-proprietary-attributes", "Boolean", "markup" },
    { &drop_font_tags, "drop-font-tags", "Boolean", "markup" },
    { &drop_empty_elements, "drop-empty-elements", "Boolean", "markup" },
    { &drop_empty_paras, "drop-empty-paras", "Boolean", "markup" },
    { &fix_bad_comments, "fix-bad-comments", "Boolean", "markup" },
    { &numeric_entities, "numeric-entities", "Boolean", "markup" },
    { &quote_marks, "quote-marks", "Boolean", "markup" },
    { &quote_nbsp, "quote-nbsp", "Boolean", "markup" },
    { &quote_ampersand, "quote-ampersand", "Boolean", "markup" },
    { &fix_backslash, "fix-backslash", "Boolean", "markup" },
    { &assume_xml_procins, "assume-xml-procins", "Boolean", "markup" },
    { &add_xml_space, "add-xml-space", "Boolean", "markup" },
    { &enclose_text, "enclose-text", "Boolean", "markup" },
    { &enclose_block_text, "enclose-block-text", "Boolean", "markup" },
    { &word_2000, "word-2000", "Boolean", "markup" },
    { &literal_attributes, "literal-attributes", "Boolean", "markup" },
    { &show_body_only, "show-body-only", "AutoBool", "markup" },
    { &fix_uri, "fix-uri", "Boolean", "markup" },
    { &lower_literals, "lower-literals", "Boolean", "markup" },
    { &hide_comments, "hide-comments", "Boolean", "markup" },
    { &indent_cdata, "indent-cdata", "Boolean", "markup" },
    { &join_classes, "join-classes", "Boolean", "markup" },
    { &join_styles, "join-styles", "Boolean", "markup" },
    { &escape_cdata, "escape-cdata", "Boolean", "markup" },
    { &ncr, "ncr", "Boolean", "markup" },
    { &replace_color, "replace-color", "Boolean", "markup" },
    { &merge_emphasis, "merge-emphasis", "Boolean", "markup" },
    { &merge_divs, "merge-divs", "AutoBool", "markup" },
    { &decorate_inferred_ul, "decorate-inferred-ul", "Boolean", "markup" },
    { &preserve_entities, "preserve-entities", "Boolean", "markup" },
    { &merge_spans, "merge-spans", "AutoBool", "markup" },
    { &anchor_as_name, "anchor-as-name", "Boolean", "markup" },
    { &write_back, "write-back", "Boolean", "misc" },
    { &quiet, "quiet", "Boolean", "misc" },
    { &keep_time, "keep-time", "Boolean", "misc" },
    { &tidy_mark, "tidy-mark", "Boolean", "misc" },
    { &gnu_emacs, "gnu-emacs", "Boolean", "misc" },
    { &force_output, "force-output", "Boolean", "misc" },
    { &markup, "markup", "Boolean", "print" },
    { &indent, "indent", "AutoBool", "print" },
    { &break_before_br, "break-before-br", "Boolean", "print" },
    { &split, "split", "Boolean", "print" },
    { &wrap_attributes, "wrap-attributes", "Boolean", "print" },
    { &wrap_script_literals, "wrap-script-literals", "Boolean", "print" },
    { &wrap_sections, "wrap-sections", "Boolean", "print" },
    { &wrap_asp, "wrap-asp", "Boolean", "print" },
    { &wrap_jste, "wrap-jste", "Boolean", "print" },
    { &wrap_php, "wrap-php", "Boolean", "print" },
    { &indent_attributes, "indent-attributes", "Boolean", "print" },
    { &vertical_space, "vertical-space", "Boolean", "print" },
    { &punctuation_wrap, "punctuation-wrap", "Boolean", "print" },
    { &indent_with_tabs, "indent-with-tabs", "Boolean", "print" },
    // LAST ENTRY
    { 0, 0, 0, 0 }
};

// Label types: 'Integer' 'String' 'Tag names' 
static LABEL label[] = {
    { &show_errors, "show-errors", "Integer", "diagnostics" },
    { &language, "language", "String", "encoding" },
    { &alt_text, "alt-text", "String", "misc" },
    { &slide_style, "slide-style", "String", "misc" },
    { &error_file, "error-file", "String", "misc" },
    { &gnu_emacs_file, "gnu-emacs-file", "String", "misc" },
    { &css_prefix, "css-prefix", "String", "misc" },
    { &new_inline_tags, "new-inline-tags", "Tag names", "misc" },
    { &new_blocklevel_tags, "new-blocklevel-tags", "Tag names", "misc" },
    { &new_empty_tags, "new-empty-tags", "Tag names", "misc" },
    { &new_pre_tags, "new-pre-tags", "Tag names", "misc" },
    { &indent_spaces, "indent-spaces", "Integer", "print" },
    { &wrap, "wrap", "Integer", "print" },
    { &tab_size, "tab-size", "Integer", "print" },
    // LAST ENTRY
    { 0, 0, 0, 0 }
};

// LineEdit types: 'Integer' 'String' 'Tag names' 
static LINED lined[] = {
    { &show_errorsEd, "show-errors", "Integer", "diagnostics" },
    { &languageEd, "language", "String", "encoding" },
    { &alt_textEd, "alt-text", "String", "misc" },
    { &slide_styleEd, "slide-style", "String", "misc" },
    { &error_fileEd, "error-file", "String", "misc" },
    { &gnu_emacs_fileEd, "gnu-emacs-file", "String", "misc" },
    { &css_prefixEd, "css-prefix", "String", "misc" },
    { &new_inline_tagsEd, "new-inline-tags", "Tag names", "misc" },
    { &new_blocklevel_tagsEd, "new-blocklevel-tags", "Tag names", "misc" },
    { &new_empty_tagsEd, "new-empty-tags", "Tag names", "misc" },
    { &new_pre_tagsEd, "new-pre-tags", "Tag names", "misc" },
    { &indent_spacesEd, "indent-spaces", "Integer", "print" },
    { &wrapEd, "wrap", "Integer", "print" },
    { &tab_sizeEd, "tab-size", "Integer", "print" },
    // LAST ENTRY
    { 0, 0, 0, 0 }
};

// ComboBox types: 'DocType' 'Encoding' 'enum' 
static COMBOS combos[] = {
    { &accessibility_checkComb, "accessibility-check", "enum", "diagnostics" },
    { &char_encodingComb, "char-encoding", "Encoding", "encoding" },
    { &input_encodingComb, "input-encoding", "Encoding", "encoding" },
    { &output_encodingComb, "output-encoding", "Encoding", "encoding" },
    { &newlineComb, "newline", "enum", "encoding" },
    { &doctypeComb, "doctype", "DocType", "encoding" },
    { &repeated_attributesComb, "repeated-attributes", "enum", "encoding" },
    { &sort_attributesComb, "sort-attributes", "enum", "print" },
    // LAST ENTRY
    { 0, 0, 0, 0 }
};

DiagnosticsTab::DiagnosticsTab( PINFOSTR pinf, QWidget *parent)
    : QWidget(parent)
{
    int i;
    QString s;

    show_info = new QCheckBox("show-info (Boolean)");
    if (getConfigBool("show-info")) {
        show_info->setChecked(true);
    }
    connect(show_info,SIGNAL(clicked()),this,SLOT(on_show_info()));


    show_warnings = new QCheckBox("show-warnings (Boolean)");
    if (getConfigBool("show-warnings")) {
        show_warnings->setChecked(true);
    }
    connect(show_warnings,SIGNAL(clicked()),this,SLOT(on_show_warnings()));


    show_errors = new QLabel("show-errors (Integer)");
    i = getConfigInt("show-errors");
    s = QString::number(i);
    show_errorsEd = new QLineEdit(s);
    show_errorsEd->setMaximumWidth(50);
    connect(show_errorsEd,SIGNAL(editingFinished()),this,SLOT(on_show_errorsEd()));

    QGroupBox *accessibility_check = new QGroupBox("accessibility-check");
    accessibility_checkComb = new QComboBox();
    accessibility_checkComb->addItem("0 (Tidy Classic)","0 (Tidy Classic)");
    accessibility_checkComb->addItem("1 (Priority 1 Checks)","1 (Priority 1 Checks)");
    accessibility_checkComb->addItem("2 (Priority 2 Checks)","2 (Priority 2 Checks)");
    accessibility_checkComb->addItem("3 (Priority 3 Checks)","3 (Priority 3 Checks)");
    s = getConfigEnum("accessibility-check");
    i = accessibility_checkComb->findData(s);
    if (i != -1) {
        accessibility_checkComb->setCurrentIndex(i);
    }
    QVBoxLayout *accessibility_checkLay = new QVBoxLayout;
    accessibility_checkLay->addWidget(accessibility_checkComb);
    accessibility_checkLay->addStretch(1);
    accessibility_check->setLayout(accessibility_checkLay);
    connect(accessibility_checkComb, SIGNAL(currentIndexChanged(int)), this, SLOT(on_accessibility_checkComb()));

    QVBoxLayout *DiagnosticsLayout = new QVBoxLayout;
    DiagnosticsLayout->addWidget(show_info);
    DiagnosticsLayout->addWidget(show_warnings);
    DiagnosticsLayout->addWidget(show_errors);
    DiagnosticsLayout->addWidget(show_errorsEd);
    DiagnosticsLayout->addWidget(accessibility_check);
    DiagnosticsLayout->addStretch(1);
    setLayout(DiagnosticsLayout);
}


EncodingTab::EncodingTab( PINFOSTR pinf, QWidget *parent)
    : QWidget(parent)
{
    int i;
    QString s;

    QGroupBox *char_encoding = new QGroupBox("char-encoding");
    char_encodingComb = new QComboBox();
    char_encodingComb->addItem("raw","raw");
    char_encodingComb->addItem("ascii","ascii");
    char_encodingComb->addItem("latin0","latin0");
    char_encodingComb->addItem("latin1","latin1");
    char_encodingComb->addItem("utf8","utf8");
    char_encodingComb->addItem("iso2022","iso2022");
    char_encodingComb->addItem("mac","mac");
    char_encodingComb->addItem("win1252","win1252");
    char_encodingComb->addItem("ibm858","ibm858");
    char_encodingComb->addItem("utf16le","utf16le");
    char_encodingComb->addItem("utf16be","utf16be");
    char_encodingComb->addItem("utf16","utf16");
    char_encodingComb->addItem("big5","big5");
    char_encodingComb->addItem("shiftjis","shiftjis");
    s = getConfigEnc("char-encoding");
    i = char_encodingComb->findData(s);
    if (i != -1) {
        char_encodingComb->setCurrentIndex(i);
    }
    QVBoxLayout *char_encodingLay = new QVBoxLayout;
    char_encodingLay->addWidget(char_encodingComb);
    char_encodingLay->addStretch(1);
    char_encoding->setLayout(char_encodingLay);
    connect(char_encodingComb, SIGNAL(currentIndexChanged(int)), this, SLOT(on_char_encodingComb()));

    QGroupBox *input_encoding = new QGroupBox("input-encoding");
    input_encodingComb = new QComboBox();
    input_encodingComb->addItem("raw","raw");
    input_encodingComb->addItem("ascii","ascii");
    input_encodingComb->addItem("latin0","latin0");
    input_encodingComb->addItem("latin1","latin1");
    input_encodingComb->addItem("utf8","utf8");
    input_encodingComb->addItem("iso2022","iso2022");
    input_encodingComb->addItem("mac","mac");
    input_encodingComb->addItem("win1252","win1252");
    input_encodingComb->addItem("ibm858","ibm858");
    input_encodingComb->addItem("utf16le","utf16le");
    input_encodingComb->addItem("utf16be","utf16be");
    input_encodingComb->addItem("utf16","utf16");
    input_encodingComb->addItem("big5","big5");
    input_encodingComb->addItem("shiftjis","shiftjis");
    s = getConfigEnc("input-encoding");
    i = input_encodingComb->findData(s);
    if (i != -1) {
        input_encodingComb->setCurrentIndex(i);
    }
    QVBoxLayout *input_encodingLay = new QVBoxLayout;
    input_encodingLay->addWidget(input_encodingComb);
    input_encodingLay->addStretch(1);
    input_encoding->setLayout(input_encodingLay);
    connect(input_encodingComb, SIGNAL(currentIndexChanged(int)), this, SLOT(on_input_encodingComb()));

    QGroupBox *output_encoding = new QGroupBox("output-encoding");
    output_encodingComb = new QComboBox();
    output_encodingComb->addItem("raw","raw");
    output_encodingComb->addItem("ascii","ascii");
    output_encodingComb->addItem("latin0","latin0");
    output_encodingComb->addItem("latin1","latin1");
    output_encodingComb->addItem("utf8","utf8");
    output_encodingComb->addItem("iso2022","iso2022");
    output_encodingComb->addItem("mac","mac");
    output_encodingComb->addItem("win1252","win1252");
    output_encodingComb->addItem("ibm858","ibm858");
    output_encodingComb->addItem("utf16le","utf16le");
    output_encodingComb->addItem("utf16be","utf16be");
    output_encodingComb->addItem("utf16","utf16");
    output_encodingComb->addItem("big5","big5");
    output_encodingComb->addItem("shiftjis","shiftjis");
    s = getConfigEnc("output-encoding");
    i = output_encodingComb->findData(s);
    if (i != -1) {
        output_encodingComb->setCurrentIndex(i);
    }
    QVBoxLayout *output_encodingLay = new QVBoxLayout;
    output_encodingLay->addWidget(output_encodingComb);
    output_encodingLay->addStretch(1);
    output_encoding->setLayout(output_encodingLay);
    connect(output_encodingComb, SIGNAL(currentIndexChanged(int)), this, SLOT(on_output_encodingComb()));

    QGroupBox *newline = new QGroupBox("newline");
    newlineComb = new QComboBox();
    newlineComb->addItem("LF","LF");
    newlineComb->addItem("CRLF","CRLF");
    newlineComb->addItem("CR","CR");
    s = getConfigEnum("newline");
    i = newlineComb->findData(s);
    if (i != -1) {
        newlineComb->setCurrentIndex(i);
    }
    QVBoxLayout *newlineLay = new QVBoxLayout;
    newlineLay->addWidget(newlineComb);
    newlineLay->addStretch(1);
    newline->setLayout(newlineLay);
    connect(newlineComb, SIGNAL(currentIndexChanged(int)), this, SLOT(on_newlineComb()));

    QGroupBox *doctype = new QGroupBox("doctype");
    doctypeComb = new QComboBox();
    doctypeComb->addItem("html5","html5");
    doctypeComb->addItem("omit","omit");
    doctypeComb->addItem("auto","auto");
    doctypeComb->addItem("strict","strict");
    doctypeComb->addItem("transitional","transitional");
    doctypeComb->addItem("user","user");
    s = getConfigPick("doctype");
    i = doctypeComb->findData(s);
    if (i != -1) {
        doctypeComb->setCurrentIndex(i);
    }
    QVBoxLayout *doctypeLay = new QVBoxLayout;
    doctypeLay->addWidget(doctypeComb);
    doctypeLay->addStretch(1);
    doctype->setLayout(doctypeLay);
    connect(doctypeComb, SIGNAL(currentIndexChanged(int)), this, SLOT(on_doctypeComb()));

    QGroupBox *repeated_attributes = new QGroupBox("repeated-attributes");
    repeated_attributesComb = new QComboBox();
    repeated_attributesComb->addItem("keep-first","keep-first");
    repeated_attributesComb->addItem("keep-last","keep-last");
    s = getConfigEnum("repeated-attributes");
    i = repeated_attributesComb->findData(s);
    if (i != -1) {
        repeated_attributesComb->setCurrentIndex(i);
    }
    QVBoxLayout *repeated_attributesLay = new QVBoxLayout;
    repeated_attributesLay->addWidget(repeated_attributesComb);
    repeated_attributesLay->addStretch(1);
    repeated_attributes->setLayout(repeated_attributesLay);
    connect(repeated_attributesComb, SIGNAL(currentIndexChanged(int)), this, SLOT(on_repeated_attributesComb()));

    ascii_chars = new QCheckBox("ascii-chars (Boolean)");
    if (getConfigBool("ascii-chars")) {
        ascii_chars->setChecked(true);
    }
    connect(ascii_chars,SIGNAL(clicked()),this,SLOT(on_ascii_chars()));


    language = new QLabel("language (String)");
    s = getConfigStg("language");
    languageEd = new QLineEdit(s);
    connect(languageEd,SIGNAL(editingFinished()),this,SLOT(on_languageEd()));

    output_bom = new QCheckBox("output-bom (AutoBool)");
    output_bom->setTristate(true);
    i = getConfigABool("output-bom");
    if (i == 0) {
        output_bom->setChecked(false);
        // output_bom->setCheckState(Qt::Unchecked); // 0
    } else if (i == 1) {
        output_bom->setChecked(true);
        // output_bom->setCheckState(Qt::Checked); // 2
    } else {
        output_bom->setCheckState(Qt::PartiallyChecked); // 1
    }
    connect(output_bom,SIGNAL(clicked()),this,SLOT(on_output_bom()));


    QVBoxLayout *EncodingLayout = new QVBoxLayout;
    EncodingLayout->addWidget(char_encoding);
    EncodingLayout->addWidget(input_encoding);
    EncodingLayout->addWidget(output_encoding);
    EncodingLayout->addWidget(newline);
    EncodingLayout->addWidget(doctype);
    EncodingLayout->addWidget(repeated_attributes);
    EncodingLayout->addWidget(ascii_chars);
    EncodingLayout->addWidget(language);
    EncodingLayout->addWidget(languageEd);
    EncodingLayout->addWidget(output_bom);
    EncodingLayout->addStretch(1);
    setLayout(EncodingLayout);
}


MarkupTab::MarkupTab( PINFOSTR pinf, QWidget *parent)
    : QWidget(parent)
{
    int i;
    QString s;

    coerce_endtags = new QCheckBox("coerce-endtags (Boolean)");
    if (getConfigBool("coerce-endtags")) {
        coerce_endtags->setChecked(true);
    }
    connect(coerce_endtags,SIGNAL(clicked()),this,SLOT(on_coerce_endtags()));


    omit_optional_tags = new QCheckBox("omit-optional-tags (Boolean)");
    if (getConfigBool("omit-optional-tags")) {
        omit_optional_tags->setChecked(true);
    }
    connect(omit_optional_tags,SIGNAL(clicked()),this,SLOT(on_omit_optional_tags()));


    hide_endtags = new QCheckBox("hide-endtags (Boolean)");
    if (getConfigBool("hide-endtags")) {
        hide_endtags->setChecked(true);
    }
    connect(hide_endtags,SIGNAL(clicked()),this,SLOT(on_hide_endtags()));


    input_xml = new QCheckBox("input-xml (Boolean)");
    if (getConfigBool("input-xml")) {
        input_xml->setChecked(true);
    }
    connect(input_xml,SIGNAL(clicked()),this,SLOT(on_input_xml()));


    output_xml = new QCheckBox("output-xml (Boolean)");
    if (getConfigBool("output-xml")) {
        output_xml->setChecked(true);
    }
    connect(output_xml,SIGNAL(clicked()),this,SLOT(on_output_xml()));


    output_xhtml = new QCheckBox("output-xhtml (Boolean)");
    if (getConfigBool("output-xhtml")) {
        output_xhtml->setChecked(true);
    }
    connect(output_xhtml,SIGNAL(clicked()),this,SLOT(on_output_xhtml()));


    output_html = new QCheckBox("output-html (Boolean)");
    if (getConfigBool("output-html")) {
        output_html->setChecked(true);
    }
    connect(output_html,SIGNAL(clicked()),this,SLOT(on_output_html()));


    add_xml_decl = new QCheckBox("add-xml-decl (Boolean)");
    if (getConfigBool("add-xml-decl")) {
        add_xml_decl->setChecked(true);
    }
    connect(add_xml_decl,SIGNAL(clicked()),this,SLOT(on_add_xml_decl()));


    uppercase_tags = new QCheckBox("uppercase-tags (Boolean)");
    if (getConfigBool("uppercase-tags")) {
        uppercase_tags->setChecked(true);
    }
    connect(uppercase_tags,SIGNAL(clicked()),this,SLOT(on_uppercase_tags()));


    uppercase_attributes = new QCheckBox("uppercase-attributes (Boolean)");
    if (getConfigBool("uppercase-attributes")) {
        uppercase_attributes->setChecked(true);
    }
    connect(uppercase_attributes,SIGNAL(clicked()),this,SLOT(on_uppercase_attributes()));


    bare = new QCheckBox("bare (Boolean)");
    if (getConfigBool("bare")) {
        bare->setChecked(true);
    }
    connect(bare,SIGNAL(clicked()),this,SLOT(on_bare()));


    clean = new QCheckBox("clean (Boolean)");
    if (getConfigBool("clean")) {
        clean->setChecked(true);
    }
    connect(clean,SIGNAL(clicked()),this,SLOT(on_clean()));


    gdoc = new QCheckBox("gdoc (Boolean)");
    if (getConfigBool("gdoc")) {
        gdoc->setChecked(true);
    }
    connect(gdoc,SIGNAL(clicked()),this,SLOT(on_gdoc()));


    logical_emphasis = new QCheckBox("logical-emphasis (Boolean)");
    if (getConfigBool("logical-emphasis")) {
        logical_emphasis->setChecked(true);
    }
    connect(logical_emphasis,SIGNAL(clicked()),this,SLOT(on_logical_emphasis()));


    drop_proprietary_attributes = new QCheckBox("drop-proprietary-attributes (Boolean)");
    if (getConfigBool("drop-proprietary-attributes")) {
        drop_proprietary_attributes->setChecked(true);
    }
    connect(drop_proprietary_attributes,SIGNAL(clicked()),this,SLOT(on_drop_proprietary_attributes()));


    drop_font_tags = new QCheckBox("drop-font-tags (Boolean)");
    if (getConfigBool("drop-font-tags")) {
        drop_font_tags->setChecked(true);
    }
    connect(drop_font_tags,SIGNAL(clicked()),this,SLOT(on_drop_font_tags()));


    drop_empty_elements = new QCheckBox("drop-empty-elements (Boolean)");
    if (getConfigBool("drop-empty-elements")) {
        drop_empty_elements->setChecked(true);
    }
    connect(drop_empty_elements,SIGNAL(clicked()),this,SLOT(on_drop_empty_elements()));


    drop_empty_paras = new QCheckBox("drop-empty-paras (Boolean)");
    if (getConfigBool("drop-empty-paras")) {
        drop_empty_paras->setChecked(true);
    }
    connect(drop_empty_paras,SIGNAL(clicked()),this,SLOT(on_drop_empty_paras()));


    fix_bad_comments = new QCheckBox("fix-bad-comments (Boolean)");
    if (getConfigBool("fix-bad-comments")) {
        fix_bad_comments->setChecked(true);
    }
    connect(fix_bad_comments,SIGNAL(clicked()),this,SLOT(on_fix_bad_comments()));


    numeric_entities = new QCheckBox("numeric-entities (Boolean)");
    if (getConfigBool("numeric-entities")) {
        numeric_entities->setChecked(true);
    }
    connect(numeric_entities,SIGNAL(clicked()),this,SLOT(on_numeric_entities()));


    quote_marks = new QCheckBox("quote-marks (Boolean)");
    if (getConfigBool("quote-marks")) {
        quote_marks->setChecked(true);
    }
    connect(quote_marks,SIGNAL(clicked()),this,SLOT(on_quote_marks()));


    quote_nbsp = new QCheckBox("quote-nbsp (Boolean)");
    if (getConfigBool("quote-nbsp")) {
        quote_nbsp->setChecked(true);
    }
    connect(quote_nbsp,SIGNAL(clicked()),this,SLOT(on_quote_nbsp()));


    quote_ampersand = new QCheckBox("quote-ampersand (Boolean)");
    if (getConfigBool("quote-ampersand")) {
        quote_ampersand->setChecked(true);
    }
    connect(quote_ampersand,SIGNAL(clicked()),this,SLOT(on_quote_ampersand()));


    fix_backslash = new QCheckBox("fix-backslash (Boolean)");
    if (getConfigBool("fix-backslash")) {
        fix_backslash->setChecked(true);
    }
    connect(fix_backslash,SIGNAL(clicked()),this,SLOT(on_fix_backslash()));


    assume_xml_procins = new QCheckBox("assume-xml-procins (Boolean)");
    if (getConfigBool("assume-xml-procins")) {
        assume_xml_procins->setChecked(true);
    }
    connect(assume_xml_procins,SIGNAL(clicked()),this,SLOT(on_assume_xml_procins()));


    add_xml_space = new QCheckBox("add-xml-space (Boolean)");
    if (getConfigBool("add-xml-space")) {
        add_xml_space->setChecked(true);
    }
    connect(add_xml_space,SIGNAL(clicked()),this,SLOT(on_add_xml_space()));


    enclose_text = new QCheckBox("enclose-text (Boolean)");
    if (getConfigBool("enclose-text")) {
        enclose_text->setChecked(true);
    }
    connect(enclose_text,SIGNAL(clicked()),this,SLOT(on_enclose_text()));


    enclose_block_text = new QCheckBox("enclose-block-text (Boolean)");
    if (getConfigBool("enclose-block-text")) {
        enclose_block_text->setChecked(true);
    }
    connect(enclose_block_text,SIGNAL(clicked()),this,SLOT(on_enclose_block_text()));


    word_2000 = new QCheckBox("word-2000 (Boolean)");
    if (getConfigBool("word-2000")) {
        word_2000->setChecked(true);
    }
    connect(word_2000,SIGNAL(clicked()),this,SLOT(on_word_2000()));


    literal_attributes = new QCheckBox("literal-attributes (Boolean)");
    if (getConfigBool("literal-attributes")) {
        literal_attributes->setChecked(true);
    }
    connect(literal_attributes,SIGNAL(clicked()),this,SLOT(on_literal_attributes()));


    show_body_only = new QCheckBox("show-body-only (AutoBool)");
    show_body_only->setTristate(true);
    i = getConfigABool("show-body-only");
    if (i == 0) {
        show_body_only->setChecked(false);
        // show_body_only->setCheckState(Qt::Unchecked); // 0
    } else if (i == 1) {
        show_body_only->setChecked(true);
        // show_body_only->setCheckState(Qt::Checked); // 2
    } else {
        show_body_only->setCheckState(Qt::PartiallyChecked); // 1
    }
    connect(show_body_only,SIGNAL(clicked()),this,SLOT(on_show_body_only()));


    fix_uri = new QCheckBox("fix-uri (Boolean)");
    if (getConfigBool("fix-uri")) {
        fix_uri->setChecked(true);
    }
    connect(fix_uri,SIGNAL(clicked()),this,SLOT(on_fix_uri()));


    lower_literals = new QCheckBox("lower-literals (Boolean)");
    if (getConfigBool("lower-literals")) {
        lower_literals->setChecked(true);
    }
    connect(lower_literals,SIGNAL(clicked()),this,SLOT(on_lower_literals()));


    hide_comments = new QCheckBox("hide-comments (Boolean)");
    if (getConfigBool("hide-comments")) {
        hide_comments->setChecked(true);
    }
    connect(hide_comments,SIGNAL(clicked()),this,SLOT(on_hide_comments()));


    indent_cdata = new QCheckBox("indent-cdata (Boolean)");
    if (getConfigBool("indent-cdata")) {
        indent_cdata->setChecked(true);
    }
    connect(indent_cdata,SIGNAL(clicked()),this,SLOT(on_indent_cdata()));


    join_classes = new QCheckBox("join-classes (Boolean)");
    if (getConfigBool("join-classes")) {
        join_classes->setChecked(true);
    }
    connect(join_classes,SIGNAL(clicked()),this,SLOT(on_join_classes()));


    join_styles = new QCheckBox("join-styles (Boolean)");
    if (getConfigBool("join-styles")) {
        join_styles->setChecked(true);
    }
    connect(join_styles,SIGNAL(clicked()),this,SLOT(on_join_styles()));


    escape_cdata = new QCheckBox("escape-cdata (Boolean)");
    if (getConfigBool("escape-cdata")) {
        escape_cdata->setChecked(true);
    }
    connect(escape_cdata,SIGNAL(clicked()),this,SLOT(on_escape_cdata()));


    ncr = new QCheckBox("ncr (Boolean)");
    if (getConfigBool("ncr")) {
        ncr->setChecked(true);
    }
    connect(ncr,SIGNAL(clicked()),this,SLOT(on_ncr()));


    replace_color = new QCheckBox("replace-color (Boolean)");
    if (getConfigBool("replace-color")) {
        replace_color->setChecked(true);
    }
    connect(replace_color,SIGNAL(clicked()),this,SLOT(on_replace_color()));


    merge_emphasis = new QCheckBox("merge-emphasis (Boolean)");
    if (getConfigBool("merge-emphasis")) {
        merge_emphasis->setChecked(true);
    }
    connect(merge_emphasis,SIGNAL(clicked()),this,SLOT(on_merge_emphasis()));


    merge_divs = new QCheckBox("merge-divs (AutoBool)");
    merge_divs->setTristate(true);
    i = getConfigABool("merge-divs");
    if (i == 0) {
        merge_divs->setChecked(false);
        // merge_divs->setCheckState(Qt::Unchecked); // 0
    } else if (i == 1) {
        merge_divs->setChecked(true);
        // merge_divs->setCheckState(Qt::Checked); // 2
    } else {
        merge_divs->setCheckState(Qt::PartiallyChecked); // 1
    }
    connect(merge_divs,SIGNAL(clicked()),this,SLOT(on_merge_divs()));


    decorate_inferred_ul = new QCheckBox("decorate-inferred-ul (Boolean)");
    if (getConfigBool("decorate-inferred-ul")) {
        decorate_inferred_ul->setChecked(true);
    }
    connect(decorate_inferred_ul,SIGNAL(clicked()),this,SLOT(on_decorate_inferred_ul()));


    preserve_entities = new QCheckBox("preserve-entities (Boolean)");
    if (getConfigBool("preserve-entities")) {
        preserve_entities->setChecked(true);
    }
    connect(preserve_entities,SIGNAL(clicked()),this,SLOT(on_preserve_entities()));


    merge_spans = new QCheckBox("merge-spans (AutoBool)");
    merge_spans->setTristate(true);
    i = getConfigABool("merge-spans");
    if (i == 0) {
        merge_spans->setChecked(false);
        // merge_spans->setCheckState(Qt::Unchecked); // 0
    } else if (i == 1) {
        merge_spans->setChecked(true);
        // merge_spans->setCheckState(Qt::Checked); // 2
    } else {
        merge_spans->setCheckState(Qt::PartiallyChecked); // 1
    }
    connect(merge_spans,SIGNAL(clicked()),this,SLOT(on_merge_spans()));


    anchor_as_name = new QCheckBox("anchor-as-name (Boolean)");
    if (getConfigBool("anchor-as-name")) {
        anchor_as_name->setChecked(true);
    }
    connect(anchor_as_name,SIGNAL(clicked()),this,SLOT(on_anchor_as_name()));


    QVBoxLayout *MarkupLayout1 = new QVBoxLayout;
    MarkupLayout1->addWidget(coerce_endtags);
    MarkupLayout1->addWidget(omit_optional_tags);
    MarkupLayout1->addWidget(hide_endtags);
    MarkupLayout1->addWidget(input_xml);
    MarkupLayout1->addWidget(output_xml);
    MarkupLayout1->addWidget(output_xhtml);
    MarkupLayout1->addWidget(output_html);
    MarkupLayout1->addWidget(add_xml_decl);
    MarkupLayout1->addWidget(uppercase_tags);
    MarkupLayout1->addWidget(uppercase_attributes);
    MarkupLayout1->addWidget(bare);
    MarkupLayout1->addWidget(clean);
    MarkupLayout1->addWidget(gdoc);
    MarkupLayout1->addWidget(logical_emphasis);
    MarkupLayout1->addWidget(drop_proprietary_attributes);
    MarkupLayout1->addWidget(drop_font_tags);
    MarkupLayout1->addWidget(drop_empty_elements);
    MarkupLayout1->addWidget(drop_empty_paras);
    MarkupLayout1->addWidget(fix_bad_comments);
    MarkupLayout1->addWidget(numeric_entities);
    MarkupLayout1->addWidget(quote_marks);
    MarkupLayout1->addWidget(quote_nbsp);
    MarkupLayout1->addWidget(quote_ampersand);
    MarkupLayout1->addStretch(1);

    QVBoxLayout *MarkupLayout2 = new QVBoxLayout;
    MarkupLayout2->addWidget(fix_backslash);
    MarkupLayout2->addWidget(assume_xml_procins);
    MarkupLayout2->addWidget(add_xml_space);
    MarkupLayout2->addWidget(enclose_text);
    MarkupLayout2->addWidget(enclose_block_text);
    MarkupLayout2->addWidget(word_2000);
    MarkupLayout2->addWidget(literal_attributes);
    MarkupLayout2->addWidget(show_body_only);
    MarkupLayout2->addWidget(fix_uri);
    MarkupLayout2->addWidget(lower_literals);
    MarkupLayout2->addWidget(hide_comments);
    MarkupLayout2->addWidget(indent_cdata);
    MarkupLayout2->addWidget(join_classes);
    MarkupLayout2->addWidget(join_styles);
    MarkupLayout2->addWidget(escape_cdata);
    MarkupLayout2->addWidget(ncr);
    MarkupLayout2->addWidget(replace_color);
    MarkupLayout2->addWidget(merge_emphasis);
    MarkupLayout2->addWidget(merge_divs);
    MarkupLayout2->addWidget(decorate_inferred_ul);
    MarkupLayout2->addWidget(preserve_entities);
    MarkupLayout2->addWidget(merge_spans);
    MarkupLayout2->addWidget(anchor_as_name);
    MarkupLayout2->addStretch(1);

    QHBoxLayout *MarkupLayout = new QHBoxLayout;
    MarkupLayout->addLayout(MarkupLayout1);
    MarkupLayout->addLayout(MarkupLayout2);
    setLayout(MarkupLayout);
}


MiscTab::MiscTab( PINFOSTR pinf, QWidget *parent)
    : QWidget(parent)
{
    //int i;
    QString s;

    alt_text = new QLabel("alt-text (String)");
    s = getConfigStg("alt-text");
    alt_textEd = new QLineEdit(s);
    connect(alt_textEd,SIGNAL(editingFinished()),this,SLOT(on_alt_textEd()));

    slide_style = new QLabel("slide-style (String)");
    s = getConfigStg("slide-style");
    slide_styleEd = new QLineEdit(s);
    connect(slide_styleEd,SIGNAL(editingFinished()),this,SLOT(on_slide_styleEd()));

    error_file = new QLabel("error-file (String)");
    s = getConfigStg("error-file");
    error_fileEd = new QLineEdit(s);
    connect(error_fileEd,SIGNAL(editingFinished()),this,SLOT(on_error_fileEd()));

    write_back = new QCheckBox("write-back (Boolean)");
    if (getConfigBool("write-back")) {
        write_back->setChecked(true);
    }
    connect(write_back,SIGNAL(clicked()),this,SLOT(on_write_back()));


    quiet = new QCheckBox("quiet (Boolean)");
    if (getConfigBool("quiet")) {
        quiet->setChecked(true);
    }
    connect(quiet,SIGNAL(clicked()),this,SLOT(on_quiet()));


    keep_time = new QCheckBox("keep-time (Boolean)");
    if (getConfigBool("keep-time")) {
        keep_time->setChecked(true);
    }
    connect(keep_time,SIGNAL(clicked()),this,SLOT(on_keep_time()));


    tidy_mark = new QCheckBox("tidy-mark (Boolean)");
    if (getConfigBool("tidy-mark")) {
        tidy_mark->setChecked(true);
    }
    connect(tidy_mark,SIGNAL(clicked()),this,SLOT(on_tidy_mark()));


    gnu_emacs = new QCheckBox("gnu-emacs (Boolean)");
    if (getConfigBool("gnu-emacs")) {
        gnu_emacs->setChecked(true);
    }
    connect(gnu_emacs,SIGNAL(clicked()),this,SLOT(on_gnu_emacs()));


    gnu_emacs_file = new QLabel("gnu-emacs-file (String)");
    s = getConfigStg("gnu-emacs-file");
    gnu_emacs_fileEd = new QLineEdit(s);
    connect(gnu_emacs_fileEd,SIGNAL(editingFinished()),this,SLOT(on_gnu_emacs_fileEd()));

    force_output = new QCheckBox("force-output (Boolean)");
    if (getConfigBool("force-output")) {
        force_output->setChecked(true);
    }
    connect(force_output,SIGNAL(clicked()),this,SLOT(on_force_output()));


    css_prefix = new QLabel("css-prefix (String)");
    s = getConfigStg("css-prefix");
    css_prefixEd = new QLineEdit(s);
    connect(css_prefixEd,SIGNAL(editingFinished()),this,SLOT(on_css_prefixEd()));

    new_inline_tags = new QLabel("new-inline-tags (String)");
    s = getConfigStg("new-inline-tags");
    new_inline_tagsEd = new QLineEdit(s);
    connect(new_inline_tagsEd,SIGNAL(editingFinished()),this,SLOT(on_new_inline_tagsEd()));

    new_blocklevel_tags = new QLabel("new-blocklevel-tags (String)");
    s = getConfigStg("new-blocklevel-tags");
    new_blocklevel_tagsEd = new QLineEdit(s);
    connect(new_blocklevel_tagsEd,SIGNAL(editingFinished()),this,SLOT(on_new_blocklevel_tagsEd()));

    new_empty_tags = new QLabel("new-empty-tags (String)");
    s = getConfigStg("new-empty-tags");
    new_empty_tagsEd = new QLineEdit(s);
    connect(new_empty_tagsEd,SIGNAL(editingFinished()),this,SLOT(on_new_empty_tagsEd()));

    new_pre_tags = new QLabel("new-pre-tags (String)");
    s = getConfigStg("new-pre-tags");
    new_pre_tagsEd = new QLineEdit(s);
    connect(new_pre_tagsEd,SIGNAL(editingFinished()),this,SLOT(on_new_pre_tagsEd()));

    QVBoxLayout *MiscLayout = new QVBoxLayout;
    MiscLayout->addWidget(alt_text);
    MiscLayout->addWidget(alt_textEd);
    MiscLayout->addWidget(slide_style);
    MiscLayout->addWidget(slide_styleEd);
    MiscLayout->addWidget(error_file);
    MiscLayout->addWidget(error_fileEd);
    MiscLayout->addWidget(write_back);
    MiscLayout->addWidget(quiet);
    MiscLayout->addWidget(keep_time);
    MiscLayout->addWidget(tidy_mark);
    MiscLayout->addWidget(gnu_emacs);
    MiscLayout->addWidget(gnu_emacs_file);
    MiscLayout->addWidget(gnu_emacs_fileEd);
    MiscLayout->addWidget(force_output);
    MiscLayout->addWidget(css_prefix);
    MiscLayout->addWidget(css_prefixEd);
    MiscLayout->addWidget(new_inline_tags);
    MiscLayout->addWidget(new_inline_tagsEd);
    MiscLayout->addWidget(new_blocklevel_tags);
    MiscLayout->addWidget(new_blocklevel_tagsEd);
    MiscLayout->addWidget(new_empty_tags);
    MiscLayout->addWidget(new_empty_tagsEd);
    MiscLayout->addWidget(new_pre_tags);
    MiscLayout->addWidget(new_pre_tagsEd);
    MiscLayout->addStretch(1);
    setLayout(MiscLayout);
}


PrintTab::PrintTab( PINFOSTR pinf, QWidget *parent)
    : QWidget(parent)
{
    int i;
    QString s;

    indent_spaces = new QLabel("indent-spaces (Integer)");
    i = getConfigInt("indent-spaces");
    s = QString::number(i);
    indent_spacesEd = new QLineEdit(s);
    indent_spacesEd->setMaximumWidth(50);
    connect(indent_spacesEd,SIGNAL(editingFinished()),this,SLOT(on_indent_spacesEd()));

    wrap = new QLabel("wrap (Integer)");
    i = getConfigInt("wrap");
    s = QString::number(i);
    wrapEd = new QLineEdit(s);
    wrapEd->setMaximumWidth(50);
    connect(wrapEd,SIGNAL(editingFinished()),this,SLOT(on_wrapEd()));

    tab_size = new QLabel("tab-size (Integer)");
    i = getConfigInt("tab-size");
    s = QString::number(i);
    tab_sizeEd = new QLineEdit(s);
    tab_sizeEd->setMaximumWidth(50);
    connect(tab_sizeEd,SIGNAL(editingFinished()),this,SLOT(on_tab_sizeEd()));

    markup = new QCheckBox("markup (Boolean)");
    if (getConfigBool("markup")) {
        markup->setChecked(true);
    }
    connect(markup,SIGNAL(clicked()),this,SLOT(on_markup()));


    indent = new QCheckBox("indent (AutoBool)");
    indent->setTristate(true);
    i = getConfigABool("indent");
    if (i == 0) {
        indent->setChecked(false);
        // indent->setCheckState(Qt::Unchecked); // 0
    } else if (i == 1) {
        indent->setChecked(true);
        // indent->setCheckState(Qt::Checked); // 2
    } else {
        indent->setCheckState(Qt::PartiallyChecked); // 1
    }
    connect(indent,SIGNAL(clicked()),this,SLOT(on_indent()));


    break_before_br = new QCheckBox("break-before-br (Boolean)");
    if (getConfigBool("break-before-br")) {
        break_before_br->setChecked(true);
    }
    connect(break_before_br,SIGNAL(clicked()),this,SLOT(on_break_before_br()));


    split = new QCheckBox("split (Boolean)");
    if (getConfigBool("split")) {
        split->setChecked(true);
    }
    connect(split,SIGNAL(clicked()),this,SLOT(on_split()));


    wrap_attributes = new QCheckBox("wrap-attributes (Boolean)");
    if (getConfigBool("wrap-attributes")) {
        wrap_attributes->setChecked(true);
    }
    connect(wrap_attributes,SIGNAL(clicked()),this,SLOT(on_wrap_attributes()));


    wrap_script_literals = new QCheckBox("wrap-script-literals (Boolean)");
    if (getConfigBool("wrap-script-literals")) {
        wrap_script_literals->setChecked(true);
    }
    connect(wrap_script_literals,SIGNAL(clicked()),this,SLOT(on_wrap_script_literals()));


    wrap_sections = new QCheckBox("wrap-sections (Boolean)");
    if (getConfigBool("wrap-sections")) {
        wrap_sections->setChecked(true);
    }
    connect(wrap_sections,SIGNAL(clicked()),this,SLOT(on_wrap_sections()));


    wrap_asp = new QCheckBox("wrap-asp (Boolean)");
    if (getConfigBool("wrap-asp")) {
        wrap_asp->setChecked(true);
    }
    connect(wrap_asp,SIGNAL(clicked()),this,SLOT(on_wrap_asp()));


    wrap_jste = new QCheckBox("wrap-jste (Boolean)");
    if (getConfigBool("wrap-jste")) {
        wrap_jste->setChecked(true);
    }
    connect(wrap_jste,SIGNAL(clicked()),this,SLOT(on_wrap_jste()));


    wrap_php = new QCheckBox("wrap-php (Boolean)");
    if (getConfigBool("wrap-php")) {
        wrap_php->setChecked(true);
    }
    connect(wrap_php,SIGNAL(clicked()),this,SLOT(on_wrap_php()));


    indent_attributes = new QCheckBox("indent-attributes (Boolean)");
    if (getConfigBool("indent-attributes")) {
        indent_attributes->setChecked(true);
    }
    connect(indent_attributes,SIGNAL(clicked()),this,SLOT(on_indent_attributes()));


    vertical_space = new QCheckBox("vertical-space (Boolean)");
    if (getConfigBool("vertical-space")) {
        vertical_space->setChecked(true);
    }
    connect(vertical_space,SIGNAL(clicked()),this,SLOT(on_vertical_space()));


    punctuation_wrap = new QCheckBox("punctuation-wrap (Boolean)");
    if (getConfigBool("punctuation-wrap")) {
        punctuation_wrap->setChecked(true);
    }
    connect(punctuation_wrap,SIGNAL(clicked()),this,SLOT(on_punctuation_wrap()));


    QGroupBox *sort_attributes = new QGroupBox("sort-attributes");
    sort_attributesComb = new QComboBox();
    sort_attributesComb->addItem("none","none");
    sort_attributesComb->addItem("alpha","alpha");
    s = getConfigEnum("sort-attributes");
    i = sort_attributesComb->findData(s);
    if (i != -1) {
        sort_attributesComb->setCurrentIndex(i);
    }
    QVBoxLayout *sort_attributesLay = new QVBoxLayout;
    sort_attributesLay->addWidget(sort_attributesComb);
    sort_attributesLay->addStretch(1);
    sort_attributes->setLayout(sort_attributesLay);
    connect(sort_attributesComb, SIGNAL(currentIndexChanged(int)), this, SLOT(on_sort_attributesComb()));

    indent_with_tabs = new QCheckBox("indent-with-tabs (Boolean)");
    if (getConfigBool("indent-with-tabs")) {
        indent_with_tabs->setChecked(true);
    }
    connect(indent_with_tabs,SIGNAL(clicked()),this,SLOT(on_indent_with_tabs()));


    QVBoxLayout *PrintLayout = new QVBoxLayout;
    PrintLayout->addWidget(indent_spaces);
    PrintLayout->addWidget(indent_spacesEd);
    PrintLayout->addWidget(wrap);
    PrintLayout->addWidget(wrapEd);
    PrintLayout->addWidget(tab_size);
    PrintLayout->addWidget(tab_sizeEd);
    PrintLayout->addWidget(markup);
    PrintLayout->addWidget(indent);
    PrintLayout->addWidget(break_before_br);
    PrintLayout->addWidget(split);
    PrintLayout->addWidget(wrap_attributes);
    PrintLayout->addWidget(wrap_script_literals);
    PrintLayout->addWidget(wrap_sections);
    PrintLayout->addWidget(wrap_asp);
    PrintLayout->addWidget(wrap_jste);
    PrintLayout->addWidget(wrap_php);
    PrintLayout->addWidget(indent_attributes);
    PrintLayout->addWidget(vertical_space);
    PrintLayout->addWidget(punctuation_wrap);
    PrintLayout->addWidget(sort_attributes);
    PrintLayout->addWidget(indent_with_tabs);
    PrintLayout->addStretch(1);
    setLayout(PrintLayout);
}

// END INSERT ///////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////
// Read and process a CONFIG file, then UPDATE all the parameters
// C:\Projects\qt\qt-gui\build\default.cfg
// alt-text:
// show-info: no
// indent: yes
// tidy-mark: no

// Label types: 'Integer' 'String' 'Tag names' 
void ConfigTab::do_configUpdate()
{
    QString s;
    bool b;
    int i, count;
    const char *missing = 0;
    // LineEdit types: 'Integer' 'String' 'Tag names' 
    PLINED pl = &lined[0];
    count = 0;
    while ( pl->p ) {
        QLineEdit *ple = *pl->p;
        if (strcmp(pl->typ,"String") == 0) {
            s = getConfigStg(pl->lab); // like "alt-text"
            ple->setText(s);
        } else if (strcmp(pl->typ,"Integer") == 0) {
            i = getConfigInt(pl->lab);
            s = QString::number(i);
            ple->setText(s);
        } else if (strcmp(pl->typ,"Tag names") == 0) {
            s = getConfigStg(pl->lab);
            ple->setText(s);
        } else {
            if (missing == 0) {
                s = QString("Missing handler for type %1 in LINED!").arg(pl->typ);
                missing = strdup( s.toStdString().c_str() );
            }
        }
        pl++;
    }
    // CheckBox types: 'AutoBool' 'Boolean'
    PCHKBXS pcb = &chkbxs[0];
    while ( pcb->p ) {
        QCheckBox *pc = *pcb->p;
        if (strcmp(pcb->typ,"Boolean") == 0) {
            if (getConfigBool(pcb->lab)) {
                b = true;
            } else {
                b = false;
            }
            pc->setChecked(b);
        } else if (strcmp(pcb->typ,"AutoBool") == 0) {
            i = getConfigABool(pcb->lab);
            if (i == 0) {
                pc->setChecked(false);
                // indent->setCheckState(Qt::Unchecked); // 0
            } else if (i == 1) {
                pc->setChecked(true);
                // indent->setCheckState(Qt::Checked); // 2
            } else {
                pc->setCheckState(Qt::PartiallyChecked); // 1
            }
        } else {
            if (missing == 0) {
                s = QString("Missing handler for type %1 in CHKBXS!").arg(pcb->typ);
                missing = strdup( s.toStdString().c_str() );
            }
        }
        pcb++;
    }
    // ComboBox types: 'DocType' 'Encoding' 'enum' 
    PCOMBOS pcom = &combos[0];
    while (pcom->p) {
        QComboBox *cbp = *pcom->p;
        if (strcmp(pcom->typ,"DocType") == 0) {
            s = getConfigEnum(pcom->lab); 
            i = cbp->findData(s);
            if (i != -1) {
                cbp->setCurrentIndex(i);
            }
        } else if (strcmp(pcom->typ,"Encoding") == 0) {
            s = getConfigEnc(pcom->lab);
            i = cbp->findData(s);
            if (i != -1) {
                cbp->setCurrentIndex(i);
            }
        } else if (strcmp(pcom->typ,"enum") == 0) {
            s = getConfigEnum(pcom->lab); 
            i = cbp->findData(s);
            if (i != -1) {
                cbp->setCurrentIndex(i);
            }
        } else {
            if (missing == 0) {
                s = QString("Missing handler for type %1 in COMBOS!").arg(pcom->typ);
                missing = strdup( s.toStdString().c_str() );
            }
        }
        pcom++; // bump to next
    }
    if (missing) {
        s = missing;
        s.append( "\nOnly the first is shown! There could be others?" );
        s.append( "\n *** FIX ME ***, or at least report an Issue.\n" );
        QMessageBox::warning(this, tr("File is Empty"),s,QMessageBox::Ok);
    }
}

// eof
