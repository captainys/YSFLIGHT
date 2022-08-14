.SUFFIXES: .c .cpp .obj
PROJECTNAME=sndYsflight
SRC_ROOT=.
IS_LIBRARY_PROJECT=0

#
# YS Standard Makefile Header for Unix and MacOSX 20131107 >>
#
# Variable PROJECTNAME and SRC_ROOT must be defined before the header.
#

# Self-memo: $* $> $@ etc are officially (?) called 'automatic variables'
# Self-memo: $(lastword $(MAKEFILE_LIST)) will give the current makefile name


# Identifying compiler >>
# Priority is given to clang over g++
TESTCLANG:=$(shell which clang 1> /dev/null; echo $$?)
TESTGPP:=$(shell which g++ 1> /dev/null; echo $$?)

ifeq ($(TESTCLANG),0)
CC=clang++
else ifeq ($(TESTGPP),0)
CC=g++
else
$(error C++ Compiler not found.)
endif
# Identifying compiler >>


RM=rm
MD=mkdir
CP=cp

BITNESSTESTSRC=ysbitnesstest.cpp
BITNESSTESTEXE=ysbitnesstest

ifndef PROJECTNAME
$(error Variable PROJECTNAME not defined.)
endif
ifndef SRC_ROOT
$(error Variable SRC_ROOT not defined.)
endif
ifndef IS_LIBRARY_PROJECT
$(error Variable IS_LIBRARY_PROJECT not defined.)
endif


ifndef YS_BITNESS
GENSRC:=$(shell echo 'int main(void){printf("%d",8*(int)sizeof(void *));return 0;} ' 1> $(BITNESSTESTSRC))
BITNESSCOMPILE:=$(shell g++ $(BITNESSTESTSRC) -o $(BITNESSTESTEXE) -include stdio.h)
SIZEOFPOINTER:=$(shell ./$(BITNESSTESTEXE))
YS_BITNESS:=$(SIZEOFPOINTER)
CLEANTEMPORARYCPP:=$(shell rm $(BITNESSTESTSRC) $(BITNESSTESTEXE))
endif

ifeq ($(YS_BITNESS),32)
  CBITNESSFLAG=-m32
else
  ifeq ($(YS_BITNESS),64)
    CBITNESSFLAG=-m64
  endif
endif

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
PLATFORM=linux
PLATFORMSHORT=unx
PLATFORMLIB=$(X11LIB) -lGLU -lGL
APPDIR=.
APPDIRSIGNATURE=
APPDIRSIGNATURERULE=dummyRule
OBJSIMPLEWIN=$(OBJDIR)/fsglxwrapper.o $(OBJDIR)/fsglxkeymap.o
endif

ifeq ($(UNAME), Darwin)
PLATFORM=macosx
PLATFORMSHORT=mac
PLATFORMLIB=-framework Cocoa -framework OpenGL
APPDIR=$(PROJECTNAME).app/Contents/MacOS
APPDIRSIGNATURE=$(APPDIR)/signature
APPDIRSIGNATURERULE=$(APPDIRSIGNATURE)
OBJSIMPLEWIN=$(OBJDIR)/fsmacosxwrapperobjc.o $(OBJDIR)/fsmacosxwrappercpp.o
ifeq ($(IS_LIBRARY_PROJECT),0)
MAKEBUNDLEDIR:=$(shell mkdir -p $(APPDIR))
endif
endif

ifndef YSBIN_ROOT
YSBIN_ROOT=$(HOME)/ysbin
endif

YSOBJ_ROOT=$(YSBIN_ROOT)/$(PLATFORM)$(YS_BITNESS)/obj
USERLIBDIR=$(YSBIN_ROOT)/$(PLATFORM)$(YS_BITNESS)/lib

OBJDIR=$(YSOBJ_ROOT)/$(PROJECTNAME)
LIBDIR=$(USERLIBDIR)
BINDIR=./bin/$(PLATFORM)$(YS_BITNESS)
MAKEFILE=$(lastword $(MAKEFILE_LIST))
#
# YS Standard Makefile Header for Unix <<
#


TARGET=$(PROJECTNAME)$(YS_BITNESS).so

OBJ=$(OBJDIR)/fsairsounddll.o

CFLAGS=-c -O2 -fPIC $(CBITNESSFLAG)

$(TARGET) : $(OBJDIR) $(OBJ)
	$(CC) -shared -lc -o $(TARGET) $(OBJ) -framework Cocoa $(CBITNESSFLAG)

$(OBJDIR) :
	mkdir -p $(OBJDIR)

cln :
	rm -f $(OBJ)
	rm -f $(TARGET)



$(OBJDIR)/fsairsounddll.o : fsairsounddll.m
	$(CC) $? $(CFLAGS) -o $@


