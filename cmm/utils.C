/* utils.C -- instrumentation for the CMM				*/

/* 
   Copyright (C) 1993 Giuseppe Attardi and Tito Flagella.

   This file is part of the POSSO Customizable Memory Manager (CMM).

   CMM is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   See file 'Copyright' for full details.

*/

showHeapPages()
{
    CmmHeap *heaps[4];
    int pages[4];
    int i, j;
    int used = 0, stable = 0;
    CmmHeap *dh = CmmHeap::DefaultHeap;
    for (i = 0; i < 4; i++) {
      heaps[i] = NULL;
      pages[i] = 0;
    }
    for (i = firstHeapPage; i <= lastHeapPage; i++) {
      CmmHeap *heap = pageHeap[i];
      if (heap == dh) {
	if (pageSpace[i] == current_space) used++;
	if (STABLE(i)) stable++;
      }
      for (j = 0; j < 4; j++)
	if (heaps[j] == heap) {
	  pages[j]++;
	  break;
	}
	else if (heaps[j] == NULL && pages[j] == 0) {
	  heaps[j] = heap;
	  pages[j]++;
	  break;
	}
    }
    for (i = 0; i < 4; i++)
      printf ("Heap: 0x%x, Pages: %d\n", heaps[i], pages[i]);
    printf ("freePages: %d, allpages: %d, usedPages: %d, stablePages: %d\n",
	    freePages, lastHeapPage-firstHeapPage+1, used, stable);
}
