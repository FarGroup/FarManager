/*
hilight.cpp

Files highlighting

*/

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

/* $ 25.09.2001 IS
     “ут храним строковые константы дл€ "раскраски файлов"
*/
struct HighlightStrings
{
  char *UseAttr,*IncludeAttributes,*ExcludeAttributes,
       *IgnoreMask,*Mask,
       *NormalColor,*SelectedColor,*CursorColor,*SelectedCursorColor,
       *MarkChar,
       *UseDate,*DateType,*DateAfter,*DateBefore,
       *UseSize,*SizeType,*SizeAbove,*SizeBelow,
       *HighlightEdit,*HighlightList;
};
const HighlightStrings HLS=
{
  "UseAttr","IncludeAttributes","ExcludeAttributes",
  "IgnoreMask","Mask",
  "NormalColor","SelectedColor","CursorColor","SelectedCursorColor",
  "MarkChar",
  "UseDate","DateType","DateAfter","DateBefore",
  "UseSize","SizeType","SizeAbove","SizeBelow",
  "HighlightEdit","HighlightList"
};
/* IS $ */

HighlightFiles::HighlightFiles()
{
  InitHighlightFiles();
}

void HighlightFiles::InitHighlightFiles()
{
  HiData.Free();
  char RegKey[80],Mask[FILEFILTER_MASK_SIZE];
  char *Ptr=MkRegKeyHighlightName(RegKey); // Ptr указывает на нужное место :-)

  while (1)
  {
    itoa(HiData.getCount(),Ptr,10);
    if (!GetRegKey(RegKey,HLS.Mask,Mask,"",sizeof(Mask)))
      break;

    FileFilterParams *HData = HiData.addItem();

    if(HData)
    {
      //ƒефолтные значени€ выбраны так чтоб как можно правильней загрузить
      //настройки старых версий фара.

      HData->SetMask((DWORD)(GetRegKey(RegKey,HLS.IgnoreMask,0)?0:1),
                     Mask);

      FILETIME DateAfter, DateBefore;
      GetRegKey(RegKey,HLS.DateAfter,(BYTE *)&DateAfter,NULL,sizeof(DateAfter));
      GetRegKey(RegKey,HLS.DateBefore,(BYTE *)&DateBefore,NULL,sizeof(DateBefore));
      HData->SetDate((DWORD)GetRegKey(RegKey,HLS.UseDate,0),
                     (DWORD)GetRegKey(RegKey,HLS.DateType,0),
                     DateAfter,
                     DateBefore);

      HData->SetSize((DWORD)GetRegKey(RegKey,HLS.UseSize,0),
                     (DWORD)GetRegKey(RegKey,HLS.SizeType,0),
                     GetRegKey64(RegKey,HLS.SizeAbove,_i64(-1)),
                     GetRegKey64(RegKey,HLS.SizeBelow,_i64(-1)));

      HData->SetAttr((DWORD)GetRegKey(RegKey,HLS.UseAttr,1),
                     (DWORD)GetRegKey(RegKey,HLS.IncludeAttributes,0),
                     (DWORD)GetRegKey(RegKey,HLS.ExcludeAttributes,0));

      HighlightDataColor Colors;
      ReWriteWorkColor(&Colors);
      Colors.Color=(WORD)GetRegKey(RegKey,HLS.NormalColor,Colors.Color);
      Colors.SelColor=(WORD)GetRegKey(RegKey,HLS.SelectedColor,Colors.SelColor);
      Colors.CursorColor=(WORD)GetRegKey(RegKey,HLS.CursorColor,Colors.CursorColor);
      Colors.CursorSelColor=(WORD)GetRegKey(RegKey,HLS.SelectedCursorColor,Colors.CursorSelColor);
      Colors.MarkChar=(WORD)GetRegKey(RegKey,HLS.MarkChar,Colors.MarkChar);

      HData->SetColors(&Colors);
    }
    else
      break;
  }

  StartHiDataCount=HiData.getCount();
}


