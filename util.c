/* Prism Versioning
 * Copyright 2014 Jeffrey Armstrong <jeff@rainbow-100.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#ifndef __GNUC__
#include <direct.h>
#endif

#include "prism.h"

char *alloc_and_combine_paths(char *p1, char *p2)
{
int l1, l2;
char *ret;

    if(p1 == NULL || p2 == NULL) return NULL;
    
    l1 = strlen(p1); l2 = strlen(p2);
    
    ret = (char *)malloc((l1+l2+strlen(DIR_SEP))*sizeof(char));
    if(ret == NULL) return ret;
    
    ret[0] = '\0';
    strcat(ret, p1);
    strcat(ret, DIR_SEP);
    strcat(ret, p2);
    
    return ret;
}
