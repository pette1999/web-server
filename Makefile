# A simple Makefile for CPSC 380

#this Makefile goes in the src directory

#change this to the correct assignment #
EXECUTABLE1 := myserver -lpthread
EXECUTABLE2 := myclient

# the source files to be built
SOURCES1 := myserver.c
SOURCES2 := myclient.c

#stuff you don't need to worry about
INCLUDES := -I ../include
EXT := out
CC := gcc

all:
	$(CC) $(INCLUDES) $(SOURCES1) -o $(EXECUTABLE1)
	$(CC) $(INCLUDES) $(SOURCES2) -o $(EXECUTABLE2)

realclean:
	find . -type f -name "*.o" -exec rm '{}' \;

# this line required by make - don't delete