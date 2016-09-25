mythread.a: mythread.o myqueue.o
	ar rcs mythread.a mythread.o myqueue.o 
	
mythread.o: mythread.c mythread.h myqueue.c myqueue.h
	gcc -c mythread.c myqueue.c

myqueue.o: myqueue.c myqueue.h
	gcc -c myqueue.c -o myqueue.o
	
clean: 
rm -f mythread.a *.o 