HighlightFiles::~HighlightFiles()
{
  ClearData();
}

void HighlightFiles::ClearData()
{
  HiData.Free();
}

void MakeTransparent(HighlightDataColor *Colors)
{
  Colors->Color|=0xFF00;
  Colors->SelColor|=0xFF00;
  Colors->CursorColor|=0xFF00;
  Colors->CursorSelColor|=0xFF00;
  Colors->MarkChar|=0xFF00;
}

void ApplyColors(HighlightDataColor *DestColors, HighlightDataColor *SrcColors)
{
  if (DestColors->Color&0xF000)
    DestColors->Color=(DestColors->Color&0x0F0F)|(SrcColors->Color&0xF0F0);
  if (DestColors->Color&0x0F00)
    DestColors->Color=(DestColors->Color&0xF0F0)|(SrcColors->Color&0x0F0F);

  if (DestColors->SelColor&0xF000)
    DestColors->SelColor=(DestColors->SelColor&0x0F0F)|(SrcColors->SelColor&0xF0F0);
  if (DestColors->SelColor&0x0F00)
    DestColors->SelColor=(DestColors->SelColor&0xF0F0)|(SrcColors->SelColor&0x0F0F);

  if (DestColors->CursorColor&0xF000)
    DestColors->CursorColor=(DestColors->CursorColor&0x0F0F)|(SrcColors->CursorColor&0xF0F0);
  if (DestColors->CursorColor&0x0F00)
    DestColors->CursorColor=(DestColors->CursorColor&0xF0F0)|(SrcColors->CursorColor&0x0F0F);

  if (DestColors->CursorSelColor&0xF000)
    DestColors->CursorSelColor=(DestColors->CursorSelColor&0x0F0F)|(SrcColors->CursorSelColor&0xF0F0);
  if (DestColors->CursorSelColor&0x0F00)
    DestColors->CursorSelColor=(DestColors->CursorSelColor&0xF0F0)|(SrcColors->CursorSelColor&0x0F0F);

  if (DestColors->MarkChar&0xFF00)
    DestColors->MarkChar=SrcColors->MarkChar;
}

bool HasTransparent(HighlightDataColor *Colors)
{
  if (Colors->Color&0xFF00)
    return true;

  if (Colors->SelColor&0xFF00)
    return true;

  if (Colors->CursorColor&0xFF00)
    return true;

  if (Colors->CursorSelColor&0xFF00)
    return true;

  if (Colors->MarkChar&0xFF00)
    return true;

  return false;
}

void RewriteTransparent(HighlightDataColor *Colors)
{
  Colors->Color&=0x00FF;
  Colors->SelColor&=0x00FF;
  Colors->CursorColor&=0x00FF;
  Colors->CursorSelColor&=0x00FF;
  Colors->MarkChar&=0x00FF;
}

void HighlightFiles::GetHiColor(WIN32_FIND_DATA *fd,struct HighlightDataColor *Colors,bool UseAttrHighlighting)
{
  FileFilterParams *CurHiData;

  ReWriteWorkColor(Colors);
  MakeTransparent(Colors);

  for (int i=0; i < HiData.getCount(); i++)
  {
    CurHiData = HiData.getItem(i);
    if (UseAttrHighlighting && CurHiData->GetMask(NULL))
      continue;
    if (CurHiData->FileInFilter(fd))
    {
      HighlightDataColor TempColors;
      CurHiData->GetColors(&TempColors);
      ApplyColors(Colors,&TempColors);
      if (!HasTransparent(Colors))
        break;
    }
  }

  RewriteTransparent(Colors);
}

