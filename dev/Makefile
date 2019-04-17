# ！bash
OBJDIR = ../obj
INCLUDE = ../include
TARGET = $(OBJDIR)/BufferManager.o $(OBJDIR)/DiskDriver.o
CFLAGS = -I"$(INCLUDE)" -std=c++11 -c
CC = g++
RM = -rm -f

all : $(TARGET)

$(OBJDIR)/BufferManager.o : BufferManager.cpp
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/DiskDriver.o	:	DiskDriver.cpp
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	$(RM) $(TARGET)
