m4_include(`farversion.m4')m4_dnl
#include <windows.h>
#include "../res.hpp"

LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_US
FAR_ICON ICON "Far.ico"
LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_US
VS_VERSION_INFO VERSIONINFO
FILEVERSION MAJOR, MINOR, 0, BUILD
PRODUCTVERSION MAJOR, MINOR, 0, BUILD
#ifdef _DEBUG
FILEFLAGS VS_FF_DEBUG
#endif
FILEOS VOS_NT_WINDOWS32
FILETYPE VFT_APP
{
	BLOCK "StringFileInfo"
	{
		BLOCK "000004E4"
		{
			VALUE "CompanyName", "Eugene Roshal & FAR Group"
			VALUE "FileDescription", "File and archive manager"
			VALUE "FileVersion", "`v'FULLVERSION"
			VALUE "InternalName", "Far"
			VALUE "LegalCopyright", "© Eugene Roshal, 1996-2000. © FAR Group, COPYRIGHTYEARS"
			VALUE "OriginalFilename", "Far.exe"
			VALUE "ProductName", "FAR Manager"
		}
	}
	BLOCK "VarFileInfo"
	{
		VALUE "Translation", 0x0, 1252
	}
}

// Manifest
CREATEPROCESS_MANIFEST_RESOURCE_ID RT_MANIFEST "bootstrap/Far.exe.manifest"
