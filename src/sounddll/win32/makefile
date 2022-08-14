.SUFFIXES: .c .cpp .obj
PROJECTNAME=sndYsflight
SRC_ROOT=.

CC=cl
RM=del





#
# BEGIN YS Standard NMAKE MAKEFILE HEADER 20131211 >>
#
# Variable PROJECTNAME must be defined before the header.
#
!IFNDEF PROJECTNAME
!ERROR PROJECTNAME not defined.
!ENDIF

!IFNDEF SRC_ROOT
!ERROR SRC_ROOT not defined.
!ENDIF

CC=cl
RM=del
MD=mkdir
CP=copy


!IFNDEF BITNESSARCH

BITNESSUTIL=ys_temp_util_sizeofpointer
BITNESSUTILCPP=$(BITNESSUTIL).cpp
BITNESSUTILEXE=$(BITNESSUTIL).exe

!IF [@echo int main(void){return sizeof(void *);} > $(BITNESSUTILCPP)]==0
!MESSAGE Created $(BITNESSUTILCPP)
!ELSE
!ERROR Cannot create $(BITNESSUTILCPP)
!ENDIF

!IF [@cl.exe /nologo $(BITNESSUTILCPP)]==0
!MESSAGE Compiled $(BITNESSUTILCPP)
!ELSE
!ERROR Cannot compile bitness identifier!
!ENDIF

!IF [$(BITNESSUTILEXE)]==4
!MESSAGE Detected 32-bit compiler
BITNESS=32
BITNESSARCH=win32
!ELSEIF [$(BITNESSUTILEXE)]==8
!MESSAGE Detected 64-bit compiler
BITNESS=64
BITNESSARCH=win64
!ELSE
!ERROR Cannot identify bitness
!ENDIF

!IF [del $(BITNESSUTIL).*]==0
!ENDIF

!ENDIF
#Closing IFNDEF BITNESSARCH



!IF "$(BITNESSARCH)"=="win32"
SYSTEMID=5.01
!ELSEIF "$(BITNESSARCH)"=="win64"
SYSTEMID=5.02
!ELSE
!ERROR Unknown bitness-architecture.
!ENDIF



!IFDEF USERLIBDIR
!MESSAGE User Library Directory=$(USERLIBDIR)
!ELSE
!MESSAGE Environment variable USERLIBDIR is not defined.  Default to C:\lib
USERLIBDIR=C:\lib
!ENDIF

!IFDEF USERINCDIR
!MESSAGE User Include Directory=$(USERINCDIR)
!ELSE
!MESSAGE Environment variable USERINCDIR is not defined.  Default to C:\include
USERINCDIR=C:\include
!ENDIF



!IFDEF USEROBJDIR
!MESSAGE User Object Directory=$(USEROBJDIR)
!ELSE
!MESSAGE Environment variable USEROBJDIR is not defined.  Default to C:\tmp\obj
USEROBJDIR=C:\tmp\obj
!ENDIF



OBJDIR=$(USEROBJDIR)\$(BITNESSARCH)\$(PROJECTNAME)
LIBDIR=$(USERLIBDIR)\$(BITNESSARCH)
INCDIR=$(USERINCDIR)
BINDIR=.\bin\$(BITNESSARCH)
PDBFILE=$(OBJDIR)\$(PROJECTNAME).pdb
PCHFILE=$(OBJDIR)\$(PROJECTNAME).pch
#
# END YS Standard NMAKE MAKEFILE HEADER <<
#



# Architecture (/MT, /MD, /MTD, etc.) needs to match with the library.  Use this setting.
CFLAGS=/nologo /c /W4 /Zi /DWIN32 /O2 /MT /Fd$(PDBFILE) /Fp$(PCHFILE) /wd4996



#Per source
#$(OBJDIR)\.obj : .cpp
#	$(CC) $? $(CFLAGS) /Fo$@





TARGET=$(PROJECTNAME)$(BITNESS).dll

OBJDIR=$(USEROBJDIR)\$(BITNESSARCH)\$(PROJECTNAME)

OBJ=$(OBJDIR)\fsairsounddll.obj
PDB=$(OBJDIR)\fsairsounddll.pdb
PCH=$(OBJDIR)\fsairsounddll.pch


LINKOPT=user32.lib winmm.lib kernel32.lib dsound.lib /link /DLL /out:$(OBJDIR)\$(TARGET)

LINKSYS=/SUBSYSTEM:WINDOWS,$(SYSTEMID)

CFLAGS=/nologo /W4 /MT /Zi /DWIN32 /DYSFLIGHT /c /O2 /I. /I.. /Fd$(PDB) /Fp$(PCH) /wd4996


$(TARGET) : $(OBJDIR) makefile $(OBJ)
	cl $(OBJ) $(LINKOPT) $(LINKSYS)
	copy $(OBJDIR)\$(TARGET) .

$(OBJDIR) :
	md $(OBJDIR)

cln:
	del $(OBJDIR)\$(TARGET)
	del $(TARGET)
	del $(OBJ)
	del $(PDB)
	del $(PCH)

$(OBJDIR)\fsairsounddll.obj : fsairsounddll_dsound.cpp
	$(CC) $? $(CFLAGS) /Fo$@

