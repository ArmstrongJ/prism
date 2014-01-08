/* Prism Versioning
 * Copyright 2014 Jeffrey Armstrong <jeff@rainbow-100.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prism.h"
#include "db.h"
#include "rfile.h"

static char *message_option;
static int revision_option;

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
    printf("\tadd\tadds the specified file to the commit queue\n");
    printf("\tclear\tclears the commit queue\n");
    printf("\tcommit\tcommits currently queued files\n");
    printf("\tdiff\tcompares a working copy versus repository\n");
    printf("\thash\tdisplays a file's hash\n");
    printf("\tinfo\tshow information about repository\n");
    printf("\tinit\tinitializes this directory for versioning\n");
    printf("\trevert\treverts the specified file to the last revision\n");
    printf("\n");
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
char new_hash[HASH_BYTES];
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
int fcount;
int rev;

    printf("Committing queue...\n");
    fcount = disp_queue();
    if(fcount <= 0) {
        printf("  ERR: no files queued\n");
        return;
    }

    rev = increment_revision(); 
    if(commit_queue(rev) == PRET_OK) {
        printf("  commit completed\n");
        if(message != NULL)
            save_commit_message(message, rev);
    } else
        printf("  ERR: commit failed\n");
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
char hash[HASH_BYTES];
    
    memset(hash, 0, HASH_BYTES);
    
    printf("Computing hash for %s...\n", filename);
    res = get_file_hash(filename, hash);
    if(res == PRET_OK)
        printf("  hash: %s\n", hash);
    else
        printf("  ERR: hash failed\n");
}

static void revert_task(const char *filename)
{
int res;

    if(revision_option == -1)
        printf("Reverting changes to %s...\n", filename);
    else
        printf("Reverting %s to revision %d...\n", filename, revision_option);
    
    res = revert(filename, revision_option);
    
    switch(res) { 
        case PRET_OK:
            printf("  changes reverted\n");
            break;
        case PRET_GZFAILED:
            printf("  ERR: decompression failure\n");
            break;
        case PRET_NOREVISION:
            printf("  ERR: no applicable revision found\n");
            break;
        default:
            printf("  ERR: revert failed\n");
    }
}

static void diff_task(const char *filename)
{
int res;

    printf("Comparing current %s to ",filename);
    if(revision_option == -1)
        printf("latest repository revision...\n");
    else
        printf("revision %d...\n", revision_option);
    
    res = diff_db_file(filename, revision_option);
    switch(res) { 
        case PRET_OK:
            printf("  diff complete\n");
            break;
        case PRET_GZFAILED:
            printf("  ERR: decompression failure\n");
            break;
        case PRET_NOREVISION:
            printf("  ERR: no applicable revision found\n");
            break;
        default:
            printf("  ERR: diff failed\n");
    }
}

static void clear_task()
{
    printf("Clearing the commit queue...\n");
    switch(clear_entire_queue()) {
        case PRET_OK:
            printf("  commit queue is now clear\n");
            break;
        default:
            printf("  ERR: clear failed\n");
    }
}

int main(int argc, char *argv[])
{
int i,j;
char *rtext;

    if(argc == 1) {
        usage(argv[0]);
        return 0;
    }

    message_option = NULL; 
    rtext = NULL;
    revision_option = -1;

    for(i = 1; i < argc; i++) {
        if(argv[i][0] == '-') {
            switch(argv[i][1]) {
                case 'm':
                    message_option = argv[i+1];
                    break;
                case 'r':
                    rtext = argv[i+1];
                    break;
                default:
                    banner();
                    printf("Unknown option encountered: %s\n", argv[i]);
                    return 0;
            }
            for(j=i+2; j<argc;j++)
                argv[j-2] = argv[j];
            argc -= 2;
        }
    }
    
    if(rtext != NULL)
        revision_option = atoi(rtext);
    
    if(strcmp(argv[1], "about") == 0)
        banner();
    else if(strcmp(argv[1], "add") == 0 && argc == 3)
        add_task(argv[2]);
    else if(strcmp(argv[1], "commit") == 0)
        commit_task(message_option);
    else if(strcmp(argv[1], "init") == 0)
        init_task();
    else if(strcmp(argv[1], "info") == 0)
        info_task();
    else if(strcmp(argv[1], "hash") == 0 && argc == 3)
        hash_task(argv[2]);
    else if(strcmp(argv[1], "revert") == 0 && argc == 3)
        revert_task(argv[2]);
    else if(strcmp(argv[1], "diff") == 0 && argc == 3)
        diff_task(argv[2]);
    else if(strcmp(argv[1], "clear") == 0)
        clear_task();
    else {
        usage(argv[0]);
        printf("Error processing command...\n\n");
    }

    printf("\n");

    return 0;
}
