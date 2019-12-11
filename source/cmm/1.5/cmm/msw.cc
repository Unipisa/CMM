/*****************************************************************************
 *
 *  msw.cc: memory manager with mark&sweep garbage collection.
 *
 *  version:    0.0.1 (27 Aug 95)
 *  authors:	Pietro Iglio
 *  email:	cmm@di.unipi.it, iglio@posso.dm.unipi.it
 *  address:	Dipartimento di Informatica
 *		Corso Italia 40
 *		I-56125 Pisa, Italy
 *
 *  Copyright (C) 1995 Pietro Iglio
 *
 *  This file is part of the PoSSo Customizable Memory Manager (CMM).
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
 ****************************************************************************/

/* Following macros can be defined at compile time to get debug info and
 * heap checking:
 *
 *	- MSW_DEBUG
 *	- MSW_CHECK_HEAP: perform automatic heap checking before/after every
 *                        collection and tempHeap destruction.
 *	- MSW_SERIAL_DEBUG
 * 	- MSW_TRACE_ONE_PAGE
 *      - MSW_DONT_CLEAN_FREE_MEM: when MSW_DEBUG is on avoid clearing every
 *	                           empty/released memory object with a special
 *				   tag.
 *
 * Other macros that can be defined at compile time:
 *
 *	- MSW_ALLOC_ZERO_OK: enables allocation of 0-sized memory objects.
 *	- MSW_GET_ALLOC_SIZE_STATS: counts blocks requests for each size.
 *	- MSW_SHOW_TIMINGS: after each collection shows the time required
 */

/* DEFINITIONS:
 *****************************************************************************
 * FreeChunks:
 *   A chunk is a set of one or more consecutives pages.
 *   A chunk class is the set of chunks of the same size.
 *   Free chunk classes are keeped in a list sorted by size. Each chunk class
 *   is keeped in a list too.
 *   Eg.   size(2) -> size(8) -> size(20)
 *	for size(2):
 *	  chunk1 -> chunk2 -> ...
 *
 *   header->nextPage points to the next chunk is the same class
 *   header->nextChunk points to the next chunk class.
 *
 *****************************************************************************/

#include "machine.h"
#include "cmm.h"
#include "msw.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <setjmp.h>
#include <unistd.h>

/******************************************************************************
 * :: Debug Stuff
 *****************************************************************************/

unsigned long	markedBytes = 0;
unsigned long	sweptBytes  = 0;   

unsigned 	totFreeFPages = 0;
unsigned long	totFreeFixedMem = 0;

unsigned long   allocSerial   = 0;
unsigned long	freeSerial    = 0;
unsigned long   collectSerial = 0;

FILE * gcOut	= stderr;

#ifdef NDEBUG
     int mswDebug = 0;
#    define mswDEBUG(STMT)
#    define mswCleanMemDEBUG(STMT)	
#    define mswSerialDEBUG(STMT)
#    define mswGcDEBUG(STMT)
#    define mswCheckDEBUG(STMT)
#    define mswCondCheckDEBUG(STMT)
#    define mswTracePageDEBUG(page)
#    define mswGetStatsDEBUG(STMT)
#else
     int mswDebug = 1;
     int mswGcDebug = 0;
     int mswCheckDebug = 1;
#    define mswDEBUG(STMT)	STMT

#    if defined(MSW_DONT_CLEAN_MEM)
#	define mswCleanMemDEBUG(STMT)	
#    else
#	define mswCleanMemDEBUG(STMT)	STMT
#    endif

     /* NOTE: `allocSerial', etc. can be confused with a root  */
#    if defined(MSW_SERIAL_DEBUG)
        static unsigned long	allocSerialBreak = 0;
#    	define mswSerialDEBUG(STMT) 	STMT
#    else
#	define mswSerialDEBUG(STMT)
#    endif


#    define mswGcDEBUG(STMT)	if (!mswGcDebug) ; else STMT

#    if defined(MSW_CHECK_HEAP)
#    		define mswCheckDEBUG(STMT)   STMT
#    		define mswCondCheckDEBUG(STMT) if (!mswCheckDebug) ; else STMT
#    else
#		define mswCheckDEBUG(STMT)
#		define mswCondCheckDEBUG(STMT)
#    endif

#    if defined(MSW_TRACE_ONE_PAGE)
                void		mswTraceFPage(Ptr page);
                static Ptr	mswTracedPage = NULL;

#    		define mswTracePageDEBUG(pagePtr) \
                            if (pagePtr) mswTraceFPage(pagePtr)
#    else
#		define mswTracePageDEBUG(pagePtr) 
#    endif

#    if defined(MSW_GET_ALLOC_SIZE_STATS)

#	define MAX_TRACED_SIZE			4000
        static unsigned long totAllocatedSizes [MAX_TRACED_SIZE];
        static unsigned long totHugeAllocated = 0;

#	define mswGetStatsDEBUG(size) \
		if ((size) < MAX_TRACED_SIZE) \
	  		totAllocatedSizes[(size)] += 1; \
		else \
                        totHugeAllocated += 1; 
#    else
#    	define mswGetStatsDEBUG(size)
#    endif

#endif

#if defined(__i386)
    extern int etext;
#   define DATASTART ((((unsigned long)(&etext)) + 0xfff) & ~0xfff)
#   define STACKBOTTOM ((Ptr)0xc0000000)
#endif
#if defined(__mips) || defined(__sparc) || defined(__alpha)
#define	ObjAlignment			8
#endif

/******************************************************************************
 *
 * :: Macros
 *
 *****************************************************************************/

/******************************* Constants *******************************/

#define MaxFixedSize			(bytesPerPage >> 1) - 16

#define PageMask		(~(Word)(bytesPerPage - 1))
#define PagesRequest			10

#if !defined(ObjAlignment)
#	define ObjAlignment     	4
#endif

#define PtrAlignment		   	sizeof(Ptr)
#define PtrSize			 	sizeof(Ptr)

/* FirstObjOffset is the offset of the first allocable obj in a page */
#define FirstObjOffset	       (Word) PTR_ALIGN(sizeof(PageHeaderStruct)+1, \
					        ObjAlignment) -1
#define AllocMask                 0x1
#define MarkMask                  0x3
#define FreeMask		  0x0

/* Page type: values for the page map */
#define PAGE_LONG_Free		0L

#define PAGE_Free		0x0
#define PAGE_Fixed		0x1
#define PAGE_Mixed		0x2
#define PAGE_Next		0x3
#define PAGE_Other		0x4

/* Page space: values for the page space */
#define SPACE_Permanent 0
#define SPACE_Temporary 1

#define MSW_TRANSPARENT_OBJ	0
#define MSW_OPAQUE_OBJ		1

#define PageMapInitialSize	bytesPerPage * 4

/* When using debug options a memory object is filled with EMPTY_MEM_TAG value
 * when is allocated and with RELEASED_MEM_TAG when is released during a 
 * collection.
 * If there is an attempt of using a free memory object, it should be easy to
 * understand if the mem obj has been released during a collection.
 */
#define EMPTY_MEM_TAG		0xdd
#define RELEASED_MEM_TAG	0xee


#define PAGE_FROM_PTR(ptr)		(Ptr)((Word)(ptr) & PageMask)
#define PTR_ALIGN(ptr, align)	\
             (Ptr)(((Word)((Ptr)(ptr)-1)+(align)) & ~((align)-1))

#define ROUND_UP(n, b)  (Word) PTR_ALIGN((n),(b))
#define ROUND_DOWN(n, b) (Word) ((Word)(n) & ~((b) -1))

#define IS_INSIDE_HEAP(p)	\
             ((Ptr)(p) < heapEndPtr && (Ptr)(p) >= heapStartPtr)

#define GET_OBJ_BASE(ptr, header) 				      	\
   (Ptr)((Word)header + header->basePointerv[(Word)(ptr)-(Word)header])

#define INDEX_FROM_PTR(p) \
        (Word)(((unsigned long) p) / bytesPerPage)

#define PAGE_FROM_INDEX(i) ((Ptr)((i) * bytesPerPage))

#define PAGE_INFO_FROM_PTR(p)  pageMap[INDEX_FROM_PTR(p)]
#define PAGE_SPACE_FROM_PTR(p) pageSpace[INDEX_FROM_PTR(p)]

/* How many pages needed for "bytes" bytes. */
#define BYTES_TO_PAGES(bytes)  1 + ((bytes) / bytesPerPage)

#define NEXT_FREE_OBJ(p)          *(Ptr *)((Ptr)(p) + 1)
#define SET_NEXT_FREE_OBJ(p, obj) *(Ptr *)((Ptr)(p) + 1) = (obj)

#define isAvailFPage(p)  (((PageHeader)(p))->nextPage != NULL)
#define isFullFPage(p)  	 (((PageHeader)(p))->freeList == NULL)
/* Address of last obj that can be allocated in "page" (fixed) */
#define GET_LAST_OBJ_PTR(pg,size) \
              (Ptr)(pg) + bytesPerPage - ((size) + ObjAlignment)

/* pageLink is a vector defined by CMM. Other heaps use it for other purposes.
 * Here it is renamed to pageMap because this name is more appropriate and
 * helps reading the code.
 */
#define pageMap		pageLink

/******************************************************************************
 *
 * :: Type Definitions
 *
 *****************************************************************************/

typedef unsigned char 	Byte;
typedef unsigned long	Word;
typedef Byte *		Ptr;

/****** PageHeader ******/

typedef struct pageHeaderStruct *	PageHeader;

typedef struct pageHeaderStruct {
	Word 	objSize;	/* Size of objects allocated in this page */
	Ptr *	basePointerv;
	PageHeader nextPage;	/* Next page with some free objects */
	Ptr	freeList;	/* First free obj in this page */
	int	allocatedObjs;
	int	nPages;		/* >0 only for mixed objects */
	char	isMarked;	/* The whole page is marked? */
	char	isOpaque;       /* Only for mixed objs. 1 => don't traverse */

} PageHeaderStruct;


/****** FreePageHeader ******/

typedef struct freePageHeaderStruct *	FreePageHeader;

typedef struct freePageHeaderStruct {
        FreePageHeader	nextChunk;  /* Next page of same chunk size */
	FreePageHeader	nextChunkGroup;
	int		nPages;

} FreePageHeaderStruct;

/****** FPageInfoStruct ******/

