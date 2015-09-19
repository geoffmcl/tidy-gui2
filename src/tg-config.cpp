/*\
 * tg-config.cpp
 *
 * Copyright (c) 2015 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
\*/

#include <QtGlobal> 
#include <QString>
#include <QFile>
#include <QTextStream>
#include <stdio.h>
#include <string.h>
#include <tidy.h>
#include <tidybuffio.h>
#include "tg-dialog.h"
#include "tg-config.h"
/*\
 * This module deals with all thing Tidy
 *
 * INIT and CLOSE library
 * GET and SET config items as they change
 * 
 * Setting the IO
 * Initialize a buffer input source
 * TIDY_EXPORT void TIDY_CALL tidyInitInputBuffer( TidyInputSource* inp, TidyBuffer* buf );
 * Initialize a buffer output sink 
 * TIDY_EXPORT void TIDY_CALL tidyInitOutputBuffer( TidyOutputSink* outp, TidyBuffer* buf );
 *
 *
 *
\*/

/*\
 * API Sample Code
#include <tidy.h>;
#include <tidybuffio.h>;
#include <stdio.h>;
#include <errno.h>;
int main(int argc, char **argv )
{
    const char* input = "<title>Hello</title><p>World!";
    TidyBuffer output = {0};
    TidyBuffer errbuf = {0};
    int rc = -1;
    Bool ok;
    // Initialize "document"
    TidyDoc tdoc = tidyCreate();
    printf( "Tidying:\t%s\n", input );
    // Convert to XHTML
    ok = tidyOptSetBool( tdoc, TidyXhtmlOut, yes );  
    if ( ok )
        rc = tidySetErrorBuffer( tdoc, &errbuf );    // Capture diagnostics
    if ( rc >= 0 )
        rc = tidyParseString( tdoc, input );         // Parse the input
    if ( rc >= 0 )
        rc = tidyCleanAndRepair( tdoc );             // Tidy it up!
    if ( rc >= 0 )
        rc = tidyRunDiagnostics( tdoc );             // Kvetch
    if ( rc > 1 )                                    // If error, force output.
        rc = ( tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1 );
    if ( rc >= 0 )
        rc = tidySaveBuffer( tdoc, &output );        // Pretty Print
    if ( rc >= 0 )
    {
    if ( rc > 0 )
        printf( "\nDiagnostics:\n\n%s", errbuf.bp );
    printf( "\nAnd here is the result:\n\n%s", output.bp );
    }
    else
        printf( "A severe error (%d) occurred.\n", rc );
    tidyBufFree( &output );
    tidyBufFree( &errbuf );
    tidyRelease( tdoc );
    return rc;
}

\*/

static const char *module = "tg-config";

static TidyDoc tdoc = 0;    // tidyCreate();
static TidyBuffer output;
static TidyBuffer errbuf;
static TidyBuffer cfgbuf;

Bool initBuffers()
{
    Bool done = no;
    tidyBufInit( &output );
    tidyBufInit( &errbuf );
    tidyBufInit( &cfgbuf );
    if (tidySetErrorBuffer( tdoc, &errbuf ) >= 0) {    // Capture diagnostics
        done = yes;
    }
    return done;
}

void clearBuffers( int flag )
{
    if ((flag == 0) || (flag & 1))
        tidyBufClear( &errbuf );
    if ((flag == 0) || (flag & 2))
        tidyBufClear( &output );
    if ((flag == 0) || (flag & 4))
        tidyBufClear( &cfgbuf );
}

char *getErrBufText() 
{
    if (errbuf.bp && *errbuf.bp)
        return (char *)errbuf.bp;
    return 0;
}

char *getOutBufText() 
{
    if (output.bp && *output.bp)
        return (char *)output.bp;
    return 0;
}

char *getCfgBufText() 
{
    if (cfgbuf.bp && *cfgbuf.bp)
        return (char *)cfgbuf.bp;
    return 0;
}


Bool openTidyLib()
{
    Bool done = no;
    if (tdoc == 0) {
        tdoc = tidyCreate();
        done = initBuffers();
    }
    return done;
}

void closeTidyLib()
{
    if (tdoc)  {

        clearBuffers(); // release buffer memory - maybe tidyBufFree() also does this???

        // free buffer memory
        tidyBufFree( &output );
        tidyBufFree( &errbuf );
        tidyBufFree( &cfgbuf );

        tidyRelease( tdoc ); /* called to free hash tables etc. */
    }
    tdoc = 0;
}

