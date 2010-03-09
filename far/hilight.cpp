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

struct HighlightStrings
{
  char *UseAttr,*IncludeAttributes,*ExcludeAttributes,*AttrSet,*AttrClear,
       *IgnoreMask,*UseMask,*Mask,
       *NormalColor,*SelectedColor,*CursorColor,*SelectedCursorColor,
       *MarkCharNormalColor,*MarkCharSelectedColor,*MarkCharCursorColor,*MarkCharSelectedCursorColor,
       *MarkChar,
       *ContinueProcessing,
       *UseDate,*DateType,*DateAfter,*DateBefore, *DateRelative,
       *UseSize,*SizeAbove,*SizeBelow,
       *HighlightEdit,*HighlightList;
};
static const HighlightStrings HLS=
{
  "UseAttr","IncludeAttributes","ExcludeAttributes","AttrSet","AttrClear",
  "IgnoreMask","UseMask","Mask",
  "NormalColor","SelectedColor","CursorColor","SelectedCursorColor",
  "MarkCharNormalColor","MarkCharSelectedColor","MarkCharCursorColor","MarkCharSelectedCursorColor",
  "MarkChar",
  "ContinueProcessing",
  "UseDate","DateType","DateAfter","DateBefore", "DateRelative",
  "UseSize","SizeAboveS","SizeBelowS",
  "HighlightEdit","HighlightList"
};

static const char fmtFirstGroup[]="Group%d";
static const char fmtUpperGroup[]="UpperGroup%d";
static const char fmtLowerGroup[]="LowerGroup%d";
static const char fmtLastGroup[]="LastGroup%d";
static const char SortGroupsKeyName[]="SortGroups";

HighlightFiles::HighlightFiles()
{
  InitHighlightFiles();
  UpdateCurrentTime();
}

void LoadFilterFromReg(FileFilterParams *HData, const char *RegKey, const char *Mask, int SortGroup, bool bSortGroup)
{
  //Дефолтные значения выбраны так чтоб как можно правильней загрузить
  //настройки старых версий фара.

  if (bSortGroup)
    HData->SetMask(GetRegKey(RegKey,HLS.UseMask,1)!=0, Mask);
  else
    HData->SetMask(GetRegKey(RegKey,HLS.IgnoreMask,0)==0, Mask);


  FILETIME DateAfter, DateBefore;
  GetRegKey(RegKey,HLS.DateAfter,(BYTE *)&DateAfter,NULL,sizeof(DateAfter));
  GetRegKey(RegKey,HLS.DateBefore,(BYTE *)&DateBefore,NULL,sizeof(DateBefore));
  HData->SetDate(GetRegKey(RegKey,HLS.UseDate,0)!=0,
                 (DWORD)GetRegKey(RegKey,HLS.DateType,0),
                 DateAfter,
                 DateBefore,
                 GetRegKey(RegKey,HLS.DateRelative,0)!=0);

  char SizeAbove[FILEFILTER_SIZE_SIZE];
  char SizeBelow[FILEFILTER_SIZE_SIZE];
  GetRegKey(RegKey,HLS.SizeAbove,SizeAbove,"",sizeof(SizeAbove));
  GetRegKey(RegKey,HLS.SizeBelow,SizeBelow,"",sizeof(SizeBelow));
  HData->SetSize(GetRegKey(RegKey,HLS.UseSize,0)!=0,
                 SizeAbove,
                 SizeBelow);

  if (bSortGroup)
  {
    HData->SetAttr(GetRegKey(RegKey,HLS.UseAttr,1)!=0,
                   (DWORD)GetRegKey(RegKey,HLS.AttrSet,0),
                   (DWORD)GetRegKey(RegKey,HLS.AttrClear,FILE_ATTRIBUTE_DIRECTORY));
  }
  else
  {
    HData->SetAttr(GetRegKey(RegKey,HLS.UseAttr,1)!=0,
                   (DWORD)GetRegKey(RegKey,HLS.IncludeAttributes,0),
                   (DWORD)GetRegKey(RegKey,HLS.ExcludeAttributes,0));
  }

  HData->SetSortGroup(SortGroup);

  HighlightDataColor Colors;
  Colors.Color[HIGHLIGHTCOLORTYPE_FILE][HIGHLIGHTCOLOR_NORMAL]=(WORD)GetRegKey(RegKey,HLS.NormalColor,0);
  Colors.Color[HIGHLIGHTCOLORTYPE_FILE][HIGHLIGHTCOLOR_SELECTED]=(WORD)GetRegKey(RegKey,HLS.SelectedColor,0);
  Colors.Color[HIGHLIGHTCOLORTYPE_FILE][HIGHLIGHTCOLOR_UNDERCURSOR]=(WORD)GetRegKey(RegKey,HLS.CursorColor,0);
  Colors.Color[HIGHLIGHTCOLORTYPE_FILE][HIGHLIGHTCOLOR_SELECTEDUNDERCURSOR]=(WORD)GetRegKey(RegKey,HLS.SelectedCursorColor,0);
  Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][HIGHLIGHTCOLOR_NORMAL]=(WORD)GetRegKey(RegKey,HLS.MarkCharNormalColor,0);
  Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][HIGHLIGHTCOLOR_SELECTED]=(WORD)GetRegKey(RegKey,HLS.MarkCharSelectedColor,0);
  Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][HIGHLIGHTCOLOR_UNDERCURSOR]=(WORD)GetRegKey(RegKey,HLS.MarkCharCursorColor,0);
  Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][HIGHLIGHTCOLOR_SELECTEDUNDERCURSOR]=(WORD)GetRegKey(RegKey,HLS.MarkCharSelectedCursorColor,0);
  Colors.MarkChar=(WORD)GetRegKey(RegKey,HLS.MarkChar,0);

  HData->SetColors(&Colors);

  HData->SetContinueProcessing(GetRegKey(RegKey,HLS.ContinueProcessing,0)!=0);
}