struct FPagesInfoStruct {
	Word	firstObjOffset;
	Ptr *	basePointerv;
	int	objectsPerPage;
};

/******************************************************************************
 *
 * :: TempHeap Typedef & Globals
 *
 *****************************************************************************/

#define	MAX_ROOT	30

typedef struct TempHeapInfoStruct *		TempHeapInfo;

struct TempHeapInfoStruct {
	Ptr 		roots [MAX_ROOT];
	int 		nRoots;
	PageHeader	availFPages   [MaxFixedSize];

	TempHeapInfo	previous;
};

TempHeapInfo	tempHeapInfo	= NULL;
int		mustResetTempHeap = 0;
int		noCollection = 0;

#define mswDisableCollection()	(noCollection = 1)
#define mswEnableCollection()	(noCollection = 0)
#define mswIsCollectionDisabled() (noCollection)

/******************************************************************************
 *
 * :: Global Variables
 *
 *****************************************************************************/

struct FPagesInfoStruct	FPagesInfo	[MaxFixedSize];

static  PageHeader	availFPages   [MaxFixedSize];
static  unsigned	totFreePages = 0;

/***** GC "Automatic" or "On Demand"  *****/

static	unsigned	mswGcType = 0;

/***** freeChunks  *****/

static  FreePageHeader 	freeChunks = NULL; 

static	int	totReleasedPages = 0;


/***** Memory Sections *****/

static  Ptr	stackStartPtr;
static	Ptr	stackEndPtr;

static	Ptr	dataStartPtr;
static  Ptr	dataEndPtr;

static  Ptr	heapStartPtr  = (Ptr) -1;
static  Ptr	heapEndPtr = NULL;

extern  int	end;	/* Compiler defined, end of data segment */


/******************************************************************************
 * :: Timings
 *****************************************************************************/

#if defined(MSW_SHOW_TIMINGS)

#	ifdef sgi  /* System V */
#   		include <sys/types.h>
#   		include <sys/times.h>
#	else
#   		include <sys/time.h>
#   		include <sys/resource.h>
#	endif

#	define mswShowTIME(STMT)	STMT
        static Word  mswGetCpuTime  (void);
        static Word  totMarkTime  = 0;
        static Word  totSweepTime = 0;
#else
#	define mswShowTIME(STMT)	
#endif

/******************************************************************************
 *
 * :: Local Prototypes
 *
 *****************************************************************************/

static PageHeader     mswAllocFPage	     (Word size);
static Ptr  	      mswAllocChunk	     (unsigned long, unsigned);
static Ptr  	      mswAllocPages	     (int nPages, Word size);

static Ptr            mswReservePages        (int nPages, Word size);
static PageHeader     mswSetupFPage      (Ptr page, Word size);

static void	      mswFreeChunk	       (PageHeader);

static void	      mswAddToFreeChunksAndMap (Ptr chunk, int nPages);
static void	      mswAddToFreeChunks       (Ptr chunk, int nPages);
static Ptr	      mswGetBestFitChunk       (int nPages);
static void	      mswRemoveFromFreeChunks  (FreePageHeader chunk);

static void	      mswPageMapSetRange       (Ptr, Ptr, unsigned);
static void	      mswPageMapSet	       (Ptr, int, unsigned);
static void	      mswPageSpaceSetRange     (Ptr, Ptr);
static void	      mswPageSpaceSetRangeTo   (Ptr, Ptr, short);
static void	      mswPageSpaceSet	       (Ptr, int);

static void	      mswCheckFreeChunks       (void);

/******************************************************************************
 *
 * :: Function Definitions
 *
 *****************************************************************************/

/******************************************************************************
 * :: mswAlloc
 *****************************************************************************/

void *
mswAlloc(unsigned long size)
{
        Ptr    	   freeList;
        PageHeader freePage;
	Ptr	   ret;

	mswSerialDEBUG({
 	    if (allocSerial++ == allocSerialBreak)
		allocSerialBreak = allocSerial; /*** PLACE A BREAKP. HERE ***/
	});

	mswTracePageDEBUG(mswTracedPage);
	
#	if defined(MSW_ALLOC_ZERO_OK)
	    if (size == 0) size = 4;
#	endif

	assert(size);

	size = ROUND_UP(size, PtrSize);

	mswGetStatsDEBUG(size);

	if (size >= MaxFixedSize)
		return mswAllocChunk(size, MSW_TRANSPARENT_OBJ);

	/**** allocate an object from pool(size) ****/

        freePage = availFPages[size];

	assert(freePage != NULL);
	assert(freePage->objSize == size);

	freeList = freePage->freeList;

	freePage->allocatedObjs += 1;
	*freeList = AllocMask;

	// Next assignment is needed because subsequent call to 
	// mswAllocFPage might release completely "freePage". This way we
	// ensure that there is at least an object alive.
	ret = freeList + 1;

	freePage->freeList = NEXT_FREE_OBJ(freeList);

	if (freePage->freeList == NULL)
		if ((availFPages[size] = freePage->nextPage) == NULL)
			mswAllocFPage(size);

	assert(freeList);
	assert(freePage->freeList == NULL ||
	       ((Ptr) freePage->freeList > (Ptr) freePage && 
		(Ptr) freePage->freeList < (Ptr) freePage + bytesPerPage));


	mswTracePageDEBUG(mswTracedPage);

	return ret;
}

/******************************************************************************
 * :: mswAllocOpaque
 *****************************************************************************/

/* Allocates an object that is guaranteed to do not contain pointers.
 * Usefull only for big chunks, that are not traversed during the collection.
 */
void *
mswAllocOpaque(unsigned long size)
{
	if (size < MaxFixedSize) 
	       return mswAlloc(size);
	else
	       return mswAllocChunk(size, MSW_OPAQUE_OBJ);
}

/******************************************************************************
 * :: mswAllocFPage
 *****************************************************************************/

/* Return a pointer to a free fixed list for "size".
 * The code looks obscure because mswAllocPages() can cause a collection
 * that can add pages to availFPages[].
 */
static PageHeader
mswAllocFPage(Word size)
{
	Ptr    page;

	page = mswAllocPages(1, size);

	if (availFPages[size] == NULL) {
		assert(page);
		availFPages[size] = mswSetupFPage(page, size);
	}

	return availFPages[size];
}

/******************************************************************************
 * :: mswShouldExpandHeap
 *****************************************************************************/

#define FIXED_MEM_WEIGHT	1
#define GC_FREE_FACTOR		10

/* Called after a GC. Return 1 if the heap should be expanded. Note that an
 * expansion might be request even if we found enough free pages after a GC.
 */ 
static int
mswShouldExpandHeap(void)
{
	Word heapSize = ((Word)lastHeapPage - firstHeapPage) * bytesPerPage;
	Word averageFreeMem = (totFreePages * bytesPerPage);

	mswDEBUG(fprintf(stdout, "mswShouldExpandHeap: "
		         "heapSize:%lu, freeMem:%lu ->",
			 heapSize, averageFreeMem););

	if (averageFreeMem * GC_FREE_FACTOR < heapSize) {
		mswDEBUG(fprintf(stdout, " expand heap\n"););
		return 1;
	}
	else {
		mswDEBUG(fprintf(stdout, " NOT expand heap\n"););
		return 0;
	}
}

/******************************************************************************
 * :: mswReservePages
 *****************************************************************************/

#define HAS_ENOUGH_PAGES(npages) (lastHeapPage - firstFreePage >= (npages) + 1)

static Ptr	mswExpandHeap		(int pagesRequest);

/* Return a chunk of "nPages" consecutives pages, getting them from a
 * garbage collection or from the operating system.
 * "size" is 0 if the pages are mixed.
 * Since more than requested pages can be allocated, the remaining chunk is
 * added to freeChunks.
 * NOTE: the pageMap is updated only for extra chunks.
 * NOTE that when mswReservePages is called no chunk of "nPages" size is
 *   currently available.
 * NOTE: a NULL value is returned only if no memory is available.
 */
static Ptr
mswReservePages(int nPages, Word size)
{
	Ptr            newPages;
	int	       pagesRequest; // = PagesRequest;

//	if (pagesRequest < nPages)
                  pagesRequest = nPages;


	if (mswGcType == MSW_Automatic
	    && ! HAS_ENOUGH_PAGES(pagesRequest)) {

		mswDEBUG(fprintf(gcOut, "Automatic gc call...\n");); 
		mswCollect();

		/* Expand anyway if not found enough free pages */
		if (mswShouldExpandHeap()) {
			newPages = mswExpandHeap(pagesRequest);
		        mswAddToFreeChunksAndMap(newPages, pagesRequest);
		}

		/* Found free objects after collection? */
		if (size != 0 &&
		    availFPages[size]) {
		        mswDEBUG(fprintf(gcOut, "Found enough objects after gc!\n"));
			return NULL;			/* <------------ */
		}

		/* Try if the request can be satisfied after GC */
		newPages = mswGetBestFitChunk(nPages);
		/* Found a free chunk after collection ? */
		if (newPages) {
		         mswDEBUG(fprintf(gcOut, "Found enough pages after gc!\n"));
		         return newPages;		/* <----------- */
		 }
	         mswDEBUG(fprintf(gcOut,
			 "Not found enough pages after gc! Expanding heap\n"));
	}

	return mswExpandHeap(pagesRequest);
}

/******************************************************************************
 * :: mswExpandHeap
 *****************************************************************************/
static Ptr
mswExpandHeap(int nPages)
{
	Ptr	newPages;
	int	pagesRequest = nPages;
	int	extraPages;
	int	lowestFreePage;
	int     lastPage = INDEX_FROM_PTR(heapEndPtr) - 1;
	Ptr	extraChunk;
	Word	bytesRequest;

	if (pageMap[lastPage] == PAGE_Free &&
	    lastPage > firstHeapPage) {
		lowestFreePage = lastPage - 1;
		while (pageMap[lowestFreePage] == PAGE_Free)
		  lowestFreePage -= 1;
		lowestFreePage += 1;
		pagesRequest -= (lastPage - lowestFreePage);
	}

	newPages = (Ptr) allocatePages(pagesRequest, Cmm::theMSHeap);

	bytesRequest = pagesRequest * bytesPerPage;
 
	/* Check that allocatePages() returns an aligned address */
	assert(newPages == PAGE_FROM_PTR(newPages));

	pagesRequest = bytesRequest / bytesPerPage;

 	assert(heapStartPtr <= newPages);

	if (heapEndPtr != newPages) {
	        mswPageMapSetRange(heapEndPtr, newPages - 1, PAGE_Other);
		mswPageSpaceSetRangeTo(heapEndPtr,newPages-1, SPACE_Permanent);
	}

	if (heapEndPtr < newPages + bytesRequest) 
	  heapEndPtr = newPages + bytesRequest;

	mswPageSpaceSet(newPages, nPages);

	if (pagesRequest != nPages) {
		Ptr chunk = PAGE_FROM_INDEX(lowestFreePage);
		mswRemoveFromFreeChunks((FreePageHeader)chunk);
		newPages = chunk;
	}

	return newPages;
}

