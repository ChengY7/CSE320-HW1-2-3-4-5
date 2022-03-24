/**
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "debug.h"
#include "sfmm.h"

#define PACK(size, alloc) ((size) | (alloc))
#define M 32
void coalesce(sf_block* bp);
int getIndex(int free_block_size);
void place_into_freelists(sf_block* bp, sf_size_t size, int calculated_size);
int twoPower(int num);
sf_block* find_free_list_spot(int calculated_size);
int get_size(sf_size_t size);

void heap() {
    sf_show_heap();
}
void *sf_malloc(sf_size_t size) {
    if(size<=0)
        return NULL;
    if(sf_mem_start()==sf_mem_end()) {
        for (int i=0; i<NUM_FREE_LISTS; i++) {
            sf_free_list_heads[i].body.links.next=&sf_free_list_heads[i];
            sf_free_list_heads[i].body.links.prev=&sf_free_list_heads[i];
        }
        void *HeapPointer = sf_mem_grow();
        if(HeapPointer==NULL) {
            sf_errno = ENOMEM;
            return NULL;
        }
        //prologue header
        ((sf_block *)HeapPointer)->header=PACK(32, THIS_BLOCK_ALLOCATED) ^ MAGIC;
        //First Free Block
        HeapPointer+=32;
        ((sf_block *)HeapPointer)->header=PACK(PAGE_SZ-48, PREV_BLOCK_ALLOCATED) ^ MAGIC;
        sf_free_list_heads[5].body.links.next=(sf_block *)HeapPointer;
        sf_free_list_heads[5].body.links.prev=(sf_block *)HeapPointer;
        ((sf_block *)HeapPointer)->body.links.next=&sf_free_list_heads[5];
        ((sf_block *)HeapPointer)->body.links.prev=&sf_free_list_heads[5];
        //Footer of the first Free Block
        HeapPointer=sf_mem_end()-16;
        ((sf_block *)HeapPointer)->prev_footer=PACK(PAGE_SZ-48, PREV_BLOCK_ALLOCATED) ^ MAGIC;
        //Epilogue
        ((sf_block *)HeapPointer)->header=PACK(0,THIS_BLOCK_ALLOCATED) ^ MAGIC;
    }
    int calculated_size=get_size(size);
    if(calculated_size<=176) {             //If size is <=176 then we check the quick list (index=(size-32)/16)
        if(sf_quick_lists[(calculated_size-32)/16].length!=0) {  //if the quick list is not empty
            printf("wrok on it later");
        }
    }
    void *bp = (void *)find_free_list_spot(calculated_size);     //find a free block in the free list
    while(bp==NULL) {                                            //if NULL then we have to grow the mem
        bp=sf_mem_grow();
        if(bp==NULL) {
            sf_errno = ENOMEM;
            return NULL;
        }
        //sf_block for new block
        bp-=16;
        int prev_alloc = ((((sf_block*)bp)->header)^MAGIC) & PREV_BLOCK_ALLOCATED;
        ((sf_block*)bp)->header=(PACK(PAGE_SZ, prev_alloc))^MAGIC;
        //place into free lists
        ((sf_block*)bp)->body.links.next=sf_free_list_heads[5].body.links.next;
        ((sf_block*)bp)->body.links.prev=&sf_free_list_heads[5];
        sf_free_list_heads[5].body.links.next=((sf_block*)bp);
        ((sf_block*)bp)->body.links.next->body.links.prev=((sf_block*)bp);
        //Footer of the added block
        bp=sf_mem_end()-16;
        ((sf_block *)bp)->prev_footer=(PACK(PAGE_SZ, prev_alloc))^MAGIC;
        //Epilogue
        ((sf_block *)bp)->header=PACK(0,THIS_BLOCK_ALLOCATED) ^ MAGIC;
    }
    place_into_freelists((sf_block*)bp, size, calculated_size);
    sf_show_heap();
    return bp+16;
    
    
    //not_enough_space:
    //mem grow
}

void sf_free(void *ptr) {
    // TO BE IMPLEMENTED
    abort();
}

void *sf_realloc(void *ptr, sf_size_t rsize) {
    // TO BE IMPLEMENTED
    abort();
}

double sf_internal_fragmentation() {
    // TO BE IMPLEMENTED
    abort();
}

double sf_peak_utilization() {
    // TO BE IMPLEMENTED
    abort();
}
void coalesce(sf_block* bp) {
    return;
}
int getIndex(int free_block_size) {
    if(free_block_size==M) return 0;
    else if(M<free_block_size && free_block_size<=2*M) return 1;
    else if(2*M<free_block_size && free_block_size<=4*M) return 2;
    else if(4*M<free_block_size && free_block_size<=8*M) return 3;
    else if(8*M<free_block_size && free_block_size<=16*M) return 4;
    else if(16*M<free_block_size && free_block_size<=32*M) return 5;
    else if(32*M<free_block_size && free_block_size<=64*M) return 6;
    else if(64*M<free_block_size && free_block_size<=128*M) return 7;
    else if(128*M<free_block_size && free_block_size<=256*M) return 8;
    else if(free_block_size>256*M) return 9;
    return -1;
}
void place_into_freelists(sf_block* bp, sf_size_t size, int calculated_size) {
    int free_block_size=((bp->header)^MAGIC) - (((bp->header)^MAGIC)&0xf); //get the size of the free block
    int prev_alloc = ((bp->header)^MAGIC) & PREV_BLOCK_ALLOCATED; //get the prev allocated bit of the header
    if(free_block_size-calculated_size<32) { //if free block size minus calculated block size creates splinter no need to split
        //change header to allocated
        if (prev_alloc)
            bp->header=(((((uint64_t)size << 32)|free_block_size)|THIS_BLOCK_ALLOCATED)|PREV_BLOCK_ALLOCATED)^MAGIC;
        else 
            bp->header=((((uint64_t)size << 32)|free_block_size)|THIS_BLOCK_ALLOCATED)^MAGIC;
        //remove from free lists
        bp->body.links.prev->body.links.next=bp->body.links.next;
        bp->body.links.next->body.links.prev=bp->body.links.prev;
        bp->body.links.next=0x0;
        bp->body.links.prev=0x0;
        //moving to next sf_block change prev footer and header
        bp=((void*)bp)+free_block_size;
        bp->prev_footer=0^MAGIC;
        bp->header=(((bp->header)^MAGIC)|PREV_BLOCK_ALLOCATED)^MAGIC;
        return;
    }
    else {                                      //need to split     
        int new_block_size = free_block_size-calculated_size;
        if(prev_alloc)
            bp->header=(((((uint64_t)size<<32)|calculated_size)|THIS_BLOCK_ALLOCATED)|PREV_BLOCK_ALLOCATED)^MAGIC;
        else
            bp->header=((((uint64_t)size << 32)|calculated_size)|THIS_BLOCK_ALLOCATED)^MAGIC;
        //remove from free lists
        bp->body.links.prev->body.links.next=bp->body.links.next;
        bp->body.links.next->body.links.prev=bp->body.links.prev;
        bp->body.links.next=0x0;
        bp->body.links.prev=0x0;
        //moving to next sf_block (the splited block)
        bp=((void*)bp)+calculated_size;
        if(prev_alloc)
            bp->prev_footer=(((((uint64_t)size<<32)|calculated_size)|THIS_BLOCK_ALLOCATED)|PREV_BLOCK_ALLOCATED)^MAGIC;
        else
            bp->prev_footer=((((uint64_t)size << 32)|calculated_size)|THIS_BLOCK_ALLOCATED)^MAGIC;
        bp->header=(PACK(new_block_size, PREV_BLOCK_ALLOCATED))^MAGIC;
        //add new block to free lists
        int index = getIndex(new_block_size);
        bp->body.links.next=sf_free_list_heads[index].body.links.next;
        bp->body.links.prev=&sf_free_list_heads[index];
        sf_free_list_heads[index].body.links.next=bp;
        bp->body.links.next->body.links.prev=bp;
        bp=((void*)bp)-calculated_size;
        bp=((void*)bp)+free_block_size;
        bp->prev_footer=(PACK(new_block_size, PREV_BLOCK_ALLOCATED))^MAGIC;
        return;
    } 
}
sf_block* find_free_list_spot(int calculated_size) {
    for(int i=0; i<NUM_FREE_LISTS; i++) {
        if(i==9 && sf_free_list_heads[i].body.links.next!=&sf_free_list_heads[i]) {             //if i==9 and it has space then jump
            sf_block* bp = sf_free_list_heads[i].body.links.next;                                //get the first free block of the free list
            while(bp!=&sf_free_list_heads[i]) {
                int free_block_size = ((bp->header)^MAGIC) - (((bp->header)^MAGIC) & 0xf);                         //get the free block size
                if(calculated_size<=free_block_size)                                             //if there enough space return pointer
                    return bp;
                bp=bp->body.links.next;
            }
            continue;   
        }
        if(calculated_size<=(twoPower(i)*M) && sf_free_list_heads[i].body.links.next!=&sf_free_list_heads[i]) {
            sf_block* bp = sf_free_list_heads[i].body.links.next;                                //get the first free block of the free list
            while(bp!=&sf_free_list_heads[i]) {
                int free_block_size = ((bp->header)^MAGIC) - (((bp->header)^MAGIC) & 0xf);                         //get the free block size
                if(calculated_size<=free_block_size)                                             //if there enough space return pointer
                    return bp;
                bp=bp->body.links.next;
            }
            continue;                                               //not enought space in this free list, continue
        }
        else
            continue;
    }
    return NULL;
}

int get_size(sf_size_t size) {
    if(size<=24)
        return 32;
    size+=8;
    if(size%16==0)
        return size;
    else
        return size-(size%16)+16;
}

int twoPower(int num) {
    int ans = 1;
    for(int i=0; i<num; i++)
        ans=ans*2;
    return ans;
}
