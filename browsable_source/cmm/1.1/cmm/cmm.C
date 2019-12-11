/* This module implements the POSSO Customisable Memory Management (CMM)
   CMM provides garbage collected storage for C++ programs.

   The technique is described in:

   G. Attardi and T. Flagella ``A customisable memory management
   framework'', Proceedings of USENIX C++ Conference 1994, Cambridge,
   Massachusetts, April 1994.

   The implementation is derived from the code of the "mostly-copying" garbage
   collection algorithm, by Joel Bartlett, of DEC.

*/

/* 
   Copyright (C) 1993 Giuseppe Attardi and Tito Flagella.

   This file is part of the POSSO Customizable Memory Manager (CMM).

   CMM is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   See file 'Copyright' for full details.

*/

/*
 *              Copyright 1990 Digital Equipment Corporation
 *                         All Rights Reserved
 *
 * Permission to use, copy, and modify this software and its documentation is
 * hereby granted only under the following terms and conditions.  Both the
 * above copyright notice and this permission notice must appear in all copies
 * of the software, derivative works or modified versions, and any portions
 * thereof, and both notices must appear in supporting documentation.
 *
 * Users of this software agree to the terms and conditions set forth herein,
 * and hereby grant back to Digital a non-exclusive, unrestricted, royalty-free
 * right and license under any changes, enhancements or extensions made to the
 * core functions of the software, including but not limited to those affording
 * compatibility with other hardware or software environments, but excluding
 * applications which incorporate this software.  Users further agree to use
 * their best efforts to return to Digital any such changes, enhancements or
 * extensions that they make and inform Digital of noteworthy uses of this
 * software.  Correspondence should be provided to Digital at:
 * 
 *                       Director of Licensing
 *                       Western Research Laboratory
 *                       Digital Equipment Corporation
 *                       250 University Avenue
 *                       Palo Alto, California  94301  
 * 
 * This software may be distributed (but not offered for sale or transferred
 * for compensation) to third parties, provided such third parties agree to
 * abide by the terms and conditions of this notice.  
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND DIGITAL EQUIPMENT CORP. DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL DIGITAL EQUIPMENT
 * CORPORATION BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#include "machine.H"
#include "cmm.H"
#include <setjmp.h>

/* Version tag */

char*  CmmVersion = "CMM 1.3";

extern "C"  void* sbrk(int);

       /**************************************
	* Garbage Collected Heap Definitions *
	**************************************/

/*
 * The heap consists of a discontiguous set of pages of memory, where each
 * page is BYTESxPAGE long.  N.B.  the page size for garbage collection is
 * independent of the processor's virtual memory page size.
 */

static int  totalPages,		/* # of pages in the heap		*/
	    heapSpanPages,	/* # of pages that span the heap	*/
	    freePages,		/* # of pages not yet allocated		*/
	    freeWords = 0,	/* # words left on the current page	*/
	    *freep,		/* Ptr to the first free word on the current
				   page */
	    freePage,		/* First possible free page		*/
	    *pageLink,		/* Page link for each page		*/
	    queue_head,		/* Head of list of stable set of pages	*/
	    queue_tail;     	/* Tail of list of stable set of pages	*/

int	    firstHeapPage,	/* Page # of first heap page		*/
	    lastHeapPage,	/* Page # of last heap page		*/
	    *ObjectMap;		/* Bitmap of objects			*/
#if !HEADER_SIZE
int	    *LiveMap;		/* Bitmap of live objects		*/
#endif


short *pageSpace;		/* Space number for each page		*/
static short *pageGroup;	/* Size of group of pages		*/

int	    current_space;	/* Current space number			*/
int	    next_space;		/* Next space number			*/

CmmHeap    **pageHeap;		/* Heap to which each page belongs	*/

#ifndef DELETE_TABLES
static int tablePages,		/* # of pages used by tables	*/
firstTablePage;			/* index of first page used by table	*/
#endif

/*
 * Groups of pages used for objects spanning multiple pages, are dealt
 * as follows:
 *
 * The first page p0 in a group contains the number n of pages in the group,
 * each of the following pages contains the offset from the first page:
 * 	pageGroup[p0] = n
 * 	pageGroup[p0+1] = -1
 * 	pageGroup[p0+n-1] = 1-n
 * Given a page p, we can compute the first page by p+pageGroup[p] if
 * pageGroup[p] < 0, otherwise p.
 */   

/*
 Tito:
   A new algorithm for garbage collection is used when MARKING is defined.
   The new algorithm is:
   1) Look at the roots to promote a set of pages whose objects cannot
      be moved. Any reachable object is marked as a reached object setting
      its bit into the "LiveMap" bitmap to 1.
   2) Scan the promoted pages, traversing all the objects marked as reached.
   Traverse applies the scavenge to any pointer internal to the traversed
   objects. scavenge does:
    - if the pointer is outside the heap do nothing;
    - if the pointer is to an object in another heap traverse that object.
    - if the object has already been reached, then if the object is in 
      a promoted page return, else set the pointer to the forward position.
    - if the object has not yet been reached, then if the object is in 
      a promoted page, mark it and apply traverse to it, else copy the object,
      set the old header to the forward position, and set the forward bit of
      the new object to 0.

   Note that any page allocated to copy reachable objects is added to the
   promoted set. For this reason you don't need to apply traverse to
   moved objects. They will be traversed as they are reachable objects in
   promoted pages.

   3) Reset the mark bit for any object in the promoted pages.
 */


       /**********************************
        * Exported Interface Definitions *
	**********************************/

#ifdef CMM_VERBOSE
int CmmVerbosity = 1;		/* controls amount of printout */
#endif

/*
 * An instance of the type gcheap is created to configure the size of the
 * initial heap, the expansion increment, the maximum size of the heap,
 * the allocation percentage to force a total collection, the allocation
 * percentage to force heap expansion, and garbage collection options.
 */

/* Default heap configuration */

const int  GCMINBYTES = 131072,		/* # of bytes of initial heap	*/
	   GCMAXBYTES = 2147483647,	/* # of bytes of the final heap */
	   GCINCBYTES = 1048576,	/* # of bytes of each increment	*/
	   GCFULLGCTHRESHOLD = 35,	/* % allocated to force total
					   collection		       	*/
	   GCINCPERCENT = 25,		/* % allocated to force expansion */
	   GCFLAGS = 0;			/* option flags			*/

/* Actual heap configuration */

static int  gcminbytes = GCMINBYTES,	/* # of bytes of initial heap	*/
	    gcmaxbytes = GCMAXBYTES,	/* # of bytes of the final heap */
	    gcincbytes = GCINCBYTES,	/* # of bytes of each increment */
	    gcFullGcThreshold = GCFULLGCTHRESHOLD,/* % allocated to force
					    total collection		*/
	    gcincpercent = GCINCPERCENT,/* % allocated to force expansion */
	    gcflags = GCFLAGS;		/* option flags			*/
static bool gcdefaults = true,		/* default setting in force	*/
	    gcheapcreated = false;	/* boolean indicating heap created */

