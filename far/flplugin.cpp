/*
flplugin.cpp

�������� ������ - ������ � ���������

*/

/* Revision: 1.66 06.06.2006 $ */

#include "headers.hpp"
#pragma hdrstop

#include "lang.hpp"
#include "filelist.hpp"
#include "plugin.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "filepanels.hpp"
#include "history.hpp"
#include "ctrlobj.hpp"
/*
   � ����� ������ ������ �� �������� - ������ ����������!
*/

void FileList::PushPlugin(HANDLE hPlugin,const wchar_t *HostFile)
{
  PluginsStackItem *stItem = new PluginsStackItem;
  stItem->hPlugin=hPlugin;
  stItem->strHostFile = NullToEmptyW(HostFile); //??NULL??
  stItem->Modified=FALSE;
  stItem->PrevViewMode=ViewMode;
  stItem->PrevSortMode=SortMode;
  stItem->PrevSortOrder=SortOrder;
  stItem->PrevNumericSort=NumericSort;
  memmove(&stItem->PrevViewSettings,&ViewSettings,sizeof(struct PanelViewSettings));

  PluginsStack=(PluginsStackItem **)xf_realloc(PluginsStack,(PluginsStackSize+1)*4);
  PluginsStack[PluginsStackSize] = stItem;
  PluginsStackSize++;
}


int FileList::PopPlugin(int EnableRestoreViewMode)
{
  struct OpenPluginInfoW Info;
  Info.StructSize=0;

  if (PluginsStackSize==0)
  {
    PanelMode=NORMAL_PANEL;
    return(FALSE);
  }

  PluginsStackSize--;

  // ��������� ������� ������.
  CtrlObject->Plugins.ClosePlugin(hPlugin);

  PluginsStackItem *PStack=PluginsStack[PluginsStackSize]; // ��������� �� ������, � �������� ������

  if (PluginsStackSize>0)
  {
    hPlugin=PluginsStack[PluginsStackSize-1]->hPlugin;

    if (EnableRestoreViewMode)
    {
      SetViewMode (PStack->PrevViewMode);

      SortMode=PStack->PrevSortMode;
      NumericSort=PStack->PrevNumericSort;
      SortOrder=PStack->PrevSortOrder;
    }

    if (PStack->Modified)
    {
      struct PluginPanelItemW PanelItem;

      string strSaveDir;

      FarGetCurDirW (strSaveDir);

      if (FileNameToPluginItem(PStack->strHostFile,&PanelItem))
        CtrlObject->Plugins.PutFiles(hPlugin,&PanelItem,1,FALSE,0);
      else
      {
        memset(&PanelItem,0,sizeof(struct PluginPanelItemW));

        PanelItem.FindData.lpwszFileName = _wcsdup(PointToNameW(PStack->strHostFile));
        CtrlObject->Plugins.DeleteFiles(hPlugin,&PanelItem,1,0);

        xf_free (PanelItem.FindData.lpwszFileName);
      }

      FarChDirW (strSaveDir);
    }

    CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);

    if ((Info.Flags & OPIF_REALNAMES)==0)
    {
      DeleteFileWithFolderW(PStack->strHostFile);  // �������� ����� �� ����������� �������
    }
  }
  else
  {
    PanelMode=NORMAL_PANEL;
    if(EnableRestoreViewMode)
    {
      SetViewMode (PStack->PrevViewMode);

      SortMode=PStack->PrevSortMode;
      NumericSort=PStack->PrevNumericSort;
      SortOrder=PStack->PrevSortOrder;
    }
  }

  delete PluginsStack[PluginsStackSize];
  PluginsStack=(PluginsStackItem **)xf_realloc(PluginsStack,PluginsStackSize*4);

  if (EnableRestoreViewMode)
    CtrlObject->Cp()->RedrawKeyBar();

  return(TRUE);
}


int FileList::FileNameToPluginItem(const wchar_t *Name,PluginPanelItemW *pi)
{
  string strTempDir = Name;

  wchar_t *ChPtr = strTempDir.GetBuffer ();

  if ((ChPtr=wcsrchr(ChPtr,L'\\'))==NULL)
    return(FALSE);
  *ChPtr=0;

  strTempDir.ReleaseBuffer ();

  FarChDirW(strTempDir);
  memset(pi,0,sizeof(*pi));

  FAR_FIND_DATA_EX fdata;

  if ( apiGetFindDataEx (Name, &fdata) )
  {
      apiFindDataExToData(&fdata, &pi->FindData);
      return TRUE;
  }

  return FALSE;
}


