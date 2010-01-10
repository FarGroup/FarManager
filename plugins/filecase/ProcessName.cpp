//   Process case convetsion of single file\directorã
//     ProcessName - convert case of given filename
//                   use all options from `Opt`
//                   Call recurse for subdirectories
void ProcessName(TCHAR *OldFullName, DWORD FileAttributes)
{
  TCHAR NewFullName[MAX_PATH];
  TCHAR NewName[MAX_PATH];
  TCHAR NewExt[MAX_PATH];
  TCHAR *ExtPtr;

  lstrcpy(NewFullName, OldFullName);

  // Path
  ExtPtr = _tcsrchr(NewFullName,_T('\\'));
  if(ExtPtr)
    ExtPtr[1] = 0;
  else
    NewFullName[0] = 0;

  //Name
  lstrcpy(NewName,GetOnlyName(OldFullName));

  //Ext
  ExtPtr = _tcsrchr(NewName,_T('.'));

  if(ExtPtr)
  {
    ExtPtr[0] = 0; //delete extension from name
    lstrcpy(NewExt,ExtPtr+1);
  }
  else
    NewExt[0] = 0;

  if (*NewExt==0 && (*NewName==0 ||
     (*NewName==_T('.') && NewName[1]==0) ||
     (*NewName==_T('.') && NewName[1]==_T('.') && NewName[2]==0)))
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
      lstrcat(NewFullName,_T("."));
      lstrcat(NewFullName,NewExt);
    }

    MoveFile(OldFullName,NewFullName);
  }

  //Recurce to directories
  if( Opt.ProcessSubDir && (FileAttributes&FILE_ATTRIBUTE_DIRECTORY))
  {
#ifndef UNICODE
    struct PluginPanelItem *Items;
#define FILE_NAME(i)  Items[i].FindData.cFileName
#define FILE_ATTR(i)  Items[i].FindData.dwFileAttributes
#else
    FAR_FIND_DATA *Items;
#define FILE_NAME(i)  Items[i].lpwszFileName
#define FILE_ATTR(i)  Items[i].dwFileAttributes
#endif
    int ItemsNumber,DirList;

    DirList = Info.GetDirList(OldFullName,&Items,&ItemsNumber);

    if(DirList && ItemsNumber)
    {
      for (int I=0; I < ItemsNumber; I++)
      {
        GetFullName(NewFullName,OldFullName,FILE_NAME(I));
        ProcessName(NewFullName,FILE_ATTR(I));
      }
    }
#undef FILE_NAME
#undef FILE_ATTR
    Info.FreeDirList(Items
#ifdef UNICODE
                          , ItemsNumber
#endif
                    );
  }
}
