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
#include <getopt.h>

#undef NULL
#define NULL ((void *) 0)

int is_int(char *str);
const char * const progname = "par ";
const char * const version = "3.20";

int is_int(char *str) {
  while(*str!='\0') {
    if (*str<48 || *str>57) 
      return 0;
    str++;
  }
  return 1;
}
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
    if (n >= 10000) return 0;     //if n>999 then its an error
  } while (*++s);

  *pn = n;

  return 1;
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
  if (is_error()) goto rlcleanup;
  pbuf = newbuffer(sizeof (char *));
  if (is_error()) goto rlcleanup;

  for (blank = 1;  ; ) {            //some kind of loop
    c = getchar();                  //get the char
    if (c == EOF) break;            //if EOF break out of loop
    if (c == '\n') {          
      if (blank) {                  //if end of line and blank==1 then were at the end of a line, put new line '\n' back into file and break out of loop
        ungetc(c,stdin);
        break;
      }
      additem(cbuf, &nullchar);     //add '\0' to buffer
      if (is_error()) goto rlcleanup;
      ln = copyitems(cbuf);         //copy items in cbuf to ln
      if (is_error()) goto rlcleanup;
      additem(pbuf, &ln);            //add char* ln into pbuf
      if (is_error()) goto rlcleanup;
      clearbuffer(cbuf);
      blank = 1;
    }
    else {
      if (!isspace(c)) blank = 0;
      ch = c;
      additem(cbuf, &ch);
      if (is_error()) goto rlcleanup;
    }
  }

  if (!blank) {                     //If we run into EOF
    additem(cbuf, &nullchar);       //add '/0' to cbuf
    if (is_error()) goto rlcleanup;
    ln = copyitems(cbuf);           //copy items of cbuf to ln
    if (is_error()) goto rlcleanup;
    additem(pbuf, &ln);             //add char* ln to pbuf
    if (is_error()) goto rlcleanup;
  }

  additem(pbuf, &nullline);         //add '/0' to pbuf
  if (is_error()) goto rlcleanup;
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