void FileList::FileListToPluginItem(struct FileListItem *fi,struct PluginPanelItemW *pi)
{
  pi->FindData.lpwszFileName = _wcsdup (fi->strName);
  pi->FindData.lpwszAlternateFileName = _wcsdup (fi->strShortName);

  pi->FindData.nFileSize=fi->UnpSize;
  pi->FindData.nPackSize=fi->PackSize;
  pi->FindData.dwFileAttributes=fi->FileAttr;
  pi->FindData.ftLastWriteTime=fi->WriteTime;
  pi->FindData.ftCreationTime=fi->CreationTime;
  pi->FindData.ftLastAccessTime=fi->AccessTime;
  pi->NumberOfLinks=fi->NumberOfLinks;
  pi->Flags=fi->UserFlags;
  if (fi->Selected)
    pi->Flags|=PPIF_SELECTED;
  pi->CustomColumnData=fi->CustomColumnData;
  pi->CustomColumnNumber=fi->CustomColumnNumber;

  //pi->Description=fi->DizText; //BUGBUG

  if (fi->UserData && (fi->UserFlags & PPIF_USERDATA))
  {
    DWORD Size=*(DWORD *)fi->UserData;
    /* $ 13.07.2000 SVS
       ������� new �� malloc
    */
    pi->UserData=(DWORD)xf_malloc(Size);
    /* SVS $ */
    memcpy((void *)pi->UserData,(void *)fi->UserData,Size);
  }
  else
    pi->UserData=fi->UserData;
  pi->CRC32=fi->CRC32;
  pi->Reserved[0]=pi->Reserved[1]=0;
  pi->Owner=fi->strOwner.IsEmpty()?NULL:(wchar_t*)(const wchar_t*)fi->strOwner;
}


void FileList::PluginToFileListItem(struct PluginPanelItemW *pi,struct FileListItem *fi)
{
  fi->strName = pi->FindData.lpwszFileName;
  fi->strShortName = pi->FindData.lpwszAlternateFileName;

  fi->strOwner = NullToEmptyW(pi->Owner);
  if (pi->Description)
  {
    fi->DizText=new wchar_t[wcslen(pi->Description)+1];

    wcscpy (fi->DizText, pi->Description);

    fi->DeleteDiz=TRUE;
  }
  else
    fi->DizText=NULL;
  fi->UnpSize=pi->FindData.nFileSize;
  fi->PackSize=pi->FindData.nPackSize;
  fi->FileAttr=pi->FindData.dwFileAttributes;
  fi->WriteTime=pi->FindData.ftLastWriteTime;
  fi->CreationTime=pi->FindData.ftCreationTime;
  fi->AccessTime=pi->FindData.ftLastAccessTime;
  fi->NumberOfLinks=pi->NumberOfLinks;
  fi->UserFlags=pi->Flags;

  if (pi->UserData && (pi->Flags & PPIF_USERDATA))
  {
    DWORD Size=*(DWORD *)pi->UserData;
    /* $ 13.07.2000 SVS
       ������� new �� malloc
    */
    fi->UserData=(DWORD)xf_malloc(Size);
    /* SVS $ */
    memcpy((void *)fi->UserData,(void *)pi->UserData,Size);
  }
  else
    fi->UserData=pi->UserData;
  if (pi->CustomColumnNumber>0)
  {
    fi->CustomColumnData=new wchar_t*[pi->CustomColumnNumber];
    for (int I=0;I<pi->CustomColumnNumber;I++)
      if (pi->CustomColumnData!=NULL && pi->CustomColumnData[I]!=NULL)
      {
        fi->CustomColumnData[I]=new wchar_t[wcslen(pi->CustomColumnData[I])+1];
        wcscpy(fi->CustomColumnData[I],pi->CustomColumnData[I]);
      }
      else
        fi->CustomColumnData[I]=L"";
  }
  fi->CustomColumnNumber=pi->CustomColumnNumber;
  fi->CRC32=pi->CRC32;
}