/******************************************************************************
 * :: mswSetupFPages
 *****************************************************************************/
static PageHeader
mswSetupFPage(Ptr page, Word size)
{
	PageHeader header = (PageHeader) page;
	Ptr        firstObj, lastObj, p;
	Word       size1 = size + ObjAlignment;

	assert(FPagesInfo[size].basePointerv);

	header->objSize       = size;
	header->basePointerv  = FPagesInfo[size].basePointerv;
	header->allocatedObjs = 0;
	header->nextPage      = NULL;
	header->nPages	      = 1;
	header->isMarked      = 0;

	firstObj = page + FPagesInfo[size].firstObjOffset;
	lastObj  = GET_LAST_OBJ_PTR(page, size);

	header->freeList      = firstObj;

	for (p = firstObj; p <= lastObj; p += size1) {
		*p = 0;                     /* mark and alloc info */
		SET_NEXT_FREE_OBJ(p, p + size1);   /* Next free list item */
		mswCleanMemDEBUG({
			Ptr pd;
			for (pd = p + 1 + sizeof(Ptr); pd < p + 1 + size; pd++)
			       *pd = EMPTY_MEM_TAG;
		});
	}

	SET_NEXT_FREE_OBJ(p - size1, NULL);   /* Last item has no tail */

	pageMap[INDEX_FROM_PTR(p)] = PAGE_Fixed;

	return header;
}

/******************************************************************************
 * :: mswRemoveAvailFPage
 *****************************************************************************/
/* NOTE: here a page is always removed because the caller will always free
 * the page.
 */
static void
mswRemoveAvailFPage(PageHeader header)
{
	PageHeader page = availFPages[header->objSize];

	if (page == header) {
		availFPages[header->objSize] = header->nextPage;
		return;  /* <--------- */
	}
	/* Note: page can be NULL if in tempHeapInfo->availFPages[] */
	while (page != NULL) {
		if (page->nextPage == header) break;
		page = page->nextPage;
	} 

	/* NOTE: we put following test here and not at the beginning of the
	 * function because this situation can happen only when there is a
	 * collection with a tempHeap active, so this is a very rare event.
	 */
	if (page == NULL) {
		/* Remove page from tempHeapInfo->availFPages */
		assert(tempHeapInfo);
		assert(pageSpace[INDEX_FROM_PTR(header)] == SPACE_Permanent);
		page = tempHeapInfo->availFPages[header->objSize];

		if (page == header) {
			tempHeapInfo->availFPages[header->objSize] =
			    header->nextPage;
			return;  /* <--------- */
		}

		do {
			if (page->nextPage == header) break;
			page = page->nextPage;
		} while (page != NULL);
	}

	assert(page);
	page->nextPage = header->nextPage;
}

/******************************************************************************
 * :: mswFree
 *****************************************************************************/
void
mswFree(void * p)
{
	PageHeader   header = (PageHeader) PAGE_FROM_PTR(p);
	Word         size   = header->objSize;
	Ptr	     tmp;

	mswSerialDEBUG(freeSerial += 1;);
	
	/* NOp when tempHeap active.
	 * !! Move first 2 assignments after this if
	 */
	if (tempHeapInfo) return;

	if (header->objSize >= MaxFixedSize) {
	        mswFreeChunk(header);
		return;				/* <-------- */
	}

	mswTracePageDEBUG(mswTracedPage);

	assert((Ptr)header != p);
	assert(IS_INSIDE_HEAP(p));
	assert(header->allocatedObjs);
	header->allocatedObjs -= 1;

	/* !! Should call mswFreePage(header) if == 0; but this requires
	 * objs allocated in page-based sublists, otherwise it is not possible
	 * to remove free objs from their lists.
	 */

	if (isFullFPage(header)) {
		/* Add to the list of fixed pages having at least a free
	         * object.
		 */
		header->nextPage = availFPages[size];
	        availFPages[size] = header;
	}
	
	if (header->allocatedObjs == 0 &&
	    isAvailFPage(header)) {
		/* Release this page completely */
		mswRemoveAvailFPage(header);
		mswFreeChunk(header);
	}
	else {
		/* Simply add the released obj to the local freeList */
		*(Ptr *)p = header->freeList;
		(Ptr) p -= 1;
		header->freeList = (Ptr) p;
		*(Ptr)p = 0;   /* Remove allocation mark */
	}

	/* totFreeFixedMem += size; */

	mswCleanMemDEBUG({
		Ptr pd;
		for (pd = (Ptr)p +1+sizeof(Ptr); pd < (Ptr)p +1+size; pd++)
		       *pd = RELEASED_MEM_TAG;
	});
}

/******************************************************************************
 * :: mswGetObjSize
 *****************************************************************************/

/* Given a pointer to an allocated object, return the size of the object.
 * Remember that, for instance, if you allocate a 20 bytes object, 24 bytes
 * are allocated.
 */
unsigned long
mswGetObjSize(void * ptr)
{
	int 	   pageInfo = PAGE_INFO_FROM_PTR(ptr);
	PageHeader page     = (PageHeader) PAGE_FROM_PTR(ptr);

	assert(pageInfo == PAGE_Fixed || pageInfo == PAGE_Mixed);

	return page->objSize;
}
/******************************************************************************
 * :: mswGetRealObjSize
 *****************************************************************************/

/* Given a pointer to an allocated object, return the real size of the object,
 * i.e. the amount of memory allocated for the object. The difference with
 * mswGetObjSize() is only for mixed pages.
 */
static unsigned long
mswGetRealObjSize(void * ptr)
{
	int 	   pageInfo = PAGE_INFO_FROM_PTR(ptr);
	PageHeader page     = (PageHeader) PAGE_FROM_PTR(ptr);

	assert(pageInfo == PAGE_Fixed || pageInfo == PAGE_Mixed);

	if (pageInfo == PAGE_Fixed)
		return page->objSize;
	else
		return (page->nPages * bytesPerPage) - FirstObjOffset -1;
}

/******************************************************************************
 * :: mswResize
 *****************************************************************************/
/* Re-allocate a previously allocated block */
void *
mswRealloc(void * p, unsigned long size)
{
	unsigned long realSize = mswGetRealObjSize(p);
	Ptr	      newPtr;
	PageHeader    page;

	if (realSize >= size) {
		/* Size of mixed objects must be updated because during mark
		 * phase we traverse only header->objSize bytes.
		 */
		if (PAGE_INFO_FROM_PTR(p) == PAGE_Mixed)
			page = (PageHeader) PAGE_FROM_PTR(p);
			page->objSize = size;
		return p;	/* <---------- */
	}
	else {
		newPtr = (Ptr) mswAlloc(size);
		page = (PageHeader) PAGE_FROM_PTR(p);
		memcpy(newPtr, p, realSize);
		mswFree(p);
		return newPtr;
	}
}

/******************************************************************************
 * :: mswCalloc
 *****************************************************************************/
/* Allocates "n" elements of size "size", all initialized to 0. */
void *
mswCalloc(unsigned long n, unsigned long size)
{
	Ptr	mem = (Ptr) mswAlloc(n * size);

	return memset(mem, 0, n * size);
}

/******************************************************************************
 *
 * :: PageMap
 *
 *****************************************************************************/

/* Mark pages in ["from".."to"] with "type".
 * NOTE: "to" is included in range.
 */
static void
mswPageMapSetRange(Ptr from, Ptr to, unsigned type)
{
	Word l;
	Word h;
	register Word k;

	l = INDEX_FROM_PTR(from);
	h = INDEX_FROM_PTR(to);

	for (k = l; k <= h; k++)
	          pageMap[k] = type;
}
/* Mark "nPages" pages starting from "pageStart" with "type" */
static void
mswPageMapSet(Ptr pageStart, int nPages, unsigned type)
{
	int	index = INDEX_FROM_PTR(pageStart);

	while (nPages--) {
		pageMap[index] = type;
		index += 1;
	}
}
/******************************************************************************
 *
 * :: PageSpace
 *
 *****************************************************************************/

static void
mswPageSpaceSetRange(Ptr from, Ptr to)
{
	Word l;
	Word h;
	short type = (tempHeapInfo ? SPACE_Temporary : SPACE_Permanent);
	register Word k;

	l = INDEX_FROM_PTR(from);
	h = INDEX_FROM_PTR(to);

	for (k = l; k <= h; k++)
	          pageSpace[k] = type;
}
static void
mswPageSpaceSetRangeTo(Ptr from, Ptr to, short type)
{
	Word l;
	Word h;
	register Word k;

	assert(type == SPACE_Permanent || type == SPACE_Temporary);

	l = INDEX_FROM_PTR(from);
	h = INDEX_FROM_PTR(to);

	for (k = l; k <= h; k++)
	          pageSpace[k] = type;
}

static void
mswPageSpaceSet(Ptr pageStart, int nPages)
{
	int	index = INDEX_FROM_PTR(pageStart);
	short	type = (tempHeapInfo ? SPACE_Temporary : SPACE_Permanent);

	while (nPages--) {
		pageSpace[index] = type;
		index += 1;
	}
}
static void
mswPageSpaceSetTo(Ptr pageStart, int nPages, short type)
{
	int	index = INDEX_FROM_PTR(pageStart);

	assert(type == SPACE_Permanent || type == SPACE_Temporary);

	while (nPages--) {
		pageSpace[index] = type;
		index += 1;
	}
}
/******************************************************************************
 *
 * :: Free Chunks Management
 *
 *****************************************************************************/

/******************************************************************************
 * :: mswAddToFreeChunks
 *****************************************************************************/
/* Add "chunk" to the list of free chunks. "nPages" is the chunk size.
 * NOTE: does not update pageMap[], to optimize cases in which it is already
 * 	updated. See mswAddToFreeChunksAndMap().
 */