void HighlightFiles::GetHiColor(struct FileListItem *FileItem,int FileCount,bool UseAttrHighlighting)
{
  if(!FileItem || !FileCount)
    return;

  FileFilterParams *CurHiData;

  for(int FCnt=0; FCnt < FileCount; ++FCnt,++FileItem)
  {
    ReWriteWorkColor(&FileItem->Colors);
    MakeTransparent(&FileItem->Colors);
    for (int i=0; i < HiData.getCount(); i++)
    {
      CurHiData = HiData.getItem(i);
      if (UseAttrHighlighting && CurHiData->GetMask(NULL))
        continue;
      if (CurHiData->FileInFilter(FileItem))
      {
        HighlightDataColor TempColors;
        CurHiData->GetColors(&TempColors);
        ApplyColors(&FileItem->Colors,&TempColors);
        if (!HasTransparent(&FileItem->Colors))
          break;
      }
    }
    RewriteTransparent(&FileItem->Colors);
  }
}

void HighlightFiles::ReWriteWorkColor(struct HighlightDataColor *Colors)
{
  memset(Colors,0,sizeof(struct HighlightDataColor));
}

void HighlightFiles::FillMenu(VMenu *HiMenu,int MenuPos)
{
  struct MenuItem HiMenuItem;
  unsigned char VerticalLine=0x0B3;

  HiMenu->DeleteItems();
  memset(&HiMenuItem,0,sizeof(HiMenuItem));

  /* $ 22.01.2003 IS
     —имвол дл€ пометки файлов показываем в кавычках, чтобы можно было
     отличить пробел от пустоты
  */
  char MarkChar[]="\" \"";
  int Short=1;
  // сначала проверим - а есть ли символы пометки файлов в меню вообще?
  for (int i=0; i<HiData.getCount(); i++)
  {
    if(HiData.getItem(i)->GetMarkChar())
    {
      Short=0;
      break;
    }
  }

  // если символов пометки в меню нет, то отводим под это поле только 1 знакоместо
  const char *emptyMarkChar=Short?" ":"   ";
  for (int i=0; i<HiData.getCount(); i++)
  {
    FileFilterParams *CurHiData=HiData.getItem(i);

    MarkChar[1]=CurHiData->GetMarkChar();

    const char *Mask;
    DWORD MaskUsed = CurHiData->GetMask(&Mask);

    DWORD IncludeAttr, ExcludeAttr;
    if (!CurHiData->GetAttr(&IncludeAttr,&ExcludeAttr))
      IncludeAttr = ExcludeAttr = 0;

    sprintf(HiMenuItem.Name,"%s %c %c%c%c%c%c%c%c%c%c%c%c %c %c%c%c%c%c%c%c%c%c%c%c %c %.54s",
      // добавим показ символа в кавычках
      (MarkChar[1] ? MarkChar : emptyMarkChar),
      VerticalLine,

      (IncludeAttr & FILE_ATTRIBUTE_READONLY) ? 'R':'.',
      (IncludeAttr & FILE_ATTRIBUTE_HIDDEN) ? 'H':'.',
      (IncludeAttr & FILE_ATTRIBUTE_SYSTEM) ? 'S':'.',
      (IncludeAttr & FILE_ATTRIBUTE_ARCHIVE) ? 'A':'.',
      (IncludeAttr & FILE_ATTRIBUTE_COMPRESSED) ? 'C': '.',
      (IncludeAttr & FILE_ATTRIBUTE_ENCRYPTED) ? 'E':'.',
      (IncludeAttr & FILE_ATTRIBUTE_DIRECTORY) ? 'F':'.',
      (IncludeAttr & FILE_ATTRIBUTE_REPARSE_POINT) ? 'L':'.',
      (IncludeAttr & FILE_ATTRIBUTE_SPARSE_FILE) ? '$':'.',
      (IncludeAttr & FILE_ATTRIBUTE_TEMPORARY) ? 'T':'.',
      (IncludeAttr & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED) ? 'I':'.',

      VerticalLine,

      (ExcludeAttr & FILE_ATTRIBUTE_READONLY) ? 'R':'.',
      (ExcludeAttr & FILE_ATTRIBUTE_HIDDEN) ? 'H':'.',
      (ExcludeAttr & FILE_ATTRIBUTE_SYSTEM) ? 'S':'.',
      (ExcludeAttr & FILE_ATTRIBUTE_ARCHIVE) ? 'A':'.',
      (ExcludeAttr & FILE_ATTRIBUTE_COMPRESSED) ? 'C':'.',
      (ExcludeAttr & FILE_ATTRIBUTE_ENCRYPTED)? 'E':'.',
      (ExcludeAttr & FILE_ATTRIBUTE_DIRECTORY) ? 'F':'.',
      (ExcludeAttr & FILE_ATTRIBUTE_REPARSE_POINT) ? 'L':'.',
      (ExcludeAttr & FILE_ATTRIBUTE_SPARSE_FILE) ? '$':'.',
      (ExcludeAttr & FILE_ATTRIBUTE_TEMPORARY) ? 'T':'.',
      (ExcludeAttr & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED) ? 'I':'.',

      VerticalLine,

      MaskUsed ? Mask : " ");

    HiMenuItem.SetSelect(i==MenuPos);
    HiMenu->AddItem(&HiMenuItem);
  }

  *HiMenuItem.Name=0;
  HiMenuItem.SetSelect(HiData.getCount()==MenuPos);
  HiMenu->AddItem(&HiMenuItem);
}

