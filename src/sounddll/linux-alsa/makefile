.SUFFIXES: .c .cpp .obj
PROJECTNAME=sndYsflight


BITNESSTESTSRC=ysbitnesstest.cpp
BITNESSTESTEXE=ysbitnesstest

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
endif

ifeq ($(UNAME), Darwin)
PLATFORM=macosx
endif

ifndef YSBIN_ROOT
YSBIN_ROOT=$(HOME)/ysbin
endif

YSOBJ_ROOT=$(YSBIN_ROOT)/$(PLATFORM)$(YS_BITNESS)/obj
YSLIBDIR=$(YSBIN_ROOT)/$(PLATFORM)$(YS_BITNESS)/lib

OBJDIR=$(YSOBJ_ROOT)/$(PROJECTNAME)_alsa



OBJ=$(OBJDIR)/fsairsounddll.o $(OBJDIR)/yswavfile.o $(OBJDIR)/ysalsa.o

CC=g++

CFLAGS=-c -O2 -fPIC $(CBITNESSFLAG)

TARGET=$(PROJECTNAME)$(YS_BITNESS).so

$(TARGET) : $(OBJDIR) $(OBJ)
	$(CC) -shared -lc -o $(TARGET) $(OBJ) -lasound $(CBITNESSFLAG)

$(OBJDIR) :
	mkdir -p $(OBJDIR)

cln :
	rm -f $(OBJ)
	rm -f $(TARGET)


$(OBJDIR)/fsairsounddll.o : fsairsounddll.cpp
	$(CC) $? $(CFLAGS) -o $@

$(OBJDIR)/yswavfile.o : ../yswavfile/yswavfile.cpp
	$(CC) $? $(CFLAGS) -o $@

$(OBJDIR)/ysalsa.o : ysalsa.cpp
	$(CC) $? $(CFLAGS) -o $@