static void
mswAddToFreeChunks(Ptr ptr, int nPages)
{
	FreePageHeader	chunks = freeChunks;
	FreePageHeader *pPrev  = &freeChunks;
	FreePageHeader  chunk  = (FreePageHeader) ptr;
	
	chunk->nPages = nPages;
	totFreePages    += nPages;

	mswCleanMemDEBUG({
		Ptr	p;
		Ptr     endPtr = ptr + (nPages * bytesPerPage);
		for (p = ptr + sizeof(struct freePageHeaderStruct) + 1; 
		     p < endPtr; p++)
		         *p = RELEASED_MEM_TAG;
	});

	if (!chunks) {
		chunk->nextChunk      = NULL;
		chunk->nextChunkGroup = NULL;
		freeChunks = chunk;
		return;				/* <---- */
	}

	while (chunks) {
		if (chunks->nPages == nPages) {
			*pPrev = chunk;
			chunk->nextChunk      = chunks;
			chunk->nextChunkGroup = chunks->nextChunkGroup;
			return;			/* <---- */
		}
		else if (chunks->nPages > nPages) {
			*pPrev = chunk;
			chunk->nextChunk      = NULL;
			chunk->nextChunkGroup = chunks;
			return;			/* <---- */
		}
		pPrev = &(chunks->nextChunkGroup);
		chunks = chunks->nextChunkGroup;
	}
	/* This point is reached iif this is the maximum chunk.
	 * Add the chunk to the tail of the list.
	 */
	*pPrev = chunk;
	chunk->nextChunk      = NULL;
	chunk->nextChunkGroup = NULL;
}

/******************************************************************************
 * :: mswAddToFreeChunksAndMap
 *****************************************************************************/

/* As mswAddToFreeChunks(), but the pageMap[] is updated. */
static void
mswAddToFreeChunksAndMap(Ptr chunk, int nPages)
{
	mswAddToFreeChunks(chunk, nPages);
	mswPageMapSet(chunk, nPages, PAGE_Free);

	mswCleanMemDEBUG({
		Ptr p = (Ptr)chunk + FirstObjOffset + 1;

		for (;p < (Ptr)chunk + nPages * bytesPerPage; p++)
		      *p = EMPTY_MEM_TAG;
	});
}
 
/******************************************************************************
 * :: mswBreakAndAllocChunk
 *****************************************************************************/

/* "chunk" is the block to be allocated;  */
static Ptr
mswBreakAndAllocChunk(FreePageHeader chunk, FreePageHeader * pPrev, int nPages)
{
	int 	       nExtraPages;
	FreePageHeader extraChunk;

	/* Remove the chunk from the chunk list */
	if (chunk->nextChunk == NULL)
		*pPrev = chunk->nextChunkGroup;
	else {
	        *pPrev = chunk->nextChunk;
		chunk->nextChunk->nextChunkGroup = chunk->nextChunkGroup;
	}
	totFreePages -= chunk->nPages;
	if (chunk->nPages > nPages) {
		nExtraPages = chunk->nPages - nPages;
		extraChunk = (FreePageHeader)((Ptr)chunk + 
					      nPages * bytesPerPage);
		mswAddToFreeChunks((Ptr)extraChunk, nExtraPages);
	}
	/* If there are in the coping phase (tempHeap ended by not destroyed)
	 * we must promote temporary pages.
	 */
	if (mustResetTempHeap)
		mswPageSpaceSet((Ptr) chunk, nPages);

	return (Ptr) chunk;
}

/******************************************************************************
 * :: mswGetBestFitChunk
 *****************************************************************************/

/* Walks through the list of free chunks finding the one that best satisfies
 * the request, returning NULL if cannot find a so big chunk.
 * If the chunk is found, it is removed from the free list. Spare pages are
 * reinserted in the free list.
 */
static Ptr
mswGetBestFitChunk(int nPages)
{
       FreePageHeader chunks = freeChunks;
       FreePageHeader *pPrev  = &freeChunks;

       while (chunks) {
	     if (chunks->nPages >= nPages)
	           return mswBreakAndAllocChunk(chunks, pPrev, nPages);
	     pPrev = &(chunks->nextChunkGroup);
	     chunks = chunks->nextChunkGroup;
       }
       return NULL;
} 

/******************************************************************************
 * :: mswRemoveFromFreeChunks
 *****************************************************************************/

static void
mswRemoveFromFreeChunks(FreePageHeader chunk)
{
       FreePageHeader chunks = freeChunks;
       FreePageHeader *pPrev  = &freeChunks;
       int	       nPages = chunk->nPages;

       totFreePages -= nPages;

       while (chunks) {
	     if (chunks->nPages == nPages) break;
	     pPrev = &(chunks->nextChunkGroup);
	     chunks = chunks->nextChunkGroup;
       }

       if (chunks == chunk) {
	       if (chunk->nextChunk) {
	       	    *pPrev = chunk->nextChunk;
		    chunk->nextChunk->nextChunkGroup = chunk->nextChunkGroup;
	       }
	       else
	       	    *pPrev = chunk->nextChunkGroup;
	       return;
       }

       do {
	       assert(chunks);
	       pPrev = &(chunks->nextChunk);
	       chunks = chunks->nextChunk;
       } while (chunks != chunk);

       assert(chunks);
       *pPrev = chunk->nextChunk;
}
/******************************************************************************
 * :: mswMergeChunkWithNeighbors
 *****************************************************************************/

/* Given a chunk that is going to be released, look at its neighbors and merge
 * it into a bigger chunk if possible.
 * Return the pointer to the new chunk and "pNPages" is updated to the new
 * size
 */
static Ptr
mswMergeChunkWithNeighbors(Ptr chunk, int* pNPages)
{
	int nPages    = *pNPages;
	int baseIndex = INDEX_FROM_PTR(chunk);
	int limitIndex= INDEX_FROM_PTR(heapEndPtr);
	int index     = baseIndex;
	Ptr newChunk  = chunk;
	FreePageHeader followChunk;

	while (index > firstHeapPage) {
		if (pageMap[index-1] != PAGE_Free)
			break;

		nPages += 1;
		index  -= 1;
	}

	/* "index" is the index of the first page of the previous free chunk.*/
	if (index != baseIndex) {
		newChunk = PAGE_FROM_INDEX(index);
		mswRemoveFromFreeChunks((FreePageHeader)newChunk);
	}
	
	index = baseIndex + *pNPages;

	/* A free chunk follows the current chunk? */
	if (index < limitIndex &&
	    pageMap[index] == PAGE_Free) {
		followChunk = (FreePageHeader) PAGE_FROM_INDEX(index);
		nPages += followChunk->nPages;
		mswRemoveFromFreeChunks(followChunk);
	}

	*pNPages = nPages;
	return newChunk;
}

/******************************************************************************
 * :: mswAllocChunk
 *****************************************************************************/

/* Alloc a chunk of "size" bytes. A chunk is allocated only if it is bigger
 * than maximum fixed object.
 */
static Ptr
mswAllocChunk(unsigned long size, unsigned type)
{
	int	nPages = (FirstObjOffset + size + bytesPerPage) 
	                    / bytesPerPage;
	int     index;
	Ptr	p;

	p = mswAllocPages(nPages, 0);

	((PageHeader)p)->objSize      = size;
	((PageHeader)p)->basePointerv = NULL;
	((PageHeader)p)->nPages	      = nPages;
	((PageHeader)p)->allocatedObjs= 1;
	((PageHeader)p)->isMarked     = 0;
	((PageHeader)p)->isOpaque     = (type == MSW_OPAQUE_OBJ ? 1 : 0);

	/* Mark pages in pageMap */
	index = INDEX_FROM_PTR(p);
	pageMap[index] = PAGE_Mixed;

	while (--nPages)
	       pageMap[++index] = PAGE_Next;

	return p + FirstObjOffset + 1;
}

/* Alloc "nPages" consecutive pages from freeChunks; if no available chunk
 * can satisfy the request, mswReservePages() is called.
 * "size" is 0 if the page is for mixed objects, else the size of fixed 
 * objects. This value is passed to mswReservepages() to decide if
 * we have enough room after a collection.
 * 
 */
static Ptr
mswAllocPages(int nPages, Word size)
{
	Ptr page = mswGetBestFitChunk(nPages);

	if (page == NULL)
		page = mswReservePages(nPages, size);

	if (tempHeapInfo && page) {
		int pg = INDEX_FROM_PTR(page);
		int lastPg = pg + nPages;
		do 
			pageSpace[pg++] = SPACE_Temporary;
		while (pg != lastPg);
	}

	return page;
}

/******************************************************************************
 * :: mswQuickFreeChunk
 *****************************************************************************/

/* Free a chunk of "totReleasedPages" preceding "p".
 * Used during sweep phase. A global variable is used because of efficiency
 * required.
 * Details: When a fixed page or a group of mixed pages is released during
 * sweep phase we avoid an expensive call to mswFreeChunk(), but we count the
 * number of consecutive pages (totReleasedPages) and as soon as we find a
 * page that is not released we call mswQuickFreeChunk() to release the
 * whole chunk of consecutive pages.
 */
static void
mswQuickFreeChunk(Ptr p)
{
	assert(totReleasedPages != 0);

	p = p - (totReleasedPages * bytesPerPage);
	mswPageMapSet(p, totReleasedPages, PAGE_Free);
	if (tempHeapInfo)
	    mswPageSpaceSetTo(p, totReleasedPages, SPACE_Permanent);

	p = mswMergeChunkWithNeighbors(p, &totReleasedPages);
	
	/* Add to freeChunks */
	mswAddToFreeChunks(p, totReleasedPages);
	totReleasedPages = 0;
}
/******************************************************************************
 * :: mswFreeChunk
 *****************************************************************************/

/* Implementation Note: the page map is marked before the chunk is merged.
 * This because if the chunk will be merged, its neighbor(s) are already
 * marked with PAGE_Free.
 */
static void
mswFreeChunk(PageHeader header)
{
	int 	nPages = header->nPages;

	mswPageMapSet((Ptr)header, nPages, PAGE_Free);
	assert(tempHeapInfo == NULL);
//	if (tempHeapInfo)
//	    mswPageSpaceSetTo((Ptr)header, nPages, SPACE_Permanent);

	header = (PageHeader) mswMergeChunkWithNeighbors((Ptr)header, &nPages);
	
	/* Add to freeChunks */
	mswAddToFreeChunks((Ptr) header, nPages);
}