void HighlightFiles::HiEdit(int MenuPos)
{
  VMenu HiMenu(MSG(MHighlightTitle),NULL,0,ScrY-4);
  HiMenu.SetHelp(HLS.HighlightList);
  HiMenu.SetFlags(VMENU_WRAPMODE|VMENU_SHOWAMPERSAND);
  HiMenu.SetPosition(-1,-1,0,0);
  HiMenu.SetBottomTitle(MSG(MHighlightBottom));

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
          ≈сли нажали ctrl+r, то восстановить значени€ по умолчанию.
        */
        case KEY_CTRLR:
          if (Message(MSG_WARNING,2,MSG(MHighlightTitle),
                        MSG(MHighlightWarning),MSG(MHighlightAskRestore),
                        MSG(MYes),MSG(MCancel))!=0)
             break;
          DeleteKeyTree(RegColorsHighlight);
          SetHighlighting();
          HiMenu.Hide();
          ClearData();
          InitHighlightFiles();
          NeedUpdate=TRUE;
          break;
        case KEY_DEL:
          if (SelectPos<HiData.getCount())
          {
            const char *Mask;
            HiData.getItem(SelectPos)->GetMask(&Mask);
            if (Message(MSG_WARNING,2,MSG(MHighlightTitle),
                        MSG(MHighlightAskDel),Mask,
                        MSG(MDelete),MSG(MCancel))!=0)
              break;

            HiData.deleteItem(SelectPos);
            NeedUpdate=TRUE;
          }
          break;
        case KEY_ENTER:
        case KEY_F4:
        {
          if (SelectPos<HiData.getCount())
            if (FileFilterConfig(HiData.getItem(SelectPos),true))
              NeedUpdate=TRUE;
          break;
        }
        case KEY_INS: case KEY_NUMPAD0:
        {
          FileFilterParams *NewHData = HiData.insertItem(SelectPos);

          if (!NewHData)
            break;

          if (FileFilterConfig(NewHData,true))
            NeedUpdate=TRUE;
          else
            HiData.deleteItem(SelectPos);

          break;
        }
        case KEY_F5:
          if (SelectPos < HiData.getCount())
          {
              FileFilterParams *HData = HiData.insertItem(SelectPos);

              if (HData)
              {
                *HData = *HiData.getItem(SelectPos+1);
                HData->SetTitle("");
                if (FileFilterConfig(HData,true))
                  NeedUpdate=TRUE;
                else
                  HiData.deleteItem(SelectPos);
             }
          }
          break;
        case KEY_CTRLUP: case KEY_CTRLNUMPAD8:
          if (SelectPos > 0 && SelectPos < HiData.getCount())
          {
            HiData.swapItems(SelectPos,SelectPos-1);
            HiMenu.SetSelection(--SelectPos);
            NeedUpdate=TRUE;
            break;
          }
          HiMenu.ProcessInput();
          break;

        case KEY_CTRLDOWN: case KEY_CTRLNUMPAD2:
          if (SelectPos < HiData.getCount()-1)
          {
            HiData.swapItems(SelectPos,SelectPos+1);
            HiMenu.SetSelection(++SelectPos);
            NeedUpdate=TRUE;
          }
          HiMenu.ProcessInput();
          break;

        default:
          HiMenu.ProcessInput();
          break;
      }
      // повтор€ющийс€ кусок!
      if(NeedUpdate)
      {
         ScrBuf.Lock(); // отмен€ем вс€кую прорисовку
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
  char RegKey[80];
  char *Ptr=MkRegKeyHighlightName(RegKey);
  for (int i=0; i<HiData.getCount(); i++)
  {
    FileFilterParams *CurHiData=HiData.getItem(i);
    itoa(i,Ptr,10);

    const char *Mask;
    SetRegKey(RegKey,HLS.IgnoreMask,(CurHiData->GetMask(&Mask) ? 0 : 1));
    SetRegKey(RegKey,HLS.Mask,Mask);

    DWORD DateType;
    FILETIME DateAfter, DateBefore;
    SetRegKey(RegKey,HLS.UseDate,CurHiData->GetDate(&DateType, &DateAfter, &DateBefore));
    SetRegKey(RegKey,HLS.DateType,DateType);
    SetRegKey(RegKey,HLS.DateAfter,(BYTE *)&DateAfter,sizeof(DateAfter));
    SetRegKey(RegKey,HLS.DateBefore,(BYTE *)&DateBefore,sizeof(DateBefore));

    DWORD SizeType;
    __int64 SizeAbove, SizeBelow;
    SetRegKey(RegKey,HLS.UseSize,CurHiData->GetSize(&SizeType, &SizeAbove, &SizeBelow));
    SetRegKey(RegKey,HLS.SizeType,SizeType);
    SetRegKey64(RegKey,HLS.SizeAbove,SizeAbove);
    SetRegKey64(RegKey,HLS.SizeBelow,SizeBelow);

    DWORD AttrSet, AttrClear;
    SetRegKey(RegKey,HLS.UseAttr,CurHiData->GetAttr(&AttrSet, &AttrClear));
    SetRegKey(RegKey,HLS.IncludeAttributes,AttrSet);
    SetRegKey(RegKey,HLS.ExcludeAttributes,AttrClear);

    HighlightDataColor Colors;
    CurHiData->GetColors(&Colors);
    SetRegKey(RegKey,HLS.NormalColor,(DWORD)Colors.Color);
    SetRegKey(RegKey,HLS.SelectedColor,(DWORD)Colors.SelColor);
    SetRegKey(RegKey,HLS.CursorColor,(DWORD)Colors.CursorColor);
    SetRegKey(RegKey,HLS.SelectedCursorColor,(DWORD)Colors.CursorSelColor);
    SetRegKey(RegKey,HLS.MarkChar,(DWORD)Colors.MarkChar);
  }
  for (int i=HiData.getCount(); i<StartHiDataCount; i++)
  {
    itoa(i,Ptr,10);
    DeleteRegKey(RegKey);
  }
}

