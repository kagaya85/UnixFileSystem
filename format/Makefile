# ！bash
TARGET = format
OBJDIR = ../obj
OBJ = $(OBJDIR)/format.o $(OBJDIR)/inode.o
INCLUDE = ../include
CFLAGS = -I"$(INCLUDE)"
CC = g++
RM = -rm -f
FLAGS = -std=c++11

all : $(TARGET)

$(TARGET) : $(OBJ)
	$(CC) $(FLAGS) $^ -o $@ 

$(OBJDIR)/format.o : format.cpp
	$(CC) $(CFLAGS) -c $< -o $@
	
$(OBJDIR)/inode.o : ../fs/INode.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY : clean
clean : 
	$(RM) $(OBJ)