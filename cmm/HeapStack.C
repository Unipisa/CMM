/* HeapStack.C -- a heap with a stack allocation policy			*/
/* 
   Copyright (C) 1993 Giuseppe Attardi and Tito Flagella.

   This file is part of the POSSO Customizable Memory Manager (CMM).

   CMM is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   See file 'Copyright' for full details.

*/

#include "machine.H"
#include "HeapStack.H"

#ifdef GC_VERBOSE
#include <iostream.h>
#endif


Container::Container(int size, CmmHeap *heap)
{
  int pages = (size + BYTESxPAGE - 1) / BYTESxPAGE;
  body = allocate_page(pages, heap);
  bytes = (pages * BYTESxPAGE);
  top = 0;
}

void Container::reset()
{
  // body was allocated using allocate_page.
  // So we are sure body is aligned to BITSxWORD

  bzero((char*)&ObjectMap[WORD_INDEX(body)],
	((usedWords() + BITSxWORD - 1) / BITSxWORD) * BYTESxWORD);
#if !HEADER_SIZE
  bzero((char*)&LiveMap[WORD_INDEX(body)],
	((usedWords() + BITSxWORD - 1) / BITSxWORD) * BYTESxWORD);
#endif
  top = 0;
}

#if !HEADER_SIZE
void Container::resetLiveMap()
{
  bzero((char*)&LiveMap[WORD_INDEX(body)],
	((usedWords() + BITSxWORD - 1) / BITSxWORD) * BYTESxWORD);
}
#endif

// This function does not check for the available space
// The caller must use room() to verify that.
GCP Container::alloc(int bytes)
{
#ifdef DOUBLE_ALIGN
    // ***** WARNING ***** doubleword alignement should be supported
#endif
  int *object = body + top;
  top += BYTEStoWORDS(bytes);
  return (GCP)(object);
}

// This function is too much dependent from the
// cmm.C internals. This code should be moved 
// to the GcObject level.
/* ----------------------------------------------------------------------
 * copy  --
 *
 * copies object into the container
 *
 * Side effects: top
 *______________________________________________________________________*/

GcObject *Container::copy(GcObject *ptr)
{
  int *object = body + top;
  register int words = ptr->words();
  if (words <= bytes - top) {
    top += words;
#ifdef HEADER_SIZE
    *object++ = ((GCP)ptr)[-HEADER_SIZE]; // Copy the header first.
#ifdef DOUBLE_ALIGN
    // ***** WARNING ***** doubleword alignement should be supported
#endif
#endif

    register int *scan = object;
#if HEADER_SIZE
    // Copy words-HEADER_SIZE because the header is included in words
    words -= HEADER_SIZE;
#endif
    while  (words--)  *scan++ = *((GCP)ptr)++;
    SET_OBJECTMAP(object);
    return (GcObject *)object;
  } else
    return (GcObject *) - 1;
}


/***********************************************************************
  RootSet
  ***********************************************************************/

RootSet::RootSet()
{
  entryInc = 10;
  entryNum = 10;
  current = 0;
  entrypNum = 10;
  currentp = 0;
  
  // Default to not-conservative
  IsConservative = false;
  
  entry = new GcObject*[entryNum];
  entryp = new GcObject**[entrypNum];
  
  for (int i = 0; i < entryNum; i++)
    entry[i] = NULL;
  for (i = 0; i < entrypNum; i++)
    entryp[i] = NULL;
  
}

void RootSet::set(GcObject *obj)
     // trivial implementation, but this is not a critical operation
{
  int i;
  for (i = 0; i < entryNum; i++)
    if (entry[i] == NULL) {
      entry[i] = obj;
      return;
    }
  GcObject **tmp = new GcObject*[entryNum + entryInc];
      
  for (i = 0; i < entryNum; i++)
    tmp[i] = entry[i];
  delete entry;
  entry = tmp;
  entry[i++] = obj;
  entryNum += entryInc;
  // put the rest to NULL.
  for (; i < entryNum; i++)
    entry[i] = NULL;
}

void RootSet::unset(GcObject *obj)
{
  int i;
  for (i = 0; ((i < entryNum) && (entry[i] != obj)); i++);
  assert (entry[i] == obj);
  entry[i] = NULL;
}

