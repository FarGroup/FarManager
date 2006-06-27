/*
hilight.cpp

Files highlighting

*/

/* Revision: 1.58 25.05.2006 $ */

#include "headers.hpp"
#pragma hdrstop

#include "colors.hpp"
#include "struct.hpp"
#include "hilight.hpp"
#include "fn.hpp"
#include "global.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "vmenu.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "filelist.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"

#define HIGHLIGHT_MASK_SIZE      2048

static struct HighlightDataColor WorkColor;

/* $ 25.09.2001 IS
     Тут храним строковые константы для "раскраски файлов"
*/
struct HighlightStrings
{
  wchar_t *IncludeAttributes,*ExcludeAttributes,*Mask,*IgnoreMask,
       *NormalColor,*SelectedColor,*CursorColor,*SelectedCursorColor,
       *MarkChar,*HighlightEdit,*HighlightList;
};
const HighlightStrings HLS=
{
  L"IncludeAttributes",L"ExcludeAttributes",L"Mask",L"IgnoreMask",
  L"NormalColor",L"SelectedColor",L"CursorColor",L"SelectedCursorColor",
  L"MarkChar",L"HighlightEdit",L"HighlightList"
};
/* IS $ */

HighlightFiles::HighlightFiles()
{
  HiData=NULL;
  /* $ 07.07.2000 IS
    Весь код вынесен в отдельную функцию, чтобы можно было его использовать
    повторно
  */
  InitHighlightFiles();
  /* IS $ */
}

void HighlightFiles::InitHighlightFiles()
{
  if(HiData)
  {
    for (int i = 0; i < HiDataCount; i++)
        delete HiData[i];

    xf_free(HiData);
  }

  HiData=NULL;
  HiDataCount=0;
  string strRegKey, strMask;

  ReWriteWorkColor();
  while (1)
  {
    strRegKey.Format (L"%s\\Group%d", RegColorsHighlight, HiDataCount);

    if (!GetRegKeyW(strRegKey,HLS.Mask,strMask,L""))
      break;
    HighlightData **NewHiData;
    HighlightData *HData=new HighlightData;

    memset (HData, 0, sizeof (HighlightData));

    HData->IgnoreMask=GetRegKeyW(strRegKey,HLS.IgnoreMask,FALSE);

    if(AddMask(HData,strMask,HData->IgnoreMask))
    {
      if ((NewHiData=(HighlightData **)xf_realloc(HiData,4*(HiDataCount+1)))==NULL)
      {
        DeleteMask(HData);
        delete HData;
        break;
      }
      HiData=NewHiData;
      HData->IncludeAttr=GetRegKeyW(strRegKey,HLS.IncludeAttributes,0);
      HData->ExcludeAttr=GetRegKeyW(strRegKey,HLS.ExcludeAttributes,0)&(~HData->IncludeAttr);
      HData->Colors.Color=(BYTE)GetRegKeyW(strRegKey,HLS.NormalColor,(DWORD)WorkColor.Color);
      HData->Colors.SelColor=(BYTE)GetRegKeyW(strRegKey,HLS.SelectedColor,(DWORD)WorkColor.SelColor);
      HData->Colors.CursorColor=(BYTE)GetRegKeyW(strRegKey,HLS.CursorColor,(DWORD)WorkColor.CursorColor);
      HData->Colors.CursorSelColor=(BYTE)GetRegKeyW(strRegKey,HLS.SelectedCursorColor,(DWORD)WorkColor.CursorSelColor);
      HData->Colors.MarkChar=(BYTE)GetRegKeyW(strRegKey,HLS.MarkChar,0);
      HiData[HiDataCount] = HData;
      HiDataCount++;
    }
    else
      break;
  }
  StartHiDataCount=HiDataCount;
}


HighlightFiles::~HighlightFiles()
{
  ClearData();
}

/* $ 06.07.2001 IS "рабочей" маски теперь у нас нет */
// вернуть шоумаску - маску, которая редактируется и отображается на экране
const wchar_t *HighlightFiles::GetMask(int Idx)
{
  return HiData[Idx]->strOriginalMasks;
}
/* IS $ */

/* $ 01.05.2001 DJ
   оптимизированный формат хранения в Masks
*/
/* $ 06.07.2001 IS
   вместо "рабочей" маски используем соответствующий класс
*/
BOOL HighlightFiles::AddMask(HighlightData *Dest,const wchar_t *Mask,BOOL IgnoreMask,struct HighlightData *Src)
{
  wchar_t *Ptr;
  string strMask;

  if(Src)
    memmove(Dest,Src,sizeof(struct HighlightData));

  strMask = Mask;

  Ptr = strMask.GetBuffer();
  // проверим
  if((Ptr=wcschr(Ptr, L'%')) != NULL && !LocalStrnicmpW(Ptr,L"%PATHEXT%",9))
  {
    int IQ1=(*(Ptr+9) == L',')?10:9, offsetPtr=(Ptr-Mask)/sizeof (wchar_t);
    // Если встречается %pathext%, то допишем в конец...
    memmove(Ptr,Ptr+IQ1,(wcslen(Ptr+IQ1)+1)*sizeof (wchar_t));

  strMask.ReleaseBuffer();

    string strTmp1 = strMask;

  wchar_t *pSeparator, *lpwszTmp1;

  lpwszTmp1 = strTmp1.GetBuffer();

    pSeparator=wcschr(lpwszTmp1, EXCLUDEMASKSEPARATOR);

    if(pSeparator)
    {
      Ptr=lpwszTmp1+offsetPtr;
      if(Ptr>pSeparator) // PATHEXT находится в масках исключения
        Add_PATHEXT(strMask); // добавляем то, чего нету.
      else
      {
        string strTmp2;
        strTmp2 = (pSeparator+1);
        *pSeparator=0;

      strTmp1.ReleaseBuffer();

        Add_PATHEXT(strTmp1);
        strMask.Format (L"%s|%s", (const wchar_t*)strTmp1, (const wchar_t*)strTmp2);
      }


    }
    else
  {
      Add_PATHEXT(strMask); // добавляем то, чего нету.
    strTmp1.ReleaseBuffer();
  }
  }
  /* $ 25.09.2001 IS
     Если IgnoreMask, то не выделяем память под класс CFileMask, т.к. он нам
     не нужен.
  */
  // память под рабочую маску
  CFileMaskW *FMasks=NULL;
  if(!IgnoreMask)
  {
    FMasks=new CFileMaskW;

  if ( !FMasks )
      return FALSE;

    if(!FMasks->Set(strMask, FMF_SILENT)) // проверим корректность маски
    {
      delete FMasks;
      return FALSE;
    }
  }
  Dest->IgnoreMask=IgnoreMask;
  /* IS $ */

  // корректирем ссылки на маски.
  Dest->FMasks=FMasks;
  Dest->strOriginalMasks=Mask;
  return TRUE;
}
/* IS $ */
/* DJ $ */

