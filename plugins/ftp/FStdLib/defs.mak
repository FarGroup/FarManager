#
# Makefile for all plugins and global definitions
#
# STD MAKEFILE RULLES:
#   $*      - target string (path+name)
#   $**     - depend string (path+name+ext)
#   $<      - target string (path+name+ext)
#   $.      - target string (name+ext)
#   $?      - depend string (path+name+ext)
#
# Macroses affected to sources geneation:
#   __NOFARINLINE__    - do not generate inline functions
#   __FILELOG__=1      - to expanf `Log` macro to save parameters in log, placed
#                        in plugin DLL directory
#

#-- PATHS --------------------------------
# Path to root of FAR samples
FAREXEROOT  = ..\..\..\far.exe
FARUSER     = PLGTest

# Path to temp output dir
OUTDIR      = $(FARROOT)\OUTPUT

# Temprorary used\generated files
RSP         = $(OUTDIR)\tmp.rsp
LLST        = $(OUTDIR)\temp.llst
TBAT        = $(OUTDIR)\tmp.bat

DEF_DEFINES = /D_CONSOLE=1

# Library contents
LIBOBJS     = $(FARROOT)\FARStdlib\fstd_Arg.obj      $(FARROOT)\FARStdlib\fstd_asrt.obj \
              $(FARROOT)\FARStdlib\fstd_cDialog.obj  $(FARROOT)\FARStdlib\fstd_ClpS.obj \
              $(FARROOT)\FARStdlib\fstd_crc32.obj    $(FARROOT)\FARStdlib\fstd_Dbg.obj \
              $(FARROOT)\FARStdlib\fstd_Dialog.obj   $(FARROOT)\FARStdlib\fstd_Dlg.obj \
              $(FARROOT)\FARStdlib\fstd_Editor.obj   $(FARROOT)\FARStdlib\fstd_err.obj \
              $(FARROOT)\FARStdlib\fstd_exSCAT.obj   $(FARROOT)\FARStdlib\fstd_exSCHC.obj \
              $(FARROOT)\FARStdlib\fstd_exSCMP.obj   $(FARROOT)\FARStdlib\fstd_exSCPY.obj \
              $(FARROOT)\FARStdlib\fstd_exSNCH.obj   $(FARROOT)\FARStdlib\fstd_exSPCH.obj \
              $(FARROOT)\FARStdlib\fstd_exSPS.obj    $(FARROOT)\FARStdlib\fstd_ilist.obj \
              $(FARROOT)\FARStdlib\fstd_INProc.obj   $(FARROOT)\FARStdlib\fstd_log.obj \
              $(FARROOT)\FARStdlib\fstd_memb.obj     $(FARROOT)\FARStdlib\fstd_mems.obj \
              $(FARROOT)\FARStdlib\fstd_memh.obj     $(FARROOT)\FARStdlib\fstd_menu.obj \
              $(FARROOT)\FARStdlib\fstd_mesg.obj     $(FARROOT)\FARStdlib\fstd_mklog.obj \
              $(FARROOT)\FARStdlib\fstd_Msg.obj      $(FARROOT)\FARStdlib\fstd_Multiline.obj \
              $(FARROOT)\FARStdlib\fstd_Patt.obj     $(FARROOT)\FARStdlib\fstd_per.obj \
              $(FARROOT)\FARStdlib\fstd_plg.obj      $(FARROOT)\FARStdlib\fstd_RegCh.obj \
              $(FARROOT)\FARStdlib\fstd_RegCp.obj    $(FARROOT)\FARStdlib\fstd_RegCr.obj \
              $(FARROOT)\FARStdlib\fstd_RegDl.obj    $(FARROOT)\FARStdlib\fstd_RegDla.obj \
              $(FARROOT)\FARStdlib\fstd_RegGB.obj    $(FARROOT)\FARStdlib\fstd_RegGI.obj \
              $(FARROOT)\FARStdlib\fstd_RegOp.obj    $(FARROOT)\FARStdlib\fstd_RegRn.obj \
              $(FARROOT)\FARStdlib\fstd_RegSB.obj    $(FARROOT)\FARStdlib\fstd_RegSC.obj \
              $(FARROOT)\FARStdlib\fstd_RegSI.obj    $(FARROOT)\FARStdlib\fstd_SCol.obj \
              $(FARROOT)\FARStdlib\fstd_scr.obj      $(FARROOT)\FARStdlib\fstd_String.obj \
              $(FARROOT)\FARStdlib\fstd_stdlibCS.obj $(FARROOT)\FARStdlib\fstd_stdlibME.obj \
              $(FARROOT)\FARStdlib\fstd_stdlibSP.obj $(FARROOT)\FARStdlib\fstd_stdlibST.obj \
              $(FARROOT)\FARStdlib\fstd_SText.obj    $(FARROOT)\FARStdlib\fstd_Trap.obj \
              $(FARROOT)\FARStdlib\fstd_exit.obj     $(FARROOT)\FARStdlib\fstd_HotKey.obj \
              $(FARROOT)\FARStdlib\fstd_Con.obj      $(FARROOT)\FARStdlib\fstd_FMsg.obj \
              $(FARROOT)\FARStdlib\fstd_CharDraw.obj $(FARROOT)\FARStdlib\fstd_OEM.obj \
              $(FARROOT)\FARStdlib\fstd_Utils.obj    $(FARROOT)\FARStdlib\fstd_FUtils.obj

_defTarg: FARLIB

TEST:
   start $(FAREXEROOT) /u $(FARUSER) /pPlugins\StdPlugins\$(PNAME) $(FARCMDLINE)

DELTMPS:
    -@if exist $(PNAME).lib del $(PNAME).lib >nul
    -@if exist $(PNAME).pdb del $(PNAME).pdb >nul
    -@if exist $(PNAME).exp del $(PNAME).exp >nul
