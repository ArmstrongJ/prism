#ifndef _PRISM_HDRS_
#define _PRISM_HDRS_

#define VERSION     "0.1"

#define PDB_DIR     "PV"

#define PRET_OK          0
#define PRET_ERROR      -1
#define PRET_EXISTS     -2
#define PRET_PERMISSION -3
#define PRET_READERROR  -4
#define PRET_WRITEERROR -5
#define PRET_UPDATEFAIL -6
#define PRET_NOTTRACKED -7
#define PRET_GZFAILED   -8
#define PRET_NOREVISION -10

#if defined(MSDOS) || defined(WIN32) || defined(atarist)
#define DIR_SEP     "\\"

#elif defined(__unix) || defined(__linux) || defined(unix)
#define DIR_SEP     "/"

#elif defined(__riscos) || defined(__riscos__) || defined(vms)
#define DIR_SEP     "."

#else
#define DIR_SEP     "/"
#warning "Making assumptions about directory separator '/'"

#endif

#endif /* _PRISM_HDRS_ */