void HighlightFiles::InitHighlightFiles()
{
  char RegKey[80],GroupName[80],Mask[FILEFILTER_MASK_SIZE];
  const int GroupDelta[4]={DEFAULT_SORT_GROUP,0,DEFAULT_SORT_GROUP+1,DEFAULT_SORT_GROUP};
  const char *KeyNames[4]={RegColorsHighlight,SortGroupsKeyName,SortGroupsKeyName,RegColorsHighlight};
  const char *GroupNames[4]={fmtFirstGroup,fmtUpperGroup,fmtLowerGroup,fmtLastGroup};
  int  *Count[4] = {&FirstCount,&UpperCount,&LowerCount,&LastCount};

  HiData.Free();

  FirstCount=UpperCount=LowerCount=LastCount=0;
  for (int j=0; j<4; j++)
  {
    for (int i=0;;i++)
    {
      sprintf(GroupName,GroupNames[j],i);
      sprintf(RegKey,"%s\\%s",KeyNames[j],GroupName);

      if (GroupDelta[j]!=DEFAULT_SORT_GROUP)
      {
        if (!GetRegKey(KeyNames[j],GroupName,Mask,"",sizeof(Mask)))
          break;
      }
      else
      {
        if (!GetRegKey(RegKey,HLS.Mask,Mask,"",sizeof(Mask)))
          break;
      }

      FileFilterParams *HData = HiData.addItem();

      if(HData)
      {
        LoadFilterFromReg(HData,RegKey,Mask,GroupDelta[j]+(GroupDelta[j]==DEFAULT_SORT_GROUP?0:i),GroupDelta[j]!=DEFAULT_SORT_GROUP);

        (*(Count[j]))++;
      }
      else
        break;
    }
  }
}


HighlightFiles::~HighlightFiles()
{
  ClearData();
}

void HighlightFiles::ClearData()
{
  HiData.Free();
  FirstCount=UpperCount=LowerCount=LastCount=0;
}

static const DWORD FarColor[] = {COL_PANELTEXT,COL_PANELSELECTEDTEXT,COL_PANELCURSOR,COL_PANELSELECTEDCURSOR};

void ApplyDefaultStartingColors(HighlightDataColor *Colors)
{
  for (int j=0; j<2; j++)
    for (int i=0; i<4; i++)
      Colors->Color[j][i]=0xFF00;
  Colors->MarkChar=0xFF00;
}

