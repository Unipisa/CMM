/* cmm.H  -- definitions for the CMM
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


/* Defining garbage collected classes
   ----------------------------------
   Classes allocated in the garbage collected heap are derived from class
   GcObject.
   The collector applies method traverse() to an object to find other objects
   to which it points.
   A method for traverse() must be supplied by the programmer for each such
   collected class which contains pointers to other collected objects,
   defined according to the following rules:

   (a) for a class containing a pointer, say class C { type *x; },
       the method C::traverse must contain scavenge(&x);

   (b) for a class containing an instance of a collected object, say
       class C { GcClass x; }, the method C::traverse must contain
       x.traverse();

   (c) for a class derived from another collected class, say
       class C: GcClass {...}, the method C::traverse must contain
       GcClass::traverse();

   (d) for a class deriving from a virtual base class, say class
       C: virtual GcClass {...}, the method C::traverse must contain
       scavenge(VirtualBase(GcClass));

   For example,

   class BigNum: public GcObject
   {
     long data;
     BigNum *next;                         // Rule (a) applies here
     void traverse();
   }

   class monomial: private BigNum          // Rule (c) applies here
   {
     PowerProduct pp;                      // Rule (b) applies here
     void traverse();
   }

   A BigNum stores in next a pointer to a collected object which needs to
   be scavenged, so traverse becomes:

   void BigNum::traverse()
   {
     CmmHeap::heap->scavenge(&next);   // Applying rule (a)
   }

   Because monomial inherits from BigNum, the method traverse for this base
   class must be invoked; finally, since a monomial contains a BigNum in pp,
   this object must be traversed as well:

   void monomial::traverse()
   {
     BigNum::traverse();                   // Appling rule (c)
     pp.traverse();                        // Applying rule (b)
   }

   Once the object has been defined, storage is allocated using the normal
   C++ mechanism:

	bn = new Bignum();


   Variable size objects
   ---------------------
   In order to allocate variable size objects, the size of the variable
   portion of the object must be defined when the object is created.
   Classes of variable sized objects must derive from class GcVarObject.

   Arrays of collected objects
   ---------------------------
   Garbage collected arrays of garbage collected objects can be created
   by using class GcArray.

   Caveats
   -------
   When the garbage collector is invoked, it searches the processor's
   registers, the stack, and the program's static area for "hints" as to what
   storage is still accessible.  These hints are used to identify objects that
   are the "roots" and are to be left in place.  Objects that the roots point
   to will be moved to compact the heap.  Because of this:

   Objects allocated in the garbage collected heap MAY MOVE.

   Pointers to garbage collected objects MAY BE passed as arguments or stored
   in static storage.

   Pointers to garbage collected objects MAY NOT be stored in dynamically 
   allocated objects that are not garbage collected, UNLESS one has specified
   the GCHEAPROOTS flag in a gcheap declaration, OR declared that region as
   a root via a call to gcroots.

   Pointers to garbage collected objects contained in garbage collected objects
   MUST always point outside the garbage collected heap or to a garbage
   collected object.  To assure this, storage is zeroed at object creation
   time.

   Sizing the heap
   ---------------
   In order to make heap allocated storage as painless as possible, the user
   does not have to do anything to configure the heap.  This default is an
   initial heap of 1 megabyte that is expanded in 1 megabyte increments
   whenever the heap is more than 25% full after a total garbage collection.
   Total garbage collections are done when the heap is more than 35% full.

   However, if this is not the desired behavior, then it is possible to "tune"
   the collector by including one or more global gcheap declarations in the
   program.  In order to understand the parameters supplied in a gcheap
   declaration, one needs an overview of the storage allocation and garbage
   collection algorithm.

   Storage is allocated from the heap until 50% of the heap has been allocated.
   All accessible objects allocated since the last collection are retained and
   made a part of the stable set.  If less than <collect all percent> of the
   heap is allocated, then the collection process is finished.  Otherwise, the
   entire heap (including the stable set) is garbage collected.  If the amount
   allocated following the total collection is greater than <increment heap
   percent>, then an attempt is made to expand the heap.

	gcheap  <CC-identifier>(<initial heap size>,
			         <maximum heap size>,
			         <increment size>,
			         <full collection>,
			         <increment heap percent>,
			         <flags>)

   The arguments are defined as follows:

	<CC-identifier>		 a legal C++ identifier.
	<initial heap size>	 initial size of the heap in bytes.
				 DEFAULT: 131072.
	<maximum heap size>	 maximum heap size in bytes.
				 DEFAULT: 2147483647.
	<increment size>	 # of bytes to add to each heap on each
				 expansion.  DEFAULT: 1048576.
	<full collection>	 number between 0 and 50 that is the percent
				 allocated after a partial collection that will
				 force a total collection.  A value of 0 will
				 disable generational collection.  DEFAULT: 35.
	<increment heap percent> number between 0 and 50 that is the percent
				 allocated after a total collection that will
				 force heap expansion.  DEFAULT: 25.
	<flags>			 controls logging on stderr, error checking,
				 and root finding:
				   & GCSTATS =  log collection statistics
				   & GCROOTLOG = log roots found in the stack,
						 registers, and static area
				   & GCHEAPROOTS = treat uncollected heap as
				   		   roots
				   & GCHEAPLOG = log possible roots in
						 uncollected heap
				   & GCTSTOBJ = perform object consistency
					        tests
			 	   & GCDEBUGLOG = log events internal to the
						  garbage collector
				 DEFAULT: 0.

   When multiple gcheap declarations occur, the one that specifies the largest
   <maximum heap size> value will control all factors except flags which is
   the inclusive-or of all <flags> values.

   Configured values may be overridden by values supplied from environment
   variables.  The user must set these variables in a consistent manner.  The
   variables and the values they set are:

	GCMINBYTES	<initial heap size>
	GCMAXBYTES	<maximum heap size>
	GCINCBYTES	<increment size>
	FULLGCTHRESHOLD	<full collection>
	GCINCPERCENT	<increment heap percent>
	GCFLAGS		<flags>

   If any of these variables are supplied, then the actual values used to
   configure the garbage collector are logged on stderr.

*/


