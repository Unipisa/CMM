/* The following program tests the ability of the garbage collector to correct
   derived pointers and pointers in the non-garbage collected heap.
*/

/* Externals */

#include <stdio.h>
#include <stdlib.h>
#include "cmm.h"

/* Cell type to build a list with. */

struct  cell : CmmObject  {
  cell  *next;
  int* value1;
  int  value2;
  cell();
  void traverse();
};

void
cell::traverse()
{
  CmmHeap *heap = Cmm::heap;
  heap->scavenge((CmmObject **)&next);
  heap->scavenge((CmmObject **)&value1);
}

cell::cell()
{
  next = 0;
  value1 = 0;
}

typedef  cell* cellptr;

/* Dynamic array of cellptr's that is not garbage collected */

struct  cella  {
  cellptr ptr[50000];
};

Cmm  dummy(1048576, 2147483647, 1048576, 35, 30, CMM_HEAPROOTS+CMM_STATS,
	   0, 0);

main()
{
	cella*  pointers = new cella;
	cellptr  cl = 0, cp;
	int i;

	/* Allocate 50000 cells referenced by an array in the non-gc heap */
	for  (i = 0; i < 50000; i++)  {
	   cp = new cell;
	   pointers->ptr[i] = cp;
	   cp->value1 = 0;
	   cp->value2 = i;
	}

	/* Make a list of 50000 cells that each point into themseleves. */
	for  (i = 0; i < 50000; i++)  {
	   cp = new cell;
	   cp = new cell;
	   cp = new cell;
	   cp = new cell;
	   cp->next = cl;
	   cp->value1 = &cp->value2;
	   cp->value2 = i;
	   cl = cp;
	}

	/* Verify that objects referenced by pointers still exist */
	for  (i = 0; i < 50000; i++)  {
	   if  (pointers->ptr[i]->value2 != i)  abort();
	}

	/* Verify that cell list is correct correct */
	for  (i = 0; i < 50000; i++)  {
	   if  (cl->value2 != *(cl->value1))  abort();
	   cl = cl->next;
	}
	exit(0);
}