static char libver[256];
const char *getLibVersion()
{
    char *ccp = libver;
    QString v = tidyLibraryVersion();
    v = v.trimmed();
    strcpy(ccp,v.toStdString().c_str());
    return ccp;
}
static char libdate[256];
const char *getLibDate()
{
    char *ccp = libdate;
    QString v = tidyReleaseDate();
    v = v.trimmed();
    strcpy(ccp,v.toStdString().c_str());
    return ccp;
}

int runTidyLib( const char *file )
{
    int rc;

    clearBuffers();

    rc = tidyParseFile( tdoc, file );
    if ( rc >= 0 )
        rc = tidyCleanAndRepair( tdoc );             // Tidy it up!
    if ( rc >= 0 )
        rc = tidyRunDiagnostics( tdoc );             // Kvetch
    if ( rc > 1 )                                    // If error, force output.
        rc = ( tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1 );
    if ( rc >= 0 )
        rc = tidySaveBuffer( tdoc, &output );        // Pretty Print
    
    QString msg = QString("Diagnotics: file %1 (%2)\n").arg(file, QString::number(rc));
    if (errbuf.bp) {
        set_errEdit(msg.toStdString().c_str());
        append_errEdit( (const char *)errbuf.bp );
        QString f = get_error_file();
        if (f.size()) {
            QFile efile( f );
            if ( efile.open(QIODevice::ReadWrite | QIODevice::Append |  QIODevice::Text) ) {
                QTextStream stream( &efile );
                msg.append( (const char *)errbuf.bp );
                stream << msg << endl;
                efile.close();
                msg = QString("Appended to %1\n").arg(f);
                append_errEdit( msg.toStdString().c_str() );
                set_error_file(f);  // set and write to persistent
            } else {
                msg = QString("UNable to appended to %1\n").arg(f);
                append_errEdit( msg.toStdString().c_str() );
            }
        }
    }

    msg = QString("Results: file %1 (%2)\n").arg(file,QString::number(rc));
    set_outNameEdit(msg);
    if (output.bp) {
        set_bigEdit( (const char *)output.bp );
    }
    return rc;
}

///////////////////////////////////////////////////////
// Config stuff
///////////////////////////////////////////////////////

TidyOptionId getTidyOptionId( const char *item )
{
    return tidyOptGetIdForName(item);
}

// implementation
bool getConfigBool( const char *item )
{
    TidyOptionId id = getTidyOptionId(item);
    if (id < N_TIDY_OPTIONS)
        return tidyOptGetBool(tdoc, id);
    return false;
}

Bool setConfigBool( const char *item, Bool val )
{
    Bool done = no;
    TidyOptionId id = getTidyOptionId(item);
    if (id < N_TIDY_OPTIONS) {
        Bool curr = tidyOptGetBool(tdoc, id);
        if (curr != val) {
            done = tidyOptSetBool(tdoc, id, val);
        }
    }
    return done;
}


// TODO: CHECK AutoBool to triSate mappings
// Tidy AutoBool has no(0), yes(1), auto(2)
// Qt4  triState has no(0), yes(2), part(1)
TidyTriState getConfigABool( const char *item )
{
    TidyOptionId id = getTidyOptionId(item);
    if (id < N_TIDY_OPTIONS)
        return (TidyTriState)tidyOptGetInt(tdoc,id);
        // return cfgAutoBool(tdoc, id);
    return TidyAutoState;
}

Bool setConfigABool( const char *item, Bool val )
{
    Bool done = no;
    TidyOptionId id = getTidyOptionId(item);
    if (id < N_TIDY_OPTIONS) {
        Bool curr = (Bool)tidyOptGetInt(tdoc,id);
        if (curr != val) {
            done = tidyOptSetInt(tdoc,id,val);
        }
    }
    return done;
}


int getConfigInt( const char *item )
{
    TidyOptionId id = getTidyOptionId(item);
    if (id < N_TIDY_OPTIONS)
         return tidyOptGetInt(tdoc,id);

    if (strcmp(item,"indent-spaces") == 0) {
        return 2;
    } else if (strcmp(item,"wrap") == 0) {
        return 68;
    } else if (strcmp(item,"tab-size") == 0) {
        return 8;
    } else if (strcmp(item,"show-errors") == 0) {
        return 6;
    } 
    return 0;
}