#ifndef _CMM_H
#define _CMM_H

#ifndef bool
#define bool int
#define false 0
#define true 1
#endif

#ifdef  __cplusplus
extern "C" {
#endif
#include <stdio.h>	/* Streams are not used as they might not be
			   initialized when needed. */
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#ifdef  __cplusplus
}
#endif
#include <new.h>

       /*******************************************************
 	* 	  CMM External Interface Definitions	      *
 	*******************************************************/

class CmmHeap;
class DefaultHeap;
class UncollectedHeap;
class GcObject;

typedef int  *GCP;		/* Pointer to a garbage collected object */

extern GCP CmmMove(GCP);		/* Copies object to next space		*/

extern GCP allocate_page(int, CmmHeap*); /* Page allocator		*/

/*
 * Predicate gcobject returns 1 if the object is allocated where it will
 * be scanned by the garbage collector, otherwise it returns 0.
 */

extern int  gcobject(void* ptr);

/*
 * Support for rule (d) above. Compiler dependent.
 */
#ifdef __GNUG__
#define VirtualBase(A) &(_vb$ ## A)
#endif

/*
 * Additional roots may be registered with the garbage collector by calling
 * the procedure gcroots with a pointer to the area and the size of the area.
 */

extern void  gcroots(void* area, int bytes);
extern void  gcunroots(void* addr);

/* The class gcheap is used to configure the heap as earlier described.  */

class  gcheap {
   public:
	gcheap(int minheapbytes,
		int maxheapbytes,
		int incheapbytes,
		int FullGcThreshold,
		int incpercent,
		int flags);
};