/******************************************************************************
 *
 * :: Mark and Sweep
 *
 *****************************************************************************/

/******************************************************************************
 * :: mswMark
 *****************************************************************************/

static jmp_buf regs;

static void		mswMarkFromTo	(Ptr from, Ptr to);
extern char **		environ;

static void
mswMark(void)
{
	int	dummy = 0;
	int	i;
	static  void *	mmenv = NULL;


	/* ensure flushing of register caches */
	if (_setjmp(regs) == 0) 
		_longjmp(regs, 1);

	if (!mmenv) 
#		if defined(STACKBOTTOM)
	                 mmenv = STACKBOTTOM-1;
#		else
	                 mmenv = environ;
#		endif

	stackStartPtr = (Ptr) mmenv;
	stackEndPtr = (Ptr) &dummy;

	if (stackStartPtr > stackEndPtr) {
		Ptr tmp = stackStartPtr;
		stackStartPtr = stackEndPtr;
		stackEndPtr  = tmp;
	}

	mswDEBUG({ markedBytes = 0; });

	mswMarkFromTo(stackStartPtr, stackEndPtr);
	mswGcDEBUG({fprintf(gcOut, " (%lu marked from stack)", markedBytes); });
	mswMarkFromTo(dataStartPtr,  dataEndPtr);
}


static void
mswMarkFromTo(Ptr from, Ptr to)
{
	long *			pt;
	Ptr			p;
	Ptr			bp;
	static PageHeader	header;
	static  unsigned	pageInfo;

	assert(from < to);

	for (pt = (long*) from; pt < (long*) to; pt++) {
		p = *(Ptr *)pt;
		if (! IS_INSIDE_HEAP(p)) continue;

		pageInfo = PAGE_INFO_FROM_PTR(p);
		if (pageInfo == PAGE_Fixed) {

			header = (PageHeader) PAGE_FROM_PTR(p);

			bp = GET_OBJ_BASE(p, header);

			/* Is really pointing to a fixed obj? */
			if ((Ptr) header == bp)
			        continue;

			/* Consider only allocated objs not marked. */
			if (*bp != AllocMask) continue;

			/* Mark this object */
			*bp = MarkMask;
			mswDEBUG({ markedBytes += header->objSize; });

			/* Mark this page */
			header->isMarked = 1;

			mswMarkFromTo(bp+1, bp + header->objSize +1);
		}
		else if (pageInfo == PAGE_Mixed) {
			p = (Ptr) ROUND_DOWN(p, bytesPerPage);
		      LAB_PageMixed:
			  /* Here `p' must point to the top of the page */

			if (((PageHeader)p)->isMarked) continue;

			header = (PageHeader) p;
			/* Mark the whole page */
			header->isMarked = 1;
			mswDEBUG({ 
			      markedBytes += header->nPages * bytesPerPage; 
		        });

			/* If opaque, don't traverse it */
			if (((PageHeader)p)->isOpaque) continue;
			
			p += FirstObjOffset + 1;

			mswMarkFromTo(p, p + header->objSize);
		}
		else if (pageInfo == PAGE_Next) {
			int index = INDEX_FROM_PTR(p) - 1;
			while (pageMap[index] != PAGE_Mixed) {
				index -= 1;
				assert(index > 0);
			}
			p = PAGE_FROM_INDEX(index);
			goto LAB_PageMixed;
		}
	}
}

/******************************************************************************
 * :: mswSweep functions
 *****************************************************************************/
static void
mswSweepFPage(Ptr page)
{
	Word	size = ((PageHeader)page)->objSize;
	Ptr	p;
	Word	size1 = size + ObjAlignment;
	int	released = 0;
	Ptr	lo = page + FPagesInfo[size].firstObjOffset;
	Ptr	hi = GET_LAST_OBJ_PTR(page, size);
	Ptr	fixedFreeList = ((PageHeader)page)->freeList;
	int	wasFull = (fixedFreeList == NULL);


	if (((PageHeader)page)->isMarked == 0) {
		if (!isFullFPage(page))
			/* A full page cannot be available */
			mswRemoveAvailFPage((PageHeader)page);
		totReleasedPages += 1;
		mswDEBUG(totFreeFPages += 1;);
		mswDEBUG({ sweptBytes += bytesPerPage; });
		return;			/* <-------- */ 
	}
	else
	        ((PageHeader)page)->isMarked = 0;

        for (p = lo; p <= hi; p += size1) {
		if (*p == (MarkMask | AllocMask)) {
			*p = AllocMask;
			continue;
		}
		/* Found a free obj */
		if (*p == FreeMask) {
			mswDEBUG(totFreeFixedMem += size;);
			continue;
		}

		assert(*p == AllocMask);
		*p = FreeMask;
		*(Ptr *)(p+1) = fixedFreeList;
		mswCleanMemDEBUG({
			Ptr pd;
			for (pd = p + 1 + sizeof(Ptr); 
			     pd < p + 1 + size; pd++)
			          *pd = EMPTY_MEM_TAG;
		});

		fixedFreeList = p;
		released += 1;
		mswDEBUG(totFreeFixedMem += size;);
	}

	/* totFreeFixedMem += (size * released); */

	if (wasFull &&
	    released != 0) {
		if (tempHeapInfo &&
		    PAGE_SPACE_FROM_PTR(page) == SPACE_Permanent) {
			((PageHeader)page)->nextPage =
			  tempHeapInfo->availFPages[size];
			tempHeapInfo->availFPages[size] =
			  (PageHeader)page;
		}
		else {
			((PageHeader)page)->nextPage =
			  availFPages[size];
			availFPages[size] = (PageHeader)page;
		}
	}
	((PageHeader)page)->freeList = fixedFreeList;
	((PageHeader)page)->allocatedObjs -= released;


	if (((PageHeader)page)->allocatedObjs == 0 &&
	    (isAvailFPage(page))) {
		mswRemoveAvailFPage((PageHeader)page);
		totReleasedPages += 1;
	}
	else if (totReleasedPages)
	    mswQuickFreeChunk(page);

	assert(((PageHeader)page)->allocatedObjs >= 0);
	mswDEBUG({ sweptBytes += released * size; });
	mswDEBUG({ if (((PageHeader)page)->allocatedObjs == 0)
	   		totFreeFPages += 1;
	});
}


/******************************************************************************
 * :: mswSweep
 *****************************************************************************/

static void
mswSweep()
{
	Ptr		page;
	unsigned	pageInfo;
	PageHeader	header;
	int		i;

	totReleasedPages = 0;

	for (page = heapStartPtr; page < heapEndPtr; page += bytesPerPage) {
		pageInfo = PAGE_INFO_FROM_PTR(page);
		if (pageInfo == PAGE_Fixed)
			mswSweepFPage(page);
		else if (pageInfo == PAGE_Mixed) {
			header = (PageHeader)page;
			/* Sweep mixed page */
			if (header->isMarked) {
			       header->isMarked = 0;
			       if (totReleasedPages)
				       mswQuickFreeChunk(page);
		        }
			else {
			       mswDEBUG(sweptBytes += 
					  header->nPages * bytesPerPage;);
			       totReleasedPages += header->nPages;
		        }
		        page += (header->nPages - 1) * bytesPerPage;
		}
		else {
			assert(pageInfo == PAGE_Free ||
			       pageInfo == PAGE_Other);
			if (totReleasedPages)
		     	   mswQuickFreeChunk(page);
		}
	}

	if (totReleasedPages)
	        mswQuickFreeChunk(page);

	/* Arrange availFPages[] so that there is no empty list */
	for (i = PtrSize; i < MaxFixedSize; i += PtrSize)
	     if (availFPages[i] == NULL)
	        availFPages[i] = mswAllocFPage(i);
}


/******************************************************************************
 * :: mswMarkAndSweep
 *****************************************************************************/

static void
mswMarkAndSweep(void)
{
	Word	time1, time2, time3;

	mswDEBUG({ 
	           markedBytes = 0; 
	           sweptBytes = 0;
		   totFreeFPages = 0;
        });

	mswShowTIME({ time1 = mswGetCpuTime(); })

	mswMark();	/********* Mark Phase *********/

	mswDEBUG({ 
	         fprintf(stdout, "."); 
		 fflush(stdout);
	});

	mswShowTIME({
		time2 = mswGetCpuTime();
		totMarkTime += (time2 - time1);
		fprintf(gcOut, "[Mark: %lu msec.]", time2 - time1);
	});

	mswSweep();	/********* Sweep Phase *********/

	mswShowTIME({
		time3 = mswGetCpuTime() - time2;
		totSweepTime += time3;
		fprintf(gcOut, "[Sweep: %lu msec.]", time3);
	});
}

/******************************************************************************
 * :: mswCollectNow
 *****************************************************************************/
/* Unconditional GC. */

void 
mswCollectNow()
{
	if (mswIsCollectionDisabled()) return;
	mswDEBUG({ totFreeFixedMem = 0;
		   collectSerial += 1;
		   fprintf(stdout, "#%lu: ", collectSerial);
	});
	mswCondCheckDEBUG(mswCheckHeap(0););

	mswDEBUG({ 
	         fprintf(stdout, "[Garbage collection.."); 
		 fflush(stdout);
	});
        mswMarkAndSweep();
	mswDEBUG({ fprintf(stdout, "done.]\n"); });
	mswDEBUG({
	     fprintf(stdout, "Marked %lu bytes\n", markedBytes);
	     fprintf(stdout, "Swept  %lu bytes\n", sweptBytes);
	     fprintf(stdout, "Tot. %u free pages (%u tot.) (%u from fixed)\n",
		     		totFreePages,
		     	      	(unsigned) (lastHeapPage - firstHeapPage),
		     		totFreeFPages);
	});
	mswCondCheckDEBUG(mswCheckHeap(0););
}

/******************************************************************************
 * :: mswCollect
 *****************************************************************************/

/* Conditional GC. Collect only if (heapSize > gcThreshold) and there is not
 * enough free memory.
 */
