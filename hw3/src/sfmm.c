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
void place_into_list(sf_block* bp, int index);
void remove_from_list(sf_block* bp);
void coalesce(sf_block* bp);
int getIndex(uint64_t free_block_size);
void place_into_freelists(sf_block* bp, sf_size_t size, sf_size_t calculated_size);
int twoPower(int num);
sf_block* find_free_list_spot(sf_size_t calculated_size);
sf_size_t get_size(sf_size_t size);

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
    sf_size_t calculated_size=get_size(size);
    if(calculated_size<=176) {             //If size is <=176 then we check the quick list (index=(size-32)/16)
        if(sf_quick_lists[(calculated_size-32)/16].length!=0) {  //if the quick list is not empty
            sf_block* tempbp=sf_quick_lists[(calculated_size-32)/16].first;
            sf_block* nbp=(sf_block*)(((void*)tempbp)+calculated_size);
            int prev_alloc=(((tempbp->header)^MAGIC))&PREV_BLOCK_ALLOCATED;
            if(prev_alloc) {
                tempbp->header=((((((uint64_t)size)<<32)|calculated_size)|THIS_BLOCK_ALLOCATED)|prev_alloc)^MAGIC;
                nbp->prev_footer=((((((uint64_t)size)<<32)|calculated_size)|THIS_BLOCK_ALLOCATED)|prev_alloc)^MAGIC;
            }
            else {
                tempbp->header=(((((uint64_t)size)<<32)|calculated_size)|THIS_BLOCK_ALLOCATED)^MAGIC;
                nbp->prev_footer=(((((uint64_t)size)<<32)|calculated_size)|THIS_BLOCK_ALLOCATED)^MAGIC;
            }
            sf_quick_lists[(calculated_size-32)/16].first=tempbp->body.links.next;
            tempbp->body.links.next=0x0;
            sf_quick_lists[(calculated_size-32)/16].length--;
            return ((void*)tempbp)+16;    
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
        void* new_page = bp;
        //Footer of the added block
        bp=sf_mem_end()-16;
        ((sf_block *)bp)->prev_footer=(PACK(PAGE_SZ, prev_alloc))^MAGIC;
        //Epilogue
        ((sf_block *)bp)->header=PACK(0,THIS_BLOCK_ALLOCATED) ^ MAGIC;
        coalesce((sf_block*)new_page);
        //find space
        bp = (void *)find_free_list_spot(calculated_size);
    }
    place_into_freelists((sf_block*)bp, size, calculated_size);
    return bp+16;
}

