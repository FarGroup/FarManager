#ifndef __FARVERSION_HPP__
#define __FARVERSION_HPP__
#include "plugin.hpp"

#define FAR_MAJOR_VER FARMANAGERVERSION_MAJOR
#define FAR_MINOR_VER FARMANAGERVERSION_MINOR
#define FAR_BUILD FARMANAGERVERSION_BUILD

#define FARCOMPANYNAME "Eugene Roshal & FAR Group\000\000"

#define FARGROUPCOPYRIGHT "Copyright © 2000-2008 FAR Group"

#define FARCOPYRIGHT "Copyright © Eugene Roshal 1996-2000, " FARGROUPCOPYRIGHT "\000\000"

#define MACROVIEWCOPYRIGHT "Copyright © Konstantin Melnikov 1998-2007, " FARGROUPCOPYRIGHT "\000\000"

#define FARPRODUCTNAME "FAR Manager\000"

#define FULLMAKEFARPRODUCTVERSION(major, minor, build) #major "." #minor " build " #build "\000"
#define MAKEFARPRODUCTVERSION(major, minor, build) FULLMAKEFARPRODUCTVERSION(major, minor, build)
#define FARPRODUCTVERSION MAKEFARPRODUCTVERSION(FAR_MAJOR_VER, FAR_MINOR_VER, FAR_BUILD)

#define fullgenericpluginrc(major, minor, build, desc, name, filename) \
1 VERSIONINFO \
FILEVERSION major, minor, 0, build \
PRODUCTVERSION FAR_MAJOR_VER, FAR_MINOR_VER, 0, FAR_BUILD \
FILEOS 4 \
FILETYPE 2 \
{ \
 BLOCK "StringFileInfo" \
 { \
  BLOCK "000004E4" \
  { \
   VALUE "CompanyName", FARCOMPANYNAME \
   VALUE "FileDescription", desc "\000" \
   VALUE "FileVersion", #major "." #minor " build " #build "\000" \
   VALUE "InternalName", name "\000" \
   VALUE "LegalCopyright", FARCOPYRIGHT \
   VALUE "OriginalFilename", filename "\000" \
   VALUE "ProductName", FARPRODUCTNAME \
   VALUE "ProductVersion", FARPRODUCTVERSION \
  } \
\
 } \
\
 BLOCK "VarFileInfo" \
 { \
   VALUE "Translation", 0, 0x4e4 \
 } \
\
}

#define genericpluginrc(major, minor, build, desc, name, filename) fullgenericpluginrc(major, minor, build, desc, name, filename)

#endif
