/* The following code is directly borrowed from:
 *
 *
 * minigzip.c -- simulate gzip using the zlib compression library
 * Copyright (C) 1995-2006, 2010, 2011 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h
   Copyright (C) 1995-2013 Jean-loup Gailly and Mark Adler

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Jean-loup Gailly        Mark Adler
  jloup@gzip.org          madler@alumni.caltech.edu


  The data format used by the zlib library is described by RFCs (Request for
  Comments) 1950 to 1952 in the files http://tools.ietf.org/html/rfc1950
  (zlib format), rfc1951 (deflate format) and rfc1952 (gzip format).
*/
    

/*
 * minigzip is a minimal implementation of the gzip utility. This is
 * only an example of using zlib and isn't meant to replace the
 * full-featured gzip. No attempt is made to deal with file systems
 * limiting names to 14 or 8+3 characters, etc... Error checking is
 * very limited. So use minigzip only for testing; use gzip for the
 * real thing. On MSDOS, use only on file names without extension
 * or in pipe mode.
 */

/* @(#) $Id$ */

#include "zlib.h"
#include <stdio.h>

#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#endif

#ifdef USE_MMAP
#  include <sys/types.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#endif

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  ifdef UNDER_CE
#    include <stdlib.h>
#  endif
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#ifdef _MSC_VER
#  define snprintf _snprintf
#endif

#ifdef RISCOS
#  define fileno(file) file->__file
#endif
#if defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#endif

#define BUFLEN      16384
#define MAX_NAME_LEN 1024

//#ifdef Z_SOLO
/* for Z_SOLO, create simplified gz* functions using deflate and inflate */

#if defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)
#  include <unistd.h>       /* for unlink() */
#endif

void *myalloc(void *q, unsigned n, unsigned m)
{
    q = Z_NULL;
    return calloc(n, m);
}

void myfree(void *q, void *p)
{
    q = Z_NULL;
    free(p);
}

static void error(const char *msg)
{
    printf("  ERR: %s\n", msg);
}

/* ===========================================================================
 * Compress input to output then close both files.
 */

void gz_compress(FILE *in, gzFile out)
{
    static char buf[BUFLEN];
    int len;
    int err;

    for (;;) {
        len = (int)fread(buf, 1, sizeof(buf), in);
        if (ferror(in)) {
            error("fread failed in gz_compress");
            return;
        }
        if (len == 0) break;

        if (gzwrite(out, buf, (unsigned)len) != len) error(gzerror(out, &err));
    }
    fclose(in);
    if (gzclose(out) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Uncompress input to output then close both files.
 */
void gz_uncompress(in, out)
    gzFile in;
    FILE   *out;
{
    static char buf[BUFLEN];
    int len;
    int err;

    for (;;) {
        len = gzread(in, buf, sizeof(buf));
        if (len < 0) error (gzerror(in, &err));
        if (len == 0) break;

        if ((int)fwrite(buf, 1, (unsigned)len, out) != len) {
            error("failed fwrite");
        }
    }
    if (fclose(out)) error("failed fclose");

    if (gzclose(in) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding compressed file
 */
int file_compress(const char *file, const char *outfile)
{
    const char *mode = "wb6 ";
    FILE  *in;
    gzFile out;

    in = fopen(file, "rb");
    if (in == NULL) {
        printf("  ERR: can't open %s\n", file);
        return -1;
    }
    out = gzopen(outfile, mode);
    if (out == NULL) {
        printf("  ERR: can't gzopen %s\n", outfile);
        return -1;
    }
    gz_compress(in, out);
    
    return 1;
}


/* ===========================================================================
 * Uncompress the given file
 */
int file_uncompress(const char *infile, const char *outfile)
{
    FILE  *out;
    gzFile in;

    in = gzopen(infile, "rb");
    if (in == NULL) {
        printf("  ERR: can't gzopen %s\n", infile);
        return -1;
    }
    out = fopen(outfile, "wb");
    if (out == NULL) {
        printf("  ERR: can't open %s\n", outfile);
        return -1;
    }

    gz_uncompress(in, out);
    return 1;
}