/* $ 06.07.2001 IS вместо "рабочей" маски используем соответствующий класс */
void HighlightFiles::DeleteMask(struct HighlightData *CurHighlightData)
{
  if(CurHighlightData->FMasks)
  {
    delete CurHighlightData->FMasks;
    CurHighlightData->FMasks=NULL;
  }

  CurHighlightData->strOriginalMasks=L"";
}
/* IS $ */

void HighlightFiles::ClearData()
{
  if(HiData)
  {
    for(int I=0; I < HiDataCount; ++I)
    {
      DeleteMask(HiData[I]);
      delete HiData[I];
    }

    xf_free(HiData);
  }
  HiData=NULL;
  HiDataCount=0;
}

/* $ 01.05.2001 DJ
   оптимизированный формат хранения Masks
*/
/* $ 06.07.2001 IS вместо "рабочей" маски используем соответствующий класс */
/* $ 25.09.2001 IS
   Узаконим следующее положение при проверке на совпадение с группой
   раскраски: если параметр Path равен NULL, то маски в анализе не
   используются, проверяются только атрибуты, причем в этом случае выбор
   происходит среди тех групп, у которых маски исключены из анализа.
*/
void HighlightFiles::GetHiColor(const wchar_t *Path,int Attr,
                                struct HighlightDataColor *Colors)
{
  struct FileListItem *FileItem = new FileListItem;
  if(Path)
    FileItem->strName = Path;
  else
    FileItem->strName=L"";
  FileItem->FileAttr=Attr;
  GetHiColor(&FileItem,1);
  memcpy(Colors,&FileItem->Colors,sizeof(struct HighlightDataColor));

  delete FileItem;
}
/* IS $ */
/* IS $ */
/* DJ $ */

void HighlightFiles::GetHiColor(struct FileListItem **FileItemEx,int FileCount)
{
  if(!FileItemEx || !FileCount)
    return;

  HighlightData *CurHiData;
  struct HighlightDataColor Colors;
  int I, FCnt;
  ReWriteWorkColor(&Colors);

  FileListItem *FileItem;
  //Path=Path?Path:""; // если Path==NULL, то считаем, что это пустая строка

  for(FCnt=0; FCnt < FileCount; ++FCnt)
  {
    FileItem = FileItemEx[FCnt];
    DWORD Attr=FileItem->FileAttr;
    string strPath = FileItem->strName;
    memcpy(&FileItem->Colors,&Colors,sizeof(struct HighlightDataColor));
    for (I=0; I < HiDataCount;I++)
    {
        CurHiData = HiData[I];
      if ((Attr & CurHiData->IncludeAttr)==CurHiData->IncludeAttr &&
          (Attr & CurHiData->ExcludeAttr)==0)
      {
        if(CurHiData->IgnoreMask || (!strPath.IsEmpty() && CurHiData->FMasks->Compare(strPath)))
        {
          memcpy(&FileItem->Colors,&CurHiData->Colors,sizeof(struct HighlightDataColor));
          break;
        }
      }
    }
  }
}

void HighlightFiles::ReWriteWorkColor(struct HighlightDataColor *Colors)
{
#if 0
  WorkColor.Color=FarColorToReal(COL_PANELTEXT);
  WorkColor.SelColor=FarColorToReal(COL_PANELSELECTEDTEXT);
  WorkColor.CursorColor=FarColorToReal(COL_PANELCURSOR);
  WorkColor.CursorSelColor=FarColorToReal(COL_PANELSELECTEDCURSOR);
  WorkColor.MarkChar=0;
  if(Colors)
    *Colors=WorkColor;
#else
  if(Colors)
    memset(Colors,0,sizeof(struct HighlightDataColor));
#endif
}

