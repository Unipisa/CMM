#include "heapstack.h"
#include <iostream.h>

class MyClass : public GcObject 
{
  int x;
  MyClass *y;
public:
  void test() { cout << "Testing ... Ok\n" ; }
  void traverse() { Cmm::heap->scavenge((GcObject **)&y); }
};


main()
{
  MyClass *MyVar;
  CmmHeap *MyHeap = new BBStack(100000);

  GcArray<MyClass> * MyVector = new (100, Cmm::heap) GcArray<MyClass> ;

  //  Instead of 
  //  .... MyVar = new MyClass[100];
  //  Use ....
  MyVar = (MyClass *) new (sizeof(MyClass) * 100, MyHeap) GcVarObject ;

  MyVar[2].test();

  Cmm::heap->collect();

  MyVar[2].test();
}
