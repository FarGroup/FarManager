m4_include(`farversion.m4')m4_dnl
#include <windows.h>
#include "res.hpp"
LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_US
FAR_ICON ICON "Far.ico"
LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_US
VS_VERSION_INFO VERSIONINFO
FILEVERSION MAJOR, MINOR, 0, BUILD
PRODUCTVERSION MAJOR, MINOR, 0, BUILD
FILEOS VOS_NT_WINDOWS32
FILETYPE VFT_APP
{
 BLOCK "StringFileInfo"
 {
  BLOCK "000004E4"
  {
   VALUE "CompanyName", "Eugene Roshal & FAR Group\000\000"
   VALUE "FileDescription", "File and archive manager\000\000"
#if __FARBIT__ == 32
   VALUE "FileVersion", "`v'FULLVERSION32\000\000"
#elif __FARBIT__ == 64
   VALUE "FileVersion", "`v'FULLVERSION64\000\000"
#else
   VALUE "FileVersion", "`v'FULLVERSIONIA64\000\000"
#endif
   VALUE "InternalName", "Far\000\000"
   VALUE "LegalCopyright", "© Eugene Roshal, 1996-2000. © FAR Group, COPYRIGHTYEARS\000\000"
   VALUE "OriginalFilename", "Far.exe\000\000"
   VALUE "ProductName", "FAR Manager\000\000"
  }
 }
 BLOCK "VarFileInfo"
 {
   VALUE "Translation", 0x0, 1252
 }
}

// Manifest
CREATEPROCESS_MANIFEST_RESOURCE_ID RT_MANIFEST "Far.exe.manifest"
