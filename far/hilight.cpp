#include "common.hpp"

HighlightFiles::HighlightFiles()
{
  HiData=NULL;
  HiDataCount=0;
  while (1)
  {
    char RegKey[80],Mask[sizeof(HiData->Masks)];
    sprintf(RegKey,"Highlight\\Group%d",HiDataCount);
    if (!GetRegKey(RegKey,"Mask",Mask,"",sizeof(Mask)))
      break;
    struct HighlightData *NewHiData,*CurHiData;
    if ((NewHiData=(struct HighlightData *)realloc(HiData,sizeof(*HiData)*(HiDataCount+1)))==NULL)
      break;
    HiData=NewHiData;
    CurHiData=&HiData[HiDataCount];
    memset(CurHiData,0,sizeof(*CurHiData));
    strcpy(CurHiData->Masks,Mask);
    CurHiData->IncludeAttr=GetRegKey(RegKey,"IncludeAttributes",0);
    CurHiData->ExcludeAttr=GetRegKey(RegKey,"ExcludeAttributes",0);
    CurHiData->Color=GetRegKey(RegKey,"NormalColor",0);
    CurHiData->SelColor=GetRegKey(RegKey,"SelectedColor",0);
    CurHiData->CursorColor=GetRegKey(RegKey,"CursorColor",0);
    CurHiData->CursorSelColor=GetRegKey(RegKey,"SelectedCursorColor",0);
    CurHiData->MarkChar=GetRegKey(RegKey,"MarkChar",0);
    HiDataCount++;
  }
  StartHiDataCount=HiDataCount;
}


HighlightFiles::~HighlightFiles()
{
  delete(HiData);
}


void HighlightFiles::GetHiColor(char *Path,int Attr,unsigned char &Color,
     unsigned char &SelColor,unsigned char &CursorColor,
     unsigned char &CursorSelColor,unsigned char &MarkChar)
{
  Color=SelColor=CursorColor=CursorSelColor=MarkChar=0;
  for (int I=0;I<HiDataCount;I++)
  {
    struct HighlightData *CurHiData=&HiData[I];
    if ((Attr & CurHiData->IncludeAttr)==CurHiData->IncludeAttr &&
        (Attr & CurHiData->ExcludeAttr)==0)
    {
      char ArgName[NM],*NamePtr=CurHiData->Masks;
      while ((NamePtr=GetCommaWord(NamePtr,ArgName))!=NULL)
        if (Path==NULL && (strcmp(ArgName,"*")==0 || strcmp(ArgName,"*.*")==0) ||
            Path!=NULL && CmpName(ArgName,Path))
        {
          if (Path!=NULL && Path[0]=='.' && Path[1]=='.' && Path[2]==0 &&
              strcmp(ArgName,"..")!=0)
            continue;
          Color=CurHiData->Color;
          SelColor=CurHiData->SelColor;
          CursorColor=CurHiData->CursorColor;
          CursorSelColor=CurHiData->CursorSelColor;
          MarkChar=CurHiData->MarkChar;
          return;
        }
    }
  }
}


