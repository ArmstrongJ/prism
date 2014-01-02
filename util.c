/* Prism Versioning
 * Copyright 2014 Jeffrey Armstrong <jeff@rainbow-100.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys\types.h>
#include <direct.h>
#include <string.h>

#include "prism.h"

char *alloc_and_combine_paths(char *p1, char *p2)
{
int l1, l2;
char *ret, *two;

    if(p1 == NULL || p2 == NULL) return NULL;
    
    l1 = strlen(p1); l2 = strlen(p2);
    
    ret = (char *)malloc((l1+l2+2)*sizeof(char));
    if(ret == NULL) return ret;
    
    ret[0] = '\0';
    strcat(ret, p1);
    two = ret + l1;
    two[0] = DIR_SEP;
    two[1] = '\0';
    two = NULL;
    strcat(ret, p2);
    
    return ret;
}