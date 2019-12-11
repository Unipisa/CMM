/* The following program tests the expansion capabilities of the garbage
   collector during collection.
*/

/* Externals */

#include <stdio.h>
#include <stdlib.h>
#include "cmm.H"

/* Define two types of cells, big_cell and little_cell.  Sizes are chosen so
   that one big_cell and one little_cell can be allocated out of the same
   page, but two big_cell's can't be.
*/

struct  big_cell : GcObject {
  big_cell  *car;
  big_cell  *cdr;
  int  value;
  int  pad[67];
  big_cell();
  void traverse();
};

void big_cell::traverse()
{
  CmmHeap *heap = CmmHeap::heap;
  heap->scavenge((GcObject **)&car);
  heap->scavenge((GcObject **)&cdr);
}

big_cell::big_cell()
{
	car = cdr = 0;
}

typedef  big_cell* bp;

struct  little_cell : GcObject {
  little_cell  *car;
  little_cell  *cdr;
  int  value;
  int  pad[47];
  little_cell();
  void traverse(CmmHeap *region);
};

void little_cell::traverse()
{
  CmmHeap *heap = CmmHeap::heap;
  heap->scavenge((GcObject **)&car);
  heap->scavenge((GcObject **)&cdr);
}

little_cell::little_cell()
{
	car = cdr = 0;
}

typedef  little_cell* lp;

gcheap  dummy(1048576, 2147483647, 1048576, 50, 45, GCMEM+GCSTATS);

main()
{
	bp  bl = 0, b1, b2;
	lp  ll = 0, l1, l2;

	for  (int i = 1; i <= 7000; i++)  {
	   b1 = new big_cell;
	   l1 = new little_cell;
	   b2 = new big_cell;
	   l2 = new little_cell;
	   b1->car = b2;
	   b1->cdr = bl;
	   b1->value = b2->value = i;
	   bl = b1;
	   l1->car = l2;
	   l1->cdr = ll;
	   l1->value = l2->value = i;
	   ll = l1;
	}
	for  (i = 7000; i >= 1; i--)  {
	   if  (bl->value != i  ||  ll->value != i)  {
	      fprintf(stderr, "Inconsistent list\n");
	      abort();
	   }
	   bl = bl->cdr;
	   ll = ll->cdr;
	}
	exit(0);
}