HANDLE FileList::OpenPluginForFile(const wchar_t *FileName,DWORD FileAttr)
{
  if(!FileName || !*FileName || (FileAttr&FA_DIREC))
    return(INVALID_HANDLE_VALUE);

  SetCurPath();

  HANDLE hFile=INVALID_HANDLE_VALUE;
  if(WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
    hFile=FAR_CreateFileW(FileName,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE ,NULL,
                         OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN|FILE_FLAG_POSIX_SEMANTICS,
                         NULL);
  if(hFile==INVALID_HANDLE_VALUE)
    hFile=FAR_CreateFileW(FileName,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE ,NULL,
                         OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN, NULL);

  if (hFile==INVALID_HANDLE_VALUE)
  {
    //Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MEditTitle),MSG(MCannotOpenFile),FileName,MSG(MOk));
    MessageW(MSG_WARNING|MSG_ERRORTYPE,1,L"",UMSG(MOpenPluginCannotOpenFile),FileName,UMSG(MOk));
    return(INVALID_HANDLE_VALUE);
  }

  char *Buffer=new char[Opt.PluginMaxReadData];
  if(Buffer)
  {
    DWORD BytesRead;
    _ALGO(SysLog("Read %d byte(s)",Opt.PluginMaxReadData));
    if(ReadFile(hFile,Buffer,Opt.PluginMaxReadData,&BytesRead,NULL))
    {
      CloseHandle(hFile);
      _ALGO(SysLogDump("First 128 bytes",0,(LPBYTE)Buffer,128,NULL));

      _ALGO(SysLog("close AnotherPanel file"));
      CtrlObject->Cp()->GetAnotherPanel(this)->CloseFile();

      _ALGO(SysLog("call Plugins.OpenFilePlugin {"));
      HANDLE hNewPlugin=CtrlObject->Plugins.OpenFilePlugin(FileName,(unsigned char *)Buffer,BytesRead);
      _ALGO(SysLog("}"));

      delete[] Buffer;

      return(hNewPlugin);
    }
    else
    {
      delete[] Buffer;
      _ALGO(SysLogLastError());
    }
  }

  CloseHandle(hFile);
  return(INVALID_HANDLE_VALUE);
}


void FileList::CreatePluginItemList(struct PluginPanelItemW *(&ItemList),int &ItemNumber,BOOL AddTwoDot)
{
  if (!ListData)
    return;

  long SaveSelPosition=GetSelPosition;
  long OldLastSelPosition=LastSelPosition;

  string strSelName;
  int FileAttr;
  ItemNumber=0;
  ItemList=new PluginPanelItemW[SelFileCount+1];
  if (ItemList!=NULL)
  {
    memset(ItemList,0,sizeof(struct PluginPanelItemW) * (SelFileCount+1));
    GetSelNameW(NULL,FileAttr);
    while (GetSelNameW(&strSelName,FileAttr))
      if (((FileAttr & FA_DIREC)==0 || !TestParentFolderNameW(strSelName))
          && LastSelPosition>=0 && LastSelPosition<FileCount)
      {
        FileListToPluginItem(ListData[LastSelPosition],ItemList+ItemNumber);
        ItemNumber++;
      }

    if(AddTwoDot && !ItemNumber && (FileAttr & FA_DIREC)) // ��� ��� ".."
    {
      ItemList->FindData.lpwszFileName = _wcsdup (ListData[0]->strName);
      ItemList->FindData.dwFileAttributes=ListData[0]->FileAttr;
      ItemNumber++;
    }
  }

  LastSelPosition=OldLastSelPosition;
  GetSelPosition=SaveSelPosition;
}


void FileList::DeletePluginItemList(struct PluginPanelItemW *(&ItemList),int &ItemNumber)
{
  struct PluginPanelItemW *PItemList=ItemList;
  if(PItemList)
  {
    for (int I=0;I<ItemNumber;I++,PItemList++)
    {
      xf_free (PItemList->FindData.lpwszFileName);
      if ((PItemList->Flags & PPIF_USERDATA) && PItemList->UserData)
        xf_free((void *)PItemList->UserData);
    }
    delete[] ItemList;
  }
}


void FileList::PluginDelete()
{
  _ALGO(CleverSysLog clv("FileList::PluginDelete()"));
  struct PluginPanelItemW *ItemList;
  int ItemNumber;
  SaveSelection();
  CreatePluginItemList(ItemList,ItemNumber);
  if (ItemList!=NULL && ItemNumber>0)
  {
    if (CtrlObject->Plugins.DeleteFiles(hPlugin,ItemList,ItemNumber,0))
    {
      SetPluginModified();
      PutDizToPlugin(this,ItemList,ItemNumber,TRUE,FALSE,NULL,&Diz);
    }
    DeletePluginItemList(ItemList,ItemNumber);
    Update(UPDATE_KEEP_SELECTION);
    Redraw();
    Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
    AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
    AnotherPanel->Redraw();
  }
}


void FileList::PutDizToPlugin(FileList *DestPanel,struct PluginPanelItemW *ItemList,
                              int ItemNumber,int Delete,int Move,DizList *SrcDiz,
                              DizList *DestDiz)
{
  _ALGO(CleverSysLog clv("FileList::PutDizToPlugin()"));
  struct OpenPluginInfoW Info;
  CtrlObject->Plugins.GetOpenPluginInfo(DestPanel->hPlugin,&Info);
  if ( DestPanel->strPluginDizName.IsEmpty() && Info.DescrFilesNumber>0)
    DestPanel->strPluginDizName = Info.DescrFiles[0];
  if ((Opt.Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED && IsDizDisplayed() ||
      Opt.Diz.UpdateMode==DIZ_UPDATE_ALWAYS) && !DestPanel->strPluginDizName.IsEmpty() &&
      (Info.HostFile==NULL || *Info.HostFile==0 || DestPanel->GetModalMode() ||
      GetFileAttributesW(Info.HostFile)!=0xFFFFFFFF))
  {
    CtrlObject->Cp()->LeftPanel->ReadDiz();
    CtrlObject->Cp()->RightPanel->ReadDiz();

    if (DestPanel->GetModalMode())
      DestPanel->ReadDiz();

    int DizPresent=FALSE;
    for (int I=0;I<ItemNumber;I++)
      if (ItemList[I].Flags & PPIF_PROCESSDESCR)
      {
          string strName = ItemList[I].FindData.lpwszFileName;
          string strShortName = ItemList[I].FindData.lpwszAlternateFileName;

        int Code;
        if (Delete)
          Code=DestDiz->DeleteDiz(strName,strShortName);
        else
        {
          Code=SrcDiz->CopyDiz(strName,strShortName,strName,strShortName,DestDiz);
          if (Code && Move)
            SrcDiz->DeleteDiz(strName,strShortName);
        }
        if (Code)
          DizPresent=TRUE;
      }
    if (DizPresent)
    {
      string strTempDir, strDizName;
      if (FarMkTempExW(strTempDir) && CreateDirectoryW(strTempDir,NULL))
      {
        string strSaveDir;
        FarGetCurDirW (strSaveDir);

        strDizName.Format (L"%s\\%s",(const wchar_t*)strTempDir, (const wchar_t*)DestPanel->strPluginDizName);

        DestDiz->Flush(L"", (const wchar_t*)strDizName);
        if (Move)
          SrcDiz->Flush(L"",NULL);
        struct PluginPanelItemW PanelItem;

        if (FileNameToPluginItem(strDizName,&PanelItem))
          CtrlObject->Plugins.PutFiles(DestPanel->hPlugin,&PanelItem,1,FALSE,OPM_SILENT|OPM_DESCR);
        else
          if (Delete)
          {
            PluginPanelItemW pi;
            memset(&pi,0,sizeof(PluginPanelItemW));

            pi.FindData.lpwszFileName = _wcsdup (DestPanel->strPluginDizName);
            CtrlObject->Plugins.DeleteFiles(DestPanel->hPlugin,&pi,1,OPM_SILENT);
            xf_free (pi.FindData.lpwszFileName);
          }
        FarChDirW (strSaveDir);
        DeleteFileWithFolderW(strDizName);
      }
    }
  }
}


void FileList::PluginGetFiles(const wchar_t *DestPath,int Move)
{
  _ALGO(CleverSysLog clv("FileList::PluginGetFiles()"));
  struct PluginPanelItemW *ItemList, *PList;
  int ItemNumber;
  SaveSelection();
  CreatePluginItemList(ItemList,ItemNumber);

  if (ItemList!=NULL && ItemNumber>0)
  {
    int GetCode=CtrlObject->Plugins.GetFiles(hPlugin,ItemList,ItemNumber,Move,DestPath,0);
    if (Opt.Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED && IsDizDisplayed() ||
        Opt.Diz.UpdateMode==DIZ_UPDATE_ALWAYS)
    {
      DizList DestDiz;
      int DizFound=FALSE;
      PList=ItemList;
      for (int I=0;I<ItemNumber;I++,PList++)
        if (PList->Flags & PPIF_PROCESSDESCR)
        {
          if (!DizFound)
          {
            CtrlObject->Cp()->LeftPanel->ReadDiz();
            CtrlObject->Cp()->RightPanel->ReadDiz();
            DestDiz.Read(DestPath);
            DizFound=TRUE;
          }
          string strName = PList->FindData.lpwszFileName;
          string strShortName = PList->FindData.lpwszAlternateFileName;
          CopyDiz(strName,strShortName,strName,strName,&DestDiz);
        }
      DestDiz.Flush(DestPath);
    }
    if (GetCode==1)
    {
      if (!ReturnCurrentFile)
        ClearSelection();
      if (Move)
      {
        SetPluginModified();
        PutDizToPlugin(this,ItemList,ItemNumber,TRUE,FALSE,NULL,&Diz);
      }
    }
    else
      if (!ReturnCurrentFile)
        PluginClearSelection(ItemList,ItemNumber);
    DeletePluginItemList(ItemList,ItemNumber);
    Update(UPDATE_KEEP_SELECTION);
    Redraw();
    Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
    AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
    AnotherPanel->Redraw();
  }
}


void FileList::PluginToPluginFiles(int Move)
{
  _ALGO(CleverSysLog clv("FileList::PluginToPluginFiles()"));
  struct PluginPanelItemW *ItemList;
  int ItemNumber;
  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
  string strTempDir;
  if (AnotherPanel->GetMode()!=PLUGIN_PANEL)
    return;
  FileList *AnotherFilePanel=(FileList *)AnotherPanel;

  if (!FarMkTempExW(strTempDir))
    return;
  SaveSelection();
  CreateDirectoryW(strTempDir,NULL);
  CreatePluginItemList(ItemList,ItemNumber);
  if (ItemList!=NULL && ItemNumber>0)
  {
    int PutCode=CtrlObject->Plugins.GetFiles(hPlugin,ItemList,ItemNumber,FALSE,strTempDir,OPM_SILENT);
    if (PutCode==1 || PutCode==2)
    {
      string strSaveDir;
      FarGetCurDirW (strSaveDir);

      FarChDirW(strTempDir);
      PutCode=CtrlObject->Plugins.PutFiles(AnotherFilePanel->hPlugin,ItemList,ItemNumber,FALSE,0);
      if (PutCode==1 || PutCode==2)
      {
        if (!ReturnCurrentFile)
          ClearSelection();
        AnotherPanel->SetPluginModified();
        PutDizToPlugin(AnotherFilePanel,ItemList,ItemNumber,FALSE,FALSE,&Diz,&AnotherFilePanel->Diz);
        if (Move)
          if (CtrlObject->Plugins.DeleteFiles(hPlugin,ItemList,ItemNumber,OPM_SILENT))
          {
            SetPluginModified();
            PutDizToPlugin(this,ItemList,ItemNumber,TRUE,FALSE,NULL,&Diz);
          }
      }
      else
        if (!ReturnCurrentFile)
          PluginClearSelection(ItemList,ItemNumber);
      FarChDirW(strSaveDir);
    }
    DeleteDirTree(strTempDir);
    DeletePluginItemList(ItemList,ItemNumber);
    Update(UPDATE_KEEP_SELECTION);
    Redraw();
    if (PanelMode==PLUGIN_PANEL)
      AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
    else
      AnotherPanel->Update(UPDATE_KEEP_SELECTION);
    AnotherPanel->Redraw();
  }
}


void FileList::PluginHostGetFiles()
{
  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
  string strDestPath;
  string strSelName;

  wchar_t *ExtPtr;
  int FileAttr;

  SaveSelection();

  GetSelNameW(NULL,FileAttr);
  if (!GetSelNameW(&strSelName,FileAttr))
    return;

  AnotherPanel->GetCurDirW(strDestPath);
  if ((!AnotherPanel->IsVisible() || AnotherPanel->GetType()!=FILE_PANEL) &&
      SelFileCount==0 || strDestPath.IsEmpty() )
  {
      strDestPath = PointToNameW(strSelName);
    // SVS: � ����� ����� ����� ����� ����� � ������?
    wchar_t *lpwszDestPath = strDestPath.GetBuffer();

    if ((ExtPtr=wcsrchr(lpwszDestPath,L'.'))!=NULL)
      *ExtPtr=0;

    strDestPath.ReleaseBuffer();
  }

  int OpMode=OPM_TOPLEVEL,ExitLoop=FALSE;
  GetSelNameW(NULL,FileAttr);
  while (!ExitLoop && GetSelNameW(&strSelName,FileAttr))
  {
    HANDLE hCurPlugin;

    if ((hCurPlugin=OpenPluginForFile(strSelName,FileAttr))!=INVALID_HANDLE_VALUE &&
        hCurPlugin!=(HANDLE)-2)
    {
      struct PluginPanelItemW *ItemList;
      int ItemNumber;
      _ALGO(SysLog("call Plugins.GetFindData()"));
      if (CtrlObject->Plugins.GetFindData(hCurPlugin,&ItemList,&ItemNumber,0))
      {
        _ALGO(SysLog("call Plugins.GetFiles()"));
        ExitLoop=CtrlObject->Plugins.GetFiles(hCurPlugin,ItemList,ItemNumber,FALSE,strDestPath,OpMode)!=1;
        if (!ExitLoop)
        {
          _ALGO(SysLog("call ClearLastGetSelection()"));
          ClearLastGetSelection();
        }
        _ALGO(SysLog("call Plugins.FreeFindData()"));
        CtrlObject->Plugins.FreeFindData(hCurPlugin,ItemList,ItemNumber);
        OpMode|=OPM_SILENT;
      }
      _ALGO(SysLog("call Plugins.ClosePlugin"));
      CtrlObject->Plugins.ClosePlugin(hCurPlugin);
    }
  }
  Update(UPDATE_KEEP_SELECTION);
  Redraw();
  AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
  AnotherPanel->Redraw();
}


void FileList::PluginPutFilesToNew()
{
  _ALGO(CleverSysLog clv("FileList::PluginPutFilesToNew()"));
  //_ALGO(SysLog("FileName='%s'",(FileName?FileName:"(NULL)")));
  _ALGO(SysLog("call Plugins.OpenFilePlugin(NULL,NULL,0)"));
  HANDLE hNewPlugin=CtrlObject->Plugins.OpenFilePlugin(NULL,NULL,0);
  if (hNewPlugin!=INVALID_HANDLE_VALUE && hNewPlugin!=(HANDLE)-2)
  {
    _ALGO(SysLog("Create: FileList TmpPanel, FileCount=%d",FileCount));
    FileList TmpPanel;
    TmpPanel.SetPluginMode(hNewPlugin,L"");
    TmpPanel.SetModalMode(TRUE);
    int PrevFileCount=FileCount;
    /* $ 12.04.2002 IS
       ���� PluginPutFilesToAnother ������� �����, �������� �� 2, �� �����
       ����������� ���������� ������ �� ��������� ����.
    */
    int rc=PluginPutFilesToAnother(FALSE,&TmpPanel);
    if (rc!=2 && FileCount==PrevFileCount+1)
    /* IS $ */
    {
      int LastPos=0;
      /* �����, ��� ����������� ���������� ��������������� �����
         ���������������� ���������� �� ���� � ������������ �����
         �������� �����. ������, ���� �����-�� ������� �������� ������
         � ������� �������� ����� � ����� �������� ������� �������,
         �� ����������� ���������������� �� ����������!
      */
      struct FileListItem *PtrListData, *PtrLastPos=ListData[0];
      for (int I=1; I < FileCount; I++)
      {
        PtrListData = ListData[I+1];
        if ((*(__int64*)&PtrListData->CreationTime - *(__int64*)&PtrLastPos->CreationTime) > 0)
        {
          PtrLastPos=ListData[LastPos=I];
        }
      }
      CurFile=LastPos;
      Redraw();
    }
  }
}


/* $ 12.04.2002 IS
     PluginPutFilesToAnother ������ int - ���������� ��, ��� ����������
     PutFiles:
     -1 - �������� ������������
      0 - �������
      1 - �����
      2 - �����, ������ ������������� ���������� �� ���� � ������ ���
          ������������� �� ����� (��. PluginPutFilesToNew)
*/
int FileList::PluginPutFilesToAnother(int Move,Panel *AnotherPanel)
{
  if (AnotherPanel->GetMode()!=PLUGIN_PANEL)
    return 0;
  FileList *AnotherFilePanel=(FileList *)AnotherPanel;
  struct PluginPanelItemW *ItemList;
  int ItemNumber,PutCode=0;
  SaveSelection();
  CreatePluginItemList(ItemList,ItemNumber);
  if (ItemList!=NULL && ItemNumber>0)
  {
    SetCurPath();
    _ALGO(SysLog("call Plugins.PutFiles"));
    PutCode=CtrlObject->Plugins.PutFiles(AnotherFilePanel->hPlugin,ItemList,ItemNumber,Move,0);
    if (PutCode==1 || PutCode==2)
    {
      if (!ReturnCurrentFile)
      {
        _ALGO(SysLog("call ClearSelection()"));
        ClearSelection();
      }
      _ALGO(SysLog("call PutDizToPlugin"));
      PutDizToPlugin(AnotherFilePanel,ItemList,ItemNumber,FALSE,Move,&Diz,&AnotherFilePanel->Diz);
      AnotherPanel->SetPluginModified();
    }
    else
      if (!ReturnCurrentFile)
        PluginClearSelection(ItemList,ItemNumber);
    _ALGO(SysLog("call DeletePluginItemList"));
    DeletePluginItemList(ItemList,ItemNumber);
    Update(UPDATE_KEEP_SELECTION);
    Redraw();
    if (AnotherPanel==CtrlObject->Cp()->GetAnotherPanel(this))
    {
      AnotherPanel->Update(UPDATE_KEEP_SELECTION);
      AnotherPanel->Redraw();
    }
  }
  return PutCode;
}


void FileList::GetPluginInfo(struct PluginInfoW *Info)
{
  _ALGO(CleverSysLog clv("FileList::GetPluginInfo()"));
  memset(Info,0,sizeof(struct PluginInfoW));
  if (PanelMode==PLUGIN_PANEL)
  {
    struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
    CtrlObject->Plugins.GetPluginInfo(ph->PluginNumber,Info);
  }
}


void FileList::GetOpenPluginInfo(struct OpenPluginInfoW *Info)
{
  _ALGO(CleverSysLog clv("FileList::GetOpenPluginInfo()"));
  //_ALGO(SysLog("FileName='%s'",(FileName?FileName:"(NULL)")));
  memset(Info,0,sizeof(struct OpenPluginInfoW));
  if (PanelMode==PLUGIN_PANEL)
    CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,Info);
}


