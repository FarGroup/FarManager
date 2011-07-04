//   Process case conversion of single file\directory
//     ProcessName - convert case of given filename
//                   use all options from `Opt`
//                   Call recurse for subdirectories
void ProcessName(const wchar_t *OldFullName, DWORD FileAttributes)
{
  wchar_t NewFullName[MAX_PATH];
  wchar_t NewName[MAX_PATH];
  wchar_t NewExt[MAX_PATH];
  wchar_t *ExtPtr;

  lstrcpy(NewFullName, OldFullName);

  // Path
  ExtPtr = wcsrchr(NewFullName,L'\\');
  if(ExtPtr)
    ExtPtr[1] = 0;
  else
    NewFullName[0] = 0;

  //Name
  lstrcpy(NewName,GetOnlyName(OldFullName));

  //Ext
  ExtPtr = wcsrchr(NewName,L'.');

  if(ExtPtr)
  {
    ExtPtr[0] = 0; //delete extension from name
    lstrcpy(NewExt,ExtPtr+1);
  }
  else
    NewExt[0] = 0;

  if (*NewExt==0 && (*NewName==0 || (*NewName==L'.' && NewName[1]==0) || (*NewName==L'.' && NewName[1]==L'.' && NewName[2]==0)))
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
      lstrcat(NewFullName,L".");
      lstrcat(NewFullName,NewExt);
    }

    MoveFile(OldFullName,NewFullName);
  }

  //Recurce to directories
  if( Opt.ProcessSubDir && (FileAttributes&FILE_ATTRIBUTE_DIRECTORY))
  {
    struct PluginPanelItem *Items;
    size_t ItemsNumber,DirList;

    DirList = Info.GetDirList(OldFullName,&Items,&ItemsNumber);

    if(DirList && ItemsNumber)
    {
      for (int I=0; I < ItemsNumber; I++)
      {
        ProcessName(Items[I].FileName,Items[I].FileAttributes);
      }
    }
    Info.FreeDirList(Items, ItemsNumber);
  }
}
