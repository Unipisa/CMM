#include "cmm.h"
#include <stream.h>

/*
 * Tests use of reference types in objects.
 *
 */

#define SIZE 1000

class cell : public CmmObject 
{
public:
  char  c;			// just to check alignment problems
  cell  &car;
  cell  &cdr;
  int  value;
  cell();
  cell(cell *initcar, cell *initcdr, int initvalue);
#ifdef FIELDTABLE
  CmmFieldTable& getFieldTable()
    {
      static CmmFieldPtr fields[] = { CmmFieldPtr(&cell::car),
                                      CmmFieldPtr(&cell::cdr) };
      static CmmFieldTable table = { 2, fields };
      return table;
    }
#else
  void traverse()
    {
      CmmHeap *heap = Cmm::heap;
      heap->scavenge(CmmRefLoc(cell, car));
      heap->scavenge(CmmRefLoc(cell, cdr));
    }
#endif
};


cell::cell(cell *initcar, cell *initcdr, int initvalue)
  : car(*initcar), cdr(*initcdr), value(initvalue)
{
}

void
visitTree(cell *zp)
{
  cell *tp = zp;
  while (tp != NULL)  
    {
      printf("%x: %d  ", tp, tp->value);
      tp = &tp->cdr;
    }
  printf("\n");
  tp = zp;
  while (tp != NULL)  
    {
      printf("%x: %d  ", tp, tp->value);
      tp = &tp->car;
    }
  printf("\n");
}

void
listTest1()
{
  int  i, j;
  cell *lp = NULL, *zp;
  
  printf("List test 1\n");
  for (i = 0; i <= SIZE; i++)  
    {
      if  (i % 50 == 0)
	{ printf("."); fflush(stdout); }
      zp = new cell(NULL, lp, i);
      lp = zp;
      Cmm::heap->collect();
      zp = lp;
      for (j = i; j >= 0 ; j--)  
	{
	  if ((zp == NULL) || (zp->value != j))
	    printf("LP is damaged at j = %d\n", j);
	  zp = &zp->cdr;
	}
    }
  printf("\n");		   
}

cell *
treeTest()
{
  int  i;
  cell  *tp, *zp;

  printf("Tree test\n");
  tp = new cell(NULL, NULL, 0);
  for (i = 1; i <= 5; i++)  
    {
      zp = new cell(tp, tp, i);
      tp = zp;
    }
  Cmm::heap->collect();
  zp = new cell(tp, tp, 6);
  Cmm::heap->collect();
  visitTree(zp);
  return(zp);
}

void
listTest2()
{
  int  i, j, length = 1000, repeat = 100;
  cell  *lp = NULL, *zp;

  printf("List Test 2\n");
  for (i = 0; i < repeat; i++)  
    {
      if  (i % 50 == 0)
	{ printf("."); fflush(stdout); }
      /* Build the list */
      for  (j = 0; j < length; j++)  
	{
	  zp = new cell(NULL, lp, j);
	  lp = zp;
	}
      /* Check the list */
      zp = lp;
      for (j = length-1; j >= 0 ; j--)  
	{
	  if ((zp == NULL) || (zp->value != j))
	    printf("LP is not a good list when j = %d\n", j);
	  zp = &zp->cdr;
	}
    }
  printf("\n");		   
}

cell *gp = NULL;		/* A global pointer */

void
main()
{
  /* List construction test */
  listTest1();

  /* Tree construction test */
  gp = treeTest();

  /* 100 1000 node lists */
  listTest2();

  /* Check that tree is still there */
  visitTree(gp);
}
