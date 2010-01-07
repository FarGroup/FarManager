m4_include(`farversion.m4')m4_dnl
#include <windows.h>
#include "../res.hpp"

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
			VALUE "FileVersion", "`v'FULLVERSION32"
#elif __FARBIT__ == 64
			VALUE "FileVersion", "`v'FULLVERSION64"
#else
			VALUE "FileVersion", "`v'FULLVERSIONIA64"
#endif
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
