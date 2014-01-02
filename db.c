/* Prism Versioning
 * Copyright 2014 Jeffrey Armstrong <jeff@rainbow-100.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <direct.h>
#include <errno.h>
#include <io.h>


#include "prism.h"

#define DB_FILENAME     PDB_DIR DIR_SEP "db.txt"
#define MSG_FILENAME    PDB_DIR DIR_SEP "message.txt"

#define DB_TEMPNAME     PDB_DIR DIR_SEP "db.tmp"

static int temp_to_permanent()
{
struct stat fileres;
int res;
    
    res = stat(DB_TEMPNAME, &fileres);
    if(res != 0)
        return PRET_OK;
    
    res = stat(DB_FILENAME, &fileres);
    if(res == 0) {
        if(unlink(DB_FILENAME) != 0)
            return PRET_UPDATEFAIL;
    }
        
    if(rename(DB_TEMPNAME, DB_FILENAME) != 0)
        return PRET_UPDATEFAIL;
        
    return PRET_OK;
}

int initdb()
{
FILE *fpdb;
FILE *fpmsg;

    if(mkdir(PDB_DIR) != 0) {
        if(errno == EEXIST)
            return PRET_EXISTS;
        else if(errno == EACCES)
            return PRET_PERMISSION;
        else
            return PRET_ERROR;
    }

    fpdb = fopen(DB_FILENAME, "w");
    fprintf(fpdb, "0\n");
    fclose(fpdb);
    
    fpmsg = fopen(MSG_FILENAME, "w");
    fprintf(fpmsg, "0\tInitialized\n");
    fclose(fpmsg);
    
    return PRET_OK;
}

int get_revision()
{
FILE *fpdb;
char *line;
char *numend;
int rev;

    fpdb = fopen(DB_FILENAME, "r");
    if(fpdb == NULL)
        return PRET_ERROR;
    
    line = (char *)malloc(32*sizeof(char));
    if(line == NULL)
        return PRET_ERROR;
    
    if(fgets(line, 32, fpdb) == NULL)
        return PRET_ERROR;
    
    fclose(fpdb);
    
    rev = strtol(line,&numend,16);
    
    free(line);
    return rev;
}

int get_file_count()
{
FILE *fpdb;
char *line;
int ret;

    fpdb = fopen(DB_FILENAME, "r");
    if(fpdb == NULL)
        return PRET_ERROR;
    
    line = (char *)malloc(512*sizeof(char));
    if(line == NULL)
        return PRET_ERROR;
        
    ret = -1;
    while(fgets(line, 512, fpdb) != NULL) ret++;
    fclose(fpdb);
    
    free(line);
    return ret;
}

int increment_revision()
{
int rev;
FILE *fpdb;
FILE *tempdb;
char *line;

    rev = get_revision();
    if(rev < 0)
        return rev;

    fpdb = fopen(DB_FILENAME, "r");
    if(fpdb == NULL)
        return PRET_READERROR;
    tempdb = fopen(DB_TEMPNAME, "w");
    if(tempdb == NULL) {
        fclose(fpdb);
        return PRET_WRITEERROR;
    }

    line = (char *)malloc(512*sizeof(char));
    if(line == NULL) {
        fclose(fpdb);
        fclose(tempdb);
        return PRET_ERROR;
    }

    rev++;

    fprintf(tempdb, "%x\n", rev);
    fgets(line, 512, fpdb);
    
    while(fgets(line, 512, fpdb) != NULL) fprintf(tempdb, line);

    fclose(fpdb);
    fclose(tempdb);
    
    temp_to_permanent();
    
    free(line);
    return rev;
}

int add_file(const char *filename)
{
int rev;
    
    return PRET_OK;
}