const	GCSTATS	   =   1,	/* Log garbage collector info		*/
	GCMEM	   =   2,	/* not used (was Log memory usage) 	*/
	GCROOTLOG  =   4,	/* Log roots found in registers, stack
				   and static area			*/
	GCHEAPROOTS =  8,	/* Treat uncollected heap as roots	*/
	GCHEAPLOG  =  16,	/* Log possible uncollected heap roots	*/
	GCTSTOBJ   =  32,	/* Extensively test objects		*/
	GCDEBUGLOG =  64;	/* Log events internal to collector	*/

#define CMM_VERBOSE
#ifdef CMM_VERBOSE
extern int CmmVerbosity;
#endif

/***********************************************************************/

/***********************************************************************
  Object Headers
 ***********************************************************************/
/*
 * Object have headers if HEADER_SIZE is not 0
 */
#define HEADER_SIZE	1	/* header size in words */

#if HEADER_SIZE
#define MAKE_TAG(index) ((index) << 21 | 1)
#define MAKE_HEADER(words, tag) ((words) << 1 | (tag))

#define HEADER_TAG(header) ((header) >> 21 & 0x7FF)
#define HEADER_WORDS(header) ((header) >> 1 & 0xFFFFF) // include HEADER_SIZE
#define MAX_HEADER_WORDS 0xFFFFF		/* 1048575 = 4,194,300 bytes */
#define FORWARDED(header) (((header) & 1) == 0)
#else
/* an object is forwarded if it is marked as live and contained
 * in a non-promoted page.
 */
#define FORWARDED(gcp) ((MARKED(gcp) && \
			 pageSpace[GCP_to_PAGE(gcp)] == current_space))
#endif

/*
 * The base address of GcObject's is noted in the ObjectMap bit map.  This 
 * allows CmmMove() to rapidly detect a derived pointer and convert it into an
 * object and an offset.
 */

extern int  firstHeapPage,	/* Page # of first heap page		*/
	    lastHeapPage,	/* Page # of last heap page		*/
	    *ObjectMap;		/* Bitmap of 1st words of user objects	*/
#if !HEADER_SIZE
extern int  *LiveMap;           /* Bitmap of objects reached during GC	*/
#endif
extern short *pageSpace;	/* Space number for each page		*/
extern CmmHeap **pageHeap;	/* Heap to which each page belongs	*/


/* Constants used below:
   2 = log2(sizeof(int))
   7 = log2(BITSxWORD) + 2 = 3 + 2*log2(sizeof(int))
   0x1F = BITSxWORD - 1
 */
#define WORD_INDEX(p)	(((int)(p)) >> 7)
#define BIT_INDEX(p)	(((int)(p)) >> 2 & 0x1F)

#define IS_OBJECT(p)	   (ObjectMap[WORD_INDEX(p)] >> BIT_INDEX(p) & 1)
#define SET_OBJECTMAP(p)   (ObjectMap[WORD_INDEX(p)] |= 1 << BIT_INDEX(p))
#define CLEAR_OBJECTMAP(p) ObjectMap[WORD_INDEX(p)] &= ~(1 << BIT_INDEX(p))

#define MARKED(p)	(LiveMap[WORD_INDEX(p)] >> BIT_INDEX(p) & 1)
#define MARK(p)		(LiveMap[WORD_INDEX(p)] |= 1 << BIT_INDEX(p))


       /*******************************************************
 	* C++ Garbage Collected Storage Interface Definitions *
 	*******************************************************/

/* Declarations for objects not directly used by the user of the interface. */

/*	Page setting					*/

/* BYTESxPAGE controls the number of bytes per page */
#define BYTESxPAGE 512
#define WORDSxPAGE (BYTESxPAGE / sizeof(int))
#define BYTESxWORD (sizeof(int))
#define	BITSxWORD  32

/* Page number <--> pointer conversion */

#define PAGE_to_GCP(p) ((GCP)((p)*BYTESxPAGE))
#define GCP_to_PAGE(p) (((int)p)/BYTESxPAGE)

/* The following define is used to compute the number of words needed for
   an object.
*/

