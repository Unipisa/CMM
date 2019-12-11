/* HeapStack.H -- a heap with a stack allocation policy			*/
/* 
   Copyright (C) 1993 Giuseppe Attardi and Tito Flagella.

   This file is part of the POSSO Customizable Memory Manager (CMM).

   CMM is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   See file 'Copyright' for full details.

*/
#ifndef _HeapStack_h
#define _HeapStack_h

#include "cmm.H"

class Container {
 public:
  GCP alloc(int);
  GcObject *copy(GcObject *);
  void reset();
#if !HEADER_SIZE
  void resetLiveMap();
#endif

  int size() { return bytes; }

  void WeakReset() { top = 0; }

  int room() { return (bytes - BYTESxWORD*top); }
  int usedBytes() { return BYTESxWORD*top; }
  int usedWords() { return top; }

  inline GcObject *current() { return (GcObject *)(body + top); }
				// current() is the first free word

  inline GcObject *bottom() {
#ifdef HEADER_SIZE
#ifdef DOUBLE_ALIGN
    return (GcObject *) (body + 2*HEADER_SIZE);
#else
    return (GcObject *) (body + HEADER_SIZE);
#endif
#else
   return (GcObject *) body;
#endif
  }

  inline bool inside(GcObject *ptr) {
    return (ptr >= bottom() && ptr < current());
  }

  Container(int bytes, CmmHeap *heap);

 private:
  int bytes;
  int *body;
  int top;			// index to the first free word
};

class RootSet
{
private:
  bool IsConservative;
  void ScanSystemRoots() {};	// Still to define

public:
  void set(GcObject *);
  void unset(GcObject *);
  GcObject *get();
  void setp(GcObject **);
  void unsetp(GcObject **);
  GcObject **getp();
  void reset();

  void scan(CmmHeap *);
  
  RootSet ();

private:
  int entryInc;			// = 10;
  int entryNum;			// = 10;
  int current;			// = 0;
  int entrypNum;		// = 10;
  int currentp;			// = 0;

  GcObject **entry;
  GcObject ***entryp;
};

class HeapStack: public CmmHeap
{
private:
  Container *FromSpace;
  Container *ToSpace;

public:
  HeapStack(int = 100000);
  void scavenge(GcObject **);
  inline GCP alloc(int size) {
    return FromSpace->alloc(size);
  }

  inline GcObject* copy(GcObject *ObjPtr) {
    return ToSpace->copy(ObjPtr);
  }

  void collect();

  inline bool inside(GcObject *ptr) {
    return (ptr >= FromSpace->bottom() && ptr <= FromSpace->current());
  }

  RootSet roots;
};

class BBStack : public CmmHeap 
{

public:

  void scavenge(GcObject **);
  void collect();
  void reset();
  void WeakReset();

  BBStack(int words = 25000) {
    ToCollect = false;
    chunkInc = 4;
      
    chunkNum = chunkInc;
    chunkSize = words;
    // Can you use expand here?
    chunk = new Container *[chunkInc];
    for (int i = 0; i < chunkInc; i++)
      chunk[i] = new Container(chunkSize, this);
  }
  
  RootSet roots;

private: 
  
  bool ToCollect;
  int chunkInc;
  int chunkNum, chunkSize;
  
  int current;			// = 0;
  Container **chunk;
  
  GcObject *copy(GcObject *);

  inline GCP alloc(int);
  
  Container *inside(GcObject *ptr) {
    for (int i = 0; i <= current; i++) {
      if (chunk[i]->inside(ptr))
	return chunk[i];
    }
    return NULL;
  }
  
  void expand();
};

#endif _HeapStack_h
/* DON'T ADD STUFF AFTER THIS #endif */
