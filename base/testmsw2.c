#include <stdlib.h>
#include <stdio.h>
#include "msw.h"

#define VECT_SIZE	350

char *	v1[VECT_SIZE];
char *	v2[VECT_SIZE];

static par = 0;

void
test()
{
	int i;

	for (i = 1; i < VECT_SIZE; i++) {
		v1[i] = (char *) mswAlloc(i);
		v1[i] = (char *) mswAllocOpaque(i * 80);
	}
	for (i = 1; i < VECT_SIZE; i++) {
		v1[i] = (char *) mswAllocOpaque(i * 80);
	}
}

extern void mswShowFreeChunks(void);

void 
main(int argc, char ** argv)
{

	if (argc == 2)
	      par = atoi(argv[1]);
	if (par)
	      printf("\n+++++ Using collector +++++\n");
	test();
	if (par) 
	      mswCollect();
	test();
	if (par)
	      mswCollect();
	mswShowInfo();
	mswCheckHeap(1);
}