#ifdef DOUBLE_ALIGN
/* GcObject's smaller than 16 bytes (including vtable) cannnot contain
   doubles (the compiler must add padding between vtable and first float)
*/
#define DOUBLE_ALIGN_OPTIMIZE
#ifdef DOUBLE_ALIGN_OPTIMIZE
#define BYTEStoWORDS(x) (((x) < 16) ? \
			 (((x) + BYTESxWORD-1) / BYTESxWORD) + HEADER_SIZE : \
			 (((x) + HEADER_SIZE + 2*BYTESxWORD-1) / \
			  (2*BYTESxWORD) * 2))
#else
#define BYTEStoWORDS(x) (((x) + HEADER_SIZE + 2*BYTESxWORD-1) / \
			 (2*BYTESxWORD) * 2)
#endif
#else
#define	BYTEStoWORDS(x) ((((x) + BYTESxWORD-1) / BYTESxWORD) + HEADER_SIZE)
#endif

/* Page space values */

#define STABLE(x) 	(! UNSTABLE(x))
#define UNSTABLE(x) 	(pageSpace[x] & 1)
#define UNALLOCATEDSPACE 1	/* neither current nor stable	*/

#define NOHEAP NULL
#define UNCOLLECTEDHEAP ((CmmHeap *)1)

#define OUTSIDE_HEAP(page) \
	(page < firstHeapPage || page > lastHeapPage || \
	 pageHeap[page] == UNCOLLECTEDHEAP)

#define HEAPPERCENT(x) (((x)*100)/(CmmHeap::theDefaultHeap->reservedPages \
			+ freePages))

#ifdef TRACE
#define WHEN_FLAGS(flags, code)	if (gcflags & flags) code
#else
#define WHEN_FLAGS(flags, code)
#endif TRACE


/***********************************************************************
 Heaps
 ***********************************************************************/

class CmmHeap
{
public:

  virtual GCP alloc(int) = 0;
  virtual void reclaim(GCP) {};

  // the garbage collector for this region
  virtual void collect() {
    fprintf(stderr, "Warning: Garbage Collection on a not collectable Heap");
  }

  virtual void scavenge(GcObject **) {};

  inline bool inside(GCP ptr) {
      int page = GCP_to_PAGE(ptr);	/* Page number */
      return (page >= firstHeapPage && page <= lastHeapPage &&
	      pageHeap[page] == this);
    }

  inline void visit(GcObject *);

  static DefaultHeap *theDefaultHeap;

  static UncollectedHeap *theUncollectedHeap;

  static CmmHeap *heap;
};


/***********************************************************************
 The UncollectedHeap
 ***********************************************************************/

class UncollectedHeap: public CmmHeap
{
public:

  GCP alloc(int size) { return (GCP)malloc(size); }

  void reclaim(GCP ptr) { free(ptr); }
};


GcObject *basePointer(GCP);


/***********************************************************************
  The DefaultHeap
 ***********************************************************************/

class DefaultHeap: public CmmHeap
{
public:
  
  GCP alloc(int);
  
  void reclaim(GCP) {}	// Bartlett's delete does nothing.
  
  void collect();		// the default garbarge collector

  void scavenge(GcObject **ptr);

  GCP  reserve_pages(int);
  
  int usedPages,		// pages in actual use
      reservedPages,		// pages reserved for this heap
      stablePages,		// # of pages in the stable set
  firstUnusedPage,		// where to start lookiing for unused pages
  firstReservedPage,		// first page used by this Heap
  lastReservedPage;		// last page used by this Heap
};


/***********************************************************************
  GC Objects
 ***********************************************************************/

class GcObject
{
public:

  virtual void traverse() {};

  virtual ~GcObject() {} ;

  CmmHeap *heap() {
    return pageHeap[GCP_to_PAGE(this)] ;
  }

  inline int size() { return (words()*BYTESxWORD); }

#if HEADER_SIZE
  inline int words() { return HEADER_WORDS(((GCP)this)[-HEADER_SIZE]); }
#else
  int words();
#endif

#ifdef MARKING
  inline void mark() { MARK(this); }

