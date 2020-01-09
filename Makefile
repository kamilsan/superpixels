CC=g++
CFLAGS=-std=c++11 -O3 -Wall -Wextra -pedantic

superpixels: main.o
	$(CC) $(CFLAGS) main.o -o superpixels

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

clean:
	rm -rf *.o superpixels