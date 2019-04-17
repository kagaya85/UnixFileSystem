# ！bash
OBJDIR = ../obj
INCLUDE = ../include
TARGET = $(OBJDIR)/Kernel.o $(OBJDIR)/User.o $(OBJDIR)/Utility.o
CFLAGS = -I"$(INCLUDE)" -std=c++11 -c
CC = g++
RM = -rm -f

all : $(TARGET)

$(OBJDIR)/Kernel.o : Kernel.cpp
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/User.o	:	User.cpp
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/Utility.o	:	Utility.cpp
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	$(RM) $(TARGET)