gcheap::gcheap(int minheapbytes,
		int maxheapbytes,
		int incheapbytes,
		int FullGcThreshold,
		int incpercent,
		int flags)  {
	if  (!gcheapcreated  &&  minheapbytes > 0  &&
	     (gcdefaults || maxheapbytes >= gcmaxbytes))  {
	   gcdefaults = false;
	   gcminbytes = minheapbytes;
	   gcmaxbytes = maxheapbytes;
	   gcincbytes = incheapbytes;
	   gcFullGcThreshold = FullGcThreshold;
	   gcincpercent = incpercent;
	   gcminbytes = MAX(gcminbytes, 4*BYTESxPAGE);
	   gcmaxbytes = MAX(gcmaxbytes, gcminbytes);
	   if  (gcFullGcThreshold < 0  ||  gcFullGcThreshold > 50)
	      gcFullGcThreshold = GCFULLGCTHRESHOLD;
	   if  (gcincpercent < 0  ||  gcincpercent > 50)
	      gcincpercent = GCINCPERCENT;
	}
	if  (gcheapcreated  &&  minheapbytes > 0  &&
	     (gcdefaults || maxheapbytes >= gcmaxbytes))  {
	   gcdefaults = false;
	   if  (getenv("GCMAXBYTES") == NULL)  gcmaxbytes = maxheapbytes;
	   if  (getenv("GCINCBYTES") == NULL)  gcincbytes = incheapbytes;
	   if  (getenv("GCFULLGCTHRESHOLD") == NULL)
	     gcFullGcThreshold = FullGcThreshold;
	   if  (getenv("GCINCPERCENT") == NULL)  gcincpercent = incpercent;
	   gcminbytes = MAX(gcminbytes, 4*BYTESxPAGE);
	   gcmaxbytes = MAX(gcmaxbytes, gcminbytes);
	   if  (gcFullGcThreshold < 0  ||  gcFullGcThreshold > 50)
	      gcFullGcThreshold = GCFULLGCTHRESHOLD;
	   if  (gcincpercent < 0  ||  gcincpercent > 50)
	      gcincpercent = GCINCPERCENT;
	}
	gcflags = gcflags | flags;
}

/*
 * Freespace objects have a tag of 0.
 * Pad objects for double alignment have a tag of 1.
 * GcObjects have a tag of 2.
 * The header for a one-word double alignment pad is kept in doublepad.
 */

#if HEADER_SIZE
static int  freespace_tag = MAKE_TAG(0);
#ifdef DOUBLE_ALIGN
static int  doublepad = MAKE_HEADER(1, MAKE_TAG(1));
#endif
#endif

#ifdef DOUBLE_ALIGN
#define ONEPAGEOBJ_WORDS (WORDSxPAGE-HEADER_SIZE)
#else
#define ONEPAGEOBJ_WORDS WORDSxPAGE
#endif


/***********************************************************************
  Roots
 ***********************************************************************/

/*
 * The following structure contains the additional roots registered with
 * the garbage collector.  It is allocated from the non-garbage collected
 * heap.
 */

static int  roots_count = 0;
static int  roots_size = 0;
#define	    ROOTS_INC 10
static int  freed_entries = 0;

static struct  roots_struct  {
  unsigned*  addr;	/* Address of the roots */
  int  bytes;		/* Number of bytes in the roots */ 
}  *roots;


/*----------------------------------------------------------------------
 * gcroots --
 *
 * Additional roots are "registered" with the garbage collector by the
 * following procedure.
 *______________________________________________________________________*/

void  gcroots(void* addr, int bytes)
{
  if (freed_entries) {
    for  (int i = 0; i < roots_count; i++) {
      if (roots[i].addr == 0) {
	roots[i].addr = (unsigned*)addr;
	roots[i].bytes = bytes;
	freed_entries--;
      }
    }
  }
  
  if  (roots_count == roots_size)   {
    roots_struct  *np;
    roots_size += ROOTS_INC;
    np = new roots_struct[roots_size];
    for  (int i = 0; i < roots_count; i++)  np[i] = roots[i];
    delete  roots;
    roots = np;
  }
  roots[roots_count].addr = (unsigned*)addr;
  roots[roots_count].bytes = bytes;
  roots_count++;
}

void  gcunroots(void* addr)
{
  int i;
  for  (i = 0; i < roots_count; i++) 
    if (roots[i].addr == addr) {
      roots[i].addr = 0;
      freed_entries++;
      break;
    }
  assert(i < roots_count);
}

	/****************************
	 * Mostly Copying Collector *
	 ****************************/

/*----------------------------------------------------------------------
 * environment_value --
 *
 * Get heap configuration information from the environment.
 *
 * Results: true if the value is provided, value in value.
 *______________________________________________________________________*/

static int  environment_value(char* name, int& value)
{
	 char* valuestring = getenv(name);

	 if (valuestring != NULL)  {
	    value = atoi(valuestring);
	    return  1;
	 }
	 return  0;
}

#if !HEADER_SIZE
/*
 * Go forward until next object, return the size in words.
 */
int GcObject::words()
{

  register int length = 1;
  register int index = WORD_INDEX(this+1);
  int shift = BIT_INDEX(this+1);
  register unsigned int bits = (unsigned int)ObjectMap[index] >> shift;
  register int inner = BITSxWORD - shift;
  int nextPage = GCP_to_PAGE(this);
  nextPage += pageGroup[nextPage];
  int max = ((int)PAGE_to_GCP(nextPage) - (int)this) / (BITSxWORD * BYTESxWORD);

  do {
    do {
      if (bits & 1) return length;
      bits = bits >> 1;
      length++;
    } while (--inner);
    bits = (unsigned int)ObjectMap[++index];
    inner = BITSxWORD;
  } while (max--);
  /* we fall out here when this is last object on page */
  return (GcObject *)PAGE_to_GCP(nextPage) - this;
}

/* Version using ffs.
   Counts the number of consecutive 1's in ObjectMap, which encode
   half the number of words of the object.
   We assume that object are an even number of words.
int words()
{
  int length = 0, bits,
  index = WORD_INDEX(this),
  shift = BIT_INDEX(this);

  while (true) {
    bits = (unsigned int)ObjectMap[index] >> shift;
    inc = ffs(~bits) - 1;
    if (inc < 0) inc = BITSxWORD;
    if (inc == (BITSxWORD - shift)) break;
    length += inc;
    index++;
    shift = 0;
  }
  return 2*length;
}

A setObjectMap which goes with this is:

setObjectMap(GCP p, int size)
{
  int index = WORD_INDEX(p),
  shift = BIT_INDEX(p);
  size = size / 2;
  while (true) {
    count = size % (BITSxWORD - shift);
    ObjectMap[index] |= (1 << count) - 1;
    size -= count;
    if (size == 0) break;
    index++;
    shift = 0;
  }
}
*/
#endif

/***********************************************************************
  Initialization
 ***********************************************************************/

#if !HEADER_SIZE
/*
 * An object of this class is used to fill free portion of page.
 */
class GcFreeObject: public GcObject {
  void traverse() {}
  int words() { return WORDSxPAGE; }
};

static GcFreeObject *aGcFreeObject;

#ifdef DOUBLE_ALIGN_OPTIMIZE
/*
 * An object of this class is used for padding.
 */
class GcPadObject: public GcObject {
  void traverse() {}
  int words() { return 1; }
};

static GcPadObject *aGcPadObject;
#endif
#endif

DefaultHeap *CmmHeap::theDefaultHeap = NULL;
CmmHeap *CmmHeap::heap = NULL;

// used during initialization of objects:
static GcObject *aGcObject;
static GcVarObject *aGcVarObject;

static unsigned  stackBottom;	// The base of the stack
static unsigned *globalHeapStart; // start of global heap

CmmInitEarly() {
  int i;
  if  (stackBottom == 0) {

#define STACKBOTTOM_ALIGNMENT_M1 0xffffff
#ifdef STACK_GROWS_DOWNWARD
    stackBottom = ((unsigned)&i + STACKBOTTOM_ALIGNMENT_M1)
      & ~STACKBOTTOM_ALIGNMENT_M1;
#else
    stackBottom = (unsigned)&i & ~STACKBOTTOM_ALIGNMENT_M1;
#endif

    /* Determine start of system heap				*/
    globalHeapStart = (unsigned *)sbrk(0);
  }
}

// #define CMM_DEBUG
#ifdef CMM_DEBUG
#include "utils.C"
#endif

/*----------------------------------------------------------------------
 * CmmInit --
 *
 * The heap is allocated and the appropriate data structures are initialized
 * by the following function.  It is called the first time any storage is
 * allocated from the heap.
 *______________________________________________________________________*/

