# ---------------------------------------------------------------------------
VERSION = BCB.01
# ---------------------------------------------------------------------------
!ifndef BCB
BCB = $(MAKEDIR)\..
!endif
# ---------------------------------------------------------------------------
PROJECT = MacroView.dll
OBJFILES = MacroView.obj
RESFILES = MacroView.res
RESDEPEN = $(RESFILES)
LIBFILES =
DEFFILE =
# ---------------------------------------------------------------------------
#CFLAG1 = -WD -O1 -O-S -v -d -k- -r -vi -c -b- -Vx -Ve -x
CFLAG1 = -WD -O2 -O-S -v -d -k- -r -vi -c -b- -Vx -Ve -x
CFLAG2 = -I$(BCB)\include;$(BCB)\include\vcl 
PFLAGS = -AWinTypes=Windows;WinProcs=Windows;DbiTypes=BDE;DbiProcs=BDE;DbiErrs=BDE \
   -U$(BCB)\lib\obj;$(BCB)\lib -I$(BCB)\include;$(BCB)\include\vcl -H -$W -$L- \
   -$D- -JPHNV -M
RFLAGS = -i$(BCB)\include;$(BCB)\include\vcl 
LFLAGS = -L$(BCB)\lib\obj;$(BCB)\lib -aa -Tpd -v -s -n -V4.0
IFLAGS = -g 
LINKER = ilink32
# ---------------------------------------------------------------------------
ALLOBJ = c0d32.obj $(OBJFILES)
ALLRES = $(RESFILES)
ALLLIB = $(LIBFILES) import32.lib cw32mt.lib
# ---------------------------------------------------------------------------
.autodepend

$(PROJECT): $(OBJFILES) $(RESDEPEN)
    $(BCB)\BIN\$(LINKER) @&&!
    $(LFLAGS) +
    $(ALLOBJ), +
    $(PROJECT),, +
    $(ALLLIB), +
    $(DEFFILE), +
    $(ALLRES) 
!

.pas.hpp:
    $(BCB)\BIN\dcc32 $(PFLAGS) { $** }

.pas.obj:
    $(BCB)\BIN\dcc32 $(PFLAGS) { $** }

.cpp.obj:
    $(BCB)\BIN\bcc32 $(CFLAG1) $(CFLAG2) -o$* $* 

.c.obj:
    $(BCB)\BIN\bcc32 $(CFLAG1) $(CFLAG2) -o$* $**

.rc.res:
    $(BCB)\BIN\brcc32 $(RFLAGS) $<
#-----------------------------------------------------------------------------
