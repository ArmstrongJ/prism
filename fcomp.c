/* fcomp.c:     

   This code has been adapted from:
   
   Software Practice and Experience 
   Vol 15, pp. 1025-1040, November 1985.
   
   "A File Comparison Program"
   by Miller and Myers
   http://tx0.org/7eq
   
   The modifications basically allow calling the diff routine
   from a separate program, upping the max lines, and modernizing
   some portions of the code.  Please don't ask me about the
   theory involved, I do not understand it.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prism.h"

#define MAXLINES 15000
#define ORIGIN MAXLINES
#define INSERT 1
#define DELETE 2

struct edit {
   struct edit *link;
   int op;
   int line1;
   int line2;
};

static char **A, **B;

static void fatal(const char *msg)
{
   printf("  ERR: %s\n",msg);
}

static void exceed(int d)
{
   printf("  ERR: the files differ in at least %d lines\n",d);
}

static int in_file(const char *filename, char **P)
{
char *buf, *save;
FILE *fp, *fopen();
int lines = 0;

    if ((fp = fopen(filename,"r")) == NULL) 
    {
        fatal("cannot open file");
        return PRET_READERROR;
    }

    buf = (char *)malloc(256*sizeof(char));
    if(buf == NULL)
        return PRET_ERROR;
    memset(buf, 0, 256);
    
    while(fgets(buf,255,fp) != NULL) 
    {
        if (lines >= MAXLINES) {
            fatal("file is to large for diff");
            return PRET_ERROR;
        }
        
        save = (char *)malloc((strlen(buf)+1)*sizeof(char));
        if (save == NULL) {
            fatal("not enough room to save the files");
            return PRET_ERROR;
        }
        
        P[lines++] = save;
        strcpy(save, buf);
    }
    fclose(fp);
    
    free(buf);
    return(lines);
}

static void put_scr(struct edit *start, const char *fname1, const char *fname2)
{
   struct edit *ep, *behind, *ahead, *a, *b;
   int change;

   ahead = start;
   ep = NULL;
   while (ahead != NULL) 
   {
      behind = ep;
      ep = ahead;
      ahead = ahead->link;
      ep->link = behind;
   }

   while( ep != NULL  && 0 == 0) 
   {
      b = ep;
      if (ep->op == INSERT)
         //printf("Inserted after line %d:\n",ep->line1);
         printf("line %s:%d || %s:%d\n", fname1, ep->line1, fname2, ep->line2);
      else 
      {
         do 
         {
            a = b;
            b = b->link;
         } while (b!=NULL && b->op == DELETE && b->line1 == a->line1+1);

         change = (b!=NULL && b->op == INSERT && b->line1 == a->line1);

         if (a == ep)
            printf("line %s:%d || %s:%d\n", fname1, ep->line1, fname2, ep->line2);
            //printf("line %d:\n",ep->line1);
         else
            printf("line %s:%d-%d || %s:%d-%d\n", fname1, ep->line1, a->line1, fname2, ep->line2, a->line2);
            //printf("lines %d-%d:\n",ep->line1,a->line1);

         do 
         {
            printf("-%s",A[ep->line1-1]);
            ep = ep->link;
         } while (ep != b);

         if (!change) {
            printf("\n");
            continue;
         }
      }
      do 
      {
         printf("+%s",B[ep->line2-1]);
         ep = ep->link;
      } while (ep != NULL && ep->op == INSERT && ep->line1 == b->line1);
      
      printf("\n");
   }
}


int compare_files(const char *fname1, const char *fname2)
{
   int   max_d,
         m,
         n,
         lower,
         upper,
         d,
         k,
         row,
         col;

   int *last_d;
   struct edit **script;

   struct edit *new;

   last_d = (int *)malloc((2*MAXLINES+1)*sizeof(int));
   script = (struct edit **)malloc((2*MAXLINES+1)*sizeof(struct edit *));

   A = (char **)malloc(MAXLINES*sizeof(char *));
   B = (char **)malloc(MAXLINES*sizeof(char *));
   if(A == NULL || B == NULL) return PRET_ERROR;
   memset(A, 0, MAXLINES);
   memset(B, 0, MAXLINES);

   max_d = 2*MAXLINES;

   m = in_file(fname1,A);
   n = in_file(fname2,B);

   for(row=0; row < m && row < n && strcmp(A[row],B[row]) == 0; ++row);
   
   last_d[ORIGIN] = row;
   script[ORIGIN] = NULL;
   
   lower = (row == m) ? ORIGIN + 1 : ORIGIN - 1;
   upper = (row == n) ? ORIGIN - 1 : ORIGIN + 1;
   
   if (lower > upper) 
   {
      printf("  the files are identical\n");
      return PRET_OK;
   }

   for (d = 1 ; d <= max_d ; ++d) 
   {
      for ( k = lower ; k <= upper ; k +=2) 
      {
         new = (struct edit *) malloc(sizeof(struct edit));
         if (new == NULL) {
            exceed(d);
            return PRET_ERROR;
         }

         if (k == ORIGIN-d || k != ORIGIN+d && last_d[k+1] >= last_d[k-1]) 
         {
            row = last_d[k+1]+1;
            new->link = script[k+1];
            new->op = DELETE;
         } 
         else 
         {
            row = last_d[k-1];
            new->link = script[k-1];
            new->op = INSERT;
         }
         new->line1 = row;
         new->line2 = col = row + k - ORIGIN;
         script[k] = new;

         while(row < m && col < n && strcmp(A[row],B[col]) == 0) 
         {
            ++row;
            ++col;
         }
         last_d[k] = row;

         if (row == m && col == n)
         {
            put_scr(script[k], fname1, fname2);
            return PRET_OK;
         }
         if (row == m)
            lower = k+2;

         if (col == n)
            upper = k-2;
      }
      --lower;
      ++upper;
   }
   
   exceed(d);
   return PRET_ERROR;
}