Bool setConfigInt( const char *item, int val )
{
    Bool done = no;
    TidyOptionId id = getTidyOptionId(item);
    if (id < N_TIDY_OPTIONS) {
        int curr = tidyOptGetInt(tdoc,id);
        if (curr != val) {
            done = tidyOptSetInt(tdoc,id,val);
        }
    }
    return done;
}

const char *getConfigStg( const char *item )
{
    TidyOptionId id = getTidyOptionId(item);
    if (id < N_TIDY_OPTIONS)
         return tidyOptGetValue(tdoc,id);
    return "";
}

Bool setConfigStg( const char *item, const char *stg )
{
    Bool done = no;
    TidyOptionId id = getTidyOptionId(item);
    if (id < N_TIDY_OPTIONS) {
        const char *curr = tidyOptGetValue(tdoc,id);
        if (curr) {
            // have a value, but only set if different
            if (strcmp(curr,stg)) {
                done = tidyOptSetValue(tdoc,id,stg);
            }
        } else {
            // presently NO VALUE, set ONLY if there is LENGTH now
            if (strlen(stg)) {
                done = tidyOptSetValue(tdoc,id,stg);
            }
        }
    }
    return done;
}

Bool setConfigStg_BAD( const char *item, const char *stg )
{
    Bool done = no;
    TidyOptionId id = getTidyOptionId(item);
    if (id < N_TIDY_OPTIONS) {
        const char *curr = tidyOptGetValue(tdoc,id);
        if (!curr || strcmp(curr,stg)) {
            done = tidyOptSetValue(tdoc,id,stg);
        }
    }
    return done;
}


const char *getConfigEnc( const char *item )
{
    TidyOptionId id = getTidyOptionId(item);
    if (id < N_TIDY_OPTIONS)
         return tidyOptGetEncName(tdoc,id);
    if (strcmp(item,"char-encoding") == 0) {
        return "utf8";
    } else if (strcmp(item,"input-encoding") == 0) {
        return "utf8";
    } else if (strcmp(item,"output-encoding") == 0) {
        return "utf8";
    } else if (strcmp(item,"newline") == 0) {
#ifdef Q_WS_WIN
        return "CRLF";
#else
#ifdef  Q_WS_MAC
        return "LF";
#else
        return "CR";
#endif
#endif
    }
    return "";
}

Bool setConfigEnc( const char *item, const char *val )
{
    Bool done = no;
    const char *curr = 0;
    TidyOptionId id = getTidyOptionId(item);
    if (id < N_TIDY_OPTIONS) {
         curr = tidyOptGetEncName(tdoc,id);
         if (strcmp(curr,item)) {
             done = tidyOptSetValue(tdoc,id,val);
         }
    }
    return done;
}

const char *getConfigEnum( const char *item )
{
    TidyOptionId id = getTidyOptionId(item);
    if (id < N_TIDY_OPTIONS)
         return tidyOptGetCurrPick(tdoc,id);
    return "";
}

Bool setConfigEnum( const char *item, const char *val )
{
    Bool done = no;
    const char *curr = 0;
    TidyOptionId id = getTidyOptionId(item);
    if (id < N_TIDY_OPTIONS) {
         curr = tidyOptGetCurrPick(tdoc,id);
         if (strcmp(curr,item)) {
             done = tidyOptSetValue(tdoc,id,val);
         }
    }
    return done;
}

const char *getConfigPick( const char *item )
{
    TidyOptionId id = getTidyOptionId(item);
    if (id < N_TIDY_OPTIONS)
         return tidyOptGetCurrPick(tdoc,id);
    return "";
}

///////////////////////////////////////////////////////////////
/* Description of an option */
typedef struct {
    ctmbstr name;  /**< Name */
    ctmbstr cat;   /**< Category */
    ctmbstr type;  /**< "String, ... */
    ctmbstr vals;  /**< Potential values. If NULL, use an external function */
    ctmbstr def;   /**< default */
    tmbchar tempdefs[80]; /**< storage for default such as integer */
    Bool haveVals; /**< if yes, vals is valid */
} OptionDesc;

typedef void (*OptionFunc)( TidyDoc, TidyOption, OptionDesc * );

static const char fmt[] = "%-27.27s %-9.9s  %-40.40s";
static const char valfmt[] = "%-27.27s %-9.9s %-1.1s%-39.39s";
static const char ul[]
        = "=================================================================";

