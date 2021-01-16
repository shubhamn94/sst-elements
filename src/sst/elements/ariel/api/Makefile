all: libarielapi.so libarielapi.a

#all: mlm.o 
libarielapi.so: arielapi.c arielapi.h
	$(CC) -fPIC -shared -o libarielapi.so arielapi.c

arielapi.o: arielapi.c arielapi.h
	$(CC) -c -o arielapi.o arielapi.c

libarielapi.a: arielapi.o
	ar rcs $@ $^

#CXX=g++

#libmlm.so: mlm.o
#	g++ -shared -O3 -g -fPIC -o libmlm.so mlm.o

#mlm.o: mlm.c mlm.h
#	g++ -O3 -c -o mlm.o -I./ -fPIC mlm.c

#clean:
#	rm libmlm.so mlm.o
