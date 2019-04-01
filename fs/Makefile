# ！bash
TARGET = SecondFS.o
OBJDIR = ../obj
INCLUDE = ../includ
CFLAGS = 
CC = g++

all : $(OBJDIR)/SecondFS.o  $(OBJDIR)/inode.o $(OBJDIR)/filesystem.o \
			$(OBJDIR)/file.o $(OBJDIR)/filemanager.o

$(OBJDIR)/SecondFS.o : SecondFS.cpp $(INCLUDE)/SecondFS.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@

$(OBJDIR)/filesystem.o	:	FileSystem.cpp $(INCLUDE)/FileSystem.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@
	
$(OBJDIR)/openfilemanager.o	:	OpenFileManager.cpp $(INCLUDE)/OpenFileManager.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@
	
$(OBJDIR)/inode.o	:	INode.cpp $(INCLUDE)/INode.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@
	
$(OBJDIR)/file.o	:	File.cpp $(INCLUDE)/File.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@

$(OBJDIR)/filemanager.o	:	FileManager.cpp $(INCLUDE)/FileManager.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@	