static ctmbstr ConfigCategoryName( TidyConfigCategory id, ctmbstr name, int count )
{
    switch( id )
    {
    case TidyMarkup:
        return "markup";
    case TidyDiagnostics:
        return "diagnostics";
    case TidyPrettyPrint:
        return "print";
    case TidyEncoding:
        return "encoding";
    case TidyMiscellaneous:
        return "misc";
    }
    fprintf(stderr, "\nFatal error: impossible value for id='%d'! count %d on %d\n\n", (int)id, count, N_TIDY_OPTIONS );
    assert(0);
    abort();
    return "never_here"; /* only for the compiler warning */
}

static Bool isAutoBool( TidyOption topt )
{
    TidyIterator pos;
    ctmbstr def;

    if ( tidyOptGetType( topt ) != TidyInteger)
        return no;

    pos = tidyOptGetPickList( topt );
    while ( pos )
    {
        def = tidyOptGetNextPick( topt, &pos );
        if (0==strcmp(def,"yes"))
           return yes;
    }
    return no;
}

/* Create description "d" related to "opt" */
static void GetOption( TidyDoc tdoc, TidyOption topt, OptionDesc *d, int count )
{
    TidyOptionId optId = tidyOptGetId( topt );
    TidyOptionType optTyp = tidyOptGetType( topt );
    TidyConfigCategory cat = tidyOptGetCategory( topt );

    if (cat == (TidyConfigCategory)-1) {
        fprintf(stderr, "\nUnable to get 'category' for option! count %d on %d\n", count, N_TIDY_OPTIONS);
        exit(2);
    }

    d->name = tidyOptGetName( topt );
    d->cat = ConfigCategoryName( cat, d->name, count );
//        ((d->name && *d->name) ? d->name : "NONAME!"), count );
    d->vals = NULL;
    d->def = NULL;
    d->haveVals = yes;

    /* Handle special cases first.
     */
    switch ( optId )
    {
    case TidyDuplicateAttrs:
    case TidySortAttributes:
    case TidyNewline:
    case TidyAccessibilityCheckLevel:
        d->type = "enum";
        d->vals = NULL;
        d->def =
            optId==TidyNewline ?
            "<em>Platform dependent</em>"
            :tidyOptGetCurrPick( tdoc, optId );
        break;

    case TidyDoctype:
        d->type = "DocType";
        d->vals = NULL;
        {
            ctmbstr sdef = NULL;
            sdef = tidyOptGetCurrPick( tdoc, TidyDoctypeMode );
            if ( !sdef || *sdef == '*' )
                sdef = tidyOptGetValue( tdoc, TidyDoctype );
            d->def = sdef;
        }
        break;

    case TidyInlineTags:
    case TidyBlockTags:
    case TidyEmptyTags:
    case TidyPreTags:
        d->type = "Tag names";
        d->vals = "tagX, tagY, ...";
        d->def = NULL;
        break;

    case TidyCharEncoding:
    case TidyInCharEncoding:
    case TidyOutCharEncoding:
        d->type = "Encoding";
        d->def = tidyOptGetEncName( tdoc, optId );
        if (!d->def)
            d->def = "?";
        d->vals = NULL;
        break;

        /* General case will handle remaining */
    default:
        switch ( optTyp )
        {
        case TidyBoolean:
            d->type = "Boolean";
            d->vals = "y/n, yes/no, t/f, true/false, 1/0";
            d->def = tidyOptGetCurrPick( tdoc, optId );
            break;

        case TidyInteger:
            if (isAutoBool(topt))
            {
                d->type = "AutoBool";
                d->vals = "auto, y/n, yes/no, t/f, true/false, 1/0";
                d->def = tidyOptGetCurrPick( tdoc, optId );
            }
            else
            {
                uint idef;
                d->type = "Integer";
                if ( optId == TidyWrapLen )
                    d->vals = "0 (no wrapping), 1, 2, ...";
                else
                    d->vals = "0, 1, 2, ...";

                idef = tidyOptGetInt( tdoc, optId );
                sprintf(d->tempdefs, "%u", idef);
                d->def = d->tempdefs;
            }
            break;

        case TidyString:
            d->type = "String";
            d->vals = NULL;
            d->haveVals = no;
            d->def = tidyOptGetValue( tdoc, optId );
            break;
        }
    }
}

