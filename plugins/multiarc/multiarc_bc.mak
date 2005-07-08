.AUTODEPEND

# берутся из переменных среды (env)
LIBPATH=$(FARLIB)
INCLUDEPATH=$(FARINC)
BCCPATH=$(FARBCC)

# сюда будет помещен результат
FINALPATH=Final

# путь к каталогу с временными файлами - obj, etc
!ifdef DEBUG
OBJPATH=OBJD
!else
OBJPATH=OBJ
!endif

.path.obj = $(OBJPATH)
.path.cpp = .
.path.c   = .
.path.exe = $(FINALPATH)

!ifdef DEBUG
OPTDEBUG=-v -R -y
OPTDEBUG2=
CSMFILE=MultiArc.csm
OPTLINKDEBUG=-v
MA_STDHDR_OBJ="$(OBJPATH)\MultiArc.#00"
!else
OPTDEBUG=-v- -R- -y-
OPTDEBUG2=-DNDEBUG
CSMFILE=MultiArc.csm
OPTLINKDEBUG=-v-
MA_STDHDR_OBJ=
!endif

!ifdef PRECOMP
PRECOMPOPT=-H -H=$(OBJPATH)\$(CSMFILE)
!else
PRECOMPOPT=-H-
!endif


BCC32   = $(BCCPATH)\bin\Bcc32 +BccW32.cfg
!ifdef ILINK
TLINK32 = $(BCCPATH)\bin\ilink32
BRC32   = $(BCCPATH)\bin\Brcc32
!else
TLINK32 = $(BCCPATH)\bin\Tlink32
BRC32   = $(BCCPATH)\bin\Brc32
!endif


!ifdef ILINK
LINKFLAGS =  -L$(LIBPATH) -OS -Tpd -aa -m -P -c $(OPTLINKDEBUG) -s -V4.0 -j.\$(OBJPATH) -Gz
!else
LINKFLAGS =  -L$(LIBPATH) -OS -Tpd -aa -m -P -c $(OPTLINKDEBUG) -s -V4.0 -j.\$(OBJPATH) -n
!endif

RESFLAGS = -i$(INCLUDEPATH)


MAOBJ=\
	$(OBJPATH)\arcreg.obj\
	$(OBJPATH)\arcread.obj\
	$(OBJPATH)\arcput.obj\
	$(OBJPATH)\arcproc.obj\
	$(OBJPATH)\arcplg.obj\
	$(OBJPATH)\arcmix.obj\
	$(OBJPATH)\arcget.obj\
	$(OBJPATH)\arccmd.obj\
	$(OBJPATH)\arccfg.obj\
	$(OBJPATH)\global.obj\
	$(OBJPATH)\ownrtl.obj\
	$(OBJPATH)\multiarc.obj


# ************************************************************************
ALL : BccW32.cfg $(FINALPATH)\MULTIARC.DLL $(FINALPATH)\Formats\Custom.fmt $(FINALPATH)\Formats\Ace.fmt $(FINALPATH)\Formats\Arj.fmt $(FINALPATH)\Formats\Cab.fmt $(FINALPATH)\Formats\Ha.fmt $(FINALPATH)\Formats\Lzh.fmt $(FINALPATH)\Formats\Rar.fmt $(FINALPATH)\Formats\TarGz.fmt $(FINALPATH)\Formats\Zip.fmt $(FINALPATH)\Formats\Arc.fmt
  @echo MakeNode

# все эти зависимости не нужны
# просто описываем одно правило и все %)
.cpp.obj:
  @if not exist $(OBJPATH) mkdir $(OBJPATH)
  -@settitle "{$.} - Compiling..."
  @$(BCC32) -c -o$@ {$. }

.c.obj:
  @if not exist $(OBJPATH) mkdir $(OBJPATH)
  -@settitle "{$.} - Compiling..."
  @$(BCC32) -c -o$@ {$. }

# уточнения
#$(OBJPATH)\syslog.obj: syslog.cpp cc.bat

