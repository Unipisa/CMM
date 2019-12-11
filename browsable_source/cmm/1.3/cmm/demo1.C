#include "HeapStack.H"

class cell : GcObject		// This class is a GC class
{
  int x;

public: 
  cell *next;			// This field is a pointer to a GC class,
				// and must be traversed.

  void traverse();		// Because "cell" has internal pointers
				// traverse must be defined.
};

void cell::traverse()
 {
   CmmHeap::heap->scavenge((GcObject **)&next); 
                               // traverse scavenge the internal pointer
                               // next to reach other cells.
 }

main()
{
  CmmHeap *MyHeap = new BBStack(100);
                               // Create the new heap MyHeap.
                               // Here you can use any of the predefined
                               // heaps.

  cell *t = new cell;		// Create a new cell. 
				// Because you have not specified a heap
				// with new, the global variable heap is used.
				// heap is initialized to the default heap.

  t->next = new (MyHeap) cell;	// Create another cell, but in MyHeap

  heap = MyHeap;
  t->next->next = new cell;	// Setting heap to MyHeap, you can allocate cells
				// from MyHeap, without specifing it as
				// a parameter to new.

  heap->collect();		// Collecting on MyHeap
  DefaultHeap->collect();	// Collecting on the Default heap.

  heap = DefaultHeap;		// Reset heap before returning.
}