void
mswCollect()
{
	Word heapSize = (lastHeapPage - firstHeapPage) * bytesPerPage;
	Word averageFreeMem;

	if (heapSize < Cmm::gcThreshold) {
		mswDEBUG(fprintf(stdout, 
				 "heap size < gc threshold: skipping GC\n"));
		return;  /* <---------- */
	}
#ifdef xxx

	averageFreeMem = (totFreePages * bytesPerPage)
	                 /* + totFreeFixedMem / FIXED_MEM_WEIGHT */ ;

	mswDEBUG(fprintf(stdout, 
		   "heapSize:%lu, averageFreeMem:%lu ",
		   heapSize, averageFreeMem));

	if (averageFreeMem * GC_FREE_FACTOR > heapSize) {
		mswDEBUG(fprintf(stdout, "-> GC request REJECTED.\n"););
		return;  /* <-------- */
	}
#endif

	mswDEBUG(fprintf(stdout, "-> GC request ACCEPTED.\n"););

	mswCollectNow();
}

/******************************************************************************
 *
 * :: MarkAndSweepInit()
 *
 *****************************************************************************/

void
mswInit(unsigned gcType)
{
	extern  void	CmmInit(void);
	int 	i, j, k;
	Word	firstObjOff, bp = 0;
	Word	lastObjOff;
	Word    size1, counter;
	static  int initialized = 0;

	if (initialized) return;	// <---------------
	initialized = 1;

	if (gcType & MSW_Automatic)
		mswGcType = MSW_Automatic;

	if (gcType & MSW_OnDemand) {
		assert(mswGcType == 0);
		mswGcType = MSW_OnDemand;
	}
	assert(mswGcType != 0);

	freeChunks   = NULL;

	dataStartPtr = (Ptr) DATASTART;
	dataEndPtr   = (Ptr) &end;

	stackStartPtr = (Ptr) &i;

	/* This loop fills FPagesInfo[] vector, that is a vector
	 * containing information about each fixed size.
	 * This information will be shared among all the fixed pages for the
	 * same size.
	 */
	for (i = PtrSize; i < MaxFixedSize; i += PtrSize) {
		firstObjOff = (Word) PTR_ALIGN(sizeof(PageHeaderStruct)+1,
					ObjAlignment) -1;
		FPagesInfo[i].firstObjOffset = firstObjOff;
		FPagesInfo[i].basePointerv = (Ptr *)
		    malloc(sizeof(Ptr) * bytesPerPage);

		for (j = 0; j < firstObjOff; j++)
			FPagesInfo[i].basePointerv[j] = 0;

		size1 = i + ObjAlignment;
		counter = 0;
		
		/* Compute the basePointer[] vector for size `i'.
		 * basePointer[] can be used to access, given the page
		 * offset of a pointer, to the base of the object.
		 * NOTE that the base of an object is the word containing
		 * the alloc/mark info, not the real start of the object,
		 * that will be one word after.
		 */
		for (j = firstObjOff;
		     j < (int) GET_LAST_OBJ_PTR(NULL, i) + size1;
		     j++) {

			if (counter == 0) {
				bp = (Word) j;
				counter = size1 - 1;

				/* Faith fake pointers: a pointer will not be
				 * traced if is pointing to an object header.
				 */
				for (k = j - ObjAlignment + 1; k <= j; k++)
				      FPagesInfo[i].basePointerv[k] = NULL;
				continue;
			}
			else	
			        counter -= 1;

			FPagesInfo[i].basePointerv[j] = (Ptr) bp;
	        }
		for (k = j; j < bytesPerPage; j++)
			FPagesInfo[i].basePointerv[k] = NULL;

		lastObjOff= bytesPerPage - size1 - firstObjOff;
		FPagesInfo[i].objectsPerPage = 
		      (lastObjOff - firstObjOff) / size1;
	}

	CmmInit();
	heapStartPtr = PAGE_FROM_INDEX(firstHeapPage);
	heapEndPtr   = heapStartPtr;

	 
	for (i = PtrSize; i < MaxFixedSize; i += PtrSize)
		availFPages[i] = mswAllocFPage(i);

	{
	  char * c = (char *) mswAlloc(4);  /* initialize allocator if not */
	  mswFree(c);
        }

}

/******************************************************************************
 *
 * MarkAndSweep::scanRoots
 *
 *****************************************************************************/
static void	scanFPageRoots(PageHeader);
static void	scanMixedPageRoots(PageHeader);

void
MarkAndSweep::scanRoots(int page)
{
	unsigned pageInfo = PAGE_INFO_FROM_PTR(page);

	// Note: here we ignore PAGE_Next because those pages are
	// traversed when the PAGE_Mixed (i.e. the header) is reached.

	if (pageInfo == PAGE_Fixed)
		scanFPageRoots((PageHeader)PAGE_FROM_INDEX(page));
	else if (pageInfo == PAGE_Mixed)
		scanMixedPageRoots((PageHeader)PAGE_FROM_INDEX(page));
}
static void
scanFPageRoots(PageHeader header)
{
	int 	size = header->objSize;
	Word	size1 = size + ObjAlignment;
	Ptr	obj;
	long*	ptr;
	Ptr	lo = (Ptr)header + FPagesInfo[size].firstObjOffset;
	Ptr	hi = GET_LAST_OBJ_PTR(header, size);

        for (obj = lo; obj <= hi; obj += size1) {
		if (*obj != AllocMask)
			continue;		// <----

		for (ptr = (long*) obj + 1 + sizeof(Ptr); 
		     ptr < (long*) obj + 1 + size; ptr++)
		     promotePage((GCP) ptr);
	}
}
static void
scanMixedPageRoots(PageHeader header)
{
	long*	ptr;
	long*   endPtr;

	if (header->isOpaque) return;	// <----

	endPtr = (long*) FirstObjOffset + 1 + header->objSize;

	for (ptr = (long*) FirstObjOffset + 1; ptr < endPtr; ptr++)
	    promotePage(ptr);
}

/******************************************************************************
 *
 * :: TempHeap
 *
 *****************************************************************************/

/******************************************************************************
 * :: mswRegisterRoot
 *****************************************************************************/
void
mswRegisterRoot(void * p)
{
	if (tempHeapInfo->nRoots == MAX_ROOT) {
		fprintf(stderr, "Too many roots...\n");
		exit(-1);
	}
	tempHeapInfo->roots[tempHeapInfo->nRoots++] = (Ptr) p;
}
/******************************************************************************
 * :: mswTempHeapFreePages
 *****************************************************************************/
/* Releases every page --EXCEPT ones marked PAGE_Other-- from 
 * tempHeapInfo->firstPage up to heapEndPtr. Uses mswQuickFreeChunk().
 * When this function is called:
 * 	- every PAGE_Other and PAGE_Free must be SPACE_Permanent
 *	- every allocated page can be either SPACE_Permanent or SPACE_Temporary
 */
static void
mswTempHeapFreePages(void)
{
	int	page;
	totReleasedPages = 0;

	/* NOTE: the following loop is ugly because optimized */

	for (page = firstHeapPage; page < lastHeapPage; page++) {
		if (pageSpace[page] == SPACE_Permanent) {
			if (totReleasedPages)
			  mswQuickFreeChunk(PAGE_FROM_INDEX(page));
			page += 1;
			while (page < lastHeapPage &&
			       pageSpace[page] == SPACE_Permanent)
			  page += 1;
			if (page != lastHeapPage) {
				totReleasedPages += 1;
				pageSpace[page] = SPACE_Permanent;
			}
		}
		else {
			totReleasedPages += 1;
			pageSpace[page] = SPACE_Permanent;
		}
	}
	if (totReleasedPages)
	  mswQuickFreeChunk(PAGE_FROM_INDEX(page));
}

/******************************************************************************
 * :: mswTempHeapStart
 *****************************************************************************/
/* Freezes availFPages[] and resets it, such that everything is
 * allocated into the new space.
 */
void
mswTempHeapStart()
{
	int	i;
	TempHeapInfo prev = tempHeapInfo;

	tempHeapInfo =
	          (TempHeapInfo) mswAlloc(sizeof(struct TempHeapInfoStruct));

	tempHeapInfo->previous = prev;
	tempHeapInfo->nRoots = 0;

	mswDisableCollection();

	for (i = PtrSize; i < MaxFixedSize; i += PtrSize) {
	        tempHeapInfo->availFPages[i] = availFPages[i];
		availFPages[i] = NULL;
		availFPages[i] = mswAllocFPage(i);
	}
	mswEnableCollection();
}
/******************************************************************************
 * :: mswTempHeapEnd
 *****************************************************************************/
void
mswTempHeapEnd()
{
	TempHeapInfo	tmp;
	int		i;

	assert(tempHeapInfo);
	mswCondCheckDEBUG(mswCheckHeap(0));

	mswDisableCollection();
	for (i = PtrSize; i < MaxFixedSize; i += PtrSize)
	        availFPages[i] = tempHeapInfo->availFPages[i];

	tmp = tempHeapInfo;
	tempHeapInfo = tempHeapInfo->previous;
	mswFree(tmp);
	mustResetTempHeap = 1;

	for (i = PtrSize; i < MaxFixedSize; i += PtrSize) {
		if (availFPages[i] == NULL)
		  availFPages[i] = mswAllocFPage(i);
	}
}

void
mswTempHeapFree()
{
	mswTempHeapFreePages();
	mustResetTempHeap = 0;
	mswEnableCollection();
	mswCondCheckDEBUG(mswCheckHeap(0));
}

/******************************************************************************
 *
 * :: Timings
 *
 *****************************************************************************/

#ifdef MSW_SHOW_TIMINGS

static Word
mswGetCpuTime(void)
{
#   ifdef sgi  /* System V */
	struct tms timebuf;
	Word r;

	times(&timebuf);
  	r = timebuf.tms_utime * 100;
  	return r/6;
#   else
	struct rusage rusage;
	getrusage (RUSAGE_SELF, &rusage);
	return (rusage.ru_utime.tv_sec*1000 + rusage.ru_utime.tv_usec/1000
              + rusage.ru_stime.tv_sec*1000 + rusage.ru_stime.tv_usec/1000);
#   endif
};

#endif /* MSW_SHOW_TIMINGS */

/******************************************************************************
 * :: mswShowInfo
 *****************************************************************************/
void
mswShowInfo(void)
{
	fprintf(gcOut, 
		"-------------------- MSW Collector ------------------\n\n");
	if (mswDebug)
	       fprintf(gcOut, "+++ Debug version +++\n");
	fprintf(gcOut, "Alignment = %d\n", ObjAlignment);
	fprintf(gcOut, "Max heap size = %lu Kbytes\n", 
	       (unsigned long) (heapEndPtr - heapStartPtr) / 1024);
	fprintf(gcOut, "Page size = %lu\n", bytesPerPage);

	mswShowTIME({
		fprintf(gcOut, "Tot. Mark Time = %lu msec.\n", totMarkTime);
		fprintf(gcOut, "Tot. Sweep Time = %lu msec.\n", totSweepTime);
	});
}