/* Array holding all options. Contains a trailing sentinel. */
typedef struct tagALLOPTIONS {
    TidyOption topt[N_TIDY_OPTIONS];
} ALLOPTIONS, *PALLOPTIONS;

static
int cmpOpt(const void* e1_, const void *e2_)
{
    const TidyOption* e1 = (const TidyOption*)e1_;
    const TidyOption* e2 = (const TidyOption*)e2_;
    return strcmp(tidyOptGetName(*e1), tidyOptGetName(*e2));
}

static
void getSortedOption( TidyDoc tdoc, PALLOPTIONS tOption )
{
    TidyIterator pos = tidyGetOptionList( tdoc );
    uint i = 0;

    while ( pos )
    {
        TidyOption topt = tidyGetNextOption( tdoc, &pos );
        tOption->topt[i] = topt;
        ++i;
    }
    tOption->topt[i] = NULL; /* sentinel */

    /* Do not sort the sentinel: hence `-1' */
    size_t one = sizeof(tOption->topt[0]);
    size_t all = sizeof(tOption->topt);
    size_t len = (all/one) - 1;
    qsort(tOption->topt, len, one, cmpOpt);

}

static void ForEachSortedOption( TidyDoc tdoc, OptionFunc OptionPrint )
{
    ALLOPTIONS tOption;
    const TidyOption *topt;
    int count = 0;

    getSortedOption( tdoc, &tOption );

    for( topt = tOption.topt; *topt; ++topt)
    {
        OptionDesc d;
        count++;
        GetOption( tdoc, *topt, &d, count );
        (*OptionPrint)( tdoc, *topt, &d );
        if (count >= N_TIDY_OPTIONS) {
            // actually should NOT get here, but seem to in unix????
            break;
        }
    }
}

//static QString all_opts;
static char tmp_buf[1024];

static
void printOptionValues( TidyDoc ARG_UNUSED(tdoc), TidyOption topt,
                        OptionDesc *d )
{
    char *cp = tmp_buf;
    uint len;
    TidyOptionId optId = tidyOptGetId( topt );
    ctmbstr ro = "";
    if (tidyOptIsReadOnly( topt )) {
        ro = "*";
        return;
    }

    switch ( optId )
    {
    case TidyInlineTags:
    case TidyBlockTags:
    case TidyEmptyTags:
    case TidyPreTags:
        {
            TidyIterator pos = tidyOptGetDeclTagList( tdoc );
            while ( pos )
            {
                d->def = tidyOptGetNextDeclTag(tdoc, optId, &pos);
                if ( pos )
                {
                    if ( *d->name ) {
                        sprintf(cp, valfmt, d->name, d->type, ro, d->def );
                    } else {
                        sprintf(cp, fmt, d->name, d->type, d->def );
                    }
                    //all_opts.append(cp);
                    strcat(cp,MEOL);
                    len = strlen(cp);
                    if (len)
                        tidyBufAppend( &cfgbuf, cp, len );
                    d->name = "";
                    d->type = "";
                }
            }
        }
        break;
    case TidyNewline:
        d->def = tidyOptGetCurrPick( tdoc, optId );
        break;
    default:
        break;
    }

    /* fix for http://tidy.sf.net/bug/873921 */
    if ( *d->name || *d->type || (d->def && *d->def) )
    {
        if ( ! d->def )
            d->def = "";
        if ( *d->name ) {
            sprintf( cp, valfmt, d->name, d->type, ro, d->def );
        } else {
            sprintf( cp, fmt, d->name, d->type, d->def );
        }
        //all_opts.append(cp);
        strcat(cp,MEOL);
        len = strlen(cp);
        if (len)
            tidyBufAppend( &cfgbuf, cp, len );

    } else {
        printf("WHAT IS THIS!!!\n");
    }
}

