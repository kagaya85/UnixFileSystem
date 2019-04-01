# ！bash
TARGET = format.o
OBJDIR = ../obj
INCLUDE = ../includ
CFLAGS = 
CC = g++
RM = -rm -f

all : $(TARGET)

$(OBJDIR)/format.o :  format.cpp $(INCLUDE)/format.h $(INCLUDE)/FileSystem.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@
	