#include "cmm.h"


#include <stdio.h>
#include <stdlib.h>
#include "tempheap.h"

struct  cell : CmmObject 
{
  cell  *car;
  cell  *cdr;
  int  value;
  cell(cell *initcar, cell *initcdr, int initvalue);
  void traverse();
};

typedef  cell* CP;

void cell::traverse()  
{
  CmmHeap *heap = Cmm::heap;
  heap->scavenge((CmmObject **)&car);
  heap->scavenge((CmmObject **)&cdr);
}


cell::cell(cell *initcar, cell *initcdr, int initvalue)
{
  car = initcar;
  cdr = initcdr;
  value = initvalue;
}

struct vector : CmmObject 
{
  vector  *car;
  vector  *cdr;
  int  value1;
  char bytes[1000];
  int  value2;
  vector(vector* x, vector* y, int v1, int v2);
  void traverse();
};

typedef  vector* VP;

void vector::traverse()
{
  CmmHeap *heap = Cmm::heap;
  heap->scavenge((CmmObject **)&car);
  heap->scavenge((CmmObject **)&cdr);
}

vector::vector(vector* x, vector* y, int v1, int v2)  
{
  car = x;
  cdr = y;
  value1 = v1;
  value2 = v2;
}


void  vectortest()
{
  int  i, j;
  VP  lp, zp;
  
  printf("Vector test\n");
  lp = NULL;
  for (i = 0; i <= 100 ; i++)  
    {
      if  (i % 15 != 14)
	printf("%d ", i);
      else
	printf("%d\n", i);
      zp = new vector[10](NULL, lp, i, i);
      lp = zp;
      Cmm::heap->collect();
      zp = lp;
      for (j = i; j >= 0 ; j--)  
	{
	  if ((zp == NULL) || (zp->value1 != j)  ||  (zp->value2 != j))
	    printf("LP is not a good list when j = %d\n", j);
	  zp = zp->cdr;
	}
    }
  printf("\n");		   
}

main()
{
  /* List of vectors > 1 page */
  vectortest();


  exit(0);
}