static void  CmmInit()
{
  char  *heap;
  int  i;
  
  /* Log actual heap parameters if from environment or logging */
  if  ((environment_value("GCMINBYTES", gcminbytes) |
	environment_value("GCMAXBYTES", gcmaxbytes) |
	environment_value("GCINCBYTES", gcincbytes) |
	environment_value("GCFULLGCTHRESHOLD", gcFullGcThreshold) |
	environment_value("GCINCPERCENT", gcincpercent) |
	environment_value("GCFLAGS", gcflags))  ||
       gcflags & GCSTATS)  {
    fprintf(stderr,
	    "***** gcalloc  gcheap(%d, %d, %d, %d, %d, %d)\n",
	    gcminbytes, gcmaxbytes, gcincbytes, gcFullGcThreshold,
	    gcincpercent, gcflags);
  }
  
  /* Allocate heap and side tables.  Exit on allocation failure. */
#ifdef DELETE_TABLES
  heapSpanPages = totalPages = gcminbytes/BYTESxPAGE;
  if  ((heap = new char[totalPages*BYTESxPAGE + BYTESxPAGE - 1]) == NULL ||
       (pageSpace = new short[heapSpanPages]) == NULL  ||
       (pageLink = new int[heapSpanPages]) == NULL  ||
       (pageGroup = new short[heapSpanPages]) == NULL  ||
       (pageHeap = new CmmHeap*[heapSpanPages]) == NULL  ||
       (ObjectMap = new int[heapSpanPages*WORDSxPAGE/BITSxWORD]) == NULL
#if !HEADER_SIZE
       || (LiveMap = new int[heapSpanPages*WORDSxPAGE/BITSxWORD]) == NULL
#endif
       ) {
    fprintf(stderr, 
	    "\n****** gcalloc  Unable to allocate %d byte heap\n",
	    gcminbytes);
    abort();
  }
  if ((unsigned)heap & (BYTESxPAGE - 1))
    heap = heap + (BYTESxPAGE - ((unsigned)heap & (BYTESxPAGE - 1)));
  firstHeapPage = GCP_to_PAGE(heap);
  lastHeapPage = firstHeapPage + heapSpanPages - 1;
  freePages = totalPages;

#else

  heapSpanPages = totalPages = (gcminbytes + BYTESxPAGE - 1)/BYTESxPAGE;
  tablePages = (totalPages*sizeof(int)*2 /* pageLink, pageHeap */
		+ totalPages*sizeof(short)*2 /* pageSpace, pageGroup */
		+ totalPages*WORDSxPAGE/BITSxWORD*sizeof(int) /* ObjectMap */
#if !HEADER_SIZE
		+ totalPages*WORDSxPAGE/BITSxWORD*sizeof(int) /* LiveMap */
#endif
		+ BYTESxPAGE - 1) / BYTESxPAGE;
  /* Allocate one block for both the heap and the tables.
   * The tables will be recycled into pages at the next collection.
   */
  if  ((heap = new char[(totalPages + tablePages)*BYTESxPAGE + BYTESxPAGE - 1])
       == NULL) {
    fprintf(stderr, 
	    "\n****** CMM  Unable to allocate %d byte heap\n", gcminbytes);
    abort();
  }
  heap = heap + BYTESxPAGE - 1;
  heap -= (int)heap % BYTESxPAGE;
  firstHeapPage = GCP_to_PAGE(heap);
  lastHeapPage = firstHeapPage + heapSpanPages - 1;
  firstTablePage = lastHeapPage + 1;
  freePages = totalPages;

  pageSpace = (short *)PAGE_to_GCP(firstTablePage);
  pageGroup = &pageSpace[totalPages];
  pageLink = (int *)&pageGroup[totalPages];
  pageHeap = (CmmHeap **)&pageLink[totalPages];
  ObjectMap = (int *)&pageHeap[totalPages];
#if !HEADER_SIZE
  LiveMap = (int *)&ObjectMap[totalPages*WORDSxPAGE/BITSxWORD];
#endif

#endif DELETE_TABLES

  /* The following definitions are safe because these vectors are accessed
     only through an address within a page. Instead of using
     pageSpace[addr - firstHeapPage]
     space is displaced by firstHeapPage so that we can use:
     pageSpace[addr]
     */

  pageSpace = pageSpace - firstHeapPage;
  pageLink = pageLink - firstHeapPage;
  pageGroup  = pageGroup  - firstHeapPage;
  pageHeap  = pageHeap  - firstHeapPage;
  ObjectMap = ObjectMap - WORD_INDEX(firstHeapPage*BYTESxPAGE);
#if !HEADER_SIZE
  LiveMap = LiveMap - WORD_INDEX(firstHeapPage*BYTESxPAGE);
#endif

  /* Initialize tables */
  for (i = firstHeapPage ; i <= lastHeapPage ; i++) {
    pageHeap[i] = NOHEAP;
//    pageSpace[i] = UNALLOCATEDSPACE;
  }
  current_space = 3;		// leave 1 as UNALLOCATEDSPACE
  next_space = 3;
  freePage = firstHeapPage;
  queue_head = 0;
  gcheapcreated = true;

  CmmHeap::theDefaultHeap->usedPages = 0;
  CmmHeap::theDefaultHeap->reservedPages = 0;
  CmmHeap::theDefaultHeap->stablePages = 0;
  CmmHeap::theDefaultHeap->firstUnusedPage =
    CmmHeap::theDefaultHeap->firstReservedPage =
      CmmHeap::theDefaultHeap->lastReservedPage = firstHeapPage;

#if !HEADER_SIZE
  aGcFreeObject = ::new GcFreeObject;
#ifdef DOUBLE_ALIGN_OPTIMIZE
  aGcPadObject = ::new GcPadObject;
#endif
#endif

  // The following initializations are needed by the GcObject::new 
  // operator. For this reason they don't use new, but ::new.
  aGcObject = ::new GcObject;
  aGcVarObject = ::new GcVarObject;
}

/*----------------------------------------------------------------------
 * shouldExpandHeap --
 *
 * Once the heap has been allocated, it is automatically expanded after garbage
 * collection until the maximum size is reached.  If space cannot be allocated
 * to expand the heap, then the heap will be left it's current size and no
 * further expansions will be attempted.
 *
 * Results: true when the heap should be expanded.
 *______________________________________________________________________*/

static int  shouldExpandHeap()
{
  return  (HEAPPERCENT(CmmHeap::theDefaultHeap->stablePages) >= gcincpercent
	   && totalPages < gcmaxbytes/BYTESxPAGE  &&  gcincbytes != 0);
}

static void  (*save_new_handler)();

static void  dummy_new_handler() { set_new_handler(save_new_handler); }

static  expandfailed = 0;

/*----------------------------------------------------------------------
 * expandHeap --
 *
 * Expands the heap by gcincbytes.
 *
 * Results: number of first new page allocated, 0 on failure
 *______________________________________________________________________*/

