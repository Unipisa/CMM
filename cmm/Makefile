#
#  Copyright (C) 1993 Tito Flagella.
#
#  This file is part of the POSSO Customizable Memory Manager (CMM).
#
#  Permission to use, copy, and modify this software and its documentation is
#  hereby granted only under the following terms and conditions.  Both the
#  above copyright notice and this permission notice must appear in all copies
#  of the software, derivative works or modified versions, and any portions
#  thereof, and both notices must appear in supporting documentation.
# 
#  Users of this software agree to the terms and conditions set forth herein,
#  and agree to license at no charge to all parties under these terms and
#  conditions any derivative works or modified versions of this software.
#  
#  This software may be distributed (but not offered for sale or transferred
#  for compensation) to third parties, provided such third parties agree to
#  abide by the terms and conditions of this notice.  
#  
#  THE SOFTWARE IS PROVIDED "AS IS" AND THE COPYRIGHT HOLDERS DISCLAIM ALL
#  WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
#  MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL THE COPYRIGHT HOLDERS
#  BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR
#  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
#  IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
#  OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
#
#  USER GUIDE
#  The most useful targets in this make file are:
#  "test"  compiles and runs all tests (listed in TESTS) reporting which
#        ones succeeded and which failed.
#  "clean"  removes all object code from this directory.

CFLAGS	= -O
CC	= g++

HFILES = machine.h cmm.h

OBJS = 	cmm.o tempheap.o msw.o memory.o

TESTS = test1 test2 test3 test4 test5 test6 test7 test8 test9 \
	testmsw1 testmsw2 testmsw3

all: libCmm.a

debug:
	make CFLAGS="-UNDEBUG -g" all

test : ${TESTS}
	@echo > tests.out
	@for t in ${TESTS}; do  \
	   echo "Running " $$t "..." >> tests.out ; \
	   ./$$t >> tests.out 2>&1 ;  \
	done

libCmm.a: $(OBJS)
	ar cr libCmm.a $(OBJS)

cmm.o: cmm.cpp $(HFILES)
	$(CC) $(CFLAGS) -c cmm.cpp

memory.o: memory.cpp $(HFILES)
	$(CC) $(CFLAGS) -c memory.cpp

tempheap.o: tempheap.cpp tempheap.h $(HFILES)
	$(CC) $(CFLAGS) -c tempheap.cpp

msw.o: msw.cpp msw.h $(HFILES)
	$(CC) $(CFLAGS) -c msw.cpp

test1:	test1.cpp libCmm.a
	$(CC) $(CFLAGS) -o test1 test1.cpp libCmm.a
test2:	test2.cpp libCmm.a
	$(CC) $(CFLAGS) -o test2 test2.cpp libCmm.a
test3:	test3.cpp libCmm.a
	$(CC) $(CFLAGS) -o test3 test3.cpp libCmm.a
test4:	test4.cpp libCmm.a
	$(CC) $(CFLAGS) -o test4 test4.cpp libCmm.a 
test5:	test5.cpp libCmm.a
	$(CC) $(CFLAGS) -o test5 test5.cpp libCmm.a 
test6:	test6.cpp libCmm.a
	$(CC) $(CFLAGS) -o test6 test6.cpp libCmm.a 
test7:	test7.cpp libCmm.a
	$(CC) $(CFLAGS) -o test7 test7.cpp libCmm.a 
test8:	test8.cpp libCmm.a
	$(CC) $(CFLAGS) -o test8 test8.cpp libCmm.a 
test9:	test9.cpp libCmm.a
	$(CC) $(CFLAGS) -o test9 test9.cpp libCmm.a 
testmsw1: testmsw1.c libCmm.a
	$(CC) $(CFLAGS) -g -o testmsw1 testmsw1.c libCmm.a
testmsw2: testmsw2.c libCmm.a
	$(CC) $(CFLAGS) -g -o testmsw2 testmsw2.c libCmm.a
testmsw3: testmsw3.cpp libCmm.a
	$(CC) $(CFLAGS) -o testmsw3 testmsw3.cpp libCmm.a
clean:
	/bin/rm -rf *.o libCmm.a $(TESTS) tests.out
