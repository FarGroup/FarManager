m4_include(`farversion.m4')m4_dnl
#include <windows.h>
#include "res.hpp"
LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_US
FAR_ICON ICON "far.ico"
LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_US
VS_VERSION_INFO VERSIONINFO
FILEVERSION MAJOR, MINOR, 0, BUILD
PRODUCTVERSION MAJOR, MINOR, 0, BUILD
FILEOS 4
FILETYPE 1
{
 BLOCK "StringFileInfo"
 {
  BLOCK "000004E4"
  {
m4_ifelse(`1',BUILDTESTONLY,`   VALUE "Comments", "TEST ONLY!\000\000"',`m4_dnl')
   VALUE "CompanyName", "Eugene Roshal & FAR Group\000\000"
   VALUE "FileDescription", "File and archive manager\000\000"
   VALUE "FileVersion", "`v'FULLVERSION\000\000"
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