static int  expandHeap()
{
  int  inc_totalPages = gcincbytes/BYTESxPAGE,
  new_firstHeapPage = firstHeapPage,
  inc_firstHeapPage,
  new_lastHeapPage = lastHeapPage,
  inc_lastHeapPage,
  new_totalPages,
  *new_pageLink = NULL,
  *new_ObjectMap = NULL,
#if !HEADER_SIZE
  *new_LiveMap = NULL,
#endif
  i;

  short *new_pageSpace = NULL;
  short *new_pageGroup = NULL;
  CmmHeap **new_pageHeap = NULL;

#ifndef DELETE_TABLES
  char  *new_tables;
  int   new_tablePages;
#endif

  char*  inc_heap;

  /* Check for previous expansion failure */
  if  (expandfailed)  return  0;

  /* Allocate additional heap and determine page span */
  save_new_handler = set_new_handler(dummy_new_handler);
#ifdef DELETE_TABLES
  inc_heap = new char[inc_totalPages*BYTESxPAGE + BYTESxPAGE - 1];
  if  (inc_heap == NULL)  goto fail;
  if ((unsigned)inc_heap & (BYTESxPAGE - 1))
    inc_heap = inc_heap +
      (BYTESxPAGE - ((unsigned)inc_heap & (BYTESxPAGE - 1)));
  inc_firstHeapPage = GCP_to_PAGE(inc_heap);
  inc_lastHeapPage = inc_firstHeapPage + inc_totalPages - 1;
  if  (inc_firstHeapPage < firstHeapPage)
    new_firstHeapPage = inc_firstHeapPage;
  if  (inc_lastHeapPage > lastHeapPage)
    new_lastHeapPage = inc_lastHeapPage;
  new_totalPages = totalPages + inc_totalPages;
  heapSpanPages = new_lastHeapPage - new_firstHeapPage + 1;
  
  /* Allocate contiguous space for each side table, recover gracefully
     from allocation failure.  */
  if  ((new_pageSpace = new short[heapSpanPages]) == NULL  ||
       (new_pageLink = new int[heapSpanPages]) == NULL  ||
       (new_pageGroup = new short[heapSpanPages]) == NULL  ||
       (new_pageHeap = new CmmHeap*[heapSpanPages]) == NULL  ||
       (new_ObjectMap = new int[heapSpanPages*WORDSxPAGE/BITSxWORD]) == NULL
#if !HEADER_SIZE
       ||  (new_LiveMap = new int[heapSpanPages*WORDSxPAGE/BITSxWORD]) == NULL
#endif
       ) {
  fail:	   set_new_handler(save_new_handler);
    if  (inc_heap)  delete inc_heap;
    if  (new_pageSpace)  delete new_pageSpace;
    if  (new_pageLink)  delete new_pageLink;
    if  (new_pageGroup)  delete new_pageGroup;
    if  (new_pageHeap)  delete new_pageHeap;
    if  (new_ObjectMap)  delete new_ObjectMap;
#if !HEADER_SIZE
    if  (new_LiveMap)  delete new_LiveMap;
#endif
    expandfailed = 1;
    WHEN_FLAGS (GCSTATS,
		fprintf(stderr,
			"\n***** gcalloc  Heap expansion failed\n"));
    return  0;
  }
  set_new_handler(save_new_handler);
  new_pageSpace = new_pageSpace - new_firstHeapPage;
  new_pageLink = new_pageLink - new_firstHeapPage;
  new_pageGroup = new_pageGroup - new_firstHeapPage;
  new_pageHeap = new_pageHeap - new_firstHeapPage;
  new_ObjectMap = new_ObjectMap - WORD_INDEX(new_firstHeapPage*BYTESxPAGE);
#if !HEADER_SIZE
  new_LiveMap = new_LiveMap - WORD_INDEX(new_firstHeapPage*BYTESxPAGE);
#endif

  /* Initialize gaps in between blocks of pages */
  for (i = inc_lastHeapPage + 1 ; i <= firstHeapPage ; i++)
    new_pageHeap[i] = UNCOLLECTEDHEAP;
  for (i = lastHeapPage + 1 ; i < inc_firstHeapPage ; i++)
    new_pageHeap[i] = UNCOLLECTEDHEAP;

#else				// !DELETE_PAGES

  inc_heap = new char[inc_totalPages*BYTESxPAGE + BYTESxPAGE - 1];
  if (inc_heap == NULL) goto fail;
  inc_heap = inc_heap + BYTESxPAGE - 1;
  inc_heap -= (int)inc_heap % BYTESxPAGE;
  inc_firstHeapPage = GCP_to_PAGE(inc_heap);
  inc_lastHeapPage = inc_firstHeapPage + inc_totalPages - 1;
  new_firstHeapPage = MIN(firstHeapPage,
			  MIN(firstTablePage, inc_firstHeapPage));
  new_lastHeapPage = MAX(lastHeapPage,
			 MAX(firstTablePage + tablePages - 1,
			     inc_lastHeapPage));
  new_totalPages = totalPages + tablePages + inc_totalPages;
  heapSpanPages = new_lastHeapPage - new_firstHeapPage + 1;

  new_tablePages = (heapSpanPages*sizeof(int)*2 /* pageLink, pageHeap */
		    + heapSpanPages*sizeof(short)*2 /* pageSpace, pageGroup */
		    + heapSpanPages*WORDSxPAGE/BITSxWORD*sizeof(int) /* ObjectMap */
#if !HEADER_SIZE
		    + heapSpanPages*WORDSxPAGE/BITSxWORD*sizeof(int) /* LiveMap */
#endif
		    + BYTESxPAGE - 1) / BYTESxPAGE;
  if ((new_tables = new char[new_tablePages*BYTESxPAGE + BYTESxPAGE - 1])
      == NULL) {
  fail: set_new_handler(save_new_handler);
    if  (inc_heap)  delete inc_heap;
    expandfailed = 1;
    WHEN_FLAGS (GCSTATS,
		fprintf(stderr, "\n***** CMM  Heap expansion failed\n"));
    return  0;
  }
  set_new_handler(save_new_handler);
  new_pageSpace = (short *)new_tables;
  new_pageGroup = &new_pageSpace[heapSpanPages];
  new_pageLink = (int *)&new_pageGroup[heapSpanPages];
  new_pageHeap = (CmmHeap **)&new_pageLink[heapSpanPages];
  new_ObjectMap = (int *)&new_pageHeap[heapSpanPages];
#if !HEADER_SIZE
  new_LiveMap = (int *)&new_ObjectMap[heapSpanPages*WORDSxPAGE/BITSxWORD];
#endif

  new_pageSpace = new_pageSpace - new_firstHeapPage;
  new_pageLink = new_pageLink - new_firstHeapPage;
  new_pageGroup = new_pageGroup - new_firstHeapPage;
  new_pageHeap = new_pageHeap - new_firstHeapPage;
  new_ObjectMap = new_ObjectMap - WORD_INDEX(new_firstHeapPage*BYTESxPAGE);
#if !HEADER_SIZE
  new_LiveMap = new_LiveMap - WORD_INDEX(new_firstHeapPage*BYTESxPAGE);
#endif

  /* Recycle old tables */
  int lastTablePage = firstTablePage + tablePages - 1;
  for (i = firstTablePage; i <= lastTablePage; i++)
    new_pageHeap[i] = NOHEAP;
  /* Fill gaps */
  int gapStart = MIN(lastTablePage, inc_lastHeapPage);
  int gap1Start = MIN(lastHeapPage, gapStart);

  int gapEnd = MAX(firstTablePage, inc_firstHeapPage);
  int gap2End = MAX(firstHeapPage, gapEnd);

  int gap1End = (gapEnd == gap2End) ?
    MAX(firstHeapPage, MIN(firstTablePage, inc_firstHeapPage)) : gapEnd;
  int gap2Start = (gapStart == gap1Start) ?
    MIN(lastHeapPage, MAX(lastTablePage, inc_lastHeapPage)) : gapStart;
  for (i = gap1Start + 1; i < gap1End; i++)
    new_pageHeap[i] = UNCOLLECTEDHEAP;
  for (i = gap2Start + 1; i < gap2End; i++)
    new_pageHeap[i] = UNCOLLECTEDHEAP;
#endif DELETE_TABLES

  /* Initialize new side tables */
  for (i = inc_firstHeapPage ; i <= inc_lastHeapPage ; i++)
    new_pageHeap[i] = NOHEAP;
  for (i = firstHeapPage ; i <= lastHeapPage ; i++) {
    new_pageSpace[i] = pageSpace[i];
    new_pageHeap[i] = pageHeap[i];
    new_pageLink[i] = pageLink[i];
    new_pageGroup[i] = pageGroup[i];
  }
  for  (i = WORD_INDEX(firstHeapPage*BYTESxPAGE);
	i < WORD_INDEX((lastHeapPage + 1)*BYTESxPAGE); i++) {
    new_ObjectMap[i] = ObjectMap[i];
#if !HEADER_SIZE
    // necessary if expandHeap() is called during collection
    new_LiveMap[i] = LiveMap[i];
#endif
  }

#ifdef DELETE_TABLES
  delete (pageSpace + firstHeapPage);
  delete (pageLink + firstHeapPage);
  delete (pageGroup + firstHeapPage);
  delete (pageHeap + firstHeapPage);
  delete (ObjectMap + WORD_INDEX(firstHeapPage*BYTESxPAGE));
#if !HEADER_SIZE
  delete (LiveMap + WORD_INDEX(firstHeapPage*BYTESxPAGE));
#endif
#endif
  pageSpace = new_pageSpace;
  pageLink = new_pageLink;
  pageGroup = new_pageGroup;
  pageHeap = new_pageHeap;
  ObjectMap = new_ObjectMap;
#if !HEADER_SIZE
  LiveMap = new_LiveMap;
#endif
  firstHeapPage = new_firstHeapPage;
  lastHeapPage = new_lastHeapPage;
  totalPages = new_totalPages;
#ifdef DELETE_TABLES
  freePages += inc_totalPages;
#else
  freePages += inc_totalPages + tablePages;
  tablePages = new_tablePages;
  firstTablePage = GCP_to_PAGE(new_tables);
#endif
  freePage = inc_firstHeapPage;
#ifdef CMM_DEBUG
  showHeapPages();
#else
#ifdef CMM_VERBOSE
  printf("Heap pages: %d\n", totalPages);
#endif
#endif
  WHEN_FLAGS (GCSTATS,
	      fprintf(stderr,
		      "\n***** gcalloc  Heap expanded to %d bytes\n",
		      totalPages*BYTESxPAGE));
  return  inc_firstHeapPage;
}

