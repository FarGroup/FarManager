//   Process case convetsion of single file\directorã
//     ProcessName - convert case of given filename
//                   use all options from `Opt`
//                   Call recurse for subdirectories
void ProcessName(char *OldFullName, DWORD FileAttributes)
{
  char NewFullName[NM];
  char NewName[NM];
  char NewExt[NM];
  char *ExtPtr;

  lstrcpy(NewFullName, OldFullName);

  // Path
  ExtPtr = strrchr(NewFullName,'\\');
  if(ExtPtr)
    ExtPtr[1] = 0;
  else
    NewFullName[0] = 0;

  //Name
  lstrcpy(NewName,GetOnlyName(OldFullName));

  //Ext
  ExtPtr = strrchr(NewName,'.');

  if(ExtPtr)
  {
    ExtPtr[0] = 0; //delete extension from name
    lstrcpy(NewExt,ExtPtr+1);
  }
  else
    NewExt[0] = 0;

  if (*NewExt==0 && (*NewName==0 ||
     (*NewName=='.' && NewName[1]==0) ||
     (*NewName=='.' && NewName[1]=='.' && NewName[2]==0)))
    return;

  //Check need to convert
  int mN = Opt.ConvertMode!=MODE_NONE && (!Opt.SkipMixedCase || !IsCaseMixed(NewName));
  int mE = Opt.ConvertModeExt!=MODE_NONE && (!Opt.SkipMixedCase || !IsCaseMixed(NewExt));

  //Case single file\dir
  if((mN || mE) && (Opt.ProcessDir || (FileAttributes&FILE_ATTRIBUTE_DIRECTORY) == 0))
  {
    if(mN)
      CaseWord(NewName,Opt.ConvertMode);

    if(mE)
      CaseWord(NewExt,Opt.ConvertModeExt);

    lstrcat(NewFullName,NewName);

    if (NewExt[0])
    {
      lstrcat(NewFullName,".");
      lstrcat(NewFullName,NewExt);
    }

    MoveFile(OldFullName,NewFullName);
  }

  //Recurce to directories
  if( Opt.ProcessSubDir && (FileAttributes&FILE_ATTRIBUTE_DIRECTORY))
  {
    struct PluginPanelItem *PanelItems;
    int ItemsNumber,DirList;

    DirList = Info.GetDirList(OldFullName,&PanelItems,&ItemsNumber);

    if(DirList && ItemsNumber)
    {
      for (int I=0; I < ItemsNumber; I++)
      {
        GetFullName(NewFullName,OldFullName,PanelItems[I].FindData.cFileName);
        ProcessName(NewFullName,PanelItems[I].FindData.dwFileAttributes);
      }
    }
    Info.FreeDirList(PanelItems);
  }
}
