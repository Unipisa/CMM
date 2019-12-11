/*---------------------------------------------------------------------------*
 *  utils.cc: instrumentation for the CMM
 *
 *  Copyright (C) 1993 Giuseppe Attardi and Tito Flagella.
 *
 *   This file is part of the POSSO Customizable Memory Manager (CMM).
 *
 *
 * Permission to use, copy, and modify this software and its documentation is
 * hereby granted only under the following terms and conditions.  Both the
 * above copyright notice and this permission notice must appear in all copies
 * of the software, derivative works or modified versions, and any portions
 * thereof, and both notices must appear in supporting documentation.
 *
 * Users of this software agree to the terms and conditions set forth herein,
 * and agree to license at no charge to all parties under these terms and
 * conditions any derivative works or modified versions of this software.
 * 
 * This software may be distributed (but not offered for sale or transferred
 * for compensation) to third parties, provided such third parties agree to
 * abide by the terms and conditions of this notice.  
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE COPYRIGHT HOLDERS DISCLAIM ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL THE COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *---------------------------------------------------------------------------*/

showHeapPages()
{
    CmmHeap *heaps[4];
    int pages[4];
    int i, j;
    int nextPages = 0, stable = 0;
    DefaultHeap *dh = Cmm::theDefaultHeap;
    for (i = 0; i < 4; i++) {
      heaps[i] = NULL;
      pages[i] = 0;
    }
    for (i = firstHeapPage; i <= lastHeapPage; i++) {
      CmmHeap *heap = pageHeap[i];
      if (heap == dh) {
	if (pageSpace[i] == nextSpace) nextPages++;
	if (pageIsStable(i)) stable++;
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
	    freePages, lastHeapPage-firstHeapPage+1, dh->usedPages, stable);
    if (dh->usedPages != nextPages + stable
	|| dh->stablePages != stable
	|| dh->reservedPages != pages[0])
      printf ("nextPages: %d, Cmm::stablePages: %d, reservedPages: %d\n",
	      nextPages, dh->stablePages, dh->reservedPages);
}