GcObject *RootSet::get()
{
  // look for a not empty entry
  while (current < entryNum) {
    if (entry[current])
      return entry[current++];
    else
      current++;
  }
  // No more entries;
  return (GcObject *)NULL;
}

void RootSet::setp(GcObject **obj)
     // trivial implementation, but this is not a critical operation
{
  int i;
  for (i = 0; i < entrypNum; i++)
    if (entryp[i] == NULL) {
      entryp[i] = obj;
      return;
    }
  GcObject ***tmp = new GcObject**[entrypNum + entryInc];
      
  for (i = 0; i < entrypNum; i++)
    tmp[i] = entryp[i];
  delete entryp;
  entryp = tmp;
  entryp[i++] = obj;
  entrypNum += entryInc;
  // put the rest to NULL.
  for (; i < entrypNum; i++)
    entryp[i] = NULL;
}

void RootSet::unsetp(GcObject **obj)
{
  int i;
  for (i = 0; ((i < entrypNum) && (entryp[i] != obj)); i++);
  assert (entryp[i] == obj);
  entryp[i] = NULL;
}

GcObject **RootSet::getp()
{
  // look for a not empty entry
  while (currentp < entrypNum) {
    if (entryp[currentp])
      return entryp[currentp++];
    else
      currentp++;
  }
  // No more entries;
  return (GcObject **)NULL;
}

void RootSet::reset()
{
  current = 0;
  currentp = 0;
}

void RootSet::scan(CmmHeap *heap)
{
  reset();
  GcObject *objPtr, **objPtrPtr;
  CmmHeap *oldHeap = CmmHeap::heap;
  CmmHeap::heap = heap;
  while (objPtr = get()) objPtr->traverse();
  while (objPtrPtr = getp()) heap->scavenge(objPtrPtr);
  CmmHeap::heap = oldHeap;
}

/***********************************************************************
  HeapStack
  ***********************************************************************/

void HeapStack::scavenge(GcObject **ptr)
{
  GCP pp = (GCP)*ptr;
  if (OUTSIDE_HEAP(GCP_to_PAGE(pp)))
    return;

  GcObject *p = basePointer(pp);
  int offset = (int)pp - (int)p;
  
  if (!inside(p))
    visit(p);
  else if
#if HEADER_SIZE
    (p->forwarded())
#else
      (MARKED(p))
#endif
    *ptr = (GcObject *)((int)p->GetForward() + offset);
  else {
    GcObject *newObj = copy(p);
    p->SetForward(newObj);
    *ptr = (GcObject *)((int)newObj + offset);
  }
}

HeapStack::HeapStack(int StackSize)
{
  FromSpace = new Container(StackSize, this);
  ToSpace = new Container(StackSize, this); 
}

void HeapStack::collect()
{
  GcObject *objPtr;
  
  if (FromSpace->usedBytes() < FromSpace->size() * 0.6)
    return;
  
#ifdef GC_VERBOSE
  cerr << "collecting on HeapStack" << endl << "From Space is " << FromSpace->usedBytes() << endl;
#endif
  
#if !HEADER_SIZE
  /* Clear the LiveMap bitmap */
  FromSpace->resetLiveMap();
  ToSpace->resetLiveMap();
#endif

  // In the first step traverse the objects registered as roots,
  // but using your scavenge instead of theirs
  
  // Force traversing roots with `this` heap.
  roots.scan(this);
  
  objPtr = ToSpace->bottom();
  SET_OBJECTMAP(ToSpace->current());	// ensure that we stop at the end
  while (ToSpace->inside(objPtr)) {
    // The default for traverse is the object's heap
    objPtr->traverse();
    objPtr = objPtr->next();
  }
  
#ifdef GC_VERBOSE
  cerr << endl << "ToSpace is " << ToSpace->usedBytes() << endl;
#endif  
  
  Container *TmpSpace;
  
  FromSpace->reset();
  TmpSpace = FromSpace;
  FromSpace = ToSpace;
  ToSpace = TmpSpace;
}

void BBStack::expand()
{
  if (current == chunkNum - 1) {
    ToCollect = true;
      
    Container **nc = new Container*[chunkNum + chunkInc];
      
    for (int i = 0;  i <= current;  i++)
      nc[i] = chunk[i];
    delete chunk;
    chunkNum += chunkInc;
    for (; i < chunkNum; i++)
      nc[i] = new Container(chunkSize, this);
    chunk = nc;
  }
  current++;
}

