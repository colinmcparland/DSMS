CCFLAGS = -I.
LDFLAGS = -lpthread -ldl

all: test1 test2

test1: test1.o sqlite3.o
	gcc -o test1 test1.o sqlite3.o $(LDFLAGS)

test2: test2.o sqlite3.o
	gcc -o test2 test2.o sqlite3.o $(LDFLAGS)

test1.o: test1.c
	gcc $(CCFLAGS) -c test1.c

test2.o: test2.c
	gcc $(CCFLAGS) -c test2.c

sqlite3.o: sqlite3.c sqlite3.h sqlite3ext.h
	gcc $(CCFLAGS) -c sqlite3.c

clean:
	-rm test1.o test2.o sqlite3.o

spotless: clean
	-rm test1 test2