/******************************************************************************
 *
 * :: Debug
 *
 *****************************************************************************/

#ifndef NDEBUG

void
mswShowMem(void)
{
	int base = INDEX_FROM_PTR(heapStartPtr);
	int top  = INDEX_FROM_PTR(heapEndPtr);
	int i;
	int j = 0;
	char c;
	const int pagesPerLine = 72;
	

	fprintf(gcOut, "+++++ (. = Free, # = Fixed, B/b = BigObj, O = Other) ++++");

	for (i = base; i < top; i++) {
		if (j == 0) {
			fprintf(gcOut, "\n%05d: ", i);
			j = pagesPerLine;
		}
		j -= 1;

		switch (pageMap[i]) {
		      case PAGE_Free: c = '.'; break;
		      case PAGE_Fixed: c = '#'; break;
		      case PAGE_Mixed: c = 'B'; break;
		      case PAGE_Next: c = 'b'; break;
		      case PAGE_Other: c = 'O'; break;
		      default:
			assert(-1);
		}
		fputc(c, gcOut);
	}
	fprintf(gcOut, "\n");
}
void
mswShowTempMem(void)
{
	int base = INDEX_FROM_PTR(heapStartPtr);
	int top  = INDEX_FROM_PTR(heapEndPtr);
	int i;
	int j = 0;
	char c;
	const int pagesPerLine = 72;
	

	fprintf(gcOut, "+++++ (. = Free, # = Fixed, B/b = BigObj, O = Other) ++++");

	for (i = base; i < top; i++) {
		if (j == 0) {
			fprintf(gcOut, "\n%05d: ", i);
			j = pagesPerLine;
		}
		j -= 1;

		if (pageSpace[i] == SPACE_Temporary)
		  c = 'T';
		else
		  c = '_';

		fputc(c, gcOut);
	}
	fprintf(gcOut, "\n");
}

void
mswShowFreeChunks(void)
{
	FreePageHeader	l = freeChunks;
	FreePageHeader	l1;

	fprintf(gcOut, "+++ Free chunks: \n");
	while (l) {
		fprintf(gcOut, "> %d pages chunks: ", l->nPages);
		l1 = l;
		while (l1) {
			fprintf(gcOut, "[%x]", (unsigned) l1);
			l1 = l1->nextChunk;
		}
		fprintf(gcOut, "\n");
		l = l->nextChunkGroup;
	}
	fprintf(gcOut, "--- free chunks end\n");
}

/* Used by mswShowFragmentation(). Returns the max number of fixed objs of
 * size "size" that can fit in one page.
 */
static int
maxObjsInOnePage(int size)
{
	return (bytesPerPage - sizeof(struct PageHeaderStruct)) 
	  / (size + ObjAlignment);
	             
}
/* Calculates fragmentation for each fixed size. */
void
mswShowFragmentation()
{
	double 	realUse;
	double 	totRealUse = 0.0;
	int	totAnalized = 0;
	int 	i, totObjs, totPages, minPages;
	PageHeader pages;

	for (i = PtrSize; i < MaxFixedSize; i += PtrSize) {
		pages = availFPages[i];

		/* Show only significant data */
		if (pages == NULL ||
		    pages->nextPage == NULL) {
			totRealUse += 100.0;
			totAnalized += 1;
			continue;    /* <------------- */
		}

		totObjs = 0;
		totPages = 0;

		while (pages) {
			totObjs += pages->allocatedObjs;
			totPages += 1;
			pages = pages->nextPage;
		}
		minPages = 1 + (totObjs / maxObjsInOnePage(i));
		realUse = (100.0 * (double) minPages) / (double) totPages;
		totRealUse +=  realUse;
		totAnalized += 1;
		fprintf(gcOut, "Pages[%d] used at %2.4g%% ", i, realUse);
		fprintf(gcOut, "(%d objs in %d pages, %d min pages)\n",
			totObjs, totPages, minPages);
	}
	if (totAnalized)
	  fprintf(gcOut, "\nTot: fixed pages used at %2.4g%%.\n", 
		  totRealUse / (double) totAnalized);
	else 
	  fprintf(gcOut, "No fragmentation for fixed objs.\n");
}
/******************************************************************************
 * :: Heap Checking Functions
 *****************************************************************************/
/* Returns 0 if "page" is not in "pages", else 1 */
static int
mswIsInAvailFPages(PageHeader page, PageHeader pages)
{
      while (pages) {
	      if (pages == page) return 1;
	      pages = pages->nextPage;
      }
      return 0;
}

/* Check that "page" looks like a fixed page */
static void
mswCheckPageIsFixed(Ptr page)
{
      PageHeader header = (PageHeader) page;
      PageHeader pg;
      Word	 size  = header->objSize;

      assert(size > 0 && size < MaxFixedSize);
      assert(size = ROUND_UP(size, PtrAlignment));
      assert(header->basePointerv);
      assert(header->allocatedObjs <= bytesPerPage / (size+1));
      assert(header->freeList == NULL ||
	     (header->freeList > (Ptr) header &&
	      header->freeList < (Ptr) header + bytesPerPage));

      if (header->freeList) {
	      /* If there is at least a free object, check that this page
	       * is among availables pages.
	       */
	      pg = availFPages[size];
	      if (mswIsInAvailFPages(header, pg) == 0) {
		      if (tempHeapInfo == NULL) {
			      assert("found a fixed page not full but not"
				     "in availFPages[]" == NULL);
		      }
		      pg = tempHeapInfo->availFPages[size];
		      if (mswIsInAvailFPages(header, pg) == 0) {
			      assert("found a fixed page not full but not"
				     "in availFPages[]" == NULL);
		      }
	      }
      }
      else {
	      /* No free objects => check that this page
	       * is NOT among availables pages.
	       */
	      pg = availFPages[size];
	      while (pg) {
		      if (pg == header) break;
		      pg = pg->nextPage;
	      }
	      assert(pg == NULL);
      }
}


/* Trace the "freeList" for objs of size "size" checking that every obj
 * is not marked, not allocated, that it is filled with EMPTY_MEM_TAG or
 * RELEASED_MEM_TAG and that the corresponding page is PAGE_Free of PAGE_Fixed.
 */
static void
mswCheckFreeFixedList(PageHeader page, int size)
{
      Ptr 	p;
      Ptr	start;
      Ptr	freeList = page->freeList;
      int	pageInfo = PAGE_INFO_FROM_PTR(page);

      if (freeList == NULL) return;

      assert(pageInfo == PAGE_Fixed || pageInfo == PAGE_Free);

      while (freeList) {
	    /* Check that every obj is inside page boundaries */
	    assert(freeList > (Ptr)page + sizeof(struct PageHeaderStruct));
	    assert(freeList < (Ptr)page + bytesPerPage);
	    /* Check that obj is marked Free */
	    assert(*((Ptr)freeList) == FreeMask);

	    start = (Ptr)freeList + 1 + sizeof(Ptr);

	    mswCleanMemDEBUG({
		    for (p = start; p < freeList + 1 + size; p++)
		      assert(*p == EMPTY_MEM_TAG ||
			     *p == RELEASED_MEM_TAG);
	    });

	    freeList = NEXT_FREE_OBJ(freeList);
      }
}
static void
mswCheckFreeFixedObjs(void)
{
      int	i;
      PageHeader page;

      for (i = PtrSize; i < MaxFixedSize; i += PtrSize) {
	      page = availFPages[i];
	      while (page) {
		      if (tempHeapInfo)
			assert(PAGE_SPACE_FROM_PTR(page) == SPACE_Temporary);
		      else
			assert(PAGE_SPACE_FROM_PTR(page) == SPACE_Permanent);
		      assert(page->objSize == i);
		      assert(page->isMarked == 0);
		      assert(page->freeList);
		      mswCheckFreeFixedList(page, i);
		      page = page->nextPage;
	      }
      }
}


static void
mswCheckFPage(Ptr page)
{
      PageHeader header = (PageHeader) page;
      Ptr	 p;
      int	 size  = header->objSize;
      int 	 size1 = size + ObjAlignment;
      Ptr	 hi = GET_LAST_OBJ_PTR(page, size);
      int	 firstObjOff;
      int	 allocatedObjs = 0;

      mswCheckPageIsFixed(page);
      firstObjOff = FPagesInfo[size].firstObjOffset;

      for (p = page + firstObjOff; p <= hi; p += size1) {
	      assert(*p == AllocMask || *p == FreeMask);
	      if (*p == AllocMask) 
		      allocatedObjs += 1;
      }
      assert(allocatedObjs == header->allocatedObjs);
}

static void
mswCheckMixedPage(PageHeader header)
{
	int	nPages = header->nPages;
	int	index = INDEX_FROM_PTR(header); 
	int	i;

	for (i = index + 1; i < index + nPages; i++)
		assert(pageMap[i] == PAGE_Next);

	assert(header->basePointerv == NULL);
	assert(header->allocatedObjs == 1);
	assert(header->nPages = (FirstObjOffset + header->objSize
				 + bytesPerPage) / bytesPerPage);
}
static void
mswCheckFixedAndMixedPages(void)
{
      Ptr page;
      int pageInfo;

      for (page = heapStartPtr; page < heapEndPtr; page += bytesPerPage) {
	     pageInfo = PAGE_INFO_FROM_PTR(page);
	     if (pageInfo == PAGE_Fixed)
	     	mswCheckFPage(page);
	     else if (pageInfo == PAGE_Mixed) {
		     mswCheckMixedPage((PageHeader)page);
		     page += (((PageHeader)page)->nPages -1)
				* bytesPerPage;

	     }
	     else
	     	assert(pageInfo == PAGE_Other || pageInfo == PAGE_Free);
      }

}

static void
mswCheckPageIsInFreeChunks(Ptr page)
{
	FreePageHeader l = freeChunks;
	FreePageHeader l1;

	while (l) {
		l1 = l;
		while (l1) {
			if ((Ptr) l1 <= page &&
			    page < (Ptr)l1 + l1->nPages * bytesPerPage)
			      return;	  /* <------- */
			l1 = l1->nextChunk;
		}
		l = l->nextChunkGroup;
	}

	assert(l);  /* Cause assertion fault */
}

