/*********************/
/* buffer.c          */
/* for Par 3.20      */
/* Copyright 1993 by */
/* Adam M. Costello  */
/*********************/

/* This is ANSI C code. */


/* additem(), copyitems(), and nextitem() rely on the fact that */
/* sizeof (char) is 1. See section A7.4.8 of The C Programming  */
/* Language, Second Edition, by Kerninghan and Ritchie.         */


#include "buffer.h"  /* Makes sure we're consistent with the */
                     /* prototypes. Also includes <stddef.h> */
#include "errmsg.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#undef NULL
#define NULL ((void *) 0)



struct buffer *newbuffer(size_t itemsize) {
  struct buffer *buf;     //init buffer
  struct block *blk;      //init block
  void *items;            //init *items, storage for the items in a block
  int maxhere;            //init maxhere, number of items that can fit in *item

  maxhere = 124 / itemsize;   //maxhere =124/size of vairable
  if (maxhere < 4) maxhere = 4;  //if maxhere < 4 then default to 4;

  buf = (struct buffer *) malloc(sizeof (struct buffer));  //malloc space for buffer
  blk = (struct block *) malloc(sizeof (struct block));    //malloc space for block
  items = malloc(maxhere * itemsize);                      //malloc space for items (max number of items * itemsize)
  if (!buf || !blk || !items) {                         
    strcpy(errmsg,outofmem);                               //if outofmem the its error
    goto nberror;
  }

  buf->itemsize = itemsize;                                //declear itemsize of buffer
  buf->firstblk = buf->current = buf->nextblk = blk;       //first block, current block, next block all equal to blk
  buf->nextindex = 0;                                      //declear nextindex
  blk->numprevious = blk->numhere = 0;                     //numprevious and numhere = 0
  blk->maxhere = maxhere;                                  //declear number of items that can fit in *item
  blk->items = items;                                      //declear *item
  blk->next=NULL;

  *errmsg = '\0';
  return buf;                                              //return the newley created buffer

  nberror:                                                  //if error, free everything and return NULL
  if (buf) free(buf);
  if (blk) free(blk);
  if (items) free(items);
  return NULL;
}


void freebuffer(struct buffer *buf)
{
  struct block *blk, *tmp;

  blk = buf->firstblk;
  while (blk) {
    tmp = blk;
    blk = blk->next;
    if (tmp->items) free(tmp->items);
    free(tmp);
  }

  free(buf);
}


void clearbuffer(struct buffer *buf)
{
  struct block *blk;

  for (blk = buf->firstblk;  blk;  blk = blk->next) {
    blk->numhere = 0;
    blk->numprevious =0;
  }

  buf->current = buf->firstblk;
}

void additem(struct buffer *buf, const void *item) {
  struct block *blk, *new;
  void *items;
  int maxhere;
  size_t itemsize = buf->itemsize;         //get the itemsize of the current buffer

  blk = buf->current;                      //blk = current block of buffer

  if (blk->numhere == blk->maxhere) {      //if current block is full
    new = blk->next;                       //set blk->next to new
    if (!new) {                            //if new is NULL
      maxhere = 2 * blk->maxhere;          //init maxhere for the new block
      new = (struct block * ) malloc(sizeof (struct block));   //malloc space for the new block
      items = malloc(maxhere * itemsize);                      //malloc space for the item inside the new block
      if (!new || !items) {
        strcpy(errmsg,outofmem);                               //if !new !items when out of memeory go to aierror
        goto aierror;
      }
      blk->next = new;                                         //declear next block
      new->next = NULL;                                        //declear the next block of the new block to null
      new->maxhere = maxhere;                                  //declear maxhere of the new block
      new->numprevious = blk->numprevious + blk->numhere;      //new block's numprevious = old block's numprevious + numhere
      new->numhere = 0;                                        //new block's numhere = 0;
      new->items = items;                                      //declear new block's item
    }
    blk = buf->current = new;                                  //update buf->current
  }

  
  memcpy( ((char *) blk->items) + (blk->numhere * itemsize), item, itemsize );  //copy item to next spot in blk->item
  
  ++blk->numhere; 
  *errmsg = '\0';
  return;                      //return 

  aierror:
  if (new) free(new);
  if (items) free(items);
}


int numitems(struct buffer *buf)
{
  struct block *blk = buf->current;
  return blk->numprevious + blk->numhere;
}


void *copyitems(struct buffer *buf) {
  int n;
  void *r;
  struct block *blk, *b;
  size_t itemsize = buf->itemsize;

  b = buf->current;                      //b = current block of buffer
  n = b->numprevious + b->numhere;       //n = numprevious + numhere (all the num of items so far)
  if (!n) return NULL;                   //if nothing in the item return NULL

  r = malloc(n * itemsize);              //malloc (num of item * item size) space
  if (!r) {            
    strcpy(errmsg,outofmem);             //if !r then return out of memory error
    return NULL;  
  }

  b = b->next;                           //get next block of current block

  for (blk = buf->firstblk;  blk != b;  blk = blk->next)    //from first block of buffer to last block
    memcpy( ((char *) r) + (blk->numprevious * itemsize),   //copy each block's item into r
            blk->items, blk->numhere * itemsize);

  *errmsg = '\0';
  return r;                                //return r
}


void rewindbuffer(struct buffer *buf)
{
  buf->nextblk = buf->firstblk;
  buf->nextindex = 0;
}


void *nextitem(struct buffer *buf)
{
  void *r;

  if (!buf->nextblk || buf->nextindex >= buf->nextblk->numhere)
    return NULL;

  r = ((char *) buf->nextblk->items) + (buf->nextindex * buf->itemsize);

  if (++buf->nextindex >= buf->nextblk->maxhere) {
    buf->nextblk = buf->nextblk->next;
    buf->nextindex = 0;
  }

  return r;
}