/*----------------------------------------------------------------------
 * empty_stableset --
 *
 * Moves the stable set back into the current_space.
 * A total collection is performed by calling this before calling
 * collect().  When generational collection is not desired, this is called
 * after collection to empty the stable set.
 *______________________________________________________________________*/

static void  empty_stableset()
{
  CmmHeap::theDefaultHeap->stablePages = 0;
  while  (queue_head)  {
    int i = queue_head;
    int pages = pageGroup[i];
    while (pages--)
      pageSpace[i++] = current_space;
    queue_head = pageLink[queue_head];
  }
}

/*----------------------------------------------------------------------
 * queue --
 *
 * Adds a page to the stable set page queue.
 *______________________________________________________________________*/

static void  queue(int page)
{
  if  (queue_head != 0)
    pageLink[queue_tail] = page;
  else 
    queue_head = page;
  pageLink[page] = 0;
  queue_tail = page;
}

/*----------------------------------------------------------------------
 * promote_page --
 *
 * Pages that have might have references in the stack or the registers are
 * promoted to the stable set.
 *
 * Note that objects that get allocated in a CONTINUED page (after a large
 * object) will never move.
 *______________________________________________________________________*/

static void  promote_page(GCP cp)
{
  int page = GCP_to_PAGE(cp);
  
  // Don't promote pages belonging to other heaps.
  
  if  (page >= firstHeapPage  &&  page <= lastHeapPage
       && pageHeap[page] == CmmHeap::theDefaultHeap
       && pageSpace[page] == current_space) {
    
#ifdef MARKING
    MARK(basePointer(cp));
    // given the basePointer we could avoid computing pagecount
#endif
    
    int pages = pageGroup[page];
    if (pages < 0) { page += pages; pages = pageGroup[page]; }

    WHEN_FLAGS (GCDEBUGLOG,
		fprintf(stderr, "promoted 0x%x\n", PAGE_to_GCP(page)));
    queue(page);
    CmmHeap::theDefaultHeap->usedPages += pages; // in next_space
    CmmHeap::theDefaultHeap->stablePages += pages;
    while (pages--)
      pageSpace[page++] = next_space;
  }
}

/*----------------------------------------------------------------------
 * basePointer --
 *
 * Results: pointer to the beginning of the containing object
 *______________________________________________________________________*/ 

GcObject *basePointer(GCP fp)
{
  fp = (GCP) ((int)fp & ~(BYTESxWORD-1));
/*
  while (! IS_OBJECT(fp))
    fp--;
  return (GcObject *)fp;  
*/
  register int index = WORD_INDEX(fp);
  register int inner = BIT_INDEX(fp);
  register int mask = 1 << inner;
  register unsigned int bits = (unsigned int)ObjectMap[index];

  do {
    do {
      if (bits & mask) return (GcObject *)fp;
      mask = mask >> 1;
      fp--;
    } while (inner--);
    bits = (unsigned int)ObjectMap[--index];
    inner = BITSxWORD-1;
    mask = 1 << BITSxWORD-1;
  } while (true);
}

/*----------------------------------------------------------------------
 * CmmHeap::scavenge --
 *
 * Replaces pointer to (within) object with pointer to scavenged object
 *
 * Results: none
 *
 * Side effects: freep, freeWords, usedPages
 *______________________________________________________________________*/

void DefaultHeap::scavenge(GcObject **ptr) {

  if (inside((GCP)*ptr)) {

    GcObject *p = basePointer((GCP)*ptr);

    if (inside((GCP)p))
      *ptr = (GcObject *)((int)CmmMove((GCP)p) + (int)*ptr - (int)p);
#ifdef MARKING
    else
      // Here you can decide to traverse or not objects in other heaps.
      // If not, simply return. Here we traverse.
      visit(p);
#endif
    }
}

/*----------------------------------------------------------------------
 * CmmMove --
 *
 * Copies object from current_space to next_space
 *
 * Results: pointer to header of copied object
 *
 * Side effects: freep, freeWords, usedPages
 *______________________________________________________________________*/

GCP CmmMove(GCP cp)
{
  int  page = GCP_to_PAGE(cp);	/* Page number */
  GCP  np;			/* Pointer to the new object */
#if HEADER_SIZE
  int  header;			/* Object header */
#endif

  /* Verify that the object is a valid pointer and decrement ptr cnt */
  WHEN_FLAGS (GCTSTOBJ, verify_object(cp, 1));

  if  (STABLE(page))
#ifdef MARKING
    {
      if (MARKED(cp))
	return(cp);
      else {
	MARK(cp);
	if (HEADER_TAG(*(cp - HEADER_SIZE)) > 1)
	  ((GcObject *)cp)->traverse();
	return(cp);
      }
    }
#else
  return(cp);
#endif
  /* If cell is already forwarded, return forwarding pointer */
#if HEADER_SIZE
  header = cp[-HEADER_SIZE];
  if  (FORWARDED(header)) {
    WHEN_FLAGS (GCTSTOBJ, {
      verify_object((GCP)header, 0);
      verify_header((GCP)header);
    });
    return ((GCP)header);
  }
#else
  if  (FORWARDED(cp))
    return ((GCP)*cp);
#endif

  /* Move the object */
  WHEN_FLAGS (GCTSTOBJ, verify_header(cp));

  /* Forward or promote object */
#if HEADER_SIZE
  register int  words = HEADER_WORDS(header);
#else
  register int  words = ((GcObject *)cp)->words();
#endif
  if  (words >= freeWords)  {
    /* Promote objects >= a page to stable set */
    /* This is to avoid expandHeap(). See note about collect().
     * We could perform copying during a full GC by reserving in advance
     * a block of pages for objects >= 1 page
     */
    if  (words >= ONEPAGEOBJ_WORDS)  {
      promote_page(cp);
      /* tito: you don't need to traverse it now.
       * Object will be traversed, when the promoted page will be swept.
       */
      return(cp);
    }
    /* Discard any partial page and allocate a new one */
    // We must ensure that this does not invoke expandHeap()
    CmmHeap::theDefaultHeap->reserve_pages(1);
    WHEN_FLAGS (GCDEBUGLOG, fprintf(stderr, "queued   0x%x\n", freep));
    queue(GCP_to_PAGE(freep));
    CmmHeap::theDefaultHeap->stablePages += 1;
  }
  /* Forward object, leave forwarding pointer in old object header */
#if HEADER_SIZE
  *freep++ = header;
#else
  GCP ocp = cp;
#endif
  np = freep;
  SET_OBJECTMAP(np);
  freeWords = freeWords - words;
#if HEADER_SIZE
  cp[-HEADER_SIZE] = (int)np;	// lowest bit 0 means forwarded
  words -= HEADER_SIZE;
  while  (words--)  *freep++ = *cp++;
#ifdef DOUBLE_ALIGN
  if  ((freeWords & 1) == 0  &&  freeWords)  {
    *freep++ = doublepad;
    freeWords = freeWords - 1;
  }
#endif
#else
  MARK(cp); // Necessary to recognise as forwarded */
  while  (words--)  *freep++ = *cp++;
  *ocp = (int)np;
#endif
#ifdef MARKING
  MARK(np);
  /* tito: no need to traverse it now.
     Object will be traversed, when the promoted page will be swept.
     */
#endif
  return(np);
}

