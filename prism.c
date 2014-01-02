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
}

static void info_hash(const char *filename)
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
    else if(strcmp(argv[1], "init") == 0)
        init_task();
    else if(strcmp(argv[1], "info") == 0)
        info_task();
    else if(strcmp(argv[1], "hash") == 0 && argc == 3)
        info_hash(argv[2]);

    return 0;
}
