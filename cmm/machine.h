/* machine.h -- Processor Dependent Definitions */

/* 
   Copyright (C) 1993 Giuseppe Attardi and Tito Flagella.

   This file is part of the POSSO Customizable Memory Manager (CMM).

   CMM is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   See file 'Copyright' for full details.

*/

#ifdef __GNUC__
extern "C"  void  bzero(void *, int);
#else
#include <string.h>
#define bzero(s, n)	memset(s, 0, n)
#endif

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#ifdef __svr4__
#define _setjmp  setjmp
#define _longjmp longjmp
#endif

#ifndef hp9000s800
#define STACK_GROWS_DOWNWARD
#endif

/***********************************************************************/
/* Architectures requiring double alignement for objects               */

#if defined(__mips) || defined(__sparc) || defined(__alpha)
#ifndef MISALIGN
#define	DOUBLE_ALIGN
#endif
#endif

/***********************************************************************/
/* Start of data segment.                                              */

#if defined(__alpha)
#   define DATASTART (0x140000000)
#elif defined(__sgi) || defined(_sgi) || defined(sgi)
    typedef    unsigned int    uint;
#   include <sys/immu.h>
#   define DATASTART USRDATA
#elif defined(__svr4__) || defined(DGUX)
    extern int etext;
#   define DATASTART Svr4DataStart(0x10000)
#elif defined(__i386) || defined(hp9000s300)
    extern int etext;
#   define DATASTART ((((unsigned long)(&etext)) + 0xfff) & ~0xfff)
#elif defined(__mips)
#   include <machine/vmparam.h>
#   define DATASTART USRDATA
#elif defined(__hppa)
    extern int __data_start;
#   define DATASTART (&__data_start)
#elif defined(ibm032)		// IBM RT
#   define DATASTART 0x10000000
#elif defined(_IBMR2)		// IBM RS/6000
#   define DATASTART 0x20000000
#   define STACKBOTTOM ((unsigned long)0x2ff80000)
#elif defined(__NeXT)
#   define DATASTART ((unsigned long) get_etext())
#   define STACKBOTTOM ((unsigned long) 0x4000000)
#elif defined(ns32000)
  extern char **environ;
#   define DATASTART (&environ)
#elif defined(__sparc)
	/* This assumes ZMAGIC, i.e. demand-loadable executables.	*/
#   define DATASTART (*(int *)0x2004+0x2000)
#elif defined(sun3)
    extern char etext;
#   define DATASTART ((((unsigned long)(&etext)) + 0x1ffff) & ~0x1ffff)
#elif defined(SCO)
#   define DATASTART ((((unsigned long)(&etext)) + 0x3fffff) & ~0x3fffff) \
		      + ((unsigned long)&etext & 0xfff)
#elif defined(_AUX_SOURCE)
	/* This only works for shared-text binaries with magic number 0413.
	   The other sorts of SysV binaries put the data at the end of the text,
	   in which case the default of &etext would work.  Unfortunately,
	   handling both would require having the magic-number available.
	   	   		-- Parag
	   */
#   define DATASTART ((((unsigned long)(&etext)) + 0x3fffff) & ~0x3fffff) \
		      + ((unsigned long)&etext & 0x1fff)
#else
/* The default case works only for contiguous text and data, such as   */
/* on a Vax.                                                           */
    extern char etext;
#   define DATASTART (&etext)
#endif

#define Svr4DataStart(max_page_size) \
   ((unsigned long)(&etext) % max_page_size ? \
    (unsigned long)(&etext) + max_page_size : \
    (unsigned long)&etext)
