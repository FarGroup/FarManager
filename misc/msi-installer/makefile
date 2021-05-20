.SILENT:

!include params.mak

!if defined(AMD64) || "$(CPU)" == "AMD64" || "$(PLATFORM)" == "X64" || "$(PLATFORM)" == "x64"
PLATFORM=x64
!else
PLATFORM=x86
!endif

!ifdef NO_LOCALISED_MSI
TRANSFORMS =
LANGUAGES=1033
!else
TRANSFORMS = $(OUTDIR)\1045 $(OUTDIR)\1049 $(OUTDIR)\1059
LANGUAGES=1033,1045,1049,1059
!endif

CPPFLAGS = -nologo -D_WIN32_WINNT=0x0500 -DUNICODE -D_UNICODE -D_CRT_SECURE_NO_WARNINGS /Zc:threadSafeInit- /W4
LINKFLAGS = -nologo -incremental:no -machine:$(PLATFORM)
CANDLEFLAGS = -nologo -dSourceDir="$(SOURCE_DIR)" -dPlatform=$(PLATFORM) -dVerMajor=$(VER_MAJOR) -dVerMinor=$(VER_MINOR) -dVerBuild=$(VER_BUILD) -dLanguages=$(LANGUAGES)
LIGHTFLAGS = -nologo -spdb -sval
ASFLAGS = -nologo -c

!if "$(PLATFORM)" == "x86"
CPPFLAGS=$(CPPFLAGS) /arch:IA32
!endif

!if "$(PLATFORM)" == "x86"
OS_VERSION=5.0
!else
OS_VERSION=5.2
!endif

!ifdef RELEASE
OUTDIR = Release
CPPFLAGS = $(CPPFLAGS) -O2 -DNDEBUG
LIGHTFLAGS = $(LIGHTFLAGS) -dcl:high
!else
OUTDIR = Debug
CPPFLAGS = $(CPPFLAGS) -Fd$(OUTDIR)\ -Zi
LINKFLAGS = $(LINKFLAGS) -debug
LIGHTFLAGS = $(LIGHTFLAGS) -cc $(OUTDIR) -reusecab
!endif

OUTDIR = $(OUTDIR).$(PLATFORM)
CPPFLAGS = -Fo$(OUTDIR)\ $(CPPFLAGS)
CANDLEFLAGS = -out $(OUTDIR)\ $(CANDLEFLAGS)
ASFLAGS = $(ASFLAGS) -Fo$(OUTDIR)\\

WIXOBJ = $(OUTDIR)\installer.wixobj $(OUTDIR)\files.wixobj $(OUTDIR)\features.wixobj $(OUTDIR)\shortcuts.wixobj $(OUTDIR)\ui.wixobj

!ifdef NIGHTLY
MSI_NAME = $(SOURCE_DIR)\final.msi
!else
MSI_NAME = $(OUTDIR)\far.msi
!endif

!if "$(PLATFORM)" == "x86"
VC10OBJ = $(OUTDIR)\vc_crt_fix.obj
VC10CPP = ..\..\plugins\common\vc_crt_fix_impl.cpp
!endif

$(MSI_NAME): $(OUTDIR) $(OUTDIR)\1033.msi $(TRANSFORMS)
	@echo $@
	copy /y $(OUTDIR)\1033.msi $(MSI_NAME)

$(OUTDIR)\1033.msi: $(WIXOBJ) en-us.wxl WixUI_en-us.wxl $(OUTDIR)\customact.dll
	@echo $@
	light $(LIGHTFLAGS) -cultures:en-us -loc en-us.wxl -loc WixUI_en-us.wxl -out $@ $(WIXOBJ)

$(OUTDIR)\1049.msi: $(WIXOBJ) ru-ru.wxl WixUI_ru-ru.wxl $(OUTDIR)\customact.dll
	@echo $@
	light $(LIGHTFLAGS) -cultures:ru-ru -loc ru-ru.wxl -loc WixUI_ru-ru.wxl -out $@ $(WIXOBJ)

$(OUTDIR)\1045.msi: $(WIXOBJ) pl-pl.wxl WixUI_pl-pl.wxl $(OUTDIR)\customact.dll
	@echo $@
	light $(LIGHTFLAGS) -cultures:pl-pl -loc pl-pl.wxl -loc WixUI_pl-pl.wxl -out $@ $(WIXOBJ)

$(OUTDIR)\1059.msi: $(WIXOBJ) be-be.wxl WixUI_be-be.wxl $(OUTDIR)\customact.dll
	@echo $@
	light $(LIGHTFLAGS) -cultures:be-be -loc be-be.wxl -loc WixUI_be-be.wxl -out $@ $(WIXOBJ)

$(OUTDIR)\1049: $(OUTDIR)\1033.msi $(OUTDIR)\1049.msi
	@echo $@
	torch -nologo -t language $(OUTDIR)\1033.msi $(OUTDIR)\1049.msi -out $@
	wisubstg.vbs $(OUTDIR)\1033.msi $(OUTDIR)\1049 1049

$(OUTDIR)\1045: $(OUTDIR)\1033.msi $(OUTDIR)\1045.msi
	@echo $@
	torch -nologo -t language $(OUTDIR)\1033.msi $(OUTDIR)\1045.msi -out $@
	wisubstg.vbs $(OUTDIR)\1033.msi $(OUTDIR)\1045 1045

$(OUTDIR)\1059: $(OUTDIR)\1033.msi $(OUTDIR)\1059.msi
	@echo $@
	torch -nologo -t language $(OUTDIR)\1033.msi $(OUTDIR)\1059.msi -out $@
	wisubstg.vbs $(OUTDIR)\1033.msi $(OUTDIR)\1059 1059

$(OUTDIR)\customact.dll: customact.cpp $(VC10CPP) customact.def $(VC10OBJ)
	@echo $@
	cl $(CPPFLAGS) -EHsc customact.cpp $(VC10CPP) -link $(LINKFLAGS) $(WINSUBSYSTEM) -dll -out:$@ -def:customact.def $(VC10OBJ) msi.lib shell32.lib rpcrt4.lib
	editbin /nologo /subsystem:console,$(OS_VERSION) /osversion:$(OS_VERSION) $(OUTDIR)\customact.dll > nul

$(WIXOBJ): params.mak defines.wxi guids_$(PLATFORM).wxi

.wxs{$(OUTDIR)}.wixobj::
	candle $(CANDLEFLAGS) $<

$(OUTDIR)\vc_crt_fix.obj: ..\..\plugins\common\vc_crt_fix.asm
	@echo $@
	$(AS) $(ASFLAGS) $**

$(OUTDIR):
	@echo $@
	if not exist $(OUTDIR) mkdir $(OUTDIR)


clean:
	if exist $(OUTDIR) rd /s /q $(OUTDIR)


.SUFFIXES: .wxs
.PHONY: clean
