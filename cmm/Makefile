TOP = ../..
include ../../Makefile.include
HFILES = machine.H cmm.H HeapStack.H
OBJS = 	cmm.o HeapStack.o
all: libCmm.a
TESTS = test1 test2 test3 test4 test5 test6
all: libCmm.a
library:
	@echo Making nothing.....
	@echo "Sorry, your Posso library is not in the expected place ;}."
test : ${TESTS}
	 @for i in ${TESTS}; do  \
	   ( echo "Running " $$i "..."; \
	     $$i >tests.out 2>&1 ;  \
	   )                    \
	done
demo: demo1 demo2
libCmm.a: $(OBJS) ${ASFILE}
	ar ru libCmm.a $(OBJS) ${ASMBO}
	ranlib libCmm.a
HeapStack.o: HeapStack.C $(HFILES)
	$(CC) $(CFLAGS) -c HeapStack.C
cmm.o: cmm.C machine.H cmm.H
	$(CC) $(CFLAGS) -c cmm.C
stamp:
	sed -e s:'"CMM .*"':\\\"CMM `date +%D`\\\": cmm.C > tmp.C
	mv tmp.C cmm.C
test1:	test1.C libCmm.a
	${CC} -o test1 ${CFLAGS} test1.C libCmm.a
test2:	test2.C libCmm.a
	${CC} -o test2 ${CFLAGS} test2.C libCmm.a
test3:	test3.C libCmm.a
	${CC} -o test3 ${CFLAGS} test3.C libCmm.a
test4:	test4.C libCmm.a
	${CC} -o test4 ${CFLAGS} test4.C libCmm.a 
test5:	test5.C libCmm.a
	${CC} -o test5 ${CFLAGS} test5.C libCmm.a 
test6:	test6.C libCmm.a
	${CC} -o test6 ${CFLAGS} test6.C libCmm.a 
test7:	test7.C libCmm.a
	${CC} -o test7 ${CFLAGS} test7.C libCmm.a 
demo1:	demo1.C libCmm.a 
	${CC} -o demo1 ${CFLAGS} demo1.C libCmm.a 
demo2:	demo2.C libCmm.a 
	${CC} -o demo2 ${CFLAGS} demo2.C libCmm.a 
tar:
	mv Makefile.dist makefile
	cd ..
	tar cf cmm-1.0.tar cmm/{Copy*,README,*.H,*.C,makefile,doc}
	mv makefile Makefile.dist
tidy:
	$(RM) *.log *.aux
clean:
	/bin/rm -rf *.o $(TESTS) demo? libCmm.a
.Makefile:
	set DIR=`pwd`; cd $(TOP); make SUBDIRS=$$DIR Makefiles