void HighlightFiles::FillMenu(VMenu *HiMenu,int MenuPos)
{
  MenuItemEx HiMenuItem;

  HiMenu->DeleteItems();

  /* $ 22.01.2003 IS
     Символ для пометки файлов показываем в кавычках, чтобы можно было
     отличить пробел от пустоты
  */
  wchar_t MarkChar[]=L"\" \"";
  int I, Short=1;
  // сначала проверим - а есть ли символы пометки файлов в меню вообще?
  for (I=0;I<HiDataCount;I++)
  {
    if(HiData[I]->Colors.MarkChar)
    {
      Short=0;
      break;
    }
  }
  // если символов пометки в меню нет, то отводим под это поле только 1 знакоместо
  const wchar_t *emptyMarkChar=Short?L" ":L"   ";
  for (I=0;I<HiDataCount;I++)
  {
    HighlightData *CurHiData=HiData[I];
    MarkChar[1]=CurHiData->Colors.MarkChar;

    HiMenuItem.Clear ();
    HiMenuItem.strName.Format (L"%s %c %c%c%c%c%c%c%c%c%c%c %c %c%c%c%c%c%c%c%c%c%c %c %.54s",
      // добавим показ символа в кавычках
      (CurHiData->Colors.MarkChar?MarkChar:emptyMarkChar),
  /* IS $ */
      VerticalLine,

       (CurHiData->IncludeAttr & FILE_ATTRIBUTE_READONLY) ? L'R':L'.',
       (CurHiData->IncludeAttr & FILE_ATTRIBUTE_HIDDEN) ? L'H':L'.',
       (CurHiData->IncludeAttr & FILE_ATTRIBUTE_SYSTEM) ? L'S':L'.',
       (CurHiData->IncludeAttr & FILE_ATTRIBUTE_ARCHIVE) ? L'A':L'.',
       (CurHiData->IncludeAttr & FILE_ATTRIBUTE_COMPRESSED) ? L'C':
         ((CurHiData->IncludeAttr & FILE_ATTRIBUTE_ENCRYPTED)?L'E':L'.'),
       (CurHiData->IncludeAttr & FILE_ATTRIBUTE_DIRECTORY) ? L'F':L'.',
       (CurHiData->IncludeAttr & FILE_ATTRIBUTE_REPARSE_POINT) ? L'L':L'.',
      (CurHiData->IncludeAttr & FILE_ATTRIBUTE_SPARSE_FILE) ? L'$':L'.',
      (CurHiData->IncludeAttr & FILE_ATTRIBUTE_TEMPORARY) ? L'T':L'.',
      (CurHiData->IncludeAttr & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED) ? L'I':L'.',

        VerticalLine,

       (CurHiData->ExcludeAttr & FILE_ATTRIBUTE_READONLY) ? L'R':L'.',
       (CurHiData->ExcludeAttr & FILE_ATTRIBUTE_HIDDEN) ? L'H':L'.',
       (CurHiData->ExcludeAttr & FILE_ATTRIBUTE_SYSTEM) ? L'S':L'.',
       (CurHiData->ExcludeAttr & FILE_ATTRIBUTE_ARCHIVE) ? L'A':L'.',
       (CurHiData->ExcludeAttr & FILE_ATTRIBUTE_COMPRESSED) ? L'C':
         ((CurHiData->ExcludeAttr & FILE_ATTRIBUTE_ENCRYPTED)?L'E':L'.'),
       (CurHiData->ExcludeAttr & FILE_ATTRIBUTE_DIRECTORY) ? L'F':L'.',
       (CurHiData->ExcludeAttr & FILE_ATTRIBUTE_REPARSE_POINT) ? L'L':L'.',
      (CurHiData->ExcludeAttr & FILE_ATTRIBUTE_SPARSE_FILE) ? L'$':L'.',
      (CurHiData->ExcludeAttr & FILE_ATTRIBUTE_TEMPORARY) ? L'T':L'.',
      (CurHiData->ExcludeAttr & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED) ? L'I':L'.',

      VerticalLine,

      /* $ 25.09.2001 IS не рисуем маски, если они игнорируются */
      CurHiData->IgnoreMask?L" ":GetMask(I));
      /* IS $ */
    HiMenuItem.SetSelect(I==MenuPos);
    HiMenu->AddItemW(&HiMenuItem);
  }
  HiMenuItem.strName=L"";
  HiMenuItem.SetSelect(HiDataCount==MenuPos);
  HiMenu->AddItemW(&HiMenuItem);
}

void HighlightFiles::HiEdit(int MenuPos)
{
  VMenu HiMenu(UMSG(MHighlightTitle),NULL,0, TRUE,ScrY-4);
  HiMenu.SetHelp(HLS.HighlightList);
  HiMenu.SetFlags(VMENU_WRAPMODE|VMENU_SHOWAMPERSAND);
  HiMenu.SetPosition(-1,-1,0,0);
  HiMenu.SetBottomTitle(UMSG(MHighlightBottom));

  FillMenu(&HiMenu,MenuPos);

  int NeedUpdate;
  Panel *LeftPanel=CtrlObject->Cp()->LeftPanel;
  Panel *RightPanel=CtrlObject->Cp()->RightPanel;

  HiMenu.Show();
  while (1)
  {
    while (!HiMenu.Done())
    {
      int SelectPos=HiMenu.GetSelectPos();
      int ItemCount=HiMenu.GetItemCount();
      int Key;

      NeedUpdate=FALSE;
      Key=HiMenu.ReadInput();
      switch(Key)
      {
        /* $ 07.07.2000 IS
          Если нажали ctrl+r, то восстановить значения по умолчанию.
        */
        case KEY_CTRLR:
          if (MessageW(MSG_WARNING,2,UMSG(MHighlightTitle),
                        UMSG(MHighlightWarning),UMSG(MHighlightAskRestore),
                        UMSG(MYes),UMSG(MCancel))!=0)
             break;
          DeleteKeyTreeW(RegColorsHighlight);
          SetHighlighting();
          HiMenu.Hide();
          ClearData();
          InitHighlightFiles();
          NeedUpdate=TRUE;
          break;
        /* IS $ */
        case KEY_DEL:
          if (SelectPos<HiMenu.GetItemCount()-1)
          {
            if (MessageW(MSG_WARNING,2,UMSG(MHighlightTitle),
                        UMSG(MHighlightAskDel),GetMask(SelectPos),
                        UMSG(MDelete),UMSG(MCancel))!=0)
              break;
            DeleteMask(HiData[SelectPos]);
            for (int I=SelectPos+1;I<ItemCount;I++)
              HiData[I-1]=HiData[I];
            HiDataCount--;
            HiData=(HighlightData **)xf_realloc(HiData,4*(HiDataCount+1));
            NeedUpdate=TRUE;
          }
          break;
        case KEY_ENTER:
        case KEY_F4:
          if (SelectPos>=HiMenu.GetItemCount()-1)
            break;
        case KEY_INS: case KEY_NUMPAD0:
          if (EditRecord(SelectPos,Key == KEY_INS))
            NeedUpdate=TRUE;
          break;
        case KEY_F5:
          if (SelectPos < HiMenu.GetItemCount()-1)
          {
            if(DupHighlightData(HiData[SelectPos],GetMask(SelectPos),HiData[SelectPos]->IgnoreMask,SelectPos))
              NeedUpdate=TRUE;
          }
          break;
        case KEY_CTRLUP: case KEY_CTRLNUMPAD8:
          if (SelectPos > 0 && SelectPos < HiMenu.GetItemCount()-1)
          {
            HighlightData HData;
            memcpy(&HData,HiData[SelectPos],sizeof(struct HighlightData));
            memcpy(HiData[SelectPos],HiData[SelectPos-1],sizeof(struct HighlightData));
            memcpy(HiData[SelectPos-1],&HData,sizeof(struct HighlightData));
            HiMenu.SetSelection(--SelectPos);
            NeedUpdate=TRUE;
            break;
          }
          HiMenu.ProcessInput();
          break;

        case KEY_CTRLDOWN: case KEY_CTRLNUMPAD2:
          if (SelectPos < HiMenu.GetItemCount()-2)
          {
            HighlightData HData;
            memcpy(&HData,HiData[SelectPos],sizeof(struct HighlightData));
            memcpy(HiData[SelectPos],HiData[SelectPos+1],sizeof(struct HighlightData));
            memcpy(HiData[SelectPos+1],&HData,sizeof(struct HighlightData));
            HiMenu.SetSelection(++SelectPos);
            NeedUpdate=TRUE;
          }
          HiMenu.ProcessInput();
          break;

        default:
          HiMenu.ProcessInput();
          break;
      }
      // повторяющийся кусок!
      if(NeedUpdate)
      {
         ScrBuf.Lock(); // отменяем всякую прорисовку
         HiMenu.Hide();
         if(Opt.AutoSaveSetup)
           SaveHiData();
         //FrameManager->RefreshFrame(); // рефрешим

         LeftPanel->Update(UPDATE_KEEP_SELECTION);
         LeftPanel->Redraw();
         RightPanel->Update(UPDATE_KEEP_SELECTION);
         RightPanel->Redraw();

         FillMenu(&HiMenu,MenuPos=SelectPos);
         HiMenu.Show();
         ScrBuf.Unlock(); // разрешаем прорисовку
      }
    }
    if (HiMenu.Modal::GetExitCode()!=-1)
    {
      HiMenu.ClearDone();
      HiMenu.WriteInput(KEY_F4);
      continue;
    }
    break;
  }
}