/*----------------------------------------------------------------------
 * DefaultHeap::collect --
 *
 * Garbage collection for the DefaultHeap. It is typically
 * called when half the pages in the heap have been allocated.
 * It may also be directly called.
 * 
 * WARNING: (freePages + reserveedPages - usedPages) must be > usedPages when
 * collect() is called to avoid the invocation of expandHeap() in the
 * middle of collection.
 *______________________________________________________________________*/

void DefaultHeap::collect()
{
  int  page;			/* Page number while walking page list */
  GCP  cp,			/* Pointers to move constituent objects */
  nextcp;	
//  static doFullGC = false;
    
  /* Check for heap not yet allocated */
  if  (!gcheapcreated)  {
    CmmInit();
    return;
  }
#ifdef CMM_DEBUG
  printf(">"); showHeapPages();
#endif
  
  /* Log entry to the collector */
  WHEN_FLAGS (GCSTATS, {
    fprintf(stderr, "***** gcalloc  Collecting - %d%% allocated  ->  ",
	    HEAPPERCENT(usedPages));
    newline_if_logging();
  });
  
  /* Allocate rest of the current page */
  if (freeWords != 0)  {
#if HEADER_SIZE
    *freep = MAKE_HEADER(freeWords, freespace_tag);
#else
    *freep = *(GCP)aGcFreeObject;
    SET_OBJECTMAP(freep);
#endif
    freeWords = 0;
  }
  
//  if (freePages + reservedPages < 2 * usedPages)
//    expandHeap();

  //  if (doFullGC) empty_stableset();

  /* Advance space.
   * Pages allocated by CmmMove() herein will belong to the stable set.
   * At the end of collect() we go back to normal.
   * Therefore objects moved once by the collector will not be moved again
   * until a full collection is enabled by empty_stableset().
   */

  next_space = current_space + 1;
  usedPages = stablePages;	// start counting in next_space

#if !HEADER_SIZE
  /* Clear the LiveMap bitmap */
  bzero((char*)&LiveMap[WORD_INDEX(firstHeapPage * BYTESxPAGE)],
	heapSpanPages * (BYTESxPAGE / BITSxWORD));
#endif
  /* Examine stack, registers, static area and possibly the non-garbage
     collected heap for possible pointers */
  WHEN_FLAGS (GCROOTLOG, fprintf(stderr, "stack roots:\n"));
  {
    jmp_buf regs;
    unsigned *fp;		/* Pointer for checking the stack */
#ifdef STACK_GROWS_DOWNWARD
    register unsigned *lim = (unsigned *)regs;
#else
    register unsigned *lim = (unsigned *)regs + sizeof(regs);
#endif
    extern int end;

    /* ensure flushing of register caches */
    if (_setjmp(regs) == 0) _longjmp(regs, 1);

#ifdef STACK_GROWS_DOWNWARD
    for (fp = lim; fp < (unsigned *)stackBottom; fp++) {
      WHEN_FLAGS (GCROOTLOG, log_root(fp));
      promote_page((GCP)*fp);
    }
#else
    for (fp = lim; fp > (unsigned *)stackBottom; fp--) {
      WHEN_FLAGS (GCROOTLOG, log_root(fp));
      promote_page((GCP)*fp);
    }
#endif
    
    WHEN_FLAGS (GCROOTLOG,
		fprintf(stderr, "static and register roots:\n"));
    for  (fp = DATASTART ; fp < (unsigned *)&end ; fp++)  {
      if (fp == (unsigned *)&freep) continue;
      // Maybe: *fp == freep or better
      // (*fp >= freep && *fp < PAGE_to_GCP(GCP_to_PAGE(freep) + 1))
      WHEN_FLAGS (GCROOTLOG, log_root(fp));
      promote_page((GCP)*fp);
    }
    for  (int i = 0; i < roots_count; i++)  {
      fp = roots[i].addr;
      for  (int j = roots[i].bytes; j > 0; j = j - BYTESxWORD)
	promote_page((GCP)*fp++);
    }
    if  (gcflags & GCHEAPROOTS)  {
      WHEN_FLAGS (GCHEAPLOG,
		  fprintf(stderr, "Uncollected heap roots:\n"));
      unsigned *globalHeapEnd = (unsigned *)sbrk(0);
      fp = globalHeapStart;
      while  (fp < globalHeapEnd)  {
	if  (!inside((GCP)fp))  {
	  WHEN_FLAGS (GCHEAPLOG, log_root(fp));
	  if  (gcflags & GCHEAPROOTS)
	    promote_page((GCP)*fp);
	  fp++;
	}
	else
	  fp = fp + WORDSxPAGE;	// skip page
      }
    }
  }
  WHEN_FLAGS (GCSTATS, {
    fprintf(stderr, "%d%% locked  ", HEAPPERCENT(usedPages));
    newline_if_logging();
  });
  
  /* Sweep across stable pages and move their constituent items.	 */
  page = queue_head;
  while  (page)  {
    cp = PAGE_to_GCP(page);
    WHEN_FLAGS (GCDEBUGLOG, fprintf(stderr, "sweeping 0x%x\n", cp));
    GCP nextPage = PAGE_to_GCP(page + 1);
    bool inCurrentPage = (page == GCP_to_PAGE(freep));
    nextcp = inCurrentPage ? freep : nextPage;
    /* current page may get filled while we sweep it */
    while (cp < nextcp ||
	   inCurrentPage && cp < (nextcp = (cp <= freep && freep < nextPage) ?
				  freep : nextPage)) {
      WHEN_FLAGS (GCTSTOBJ, verify_header(cp + HEADER_SIZE));
#if HEADER_SIZE
      if ((HEADER_TAG(*cp) > 1)
#ifdef MARKING
	  || MARKED(cp + HEADER_SIZE)
#endif
	  )
	((GcObject *)(cp + HEADER_SIZE))->traverse();
      cp = cp + HEADER_WORDS(*cp);
#else
      ((GcObject *)cp)->traverse();
      cp = cp + ((GcObject *)cp)->words();
#endif
    }
    page = pageLink[page];
  }
  
  /* Finished, all retained pages are now part of the stable set */
  current_space = current_space + 2;
  next_space = current_space;
  WHEN_FLAGS (GCSTATS,
	      fprintf(stderr, "%d%% stable.\n", HEAPPERCENT(stablePages)));
  
  /* Check for total collection and heap expansion.  */
  if  (gcFullGcThreshold)  {
    /* Performing generational collection */
    /*    if (doFullGC) {
	  doFullGC = false;
	  if  (shouldExpandHeap())  expandHeap();
	  }
	  else */
    if  (HEAPPERCENT(usedPages) >= gcFullGcThreshold)  {
      /* Perform a total collection and then expand the heap */
      //      doFullGC = true;
      empty_stableset();
      int  save_gcFullGcThreshold = gcFullGcThreshold;
      gcFullGcThreshold = 100;
      cp = NULL;		// or collect will promote it again
      collect();
      if  (shouldExpandHeap())  expandHeap();
      gcFullGcThreshold = save_gcFullGcThreshold;
    }
  }
  else  {
    /* Not performing generational collection */
    if  (shouldExpandHeap())  expandHeap();
    empty_stableset();
  }
#ifdef CMM_DEBUG
  printf("<"); showHeapPages();
#endif
}

