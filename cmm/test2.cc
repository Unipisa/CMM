/*
 * The following program tests the ability of the garbage collector to detect
 * pointers to CmmObjects from the uncollected heap.
 */

#include <stdio.h>
#include <stdlib.h>
#include "cmm.h"

/* Cell type to build a list with. */

struct  cell : CmmObject  {
  cell *next;
  int  *value1;
  int  value2;

  cell()
    {
      next = 0;
      value1 = 0;
    }

  void traverse()
    {
      CmmHeap *heap = Cmm::heap;
      heap->scavenge((CmmObject **)&next);
      heap->scavenge((CmmObject **)&value1);
    }
};

typedef  cell* cellptr;

#define TOT	50000

struct  cella  {
  cellptr ptr[TOT];
};

Cmm dummy(CMM_MINHEAP, CMM_MAXHEAP, CMM_INCHEAP, CMM_GENERATIONAL,
	  CMM_INCPERCENT, CMM_GCTHRESHOLD, CMM_HEAPROOTS | CMM_STATS, 0);

main()
{
	cella*  pointers = new cella; // allocated in uncollected heap
	cellptr cl = NULL, cp;
	int i;

	/* Allocate TOT cells referenced from array pointers */
	for  (i = 0; i < TOT; i++)  {
	   cp = new cell;
	   pointers->ptr[i] = cp;
	   cp->value1 = 0;
	   cp->value2 = i;
	}

	/* Make a list of TOT cells, each pointing to itself */
	for  (i = 0; i < TOT; i++)  {
	   cp = new cell;	// garbage
	   cp = new cell;	// garbage
	   cp = new cell;	// garbage
	   cp = new cell;
	   cp->next = cl;
	   cp->value1 = &cp->value2;
	   cp->value2 = i;
	   cl = cp;
	}

	/* Verify that cells referenced from pointers still exist */
	for  (i = 0; i < TOT; i++)
	  if (pointers->ptr[i]->value2 != i) {
	    fprintf(stderr, "cell %d not valid\n", i);
	    abort();
	  }

	/* Verify that cell list is still correct */
	for  (i = 0; i < TOT; i++)  {
	  if  (cl->value2 != *cl->value1) {
	    fprintf(stderr, "cell list damaged\n");
	    abort();
	  }
	  cl = cl->next;
	}
}