void HighlightFiles::SaveHiData()
{
  int I;
  string strRegKey;
  for (I=0;I<HiDataCount;I++)
  {
    struct HighlightData *CurHiData=HiData[I];
    strRegKey.Format (L"%s\\Group%d", RegColorsHighlight, I);
    SetRegKeyW(strRegKey,HLS.Mask,GetMask(I));
    SetRegKeyW(strRegKey,HLS.IgnoreMask,CurHiData->IgnoreMask);
    SetRegKeyW(strRegKey,HLS.IncludeAttributes,CurHiData->IncludeAttr);
    SetRegKeyW(strRegKey,HLS.ExcludeAttributes,CurHiData->ExcludeAttr);
    SetRegKeyW(strRegKey,HLS.NormalColor,(DWORD)CurHiData->Colors.Color);
    SetRegKeyW(strRegKey,HLS.SelectedColor,(DWORD)CurHiData->Colors.SelColor);
    SetRegKeyW(strRegKey,HLS.CursorColor,(DWORD)CurHiData->Colors.CursorColor);
    SetRegKeyW(strRegKey,HLS.SelectedCursorColor,(DWORD)CurHiData->Colors.CursorSelColor);
    SetRegKeyW(strRegKey,HLS.MarkChar,(DWORD)CurHiData->Colors.MarkChar);
  }
  for (I=HiDataCount;I<StartHiDataCount;I++)
  {
    strRegKey.Format (L"%s\\Group%d", RegColorsHighlight, I);
    DeleteRegKeyW(strRegKey);
  }
}

enum enumHighlightEditRecords
{
  ID_HER_TITLE = 0,
  ID_HER_MATCHMASK,
  ID_HER_MASKEDIT,
  ID_HER_SEPARATOR1,
  ID_HER_ATTRBR,
  ID_HER_ATTRBH,
  ID_HER_ATTRBS,
  ID_HER_ATTRBA,
  ID_HER_ATTRBC,
  ID_HER_ATTRBE,
  ID_HER_ATTRBF,
  ID_HER_ATTRBL,
  ID_HER_ATTRBSP,
  ID_HER_ATTRBT,
  ID_HER_ATTRBNI,
  ID_HER_SEPARATOR2,
  ID_HER_MARK_TITLE,
  ID_HER_MARKEDIT,
  ID_HER_SEPARATOR3,
  ID_HER_NORMAL,
  ID_HER_SELECTED,
  ID_HER_CURSOR,
  ID_HER_SELECTEDCURSOR,
  ID_HER_COLOREXAMPLE,
  ID_HER_SEPARATOR4,
  ID_HER_OK,
  ID_HER_CANCEL
};

void HighlightDlgUpdateUserControl(CHAR_INFO *VBufColorExample, struct HighlightDataColor &Colors)
{
  const wchar_t *ptr;
  DWORD Color;
  DWORD Default=F_BLACK|B_BLACK;
  for (int j=0; j<4; j++)
  {
    switch (j)
    {
      case 0:
        Color=(DWORD)Colors.Color;
        if (Color==Default)
          Color=(DWORD)Palette[COL_PANELTEXT-COL_FIRSTPALETTECOLOR];
        break;
      case 1:
        Color=(DWORD)Colors.SelColor;
        if (Color==Default)
          Color=(DWORD)Palette[COL_PANELSELECTEDTEXT-COL_FIRSTPALETTECOLOR];
        break;
      case 2:
        Color=(DWORD)Colors.CursorColor;
        if (Color==Default)
          Color=(DWORD)Palette[COL_PANELCURSOR-COL_FIRSTPALETTECOLOR];
        break;
      case 3:
        Color=(DWORD)Colors.CursorSelColor;
        if (Color==Default)
          Color=(DWORD)Palette[COL_PANELSELECTEDCURSOR-COL_FIRSTPALETTECOLOR];
        break;
    }
    if (Colors.MarkChar)
      ptr=UMSG(MHighlightExample2);
    else
      ptr=UMSG(MHighlightExample1);
    for (int k=0; k<15; k++)
    {
      VBufColorExample[15*j+k].Char.UnicodeChar=ptr[k];
      VBufColorExample[15*j+k].Attributes=Color;
    }
    if (Colors.MarkChar)
      VBufColorExample[15*j+1].Char.UnicodeChar=Colors.MarkChar;
    VBufColorExample[15*j].Attributes=(DWORD)Palette[COL_PANELBOX-COL_FIRSTPALETTECOLOR];
    VBufColorExample[15*j+14].Attributes=(DWORD)Palette[COL_PANELBOX-COL_FIRSTPALETTECOLOR];
  }
}