/*----------------------------------------------------------------------
 * next_page --
 *
 * Results: index of next page (wrapped at the end)
 *______________________________________________________________________*/

static inline int  next_page(int page)
{
  return  (page == lastHeapPage) ? firstHeapPage : page + 1;
}

/*----------------------------------------------------------------------
 * allocate_page --
 *
 * Page allocator.
 * Allocates a number of additional pages to the indicated heap.
 *
 * Results: address of first page
 *
 * Side effects: freePage
 *______________________________________________________________________*/

GCP  allocate_page(int pages, CmmHeap *heap)
{
  int  free,		/* # contiguous free pages */
  firstPage,	/* Page # of first free page */
  allPages; /* # of pages in the heap */
  GCP  firstByte;	/* address of first free page */
  
  allPages = heapSpanPages;
  free = 0;
  firstPage = freePage;
  while  (allPages--)  {
    if  (pageHeap[freePage] == NOHEAP)  {
      if  (++free == pages)  goto FOUND;
    } else
      free = 0;
    freePage = next_page(freePage);
    if  (freePage == firstHeapPage)  free = 0;
    if (free == 0) firstPage = freePage;
  }
  /* Failed to allocate space, try expanding the heap.  Assure
     that minimum increment size is at least the size of this object.
     */
  if  (!gcheapcreated)  CmmInit(); /* initialize heap, if not done yet */
  gcincbytes = MAX(gcincbytes, pages*BYTESxPAGE);
  if  ((firstPage = expandHeap()) == 0)  {
    /* Can't do it */
    fprintf(stderr,
	    "\n***** allocate_page  Unable to allocate %d pages\n", pages);
    abort();
  }
 FOUND:
  // Ok, I found all needed contiguous pages.
  freePages -= pages;
  firstByte = PAGE_to_GCP(firstPage);
  int i = 1;
  while (pages--) {
    pageHeap[firstPage+pages] = heap;
#if !HEADER_SIZE
    // Fake groups so that words() works also outside the DefaultHeap;
    pageGroup[firstPage+pages] = i++;
#endif
  }
  return firstByte;
}

/*----------------------------------------------------------------------
 * DefaultHeap::reserve_pages --
 *
 * When alloc() is unable to allocate storage, it calls this routine to
 * allocate one or more pages.  If space is not available then the garbage
 * collector is called and/or the heap is expanded.
 *
 * Results: address of first page
 *
 * Side effects: freePage, freep, freeWords, usedPages
 *______________________________________________________________________*/

GCP  DefaultHeap::reserve_pages(int pages)
{
  int firstPage;		/* Page # of first free page	*/
  int i;
  
  /* Garbage collect if not enough pages will be left for collection.	*/
  if  (current_space == next_space && /* not within CmmMove()  		*/
       usedPages - stablePages + pages >
       freePages + reservedPages - usedPages - pages)  {
    // freep is seen by the collector: it should not consider it as a root.
    collect();
  }
  /* Discard any remaining portion of current page */
  if  (freeWords != 0)  {
#if HEADER_SIZE
    *freep = MAKE_HEADER(freeWords, freespace_tag);
#else
    *freep = *(GCP)aGcFreeObject;
    SET_OBJECTMAP(freep);
#endif
    freeWords = 0;
  }
  if (reservedPages - usedPages > reservedPages / 16) {
    // not worth looking for the last few ones dispersed through the heap
    int free = 0; /* # contiguous free pages	*/
    int allPages = lastReservedPage - firstReservedPage;
    firstPage = firstUnusedPage;
    while  (allPages--)  {
      if  (pageHeap[firstUnusedPage] == this &&
	   // in previous generation
	   pageSpace[firstUnusedPage] != current_space  &&
	   UNSTABLE(firstUnusedPage))  { // but not in stable set
	if  (++free == pages) {
	  freep = PAGE_to_GCP(firstPage);
	  goto FOUND;
	}
      } else {
	free = 0;
	firstPage = firstUnusedPage+1;
      }
      if (firstUnusedPage == lastReservedPage) {
	firstUnusedPage = firstPage = firstReservedPage;
	free = 0;
      } else
	firstUnusedPage++;
    }
  }
  {
    int reserved = MAX(8, pages); // get a bunch of them
    freep = allocate_page(reserved, this);
    firstUnusedPage = firstPage = GCP_to_PAGE(freep);
    i = firstPage + reserved - 1;
    lastReservedPage = MAX(lastReservedPage, i);
    reservedPages += reserved;
    for (i = pages; i < reserved; i++)
      pageSpace[firstPage + i] = UNALLOCATEDSPACE;
  }
 FOUND:
  // I found all needed contiguous pages.
  bzero((char*)freep, pages*BYTESxPAGE);
#if HEADER_SIZE && defined(DOUBLE_ALIGN)
  *freep++ = doublepad;
  freeWords = pages*WORDSxPAGE - 1;
#else
  freeWords = pages*WORDSxPAGE;
#endif
  usedPages += pages;
  bzero((char*)&ObjectMap[WORD_INDEX(firstPage*BYTESxPAGE)],
	pages*(BYTESxPAGE/BITSxWORD));
  pageSpace[firstPage] = next_space;
  pageGroup[firstPage] = pages;
  i = -1;
  while (--pages)  {
    pageSpace[++firstPage] = next_space;
    pageGroup[firstPage] = i--;
  }
  return freep;
}

/*----------------------------------------------------------------------
 * DefaultHeap::alloc --
 *
 * Storage is allocated by the following function.
 * It is up to the specific constructor procedure to assure that all
 * pointer slots are correctly initialized.
 *
 * Results: pointer to the object
 *
 * Side effects: freep, freeWords
 *______________________________________________________________________*/

GCP DefaultHeap::alloc(int size)
{
  GCP  object;		/* Pointer to the object */
  
  size = BYTEStoWORDS(size); // add size of header
  
  /* Try to allocate from current page */
  if  (size <= freeWords)  {
#if HEADER_SIZE
    object = freep;
    freeWords = freeWords - size;
    freep = freep + size;
#ifdef DOUBLE_ALIGN
    if  ((freeWords & 1) == 0  &&  freeWords)  {
      *freep++ = doublepad;
      freeWords = freeWords - 1;
    }
#endif
    return(object);
#else				// !HEADER_SIZE
#ifdef DOUBLE_ALIGN_OPTIMIZE
    if (size < 16 || ((int)freep & 7) == 0) {
#endif
      object = freep;
      freeWords = freeWords - size;
      freep = freep + size;
      return(object);
#ifdef DOUBLE_ALIGN_OPTIMIZE
    } else if (size <= freeWords - 1) {
      SET_OBJECTMAP(freep);
      *freep++ = *(GCP)aGcPadObject;
      object = freep;
      freeWords = freeWords - size - 1;
      freep = freep + size;
      return(object);
    }
#endif
#endif
  }
  /* Object fits in one page with left over free space*/
  if  (size < ONEPAGEOBJ_WORDS)  {
    reserve_pages(1);
    object = freep;
    freeWords = freeWords - size;
    freep = freep + size;
#if HEADER_SIZE && defined(DOUBLE_ALIGN)
    if  ((freeWords & 1) == 0  &&  freeWords)  {
      *freep++ = doublepad;
      freeWords = freeWords - 1;
    }
#endif
    return(object);
  }
  
  /* Object >= 1 page in size.
   * It is allocated at the beginning of next page.
   */
#if HEADER_SIZE
  if  (size > MAX_HEADER_WORDS)  {
    fprintf(stderr,
	    "\n***** gcalloc  Unable to allocate objects larger than %d bytes\n",
	    MAX_HEADER_WORDS * BYTESxWORD - BYTESxWORD);
    abort();
  }
#endif
#if HEADER_SIZE && defined(DOUBLE_ALIGN)
  reserve_pages((size + WORDSxPAGE) / WORDSxPAGE);
#else
  reserve_pages((size + WORDSxPAGE - 1) / WORDSxPAGE);
#endif
  
  object = freep;
  /* No object is allocated in final page after object > 1 page */
  if  (freeWords != 0)  {
#if HEADER_SIZE
    *freep = MAKE_HEADER(freeWords, freespace_tag);
#else
    *freep = *(GCP)aGcFreeObject;
    SET_OBJECTMAP(freep);
#endif
    freeWords = 0;
  }
  freep = NULL;
  return(object);
}