/*
 ‘ормирует им€ ключа в реестре;  возвращает указатель на конец строки
 ѕрименение:
  char RegKey[80];
  char *Ptr=MkRegKeyHighlightName(RegKey);
  for(I=0;...)
  {
    itoa(I,Ptr,10);
  }
*/
char *MkRegKeyHighlightName(char *RegKey)
{
  return RegKey+strlen(strcat(strcpy(RegKey,RegColorsHighlight),"\\Group"));
}


void SetHighlighting()
{
#if 0
  if (CheckRegKey(RegColorsHighlight))
    return;

  int I;
  char RegKey[80], *Ptr;
  // сразу пропишем %PATHEXT%, а HighlightFiles::GetHiColor() сам подстановку
  // сделает.
  char CmdExt[512]="*.exe,*.com,*.bat,%PATHEXT%";
  static char *Masks[]={
  /* 0 */ "*.*",
  /* 1 */ "",
  /* 2 */ "*.rar,*.zip,*.[zj],*.[bg7]z,*.[bg]zip,*.tar,*.t[ag]z,*.ar[cj],*.r[0-9][0-9],*.a[0-9][0-9],*.bz2,*.cab,*.msi,*.jar,*.lha,*.lzh,*.ha,*.ac[bei],*.pa[ck],*.rk,*.cpio,*.rpm,*.zoo,*.hqx,*.sit,*.ice,*.uc2,*.ain,*.imp,*.777,*.ufa,*.boa,*.bs[2a],*.sea,*.hpk,*.ddi,*.x2,*.rkv,*.[lw]sz,*.h[ay]p,*.lim,*.sqz,*.chz",
  /* 3 */ "*.bak,*.tmp",                                                                                                                                                                                //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ -> может к терапевту? ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
          /* $ 25.09.2001  IS
              Ёта маска дл€ каталогов: обрабатывать все каталоги, кроме тех, что
              €вл€ютс€ родительскими (их имена - две точки).
          */
  /* 4 */ "*.*|..", // маска дл€ каталогов
  /* 5 */ "..",     // такие каталоги окрашивать как простые файлы
          /* IS $ */
  };
  /* $ 06.07.2001 IS
     Ќечего дразнить судьбу - используем только OriginalMasks
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
     /* 4 */{CmdExt,   NULL, 0, 0x0000, 0x0000, {0x1A, 0x00, 0x3A, 0x00, 0x00, 0x00, 0x00, 0x00}},
     /* 5 */{Masks[2], NULL, 0, 0x0000, 0x0000, {0x1D, 0x00, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00}},
     /* 6 */{Masks[3], NULL, 0, 0x0000, 0x0000, {0x16, 0x00, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00}},
            // это настройка дл€ каталогов на тех панел€х, которые должны раскрашиватьс€
            // без учета масок (например, список хостов в "far navigator")
     /* 7 */{Masks[0], NULL, 1, 0x0010, 0x0000, {0x1F, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00}},
  };

  // дл€ NT добавл€ем CMD
  if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
    strcat(CmdExt,",*.cmd");

  Ptr=MkRegKeyHighlightName(RegKey);
  for(I=0; I < sizeof(StdHighlightData)/sizeof(StdHighlightData[0]); ++I)
  {
    itoa(I,Ptr,10);
    SetRegKey(RegKey,HLS.Mask,StdHighlightData[I].OriginalMasks);
    SetRegKey(RegKey,HLS.IgnoreMask,StdHighlightData[I].IgnoreMask);
    if(StdHighlightData[I].IncludeAttr)
      SetRegKey(RegKey,HLS.IncludeAttributes,StdHighlightData[I].IncludeAttr);
    if(StdHighlightData[I].ExcludeAttr)
      SetRegKey(RegKey,HLS.ExcludeAttributes,StdHighlightData[I].ExcludeAttr);
    if(StdHighlightData[I].Colors.Color)
      SetRegKey(RegKey,HLS.NormalColor,StdHighlightData[I].Colors.Color);
    if(StdHighlightData[I].Colors.SelColor)
      SetRegKey(RegKey,HLS.SelectedColor,StdHighlightData[I].Colors.SelColor);
    if(StdHighlightData[I].Colors.CursorColor)
      SetRegKey(RegKey,HLS.CursorColor,StdHighlightData[I].Colors.CursorColor);
    if(StdHighlightData[I].Colors.CursorSelColor)
      SetRegKey(RegKey,HLS.SelectedCursorColor,StdHighlightData[I].Colors.CursorSelColor);
    if(StdHighlightData[I].Colors.MarkChar)
      SetRegKey(RegKey,HLS.MarkChar,StdHighlightData[I].Colors.MarkChar);
  }
#endif
}