void sf_free(void *ptr) {
    ptr-=16;
    if(ptr==NULL)
        abort();
    if(((uint64_t)ptr%16)!=0)
        abort();
    sf_block* bp = (sf_block*)ptr;
    uint64_t this_block_size=(((bp->header)^MAGIC)&0xFFFFFFFF)-(((bp->header)^MAGIC)&0xF);
    if(this_block_size<32)
        abort();
    if((this_block_size%16)!=0)
        abort();
    //ptr is prologue or before heap
    if(ptr<=sf_mem_start())
        abort();
    //ptr footer is after the end of the last block in the heap
    if((ptr+this_block_size)>sf_mem_end()-16)
        abort();
    int alloc = ((bp->header)^MAGIC)&THIS_BLOCK_ALLOCATED;
    if(!alloc)
        abort();
    int prev_alloc=((bp->header)^MAGIC)&PREV_BLOCK_ALLOCATED;
    if(prev_alloc==0) {
        int footer_prev_alloc=((bp->prev_footer)^MAGIC)&THIS_BLOCK_ALLOCATED;
        if(footer_prev_alloc)
            abort();
        else{
            uint64_t prev_block_size=(((bp->prev_footer)^MAGIC)&0xFFFFFFFF)-(((bp->prev_footer)^MAGIC)&0xF);
            sf_block* pbp=((void*)bp)-prev_block_size;
            int header_previous_alloc=((pbp->header)^MAGIC)&THIS_BLOCK_ALLOCATED;
            if(footer_prev_alloc!=header_previous_alloc)
                abort();
        }
    }
    if(this_block_size<=176) {
        if(sf_quick_lists[(this_block_size-32)/16].length>=QUICK_LIST_MAX) { //flush
            sf_block* tempbp;
            for(int i=0; i<QUICK_LIST_MAX; i++) {
                tempbp = sf_quick_lists[(this_block_size-32)/16].first;
                int prev_alloc=(((tempbp->header)^MAGIC))&PREV_BLOCK_ALLOCATED;
                sf_block* nbp=(sf_block*)(((void*)tempbp)+this_block_size);
                if(prev_alloc) {
                    tempbp->header=(this_block_size|prev_alloc)^MAGIC;
                    nbp->prev_footer=(this_block_size|prev_alloc)^MAGIC;
                }
                else {
                    tempbp->header=this_block_size^MAGIC;
                    nbp->prev_footer=this_block_size^MAGIC;
                }
                nbp->header=(((nbp->header)^MAGIC)&0xFFFFFFFD)^MAGIC;
                int next_block_size=(((nbp->header)^MAGIC)&0xFFFFFFFF)-(((nbp->header)^MAGIC)&0xF);
                sf_block* nnbp = (sf_block*)(((void*)nbp)+next_block_size);
                nnbp->prev_footer=(((nbp->header)^MAGIC)&0xFFFFFFFD)^MAGIC;
                sf_quick_lists[(this_block_size-32)/16].first=sf_quick_lists[(this_block_size-32)/16].first->body.links.next;
                tempbp->body.links.next=0x0;
                place_into_list(tempbp, getIndex(this_block_size));
                coalesce(tempbp);
            }
            bp->body.links.next=sf_quick_lists[(this_block_size-32)/16].first;
            sf_quick_lists[(this_block_size-32)/16].first=bp;
            sf_quick_lists[(this_block_size-32)/16].length++;
            int header_4bits=((bp->header)^MAGIC)&0xF;
            bp->header=((this_block_size|header_4bits)|IN_QUICK_LIST)^MAGIC; 
        }
        else { //no flush
            bp->body.links.next=sf_quick_lists[(this_block_size-32)/16].first;
            sf_quick_lists[(this_block_size-32)/16].first=bp;
            sf_quick_lists[(this_block_size-32)/16].length++;
            int header_4bits=((bp->header)^MAGIC)&0xF;
            bp->header=((this_block_size|header_4bits)|IN_QUICK_LIST)^MAGIC;        
        }
    }
    else {
        int prev_alloc=(((bp->header)^MAGIC))&PREV_BLOCK_ALLOCATED;
        sf_block* nbp=(sf_block*)(((void*)bp)+this_block_size);
        if(prev_alloc) {
            bp->header=(this_block_size|prev_alloc)^MAGIC;
            nbp->prev_footer=(this_block_size|prev_alloc)^MAGIC;
        }
        else {
            bp->header=this_block_size^MAGIC;
            nbp->prev_footer=this_block_size^MAGIC;
        }
        nbp->header=(((nbp->header)^MAGIC)&0xFFFFFFFD)^MAGIC;
        int next_block_size=(((nbp->header)^MAGIC)&0xFFFFFFFF)-(((nbp->header)^MAGIC)&0xF);
        sf_block* nnbp = (sf_block*)(((void*)nbp)+next_block_size);
        nnbp->prev_footer=(((nbp->header)^MAGIC)&0xFFFFFFFD)^MAGIC;
        place_into_list(bp, getIndex(this_block_size));
        coalesce(bp);
    }
}

