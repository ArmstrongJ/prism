/* Prism Versioning
 * Copyright 2014 Jeffrey Armstrong <jeff@rainbow-100.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <direct.h>
#include <errno.h>
#include <io.h>

#include "prism.h"
#include "db.h"
#include "rfile.h"

#define DB_FILENAME     PDB_DIR DIR_SEP "db.txt"
#define MSG_FILENAME    PDB_DIR DIR_SEP "message.txt"
#define Q_FILENAME      PDB_DIR DIR_SEP "queue.txt"

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
FILE *fp;
       
    if(filename == NULL)
        return PRET_ERROR;
    
    fp = fopen(Q_FILENAME, "a");
    if(fp == NULL)
        return PRET_WRITEERROR;
    
    fprintf(fp,"%s\n",filename);
    fclose(fp);
    
    return PRET_OK;
}

int get_db_fileinfo(const char *filename, struct db_file *info)
{
char *line;
FILE *fpdb;
char *reader;
char *sep;
int res;

    if(filename == NULL || info == NULL)
        return PRET_ERROR;
        
    fpdb = fopen(DB_FILENAME, "r");
    if(fpdb == NULL)
        return PRET_READERROR;

    line = (char *)malloc(512*sizeof(char));
    if(line == NULL) {
        fclose(fpdb);
        return PRET_ERROR;
    }
    
    info->id = -1;

    fgets(line, 512, fpdb);    
    while(fgets(line, 512, fpdb) != NULL) {
        reader = strrchr(line, '\r');
        if(reader == NULL) reader = strrchr(line, '\n');
        if(reader != NULL) reader[0] = '\0';
        
        reader = strchr(line, ' ') + 1; /* Advance to revision */
        reader = strchr(reader, ' ') + 1; /* Advance to hash */
        reader = strchr(reader, ' ') + 1; /* Advance to filename */
        
#if defined(MSDOS) || defined(WIN32)        
        res = strcmpi(filename, reader);
#else   
        res = strcmpi(filename, reader);
#endif
        if(res == 0) {
            info->id = strtol(line,&reader,16);
            info->revision = strtol(reader,&reader,16);
            
            reader++;
            sep = strchr(reader, ' ');
            memset(info->hash, 0, 33);
            memcpy(info->hash, reader, 32);
            
            break;
        }
    }
    
    fclose(fpdb);

    if(info->id >= 0)
        return PRET_OK;
    else
        return PRET_NOTTRACKED;
}

int disp_queue()
{
int i;
char *line;
FILE *fp;

    fp = fopen(Q_FILENAME, "r");
    if(fp == NULL) 
        return PRET_READERROR;
    
    line = (char *)malloc(512*sizeof(char));
    if(line == NULL) {
        fclose(fp);
        return PRET_ERROR;
    }
    
    i = 0;
    while(fgets(line, 512, fp) != NULL) {
        if(i == 0)
            printf("  currently queued files:\n");
        printf("    %s", line);
        i++;
    }
    fclose(fp);
    
    return i;
}

static int new_file_id()
{
char *line;
FILE *fp;
int i;

    fp = fopen(DB_FILENAME, "r");
    if(fp == NULL) 
        return PRET_READERROR;
    
    line = (char *)malloc(512*sizeof(char));
    if(line == NULL) {
        fclose(fp);
        return PRET_ERROR;
    }
    
    i = -1;
    while(fgets(line, 512, fp) != NULL) i++;
    
    fclose(fp);
    free(line);
    
    return i;
}

static int update_db_newfile(const char *filename, struct db_file *info)
{
FILE *fp;

    info->id = new_file_id();
    if(info->id < 0)
        return info->id;

    fp = fopen(DB_FILENAME, "a");
    if(fp == NULL) 
        return PRET_WRITEERROR;
    
    fprintf(fp, "%x %x %s %s\n", info->id, info->revision, info->hash, filename);
    
    fclose(fp);
    
    return PRET_OK;
}

