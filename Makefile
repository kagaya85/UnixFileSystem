# ！bash
TARGET = format
OBJDIR = ./obj
INCLUDE = ./includ
CFLAGS = 
CC = g++
RM = -rm -f
SUBDIRS = format fs

all : $(TARGET) subDirs

subDirs : $(SUBDIRS)
	for dir in $(SUBDIRS);\
	do make -C $$dir all||exit 1;\
	done

format :  $(OBJDIR)/format.o $(OBJDIR)/filesystem.o $(OBJDIR)/inode.o
	$(CC) $(CFLAGS) $^ -o $@

# SecondFS : $(OBJDIR)/SecondFS.o
# 	$(CC) $(CFLAGS) $^ -o $@

.PHONY : clean
clean : 
	$(RM) $(TARGET) $(OBJDIR)/*