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

# ifdef __CYGWIN32__
  /* from Win32API */
typedef void *PVOID;
typedef const void *LPCVOID;
typedef struct _MEMORY_BASIC_INFORMATION { 
  PVOID BaseAddress;            
  PVOID AllocationBase;         
  DWORD AllocationProtect;      
  DWORD RegionSize;             
  DWORD State;                  
  DWORD Protect;                
  DWORD Type;                   
} MEMORY_BASIC_INFORMATION; 
typedef MEMORY_BASIC_INFORMATION *PMEMORY_BASIC_INFORMATION; 
# endif

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
CmmGetWritableLength(char *p)
{
    MEMORY_BASIC_INFORMATION buf;
    unsigned long result;
    unsigned long protect;
    
    result = VirtualQuery(p, &buf, sizeof(buf));
    if (result != sizeof(buf)) {
      fprintf(stderr, "Weird VirtualQuery result\n");
      exit(-1);
    }
    protect = (buf.Protect & ~(PAGE_GUARD | PAGE_NOCACHE));
    if (!is_writable(protect)) {
        return(0);
    }
    if (buf.State != MEM_COMMIT) return(0);
    return(buf.RegionSize);
}

Ptr
CmmGetStackBase(void)
{
    int dummy;
    char *sp = (char *)(&dummy);
    char *trunc_sp = (char *)((unsigned long)sp & ~(CmmGetPageSize() - 1));
    unsigned long size = CmmGetWritableLength(trunc_sp);
   
    return (Ptr) (trunc_sp + size);
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
 * Assumes VirtualQuery returns correct information for start.
 */
char *
CmmLeastDescribedAddress(char * start)
{  
  MEMORY_BASIC_INFORMATION buf;
  SYSTEM_INFO sysinfo;
  DWORD result;
  LPVOID limit;
  char * p;
  LPVOID q;
  
  GetSystemInfo(&sysinfo);
  limit = sysinfo.lpMinimumApplicationAddress;
  p = (char *)((unsigned long)start & ~(CmmGetPageSize() - 1));
  for (;;) {
  	q = (LPVOID)(p - CmmGetPageSize());
  	if ((char *)q > (char *)p /* underflow */ || q < limit) break;
  	result = VirtualQuery(q, &buf, sizeof(buf));
  	if (result != sizeof(buf) || buf.AllocationBase == 0) break;
  	p = (char *)(buf.AllocationBase);
  }
  return(p);
}

void
CmmExamineStaticAreas(void (*ExamineArea)(GCP, GCP))
{
  static char static_root;
  MEMORY_BASIC_INFORMATION buf;
  SYSTEM_INFO sysinfo;
  DWORD result;
  DWORD protect;
  LPVOID p;
  char *base, *limit, *new_limit;
  static char *mallocHeapBase = 0;
  unsigned long v = GetVersion();

  /* Check that this is not NT, and Windows major version <= 3	*/
  if (!((v & 0x80000000) && (v & 0xff) <= 3))
    return;
  /* find base of region used by malloc()	*/
  if (mallocHeapBase == 0) {
    extern int  firstHeapPage;
    result = VirtualQuery((LPCVOID)firstHeapPage, &buf, sizeof(buf));
    if (result != sizeof(buf)) {
      fprintf(stderr, "Weird VirtualQuery result\n");
      exit(-1);
    }
    mallocHeapBase = (char *)(buf.AllocationBase);
  }
  p = base = limit = CmmLeastDescribedAddress(&static_root);
  GetSystemInfo(&sysinfo);
  while (p < sysinfo.lpMaximumApplicationAddress) {
    result = VirtualQuery(p, &buf, sizeof(buf));
    if (result != sizeof(buf) || buf.AllocationBase == mallocHeapBase)
      break;
    new_limit = (char *)p + buf.RegionSize;
    protect = buf.Protect;
    if (buf.State == MEM_COMMIT
	&& is_writable(protect)) {
      if ((char *)p == limit)
	limit = new_limit;
      else {
	if (base != limit)
	  (*ExamineArea)((GCP)base, (GCP)limit);
	base = (char*) p;
	limit = new_limit;
      }
    }
    if (p > (LPVOID)new_limit	/* overflow */) break;
    p = (LPVOID)new_limit;
  }
  if (base != limit)
    (*ExamineArea)((GCP)base, (GCP)limit);
}

#else

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
#   ifdef STACKBOTTOM
	stackBottom = (Word) STACKBOTTOM;
#   else
#     define STACKBOTTOM_ALIGNMENT_M1 0xffffff
#     ifdef STACK_GROWS_DOWNWARD
      stackBottom = (bottom + STACKBOTTOM_ALIGNMENT_M1)
	& ~STACKBOTTOM_ALIGNMENT_M1;
#     else
      stackBottom = bottom & ~STACKBOTTOM_ALIGNMENT_M1;
#     endif
#   endif
}
