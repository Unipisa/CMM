// Contributed by Christian Heckler <chh@plato.uni-paderborn.de>
// This tests the MARKING algorithm when objects have pointers to later
// objects in the page.

#include "cmm.h"
#include <stream.h>

const int length=100000;

class TestClass: public CmmObject
{
 public:
  int data;
  TestClass *next;
  void traverse();
};

void TestClass::traverse()
{
  Cmm::heap->scavenge((CmmObject **) &next);
}

typedef TestClass* Classptr;

void createlist(Classptr p)
{
  int i;

  for (i=0; i<length; i++)
    {
      p = p->next = new TestClass;
      if (i % 10000 == 0)
	{ printf("|"); fflush(stdout); }
      else if (i % 1000 == 0) printf(".");
    }
  p->next = NULL;
}

main()
{
  Classptr p0;
  int i;

  p0 = new TestClass;

  createlist(p0);
}