void ApplyBlackOnBlackColors(HighlightDataColor *Colors)
{
  for (int i=0; i<4; i++)
  {
    //Применим black on black.
    //Для файлов возьмем цвета панели не изменяя прозрачность.
    //Для пометки возьмем цвета файла включая прозрачность.

    if (!(Colors->Color[HIGHLIGHTCOLORTYPE_FILE][i]&0x00FF))
      Colors->Color[HIGHLIGHTCOLORTYPE_FILE][i]=(Colors->Color[HIGHLIGHTCOLORTYPE_FILE][i]&0xFF00)|(0x00FF&Palette[FarColor[i]-COL_FIRSTPALETTECOLOR]);

    if (!(Colors->Color[HIGHLIGHTCOLORTYPE_MARKCHAR][i]&0x00FF))
      Colors->Color[HIGHLIGHTCOLORTYPE_MARKCHAR][i]=Colors->Color[HIGHLIGHTCOLORTYPE_FILE][i];
  }
}

void ApplyColors(HighlightDataColor *DestColors, HighlightDataColor *SrcColors)
{
  //Обработаем black on black чтоб наследовать правильные цвета
  //и чтоб после наследования были правильные цвета.
  ApplyBlackOnBlackColors(DestColors);
  ApplyBlackOnBlackColors(SrcColors);

  for (int j=0; j<2; j++)
  {
    for (int i=0; i<4; i++)
    {
      //Если текущие цвета в Src (fore и/или back) не прозрачные
      //то унаследуем их в Dest.
      if (!(SrcColors->Color[j][i]&0xF000))
        DestColors->Color[j][i]=(DestColors->Color[j][i]&0x0F0F)|(SrcColors->Color[j][i]&0xF0F0);
      if (!(SrcColors->Color[j][i]&0x0F00))
        DestColors->Color[j][i]=(DestColors->Color[j][i]&0xF0F0)|(SrcColors->Color[j][i]&0x0F0F);
    }
  }

  //Унаследуем пометку из Src если она не прозрачная
  if (!(SrcColors->MarkChar&0xFF00))
    DestColors->MarkChar=SrcColors->MarkChar;
}

/*
bool HasTransparent(HighlightDataColor *Colors)
{
  for (int j=0; j<2; j++)
    for (int i=0; i<4; i++)
      if (Colors->Color[j][i]&0xFF00)
        return true;

  if (Colors->MarkChar&0xFF00)
    return true;

  return false;
}
*/

void ApplyFinalColors(HighlightDataColor *Colors)
{
  //Обработаем black on black чтоб после наследования были правильные цвета.
  ApplyBlackOnBlackColors(Colors);

  for (int j=0; j<2; j++)
    for (int i=0; i<4; i++)
    {
      //Если какой то из текущих цветов (fore или back) прозрачный
      //то унаследуем соответствующий цвет с панелей.
      BYTE temp=(BYTE)((Colors->Color[j][i]&0xFF00)>>8);
      Colors->Color[j][i]=((~temp)&(BYTE)Colors->Color[j][i])|(temp&(BYTE)Palette[FarColor[i]-COL_FIRSTPALETTECOLOR]);
    }

  //Если символ пометки прозрачный то его как бы и нет вообще.
  if (Colors->MarkChar&0xFF00)
    Colors->MarkChar=0;

  //Параноя но случится может:
  //Обработаем black on black снова чтоб обработались унаследованые цвета.
  ApplyBlackOnBlackColors(Colors);
}

void HighlightFiles::UpdateCurrentTime()
{
  SYSTEMTIME cst;
  FILETIME cft;
  GetSystemTime(&cst);
  SystemTimeToFileTime(&cst, &cft);

  ULARGE_INTEGER current;
  current.u.LowPart  = cft.dwLowDateTime;
  current.u.HighPart = cft.dwHighDateTime;

  CurrentTime = current.QuadPart;
}

