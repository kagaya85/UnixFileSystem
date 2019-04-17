# ！bash
TARGET = format
OBJDIR = ../obj
OBJ = $(OBJDIR)/Format.o
INCLUDE = ../include
CFLAGS = -I"$(INCLUDE)" -std=c++11
CC = g++
RM = -rm -f

all : $(TARGET)

$(TARGET) : $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@
	mv format ../

$(OBJDIR)/Format.o : Format.cpp
	$(CC) $(CFLAGS) -c $< -o $@
	
.PHONY : clean
clean : 
	$(RM) $(OBJ)