void *sf_realloc(void *ptr, sf_size_t rsize) {
    ptr-=16;
    if(ptr==NULL){
        sf_errno=EINVAL;
        return NULL;
    }
    if(((uint64_t)ptr%16)!=0){
        sf_errno=EINVAL;
        return NULL;
    }
    sf_block* bp = (sf_block*)ptr;
    uint64_t this_block_size=(((bp->header)^MAGIC)&0xFFFFFFFF)-(((bp->header)^MAGIC)&0xF);
    if(this_block_size<32){
        sf_errno=EINVAL;
        return NULL;
    }
    if((this_block_size%16)!=0){
        sf_errno=EINVAL;
        return NULL;
    }
    //ptr is prologue or before heap
    if(ptr<=sf_mem_start()){
        sf_errno=EINVAL;
        return NULL;
    }
    //ptr footer is after the end of the last block in the heap
    if((ptr+this_block_size)>sf_mem_end()-16){
        sf_errno=EINVAL;
        return NULL;
    }
    int alloc = ((bp->header)^MAGIC)&THIS_BLOCK_ALLOCATED;
    if(!alloc){
        sf_errno=EINVAL;
        return NULL;
    }
    int prev_alloc=((bp->header)^MAGIC)&PREV_BLOCK_ALLOCATED;
    if(prev_alloc==0) {
        int footer_prev_alloc=((bp->prev_footer)^MAGIC)&THIS_BLOCK_ALLOCATED;
        if(footer_prev_alloc){
            sf_errno=EINVAL;
            return NULL;
        }
        else{
            uint64_t prev_block_size=(((bp->prev_footer)^MAGIC)&0xFFFFFFFF)-(((bp->prev_footer)^MAGIC)&0xF);
            sf_block* pbp=((void*)bp)-prev_block_size;
            int header_previous_alloc=((pbp->header)^MAGIC)&THIS_BLOCK_ALLOCATED;
            if(footer_prev_alloc!=header_previous_alloc){
                sf_errno=EINVAL;
                return NULL;
            }
        }
    }
    int calculated_size=get_size(rsize);
    uint64_t payload = ((bp->header)^MAGIC)>>32;
    if(rsize<=0) {
        sf_free(ptr+16);
        return NULL;
    }
    else if(calculated_size>this_block_size) {
        void* newPtr = sf_malloc(rsize);
        if(newPtr==NULL)
            return NULL;
        memcpy(newPtr+16, ptr, payload);
        sf_free(ptr+16);
        return newPtr;
    }
    else{
        int header_4bits=((bp->header)^MAGIC)&0xF;
        if(this_block_size-calculated_size<32) {
            bp->header=(((((uint64_t)rsize)<<32)|this_block_size)|header_4bits)^MAGIC;
            sf_block* nbp=(sf_block*)(((void*)bp)+this_block_size);
            nbp->prev_footer=(((((uint64_t)rsize)<<32)|this_block_size)|header_4bits)^MAGIC;
            return ((void*)bp)+16;
        }
        else{
            int new_free_block_size=this_block_size-calculated_size;
            bp->header=(((((uint64_t)rsize)<<32)|calculated_size)|header_4bits)^MAGIC;
            sf_block* nnbp=(sf_block*)(((void*)bp)+this_block_size);
            sf_block* nbp=(sf_block*)(((void*)bp)+calculated_size);
            nbp->prev_footer=(((((uint64_t)rsize)<<32)|calculated_size)|header_4bits)^MAGIC;
            nbp->header=(PACK(new_free_block_size, PREV_BLOCK_ALLOCATED))^MAGIC;
            nnbp->prev_footer=(PACK(new_free_block_size, PREV_BLOCK_ALLOCATED))^MAGIC;
            nnbp->header=(((nnbp->header)^MAGIC)&0xFFFFFFFD)^MAGIC;
            uint64_t next_next_block_size=(((nnbp->header)^MAGIC)&0xFFFFFFFF)-(((nnbp->header)^MAGIC)&0xF);
            sf_block* nnnbp=(sf_block*)(((void*)nnbp)+next_next_block_size);
            nnnbp->prev_footer=(((nnnbp->prev_footer)^MAGIC)&0xFFFFFFFD)^MAGIC;
            place_into_list(nbp, getIndex(new_free_block_size));
            coalesce((sf_block*)nbp);
            return ((void*)bp)+16;
        }
    }
    return NULL;
}

double sf_internal_fragmentation() {
    // TO BE IMPLEMENTED
    abort();
}

