/* Prism Versioning
 * Copyright 2014 Jeffrey Armstrong <jeff@rainbow-100.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __GNUC__
#include <io.h>
#endif

#include "prism.h"
#include "md5.h"
#include "rfile.h"
#include "compress.h"
#include "fcomp.h"

static const char hex_str[]= "0123456789abcdef";

int get_archive_name(char *filename, int maxfile, int id, int revision)
{
    if(filename == NULL)
        return PRET_ERROR;
       
    snprintf(filename, maxfile, "%s%s%x.%x", PDB_DIR, DIR_SEP, id, revision);
    return strlen(filename);
}

int get_file_hash(const char *filename, char *hash)
{
FILE *fp;
MD5_CTX hb;
char buf[16];
int i;

    if(filename == NULL || hash == NULL)
        return PRET_ERROR;
        
    fp = fopen(filename, "rb");
    if(fp == NULL)
        return PRET_READERROR;
    
    MD5_Init(&hb);
    while((i = fread(buf, sizeof(char), 15, fp)) > 0) {
        MD5_Update(&hb, buf, i);
    }
    fclose(fp);
    
    MD5_Final(buf, &hb);
    
    memset(hash, 0, HASH_BYTES);
    
    for(i=0; i<16; i++) {
        hash[2*i] = hex_str[buf[i] >> 4];
        hash[2*i+1] = hex_str[buf[i] & 0x0F];
    }
    
    return PRET_OK;
}    

int compress_file(const char *filename, int id, int revision)
{
char *newfile;

    /* Arbitrary size... */
    newfile = (char *)malloc(128*sizeof(char));
    if(newfile == NULL)
        return PRET_ERROR;
    get_archive_name(newfile, 128, id, revision);
    
    if(file_compress(filename, newfile) >= 0)
        return PRET_OK;
    
    return PRET_GZFAILED;
}

int decompress_file(const char *destination, int id, int revision)
{
char *stored_name;

    /* Arbitrary size... */
    stored_name = (char *)malloc(128*sizeof(char));
    if(stored_name == NULL)
        return PRET_ERROR;
    get_archive_name(stored_name, 128, id, revision);
    
    unlink(destination);
    
    if(file_uncompress(stored_name, destination) >= 0)
        return PRET_OK;
    
    return PRET_GZFAILED;
}

int diff_file(const char *working, int id, int revision)
{
int res;
int namelength;
char *rev_name;
char *extension;
    
    if(working == NULL) return PRET_ERROR;
    namelength = strlen(working)+strlen(PDB_DIR)+strlen(DIR_SEP)+16;
    rev_name = (char *)malloc(namelength*sizeof(char));
    if(rev_name == NULL)
        return PRET_ERROR;
    
    snprintf(rev_name, namelength, "%s%s%s", PDB_DIR, DIR_SEP, working);
    extension = strrchr(rev_name, '.');
    if(extension != NULL) extension[0] = '\0';

#if defined(MSDOS) || defined(WIN32)    
    snprintf(rev_name, namelength, "%s.%.3x", rev_name, revision);
#else
    snprintf(rev_name, namelength, "%s.%x", rev_name, revision);
#endif
    
    /* Decompress the archived version to a temporary file */
    res = decompress_file(rev_name, id, revision);
    if(res != PRET_OK) {
        free(rev_name);
        return res;
    }
    
    /* Perform the comparison */
    res = compare_files(rev_name, working);
    
    unlink(rev_name);
    free(rev_name);
    
    return res;
}
