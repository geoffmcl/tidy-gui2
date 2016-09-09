/*\
 * tg-main.cpp
 *
 * Copyright (c) 2015 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
\*/

#ifndef TABDIALOG_H
#define TABDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QString>
#include <QTextEdit>
#include <QTabWidget>
#include <QTabBar>

typedef struct tabINFOSTR {
    int count;
    QString inputStr;
    QString outputStr;
    QString configStr;
    QString errorStr;
}INFOSTR, *PINFOSTR;

// Hmmm, seems another way to include things - strange
QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QFileInfo;
class QTabWidget;
QT_END_NAMESPACE

#define USE_MYTAB_WIDGET

extern QSettings *m_settings;    // = new QSettings(tmp,QSettings::IniFormat,this);
extern void set_bigEdit( const char *text );
extern void append_bigEdit( const char *text );
extern void set_errEdit( const char *text );
extern void append_errEdit( const char *text );
extern void set_outNameEdit(QString);
// forward refs
extern QString get_error_file();
extern void set_error_file(QString s);

#ifdef USE_MYTAB_WIDGET
/**
 *  Derived Class from QTabWidget.
 *  Protected QTabWidget::tabBar() method is 
 *  overridden in order to make it accessible.
 */
class MyTabWidget:public QTabWidget
{
public:
    MyTabWidget(QWidget* parent = 0)
    {
      setParent(parent);
    }
    
    //Overridden method from QTabWidget
    QTabBar* tabBar()
    {
      return QTabWidget::tabBar();
    }
};
#endif // USE_MYTAB_WIDGET


class TabDialog : public QDialog
{
    Q_OBJECT

public:
    TabDialog(PINFOSTR pinfo, QWidget *parent = 0);
    void closeEvent(QCloseEvent *event);
#ifdef USE_MYTAB_WIDGET
    MyTabWidget *tabWidget;
#else
    QTabWidget *tabWidget;
#endif

public slots:
    void onQuit();
    void onShow();
    void on_buttonTidy();
    void on_about();
    void on_tab_changed();

private:
    QDialogButtonBox *buttonBox;
};

extern TabDialog *m_tabDialog;

////////////////////////////////////////////////
// TABS
class GeneralTab : public QWidget
{
    Q_OBJECT

public:
    GeneralTab( PINFOSTR pinf, QWidget *parent = 0);

public slots:
    void on_fileNameBrowse();
    void on_fileNameEdit();
    void on_outputNameBrowse();
    void on_outputNameEdit();
    void on_configNameBrowse();
    void on_configNameEdit();
};

class OutputTab : public QWidget
{
    Q_OBJECT
public:
    OutputTab( PINFOSTR pinf, QWidget *parent = 0);

public slots:
    void on_butSaveAs();

};

class ConfigTab : public QWidget
{
    Q_OBJECT
public:
    ConfigTab( PINFOSTR pinf, QWidget *parent = 0);
    QDialogButtonBox *cfgbuttonBox;
    void do_configUpdate();
    void loadConfig(QString file, int options = 0 );
    bool saveConfig(QString file, const char *ccp, int options = 0 );

public slots:
    void on_buttonView();
    void on_buttonSaveAs();
    void on_buttonLoad();
    void on_read_only();
    void on_show_detailed();
    void on_show_all();

};

///////////////////////////////////////////////////////



class DiagnosticsTab : public QWidget
{
    Q_OBJECT
public:
    DiagnosticsTab( PINFOSTR pinf, QWidget *parent = 0);

public slots: // ADD to DiagnosticsTab
    void on_show_info();
    void on_show_warnings();
    void on_show_errorsEd();
    void on_accessibility_checkComb();

};

class EncodingTab : public QWidget
{
    Q_OBJECT
public:
    EncodingTab( PINFOSTR pinf, QWidget *parent = 0);

public slots: // ADD to EncodingTab
    void on_char_encodingComb();
    void on_input_encodingComb();
    void on_output_encodingComb();
    void on_newlineComb();
    void on_doctypeComb();
    void on_repeated_attributesComb();
    void on_ascii_chars();
    void on_languageEd();
    void on_output_bom();

};

class MarkupTab : public QWidget
{
    Q_OBJECT
public:
    MarkupTab( PINFOSTR pinf, QWidget *parent = 0);

public slots: // ADD to MarkupTab
    void on_coerce_endtags();
    void on_omit_optional_tags();
    void on_hide_endtags();
    void on_input_xml();
    void on_output_xml();
    void on_output_xhtml();
    void on_output_html();
    void on_add_xml_decl();
    void on_uppercase_tags();
    void on_uppercase_attributes();
    void on_bare();
    void on_clean();
    void on_gdoc();
    void on_logical_emphasis();
    void on_drop_proprietary_attributes();
    void on_drop_font_tags();
    void on_drop_empty_elements();
    void on_drop_empty_paras();
    void on_fix_bad_comments();
    void on_numeric_entities();
    void on_quote_marks();
    void on_quote_nbsp();
    void on_quote_ampersand();
    void on_fix_backslash();
    void on_assume_xml_procins();
    void on_add_xml_space();
    void on_enclose_text();
    void on_enclose_block_text();
    void on_word_2000();
    void on_literal_attributes();
    void on_show_body_only();
    void on_fix_uri();
    void on_lower_literals();
    void on_hide_comments();
    void on_indent_cdata();
    void on_join_classes();
    void on_join_styles();
    void on_escape_cdata();
    void on_ncr();
    void on_replace_color();
    void on_merge_emphasis();
    void on_merge_divs();
    void on_decorate_inferred_ul();
    void on_preserve_entities();
    void on_merge_spans();
    void on_anchor_as_name();
    void on_skip_nested();
    void on_strict_tags_attributes();

};

class MiscTab : public QWidget
{
    Q_OBJECT
public:
    MiscTab( PINFOSTR pinf, QWidget *parent = 0);

public slots: // ADD to MiscTab
    void on_alt_textEd();
    void on_slide_styleEd();
    void on_error_fileEd();
    void on_write_back();
    void on_quiet();
    void on_keep_time();
    void on_tidy_mark();
    void on_gnu_emacs();
    void on_gnu_emacs_fileEd();
    void on_force_output();
    void on_css_prefixEd();
    void on_new_inline_tagsEd();
    void on_new_blocklevel_tagsEd();
    void on_new_empty_tagsEd();
    void on_new_pre_tagsEd();

};

class PrintTab : public QWidget
{
    Q_OBJECT
public:
    PrintTab( PINFOSTR pinf, QWidget *parent = 0);

public slots: // ADD to PrintTab
    void on_indent_spacesEd();
    void on_wrapEd();
    void on_tab_sizeEd();
    void on_markup();
    void on_indent();
    void on_break_before_br();
    void on_split();
    void on_wrap_attributes();
    void on_wrap_script_literals();
    void on_wrap_sections();
    void on_wrap_asp();
    void on_wrap_jste();
    void on_wrap_php();
    void on_indent_attributes();
    void on_vertical_space();
    void on_punctuation_wrap();
    void on_sort_attributesComb();
    void on_indent_with_tabs();
    void on_escape_scripts();

};

////////////////////////////////////////////////////////

#endif