void HighlightFiles::HiEdit(int MenuPos)
{
  struct MenuItem HiMenuItem;
  HiMenuItem.Checked=HiMenuItem.Separator=*HiMenuItem.UserData=HiMenuItem.UserDataSize=0;

  {
    VMenu HiMenu(MSG(MHighlightTitle),NULL,0,ScrY-4);
    HiMenu.SetHelp("Highlight");
    HiMenu.SetFlags(MENU_WRAPMODE|MENU_SHOWAMPERSAND);
    HiMenu.SetPosition(-1,-1,0,0);
    HiMenu.SetBottomTitle(MSG(MHighlightBottom));
    for (int I=0;I<HiDataCount;I++)
    {
      struct HighlightData *CurHiData=&HiData[I];
      sprintf(HiMenuItem.Name,"%c%c%c%c%c%c ³ %c%c%c%c%c%c ³ %.60s",
        (CurHiData->IncludeAttr & FILE_ATTRIBUTE_READONLY) ? 'R':'.',
        (CurHiData->IncludeAttr & FILE_ATTRIBUTE_HIDDEN) ? 'H':'.',
        (CurHiData->IncludeAttr & FILE_ATTRIBUTE_SYSTEM) ? 'S':'.',
        (CurHiData->IncludeAttr & FILE_ATTRIBUTE_ARCHIVE) ? 'A':'.',
        (CurHiData->IncludeAttr & FILE_ATTRIBUTE_COMPRESSED) ? 'C':'.',
        (CurHiData->IncludeAttr & FILE_ATTRIBUTE_DIRECTORY) ? 'F':'.',
        (CurHiData->ExcludeAttr & FILE_ATTRIBUTE_READONLY) ? 'R':'.',
        (CurHiData->ExcludeAttr & FILE_ATTRIBUTE_HIDDEN) ? 'H':'.',
        (CurHiData->ExcludeAttr & FILE_ATTRIBUTE_SYSTEM) ? 'S':'.',
        (CurHiData->ExcludeAttr & FILE_ATTRIBUTE_ARCHIVE) ? 'A':'.',
        (CurHiData->ExcludeAttr & FILE_ATTRIBUTE_COMPRESSED) ? 'C':'.',
        (CurHiData->ExcludeAttr & FILE_ATTRIBUTE_DIRECTORY) ? 'F':'.',
        CurHiData->Masks);
      HiMenuItem.Selected=(I==MenuPos);
      HiMenu.AddItem(&HiMenuItem);
    }
    *HiMenuItem.Name=0;
    HiMenuItem.Selected=(HiDataCount==MenuPos);
    HiMenu.AddItem(&HiMenuItem);

    {
      HiMenu.Show();
      while (1)
      {
        Panel *LeftPanel=CtrlObject->LeftPanel;
        Panel *RightPanel=CtrlObject->RightPanel;
        while (!HiMenu.Done())
        {
          int SelectPos=HiMenu.GetSelectPos();
          int ItemCount=HiMenu.GetItemCount();
          switch(HiMenu.ReadInput())
          {
            case KEY_DEL:
              if (SelectPos<HiMenu.GetItemCount()-1)
              {
                if (Message(MSG_WARNING,2,MSG(MHighlightTitle),
                            MSG(MHighlightAskDel),HiData[SelectPos].Masks,
                            MSG(MDelete),MSG(MCancel))!=0)
                  break;
                for (int I=SelectPos+1;I<ItemCount;I++)
                  HiData[I-1]=HiData[I];
                HiDataCount--;
                HiData=(struct HighlightData *)realloc(HiData,sizeof(*HiData)*(HiDataCount+1));
                HiMenu.Hide();
                SaveHiData();
                LeftPanel->Update(UPDATE_KEEP_SELECTION);
                LeftPanel->Redraw();
                RightPanel->Update(UPDATE_KEEP_SELECTION);
                RightPanel->Redraw();
                HiEdit(SelectPos);
                return;
              }
              break;
            case KEY_INS:
              if (EditRecord(SelectPos,TRUE))
              {
                HiMenu.Hide();
                SaveHiData();
                LeftPanel->Update(UPDATE_KEEP_SELECTION);
                LeftPanel->Redraw();
                RightPanel->Update(UPDATE_KEEP_SELECTION);
                RightPanel->Redraw();
                HiEdit(SelectPos);
                return;
              }
              break;
            case KEY_ENTER:
            case KEY_F4:
              if (SelectPos<HiMenu.GetItemCount()-1)
                if (EditRecord(SelectPos,FALSE))
                {
                  HiMenu.Hide();
                  SaveHiData();
                  LeftPanel->Update(UPDATE_KEEP_SELECTION);
                  LeftPanel->Redraw();
                  RightPanel->Update(UPDATE_KEEP_SELECTION);
                  RightPanel->Redraw();
                  HiEdit(SelectPos);
                  return;
                }
              break;
            default:
              HiMenu.ProcessInput();
              break;
          }
        }
        if (HiMenu.GetExitCode()!=-1)
        {
          HiMenu.ClearDone();
          HiMenu.WriteInput(KEY_F4);
          continue;
        }
        break;
      }
    }
  }
}