/*
   ������� ��� ������ ������� "�������� �������" (Shift-F3)
*/
void FileList::ProcessHostFile()
{
  _ALGO(CleverSysLog clv("FileList::ProcessHostFile()"));
  //_ALGO(SysLog("FileName='%s'",(FileName?FileName:"(NULL)")));
  if (FileCount>0 && SetCurPath())
  {
    int Done=FALSE;

    SaveSelection();

    if (PanelMode==PLUGIN_PANEL && !PluginsStack[PluginsStackSize-1]->strHostFile.IsEmpty() )
    {
      struct PluginPanelItemW *ItemList;
      int ItemNumber;
      _ALGO(SysLog("call CreatePluginItemList"));
      CreatePluginItemList(ItemList,ItemNumber);
      _ALGO(SysLog("call Plugins.ProcessHostFile"));
      Done=CtrlObject->Plugins.ProcessHostFile(hPlugin,ItemList,ItemNumber,0);
      if (Done)
        SetPluginModified();
      else
      {
        if (!ReturnCurrentFile)
          PluginClearSelection(ItemList,ItemNumber);
        Redraw();
      }
      _ALGO(SysLog("call DeletePluginItemList"));
      DeletePluginItemList(ItemList,ItemNumber);
      if (Done)
        ClearSelection();
    }
    else
    {
      int SCount=GetRealSelCount();
      if(SCount > 0)
      {
        struct FileListItem *CurPtr;
        for(int I=0; I < FileCount; ++I)
        {
          CurPtr = ListData[I];
          if (CurPtr->Selected)
          {
            Done=ProcessOneHostFile(I);
            if(Done == 1)
              Select(CurPtr,0);
            else if(Done == -1)
              continue;
            else       // ���� ��� ������, ��... ����� ���� ESC �� ������ ������
              break;   //
          }
        }

        if (SelectedFirst)
          SortFileList(TRUE);
      }
      else
      {
        if((Done=ProcessOneHostFile(CurFile)) == 1)
         ClearSelection();
      }
    }

    if (Done)
    {
      Update(UPDATE_KEEP_SELECTION);
      Redraw();
      Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
      AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
      AnotherPanel->Redraw();
    }
  }
}

