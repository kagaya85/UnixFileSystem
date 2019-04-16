# ！bash
TARGET = format SecondFS
OBJDIR = ./obj
INCLUDE = ./includ
CFLAGS = 
CC = g++
RM = -rm -f
SUBDIRS = dev fs kernel

all : $(TARGET) subDirs

subDirs : $(SUBDIRS)
	for dir in $(SUBDIRS);\
	do make -C $$dir all||exit 1;\
	done

format :  
	make -C format

SecondFS : $(OBJDIR)/SecondFS.o $(OBJDIR)BufferManager.o $(OBJDIR)/DiskDriver.o \
	$(OBJDIR)File.o $(OBJDIR)/FileManager.o $(OBJDIR)/FileSystem.o \
	$(OBJDIR)/INode.o $(OBJDIR)/SecondFS.o \
	$(OBJDIR)Kernel.o $(OBJDIR)/User.o $(OBJDIR)/Utility.o
	$(CC) $(CFLAGS) $^ -o $@

.PHONY : clean
clean : 
	$(RM) $(TARGET) $(OBJDIR)/*