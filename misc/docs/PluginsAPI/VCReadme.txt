
       How to compile plugins in MS Visual C++
       =======================================

Brief instructions:
-------------------
1. Compile with two necessary options: -Zp2 -D_export=.
   If you use console API functions (and possibly in other cases), you may
   need to switch alignment "on the fly", see explanations below.
2. Describe all of your exports in a .DEF file to assign to them "naked"
   aliases.


Explanations:
-------------

1. FAR plugin sources were designed for Borland C++. The default alignment
in BC++ is 1-byte bound, while in MS VC++ it's 8-byte. Thus, you must
change it. Using 2-byte alignment works good, and you should compile
with -Zp2 option. -Zp1 is also OK.

Attention! You may experience problems with some Win32 API calls if you use
1-byte alignment. In such cases, do the following trick (suppose you
have problems with Console API structures):

#define _WINCON_ // to prevent including wincon.h
#include <windows.h>
#undef _WINCON_
#include "plugin.hpp" //this file wants 1-byte alignment
#pragma pack(8)
#include <wincon.h> //this file wants 8-byte alignment

All other alignment switching methods do not work well in VC++: due to
some unknown reason, after pack(8) you can't turn it back to pack(1).

2. Exported names required by FAR are "naked", without underscores and other
extra characters in names. When VC++ compiles functions with the WINAPI
descriptor (which is defined as __stdcall), it converts your names to
something like this: _OpenPlugin@8 . You can see these names in your .obj
files. To make it understandable by FAR, you must have them exported with
the names like this: OpenPlugin. You can do it using a .def file.
The .def looks like this:

========= Begin plugin.def =============
LIBRARY
EXPORTS
; list all your exported functions here, for example:
   OpenPlugin
   GetFindData
   ClosePlugin
=========================================
Before you use it, you should delete all the lines with the names you
don't use, otherwise you'll get linker errors.

There is no need to declare your names like  ClosePlugin=_ClosePlugin@4
because linker does it automatically.

Michael Yutsis
michael.y@bigfoot.com
Fidonet: 2:400/333.15
