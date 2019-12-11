#include "HeapStack.H"

struct  cell : GcObject  {
  cell  *next;
  int  value;
  cell();
  void traverse();
};

void cell::traverse()
{
    CmmHeap::heap->scavenge((GcObject **)&next);
}

cell::cell()
{
  next = 0;
}

cell root2;				    // Define a cell variable

main()
{
  cell *root1 = new cell;		    // Define a cell pointer
  HeapStack *MyHeap = new HeapStack(10000); // Create a new heap

  MyHeap->roots.setp((GcObject **)&root1);  // Register the pointer as a root
  MyHeap->roots.set(&root2);                // Register the cell as a root

  root1->next = new(MyHeap) cell;	    // Allocates some new cells
  root2.next = new(MyHeap) cell;

  MyHeap->collect();			    // The collector will identify
					    // any allocated cell, starting
					    // traversing from cell root1 & root2

  MyHeap->roots.unsetp((GcObject **)&root1);
                                            // Deregister the local root.
}