# ************************************************************************
$(OBJPATH)\MultiArc.res :  MultiArc.rc
  @if not exist $(OBJPATH) mkdir $(OBJPATH)
  -@settitle "Compiling resource..."
  $(BRC32) -R @&&|
 $(RESFLAGS)  -FO$@ MultiArc.rc
|


#$(LIBPATH)\c0d32.obj $(MAOBJ) $(MA_STDHDR_OBJ)

$(FINALPATH)\MULTIARC.DLL : BccW32.cfg plugin.def $(OBJPATH)\MultiArc.res $(MAOBJ)
  -@settitle "Linking..."
  @if not exist $(FINALPATH) mkdir $(FINALPATH)
  @$(TLINK32)  $(LINKFLAGS) @&&|
$(MAOBJ) $(MA_STDHDR_OBJ)
$<,$*
$(LIBPATH)\import32.lib $(LIBPATH)\cw32.lib
plugin.def
|
   @copy MultiArc.map $(FINALPATH)\MultiArc.map
   @$(BRC32) $(OBJPATH)\MultiArc.res $(OBJPATH)\MultiArc.res $<

# обязательно! Что бы в ручную не делать...
   @copy  ArcEng.lng   $(FINALPATH)\ArcEng.lng > nul
   @copy  ArcRus.lng   $(FINALPATH)\ArcRus.lng > nul
   -@awk -f mkhlf.awk -v FV1=$(FV1) -v FV2=$(FV2) -v FV3=$(FV3) ArcEng.hlf > $(FINALPATH)\ArcEng.hlf
   -@awk -f mkhlf.awk -v FV1=$(FV1) -v FV2=$(FV2) -v FV3=$(FV3) ArcRus.hlf > $(FINALPATH)\ArcRus.hlf


$(FINALPATH)\Formats\Ace.fmt :  Ace.cpp fmt.hpp Ace.def Ace.rc
	nmake /c fmt_vc.mak FMT=Ace


$(FINALPATH)\Formats\Arc.fmt :  Arc.cpp fmt.hpp Arc.def Arc.rc
	nmake /c fmt_vc.mak FMT=Arc


$(FINALPATH)\Formats\Arj.fmt :  Arj.cpp fmt.hpp Arj.def Arj.rc
	nmake /c fmt_vc.mak FMT=Arj


$(FINALPATH)\Formats\Cab.fmt :  Cab.cpp fmt.hpp Cab.def Cab.rc
	nmake /c fmt_vc.mak FMT=Cab


$(FINALPATH)\Formats\Custom.fmt :  Custom.cpp fmt.hpp Custom.def Custom.rc
	nmake /c fmt_vc.mak FMT=Custom


$(FINALPATH)\Formats\Ha.fmt :  Ha.cpp fmt.hpp Ha.def Ha.rc
	nmake /c fmt_vc.mak FMT=Ha


$(FINALPATH)\Formats\Lzh.fmt :  Lzh.cpp fmt.hpp Lzh.def Lzh.rc
	nmake /c fmt_vc.mak FMT=Lzh


$(FINALPATH)\Formats\Rar.fmt :  Rar.cpp fmt.hpp Rar.def Rar.rc
	nmake /c fmt_vc.mak FMT=Rar


$(FINALPATH)\Formats\TarGz.fmt :  TarGz.cpp fmt.hpp TarGz.def TarGz.rc
	nmake /c fmt_vc.mak FMT=TarGz


$(FINALPATH)\Formats\Zip.fmt :  Zip.cpp fmt.hpp Zip.def Zip.rc
	nmake /c fmt_vc.mak FMT=Zip


# Compiler configuration file
BccW32.cfg :
   Copy &&|
-D__USE_OWN_RTL__
-I$(INCLUDEPATH)
$(OPTDEBUG)
$(OPTDEBUG2)
$(PRECOMPOPT)
-a1
-x-
-RT-
-K
-M-
-G
-O-d
-O
-Ob
-Oe
-Os
-Ol
-Oc
| $@
