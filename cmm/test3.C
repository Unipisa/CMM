/* This program is used to test the ability of gcalloc to recover from heap
   allocation failures.  It is run:

	expand [<initial heap> [<max heap> [<increment>]]]

   where all values are optional and expressed in megabytes.
*/

#include <string.h>
#include <stream.h>
#include <ctype.h>

#include "cmm.C"

#define MB  1048576

/* The basic data structure is a list of blocks.  */

struct  block : GcObject {
  block*  prev;
  int  number[25000];
  block(block* ptr, int x);
  void traverse();
};

void block::traverse()
{
  CmmHeap::heap->scavenge((GcObject **)&prev);
}

block::block(block* ptr, int x)
{
	prev = ptr;
	for  (int i = 0 ; i < 25000 ; i++)  number[i] = x;
}

void  makeheap(int initial, int final, int inc)
{
	gcheap  heap(initial*MB, final*MB, inc*MB, 0, 1, GCSTATS);
}

main(int argc, char* argv[])
{
	block  *lp = NULL;
	int  i;

	makeheap((argc == 1) ? 1 : atoi(argv[1]),
		  (argc < 3) ? 1000 : atoi(argv[2]),
		  (argc < 4) ? 2 : atoi(argv[3]));
	for  (i = 0 ; expandfailed != 1 &&  i < 20; i++)  {
	   lp = new block(lp, i);
	   gccollect();
	}
	lp = new block(lp, i);
	gccollect();
	lp = new block(lp, i+1);
	gccollect();
	i = 0;
	while  (lp != NULL)  {
	   cout << lp->number[0] << " " << lp->number[24999] << "  ";
	   lp = lp->prev;
	   if  (++i % 10 == 0)  cout << "\n";
	}
	cout << "\n";
	exit(0);
}