/* $ 17.05.2001 DJ
   обработка взаимоисключений (вместо обработки в явном цикле диалога)
*/

static long WINAPI HighlightDlgProc(HANDLE hDlg, int Msg, int Param1, long Param2)
{
  switch (Msg)
  {
    case DN_BTNCLICK:
    case DN_MOUSECLICK:
      if((Msg==DN_BTNCLICK && Param1 >= ID_HER_NORMAL && Param1 <= ID_HER_SELECTEDCURSOR)
         || (Msg==DN_MOUSECLICK && Param1==ID_HER_COLOREXAMPLE && ((MOUSE_EVENT_RECORD *)Param2)->dwButtonState==FROM_LEFT_1ST_BUTTON_PRESSED))
      {
        HighlightData *EditData = (HighlightData *) Dialog::SendDlgMessage (hDlg, DM_GETDLGDATA, 0, 0);
        unsigned int Color;
        if (Msg==DN_MOUSECLICK)
          Param1 = ID_HER_NORMAL + ((MOUSE_EVENT_RECORD *)Param2)->dwMousePosition.Y;
        switch (Param1)
        {
          case ID_HER_NORMAL:
            Color=(DWORD)EditData->Colors.Color;
            break;
          case ID_HER_SELECTED:
            Color=(DWORD)EditData->Colors.SelColor;
            break;
          case ID_HER_CURSOR:
            Color=(DWORD)EditData->Colors.CursorColor;
            break;
          case ID_HER_SELECTEDCURSOR:
            Color=(DWORD)EditData->Colors.CursorSelColor;
            break;
        }
        GetColorDialog(Color);
        switch (Param1)
        {
          case ID_HER_NORMAL:
            EditData->Colors.Color=(BYTE)Color;
            break;
          case ID_HER_SELECTED:
            EditData->Colors.SelColor=(BYTE)Color;
            break;
          case ID_HER_CURSOR:
            EditData->Colors.CursorColor=(BYTE)Color;
            break;
          case ID_HER_SELECTEDCURSOR:
            EditData->Colors.CursorSelColor=(BYTE)Color;
            break;
        }
        FarDialogItem MarkChar, ColorExample;
        Dialog::SendDlgMessage(hDlg,DM_GETDLGITEM,ID_HER_MARKEDIT,(long)&MarkChar);
        Dialog::SendDlgMessage(hDlg,DM_GETDLGITEM,ID_HER_COLOREXAMPLE,(long)&ColorExample);

        if ( MarkChar.PtrData )
            EditData->Colors.MarkChar=*(MarkChar.PtrData);
        HighlightDlgUpdateUserControl(ColorExample.Param.VBuf,EditData->Colors);
        Dialog::SendDlgMessage(hDlg,DM_SETDLGITEM,ID_HER_COLOREXAMPLE,(long)&ColorExample);
        return TRUE;
      }
      break;

    case DN_EDITCHANGE:
      if (Param1 == ID_HER_MARKEDIT)
      {
        HighlightData *EditData = (HighlightData *) Dialog::SendDlgMessage (hDlg, DM_GETDLGDATA, 0, 0);
        FarDialogItem *MarkChar, ColorExample;
        MarkChar=(FarDialogItem *)Param2;
        Dialog::SendDlgMessage(hDlg,DM_GETDLGITEM,ID_HER_COLOREXAMPLE,(long)&ColorExample);

        if ( MarkChar->PtrData )
            EditData->Colors.MarkChar=*(MarkChar->PtrData);
        HighlightDlgUpdateUserControl(ColorExample.Param.VBuf,EditData->Colors);
        Dialog::SendDlgMessage(hDlg,DM_SETDLGITEM,ID_HER_COLOREXAMPLE,(long)&ColorExample);
        return TRUE;
      }
  }
  return Dialog::DefDlgProc (hDlg, Msg, Param1, Param2);
}

/* DJ $ */