/*
  ��������� ������ ����-�����.
  Return:
    -1 - ���� ���� ������� �������� �� ���������
     0 - ������ ������ FALSE
     1 - ������ ������ TRUE
*/
int FileList::ProcessOneHostFile(int Idx)
{
  _ALGO(CleverSysLog clv("FileList::ProcessOneHostFile()"));
  int Done=-1;

  _ALGO(SysLog("call OpenPluginForFile([Idx=%d] '%s')",Idx,ListData[Idx]->Name));

  string strName = ListData[Idx]->strName;

  HANDLE hNewPlugin=OpenPluginForFile(strName,ListData[Idx]->FileAttr);

  if (hNewPlugin!=INVALID_HANDLE_VALUE && hNewPlugin!=(HANDLE)-2)
  {
    struct PluginPanelItemW *ItemList;
    int ItemNumber;
    _ALGO(SysLog("call Plugins.GetFindData"));
    if (CtrlObject->Plugins.GetFindData(hNewPlugin,&ItemList,&ItemNumber,OPM_TOPLEVEL))
    {
      /* $ 26.04.2001 DJ
         � ProcessHostFile �� ����������� OPM_TOPLEVEL
      */
      _ALGO(SysLog("call Plugins.ProcessHostFile"));
      Done=CtrlObject->Plugins.ProcessHostFile(hNewPlugin,ItemList,ItemNumber,OPM_TOPLEVEL);
      /* DJ $ */
      _ALGO(SysLog("call Plugins.FreeFindData"));
      CtrlObject->Plugins.FreeFindData(hNewPlugin,ItemList,ItemNumber);
    }
    _ALGO(SysLog("call Plugins.ClosePlugin"));
    CtrlObject->Plugins.ClosePlugin(hNewPlugin);
  }
  return Done;
}



