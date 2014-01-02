#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prism.h"
#include "md5.h"
#include "rfile.h"

static const char hex_str[]= "0123456789abcdef";

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
    while(fgets(buf, 15, fp)) {
        MD5_Update(&hb, buf, strlen(buf));
    }
    fclose(fp);
    
    MD5_Final(buf, &hb);
    
    for(i=0; i<16; i++) {
        hash[2*i] = hex_str[buf[i] >> 4];
        hash[2*i+1] = hex_str[buf[i] & 0x0F];
    }
    
    return PRET_OK;
}    