void HighlightFiles::SaveHiData()
{
  int I;
  for (I=0;I<HiDataCount;I++)
  {
    struct HighlightData *CurHiData=&HiData[I];
    char RegKey[80];
    sprintf(RegKey,"Highlight\\Group%d",I);
    SetRegKey(RegKey,"Mask",CurHiData->Masks);
    SetRegKey(RegKey,"IncludeAttributes",CurHiData->IncludeAttr);
    SetRegKey(RegKey,"ExcludeAttributes",CurHiData->ExcludeAttr);
    SetRegKey(RegKey,"NormalColor",CurHiData->Color);
    SetRegKey(RegKey,"SelectedColor",CurHiData->SelColor);
    SetRegKey(RegKey,"CursorColor",CurHiData->CursorColor);
    SetRegKey(RegKey,"SelectedCursorColor",CurHiData->CursorSelColor);
    SetRegKey(RegKey,"MarkChar",CurHiData->MarkChar);
  }
  for (I=HiDataCount;I<StartHiDataCount;I++)
  {
    char RegKey[80];
    sprintf(RegKey,"Highlight\\Group%d",I);
    DeleteRegKey(RegKey);
  }
}


int HighlightFiles::EditRecord(int RecPos,int New)
{
  const char *HistoryName="Masks";
  static struct DialogData HiEditDlgData[]={
    DI_DOUBLEBOX,3,1,72,19,0,0,0,0,(char *)MHighlightEditTitle,
    DI_TEXT,5,2,0,0,0,0,0,0,(char *)MHighlightMasks,
    DI_EDIT,5,3,70,3,1,(DWORD)HistoryName,DIF_HISTORY,0,"",
    DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_TEXT,5,5,0,0,0,0,DIF_BOXCOLOR,0,(char *)MHighlightIncludeAttr,
    DI_CHECKBOX,5,6,0,0,0,0,0,0,(char *)MHighlightRO,
    DI_CHECKBOX,5,7,0,0,0,0,0,0,(char *)MHighlightHidden,
    DI_CHECKBOX,5,8,0,0,0,0,0,0,(char *)MHighlightSystem,
    DI_CHECKBOX,5,9,0,0,0,0,0,0,(char *)MHighlightArchive,
    DI_CHECKBOX,5,10,0,0,0,0,0,0,(char *)MHighlightCompressed,
    DI_CHECKBOX,5,11,0,0,0,0,0,0,(char *)MHighlightFolder,
    DI_TEXT,37,5,0,0,0,0,DIF_BOXCOLOR,0,(char *)MHighlightExcludeAttr,
    DI_CHECKBOX,37,6,0,0,0,0,0,0,(char *)MHighlightRO,
    DI_CHECKBOX,37,7,0,0,0,0,0,0,(char *)MHighlightHidden,
    DI_CHECKBOX,37,8,0,0,0,0,0,0,(char *)MHighlightSystem,
    DI_CHECKBOX,37,9,0,0,0,0,0,0,(char *)MHighlightArchive,
    DI_CHECKBOX,37,10,0,0,0,0,0,0,(char *)MHighlightCompressed,
    DI_CHECKBOX,37,11,0,0,0,0,0,0,(char *)MHighlightFolder,
    DI_TEXT,-1,12,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,(char *)MHighlightColors,
    DI_BUTTON,5,13,0,0,0,0,0,0,(char *)MHighlightNormal,
    DI_BUTTON,5,14,0,0,0,0,0,0,(char *)MHighlightSelected,
    DI_BUTTON,37,13,0,0,0,0,0,0,(char *)MHighlightCursor,
    DI_BUTTON,37,14,0,0,0,0,0,0,(char *)MHighlightSelectedCursor,
    DI_TEXT,3,15,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_TEXT,7,16,0,0,0,0,0,0,(char *)MHighlightMarkChar,
    DI_FIXEDIT,5,16,5,17,0,0,0,0,"",
    DI_TEXT,3,17,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
    DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };
  MakeDialogItems(HiEditDlgData,HiEditDlg);
  struct HighlightData EditData;
  int ExitCode=0;

  if (!New && RecPos<HiDataCount)
    EditData=HiData[RecPos];
  else
    memset(&EditData,0,sizeof(EditData));

  strcpy(HiEditDlg[2].Data,EditData.Masks);
  HiEditDlg[5].Selected=(EditData.IncludeAttr & FILE_ATTRIBUTE_READONLY)!=0;
  HiEditDlg[6].Selected=(EditData.IncludeAttr & FILE_ATTRIBUTE_HIDDEN)!=0;
  HiEditDlg[7].Selected=(EditData.IncludeAttr & FILE_ATTRIBUTE_SYSTEM)!=0;
  HiEditDlg[8].Selected=(EditData.IncludeAttr & FILE_ATTRIBUTE_ARCHIVE)!=0;
  HiEditDlg[9].Selected=(EditData.IncludeAttr & FILE_ATTRIBUTE_COMPRESSED)!=0;
  HiEditDlg[10].Selected=(EditData.IncludeAttr & FILE_ATTRIBUTE_DIRECTORY)!=0;
  HiEditDlg[12].Selected=(EditData.ExcludeAttr & FILE_ATTRIBUTE_READONLY)!=0;
  HiEditDlg[13].Selected=(EditData.ExcludeAttr & FILE_ATTRIBUTE_HIDDEN)!=0;
  HiEditDlg[14].Selected=(EditData.ExcludeAttr & FILE_ATTRIBUTE_SYSTEM)!=0;
  HiEditDlg[15].Selected=(EditData.ExcludeAttr & FILE_ATTRIBUTE_ARCHIVE)!=0;
  HiEditDlg[16].Selected=(EditData.ExcludeAttr & FILE_ATTRIBUTE_COMPRESSED)!=0;
  HiEditDlg[17].Selected=(EditData.ExcludeAttr & FILE_ATTRIBUTE_DIRECTORY)!=0;

  *HiEditDlg[25].Data=EditData.MarkChar;

  while (ExitCode!=27)
  {
    SaveScreen SaveScr;
    Dialog Dlg(HiEditDlg,sizeof(HiEditDlg)/sizeof(HiEditDlg[0]));
    Dlg.SetHelp("Highlight");
    Dlg.SetPosition(-1,-1,76,21);
    Dlg.Process();
    if ((ExitCode=Dlg.GetExitCode())<0)
      return(FALSE);
    switch(ExitCode)
    {
      case 28:
        return(FALSE);
      case 19:
        GetColorDialog(EditData.Color);
        break;
      case 20:
        GetColorDialog(EditData.SelColor);
        break;
      case 21:
        GetColorDialog(EditData.CursorColor);
        break;
      case 22:
        GetColorDialog(EditData.CursorSelColor);
        break;
    }
    Dlg.InitDialogObjects();
  }
  if (*HiEditDlg[2].Data==0)
    return(FALSE);
  strcpy(EditData.Masks,HiEditDlg[2].Data);
  EditData.IncludeAttr=EditData.ExcludeAttr=0;
  if (HiEditDlg[5].Selected)
    EditData.IncludeAttr|=FILE_ATTRIBUTE_READONLY;
  if (HiEditDlg[6].Selected)
    EditData.IncludeAttr|=FILE_ATTRIBUTE_HIDDEN;
  if (HiEditDlg[7].Selected)
    EditData.IncludeAttr|=FILE_ATTRIBUTE_SYSTEM;
  if (HiEditDlg[8].Selected)
    EditData.IncludeAttr|=FILE_ATTRIBUTE_ARCHIVE;
  if (HiEditDlg[9].Selected)
    EditData.IncludeAttr|=FILE_ATTRIBUTE_COMPRESSED;
  if (HiEditDlg[10].Selected)
    EditData.IncludeAttr|=FILE_ATTRIBUTE_DIRECTORY;
  if (HiEditDlg[12].Selected)
    EditData.ExcludeAttr|=FILE_ATTRIBUTE_READONLY;
  if (HiEditDlg[13].Selected)
    EditData.ExcludeAttr|=FILE_ATTRIBUTE_HIDDEN;
  if (HiEditDlg[14].Selected)
    EditData.ExcludeAttr|=FILE_ATTRIBUTE_SYSTEM;
  if (HiEditDlg[15].Selected)
    EditData.ExcludeAttr|=FILE_ATTRIBUTE_ARCHIVE;
  if (HiEditDlg[16].Selected)
    EditData.ExcludeAttr|=FILE_ATTRIBUTE_COMPRESSED;
  if (HiEditDlg[17].Selected)
    EditData.ExcludeAttr|=FILE_ATTRIBUTE_DIRECTORY;
  EditData.MarkChar=*HiEditDlg[25].Data;
  if (!New && RecPos<HiDataCount)
    HiData[RecPos]=EditData;
  if (New)
  {
    struct HighlightData *NewHiData;
    HiDataCount++;
    if ((NewHiData=(struct HighlightData *)realloc(HiData,sizeof(*HiData)*HiDataCount))==NULL)
      return(FALSE);
    HiData=NewHiData;
    for (int I=HiDataCount-1;I>RecPos;I--)
      HiData[I]=HiData[I-1];
    HiData[RecPos]=EditData;
  }
  return(TRUE);
}