int original_main(int argc, char * const *argv) {
  int width, widthbak = -1, prefix, prefixbak = -1, suffix, suffixbak = -1, hang, hangbak = -1, last, lastbak = -1, min, minbak = -1, c;
  char *parinit=NULL, *picopy = NULL, *opt=NULL, **inlines = NULL, **outlines = NULL, **line=NULL;
  const char * const whitechars = " \f\n\r\t\v";
  char *tempChar=NULL;
  char *tempError=NULL;
  char *invalidNum = "invalid number: ";
  char *invalidOpt = "bad option: ";
  parinit = getenv("PARINIT");
  if (parinit) {                         //If enviroment variable parinit is enabled
    picopy = malloc((strlen(parinit) + 1) * sizeof (char));
    if (!picopy) {
      set_error("Out of memory.\n");
      goto parcleanup;
    }
    strcpy(picopy,parinit);
    opt = strtok(picopy,whitechars);
    while (opt) {
      //parseopt(opt, &widthbak, &prefixbak, &suffixbak, &hangbak, &lastbak, &minbak);
      if (is_error()) goto parcleanup;
      opt = strtok(NULL,whitechars);
    }
    free(picopy);
    picopy = NULL;
  }
  int opt_char;
  int option_index=0;
  static int last_flag=-1;
  static int min_flag=-1;
  static int version_flag=0;
  static struct option long_options[]={
    {"version", no_argument, &version_flag, 1},
    {"width", required_argument, 0, 'w'},
    {"prefix", required_argument, 0,'p'},
    {"suffix", required_argument, 0, 's'},
    {"hang", optional_argument, 0, 'h'},
    {"no-last", no_argument, &last_flag, 0},
    {"last", no_argument, &last_flag, 1},
    {"no-min", no_argument, &min_flag, 0},
    {"min", no_argument, &min_flag, 1},
    {0, 0, 0, 0}
  };
  loop:
  if(optind<argc && is_int(argv[optind])) {
    int temp=-1;
    optarg=argv[optind++];
    if(!(strtoudec(optarg, &temp))) {
      tempChar=optarg;
      tempError=malloc(strlen(tempChar) + strlen(invalidNum) + 1);
      if(!tempError) {
        set_error("Out of memory.\n");
        goto parcleanup;
      }
      strcpy(tempError, invalidNum);
      strcat(tempError, tempChar);
      set_error(tempError);
      free(tempError);
      goto parcleanup;
    }
    else {
      if(temp<=8)
        prefixbak=temp;
      else
        widthbak=temp;
    }
    if(optind<argc && is_int(argv[optind]))
      goto loop;
  }
  if(optind<argc && (argv[optind][0])!='-') {
    tempChar=argv[optind];
    tempError=malloc(strlen(tempChar) + strlen(invalidOpt) +1);
    if(!tempError) {
      set_error("Out of memory.\n");
      goto parcleanup;
    }
    strcpy(tempError, invalidOpt);
    strcat(tempError, tempChar);
    set_error(tempError);
    free(tempError);
    goto parcleanup;
  }
  if(optind<argc && strcmp(argv[optind], "-")==0) {
    tempChar=argv[optind];
    tempError=malloc(strlen(tempChar) + strlen(invalidOpt) +1);
    if(!tempError) {
      set_error("Out of memory.\n");
      goto parcleanup;
    }
    strcpy(tempError, invalidOpt);
    strcat(tempError, tempChar);
    set_error(tempError);
    free(tempError);
    goto parcleanup;
  }
  while((opt_char = getopt_long(argc, argv, "w:p:s:h::l::m::", long_options, &option_index)) != -1) {
    if(version_flag) {
      tempError=malloc(strlen(progname) + strlen(version) +1);
      if(!tempError) {
        set_error("Out of memory.\n");
        goto parcleanup;
      }
      strcpy(tempError, progname);
      strcat(tempError, version);
      set_error(tempError);
      free(tempError);
      report_error(stdout);
      clear_error();
      goto parcleanup;
    }
    if(min_flag==1)
      minbak=1;
    if(min_flag==0)
      minbak=0;
    if(last_flag==1)
      lastbak=1;
    if(last_flag==0)
      lastbak=0;
    switch(opt_char) {
      case 'w':
        if(!(strtoudec(optarg, &widthbak))) {
          tempChar=optarg;
          tempError=malloc(strlen(tempChar) + strlen(invalidNum) + 1);
          if(!tempError) {
            set_error("Out of memory.\n");
            goto parcleanup;
          }
          strcpy(tempError, invalidNum);
          strcat(tempError, tempChar);
          set_error(tempError);
          free(tempError);
          goto parcleanup;
        }
        break;
      case 'p':
        if(!(strtoudec(optarg, &prefixbak))) {
          tempChar=optarg;
          tempError=malloc(strlen(tempChar) + strlen(invalidNum) + 1);
          if(!tempError) {
            set_error("Out of memory.\n");
            goto parcleanup;
          }
          strcpy(tempError, invalidNum);
          strcat(tempError, tempChar);
          set_error(tempError);
          free(tempError);
          goto parcleanup;
        }
        break;
      case 's':
        if(!(strtoudec(optarg, &suffixbak))) {
          tempChar=optarg;
          tempError=malloc(strlen(tempChar) + strlen(invalidNum) + 1);
          if(!tempError) {
            set_error("Out of memory.\n");
            goto parcleanup;
          }
          strcpy(tempError, invalidNum);
          strcat(tempError, tempChar);
          set_error(tempError);
          free(tempError);
          goto parcleanup;
        }
        break;
      case 'h':
        if (optarg == NULL && optind < argc && argv[optind][0] != '-') 
          optarg = argv[optind++];
        if (optarg==NULL)  //no arg
          hangbak=1;
        else               //yes arg
          if(!(strtoudec(optarg, &hangbak))) {
            tempChar=optarg;
            tempError=malloc(strlen(tempChar) + strlen(invalidNum) + 1);
            if(!tempError) {
              set_error("Out of memory.\n");
              goto parcleanup;
            }
            strcpy(tempError, invalidNum);
            strcat(tempError, tempChar);
            set_error(tempError);
            free(tempError);
            goto parcleanup;
          }
        break;
      case 'm':
        if (optarg == NULL && optind < argc && argv[optind][0] != '-')
          optarg = argv[optind++];
        if (optarg==NULL)
          minbak=1;
        else {
          if(!(strtoudec(optarg, &minbak))) {
            tempChar=optarg;
            tempError=malloc(strlen(tempChar) + strlen(invalidNum) + 1);
            if(!tempError) {
              set_error("Out of memory.\n");
              goto parcleanup;
            }
            strcpy(tempError, invalidNum);
            strcat(tempError, tempChar);
            set_error(tempError);
            free(tempError);
            goto parcleanup;
          }
          if(minbak!=0 && minbak!=1) {
            tempError="Min can only be 0 or 1\n";
            set_error(tempError);
            goto parcleanup;
          }
        }
        break;
      case 'l':
        if (optarg == NULL && optind < argc && argv[optind][0] != '-')
          optarg = argv[optind++];
        if (optarg==NULL)
          lastbak=1;
        else {
          if(!(strtoudec(optarg, &lastbak))) {
            tempChar=optarg;
            tempError=malloc(strlen(tempChar) + strlen(invalidNum) + 1);
            if(!tempError) {
              set_error("Out of memory.\n");
              goto parcleanup;
            }
            strcpy(tempError, invalidNum);
            strcat(tempError, tempChar);
            set_error(tempError);
            free(tempError);
            goto parcleanup;
          }
          if(lastbak!=0 && lastbak!=1) {
            tempError=("Last can only be 0 or 1\n");
            set_error(tempError);
            goto parcleanup;
          }
        }
        break;
      default:
        break;
    }
    if(optind<argc && is_int(argv[optind]))
      goto loop;
    if(optind<argc && (argv[optind][0])!='-') {
      tempChar=argv[optind];
      tempError=malloc(strlen(tempChar) + strlen(invalidOpt) + 1);
      if(!tempError) {
        set_error("Out of memory.\n");
        goto parcleanup;
      }
      strcpy(tempError, invalidOpt);
      strcat(tempError, tempChar);
      set_error(tempError);
      free(tempError);
      goto parcleanup;
    }
    if(optind<argc && strcmp(argv[optind], "-")==0) {
      tempChar=argv[optind];
      tempError=malloc(strlen(tempChar) + strlen(invalidOpt) +1);
      if(!tempError) {
        set_error("Out of memory.\n");
        goto parcleanup;
      }
      strcpy(tempError, invalidOpt);
      strcat(tempError, tempChar);
      set_error(tempError);
      free(tempError);
      goto parcleanup;
    }
  }
  //printf("Width: %d Prefix: %d Suffix: %d Hang: %d Min: %d Last: %d\n", widthbak, prefixbak, suffixbak, hangbak, minbak, lastbak);
  
  for (;;) {
    for (;;) {
      c = getchar();
      if (c==EOF) goto parcleanup;
      if (c != '\n') break;
      putchar(c);
    }
    ungetc(c,stdin);                    //The above code gets rid of newlines in the beginnning
    inlines = readlines();              //reads the lines of a file and store it in **inline
    if (is_error()) goto parcleanup;
    if (!*inlines) {
      free(inlines);
      inlines = NULL;
      continue;
    }
    width = widthbak;  prefix = prefixbak;  suffix = suffixbak;
    hang = hangbak;  last = lastbak;  min = minbak;                //init all the values before setting default
    setdefaults((const char * const *) inlines, &width, &prefix, &suffix, &hang, &last, &min);
    if (width <= prefix+suffix) {
      set_error("width cannot be <= prefix+suffix\n");
     goto parcleanup;
    }
    outlines = reformat((const char * const *) inlines, width, prefix, suffix, hang, last, min);
    if (is_error()) goto parcleanup;
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

  if (is_error()) {
    report_error(stdout);
    clear_error();
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