double sf_peak_utilization() {
    // TO BE IMPLEMENTED
    abort();
}
void place_into_list(sf_block* bp, int index) {
    bp->body.links.next=sf_free_list_heads[index].body.links.next;
    bp->body.links.prev=&sf_free_list_heads[index];
    sf_free_list_heads[index].body.links.next=bp;
    bp->body.links.next->body.links.prev=bp;
}
void remove_from_list(sf_block* bp) {
    bp->body.links.prev->body.links.next=bp->body.links.next;
    bp->body.links.next->body.links.prev=bp->body.links.prev;
    bp->body.links.next=0x0;
    bp->body.links.prev=0x0;
}
void coalesce(sf_block* bp) {
    uint64_t this_block_size=(((bp->header)^MAGIC)&0xFFFFFFFF)-(((bp->header)^MAGIC)&0xF);
    int prev_alloc=((bp->header)^MAGIC)&PREV_BLOCK_ALLOCATED;
    uint64_t prev_block_size=(((bp->prev_footer)^MAGIC)&0xFFFFFFFF)-(((bp->prev_footer)^MAGIC)&0xF);
    sf_block* nbp=(sf_block*)(((void*)bp)+this_block_size);
    sf_block* pbp=(sf_block*)(((void*)bp)-prev_block_size);
    int next_alloc=((nbp->header)^MAGIC)&THIS_BLOCK_ALLOCATED;
    uint64_t next_block_size=(((nbp->header)^MAGIC)&0xFFFFFFFF)-(((nbp->header)^MAGIC)&0xF);
    int header_4bits=0;
    uint64_t new_block_size=0;
    if(prev_alloc && next_alloc)
        return;
    else if(prev_alloc && !next_alloc) {
        header_4bits=((bp->header)^MAGIC)&0xF;
        new_block_size=this_block_size+next_block_size;
        //remove this free block from free list
        remove_from_list(bp);
        //remove next free block from free list
        remove_from_list(nbp);
        //remove next free block prev_footer and header
        nbp->prev_footer=0^MAGIC;
        nbp->header=0^MAGIC;
        //set block's header
        bp->header=(PACK(new_block_size, header_4bits))^MAGIC;
        void* new_next_block=((void*)bp)+new_block_size;
        //set block's footer
        ((sf_block*)new_next_block)->prev_footer=(PACK(new_block_size, header_4bits))^MAGIC;
        place_into_list(bp, getIndex(new_block_size));
        return;
    }
    else if(!prev_alloc && next_alloc) {
        header_4bits=((pbp->header)^MAGIC)&0xF;
        new_block_size=prev_block_size+this_block_size;
        //remove previous free block from free list
        remove_from_list(pbp);
        //remove current free block from free list
        remove_from_list(bp);
        //remove this free block prev_footer and header
        bp->prev_footer=0^MAGIC;
        bp->header=4^MAGIC;
        //set block's header
        pbp->header=(PACK(new_block_size, header_4bits))^MAGIC;
        //set block's footer
        nbp->prev_footer=(PACK(new_block_size, header_4bits))^MAGIC;
        place_into_list(pbp, getIndex(new_block_size));
        return;
    }
    else if(!prev_alloc && !next_alloc) {
        header_4bits=((pbp->header)^MAGIC)&0xF;
        new_block_size=prev_block_size+this_block_size+next_block_size;
        //remove previous free block from free list
        remove_from_list(pbp);
        //remove current free block from free list
        remove_from_list(bp);
        //remove next free block from free list
        remove_from_list(nbp);
        //remove this free block prev_footer and header
        bp->prev_footer=0^MAGIC;
        bp->header=0^MAGIC;
        //remove next free block prev_footer and header
        nbp->prev_footer=0^MAGIC;
        nbp->header=0^MAGIC;
        //set block's footer
        pbp->header=(PACK(new_block_size, header_4bits))^MAGIC;
        void* new_next_block=((void*)pbp)+new_block_size;
        //set block's footer
        ((sf_block*)new_next_block)->prev_footer=(PACK(new_block_size, header_4bits))^MAGIC;
        place_into_list(pbp, getIndex(new_block_size));
        return;
    }
    return;
}
int getIndex(uint64_t free_block_size) {
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
void place_into_freelists(sf_block* bp, sf_size_t size, sf_size_t calculated_size) {
    uint64_t free_block_size=((bp->header)^MAGIC) - (((bp->header)^MAGIC)&0xf); //get the size of the free block
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
        uint64_t new_block_size = free_block_size-calculated_size;
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
sf_block* find_free_list_spot(sf_size_t calculated_size) {
    for(int i=getIndex(calculated_size); i<NUM_FREE_LISTS; i++) {
        if(i==9 && sf_free_list_heads[i].body.links.next!=&sf_free_list_heads[i]) {             //if i==9 and it has space then jump
            sf_block* bp = sf_free_list_heads[i].body.links.next;                                //get the first free block of the free list
            while(bp!=&sf_free_list_heads[i]) {
                uint64_t free_block_size = ((bp->header)^MAGIC) - (((bp->header)^MAGIC) & 0xf);                         //get the free block size
                if(calculated_size<=free_block_size)                                             //if there enough space return pointer
                    return bp;
                bp=bp->body.links.next;
            }
            continue;   
        }
        if(calculated_size<=(twoPower(i)*M) && sf_free_list_heads[i].body.links.next!=&sf_free_list_heads[i]) {
            sf_block* bp = sf_free_list_heads[i].body.links.next;                                //get the first free block of the free list
            while(bp!=&sf_free_list_heads[i]) {
                uint64_t free_block_size = ((bp->header)^MAGIC) - (((bp->header)^MAGIC) & 0xf);                         //get the free block size
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

sf_size_t get_size(sf_size_t size) {
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
