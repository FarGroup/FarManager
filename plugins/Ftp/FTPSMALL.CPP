EnumHost::EnumHost(HKEY hRoot,char *HostsPath)
{
  HostPos=0;
  strcpy(RootKey,"Hosts");
  if (*HostsPath)
  {
    strcat(RootKey,"\\");
    strcat(RootKey,HostsPath);
  }
  hEnum=OpenRegKey(hRoot,RootKey);
}


EnumHost::~EnumHost()
{
  if (hEnum!=NULL)
    RegCloseKey(hEnum);
}


BOOL EnumHost::GetNextHost(char *EnumKey,char *HostName,FILETIME *LastWrite)
{
  if (hEnum==NULL)
    return(FALSE);
  char RegKey[1024],SubKey[1024];

  DWORD Size=sizeof(SubKey);
  FILETIME TmpLastWrite;
  FILETIME *CurLastWrite=(LastWrite==NULL) ? &TmpLastWrite:LastWrite;

  if (RegEnumKeyEx(hEnum,HostPos,SubKey,&Size,NULL,NULL,NULL,CurLastWrite)!=ERROR_SUCCESS)
    return(FALSE);
  HostPos++;

  strcpy(EnumKey,RootKey);
  if (*EnumKey)
    strcat(EnumKey,"\\");
  strcat(EnumKey,SubKey);
  if (HostName!=NULL)
    strcpy(HostName,SubKey);
  return(TRUE);
}



