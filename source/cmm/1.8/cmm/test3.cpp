/* This program is used to test the ability of DefaultHeap::alloc()
   to recover from heap allocation failures.  It is run:

	expand [<initial heap> [<max heap> [<increment>]]]

   where all values are optional and expressed in megabytes.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "cmm.h"

#define MB  1048576

/* The basic data structure is a list of blocks.  */

struct  block : CmmObject {
  block*  prev;
  int  number[25000];
  block(block* ptr, int x);
  void traverse();
};

void
block::traverse()
{
  Cmm::heap->scavenge((CmmObject **)&prev);
}

block::block(block* ptr, int x)
{
	prev = ptr;
	for  (int i = 0 ; i < 25000 ; i++)  number[i] = x;
}

void
makeheap(int initial, int final, int inc)
{
  Cmm dummy(initial*MB, final*MB, inc*MB,
	    0, 1, CMM_GCTHRESHOLD, CMM_FLAGS, 0);
}

void
main(int argc, char* argv[])
{
	block  *lp = NULL;
	int  i, max;

	max = (argc < 3) ? 50 : atoi(argv[2]);
	makeheap((argc == 1) ? 1 : atoi(argv[1]),
		  max,
		  (argc < 4) ? 2 : atoi(argv[3]));
	printf("Will try to allocate up to %dMB of heap\n", max);
	for  (i = 0 ; i < 20; i++)  {
	   lp = new block(lp, i);
	   Cmm::heap->collect();
	}
	lp = new block(lp, i);
	Cmm::heap->collect();
	lp = new block(lp, i+1);
	Cmm::heap->collect();
	i = 0;
	while  (lp != NULL)  {
	   printf("%d %d ", lp->number[0], lp->number[24999]);
	   lp = lp->prev;
	   if  (++i % 10 == 0)  printf("\n");
	}
	printf("\n");
	exit(0);
}
