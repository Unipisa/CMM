
/*
 * Test for CmmArray (requires GNU C++)
 */

#include "cmm.h"
#include <stdio.h>
#include <stdlib.h>

int count = 0;

class Item : public CmmObject
{
public:
  Item  *car;
  Item  *cdr;
  int  value1;
  char bytes[bytesPerPage+2];
  int  value2;
  Item() { value1 = ++count; };
  Item(Item* x, Item* y, int v1, int v2);
  void traverse();
  void *operator new[](size_t size);
};

typedef  Item* VP;

void
Item::traverse()
{
  CmmHeap *heap = Cmm::heap;
  heap->scavenge((CmmObject **)&car);
  heap->scavenge((CmmObject **)&cdr);
}

Item::Item(Item* x, Item* y, int v1, int v2)
{
  car = x;
  cdr = y;
  value1 = v1;
  value2 = v2;
}

void*
Item::operator new[](size_t size)
{
  return sizeof(size_t) + (char*)new(size) CmmArray<Item>;
}

void
main()
{
  int  i, j;
  VP  lp, zp;
  
  printf("Item test\n");
  lp = NULL;
  for (i = 0; i <= 100 ; i++)  
    {
      if  (i % 15 != 14)
	printf("%d ", i);
      else
	printf("%d\n", i);
      zp = new Item[8];
      zp[0] = Item(NULL, lp, i, i);
      lp = zp;
      Cmm::heap->collect();
      zp = lp;
      for (j = i; j >= 0 ; j--)  
	{
	  if ((zp == NULL) || (zp[0].value1 != j)  ||  (zp[0].value2 != j))
	    printf("LP is not a good list when j = %d\n", j);
	  zp = zp->cdr;
	}
    }
  printf("\n");		   
}
