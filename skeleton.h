#ifndef __SKELETON_H__
#define __SKELETON_H__

///////////////////////////////////////////////////////////////////////////////
//
// CS 1541 Introduction to Computer Architecture
// You may use this skeleton code to create a cache instance and implement cache operations.
// Feel free to add new variables in the structure if needed.
//
///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>

struct cache_blk_t {
  unsigned long tag;
  unsigned long address;
  char valid;
  char dirty;
  //pointer for linked list functionality
  //when address first enters our cache (FIFO)
  long long first;
  //when address was last recently hit(LRU)
  long long last;
};

enum cache_policy {
  LRU,
  FIFO
};

struct cache_t {
  int nsets;		// # sets
  int bsize;		// block size
  int assoc;		// associativity

  enum cache_policy policy;       // cache replacement policy
  struct cache_blk_t **blocks;    // cache blocks in the cache
};

struct cache_t *
cache_create(int size, int blocksize, int assoc, enum cache_policy policy)
{
// The cache is represented by a 2-D array of blocks. 
// The first dimension of the 2D array is "nsets" which is the number of sets (entries)
// The second dimension is "assoc", which is the number of blocks in each set.

	int i;
  int nblocks = 1; // number of blocks in the cache
  int nsets = 1;   // number of sets (entries) in the cache

  size *= 1024;
  nblocks = (size / blocksize);
  nsets = (nblocks / assoc);

  struct cache_t *C = (struct cache_t *)calloc(1, sizeof(struct cache_t));
		
  C->nsets = nsets; 
  C->bsize = blocksize;
  C->assoc = assoc;
  C->policy = policy;

  C->blocks= (struct cache_blk_t **)calloc(nsets, sizeof(struct cache_blk_t));

	for(i = 0; i < nsets; i++) {
		C->blocks[i] = (struct cache_blk_t *)calloc(assoc, sizeof(struct cache_blk_t));
	}

  return C;
}

int
cache_access(struct cache_t *cp, unsigned long address, 
             char access_type, unsigned long long now, int trace_view_on)
{
  int index, tag, i, hit = 0, emptyIndex = -1; 

  //tempaddress to show traceview
  unsigned long tempADDRESS;

  //calculate tag and index to set this address to where it belongs 
  index = (address / cp->bsize) % cp->nsets;
  tag = (address / cp->bsize) / cp->nsets;

  //Check for hit
  for(i = 0; i < cp->assoc; i++){
    if(cp->blocks[index][i].tag == tag && cp->blocks[index][i].valid == 1){
      cp->blocks[index][i].last = now;
      if(access_type == 4){
        cp->blocks[index][i].dirty = 1;
      }
      hit = 1;
      break;
    }
	//finds an empty space in cache
    else if(cp->blocks[index][i].valid == 0){
      emptyIndex = i;
      break;
    }
  }

  //return if hit
  if(hit == 1){
    if (trace_view_on) printf("HIT\n");
    return 0;
  }
  else{
    //We have an open associative position
    //fill empty spot if open
    if(emptyIndex != -1){
      cp->blocks[index][emptyIndex].tag = tag;
      cp->blocks[index][emptyIndex].valid = 1;
      cp->blocks[index][emptyIndex].last = now;
      cp->blocks[index][emptyIndex].first = now;
      if(access_type == 3){
        cp->blocks[index][emptyIndex].dirty = 0;
      }
      else if(access_type == 4){
        cp->blocks[index][emptyIndex].dirty = 1;
      }
      if (trace_view_on){ 
		printf("MISS\n") ;
		printf("NO EVICTION\n");
	
	}
      return 1;
    } 

    //evict an address
    //LRU eviction algorithm
    else if (cp->policy == 0){
      int min = cp->blocks[index][0].last;
      int minindex = 0;

      //find least recently used by finding cycle when it was used
      //older the cycle the least recent of use
      for(i = 1; i < cp->assoc; i++){
        if(cp->blocks[index][i].last < min){
          min = cp->blocks[index][i].last;
          minindex = i;
        }
      }

      char write_back = cp->blocks[index][minindex].dirty;
	if (trace_view_on){
		tempADDRESS = cp->blocks[index][minindex].tag * cp->nsets * cp->bsize + index * cp->bsize;
		if(write_back == 0)
			printf("MISS\n") ;
		else
			printf("MISS WITH WRITEBACK\n");
		printf("%X ADDRESS EVICTED\n", tempADDRESS);
	}

	//replace/evict
      cp->blocks[index][minindex].tag = tag;
      cp->blocks[index][minindex].valid = 1;
      cp->blocks[index][minindex].last = now;
      cp->blocks[index][minindex].first = now;
      if(access_type == 3){
        cp->blocks[index][minindex].dirty = 0;
      }
      else if(access_type == 4){
        cp->blocks[index][minindex].dirty = 1;
      }
	//if not dirty
      if(write_back == 0){
	
        return 1;
      }
	//if dirty
      else{
	
        return 2;
	
      }
    }

	//fifo
    else if(cp->policy == 1){
      int min = cp->blocks[index][0].first;
      int minindex = 0;

	//determine first one in
      for(i = 1; i < cp->assoc; i++){
        if(cp->blocks[index][i].first < min){
          min = cp->blocks[index][i].first;
          minindex = i;
        }
      }

      char write_back = cp->blocks[index][minindex].dirty;

	if (trace_view_on){
		tempADDRESS = cp->blocks[index][minindex].tag * cp->nsets + index * cp->bsize;
		if(write_back == 0)
			printf("MISS\n") ;
		else
			printf("MISS WITH WRITEBACK\n");
		printf("%X ADDRESS EVICTED\n", tempADDRESS);
	}

	//replace/evict
      cp->blocks[index][minindex].tag = tag;
      cp->blocks[index][minindex].valid = 1;
      cp->blocks[index][minindex].last = now;
      cp->blocks[index][minindex].first = now;
      if(access_type == 3){
        cp->blocks[index][minindex].dirty = 0;
      }
      else if(access_type == 4){
        cp->blocks[index][minindex].dirty = 1;
      }
	//dirty
      if(write_back == 0){
	
        return 1;
      }
	//not dirty
      else{
	
        return 2;
      }
    }
  }
	//////////////////////////////////////////////////////////////////////
  //
  // Your job:
  // based on address determine the set to access in cp
  // examine blocks in the set to check hit/miss
  // if miss, determine the victim in the set to replace
  // if update the block list based on the replacement policy
  // return 0 if a hit, 1 if a miss or 2 if a miss_with_write_back
  //
	//////////////////////////////////////////////////////////////////////
}

#endif