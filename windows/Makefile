###########################################
# Simple Makefile for HIDAPI test program
#
# Alan Ott
# Signal 11 Software
# 2010-06-01
###########################################

all: aboot libhidapi.dll

CC=i686-w64-mingw32-gcc
CXX=i686-w64-mingw32-g++
LD=i686-w64-mingw32-ld
COBJS=hid.o
CPPOBJS=aboot.o
OBJS=$(COBJS) $(CPPOBJS)
CFLAGS=-I../hidapi -g -c
LIBS= -lsetupapi
DLL_LDFLAGS =  -lsetupapi

aboot : $(OBJS)
	$(CC) -I../hidapi $^ $(LIBS) -o aboot.exe

libhidapi.dll: $(OBJS)
	$(CC) $^ $(DLL_LDFLAGS) -o libhidapi.dll

$(COBJS): %.o: %.c
	$(CC) $(CFLAGS) $< -o $@

$(CPPOBJS): %.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o *.dll aboot.exe

.PHONY: clean
