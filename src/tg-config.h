/*\
 * tg-config.h
 *
 * Copyright (c) 2014 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
\*/

#ifndef _TABCONFIG_H_
#define _TABCONFIG_H_
#include <tidy.h>

#if (defined(_WIN32) || defined(WIN32))
#define MEOL "\r\n"
#else
#define MEOL "\n"
#endif


extern Bool openTidyLib();  // init library
extern void closeTidyLib(); // close libray
extern void runTidyLib( const char *file ); // tidy a file

extern void clearBuffers(int flag = 0); // 0=all, 1=errbuf, 2=outbuf, 4=cfgbuf
extern char *getErrBufText();
extern char *getOutBufText(); 
extern char *getCfgBufText();

extern bool getConfigBool( const char *item );
extern int getConfigInt( const char *item );
extern const char *getConfigStg( const char *item );
extern const char *getConfigEnc( const char *item );
extern const char *getConfigEnum( const char *item );
extern const char *getConfigPick( const char *item );
extern TidyTriState getConfigABool( const char *item );

extern Bool setConfigEnc( const char *item, const char *val );   // set encoding
extern Bool setConfigEnum( const char *item, const char *val );  // set enum and doctype
extern Bool setConfigBool( const char *item, Bool val );
extern Bool setConfigABool( const char *item, Bool val );
extern Bool setConfigStg( const char *item, const char *stg );
extern Bool setConfigInt( const char *item, int val );


extern int showConfig();
extern const char *get_all_options(bool,bool);
extern int load_config( const char *file );


#endif // #ifndef _TABCONFIG_H_
// eof - tabconfig.h
