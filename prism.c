/* Prism Versioning
 * Copyright 2014 Jeffrey Armstrong <jeff@rainbow-100.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prism.h"
#include "db.h"
#include "rfile.h"

static void banner()
{
    printf("Prism Versioning\nRelease %s\nBy Jeff Armstrong <jeff@rainbow-100.com>\n",VERSION);
}

static void usage(const char *name)
{
    banner();
    printf("\nUsage: %s <command> ...\n\n", name);
    printf("Commands:\n\n");
    printf("\tabout\tprints a banner and exits\n");
    printf("\thash\tdisplays a file's hash\n");
    printf("\tinfo\tshow information about repository\n");
    printf("\tinit\tinitializes this directory for versioning\n");
}
    
static void init_task()
{
    printf("Initializing Prism...\n");
    
    switch(initdb()) {
        case PRET_EXISTS:
            printf("  ERR: already initialized\n");
            break;
        case PRET_PERMISSION:
            printf("  ERR: permissions error\n");
            break;
        case PRET_ERROR:
            printf("  ERR: general error\n");
            break;
        default:
            printf("  OK: initialized\n");
    }
}

static void add_task(const char *filename)
{
char new_hash[33];
int res;
struct db_file oldfile;

    if(filename == NULL) {
        printf("Nothing to do...\n");
        return;
    }

    printf("Adding '%s' to the queue...\n", filename);
    res = get_db_fileinfo(filename, &oldfile);
    if(res == PRET_NOTTRACKED) {
        add_file(filename);
        printf("  new file is queued\n");
    } else if(res == PRET_OK) {
        res = get_file_hash(filename, new_hash);
        if(strcmp(oldfile.hash, new_hash) == 0) {
            printf("  ERR: %s hasn't changed", filename);
        } else {
            add_file(filename);
            printf("  modified file is queued\n");
        }
    } else
        printf("  ERR: problem retrieving previous hash\n");
}

static void commit_task(const char *message) 
{

}

static void info_task()
{
int res;

    printf("Retrieving Prism information...\n");
    res = get_revision();
    if(res < 0) {
        printf("  ERR: no revision could be retrieved\n");
        return;
    } else
        printf("  current revision: %d\n", res);

    res = get_file_count();
    if(res < 0) {
        printf("  ERR: file count error\n");
        return;
    } else
        printf("  tracking %d files\n", res);
    
    res = disp_queue();
    if(res == PRET_READERROR)
        printf("  no files in queue\n");
}

static void hash_task(const char *filename)
{
int res;
char hash[33];
    
    memset(hash, 0, 33);
    
    printf("Computing hash for %s...\n", filename);
    res = get_file_hash(filename, hash);
    if(res == PRET_OK)
        printf("  hash: %s\n", hash);
    else
        printf("  ERR: hash failed\n");
}

int main(int argc, char *argv[])
{
    if(argc == 1) {
        usage(argv[0]);
        return 0;
    }
    
    if(strcmp(argv[1], "about") == 0)
        banner();
    else if(strcmp(argv[1], "add") == 0 && argc == 3)
        add_task(argv[2]);
    else if(strcmp(argv[1], "init") == 0)
        init_task();
    else if(strcmp(argv[1], "info") == 0)
        info_task();
    else if(strcmp(argv[1], "hash") == 0 && argc == 3)
        hash_task(argv[2]);
    else
        printf("Error processing command...\n");

    return 0;
}
