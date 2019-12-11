/*---------------------------------------------------------------------------*
 *
 *  tempheap.h:	a heap for temporary data
 *  date:	30 November 1994
 *  authors:	Giuseppe Attardi and Tito Flagella
 *  email:	cmm@di.unipi.it
 *  address:	Dipartimento di Informatica
 *		Corso Italia 40
 *		I-56125 Pisa, Italy
 *
 *  Copyright (C) 1993, 1994, 1995 Giuseppe Attardi and Tito Flagella.
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

#ifndef _tempheap_h
#define _tempheap_h

#include "cmm.h"

#if HEADER_SIZE && defined(DOUBLE_ALIGN)
#define BOTTOM	1
#else
#define BOTTOM	0
#endif

/*---------------------------------------------------------------------------*
 *
 * -- Container
 *
 *---------------------------------------------------------------------------*/

class Container {
 public:
  GCP alloc(int);
  CmmObject *copy(CmmObject *);
  void reset();
#if !HEADER_SIZE || defined(MARKING)
  void resetliveMap();
#endif

  void weakReset() { top = BOTTOM; }

  inline unsigned room() { return ((size - top) * bytesPerWord); }
  int usedBytes() { return bytesPerWord*top; }
  int usedWords() { return top; }

  inline CmmObject *current() {	// returns the first free word
    return (CmmObject *)(body + top);
  }

  inline CmmObject *bottom() {
    return (CmmObject *)(body + BOTTOM + HEADER_SIZE);
  }

  inline bool inside(CmmObject *ptr) {
    return (ptr >= bottom() && ptr < current());
  }

  Container(int, CmmHeap *);

 private:
  int size;			// in words
  GCP body;
  int top;			// index to the first free word
};


/*---------------------------------------------------------------------------*
 *
 * -- RootSet
 *
 *---------------------------------------------------------------------------*/

class RootSet
{
private:
  bool isConservative;
  void scanSystemRoots() {}; // Still to define

public:
  RootSet();
  void insert(CmmObject* p) { roots.insert(p); }
  void insert(CmmObject** p){ rootsLoc.insert(p); }
  void erase(CmmObject* p)  { roots.erase(p); }
  void erase(CmmObject** p) { rootsLoc.erase(p); }
  void scan(CmmHeap *);
  
  // back compatibility
  void set(CmmObject* p)    { roots.insert(p); }
  void setp(CmmObject** p)  { rootsLoc.insert(p); }
  void unset(CmmObject* p)  { roots.erase(p); }
  void unsetp(CmmObject** p){ rootsLoc.erase(p); }
  
private:
  
  Set<CmmObject>  roots;
  Set<CmmObject*> rootsLoc;
};


/*---------------------------------------------------------------------------*
 *
 * -- TempHeap
 *
 *---------------------------------------------------------------------------*/

#define BBStack TempHeap	/* back compatibility */

class TempHeap : public CmmHeap 
{
  
public:
  
  void scavenge(CmmObject **);
  void collect();
  void reset();
  void weakReset();
  
  TempHeap(int bytes = 100000) {
    
    current = 0;
    toCollect = false;
    chunkInc = 4;
    
    chunkNum = chunkInc;
    chunkSize = bytes;
    // Can you use expand here?
    chunk = new Container *[chunkInc];
    for (int i = 0; i < chunkInc; i++)
      chunk[i] = new Container(chunkSize, this);
  }
  
  RootSet roots;
  
private: 
  
  bool toCollect;
  int chunkInc;
  int chunkNum, chunkSize;
  
  int current;
  Container **chunk;
  
  CmmObject *copy(CmmObject *);
  
  GCP alloc(unsigned long);
  
  Container *inside(CmmObject *ptr) {
    for (int i = 0; i <= current; i++)
      {
	if (chunk[i]->inside(ptr))
	  return chunk[i];
      }
    return NULL;
  }
  
  void expand();
};

#endif				// _tempheap_h
/* DON'T ADD STUFF AFTER THIS #endif */
