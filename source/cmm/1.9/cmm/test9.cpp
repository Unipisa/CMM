#include "cmm.h"
#include <stream.h>

/*
 * Test contributed by Christian Heckler <chh@plato.uni-paderborn.de>
 *
 * Tests case when scavenge() allocates objects in page already scanned by
 * the collector.
 * This happens when current page contains pointer to large object, whose
 * page gets promoted and added to the queue. When this page is scanned,
 * objects it contains are moved to the current page, which however has been
 * already scanned.
 *
 */

// For this values it works
//const int length1=10;
//const int length2=15;
//const int numberoflistallocs=2;

// For this values it (mostly) does not work
const int length1=50;
const int length2=50;
const int numberoflistallocs=2;


/*
   class TestClass: In the dynamic part of objects of
   this class first two pointers are stored and then
   "counter" number of longs. Counter is determined randomly
   for each object.
*/

class TestClass: public CmmVarObject
{
  public:
  long counter;

  void traverse();

  /* Set and get the first pointer */
  void TestClass::setp1(TestClass* p);
  TestClass* TestClass::getp1();

  /* Set and get the second pointer */
  void TestClass::setp2(TestClass* p);
  TestClass* TestClass::getp2();

  /* Set and test the longs */
  void TestClass::set(long l);
  int TestClass::test(long l);
};


/*
   traverse the two pointers in the dynamic part of the object
*/

void TestClass::traverse()
{
  TestClass **q = (TestClass**) (&counter+1);
  Cmm::heap->scavenge((CmmObject **) q);
  Cmm::heap->scavenge((CmmObject **) q+1);
}

void TestClass::setp1(TestClass* p)
{
  *((TestClass**)(&counter+1)) = p;
}

TestClass* TestClass::getp1()
{
  return *((TestClass**)(&counter+1));
}

void TestClass::setp2(TestClass* p)
{
  *((TestClass**)(&counter+2))= p;
}

TestClass* TestClass::getp2()
{
  return *(((TestClass**)(&counter+2)));
}


void TestClass::set(long l)
{
  long *pl;
  pl=(long*)((TestClass**)(&counter+3));
  for (long i=0; i<counter; i++, pl++)
    *pl=l;
}

int TestClass::test(long l)
{
  long *pl;
  pl=(long*)((TestClass**)(&counter+3));
  for (long i=0; i<counter; i++, pl++)
   if (*pl != l)
    return 0;

  return 1;
}


typedef TestClass* Classptr;

/* Create a list using the first pointer of each object */
void createlist1(Classptr p)
{
  long i;
  long counter;
  Classptr localpointer, newpointer;

  localpointer=p;
  localpointer->set(0);

  counter= rand() & 700;
  newpointer = new(counter*sizeof(long)+2*sizeof(TestClass*)) TestClass;
  newpointer->counter=counter;

  localpointer->setp1(newpointer);

  localpointer = newpointer;

  for (i=1; i<length1; i++)
  {
    localpointer->set(i);
    localpointer->setp2(NULL);
    localpointer->setp1(NULL);

    counter=(rand()&700);
    newpointer = new(counter*sizeof(long)+2*sizeof(TestClass*)) TestClass;
    newpointer->counter=counter;

    localpointer->setp1(newpointer);

    localpointer = newpointer;
  }
  localpointer->set(i);
  localpointer->setp1(NULL);
  localpointer->setp2(NULL);
}


/* Create a list using the first pointer of each object */
void createlist2(Classptr p)
{
  long i;
  Classptr localpointer, newpointer;
  long counter;

  localpointer=p;

  localpointer->set(0);

  counter=(rand()&700);
  //cout << counter << " ";
  newpointer = new(counter*sizeof(long)+2*sizeof(TestClass*)) TestClass;
  newpointer->counter=counter;

  localpointer->setp2(newpointer);

  localpointer = newpointer;

  for (i=1; i<length2; i++)
  {
    localpointer->set(i);
    localpointer->setp1(NULL);
    localpointer->setp2(NULL);

    counter=(rand()&700);
    newpointer = new(counter*sizeof(long)+2*sizeof(TestClass*)) TestClass;
    newpointer->counter=counter;

    localpointer->setp2(newpointer);

    localpointer = newpointer;
  }
  localpointer->set(i);
  localpointer->setp1(NULL);
  localpointer->setp2(NULL);
}


/* Test if list 1 is still correct */
void
testlist1(Classptr p)
{
  for (long i=0; i<length1; i++)
  {
    if (!(p->test(i)))
    {
      cout << "Error in List 1 by (1," << i << ")! \n";
      exit(1);
    }
    p=p->getp1();
  }
}

/* Test if list 2 is still correct */
void
testlist2(Classptr p)
{
  for (long i=0; i<length2; i++)
  {
    if (!(p->test(i)))
    {
      cout << "Error in List 2 by (2," << i << ")! \n";
      exit(1);
    }
    p=p->getp2();
  }
}


main()
{
  Classptr p0=NULL;
  int i;
  long count;

  cout << "Process 0 is initializing the base pointer of the two lists!\n";

  count=(rand()&700);
  p0 = new(count*sizeof(long)+2*sizeof(TestClass*)) TestClass;
  p0->setp1(NULL);
  p0->setp2(NULL);
  p0->counter=count;

  cout << "Process 0 is initializing list 1!\n";
  for (i=0; i<numberoflistallocs; i++) createlist1(p0);
  cout << "Process 0 is testing list 1!\n";
  testlist1(p0);
  cout << "Process 0: 1 ok!\n";

  cout << "Process 0 is initializing list 2!\n";
  for (i=0; i<numberoflistallocs; i++) createlist2(p0);
  cout << "Process 0 is testing list 2!\n";
  testlist2(p0);
  cout << "Process 0: 2 ok!\n";
  cout << "Process 0 is testing list 1 again!\n";
  testlist1(p0);
  cout << "Process 0: 1 ok!\n";

}