/* $ 25.09.2001 IS
     Обработка IgnoreMask
*/
int HighlightFiles::EditRecord(int RecPos,int New)
{
  const wchar_t *HistoryName=L"Masks";
  static struct DialogDataEx HiEditDlgData[]={
  /* 00 */DI_DOUBLEBOX,3,1,65,20,0,0,0,0,(const wchar_t *)MHighlightEditTitle,
  /* 01 */DI_CHECKBOX,5,2,0,0,0,0,DIF_AUTOMATION,0,(const wchar_t *)MHighlightMasks,
  /* 02 */DI_EDIT,5,3,63,3,1,(DWORD)HistoryName,DIF_HISTORY,0,L"",
  /* 03 */DI_TEXT,-1,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,(const wchar_t *)MHighlightIncExcTitle,
  /* 04 */DI_CHECKBOX,5,5,0,0,0,0,DIF_3STATE,0,(const wchar_t *)MHighlightRO,
  /* 05 */DI_CHECKBOX,5,6,0,0,0,0,DIF_3STATE,0,(const wchar_t *)MHighlightHidden,
  /* 06 */DI_CHECKBOX,5,7,0,0,0,0,DIF_3STATE,0,(const wchar_t *)MHighlightSystem,
  /* 07 */DI_CHECKBOX,5,8,0,0,0,0,DIF_3STATE,0,(const wchar_t *)MHighlightArchive,
  /* 08 */DI_CHECKBOX,5,9,0,0,0,0,DIF_3STATE,0,(const wchar_t *)MHighlightCompressed,
  /* 09 */DI_CHECKBOX,5,10,0,0,0,0,DIF_3STATE,0,(const wchar_t *)MHighlightEncrypted,
  /* 10 */DI_CHECKBOX,35,5,0,0,0,0,DIF_3STATE,0,(const wchar_t *)MHighlightFolder,
  /* 11 */DI_CHECKBOX,35,6,0,0,0,0,DIF_3STATE,0,(const wchar_t *)MHighlightJunction,
  /* 12 */DI_CHECKBOX,35,7,0,0,0,0,DIF_3STATE,0,(const wchar_t *)MHighlightSparse,
  /* 13 */DI_CHECKBOX,35,8,0,0,0,0,DIF_3STATE,0,(const wchar_t *)MHighlightTemporary,
  /* 14 */DI_CHECKBOX,35,9,0,0,0,0,DIF_3STATE,0,(const wchar_t *)MHighlightNotIndexed,
  /* 15 */DI_TEXT,3,11,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 16 */DI_TEXT,7,12,0,0,0,0,0,0,(const wchar_t *)MHighlightMarkChar,
  /* 17 */DI_FIXEDIT,5,12,5,12,0,0,0,0,L"",
  /* 18 */DI_TEXT,-1,13,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,(const wchar_t *)MHighlightColors,
  /* 19 */DI_BUTTON,5,14,0,0,0,0,DIF_BTNNOCLOSE,0,(const wchar_t *)MHighlightNormal,
  /* 20 */DI_BUTTON,5,15,0,0,0,0,DIF_BTNNOCLOSE,0,(const wchar_t *)MHighlightSelected,
  /* 21 */DI_BUTTON,5,16,0,0,0,0,DIF_BTNNOCLOSE,0,(const wchar_t *)MHighlightCursor,
  /* 22 */DI_BUTTON,5,17,0,0,0,0,DIF_BTNNOCLOSE,0,(const wchar_t *)MHighlightSelectedCursor,
  /* 23 */DI_USERCONTROL,65-15-1,14,65-2,17,0,0,DIF_NOFOCUS,0,L"",
  /* 24 */DI_TEXT,3,18,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 25 */DI_BUTTON,0,19,0,0,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MOk,
  /* 26 */DI_BUTTON,0,19,0,0,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel
  };
  MakeDialogItemsEx(HiEditDlgData,HiEditDlg);
  HighlightData *EditData;
  CHAR_INFO VBufColorExample[15*4];

  string strMask;
  const wchar_t *Ptr;
  bool bNew = false;

  if (!New && RecPos<HiDataCount)
  {
    EditData=HiData[RecPos];
    if((Ptr=GetMask(RecPos)) != NULL)
      strMask = Ptr;
  }
  else
  {
    bNew = true;
    EditData = new HighlightData;
      memset(EditData,0,sizeof(HighlightData));
  }

  memset(VBufColorExample,0,sizeof(VBufColorExample));
  HighlightDlgUpdateUserControl(VBufColorExample,EditData->Colors);
  HiEditDlg[ID_HER_COLOREXAMPLE].VBuf=VBufColorExample;

  if(FALSE==(HiEditDlg[ID_HER_MATCHMASK].Selected=!EditData->IgnoreMask))
     HiEditDlg[ID_HER_MASKEDIT].Flags|=DIF_DISABLE;

  HiEditDlg[ID_HER_MASKEDIT].strData = strMask;

  if ((EditData->IncludeAttr & FILE_ATTRIBUTE_READONLY)!=0)
    HiEditDlg[ID_HER_ATTRBR].Selected=1;
  else if ((EditData->ExcludeAttr & FILE_ATTRIBUTE_READONLY)==0)
    HiEditDlg[ID_HER_ATTRBR].Selected=2;

  if ((EditData->IncludeAttr & FILE_ATTRIBUTE_HIDDEN)!=0)
    HiEditDlg[ID_HER_ATTRBH].Selected=1;
  else if ((EditData->ExcludeAttr & FILE_ATTRIBUTE_HIDDEN)==0)
    HiEditDlg[ID_HER_ATTRBH].Selected=2;

  if ((EditData->IncludeAttr & FILE_ATTRIBUTE_SYSTEM)!=0)
    HiEditDlg[ID_HER_ATTRBS].Selected=1;
  else if ((EditData->ExcludeAttr & FILE_ATTRIBUTE_SYSTEM)==0)
    HiEditDlg[ID_HER_ATTRBS].Selected=2;

  if ((EditData->IncludeAttr & FILE_ATTRIBUTE_ARCHIVE)!=0)
    HiEditDlg[ID_HER_ATTRBA].Selected=1;
  else if ((EditData->ExcludeAttr & FILE_ATTRIBUTE_ARCHIVE)==0)
    HiEditDlg[ID_HER_ATTRBA].Selected=2;

  if ((EditData->IncludeAttr & FILE_ATTRIBUTE_COMPRESSED)!=0)
    HiEditDlg[ID_HER_ATTRBC].Selected=1;
  else if ((EditData->ExcludeAttr & FILE_ATTRIBUTE_COMPRESSED)==0)
    HiEditDlg[ID_HER_ATTRBC].Selected=2;

  if ((EditData->IncludeAttr & FILE_ATTRIBUTE_ENCRYPTED)!=0)
    HiEditDlg[ID_HER_ATTRBE].Selected=1;
  else if ((EditData->ExcludeAttr & FILE_ATTRIBUTE_ENCRYPTED)==0)
    HiEditDlg[ID_HER_ATTRBE].Selected=2;

  if ((EditData->IncludeAttr & FILE_ATTRIBUTE_DIRECTORY)!=0)
    HiEditDlg[ID_HER_ATTRBF].Selected=1;
  else if ((EditData->ExcludeAttr & FILE_ATTRIBUTE_DIRECTORY)==0)
    HiEditDlg[ID_HER_ATTRBF].Selected=2;

  if ((EditData->IncludeAttr & FILE_ATTRIBUTE_REPARSE_POINT)!=0)
    HiEditDlg[ID_HER_ATTRBL].Selected=1;
  else if ((EditData->ExcludeAttr & FILE_ATTRIBUTE_REPARSE_POINT)==0)
    HiEditDlg[ID_HER_ATTRBL].Selected=2;

  if ((EditData->IncludeAttr & FILE_ATTRIBUTE_SPARSE_FILE)!=0)
    HiEditDlg[ID_HER_ATTRBSP].Selected=1;
  else if ((EditData->ExcludeAttr & FILE_ATTRIBUTE_SPARSE_FILE)==0)
    HiEditDlg[ID_HER_ATTRBSP].Selected=2;

  if ((EditData->IncludeAttr & FILE_ATTRIBUTE_TEMPORARY)!=0)
    HiEditDlg[ID_HER_ATTRBT].Selected=1;
  else if ((EditData->ExcludeAttr & FILE_ATTRIBUTE_TEMPORARY)==0)
    HiEditDlg[ID_HER_ATTRBT].Selected=2;

  if ((EditData->IncludeAttr & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)!=0)
    HiEditDlg[ID_HER_ATTRBNI].Selected=1;
  else if ((EditData->ExcludeAttr & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)==0)
    HiEditDlg[ID_HER_ATTRBNI].Selected=2;

  wchar_t *lpwszData = HiEditDlg[ID_HER_MARKEDIT].strData.GetBuffer (2);

  *lpwszData = EditData->Colors.MarkChar;

  HiEditDlg[ID_HER_MARKEDIT].strData.ReleaseBuffer ();

  /* $ 18.05.2001 DJ
     обработка взаимоисключений и кнопок перенесена в обработчик диалога
  */
  {
    Dialog Dlg(HiEditDlg,sizeof(HiEditDlg)/sizeof(HiEditDlg[0]),HighlightDlgProc,(long) &EditData);

    Dlg.SetHelp(HLS.HighlightEdit);
    Dlg.SetPosition(-1,-1,69,22);
    Dlg.SetAutomation(1,2,DIF_DISABLE,0,0,DIF_DISABLE);


    /* $ 06.07.2001 IS
       Проверим маску на корректность
    */
    CFileMaskW FMask;
    for(;;)
    {
      Dlg.ClearDone();
      Dlg.Process();
      if (Dlg.GetExitCode() != ID_HER_OK)
      {
        if ( bNew )
          delete EditData;
        return(FALSE);
      }

      strMask = HiEditDlg[ID_HER_MASKEDIT].strData;

      if((FALSE!=(EditData->IgnoreMask=!HiEditDlg[ID_HER_MATCHMASK].Selected)))
      {
        if ( strMask.IsEmpty())
          strMask = L"*"; // для красоты и во избежание неприятностей
        break; // не проверяем маску лишний раз
      }
      if ( HiEditDlg[ID_HER_MASKEDIT].strData.IsEmpty() )
      {
        if ( bNew )
          delete EditData;
        return(FALSE);
      }
      if(FMask.Set(HiEditDlg[ID_HER_MASKEDIT].strData, 0))
        break;
    }
    /* IS $ */
  }
  /* DJ $ */
  EditData->IncludeAttr=EditData->ExcludeAttr=0;

  if (HiEditDlg[ID_HER_ATTRBR].Selected==1)
    EditData->IncludeAttr|=FILE_ATTRIBUTE_READONLY;
  else if (HiEditDlg[ID_HER_ATTRBR].Selected==0)
    EditData->ExcludeAttr|=FILE_ATTRIBUTE_READONLY;

  if (HiEditDlg[ID_HER_ATTRBH].Selected==1)
    EditData->IncludeAttr|=FILE_ATTRIBUTE_HIDDEN;
  else if (HiEditDlg[ID_HER_ATTRBH].Selected==0)
    EditData->ExcludeAttr|=FILE_ATTRIBUTE_HIDDEN;

  if (HiEditDlg[ID_HER_ATTRBS].Selected==1)
    EditData->IncludeAttr|=FILE_ATTRIBUTE_SYSTEM;
  else if (HiEditDlg[ID_HER_ATTRBS].Selected==0)
    EditData->ExcludeAttr|=FILE_ATTRIBUTE_SYSTEM;

  if (HiEditDlg[ID_HER_ATTRBA].Selected==1)
    EditData->IncludeAttr|=FILE_ATTRIBUTE_ARCHIVE;
  else if (HiEditDlg[ID_HER_ATTRBA].Selected==0)
    EditData->ExcludeAttr|=FILE_ATTRIBUTE_ARCHIVE;

  if (HiEditDlg[ID_HER_ATTRBE].Selected==1)
    EditData->IncludeAttr|=FILE_ATTRIBUTE_ENCRYPTED;
  else if (HiEditDlg[ID_HER_ATTRBE].Selected==0)
    EditData->ExcludeAttr|=FILE_ATTRIBUTE_ENCRYPTED;
  if (HiEditDlg[ID_HER_ATTRBC].Selected==1)
  {
    EditData->IncludeAttr|=FILE_ATTRIBUTE_COMPRESSED;
    EditData->IncludeAttr&=~FILE_ATTRIBUTE_ENCRYPTED;
  }
  else if (HiEditDlg[ID_HER_ATTRBC].Selected==0)
  {
    EditData->ExcludeAttr|=FILE_ATTRIBUTE_COMPRESSED;
    EditData->ExcludeAttr&=~FILE_ATTRIBUTE_COMPRESSED;
  }

  if (HiEditDlg[ID_HER_ATTRBF].Selected==1)
    EditData->IncludeAttr|=FILE_ATTRIBUTE_DIRECTORY;
  else if (HiEditDlg[ID_HER_ATTRBF].Selected==0)
    EditData->ExcludeAttr|=FILE_ATTRIBUTE_DIRECTORY;

  if (HiEditDlg[ID_HER_ATTRBL].Selected==1)
    EditData->IncludeAttr|=FILE_ATTRIBUTE_REPARSE_POINT;
  else if (HiEditDlg[ID_HER_ATTRBL].Selected==0)
    EditData->ExcludeAttr|=FILE_ATTRIBUTE_REPARSE_POINT;

  if (HiEditDlg[ID_HER_ATTRBSP].Selected==1)
    EditData->IncludeAttr|=FILE_ATTRIBUTE_SPARSE_FILE;
  else if (HiEditDlg[ID_HER_ATTRBSP].Selected==0)
    EditData->ExcludeAttr|=FILE_ATTRIBUTE_SPARSE_FILE;

  if (HiEditDlg[ID_HER_ATTRBT].Selected==1)
    EditData->IncludeAttr|=FILE_ATTRIBUTE_TEMPORARY;
  else if (HiEditDlg[ID_HER_ATTRBT].Selected==0)
    EditData->ExcludeAttr|=FILE_ATTRIBUTE_TEMPORARY;

  if (HiEditDlg[ID_HER_ATTRBNI].Selected==1)
    EditData->IncludeAttr|=FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
  else if (HiEditDlg[ID_HER_ATTRBNI].Selected==0)
    EditData->ExcludeAttr|=FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;

  EditData->Colors.MarkChar=HiEditDlg[ID_HER_MARKEDIT].strData.At(0);

  if (!New && RecPos<HiDataCount)
  {
    if(!AddMask(HiData[RecPos],strMask,EditData->IgnoreMask,EditData))
    {
      if ( bNew )
          delete EditData;
        return FALSE;
    }
  }
  if (New)
    DupHighlightData(EditData,strMask,EditData->IgnoreMask,RecPos);

  if ( bNew )
    delete EditData;
  return(TRUE);
}
/* IS $ */

