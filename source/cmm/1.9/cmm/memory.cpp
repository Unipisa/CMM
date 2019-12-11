/*---------------------------------------------------------------------------*
 *
 *  memory.c:	Memory areas
 *  date:	16 February 1996
 *  authors:	Giuseppe Attardi
 *  email:	cmm@di.unipi.it
 *  address:	Dipartimento di Informatica
 *		Corso Italia 40
 *		I-56125 Pisa, Italy
 *
 *  Copyright (C) 1996 Giuseppe Attardi.
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
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include "machine.h"

void *		globalHeapStart = 0; // start of global heap
static void *	globalHeapEnd;

/*---------------------------------------------------------------------------*
 * -- MS Windows
 *---------------------------------------------------------------------------*/
#ifdef _WIN32

/* Code contributed by H. Boehm of Xerox PARC
   Modified by G. Attardi

   Under win32, all writable pages outside of the heaps and stack are
   scanned for roots.  Thus the collector sees pointers in DLL data
   segments.  Under win32s, only the main data segment is scanned.

 */
/*
 * Copyright (c) 1991-1994 by Xerox Corporation.  All rights reserved.
 *
 * THIS MATERIAL IS PROVIDED AS IS, WITH ABSOLUTELY NO WARRANTY EXPRESSED
 * OR IMPLIED.  ANY USE IS AT YOUR OWN RISK.
 *
 * Permission is hereby granted to use or copy this program
 * for any purpose,  provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is granted,
 * provided the above notices are retained, and a notice that the code was
 * modified is included with the above copyright notice.
 */

#include <windows.h>

/* Get the page size.	*/
static unsigned long CmmPageSize = 0;

unsigned long
CmmGetPageSize(void)
{
    SYSTEM_INFO sysinfo;

    if (CmmPageSize == 0) {
        GetSystemInfo(&sysinfo);
        CmmPageSize = sysinfo.dwPageSize;
    }
    return(CmmPageSize);
}

# define is_writable(prot) ((prot) == PAGE_READWRITE \
			    || (prot) == PAGE_WRITECOPY \
			    || (prot) == PAGE_EXECUTE_READWRITE \
			    || (prot) == PAGE_EXECUTE_WRITECOPY)
/* Return the number of bytes that are writable starting at p.	*/
/* The pointer p is assumed to be page aligned.			*/
unsigned long
CmmGetWritableLength(void *p)
{
    MEMORY_BASIC_INFORMATION info;
    unsigned long result;
    unsigned long protect;
    
    result = VirtualQuery(p, &info, sizeof(info));
    if (result != sizeof(info)) {
      fprintf(stderr, "Weird VirtualQuery result\n");
      exit(-1);
    }
    protect = (info.Protect & ~(PAGE_GUARD | PAGE_NOCACHE));
    if (!is_writable(protect)) {
        return(0);
    }
    if (info.State != MEM_COMMIT) return(0);
    return(info.RegionSize);
}

void *
CmmGetStackBase(void)
{
    int dummy;
    void *sp = (void *)(&dummy);
    char *trunc_sp = (char *)((unsigned long)sp & ~(CmmGetPageSize() - 1));
    unsigned long size = CmmGetWritableLength(trunc_sp);
   
    return (void *) (trunc_sp + size);
}

/*
 * Unfortunately, we have to handle win32s very differently from NT,
 * Since VirtualQuery has very different semantics.  In particular,
 * under win32s a VirtualQuery call on an unmapped page returns an
 * invalid result.  Under NT CmmExamineStaticAreas is a noop and
 * all real work is done by GC_register_dynamic_libraries.  Under
 * win32s, we cannot find the data segments associated with dll's.
 * We examine the main data segment here.

 * Return the smallest address p such that VirtualQuery
 * returns correct results for all addresses between p and start.
 */
void *
CmmLeastDescribedAddress(void * start)
{  
  MEMORY_BASIC_INFORMATION info;
  SYSTEM_INFO sysinfo;
  DWORD result;
  LPVOID limit;
  LPVOID p;
  LPVOID q;
  
  GetSystemInfo(&sysinfo);
  limit = sysinfo.lpMinimumApplicationAddress;
  p = (LPVOID)((unsigned long)start & ~(CmmGetPageSize() - 1));
  for (;;) {
  	q = (LPVOID)((char *)p - CmmGetPageSize());
  	if (q > p /* underflow */ || q < limit) break;
  	result = VirtualQuery(q, &info, sizeof(info));
  	if (result != sizeof(info) || info.AllocationBase == 0) break;
  	p = info.AllocationBase;
  }
  return p;
}