void FileList::SetPluginMode(HANDLE hPlugin,const wchar_t *PluginFile)
{
  if (PanelMode!=PLUGIN_PANEL)
  {
    CtrlObject->FolderHistory->AddToHistory(strCurDir,NULL,0);
  }

  PushPlugin(hPlugin,PluginFile);

  FileList::hPlugin=hPlugin;

  PanelMode=PLUGIN_PANEL;
  struct OpenPluginInfoW Info;
  CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
  if (Info.StartPanelMode)
    SetViewMode(VIEW_0+Info.StartPanelMode-'0');
  CtrlObject->Cp()->RedrawKeyBar();
  if (Info.StartSortMode)
  {
    SortMode=Info.StartSortMode-(SM_UNSORTED-UNSORTED);
    SortOrder=Info.StartSortOrder ? -1:1;
  }
  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
  if (AnotherPanel->GetType()!=FILE_PANEL)
  {
    AnotherPanel->Update(UPDATE_KEEP_SELECTION);
    AnotherPanel->Redraw();
  }
}


void FileList::PluginGetPanelInfo(struct PanelInfo *Info,int FullInfo)
{
  Info->PanelItems=NULL;
  Info->SelectedItems=NULL;

  if(FullInfo)
  {
    Info->ItemsNumber=0;
    Info->PanelItems=new PluginPanelItemW[FileCount+1];
    if (Info->PanelItems!=NULL)
    {
      struct FileListItem *CurPtr;
      for (int I=0; I < FileCount; I++)
      {
        CurPtr = ListData[I];
        FileListToPluginItem(CurPtr,Info->PanelItems+Info->ItemsNumber);
        Info->ItemsNumber++;
      }
    }
    CreatePluginItemList(Info->SelectedItems,Info->SelectedItemsNumber,FALSE);
  }
  else
  {
    Info->ItemsNumber=FileCount;
    Info->SelectedItemsNumber=GetSelCount();
  }

  Info->CurrentItem=CurFile;
  Info->TopPanelItem=CurTopFile;

  string strColumnTypes, strColumnWidths;
  ViewSettingsToText(ViewSettings.ColumnType,ViewSettings.ColumnWidth,
                     ViewSettings.ColumnCount,strColumnTypes,strColumnWidths);

  Info->lpwszColumnTypes = _wcsdup (strColumnTypes);
  Info->lpwszColumnWidths = _wcsdup (strColumnWidths);
  Info->ShortNames=ShowShortNames;
}