static void
mswCheckPageMap(void)
{
	Ptr	page;
	int	pageInfo;

	assert(heapStartPtr < heapEndPtr);

	for (page = heapStartPtr; page < heapEndPtr; page+= bytesPerPage) {
		pageInfo = PAGE_INFO_FROM_PTR(page);
		assert(pageInfo == PAGE_Free || pageInfo == PAGE_Fixed ||
		    pageInfo == PAGE_Mixed || pageInfo == PAGE_Other ||
		    pageInfo == PAGE_Next);

		if (pageInfo == PAGE_Free) {
		     mswCheckPageIsInFreeChunks(page);
		     assert(PAGE_SPACE_FROM_PTR(page) == SPACE_Permanent);
	        }
		else if (pageInfo == PAGE_Fixed)
		     mswCheckPageIsFixed(page);
	}
}
/* !! TODO: should check that free space contains RELEASED_MEM_TAG or 
 * EMPTY_MEM_TAG
 */
static void
mswCheckFreeChunks()
{
	FreePageHeader	l = freeChunks;
	FreePageHeader	l1;
	int		i, nPages, nGroupPages;
	int		numFreePgs = 0;
	Ptr		p;

	while (l) {
		assert(IS_INSIDE_HEAP(l));
		l1 = l;
		if (l1)
			nGroupPages = l1->nPages;
		while (l1) {
			assert(IS_INSIDE_HEAP(l1));
			/* Check that is a page address */
			assert((Word)l1 == ROUND_DOWN(l1, bytesPerPage));
			assert(l1->nPages > 0 &&
			       l1->nPages < 20000);
			assert(l1->nPages == nGroupPages);

			numFreePgs += nGroupPages;
			/* Check pages are marked with PAGE_Free */
			i = INDEX_FROM_PTR(l1);
			nPages = nGroupPages;
			while (nPages--)
				assert(pageMap[i++] == PAGE_Free);


			mswCleanMemDEBUG({
			  for (p = (Ptr)l1+sizeof(struct freePageHeaderStruct)+1; p < (Ptr)l1 + (l1->nPages * bytesPerPage); p++)
			    assert(*p == RELEASED_MEM_TAG ||
				   *p == EMPTY_MEM_TAG);
		        });
		
			l1 = l1->nextChunk;
		}
		l = l->nextChunkGroup;
	}
	assert(totFreePages == numFreePgs);
}
	       

#endif /* ! NDEBUG */

void
mswCheckHeap(int verbose)
{
#   if !defined(NDEBUG)

	if (verbose)
		fprintf(gcOut, "\n------------- Checking heap corruption ---------------\n");
	if (verbose) fprintf(gcOut, "- checking free chunks...\n");
	mswCheckFreeChunks();
	if (verbose) fprintf(gcOut, "- checking page map...\n");
	mswCheckPageMap();
	if (verbose) fprintf(gcOut, "- checking free fixed objects...\n");
	mswCheckFreeFixedObjs();
	if (verbose) fprintf(gcOut, "- checking fixed and mixed pages...\n");
	mswCheckFixedAndMixedPages();
	if (verbose) fprintf(gcOut, "-------------------- Heap is OK ------------------------\n\n");
#   endif
}

#ifndef NDEBUG
/******************************************************************************
 * :: Functions useful during debugging sessions
 *****************************************************************************/

/******************************************************************************
 * :: getPtrInfo
 *****************************************************************************/
/* Given a pointer, shows some useful information: page to which belong, size
 * of the pointed object, etc.
 * To be used when a crash happen to get information about the pointer causing
 * it.
 */
void
getPtrInfo(Ptr p)
{
	PageHeader page;
	Ptr	   base;
	char *	   temporary = "";

	fprintf(gcOut, "---- getPtrInfo(%lx): ----\n", p);

	if (! IS_INSIDE_HEAP(p)) {
		fprintf(gcOut, "Hum... this pointer is outside MSW heap...\n");
		return;
	}
	page = (PageHeader) PAGE_FROM_PTR(p);

	switch (PAGE_INFO_FROM_PTR(p)) {
	      case PAGE_Fixed:
		if (pageSpace[INDEX_FROM_PTR(p)] == SPACE_Temporary)
		   temporary = " *temporary*";

		fprintf(gcOut, "This pointer is pointing inside a%s fixed page.\n", temporary);
		fprintf(gcOut, "Page: %lx  (index: %d, offset: %d)\n", 
			page, INDEX_FROM_PTR(p), (Word)p - (Word)page);
		fprintf(gcOut, "Size of objs in this page: %d\n", 
			page->objSize);
		fprintf(gcOut, "Objs allocated in this page: %d\n", 
			page->allocatedObjs);

		if (p - (Ptr) page < FirstObjOffset) {
			fprintf(gcOut, "Address before first object offset.\n");
                        return;
		}
		base = GET_OBJ_BASE(p, page);
		fprintf(gcOut, "pointer base: %lx\n", base);
		if (base == p) {
			fprintf(gcOut, "WARNING: the pointer is pointing before the start of the object!\n");
			return;
		}
		break;
	      case PAGE_Next: {
		int index;

		if (pageSpace[INDEX_FROM_PTR(p)] == SPACE_Temporary)
		   temporary = " *temporary*";

		fprintf(gcOut, "This pointer is in the middle of a%s mixed object\n", temporary);
		fprintf(gcOut, "finding the base page...\n");
		
		index = INDEX_FROM_PTR(p) - 1;
		while (pageMap[index] != PAGE_Mixed) {
			index -= 1;
			assert(index > 0);
		}
		p = PAGE_FROM_INDEX(index);
		goto LAB_PageMixed;
	      }
	      case PAGE_Mixed:

		if (pageSpace[INDEX_FROM_PTR(p)] == SPACE_Temporary)
		   temporary = " *temporary*";

		fprintf(gcOut, "This pointer is pointing inside a%s mixed page\n", temporary);
	      LAB_PageMixed:
		page = (PageHeader) PAGE_FROM_PTR(p);

		fprintf(gcOut, "Size of the obj in this page: %d\n", 
			page->objSize);
		fprintf(gcOut, "Number of pages for this obj: %d\n", 
			page->nPages);

		if (p - (Ptr) page < FirstObjOffset) {
			fprintf(gcOut, "The pointer is pointing before the first object offset!\n");
                        return;
		}
		return;

	      case PAGE_Free:
		if (pageSpace[INDEX_FROM_PTR(p)] == SPACE_Temporary)
		   temporary = " *temporary*";

		fprintf(gcOut, "This pointer is pointing to a chunk of released%s memory.\n", temporary);
		return;

	      case PAGE_Other:
		fprintf(gcOut, "This pointer is pointing to another heap.\n");
		return;

	      default:
		fprintf(gcOut, "BUG -- getPtrInfo(): unknown page type (probably the page map is corrupted).\n");
	}
}

/******************************************************************************
 * :: Functions boxing some macros -- useful for debugging 
 *****************************************************************************/

Ptr
pageFromPtr(Ptr p)
{
	return PAGE_FROM_PTR(p);
}
Ptr
objBase(Ptr p, PageHeader header)
{
	return GET_OBJ_BASE(p, header);
}
int
isInsideHeap(Ptr p)
{
	return IS_INSIDE_HEAP(p);
}
int
indexFromPtr(Ptr p)
{
	return INDEX_FROM_PTR(p);
}
Ptr
pageFromIndex(int index)
{
	return PAGE_FROM_INDEX(index);
}
int
pageSpaceFromPtr(Ptr p)
{
	int space = PAGE_SPACE_FROM_PTR(p);
	if (space == SPACE_Temporary)
	  printf("(temporary)\n");
	else
	  printf("(permanent)\n");
	return space;
}
int
pageInfoFromPtr(Ptr p)
{
	int info = PAGE_INFO_FROM_PTR(p);
	char * s;
	switch (info) {
	      case PAGE_Free: s = "Free"; break;
	      case PAGE_Fixed: s = "Fixed"; break;
	      case PAGE_Mixed: s = "Mixed"; break;
	      case PAGE_Next: s = "Next"; break;
	      case PAGE_Other: s = "Other"; break;
	}
	printf("(%s)\n", s);
	return info;
}
int
bytesToPages(unsigned long bytes)
{
	return BYTES_TO_PAGES(bytes);
}

Word
roundUp(Word a, Word b)
{
	return ROUND_UP(a,b);
}
Word
roundDown(Word a, Word b)
{
	return ROUND_DOWN(a,b);
}
Ptr
getLastObjPtr(Ptr p, Word size)
{
	return GET_LAST_OBJ_PTR(p, size);
}

#ifdef MSW_TRACE_ONE_PAGE
/* Add a call to mswTraceFPage(xxx) on the top of mswAlloc();
 * When you get the problem with a fixed page, take its address and restart
 * setting xxx = address of the corrupted page.
 */
void
mswTraceFPage(Ptr page)
{
	if (IS_INSIDE_HEAP(page) &&
	    PAGE_INFO_FROM_PTR(page) == PAGE_Fixed)
	       mswCheckFPage(page);
}
#endif /* MSW_TRACE_ONE_PAGE */

void
mswCheckAllocatedObj(void * ptr)
{
	Ptr	   obj = (Ptr) ptr;
	int 	   pageInfo = PAGE_INFO_FROM_PTR(obj);
	PageHeader header = (PageHeader) PAGE_FROM_PTR(obj);

	assert(IS_INSIDE_HEAP(obj));
	assert(pageInfo == PAGE_Fixed || pageInfo == PAGE_Mixed);

	if (pageInfo == PAGE_Fixed) {
		Ptr bp = GET_OBJ_BASE(obj, header);
		assert(bp != (Ptr) header);
	}
	else
	        assert((Word) obj - (Word)header  == FirstObjOffset + 1);

}

#ifdef MSW_GET_ALLOC_SIZE_STATS
void
mswShowStats()
{

	int i;
	for (i = 1; i < MAX_TRACED_SIZE; i++)
	  if (totAllocatedSizes[i])
		  printf("%lu objs of size = %lu\n",totAllocatedSizes[i], i);
	printf("%lu objs of size = %lu\n", totHugeAllocated,MAX_TRACED_SIZE);

}
#endif /* MSW_GET_ALLOC_SIZE_STATS */

#endif /* ! NDEBUG */

