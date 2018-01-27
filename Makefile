WORKROOT=..

###########################################################

AR   = ar
CC   = gcc
CPP  = g++
PERL = perl
PY   = python

###########################################################

CPPFLAGS = -I ./ -I ./include

LDLIBS   = -L. -lpthread  -lz

CXXFLAGS = -g -W -Wall -Wno-unused-parameter -pipe 

###########################################################

SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp,%.o,$(SRC))
BIN = minify-php

.PHONY: all clean

all : $(BIN)
	rm -rf output
	mkdir -p output
	cp $^ output/
	rm -f *.o

minify-php : $(OBJ)
	$(CPP) -o $@ $^ $(LDLIBS) 

%.o : %.cpp
	$(CPP) -c $(CXXFLAGS) $(CPPFLAGS) -o $@ $^ $(INCLUDES) 

clean : 
	rm -f $(BIN) *.o *.pyc
	rm -rf output
