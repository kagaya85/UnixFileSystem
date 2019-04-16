# ！bash
OBJDIR = ../obj
INCLUDE = ../includ
TARGET = $(OBJDIR)/File.o $(OBJDIR)/FileManager.o $(OBJDIR)/FileSystem.o \
	$(OBJDIR)/INode.o $(OBJDIR)/SecondFS.o
CFLAGS = -I"$(INCLUDE)" -std=c++11 -c
CC = g++
RM = -rm -f

all : $(TARGET)

$(OBJDIR)/File.o	:	File.cpp
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/FileManager.o	:	FileManager.cpp
	$(CC) $(CFLAGS) $< -o $@
	
$(OBJDIR)/FileSystem.o	:	FileSystem.cpp
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/INode.o	:	INode.cpp
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/SecondFS.o	:	SecondFS.cpp
	$(CC) $(CFLAGS) $< -o $@


.PHONY: clean
clean:
	$(RM) $(TARGET)