void FileList::PluginSetSelection(struct PanelInfo *Info)
{
  /* $ 06.07.2001 IS �������� ������ ��������� */
  SaveSelection();
  /* IS $ */
  struct FileListItem *CurPtr;
  for (int I=0; I < FileCount && I < Info->ItemsNumber; I++)
  {
    CurPtr = ListData[I];
    int Selection=(Info->PanelItems[I].Flags & PPIF_SELECTED)!=0;
    Select(CurPtr,Selection);
  }
  if (SelectedFirst)
    SortFileList(TRUE);
}


void FileList::ProcessPluginCommand()
{
  _ALGO(CleverSysLog clv("FileList::ProcessPluginCommand"));
  _ALGO(SysLog("PanelMode=%s",(PanelMode==PLUGIN_PANEL?"PLUGIN_PANEL":"NORMAL_PANEL")));
  int Command=PluginCommand;
  PluginCommand=-1;
  if (PanelMode==PLUGIN_PANEL)
    switch(Command)
    {
      case FCTL_CLOSEPLUGIN:
        _ALGO(SysLog("Command=FCTL_CLOSEPLUGIN"));
        SetCurDirW((const wchar_t *)PluginParam,TRUE);
        if(!PluginParam || !*(const wchar_t *)PluginParam)
          Update(UPDATE_KEEP_SELECTION);
        Redraw();
        break;
    }
}

void FileList::SetPluginModified()
{
  if (PluginsStackSize>0)
    PluginsStack[PluginsStackSize-1]->Modified=TRUE;
}


HANDLE FileList::GetPluginHandle()
{
  return(hPlugin);
}


int FileList::ProcessPluginEvent(int Event,void *Param)
{
  if (PanelMode==PLUGIN_PANEL)
    return(CtrlObject->Plugins.ProcessEvent(hPlugin,Event,Param));
  return(FALSE);
}


void FileList::PluginClearSelection(struct PluginPanelItemW *ItemList,int ItemNumber)
{
  /* $ 06.07.2001 IS �������� ������ ��������� */
  SaveSelection();
  /* IS $ */
  int FileNumber=0,PluginNumber=0;
  while (PluginNumber<ItemNumber)
  {
    struct PluginPanelItemW *CurPluginPtr=ItemList+PluginNumber;
    if ((CurPluginPtr->Flags & PPIF_SELECTED)==0)
    {
      while (LocalStricmpW(CurPluginPtr->FindData.lpwszFileName,ListData[FileNumber]->strName)!=0)
        if (++FileNumber>=FileCount)
          return;
      Select(ListData[FileNumber++],0);
    }
    PluginNumber++;
  }
}