void HighlightFiles::GetHiColor(WIN32_FIND_DATA *fd,struct HighlightDataColor *Colors,bool UseAttrHighlighting)
{
  FileFilterParams *CurHiData;

  ApplyDefaultStartingColors(Colors);

  for (unsigned int i=0; i < HiData.getCount(); i++)
  {
    CurHiData = HiData.getItem(i);

    if (UseAttrHighlighting && CurHiData->GetMask(NULL))
      continue;

    if (CurHiData->FileInFilter(fd, CurrentTime))
    {
      HighlightDataColor TempColors;
      CurHiData->GetColors(&TempColors);
      ApplyColors(Colors,&TempColors);
      if (!CurHiData->GetContinueProcessing())// || !HasTransparent(Colors))
        break;
    }
  }

  ApplyFinalColors(Colors);
}

void HighlightFiles::GetHiColor(struct FileListItem *FileItem,int FileCount,bool UseAttrHighlighting)
{
  if(!FileItem || !FileCount)
    return;

  FileFilterParams *CurHiData;

  for(int FCnt=0; FCnt < FileCount; ++FCnt,++FileItem)
  {
    ApplyDefaultStartingColors(&FileItem->Colors);

    for (unsigned int i=0; i < HiData.getCount(); i++)
    {
      CurHiData = HiData.getItem(i);

      if (UseAttrHighlighting && CurHiData->GetMask(NULL))
        continue;

      if (CurHiData->FileInFilter(FileItem, CurrentTime))
      {
        HighlightDataColor TempColors;
        CurHiData->GetColors(&TempColors);
        ApplyColors(&FileItem->Colors,&TempColors);
        if (!CurHiData->GetContinueProcessing())// || !HasTransparent(&FileItem->Colors))
          break;
      }
    }

    ApplyFinalColors(&FileItem->Colors);
  }
}

int HighlightFiles::GetGroup(WIN32_FIND_DATA *fd)
{
  for (int i=FirstCount; i<FirstCount+UpperCount; i++)
  {
    FileFilterParams *CurGroupData=HiData.getItem(i);
    if(CurGroupData->FileInFilter(fd, CurrentTime))
       return(CurGroupData->GetSortGroup());
  }

  for (int i=FirstCount+UpperCount; i<FirstCount+UpperCount+LowerCount; i++)
  {
    FileFilterParams *CurGroupData=HiData.getItem(i);
    if(CurGroupData->FileInFilter(fd, CurrentTime))
       return(CurGroupData->GetSortGroup());
  }
  return DEFAULT_SORT_GROUP;
}

int HighlightFiles::GetGroup(FileListItem *fli)
{
  for (int i=FirstCount; i<FirstCount+UpperCount; i++)
  {
    FileFilterParams *CurGroupData=HiData.getItem(i);
    if(CurGroupData->FileInFilter(fli, CurrentTime))
       return(CurGroupData->GetSortGroup());
  }

  for (int i=FirstCount+UpperCount; i<FirstCount+UpperCount+LowerCount; i++)
  {
    FileFilterParams *CurGroupData=HiData.getItem(i);
    if(CurGroupData->FileInFilter(fli, CurrentTime))
       return(CurGroupData->GetSortGroup());
  }

  return DEFAULT_SORT_GROUP;
}

void HighlightFiles::FillMenu(VMenu *HiMenu,int MenuPos)
{
  struct MenuItem HiMenuItem;
  const int Count[4][2] = {
                            {0,                               FirstCount},
                            {FirstCount,                      FirstCount+UpperCount},
                            {FirstCount+UpperCount,           FirstCount+UpperCount+LowerCount},
                            {FirstCount+UpperCount+LowerCount,FirstCount+UpperCount+LowerCount+LastCount}
                          };

  HiMenu->DeleteItems();
  memset(&HiMenuItem,0,sizeof(HiMenuItem));

  for (int j=0; j<4; j++)
  {
    for (int i=Count[j][0]; i<Count[j][1]; i++)
    {
      MenuString(HiMenuItem.Name,HiData.getItem(i),true);
      HiMenu->AddItem(&HiMenuItem);
    }

    *HiMenuItem.Name=0;
    HiMenu->AddItem(&HiMenuItem);
    if (j<3)
    {
      if (j==0)
        xstrncpy(HiMenuItem.Name,MSG(MHighlightUpperSortGroup),sizeof(HiMenuItem.Name)-1);
      else if (j==1)
        xstrncpy(HiMenuItem.Name,MSG(MHighlightLowerSortGroup),sizeof(HiMenuItem.Name)-1);
      else
        xstrncpy(HiMenuItem.Name,MSG(MHighlightLastGroup),sizeof(HiMenuItem.Name)-1);

      HiMenuItem.Flags|=LIF_SEPARATOR;
      HiMenu->AddItem(&HiMenuItem);
      HiMenuItem.Flags=0;
    }
  }

  HiMenu->SetSelectPos(MenuPos,1);
}

