/*
 * Test for arrays built with CmmVarObject in TempHeap
 */

#include "tempheap.h"
#include <stdio.h>

class Item : public CmmObject 
{
  int x;
  Item *y;
public:
  void test() { printf("Testing ... OK\n") ; }
  void traverse() { Cmm::heap->scavenge((CmmObject **)&y); }
};

void
main()
{
  CmmHeap *tempHeap = new TempHeap(100000);

  //  Instead of 
  //  .... items = new Item[100];
  //  Use ....
  Item *items = (Item *) new (sizeof(Item) * 100, tempHeap) CmmVarObject;

  items[2].test();

  Cmm::heap->collect();

  items[2].test();
}
