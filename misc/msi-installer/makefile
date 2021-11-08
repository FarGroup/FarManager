ROOTDIR = ..\..
NAME = customact

LINK_OBJS = \
	$(INTDIR)\customact.obj \

USERLIBS = \
	msi.lib \

!include $(ROOTDIR)\plugins\makefile_vc_def_inc
!include $(ROOTDIR)\plugins\makefile_vc_target_inc

$(INTDIR)\customact.obj : customact.cpp

!if "$(BUILD_PLATFORM)" == "AMD64"
WIX_PLATFORM=x64
!elseif "$(BUILD_PLATFORM)" == "ARM64"
WIX_PLATFORM=arm64
!else
WIX_PLATFORM=x86
!endif

!ifdef NO_LOCALISED_MSI
TRANSFORMS =
LANGUAGES=1033
!else
TRANSFORMS = $(OUTDIR)\1045 $(OUTDIR)\1049 $(OUTDIR)\1059
LANGUAGES=1033,1045,1049,1059
!endif

CANDLEFLAGS = -nologo -out $(OUTDIR)\ -dSourceDir="$(SOURCE_DIR)" -dPlatform=$(WIX_PLATFORM) -dVerMajor=$(VER_MAJOR) -dVerMinor=$(VER_MINOR) -dVerBuild=$(VER_BUILD) -dLanguages=$(LANGUAGES)

LIGHTFLAGS = -nologo -spdb -sval

AFLAGS = -nologo -c

!ifdef DEBUG
LIGHTFLAGS = $(LIGHTFLAGS) -cc $(OUTDIR) -reusecab
!else
LIGHTFLAGS = $(LIGHTFLAGS) -dcl:high
!endif

WIXOBJ =\
	$(OUTDIR)\installer.wixobj\
	$(OUTDIR)\files.wixobj\
	$(OUTDIR)\features.wixobj\
	$(OUTDIR)\shortcuts.wixobj\
	$(OUTDIR)\ui.wixobj\


!ifdef NIGHTLY
MSI_NAME = $(SOURCE_DIR)\final.msi
!else
MSI_NAME = $(OUTDIR)\far.msi
!endif

ALL: $(MSI_NAME)

$(MSI_NAME): $(OUTDIR) $(INTDIR) $(OUTDIR)\1033.msi $(TRANSFORMS)
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

$(WIXOBJ): defines.wxi guids_$(WIX_PLATFORM).wxi

.wxs{$(OUTDIR)}.wixobj::
	candle $(CANDLEFLAGS) $<

$(OUTDIR):
	@echo $@
	if not exist "$(OUTDIR)\$(NULL)" mkdir "$(OUTDIR)"

$(INTDIR):
	@echo $@
	if not exist "$(INTDIR)\$(NULL)" mkdir "$(INTDIR)"

clean:
	if exist $(OUTDIR) rd /s /q $(OUTDIR)


.SUFFIXES: .wxs
.PHONY: clean