  inline bool Marked() { return (MARKED(this)); }
#endif

  inline int forwarded() {
#if HEADER_SIZE
    return FORWARDED(((GCP)this)[-HEADER_SIZE]);
#else
    extern int current_space;
    return FORWARDED(((GCP)this));
#endif
  }
  inline void SetForward(GcObject *ptr) {
#if !HEADER_SIZE
    MARK(this);
#endif
    ((GCP)this)[-HEADER_SIZE] = (int)ptr;
  }
  inline GcObject *GetForward() {
    return (GcObject *) ((GCP)this)[-HEADER_SIZE];
  }
  inline GcObject *next() {return (GcObject *)(((GCP)this) + words()); }

  void* operator new(size_t, CmmHeap* = CmmHeap::heap);
  void operator delete(void *);
};

class GcVarObject: public GcObject 
{
public:
  void* operator new(size_t, size_t = (size_t)0, CmmHeap* = CmmHeap::heap);
};

/***********************************************************************
  Arrays of objects
 ***********************************************************************/

// Class GcArray must be used to create arrays of GcObject's as follows:
//
//       GcArray<MyClass> MyVector = * new (100) GcArray<MyClass> ;
//       
// Then you can use the [] operator to get GcObjects as usual.
// Ex:
//       MyVector[i]->print();
// or:
//       MyClass mc = MyVector[3];
//

template <class T>
class GcArray : GcObject
{

public:

  void * operator new(size_t s1, size_t s2 = 0, CmmHeap* hz = CmmHeap::heap) {
    // count = s2;
    void* res = new (s1 + sizeof(T) * (s2-1), hz) GcVarObject ;
    T* array = (T*)&(((GcArray<T> *)res)->ptr[0]);
    for (size_t i = 1; i < s2; i++)
	 ::new (&array[i]) T;

    return res;
  }
/* Tito's version:
    // It's necessary to initialize the GcObject part of any object 
    // in the array to get virtual functions, like traverse.
    // Use tmp to do that.
    T *tmp = new T;

    void *res, *tres;
    res = new (s1 + sizeof(T) * (s2-1), hz) GcVarObject;
    tres = &(((GcArray<T> *)res)->ptr[0]);
    for (int i = 0 ; i < s2; i++) {
      for (int j = 0; j < (sizeof(GcObject) / sizeof(int)); j++)
	((GCP)tres)[j] = ((GCP)tmp)[j];
      tres += sizeof(T);
    }

    return (void *) res;
  } */

  ~GcArray() {
    size_t i;
    unsigned int count = ((size() - sizeof(GcArray)) / sizeof(T)) + 1 ;
    for (i = 1; i < count; ++i)
      ptr[i].~T();
  }

  T & operator[](unsigned int index) {
    return ptr[index];
  }

  void traverse() {
    unsigned int count = ((size() - sizeof(GcArray)) / sizeof(T)) + 1 ;
    for (int i = 0 ; i < count; i++)
      ptr[i].traverse();
  }

private:
  //  unsigned int count;
  T ptr[1];
};


inline void CmmHeap::visit(GcObject *ptr) {
#ifdef MARKING
    if (! ptr->Marked()) {
      ptr->mark();
      ptr->traverse();
    }
#else
    ptr->traverse();
#endif
  }

/***********************************************************************
 Library initialization
 ***********************************************************************/

class _CmmInit
{
public:
  _CmmInit()
    {
      extern CmmInitEarly();

      if (CmmHeap::theDefaultHeap == 0) {
	CmmInitEarly();
	
	CmmHeap::heap = CmmHeap::theDefaultHeap = new DefaultHeap;
      }
    }
  ~_CmmInit() {};		// destroy _DummyCmmInit after loading cmm.H
};

static _CmmInit _DummyCmmInit;

#endif				// _CMM_H
/* DON'T ADD STUFF AFTER THIS #endif */