static
void printOptionValues2( TidyDoc ARG_UNUSED(tdoc), TidyOption topt,
                        OptionDesc *d )
{
    char *cp = tmp_buf;
    uint len;
    TidyOptionId optId = tidyOptGetId( topt );
    ctmbstr ro = "";
    if (tidyOptIsReadOnly( topt )) {
        ro = "*";
        return;
    }

    switch ( optId )
    {
    case TidyInlineTags:
    case TidyBlockTags:
    case TidyEmptyTags:
    case TidyPreTags:
        {
            TidyIterator pos = tidyOptGetDeclTagList( tdoc );
            while ( pos )
            {
                d->def = tidyOptGetNextDeclTag(tdoc, optId, &pos);
                if ( pos )
                {
                    if ( *d->name ) {
                        sprintf(cp, "%s: %s", d->name, d->def );
                    } else {
                        sprintf(cp, "%s: %s", d->name, d->def );
                    }
                    //all_opts.append(cp);
                    strcat(cp,MEOL);
                    len = strlen(cp);
                    if (len)
                        tidyBufAppend( &cfgbuf, cp, len );
                    d->name = "";
                    d->type = "";
                }
            }
        }
        break;
    case TidyNewline:
        d->def = tidyOptGetCurrPick( tdoc, optId );
        break;
    default:
        break;
    }

    /* fix for http://tidy.sf.net/bug/873921 */
    if ( *d->name || *d->type || (d->def && *d->def) )
    {
        if ( ! d->def )
            d->def = "";
        if ( *d->name ) {
            sprintf( cp, "%s: %s", d->name, d->def );
        } else {
            sprintf( cp, "%s: %s", d->name, d->def );
        }
        //all_opts.append(cp);
        strcat(cp,MEOL);
        len = strlen(cp);
        if (len)
            tidyBufAppend( &cfgbuf, cp, len );

    } else {
        printf("WHAT IS THIS!!!\n");
    }
}

typedef struct tagSINKDATA {
    int context;
    QString text;
}SINKDATA, *PSINKDATA;

// TIDY_EXPORT int TIDY_CALL         tidyOptSaveSink( TidyDoc tdoc, TidyOutputSink* sink );
static TidyOutputSink sink;
// static SINKDATA sinkdata;
static void TIDY_CALL putByteFunc(void* sinkData, byte bt )
{
    // do something with the byte
    if (sinkData && bt) {
        PSINKDATA psd = (PSINKDATA)sinkData;
        psd->text.append(bt);
        //printf("%c",bt);
    }
}

int showConfig()
{
    int iret = 1;
    PSINKDATA psd = new SINKDATA;
    psd->context = 1;
    psd->text = "";
    if (tidyInitSink( &sink, psd, &putByteFunc )) {
        iret = tidyOptSaveSink(tdoc, &sink);
        if (psd->text.size()) {
            set_errEdit( "Display of configuration items not equal default...\n" ); 
            append_errEdit( psd->text.toStdString().c_str() );
        } else {
            set_errEdit( "All configuration items equal default!\n" ); 
        }
    } else {
        set_errEdit("Oops! internal error: tidyInitSink() FAILED\n");
    }
    return iret;
}

typedef struct tagSINKDATA2 {
    int context;
    TidyBuffer *tbp;
}SINKDATA2, *PSINKDATA2;

static void TIDY_CALL putByteFunc2(void* sinkData, byte bt )
{
    // do something with the byte
    if (sinkData && bt) {
        PSINKDATA2 psd2 = (PSINKDATA2)sinkData;
        tidyBufPutByte( psd2->tbp, bt );
    }
}


const char *get_all_options(bool show_all, bool detailed)
{
    tidyBufInit( &cfgbuf );
    if (show_all) {
        ForEachSortedOption( tdoc, 
            ( detailed ? printOptionValues : printOptionValues2 ) );
    } else {
        int iret;
        SINKDATA2 sd2;
        sd2.context = 2;
        sd2.tbp = &cfgbuf;
        if (tidyInitSink( &sink, &sd2, &putByteFunc2 )) {
            iret = tidyOptSaveSink(tdoc, &sink);
            if ( !(cfgbuf.bp && strlen((const char *)cfgbuf.bp)) ) {
                const char *err1 = "Oops! All configuration items are equal default!" MEOL;
                tidyBufAppend(&cfgbuf,(void *)err1,strlen(err1));
            }
        } else {
            const char *err2 = "Oops! internal error: tidyInitSink() FAILED" MEOL;
            tidyBufAppend(&cfgbuf,(void *)err2,strlen(err2));
        }
    }
    return (const char *)cfgbuf.bp;
}


int load_config( const char *file )
{
    int iret = 0;

    tidyBufClear( &errbuf );
    // if (tidySetErrorBuffer( tdoc, &errbuf ) >= 0) {    // Capture diagnostics

    iret = tidyLoadConfig( tdoc, file );
    return iret;
}


// eof = tg-conifg.cpp
