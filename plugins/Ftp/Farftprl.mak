#
# Borland C++ IDE generated makefile
# Generated 22.01.97 at 17:32:32
#
.AUTODEPEND


#
# Borland C++ tools
#
IMPLIB  = Implib
BCC32   = Bcc32 +BccW32.cfg
TLINK32 = TLink32
TLIB    = TLib
BRC32   = Brc32
TASM32  = Tasm32
#
# IDE macros
#


#
# Options
#
IDE_LinkFLAGS32 =  -LD:\LANG\BC5\LIB
LinkerLocalOptsAtC32_farftpdlib =  -Tpd -ap -c -v
ResLocalOptsAtC32_farftpdlib =
BLocalOptsAtC32_farftpdlib =
CompInheritOptsAt_farftpdlib = -ID:\LANG\BC5\INCLUDE
LinkerInheritOptsAt_farftpdlib = -x
LinkerOptsAt_farftpdlib = $(LinkerLocalOptsAtC32_farftpdlib)
ResOptsAt_farftpdlib = $(ResLocalOptsAtC32_farftpdlib)
BOptsAt_farftpdlib = $(BLocalOptsAtC32_farftpdlib)

#
# Dependency List
#
Dep_Farftp = \
   farftp.lib

Farftp : BccW32.cfg $(Dep_Farftp)
  echo MakeNode

farftp.lib : farftp.dll
  $(IMPLIB) $@ farftp.dll


Dep_farftpddll = \
   ftpmain.obj\
   ftpfn.obj\
   main.obj\
   fake.obj\
   connect.obj\
   cmds.obj\
   farftp.obj

farftp.dll : $(Dep_farftpddll)
  $(TLINK32) @&&|
 $(IDE_LinkFLAGS32) $(LinkerOptsAt_farftpdlib) $(LinkerInheritOptsAt_farftpdlib) +
D:\LANG\BC5\LIB\c0d32.obj+
ftpmain.obj+
ftpfn.obj+
main.obj+
fake.obj+
connect.obj+
cmds.obj+
farftp.obj
$<,$*
D:\LANG\BC5\LIB\import32.lib+
D:\LANG\BC5\LIB\cw32.lib

|

ftpmain.obj :  ftpmain.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_farftpdlib) $(CompInheritOptsAt_farftpdlib) -o$@ ftpmain.cpp
|

ftpfn.obj :  ftpfn.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_farftpdlib) $(CompInheritOptsAt_farftpdlib) -o$@ ftpfn.cpp
|

main.obj :  main.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_farftpdlib) $(CompInheritOptsAt_farftpdlib) -o$@ main.cpp
|

fake.obj :  fake.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_farftpdlib) $(CompInheritOptsAt_farftpdlib) -o$@ fake.cpp
|

connect.obj :  connect.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_farftpdlib) $(CompInheritOptsAt_farftpdlib) -o$@ connect.cpp
|

cmds.obj :  cmds.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_farftpdlib) $(CompInheritOptsAt_farftpdlib) -o$@ cmds.cpp
|

farftp.obj :  farftp.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_farftpdlib) $(CompInheritOptsAt_farftpdlib) -o$@ farftp.cpp
|

# Compiler configuration file
BccW32.cfg :
   Copy &&|
-v
-w
-R
-H
-H=farftp.csm
-WCD
-f-
-w-
-K
-k-
-N-
-x-
-RT-
-R-
-Od
-Og
| $@