static int update_db_file(const char *filename, struct db_file *info)
{
FILE *fpdb;
FILE *tempdb;
char *line;
char *reader;
int check_id;

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

    /* Revision line */
    fgets(line, 512, fpdb);
    fprintf(tempdb, line);
    
    while(fgets(line, 512, fpdb) != NULL) {
        check_id = strtol(line,&reader,16);
        if(check_id == info->id) 
            fprintf(tempdb, "%x %x %s %s\n", info->id, info->revision, info->hash, filename);
        else 
            fprintf(tempdb, line);
    }

    fclose(fpdb);
    fclose(tempdb);
    
    temp_to_permanent();
    
    free(line);
    
    return PRET_OK;
}

static int update_db_info(const char *filename, struct db_file *info)
{
    if(info->id < 0)
        return update_db_newfile(filename, info);
    else
        return update_db_file(filename, info);
}

int commit_queue(int revision)
{
int res;
FILE *fpq;
char *line;
char *tmp;
struct db_file info;
char newhash[33];

    fpq = fopen(Q_FILENAME, "r");
    if(fpq == NULL) 
        return PRET_READERROR;
    
    line = (char *)malloc(512*sizeof(char));
    if(line == NULL) {
        fclose(fpq);
        return PRET_ERROR;
    }
    
    while(fgets(line, 512, fpq) != NULL) {
        tmp = strchr(line, '\r');
        if(tmp == NULL) tmp = strchr(line, '\n');
        if(tmp != NULL) tmp[0] = '\0';
        if(strlen(line) == 0) continue;
        
        info.id = -1;
        res = get_db_fileinfo(line, &info);
        info.revision = revision;
        
        res = get_file_hash(line, newhash);
        if(info.id >= 0 && strcmp(newhash, info.hash) == 0)
            continue;
        
        memcpy(info.hash, newhash, 33);
        
        res = update_db_info(line, &info);
        if(res != PRET_OK)
            printf("  ERR: could not update file %s\n", line);
        else {
            res = compress_file(line, info.id, revision);
            if(res != PRET_OK)
                printf("  ERR: could not compress/store file %s\n", line);
        }
    }
    
    fclose(fpq);
    
    /* When complete, delete the queue */
    unlink(Q_FILENAME);
    
    return PRET_OK;
}

static int determine_revision(struct db_file *info, int desired_revision)
{
char *fname;
int fnamelength;
struct stat fileres;
int res, i;
    
    if(desired_revision > info->revision)
        return info->revision;
    
    fnamelength = strlen(PDB_DIR) + strlen(DIR_SEP) + 32;
    fname = (char *)malloc(fnamelength*sizeof(char));
    if(fname == NULL)
        return PRET_ERROR;
    
    i = desired_revision+1;
    do {
        i--;
        snprintf(fname, fnamelength, "%s%s%x.%x", PDB_DIR, DIR_SEP, info->id, i);
        res = stat(fname, &fileres);
    } while(res != 0 && i > 0);
    
    if(res == 0)
        return i;
        
    return PRET_NOTTRACKED;
}

int revert(const char *filename, int revision)
{
int target_revision;
struct db_file info;
int res;


    res = get_db_fileinfo(filename, &info);
    if(res != PRET_OK) 
        return res;
        
    if(revision < 0)
        target_revision = info.revision;
    else {
        target_revision = determine_revision(&info, revision);
        if(target_revision < 0)
            return PRET_NOREVISION;
    }
    
    return decompress_file(filename, info.id, target_revision);
}

int diff_db_file(const char *filename, int revision)
{
int target_revision;
struct db_file info;
int res;

    res = get_db_fileinfo(filename, &info);
    if(res != PRET_OK) 
        return res;
        
    if(revision < 0)
        target_revision = info.revision;
    else {
        target_revision = determine_revision(&info, revision);
        if(target_revision < 0)
            return PRET_NOREVISION;
    }
    
    return diff_file(filename, info.id, target_revision);
}