void *
getGlobalHeapStart()
{
  MEMORY_BASIC_INFORMATION info;
  DWORD result;
  /* need here a location allocated by malloc() */
  if (globalHeapStart == 0)
	  globalHeapStart = malloc(1);
  result = VirtualQuery((LPCVOID)(globalHeapStart), &info, sizeof(info));
  if (result != sizeof(info)) {
    fprintf(stderr, "Weird VirtualQuery result\n");
    exit(-1);
  }
  globalHeapEnd = globalHeapStart = info.AllocationBase;
  return globalHeapStart;
}

void *
getGlobalHeapEnd()
{
  MEMORY_BASIC_INFORMATION info;
  DWORD result;
  char * limit;
  char * p;
  
  VirtualQuery((LPVOID)globalHeapStart, &info, sizeof(info));
  limit = (char *)globalHeapStart + info.RegionSize;
  for (p = (char *)globalHeapEnd; p < limit; p += CmmPageSize) {
    result = VirtualQuery((LPVOID)p, &info, sizeof(info));
    if (info.State != MEM_COMMIT
	|| !is_writable(info.Protect))
	break;
  }
  globalHeapEnd = p;
  return(globalHeapEnd);
}

void
CmmExamineStaticAreas(void (*ExamineArea)(GCP, GCP))
{
  /* static_root is any static variable */
#define static_root globalHeapStart
  MEMORY_BASIC_INFORMATION info;
  SYSTEM_INFO sysinfo;
  DWORD result;
  LPVOID p;
  char *base, *limit, *new_limit;
#ifdef win3_1
  unsigned long v = GetVersion();

  /* Check that this is not NT, and Windows major version <= 3	*/
  if (!((v & 0x80000000) && (v & 0xff) <= 3))
    return;
#endif
  /* find base of region used by malloc()	*/
  getGlobalHeapStart();
  p = base = limit = (char *)CmmLeastDescribedAddress(&static_root);
  GetSystemInfo(&sysinfo);
  while (p < sysinfo.lpMaximumApplicationAddress) {
    result = VirtualQuery(p, &info, sizeof(info));
    if (result != sizeof(info) || info.AllocationBase == globalHeapStart)
      break;
    new_limit = (char *)p + info.RegionSize;
    if (info.State == MEM_COMMIT
	&& is_writable(info.Protect)) {
      if ((char *)p == limit)
	limit = new_limit;
      else {
	if (base != limit)
	  (*ExamineArea)((GCP)base, (GCP)limit);
	base = (char *)p;
	limit = new_limit;
      }
    }
    if (p > (LPVOID)new_limit)	/* overflow */
	break;
    p = (LPVOID)new_limit;
  }
  if (base != limit)
    (*ExamineArea)((GCP)base, (GCP)limit);
}

#elif defined(macintosh)

#include <Memory.h>

void *
getGlobalHeapEnd()
{
	/* PCB:  this is an upper limit. */
	THz applicationHeap = ApplicationZone();
	return (void*)applicationHeap->bkLim;		/* PCB:  end of the application heap is as far as it goes. */
}

void CmmExamineStaticAreas(void (*ExamineArea)(GCP, GCP))
{
	extern char __data_start__[], __data_end__[];
	(*ExamineArea)((GCP)&__data_start__, (GCP)&__data_end__);
}

#else

void *
getGlobalHeapEnd()
{
  return sbrk(0);
}

void
CmmExamineStaticAreas(void (*ExamineArea)(GCP, GCP))
{
  extern int end;
  (*ExamineArea)((GCP)DATASTART, (GCP)&end);
}

#endif /* _WIN32 */

/*---------------------------------------------------------------------------*
 * -- Stack Bottom
 *---------------------------------------------------------------------------*/

Word stackBottom;	/* The base of the stack	*/

void
CmmSetStackBottom(Word bottom)
{
#ifdef STACKBOTTOM
	stackBottom = (Word) STACKBOTTOM;
#else
#	define STACKBOTTOM_ALIGNMENT_M1 0xffffff
#	ifdef STACK_GROWS_DOWNWARD
	stackBottom = (bottom + STACKBOTTOM_ALIGNMENT_M1)
	& ~STACKBOTTOM_ALIGNMENT_M1;
#	else
	stackBottom = bottom & ~STACKBOTTOM_ALIGNMENT_M1;
#	endif
#endif
}

/*---------------------------------------------------------------------------*
 * -- Data Start
 *---------------------------------------------------------------------------*/

#ifdef __svr4__

char *
CmmSVR4DataStart(int max_page_size)
{
  Word text_end = ((Word)(&etext) + sizeof(Word) - 1) & ~(sizeof(Word) - 1);
  /* etext rounded to word boundary	*/
  Word next_page = (text_end + (Word)max_page_size - 1)
    & ~((Word)max_page_size - 1);
  Word page_offset = (text_end & ((Word)max_page_size - 1));
  char * result = (char *)(next_page + page_offset);
  /* Note that this isn't equivalent to just adding
   * max_page_size to &etext if &etext is at a page boundary
   */
  return(result);
}
#endif
