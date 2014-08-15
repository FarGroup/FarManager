m4_include(`farversion.m4')m4_dnl
FILEVERSION MAJOR, MINOR, BUILD, REVISION
PRODUCTVERSION MAJOR, MINOR, BUILD, REVISION
{
	BLOCK "StringFileInfo"
	{
		BLOCK "000004E4"
		{
			VALUE "CompanyName", "Eugene Roshal & FAR Group"
			VALUE "FileDescription", "File and archive manager"
			VALUE "FileVersion", "`v'FULLVERSION"
			VALUE "InternalName", "Far"
			VALUE "LegalCopyright", "© Eugene Roshal, 1996-2000. © Far Group, COPYRIGHTYEARS"
			VALUE "OriginalFilename", "Far.exe"
			VALUE "ProductName", "Far Manager"
		}
	}
	BLOCK "VarFileInfo"
	{
		VALUE "Translation", 0x0, 1252
	}
}
