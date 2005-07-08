# awk -f ver.awk -v FV1=1 -v FV2=70 -v FV3=2  -v FV4=Patch -v BETA=" beta " > multiarc.rc
BEGIN {
  if(FV4 == "")
  {
    getline FV4 < "vbuild"
    close("vbuild")
  }
  print '#include <windows.h>'
  print 'LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_US'
  print '1 VERSIONINFO'
  print 'FILEVERSION ' FV1 ', ' FV2 ', ' FV3 ', ' FV4
  print 'PRODUCTVERSION ' FV1 ', ' FV2 ', ' FV3 ', 0'
  print 'FILEOS 4'
  print 'FILEFLAGS 0'
  print 'FILETYPE 2'
  print 'FILESUBTYPE 0'
  print '{'
  print ' BLOCK "StringFileInfo"'
  print ' {'
  print '  BLOCK "000004E4"'
  print '  {'
  print '   VALUE "CompanyName", "Eugene Roshal & FAR Group\\000\\000"'
  print '   VALUE "FileDescription", "Archive support plugin for FAR Manager\\000\\000"'
  print '   VALUE "FileVersion", "' FV1 '.' FV2 ' ' BETA ' ' FV3 ' build ' FV4 '\\000\\000"'
  print '   VALUE "InternalName", "MultiArc\\000\\000"'
  print '   VALUE "LegalCopyright", "© Eugene Roshal, 1996-2000. © FAR Group, 2000-2005\\000\\000"'
  print '   VALUE "OriginalFilename", "MULTIARC.DLL\\000\\000"'
  print '   VALUE "ProductName", "FAR manager\\000\\000"'
  print '   VALUE "ProductVersion", "' FV1 '.' FV2 ' ' BETA ' ' FV3 '\\000\\000"'
  print '  }'
  print ' }'
  print ' BLOCK "VarFileInfo"'
  print ' {'
  print '   VALUE "Translation", 0x0, 1252'
  print ' }'
  print '}'
}
