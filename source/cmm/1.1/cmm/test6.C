#include "HeapStack.H"

class MyClass : GcObject 
{
  int x;
  MyClass *y;
public:
  void test() { cout << "Testing ... Ok\n" ; }
  void traverse() { CmmHeap::heap->scavenge((GcObject **)&y); }
};


main()
{
  MyClass *MyVar;
  CmmHeap *MyHeap = new BBStack(100000);

  GcArray<MyClass> * MyVector = new (100, heap) GcArray<MyClass> ;

  //  Instead of 
  //  .... MyVar = new MyClass[100];
  //  Use ....
  MyVar = (MyClass *) new (sizeof(MyClass) * 100, MyHeap) GcVarObject ;

  MyVar[2].test();

  heap->collect();

  MyVar[2].test();
}