GCP BBStack::alloc(int bytes)
{ 
  if ((chunk[current]->room()) < BYTEStoWORDS(bytes) * BYTESxWORD + 4)
    // + 4 to take into account possible alignement
    expand(); // expand() modifies current.
  
  return chunk[current]->alloc(bytes);
}

GcObject *BBStack::copy(GcObject *ptr)
{
  
  if (chunk[current]->room() < ptr->size())
    expand();
  return chunk[current]->copy(ptr);
}

void swap(Container **ptr1, Container **ptr2)
{ 
  Container *tmp = *ptr1;
  *ptr1 = *ptr2;
  *ptr2 = tmp;
}

void BBStack::scavenge(GcObject **ptr)
{
  GCP pp = (GCP)*ptr;
  if (OUTSIDE_HEAP(GCP_to_PAGE(pp)))
    return;
  
  GcObject *oldPtr = basePointer(pp);
  int offset = (int)pp - (int)oldPtr;
  
  if (!inside(oldPtr)) {
#ifdef MARKING
    visit(oldPtr);
#else
    /* When not using marking you would loop here.
       Note that following is pheraps useless here, if we
       decide, as seems, to scan all the containers
       of other heaps when collecting on the default one.
       The choice still makes sense anyway for other heaps. */
    return;
#endif
  }
  
  if
#if HEADER_SIZE
    (oldPtr->forwarded())
#else
      (MARKED(oldPtr))
#endif
    *ptr = (GcObject *)((int)oldPtr->GetForward() + offset);
  else {
    GcObject *newObj = copy(oldPtr);
    oldPtr->SetForward(newObj);
    *ptr = (GcObject *)((int)newObj + offset);
  }
}

void BBStack::collect()
{
  GcObject *objPtr;
  int i;
  
  if (!ToCollect && current < chunkNum * 0.8)
    return;
  
#ifdef GC_VERBOSE
  cerr << "BBStack Collector:\n\tfrom: C[0-" << current << "]\n";
#endif
  
#if !HEADER_SIZE
  /* Clear the LiveMap bitmap */
  for (i = 0; i <= current; i++)
    chunk[i]->resetLiveMap();
#endif

  // Expand is used to start with a new Container which will
  // be the first one of the ToSpace.
  // A simple increment of current is not enough when the stack needs
  // to be expanded.
  
  expand();
  int toSpaceIndex = current;
  
  roots.scan(this);
  
  for (i = toSpaceIndex; i <= current; i++) {
    Container *container = chunk[i];
    
    objPtr = container->bottom();
    
    SET_OBJECTMAP(container->current()); // ensure that we stop at the end
    while (objPtr < container->current()) {
      objPtr->traverse();
      objPtr = objPtr->next();
    }
  }
  
#ifdef GC_VERBOSE
  cerr << "\t  to: C[" << toSpaceIndex << "-" << top << "]\n"; 
#endif
  
  for (i = 0; i <= current - toSpaceIndex; i++)
    swap(chunk+i, chunk+toSpaceIndex+i);
  
  for (; i <= current; i++)
    chunk[i]->reset();

  current -= toSpaceIndex;

  ToCollect = false;		// It's better at the end, because expand 
				// could reset it to true.
}

void BBStack::reset()
{
  // It resets the ObjectMap bitmap. In fact collect is
  // expected to coexist with WeakReset. Use WeakReset
  // elsewhere.
  
#ifdef GC_VERBOSE
  cerr << "Resetting BBStack: C[0-" << current << "]\n";
#endif
  
  for (int i = 0; i <= current; i++)
    chunk[i]->reset();
  
  current = 0;
}

void BBStack::WeakReset()
{
  // It doesn't reset the ObjectMap bitmap. In fact collect is not
  // expected to coexist with WeakReset. And ObjectMap is only used
  // by collect.
  
#ifdef GC_VERBOSE
  cerr << "Weak Resetting BBStack: C[0-" << current << "]\n";
#endif
  
  for (int i = 0; i <= current; i++)
    chunk[i]->WeakReset();
  
  current = 0;
}
