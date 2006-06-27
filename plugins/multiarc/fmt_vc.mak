# nmake fmt_vc.mak FMT=name
!if "$(FMT)" == ""
!error An invalid FMT-target
!endif

!if "$(OS)" == "Windows_NT"
NULL=
!else
NULL=nul
!endif


#COMP=BC

!if "$(COMP)" == ""
COMP=VC
INTDIR=obj
!else
COMP=BC
INTDIR=obj
!endif

OUTDIR=Final\Formats
CODDIR=obj\cod
SRCDIR=.

INCLUDE = $(INCLUDE);..\common;.\libpcre
LIB = $(LIB);$(INTDIR)


FMT_FILE=$(OUTDIR)\$(FMT).fmt
MAP_FILE=$(INTDIR)\$(FMT).map
LIB_FILE=$(INTDIR)\$(FMT).lib
OBJ_FILE=$(INTDIR)\$(FMT).obj
DEF_FILE=$(SRCDIR)\$(FMT).def
CPP_FILE=$(SRCDIR)\$(FMT).cpp
RC_FILE=$(SRCDIR)\$(FMT).rc
RES_FILE=$(INTDIR)\$(FMT).res

!if "$(COMP)" == "VC"
CC=cl.exe
#CPPFLAGS=/nologo /Fo"$(INTDIR)\\" /Fa"$(CODDIR)\\" /FD /Zp1 /ML /GX- /Ox /Os /Og /GF /Gr /GR- /LD  -c /D "WIN32" /D "NDEBUG" /opt:nowin98
CPPFLAGS =/nologo /Gs /Ox /Zp1 /GR- /GX- /c /MD /opt:nowin98 /Fa"$(CODDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /D "WIN32" /D "NDEBUG"
LINK=link.exe
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo$(RES_FILE) /d "NDEBUG"
LINK_FLAGS=/opt:nowin98 /noentry /DLL /map:$(MAP_FILE) /nologo /def:"$(DEF_FILE)" /stub:..\common\minstub.exe kernel32.LIB user32.LIB pcre.lib /out:$(FMT_FILE) /implib:$(LIB_FILE) /machine:I386 /release
!else
CC=bcc32.exe
CPPFLAGS=-4 -I$(FARINC) -c -a1 -R- -x- -RT- -K -M- -v- -u -G -o$(INTDIR)\$(FMT)
LINK=tlink32
LINK_FLAGS=-OS -Tpd -aa -V4.0 -m -c -v- -P -s -L$(FARLIB) $(OBJ_FILE),$(FMT_FILE),$(MAP_FILE),import32
!endif


ALL : $(FMT_FILE)


CLEAN :
	-@erase "$(INTDIR)\$(FMT).*" > nul
	-@erase "$(OUTDIR)\$(FMT).*" > nul
	-@erase "$(PLUGDOC_SRC)\$(FMT).*" > nul



$(OUTDIR) :
    @if not exist "$(OUTDIR)/$(NULL)"  mkdir "$(OUTDIR)"
    @if not exist "$(INTDIR)/$(NULL)"  mkdir "$(INTDIR)"
    @if not exist "$(CODDIR)/$(NULL)"  mkdir "$(CODDIR)"

$(INTDIR) :
    @if not exist "$(OUTDIR)/$(NULL)"  mkdir "$(OUTDIR)"
    @if not exist "$(INTDIR)/$(NULL)"  mkdir "$(INTDIR)"
    @if not exist "$(CODDIR)/$(NULL)"  mkdir "$(CODDIR)"

$(CODDIR) :
    @if not exist "$(OUTDIR)/$(NULL)"  mkdir "$(OUTDIR)"
    @if not exist "$(INTDIR)/$(NULL)"  mkdir "$(INTDIR)"
    @if not exist "$(CODDIR)/$(NULL)"  mkdir "$(CODDIR)"


.c{$(INTDIR)}.obj::
   @$(CC) @<<
  $(CPPFLAGS)  $<
<<


.cpp{$(INTDIR)}.obj::
   @$(CC) @<<
  $(CPPFLAGS)  $<
<<


$(RES_FILE) : $(RC_FILE) "$(INTDIR)" ../common/farversion.hpp multiarcversion.hpp
	$(RSC) $(RSC_PROJ) $(RC_FILE)


$(CPP_FILE) : fmt.hpp {$(INCLUDE)}plugin.hpp $(CODDIR)


!if "$(COMP)" == "VC"
$(FMT_FILE) : "$(OUTDIR)" $(DEF_FILE) $(OBJ_FILE) $(RES_FILE)
    @$(LINK) @<<
  $(LINK_FLAGS) $(OBJ_FILE) $(RES_FILE)
<<

!else
$(FMT_FILE) : "$(OUTDIR)" $(DEF_FILE) $(OBJ_FILE)  $(RES_FILE)
    @$(LINK) @<<
  $(LINK_FLAGS) -o$(OBJ_FILE)
<<

!endif
!IF "$(FMT)" == "Custom"
     -@copy $(SRCDIR)\$(FMT).ini "$(OUTDIR)\$(FMT).ini" > nul
!ENDIF
