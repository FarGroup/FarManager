
Usage of FAR plugin "standart" library.

  1) Unpack archive with restore dirrectory structure
     U must have at least:
       <you path>\             - place where base header and makefiles placed
                                 Generated libraries also will be placed here
       <you path>\FARStdlib    - sources of FStdLibrary

     If you download samples plugins it will be installed in separate
     directories f.e.:
       <you path>\ftp          - sources of FTP plugin
       <you path>\FileCase     - sources of FileCase plugin

     There are target makefiles in each plugin directory.
     Makefile called by default:
       makefile.bcb            - for BCB
       makefile.vc6            - for VC6

     Recomended compile methods are:
       BCB:
         $(BCB)\bin\make -f"makefile.bcb"
       VC6:
         $(CROOT)\nmake /nologo /f"makefile.vc6"

     Final makefiles has a very simple form. See "Simple template description"
     for syntax description\recomendations.

  2) Correct base makefiles for you target compillers
    a) defs.mak - defines of uniplatform variables
       Edit:
         FARROOT     - path where you install library
         FAREXEROOT  - path where installed existing FAR u planed to test
                       plugins with
         FARUSER     - FAR plugins user (see FAR documentation on "/u" switch)

    b) make_defs.vc6 - Visual C (v6+) makefile
       Edit:
         CROOT, MSROOT     - place of visual command line compiller
         CC,LIB,LINKER,RCC - used tools
    c) make_defs.bcb - Borland CPP Builder (tested w v5+)
       Edit:
         CROOT             - place of command line tools
         CC,LIB,LINKER,
         IMPLIB,RCC        - used tools
      !!Do not edit MAKERSP path

  3) Making library
     It`s normally do not need make library for you compiller separatelly.
     If you use recomended syntax of finel make library will be maed
     automatically.

     If u need to make library separately simple run make_defs.XXX make with
     "FARLIB" target.

     !!To simplify makefile structures all object files for all compillers
     will be placed in the same directory (the same where sources are), so
     if u need to use more then one compiller with incompatible obj file
     structure - do not forget remove object files by yourself before compile
     with different compiller.

  4) Compile existing sample plugins
     To compile existing samples simple go to plugin directory and run
     makefiles for your target compiller.
     During makefile processing plugin sources and sources of library
     (if not compilled yet or up to date) will be compilled and will be
     created plugin DLL.

  5) Run compilled plugin
     To correctly run compilled plugin u must use nex files:
       1. *.DLL - plugin library
       2. *.LNG - plugin language files (if any)
       3. *.HLF - plugin help files
     To run plugin u can do next:
       1. create test directory into you existing FAR "Plugins" directory and
          copy all plugin files where (not recommended)
       2. run your existing FAR to use tested plugin directory as base
          directory for plugins to load and use (recomended).
          See FAR documentations on "/u" and "/p" command line switches.
       3. simple run makefile with test taget.
          F.e. ceate batfile called go.bat with next contents:
            @nmake /nologo /f"makefile.vc6" FARCMDLINE="%1 %2 %3 %4 %5" TEST
          to use VC maker; place it in plugin folder and run it each time u
          want to test plugin separately.

          !! Note, that existing test target will be execute FAR correctly
             only if plugin drectory and plugin DLL has the same name

  6) Create new plugin makefile for existing compiller
     The templates for makefiles for supported compillers can be founed in
     base directory with names simple.XXX there "XXX" is name of supported
     compiller.
     See "Template description" for details.

     Text placed in '<' and '>' u must replace with your specific plugin
     contents.
     Text placed in '{' and '}' is recomended values and must be removed
     before run.

     Read starting comments in "defs.mak" to description of global macroses

  7) Simple template description
     If u do not need some of macroses simply stay it blank.
     User defined macros are:
      LIBROOT    - relative path for base of FARStdLib
      PNAME      - plugin name
      PRES       - sist of resource (RES) files need to be placed if DLL
      PROJECT    - plugin DLL name. U simple can use "$(PNAME).dll"
      PDEF       - DEF file of plugin
      DEBUG      - set to any nonempty value to produce debug info
      MAPFILE    - set to name of mapfile (if empty map file will not be
                   generated)
      CFLAG_ADD  - additional compiller flags.
                   Recomended flags for supported compillers are:
                     BCB:
                     VC6: /GX- /GR- /J
      LFLAG_ADD  - additional linker flags and list of additional libraries
                   used
                   Recomended flags for supported compillers are:
                     BCB: -Tpd -aa -Gi
                     VC6: /dll /section:.rdata,ERS
                          /section:.edata,ERS
                          /merge:.rdata=.text /merge:.edata=.text
      POBJS     - list of plugin object files delimited w space