int HighlightFiles::DupHighlightData(struct HighlightData *EditData,const wchar_t *Mask,BOOL IgnoreMask,int RecPos)
{
  HighlightData **NewHiData;
  HighlightData *HData= new HighlightData;
  memset (HData, 0, sizeof (HighlightData));
  string strTmpMask;

  strTmpMask = Mask;
  if(!AddMask(HData,strTmpMask,IgnoreMask,EditData))
    return FALSE;

  if ((NewHiData=(HighlightData **)xf_realloc(HiData,4*(HiDataCount+1)))==NULL)
  {
    DeleteMask(HData);
    delete HData;

    return(FALSE);
  }

  HiDataCount++;
  HiData=NewHiData;
  for (int I=HiDataCount-1;I>RecPos;I--)
    HiData[I]=HiData[I-1];
  memcpy(HiData[RecPos],HData,sizeof(struct HighlightData));
  return TRUE;
}


void SetHighlighting()
{
  if (CheckRegKeyW(RegColorsHighlight))
    return;

  int I;
  string strRegKey;
  // сразу пропишем %PATHEXT%, а HighlightFiles::GetHiColor() сам подстановку
  // сделает.
  string strCmdExt=L"*.exe,*.com,*.bat,%PATHEXT%";
  static wchar_t *Masks[]={
  /* 0 */ L"*.*",
  /* 1 */ L"",
  /* 2 */ L"*.rar,*.zip,*.[zj],*.[bg7]z,*.[bg]zip,*.tar,*.t[ag]z,*.ar[cj],*.r[0-9][0-9],*.a[0-9][0-9],*.bz2,*.cab,*.msi,*.jar,*.lha,*.lzh,*.ha,*.ac[bei],*.pa[ck],*.rk,*.cpio,*.rpm,*.zoo,*.hqx,*.sit,*.ice,*.uc2,*.ain,*.imp,*.777,*.ufa,*.boa,*.bs[2a],*.sea,*.hpk,*.ddi,*.x2,*.rkv,*.[lw]sz,*.h[ay]p,*.lim,*.sqz,*.chz",
  /* 3 */ L"*.bak,*.tmp",                                                                                                                                                                                //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ -> может к терапевту? ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
          /* $ 25.09.2001  IS
              Эта маска для каталогов: обрабатывать все каталоги, кроме тех, что
              являются родительскими (их имена - две точки).
          */
  /* 4 */ L"*.*|..", // маска для каталогов
  /* 5 */ L"..",     // такие каталоги окрашивать как простые файлы
          /* IS $ */
  };
  /* $ 06.07.2001 IS
     Нечего дразнить судьбу - используем только OriginalMasks
  */
  struct HighlightData  StdHighlightData[]=
  { /*
             OriginalMask                        NormalColor       SelectedCursorColor
                                IncludeAttributes       SelectedColor     MarkChar
                       FMasks           ExcludeAttributes     CursorColor             */
     /* 0 */{Masks[0], NULL, 0, 0x0002, 0x0000, {0x13, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00}},
     /* 1 */{Masks[0], NULL, 0, 0x0004, 0x0000, {0x13, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00}},
     /* 2 */{Masks[4], NULL, 0, 0x0010, 0x0000, {0x1F, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00}},
     /* 3 */{Masks[5], NULL, 0, 0x0010, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
     /* 4 */{strCmdExt,   NULL, 0, 0x0000, 0x0000, {0x1A, 0x00, 0x3A, 0x00, 0x00, 0x00, 0x00, 0x00}},
     /* 5 */{Masks[2], NULL, 0, 0x0000, 0x0000, {0x1D, 0x00, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00}},
     /* 6 */{Masks[3], NULL, 0, 0x0000, 0x0000, {0x16, 0x00, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00}},
            // это настройка для каталогов на тех панелях, которые должны раскрашиваться
            // без учета масок (например, список хостов в "far navigator")
     /* 7 */{Masks[0], NULL, 1, 0x0010, 0x0000, {0x1F, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00}},
  };

  // для NT добавляем CMD
  if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
    strCmdExt += L",*.cmd";

  for(I=0; I < sizeof(StdHighlightData)/sizeof(StdHighlightData[0]); ++I)
  {
    strRegKey.Format (L"%s\\Group%d", RegColorsHighlight, I);
    SetRegKeyW(strRegKey,HLS.Mask,StdHighlightData[I].strOriginalMasks);
    SetRegKeyW(strRegKey,HLS.IgnoreMask,StdHighlightData[I].IgnoreMask);
    if(StdHighlightData[I].IncludeAttr)
      SetRegKeyW(strRegKey,HLS.IncludeAttributes,StdHighlightData[I].IncludeAttr);
    if(StdHighlightData[I].ExcludeAttr)
      SetRegKeyW(strRegKey,HLS.ExcludeAttributes,StdHighlightData[I].ExcludeAttr);
    if(StdHighlightData[I].Colors.Color)
      SetRegKeyW(strRegKey,HLS.NormalColor,StdHighlightData[I].Colors.Color);
    if(StdHighlightData[I].Colors.SelColor)
      SetRegKeyW(strRegKey,HLS.SelectedColor,StdHighlightData[I].Colors.SelColor);
    if(StdHighlightData[I].Colors.CursorColor)
      SetRegKeyW(strRegKey,HLS.CursorColor,StdHighlightData[I].Colors.CursorColor);
    if(StdHighlightData[I].Colors.CursorSelColor)
      SetRegKeyW(strRegKey,HLS.SelectedCursorColor,StdHighlightData[I].Colors.CursorSelColor);
    if(StdHighlightData[I].Colors.MarkChar)
      SetRegKeyW(strRegKey,HLS.MarkChar,StdHighlightData[I].Colors.MarkChar);
  }
}
