
       How to compile plugins using Cygwin/MinGW32
       ===========================================

Introduction:
-------------
This document is derived from VCReadme.txt by Michael Yutsis.


Explanation:
------------
1. Exported names are required by FAR to be "naked", without underscores and
other extra characters. When GCC compiles functions with the WINAPI call
convention (which is defined as __stdcall), it converts the names to something
like this: "_OpenPlugin@8". You can see those names in your object (*.o) files.
To make them understandable by FAR, you must have them exported with names like
this: "OpenPlugin". You can do this using a "def" file. Example of a typical
"def" file is given below. Before you use it, you should delete all the lines
with names that you don't use, otherwise you'll get linker errors.

========= Begin plugin.def =============
LIBRARY
EXPORTS
   ClosePlugin=_ClosePlugin@4
   Compare=_Compare@16
   Configure=_Configure@4
   DeleteFiles=_DeleteFiles@16
   ExitFAR=_ExitFAR@0
   FreeFindData=_FreeFindData@12
   FreeVirtualFindData=_FreeVirtualFindData@12
   GetFiles=_GetFiles@24
   GetFindData=_GetFindData@16
   GetMinFarVersion=_GetMinFarVersion@0
   GetOpenPluginInfo=_GetOpenPluginInfo@8
   GetPluginInfo=_GetPluginInfo@4
   GetVirtualFindData=_GetVirtualFindData@16
   MakeDirectory=_MakeDirectory@12
   OpenFilePlugin=_OpenFilePlugin@12
   OpenPlugin=_OpenPlugin@8
   ProcessDialogEvent=_ProcessDialogEvent@8
   ProcessEditorEvent=_ProcessEditorEvent@8
   ProcessEditorInput=_ProcessEditorInput@4
   ProcessEvent=_ProcessEvent@12
   ProcessHostFile=_ProcessHostFile@16
   ProcessKey=_ProcessKey@12
   ProcessViewerEvent=_ProcessViewerEvent@8
   PutFiles=_PutFiles@20
   SetDirectory=_SetDirectory@12
   SetFindList=_SetFindList@12
   SetStartupInfo=_SetStartupInfo@4
=========================================

2. You may find the following Makefile to be a useful reference:

========= Begin Makefile ================
DLLNAME = filecase.dll
SRCS = FileCase.cpp
DEF = FileCase.def

CXX = g++
CXXFLAGS = -mno-cygwin -mdll -O2 -pedantic
LNKFLAGS = -mno-cygwin -mdll -luser32 -s
RM=rm -f
DLLTOOL=dlltool

all: $(DLLNAME)

OBJS = $(patsubst %.def,%.o,$(patsubst %.cpp,%.o,$(filter %.cpp %.def,$(SRCS))))
DEPS = $(patsubst %.cpp,%.d,$(filter %.cpp,$(SRCS)))

%.d: %.cpp
        @echo making depends for $<
        @$(SHELL) -ec '$(CXX) -c -M $(CXXFLAGS) $< \
                | sed '\''s;\($*\).o[ :]*;\1.o $@: ;g'\'' > $@; [ -s $@ ] || rm -f $@'

%.o: %.cpp
        @echo compiling $<
        @$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.def
        @echo creating export file
        @dlltool -e $@ -d $<

$(DLLNAME): $(OBJS)
        @echo linking
        @$(CXX) -mdll -o nul -Xlinker --base-file -Xlinker $@.base $^ $(LNKFLAGS)
        @$(DLLTOOL) --dllname $@ --base-file $@.base --output-exp $@.exp --def $(DEF)
        @$(CXX) -mdll  -o $@ $^ $@.exp $(LNKFLAGS)
        @$(RM) $@.base
        @$(RM) $@.exp

-include $(DEPS)

clean:
        @echo cleaning up
        @rm -f $(DEPS) $(OBJS)
=========================================
