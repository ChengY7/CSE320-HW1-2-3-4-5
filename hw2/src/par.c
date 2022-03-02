/*********************/
/* par.c             */
/* for Par 3.20      */
/* Copyright 1993 by */
/* Adam M. Costello  */
/*********************/

/* This is ANSI C code. */


#include "errmsg.h"
#include "buffer.h"    /* Also includes <stddef.h>. */
#include "reformat.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#undef NULL
#define NULL ((void *) 0)


const char * const progname = "par";
const char * const version = "3.20";

/* Returns the value represented by the digit c,   */
/* or -1 if c is not a digit. Does not use errmsg. */
static int digtoint(char c) {
  return c == '0' ? 0 :
         c == '1' ? 1 :
         c == '2' ? 2 :
         c == '3' ? 3 :
         c == '4' ? 4 :
         c == '5' ? 5 :
         c == '6' ? 6 :
         c == '7' ? 7 :
         c == '8' ? 8 :
         c == '9' ? 9 :
         -1;

  /* We can't simply return c - '0' because this is ANSI  */
  /* C code, so it has to work for any character set, not */
  /* just ones which put the digits together in order.    */
}

/* Puts the decimal value of the string s into *pn, returning */
/* 1 on success. If s is empty, or contains non-digits,       */
/* or represents an integer greater than 9999, then *pn       */
/* is not changed and 0 is returned. Does not use errmsg.     */
static int strtoudec(const char *s, int *pn) {
  int n = 0;
  if (!*s) return 0; //if s pointer is null then return 0


  do {
    if (!isdigit(*s)) return 0;  //if encounter non digit then return error
    n = (10 * n) + digtoint(*s);
    if (n >= 1000) return 0;     //if n>999 then its an error
  } while (*++s);

  *pn = n;

  return 1;
}

/* Parses the single option in opt, setting *pwidth, *pprefix,     */
/* *psuffix, *phang, *plast, or *pmin as appropriate. Uses errmsg. */
static void parseopt(const char *opt, int *pwidth, int *pprefix, int *psuffix, int *phang, int *plast, int *pmin) {
  const char *saveopt = opt;      //make a copy of opt that points the beginning of the option
  char oc;
  int n, r;

  if (*opt == '-') ++opt;         //if option is '-' then we move to next option

  if (!strcmp(opt, "version")) {
    sprintf(errmsg, "%s %s\n", progname, version);  //if option is version then print version number
    return;
  }
  oc = *opt; //save the next char of opt in oc
  if (isdigit(oc)) {                               //if oc is digit
    if (!strtoudec(opt, &n)) goto badopt;          //convert option to int if error then goto badopt
    if (n <= 8) *pprefix = n;                      //if n is 8 or less, n sets to prefix
    else *pwidth = n;                              //if n is 9 or more, n sets to width
  }
  else {
    if (!oc) goto badopt;                    //if oc is null go to badopt
    n = 1;                                   //This is case for -[letter][number]
    r = strtoudec(opt + 1, &n);              //get the number and store it inside n
    if (opt[1] && !r) goto badopt;           //If number exist but r==0 then it's an error because strtoudec casued an error

    if (oc == 'w' || oc == 'p' || oc == 's') {
      if (!r) goto badopt;                   //-w, -p, -s needs a number after or r is going to be 0. If r is false go to badopt
      if      (oc == 'w') *pwidth  = n;      //set -w
      else if (oc == 'p') *pprefix = n;      //set -p
      else                *psuffix = n;      //set -s
    }
    else if (oc == 'h') *phang = n;          //set -h, if h given without number then value 1 is assumed
    else if (n <= 1) {                       // n==0 or 1 then consider -l and -m
      if      (oc == 'l') *plast = n;       
      else if (oc == 'm') *pmin = n;
    }
    else goto badopt;                        //everything else go to badopt
  }

  *errmsg = '\0';
  return;

badopt:
  sprintf(errmsg, "Bad option: %.149s\n", saveopt);
}

/* Reads lines from stdin until EOF, or until a blank line is encountered, */
/* in which case the newline is pushed back onto the input stream. Returns */
/* a NULL-terminated array of pointers to individual lines, stripped of    */
/* their newline characters. Uses errmsg, and returns NULL on failure.     */
static char **readlines(void) {
  struct buffer *cbuf = NULL, *pbuf = NULL;
  int c, blank;
  char ch, *ln=NULL, *nullline = NULL, nullchar = '\0', **lines = NULL;

  cbuf = newbuffer(sizeof (char));
  if (*errmsg) goto rlcleanup;
  pbuf = newbuffer(sizeof (char *));
  if (*errmsg) goto rlcleanup;

  for (blank = 1;  ; ) {            //some kind of loop
    c = getchar();                  //get the char
    if (c == EOF) break;            //if EOF break out of loop
    if (c == '\n') {          
      if (blank) {                  //if end of line and blank==1 then were at the end of a line, put new line '\n' back into file and break out of loop
        ungetc(c,stdin);
        break;
      }
      additem(cbuf, &nullchar);     //add '\0' to buffer
      if (*errmsg) goto rlcleanup;
      ln = copyitems(cbuf);         //copy items in cbuf to ln
      if (*errmsg) goto rlcleanup;
      additem(pbuf, &ln);            //add char* ln into pbuf
      if (*errmsg) goto rlcleanup;
      clearbuffer(cbuf);
      blank = 1;
    }
    else {
      if (!isspace(c)) blank = 0;
      ch = c;
      additem(cbuf, &ch);
      if (*errmsg) goto rlcleanup;
    }
  }

  if (!blank) {                     //If we run into EOF
    additem(cbuf, &nullchar);       //add '/0' to cbuf
    if (*errmsg) goto rlcleanup;
    ln = copyitems(cbuf);           //copy items of cbuf to ln
    if (*errmsg) goto rlcleanup;
    additem(pbuf, &ln);             //add char* ln to pbuf
    if (*errmsg) goto rlcleanup;
  }

  additem(pbuf, &nullline);         //add '/0' to pbuf
  if (*errmsg) goto rlcleanup;
  lines = copyitems(pbuf);

rlcleanup:
  if (cbuf) freebuffer(cbuf);
  if (pbuf) {
    if (!lines) {
      for (;;) {
        lines = nextitem(pbuf);
        if (!lines) break;
        free(*lines);
      }
    }
    freebuffer(pbuf);
  }


  return lines;
}