/*----------------------------------------------------------------------
 * gcobject --
 *
 * Results: 1 if the object is checked by the garbage collector, otherwise 0.
 *______________________________________________________________________*/

int gcobject(void *obj)
{
  extern int end;
  if (obj >= (void *)(&end)  &&
#ifdef STACK_GROWS_DOWNWARD
      obj < (void *)(&obj)
#else
      obj > (void *)(&obj)
#endif
      ) {
    int page = GCP_to_PAGE(obj);
    if (OUTSIDE_HEAP(page))
      return 0;
  }
  return 1;
}

/***********************************************************************
  new
  ***********************************************************************/

/*----------------------------------------------------------------------
 * GcObject::operator new --
 *
 * The creation of a new GC object requires:
 *	- to mark its address in table ObjectMap
 *	- to record its size in the header
 *______________________________________________________________________*/

void* GcObject::operator new(size_t size, CmmHeap *heap)
{
  GCP object = heap->alloc(size) + HEADER_SIZE;
  
  // To avoid problems in GC after new but during constructor
  // for (int i = 0; i < sizeof(GcObject)/sizeof(int); i++)
  //   object[i] = ((GCP)aGcObject)[i];
  //  actually just one element:
  *object = *((GCP)aGcObject);
  
#if HEADER_SIZE
  object[-HEADER_SIZE] = MAKE_HEADER(BYTEStoWORDS(size), MAKE_TAG(2));
#endif
  SET_OBJECTMAP(object);
  return (void *)object;
}

void GcObject::operator delete(void *obj)
{
  (((GcObject *)obj)->heap())->reclaim((GCP)obj);
}

/*----------------------------------------------------------------------
 * GcVarObject::operator new --
 *______________________________________________________________________*/

void* GcVarObject::operator new(size_t size, size_t ExtraSize, CmmHeap *heap)
{
  size += ExtraSize;
  
  GCP object = heap->alloc(size) + HEADER_SIZE;
  
  // To avoid problems in GC after new but during constructor
  // for (int i = 0; i < sizeof(GcVarObject)/sizeof(int); i++)
  //    object[i] = ((GCP)aGcVarObject)[i];
  //  actually just one element:
  *object = *((GCP)aGcVarObject);
  
#if HEADER_SIZE
  object[-HEADER_SIZE] = MAKE_HEADER(BYTEStoWORDS(size), MAKE_TAG(2));
#endif
  SET_OBJECTMAP(object);
  return (void *)object;
}

/***********************************************************************
  Verification
  ***********************************************************************/

/*----------------------------------------------------------------------
 * next_object --
 *
 * A pointer pointing to the header of an object is stepped to the next
 * header.  Forwarded headers are correctly handled.
 *
 * Results: address of immediately consecutive object
 *______________________________________________________________________*/

static GCP  next_object(GCP xp)
{
#if HEADER_SIZE
  if  (FORWARDED(*xp))
    return  xp + HEADER_WORDS(*((int*)(*xp) - HEADER_SIZE));
  else
    return  xp + HEADER_WORDS(*xp);
#else
  return  xp+ ((GcObject *)xp)->words();
#endif
}

/*----------------------------------------------------------------------
 * verify_object --
 *
 * Verifies that a pointer points to an object in the heap.
 * An invalid pointer will be logged and the program will abort.
 *______________________________________________________________________*/

static void  verify_object(GCP cp, int old)
{
  int  page = GCP_to_PAGE(cp);
  GCP  xp = PAGE_to_GCP(page); /* Ptr to start of page */
  int  error = 0;
  
  if  (page < firstHeapPage)  goto fail;
  error = 1;
  if  (page > lastHeapPage)  goto fail;
  error = 2;
  if  (pageSpace[page] == UNALLOCATEDSPACE)  goto fail;
  error = 3;
  if  (old  &&  UNSTABLE(page)  &&  pageSpace[page] != current_space)
    goto fail;
  error = 4;
  if  (old == 0  &&  pageSpace[page] != next_space)  goto fail; 
  error = 5;
  while  (cp > xp + HEADER_SIZE)  xp = next_object(xp);
  if  (cp == xp + HEADER_SIZE)  return;
 fail:
  fprintf(stderr,
	  "\n***** gcalloc  invalid pointer  error: %d  pointer: 0x%x\n",
	  error, cp);
  abort();
}

/*----------------------------------------------------------------------
 * verify_header --
 *
 * Verifies an object's header.
 * An invalid header will be logged and the program will abort.
 *______________________________________________________________________*/

#ifdef DOUBLE_ALIGN
#define HEADER_PAGES(header) ((HEADER_WORDS(header)+WORDSxPAGE)/WORDSxPAGE)
#else
#define HEADER_PAGES(header) ((HEADER_WORDS(header)+WORDSxPAGE-1)/WORDSxPAGE)
#endif

static void  verify_header(GCP cp)
{
#if HEADER_SIZE
  int  size = HEADER_WORDS(cp[-HEADER_SIZE]),
#else
  int  size = ((GcObject *)cp)->words(),
#endif
  page = GCP_to_PAGE(cp),
  error = 0;
  
  if  FORWARDED(cp[-HEADER_SIZE])  goto fail;
  error = 1;
#if HEADER_SIZE
  if  ((unsigned)HEADER_TAG(cp[-HEADER_SIZE]) > 2)  goto fail;
#endif
  if  (size <= ONEPAGEOBJ_WORDS)  {
    error = 2;
    if  (cp - HEADER_SIZE + size > PAGE_to_GCP(page + 1))  goto fail;
  } else  {
    error = 3;
#if HEADER_SIZE
    int  pages = HEADER_PAGES(cp[-HEADER_SIZE]);
#else
    int pages = pageGroup[page];
    if (pages < 0) pages = pageGroup[page+pages];
#endif
    int pagex = page;
    while  (--pages)  {
      pagex++;
      if  (pagex > lastHeapPage  ||
	   pageGroup[pagex] > 0  ||
	   pageSpace[pagex] != pageSpace[page])
	goto fail;
    }
  }
  return;
 fail:	fprintf(stderr,
		"\n***** gcalloc  invalid header  error: %d  object&: 0x%x  header: 0x%x\n",
		error, cp, cp[-HEADER_SIZE]);
  abort();
}

/***********************************************************************
  Logging and Statistics
  ***********************************************************************/

/*----------------------------------------------------------------------
 * log_root --
 *
 * Logs a root to stderr.
 *______________________________________________________________________*/

static void  log_root(unsigned* fp)
{
  int  page = GCP_to_PAGE(fp);
  if  (page < firstHeapPage  ||  page > lastHeapPage  ||
       pageSpace[page] == UNALLOCATEDSPACE  ||
       (UNSTABLE(page)  &&  pageSpace[page] != current_space))
    return;
  int pages = pageGroup[page];
  if (pages < 0) page += pages;
  GCP  p1, p2 = PAGE_to_GCP(page);
  while  (p2 < (GCP)fp)  {
    p1 = p2;
    p2 = next_object(p2);
  }
  fprintf(stderr, "***** DefaultHeap::alloc  root&: 0x%x  object&: 0x%x  %s\n",
	  fp, p1,
#if HEADER_SIZE
	  HEADER_TAG(*p1)
#else
	  *p1
#endif
	  );
}

/* Output a newline to stderr if logging is enabled. */

void  newline_if_logging()
{
  WHEN_FLAGS ((GCDEBUGLOG | GCROOTLOG | GCHEAPLOG),
	      fprintf(stderr, "\n"));
}