void HighlightFiles::ProcessGroups()
{
  for (int i=0; i<FirstCount; i++)
    HiData.getItem(i)->SetSortGroup(DEFAULT_SORT_GROUP);

  for (int i=FirstCount; i<FirstCount+UpperCount; i++)
    HiData.getItem(i)->SetSortGroup(i-FirstCount);

  for (int i=FirstCount+UpperCount; i<FirstCount+UpperCount+LowerCount; i++)
    HiData.getItem(i)->SetSortGroup(DEFAULT_SORT_GROUP+1+i-FirstCount-UpperCount);

  for (int i=FirstCount+UpperCount+LowerCount; i<FirstCount+UpperCount+LowerCount+LastCount; i++)
    HiData.getItem(i)->SetSortGroup(DEFAULT_SORT_GROUP);
}

int HighlightFiles::MenuPosToRealPos(int MenuPos, int **Count, bool Insert)
{
  int Pos=MenuPos;
  *Count=NULL;
  int x = Insert ? 1 : 0;

  if (MenuPos<FirstCount+x)
  {
    *Count=&FirstCount;
  }
  else if (MenuPos>FirstCount+1 && MenuPos<FirstCount+UpperCount+2+x)
  {
    Pos=MenuPos-2;
    *Count=&UpperCount;
  }
  else if (MenuPos>FirstCount+UpperCount+3 && MenuPos<FirstCount+UpperCount+LowerCount+4+x)
  {
    Pos=MenuPos-4;
    *Count=&LowerCount;
  }
  else if (MenuPos>FirstCount+UpperCount+LowerCount+5 && MenuPos<FirstCount+UpperCount+LowerCount+LastCount+6+x)
  {
    Pos=MenuPos-6;
    *Count=&LastCount;
  }

  return Pos;
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
      int Key=HiMenu.ReadInput();
      int SelectPos=HiMenu.GetSelectPos();

      NeedUpdate=FALSE;
      switch(Key)
      {
        /* $ 07.07.2000 IS
          Если нажали ctrl+r, то восстановить значения по умолчанию.
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
        case KEY_NUMDEL:
        case KEY_DEL:
          {
            int *Count=NULL;
            int RealSelectPos=MenuPosToRealPos(SelectPos,&Count);
            if (Count && RealSelectPos<(int)HiData.getCount())
            {
              const char *Mask;
              HiData.getItem(RealSelectPos)->GetMask(&Mask);
              if (Message(MSG_WARNING,2,MSG(MHighlightTitle),
                          MSG(MHighlightAskDel),Mask,
                          MSG(MDelete),MSG(MCancel))!=0)
                break;

              HiData.deleteItem(RealSelectPos);
              (*Count)--;
              NeedUpdate=TRUE;
            }
            break;
          }
        case KEY_NUMENTER:
        case KEY_ENTER:
        case KEY_F4:
        {
          int *Count=NULL;
          int RealSelectPos=MenuPosToRealPos(SelectPos,&Count);
          if (Count && RealSelectPos<(int)HiData.getCount())
            if (FileFilterConfig(HiData.getItem(RealSelectPos),true))
              NeedUpdate=TRUE;
          break;
        }
        case KEY_INS: case KEY_NUMPAD0:
        {
          int *Count=NULL;
          int RealSelectPos=MenuPosToRealPos(SelectPos,&Count,true);
          if (Count)
          {
            FileFilterParams *NewHData = HiData.insertItem(RealSelectPos);

            if (!NewHData)
              break;

            if (FileFilterConfig(NewHData,true))
            {
              (*Count)++;
              NeedUpdate=TRUE;
            }
            else
              HiData.deleteItem(RealSelectPos);
          }
          break;
        }
        case KEY_F5:
          {
            int *Count=NULL;
            int RealSelectPos=MenuPosToRealPos(SelectPos,&Count);
            if (Count && RealSelectPos<(int)HiData.getCount())
            {
              FileFilterParams *HData = HiData.insertItem(RealSelectPos);

              if (HData)
              {
                *HData = *HiData.getItem(RealSelectPos+1);
                HData->SetTitle("");
                if (FileFilterConfig(HData,true))
                {
                  NeedUpdate=TRUE;
                  (*Count)++;
                }
                else
                  HiData.deleteItem(RealSelectPos);
              }
            }
            break;
          }
        case KEY_CTRLUP: case KEY_CTRLNUMPAD8:
          {
            int *Count=NULL;
            int RealSelectPos=MenuPosToRealPos(SelectPos,&Count);
            if (Count && SelectPos > 0)
            {
              if (UpperCount && RealSelectPos==FirstCount && RealSelectPos<FirstCount+UpperCount)
              {
                FirstCount++;
                UpperCount--;
                SelectPos--;
              }
              else if (LowerCount && RealSelectPos==FirstCount+UpperCount && RealSelectPos<FirstCount+UpperCount+LowerCount)
              {
                UpperCount++;
                LowerCount--;
                SelectPos--;
              }
              else if (LastCount && RealSelectPos==FirstCount+UpperCount+LowerCount)
              {
                LowerCount++;
                LastCount--;
                SelectPos--;
              }
              else
                HiData.swapItems(RealSelectPos,RealSelectPos-1);
              HiMenu.SetSelection(--SelectPos);
              NeedUpdate=TRUE;
              break;
            }
            HiMenu.ProcessInput();
            break;
          }
        case KEY_CTRLDOWN: case KEY_CTRLNUMPAD2:
          {
            int *Count=NULL;
            int RealSelectPos=MenuPosToRealPos(SelectPos,&Count);
            if (Count && SelectPos < HiMenu.GetItemCount()-2)
            {
              if (FirstCount && RealSelectPos==FirstCount-1)
              {
                FirstCount--;
                UpperCount++;
                SelectPos++;
              }
              else if (UpperCount && RealSelectPos==FirstCount+UpperCount-1)
              {
                UpperCount--;
                LowerCount++;
                SelectPos++;
              }
              else if (LowerCount && RealSelectPos==FirstCount+UpperCount+LowerCount-1)
              {
                LowerCount--;
                LastCount++;
                SelectPos++;
              }
              else
                HiData.swapItems(RealSelectPos,RealSelectPos+1);
              HiMenu.SetSelection(++SelectPos);
              NeedUpdate=TRUE;
            }
            HiMenu.ProcessInput();
            break;
          }

        default:
          HiMenu.ProcessInput();
          break;
      }
      // повторяющийся кусок!
      if(NeedUpdate)
      {
        ScrBuf.Lock(); // отменяем всякую прорисовку
        HiMenu.Hide();

        ProcessGroups();

        if(Opt.AutoSaveSetup)
          SaveHiData();
        //FrameManager->RefreshFrame(); // рефрешим

        LeftPanel->Update(UPDATE_KEEP_SELECTION);
        LeftPanel->Redraw();
        RightPanel->Update(UPDATE_KEEP_SELECTION);
        RightPanel->Redraw();

        FillMenu(&HiMenu,MenuPos=SelectPos);
        HiMenu.SetPosition(-1,-1,0,0);
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

void SaveFilterToReg(FileFilterParams *CurHiData, const char *RegKey, bool bSortGroup)
{
  if (bSortGroup)
    SetRegKey(RegKey,HLS.UseMask,CurHiData->GetMask(NULL)?1:0);
  else
  {
    const char *Mask;
    SetRegKey(RegKey,HLS.IgnoreMask,(CurHiData->GetMask(&Mask)?0:1));
    SetRegKey(RegKey,HLS.Mask,Mask);
  }

  DWORD DateType;
  FILETIME DateAfter, DateBefore;
  bool bRelative;
  SetRegKey(RegKey,HLS.UseDate,CurHiData->GetDate(&DateType, &DateAfter, &DateBefore, &bRelative)?1:0);
  SetRegKey(RegKey,HLS.DateType,DateType);
  SetRegKey(RegKey,HLS.DateAfter,(BYTE *)&DateAfter,sizeof(DateAfter));
  SetRegKey(RegKey,HLS.DateBefore,(BYTE *)&DateBefore,sizeof(DateBefore));
  SetRegKey(RegKey,HLS.DateRelative,bRelative?1:0);

  const char *SizeAbove, *SizeBelow;
  SetRegKey(RegKey,HLS.UseSize,CurHiData->GetSize(&SizeAbove, &SizeBelow)?1:0);
  SetRegKey(RegKey,HLS.SizeAbove,SizeAbove);
  SetRegKey(RegKey,HLS.SizeBelow,SizeBelow);

  DWORD AttrSet, AttrClear;
  SetRegKey(RegKey,HLS.UseAttr,CurHiData->GetAttr(&AttrSet, &AttrClear)?1:0);
  SetRegKey(RegKey,(bSortGroup?HLS.AttrSet:HLS.IncludeAttributes),AttrSet);
  SetRegKey(RegKey,(bSortGroup?HLS.AttrClear:HLS.ExcludeAttributes),AttrClear);

  HighlightDataColor Colors;
  CurHiData->GetColors(&Colors);
  SetRegKey(RegKey,HLS.NormalColor,(DWORD)Colors.Color[HIGHLIGHTCOLORTYPE_FILE][HIGHLIGHTCOLOR_NORMAL]);
  SetRegKey(RegKey,HLS.SelectedColor,(DWORD)Colors.Color[HIGHLIGHTCOLORTYPE_FILE][HIGHLIGHTCOLOR_SELECTED]);
  SetRegKey(RegKey,HLS.CursorColor,(DWORD)Colors.Color[HIGHLIGHTCOLORTYPE_FILE][HIGHLIGHTCOLOR_UNDERCURSOR]);
  SetRegKey(RegKey,HLS.SelectedCursorColor,(DWORD)Colors.Color[HIGHLIGHTCOLORTYPE_FILE][HIGHLIGHTCOLOR_SELECTEDUNDERCURSOR]);
  SetRegKey(RegKey,HLS.MarkCharNormalColor,(DWORD)Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][HIGHLIGHTCOLOR_NORMAL]);
  SetRegKey(RegKey,HLS.MarkCharSelectedColor,(DWORD)Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][HIGHLIGHTCOLOR_SELECTED]);
  SetRegKey(RegKey,HLS.MarkCharCursorColor,(DWORD)Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][HIGHLIGHTCOLOR_UNDERCURSOR]);
  SetRegKey(RegKey,HLS.MarkCharSelectedCursorColor,(DWORD)Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][HIGHLIGHTCOLOR_SELECTEDUNDERCURSOR]);
  SetRegKey(RegKey,HLS.MarkChar,(DWORD)Colors.MarkChar);

  SetRegKey(RegKey,HLS.ContinueProcessing,(CurHiData->GetContinueProcessing()?1:0));
}

void HighlightFiles::SaveHiData()
{
  char RegKey[80],GroupName[80];
  const char *KeyNames[4]={RegColorsHighlight,SortGroupsKeyName,SortGroupsKeyName,RegColorsHighlight};
  const char *GroupNames[4]={fmtFirstGroup,fmtUpperGroup,fmtLowerGroup,fmtLastGroup};
  const int Count[4][2] = {
                            {0,                               FirstCount},
                            {FirstCount,                      FirstCount+UpperCount},
                            {FirstCount+UpperCount,           FirstCount+UpperCount+LowerCount},
                            {FirstCount+UpperCount+LowerCount,FirstCount+UpperCount+LowerCount+LastCount}
                          };

  for (int j=0; j<4; j++)
  {
    for (int i=Count[j][0]; i<Count[j][1]; i++)
    {
      sprintf(GroupName,GroupNames[j],i-Count[j][0]);
      sprintf(RegKey,"%s\\%s",KeyNames[j],GroupName);

      FileFilterParams *CurHiData=HiData.getItem(i);

      if (j!=0 && j!=3)
      {
        const char *Mask;
        CurHiData->GetMask(&Mask);
        SetRegKey(KeyNames[j],GroupName,Mask);
      }

      SaveFilterToReg(CurHiData,RegKey,(j==0 || j==3?false:true));
    }

    for (int i=0; i<5; i++)
    {
      sprintf(GroupName,GroupNames[j],Count[j][1]-Count[j][0]+i);
      sprintf(RegKey,"%s\\%s",KeyNames[j],GroupName);
      if (j!=0 && j!=3)
        DeleteRegValue(KeyNames[j],GroupName);
      DeleteRegKey(RegKey);
    }
  }
}

void SetHighlighting()
{
  if (CheckRegKey(RegColorsHighlight))
    return;

  char RegKey[80], *Ptr;
  // сразу пропишем %PATHEXT%, а FileFilterParams::SetMask() сам подстановку
  // сделает.
  char CmdExt[512]="*.exe,*.com,*.bat,%PATHEXT%";
  static const char *Masks[]={
  /* 0 */ "*.*",
  /* 1 */ "*.rar,*.zip,*.[zj],*.[bg7]z,*.[bg]zip,*.tar,*.t[agbx]z,*.ar[cj],*.r[0-9][0-9],*.a[0-9][0-9],*.bz2,*.cab,*.msi,*.jar,*.lha,*.lzh,*.ha,*.ac[bei],*.pa[ck],*.rk,*.cpio,*.rpm,*.zoo,*.hqx,*.sit,*.ice,*.uc2,*.ain,*.imp,*.777,*.ufa,*.boa,*.bs[2a],*.sea,*.hpk,*.ddi,*.x2,*.rkv,*.[lw]sz,*.h[ay]p,*.lim,*.sqz,*.chz",
  /* 2 */ "*.bak,*.tmp",                                                                                                                                                                                //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ -> может к терапевту? ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
          /* $ 25.09.2001  IS
              Эта маска для каталогов: обрабатывать все каталоги, кроме тех, что
              являются родительскими (их имена - две точки).
          */
  /* 3 */ "*.*|..", // маска для каталогов
  /* 4 */ "..",     // такие каталоги окрашивать как простые файлы
  };

  static struct DefaultData
  {
    const char *Mask;
    bool IgnoreMask;
    DWORD IncludeAttr;
    BYTE NormalColor;
    BYTE CursorColor;
  }
  StdHighlightData[]=
  { /*
             Mask                     NormalColor
                              IncludeAttributes
                       IgnoreMask           CursorColor             */
     /* 0 */{Masks[0], false, 0x0002, 0x13, 0x38},
     /* 1 */{Masks[0], false, 0x0004, 0x13, 0x38},
     /* 2 */{Masks[3], false, 0x0010, 0x1F, 0x3F},
     /* 3 */{Masks[4], false, 0x0010, 0x00, 0x00},
     /* 4 */{CmdExt,   false, 0x0000, 0x1A, 0x3A},
     /* 5 */{Masks[1], false, 0x0000, 0x1D, 0x3D},
     /* 6 */{Masks[2], false, 0x0000, 0x16, 0x36},
            // это настройка для каталогов на тех панелях, которые должны раскрашиваться
            // без учета масок (например, список хостов в "far navigator")
     /* 7 */{Masks[0], true,  0x0010, 0x1F, 0x3F},
  };

  // для NT добавляем CMD
  if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
    strcat(CmdExt,",*.cmd");

  Ptr=RegKey+strlen(strcat(strcpy(RegKey,RegColorsHighlight),"\\Group"));
  for(int I=0; I < sizeof(StdHighlightData)/sizeof(StdHighlightData[0]); I++)
  {
    itoa(I,Ptr,10);
    SetRegKey(RegKey,HLS.Mask,StdHighlightData[I].Mask);
    SetRegKey(RegKey,HLS.IgnoreMask,StdHighlightData[I].IgnoreMask?1:0);
    SetRegKey(RegKey,HLS.IncludeAttributes,StdHighlightData[I].IncludeAttr);
    SetRegKey(RegKey,HLS.NormalColor,StdHighlightData[I].NormalColor);
    SetRegKey(RegKey,HLS.CursorColor,StdHighlightData[I].CursorColor);
  }
}