/* If any of *pwidth, *pprefix, *psuffix, *phang, *plast, *pmin are     */
/* less than 0, sets them to default values based on inlines, according */
/* to "par.doc". Does not use errmsg because it always succeeds.        */
static void setdefaults(const char * const *inlines, int *pwidth, int *pprefix, int *psuffix, int *phang, int *plast, int *pmin) {
  int numlines;
  const char *start=NULL, *end=NULL, * const *line=NULL, *p1=NULL, *p2=NULL;
  if (*pwidth < 0) *pwidth = 72;
  if (*phang < 0) *phang = 0;
  if (*plast < 0) *plast = 0;
  if (*pmin < 0) *pmin = *plast;

  for (line = inlines;  *line;  ++line);
  numlines = line - inlines;

  if (*pprefix < 0) {
    if (numlines <= *phang + 1)     //if numlines is no more than hang+1 then defaults to 0
      *pprefix = 0;
    else {                          //else pprefix is the common prefix of all lines
      start = inlines[*phang];
      for (end = start;  *end;  ++end);
      for (line = inlines + *phang + 1;  *line;  ++line) {
        for (p1 = start, p2 = *line;  p1 < end && *p1 == *p2;  ++p1, ++p2);
        end = p1;
      }
      *pprefix = end - start;                   
    }
  }

  if (*psuffix < 0) {
    if (numlines <= 1)              //if numlnes is no more than 1 then defaults to 0
      *psuffix = 0;
    else {
      start = *inlines;
      for (end = start;  *end;  ++end);
      for (line = inlines + 1;  *line;  ++line) {
        for (p2 = *line;  *p2;  ++p2);
        for (p1 = end;
             p1 > start && p2 > *line && p1[-1] == p2[-1];
             --p1, --p2);
        start = p1;
      }
      while (end - start >= 2 && isspace(*start) && isspace(start[1])) ++start;
      *psuffix = end - start;
    }
  }
}



static void freelines(char **lines)
/* Frees the strings pointed to in the NULL-terminated array lines, then */
/* frees the array. Does not use errmsg because it always succeeds.      */
{
  char *line=NULL;
  char **temp = lines;
  
  for (line = *lines;  *lines;) {
    free(line);
    lines++;
    line=*lines;
  }
  free(temp);
}


int original_main(int argc, const char * const *argv) {
  int width, widthbak = -1, prefix, prefixbak = -1, suffix, suffixbak = -1, hang, hangbak = -1, last, lastbak = -1, min, minbak = -1, c;
  char *parinit=NULL, *picopy = NULL, *opt=NULL, **inlines = NULL, **outlines = NULL, **line=NULL;
  const char * const whitechars = " \f\n\r\t\v";
  parinit = getenv("PARINIT");
  if (parinit) {                         //If enviroment variable parinit is enabled
    picopy = malloc((strlen(parinit) + 1) * sizeof (char));
    if (!picopy) {
      strcpy(errmsg,outofmem);
      goto parcleanup;
    }
    strcpy(picopy,parinit);
    opt = strtok(picopy,whitechars);
    while (opt) {
      parseopt(opt, &widthbak, &prefixbak, &suffixbak, &hangbak, &lastbak, &minbak);
      if (*errmsg) goto parcleanup;
      opt = strtok(NULL,whitechars);
    }
    free(picopy);
    picopy = NULL;
  }
  while (*++argv) {            
    parseopt(*argv, &widthbak, &prefixbak, &suffixbak, &hangbak, &lastbak, &minbak);  //loop through argv and call parseopt
    if (*errmsg) goto parcleanup;
  }
  
  for (;;) {
    for (;;) {
      c = getchar();
      if (c==EOF) goto parcleanup;
      if (c != '\n') break;
      putchar(c);
    }
    ungetc(c,stdin);                    //The above code gets rid of newlines in the beginnning
    inlines = readlines();              //reads the lines of a file and store it in **inline
    if (*errmsg) goto parcleanup;
    if (!*inlines) {
      free(inlines);
      inlines = NULL;
      continue;
    }
    width = widthbak;  prefix = prefixbak;  suffix = suffixbak;
    hang = hangbak;  last = lastbak;  min = minbak;                //init all the values before setting default
    setdefaults((const char * const *) inlines, &width, &prefix, &suffix, &hang, &last, &min);
    if (width <= prefix+suffix)
     goto parcleanup;
    outlines = reformat((const char * const *) inlines, width, prefix, suffix, hang, last, min);
    if (*errmsg) goto parcleanup;
    freelines(inlines);
    inlines = NULL;
    for (line = outlines;  *line;  ++line)
      puts(*line);
    freelines(outlines);
    outlines = NULL;
  }

parcleanup:

  if (picopy) free(picopy);
  if (inlines) freelines(inlines);
  if (outlines) freelines(outlines);

  if (*errmsg) {
    fprintf(stderr, "%.163s", errmsg);
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
