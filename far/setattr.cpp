/*
setattr.cpp

Установка атрибутов файлов

*/

/* Revision: 1.27 07.05.2001 $ */

/*
Modify:
  07.05.2001 SVS
    ! SysLog(); -> _D(SysLog());
  06.05.2001 DJ
    ! перетрях #include
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  12.04.2001 SVS
    ! Для FILE_ATTRIBUTE_REPARSE_POINT всегда показываем "Link to:",
      но, если данные не доступны - так и говорим - "НЕТУ!"
  09.04.2001 SVS
    - нужно было использовать локальные копии для параметров SrcDate и SrcTime
      в функции ReadFileTime
    ! немного оптимизации в функции ReadFileTime
  08.04.2001 SVS
    ! Полная независимость выставления значений для даты и времени.
    - Исправлен баг с "неустановкой" даты для одиночного файла.
  08.04.2001 IS
    - не работало изменение атрибутов после ?557
  04.04.2001 VVM
    + Кнопка [ Blank ] в диалоге. Очистить поля даты/времени.
  03.04.2001 SVS
    ! FillFileldDir -> FillingOfFields ;-)
  28.02.2001 SVS
    - Бага в Win2K с взаимоисключениями Сжатого и Шифрованного атрибута
    + Выставляем заголовок консоли во время процесса установки атрибутов
    + В месаге процесса установки отображаем текущЕЕ файлО.
  30.01.2001 SVS
    ! снимаем 3-state, если "есть все или нет ничего"
      за исключением случая, если есть Фолдер среди объектов
  23.01.2001 SVS
    + Немного оптимизации кода :-)
  22.01.2001 SVS
    ! ShellSetFileAttributes теперь возвращает результат в виде TRUE или FALSE
    + Если это плагиновая панель, то посмотрим на OPIF_REALNAMES
  22.01.2001 SVS
    + Больше интелектуальности диалогу установки атрибутов !!!! :-)))
      Теперь, для случая Multi, если есть подряд идущие атрибуты, то
      они изначально инициализируются как надо - либо [x] либо [ ] либо
      [*] для случая если "не все"
  14.01.2001 SVS
    + обработка случая, если ЭТО SymLink
  04.01.2001 SVS
    - Бага с одиночным файлом - переоптимизировал ;-(
  03.01.2001 SVS
    ! ускорим процесс за счет "необработки" подобных атрибут
    - бага с переходами между контролами
  03.01.2001 SVS
    ! новый имидж диалога атрибутов - один интелектуальный диалог на
      все случаи жизни :-)
  30.12.2000 SVS
    ! Функции для работы с файловыми атрибутами вынесены в fileattr.cpp
  21.12.2000 SVS
    ! Если папка одна, то включение "Process subfolders" не очищает
      область с атрибутами.
  14.12.2000 SVS
    ! Показываем недостающие атрибуты, но делаем их недоступными.
  24.11.2000 SVS
    + Правило на счет установки атрибутов на каталоги
  16.11.2000 SVS
    ! массивы для масок имеют постоянный адрес прописки - объявлены как static
  11.11.2000 SVS
    - "сложности" с криптованием :-))))
  02.11.2000 SVS
    - исправляем баги :-)
  20.10.2000 SVS
    + Новый атрибут Encripted (NTFS/Win2K)
  14.08.2000 KM
    ! Изменена инициализация диалога с учётом возможностей Mask.
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "fn.hpp"
#include "flink.hpp"
#include "global.hpp"
#include "lang.hpp"
#include "dialog.hpp"
#include "chgprior.hpp"
#include "scantree.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "savescr.hpp"

int OriginalCBAttr0[16]; // значения CheckBox`ов на момент старта диалога
int OriginalCBAttr[16]; // значения CheckBox`ов на момент старта диалога
int OriginalCBAttr2[16]; //
DWORD OriginalCBFlag[16];

static int ReadFileTime(int Type,char *Name,DWORD FileAttr,FILETIME *FileTime,
                       char *OSrcDate,char *OSrcTime)
{
  FILETIME ft, oft;
  SYSTEMTIME st, ost;
  unsigned DateN[3],TimeN[3];
  int DigitCount,I;
  int SetTime,GetTime;
  FILETIME OriginalFileTime;
  char SrcDate[32], SrcTime[32];
  char *Ptr,Digit[16],*PtrDigit;

  // ****** ОБРАБОТКА ДАТЫ ******** //
  strncpy(SrcDate,OSrcDate,sizeof(SrcDate));
  DateN[0]=DateN[1]=DateN[2]=(unsigned)-1;
  I=0;
  Ptr=SrcDate;
  int DateSeparator=GetDateSeparator();
  while((Ptr=GetCommaWord(Ptr,Digit,DateSeparator)) != NULL)
  {
    PtrDigit=Digit;
    while (*PtrDigit && !isdigit(*PtrDigit))
      PtrDigit++;
    if(*PtrDigit)
      DateN[I]=atoi(PtrDigit);
    ++I;
  }

  // ****** ОБРАБОТКА ВРЕМЕНИ ******** //
  strncpy(SrcTime,OSrcTime,sizeof(SrcTime));
  TimeN[0]=TimeN[1]=TimeN[2]=(unsigned)-1;
  I=0;
  Ptr=SrcTime;
  int TimeSeparator=GetTimeSeparator();
  while((Ptr=GetCommaWord(Ptr,Digit,TimeSeparator)) != NULL)
  {
    PtrDigit=Digit;
    while (*PtrDigit && !isdigit(*PtrDigit))
      PtrDigit++;
    if(*PtrDigit)
      TimeN[I]=atoi(PtrDigit);
    ++I;
  }

  // исключаем лишние телодвижения
  if(DateN[0] == -1 || DateN[1] == -1 || DateN[2] == -1 ||
     TimeN[0] == -1 || TimeN[1] == -1 || TimeN[2] == -1)
  {
    // получаем инфу про оригинальную дату и время файла.
    HANDLE hFile=CreateFile(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
                 NULL,OPEN_EXISTING,
                 (FileAttr & FA_DIREC) ? FILE_FLAG_BACKUP_SEMANTICS:0,NULL);
    if (hFile==INVALID_HANDLE_VALUE)
      return(FALSE);
    GetTime=GetFileTime(hFile,
                        (Type == 0?&OriginalFileTime:NULL),
                        (Type == 1?&OriginalFileTime:NULL),
                        (Type == 2?&OriginalFileTime:NULL));
    CloseHandle(hFile);

    if(!GetTime)
      return(FALSE);

    // конвертнем в локальное время.
    FileTimeToLocalFileTime(&OriginalFileTime,&oft);
    FileTimeToSystemTime(&oft,&ost);
    st.wDayOfWeek=ost.wDayOfWeek;
    st.wMilliseconds=ost.wMilliseconds;
    DigitCount=TRUE;
  }
  else
    DigitCount=FALSE;

  // "Оформим"
  switch(GetDateFormat())
  {
    case 0:
      st.wMonth=DateN[0]!=(unsigned)-1?DateN[0]:ost.wMonth;
      st.wDay  =DateN[1]!=(unsigned)-1?DateN[1]:ost.wDay;
      st.wYear =DateN[2]!=(unsigned)-1?DateN[2]:ost.wYear;
      break;
    case 1:
      st.wDay  =DateN[0]!=(unsigned)-1?DateN[0]:ost.wDay;
      st.wMonth=DateN[1]!=(unsigned)-1?DateN[1]:ost.wMonth;
      st.wYear =DateN[2]!=(unsigned)-1?DateN[2]:ost.wYear;
      break;
    default:
      st.wYear =DateN[0]!=(unsigned)-1?DateN[0]:ost.wYear;
      st.wMonth=DateN[1]!=(unsigned)-1?DateN[1]:ost.wMonth;
      st.wDay  =DateN[2]!=(unsigned)-1?DateN[2]:ost.wDay;
      break;
  }
  st.wHour   = TimeN[0]!=(unsigned)-1? (TimeN[0]):ost.wHour;
  st.wMinute = TimeN[1]!=(unsigned)-1? (TimeN[1]):ost.wMinute;
  st.wSecond = TimeN[2]!=(unsigned)-1? (TimeN[2]):ost.wSecond;

  if (st.wYear<100)
    if (st.wYear<80)
      st.wYear+=2000;
    else
      st.wYear+=1900;

  // преобразование в "удобоваримый" формат
  SystemTimeToFileTime(&st,&ft);
  LocalFileTimeToFileTime(&ft,FileTime);
  if(DigitCount)
    return (!CompareFileTime(FileTime,&OriginalFileTime))?FALSE:TRUE;
  return TRUE;
}


static void IncludeExcludeAttrib(int FocusPos,struct DialogItem *Item, int FocusPosSet, int FocusPosSkip)
{
  if(FocusPos == FocusPosSet && Item[FocusPosSet].Selected && Item[FocusPosSkip].Selected)
    Item[FocusPosSkip].Selected=0;
  if(FocusPos == FocusPosSkip && Item[FocusPosSkip].Selected && Item[FocusPosSet].Selected)
    Item[FocusPosSet].Selected=0;
}


static void EmptyDialog(struct DialogItem *AttrDlg,int ClrAttr,int SelCount1)
{
  if(ClrAttr)
  {
    AttrDlg[4].Selected=
    AttrDlg[5].Selected=
    AttrDlg[6].Selected=
    AttrDlg[7].Selected=
    AttrDlg[8].Selected=
    AttrDlg[9].Selected=SelCount1?0:2;
  }

  AttrDlg[16].Data[0]=
  AttrDlg[17].Data[0]=
  AttrDlg[19].Data[0]=
  AttrDlg[20].Data[0]=
  AttrDlg[22].Data[0]=
  AttrDlg[23].Data[0]='\0';
}

/* $ 22.11.2000 SVS
   Заполнение полей
*/
static void FillingOfFields(char *SelName,int FileAttr,
                          struct DialogItem *AttrDlg,
                          int SetAttr)
{
  HANDLE FindHandle;
  WIN32_FIND_DATA FindData;
  if ((FindHandle=FindFirstFile(SelName,&FindData))!=INVALID_HANDLE_VALUE)
  {
    FindClose(FindHandle);
    ConvertDate(&FindData.ftLastWriteTime, AttrDlg[16].Data,AttrDlg[17].Data,8,FALSE,FALSE,TRUE,TRUE);
    ConvertDate(&FindData.ftCreationTime,  AttrDlg[19].Data,AttrDlg[20].Data,8,FALSE,FALSE,TRUE,TRUE);
    ConvertDate(&FindData.ftLastAccessTime,AttrDlg[22].Data,AttrDlg[23].Data,8,FALSE,FALSE,TRUE,TRUE);
  }
  if(SetAttr)
  {
    AttrDlg[4].Selected=(FileAttr & FA_RDONLY)!=0;
    AttrDlg[5].Selected=(FileAttr & FA_ARCH)!=0;
    AttrDlg[6].Selected=(FileAttr & FA_HIDDEN)!=0;
    AttrDlg[7].Selected=(FileAttr & FA_SYSTEM)!=0;
    AttrDlg[8].Selected=(FileAttr & FILE_ATTRIBUTE_COMPRESSED)!=0;
    AttrDlg[9].Selected=(FileAttr & FILE_ATTRIBUTE_ENCRYPTED)!=0;
  }
}
/* SVS $ */


// обработчик диалога - пока это отлов нажатий нужных кнопок.
long WINAPI SetAttrDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  if(Msg == DN_BTNCLICK)
  {
    if(Param1 >= 4 && Param2 <= 9)
    {
      OriginalCBAttr[Param1-4] = Param2;
      OriginalCBAttr2[Param1-4] = 0;
    }
  }
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

int ShellSetFileAttributes(Panel *SrcPanel)
{
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
/*MSetAttrJunction
00                                             00
01   +------------ Attributes -------------+   01
02   |     Change file attributes for      |   02
03   |                 foo                 |   03
04   |          Link: blach blach          | < 04 <<
05   +-------------------------------------+   05
06   | [ ] Read only                       |   06
07   | [ ] Archive                         |   07
08   | [ ] Hidden                          |   08
09   | [ ] System                          |   09
10   | [ ] Compressed                      |   10
11   | [ ] Encrypted                       |   11
12   +-------------------------------------+   12
13   | [x] Process subfolders              |   13
14   +-------------------------------------+   14
15   |  File time      DD.MM.YYYY hh:mm:ss |   15
16   | Modification      .  .       :  :   |   16
17   | Creation          .  .       :  :   |   17
18   | Last access       .  .       :  :   |   18
19   |               [ Current ] [ Blank ] |   19
20   +-------------------------------------+   20
21   |         [ Set ]  [ Cancel ]         |   21
22   +-------------------------------------+   22
23                                             23
*/
  static struct DialogData AttrDlgData[]={
  /* 00 */DI_DOUBLEBOX,3,1,41,21,0,0,0,0,(char *)MSetAttrTitle,
  /* 01 */DI_TEXT,-1,2,0,0,0,0,0,0,(char *)MSetAttrFor,
  /* 02 */DI_TEXT,-1,3,0,0,0,0,DIF_SHOWAMPERSAND,0,"",
  /* 03 */DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 04 */DI_CHECKBOX,5, 5,0,0,1,0,DIF_3STATE,0,(char *)MSetAttrRO,
  /* 05 */DI_CHECKBOX,5, 6,0,0,0,0,DIF_3STATE,0,(char *)MSetAttrArchive,
  /* 06 */DI_CHECKBOX,5, 7,0,0,0,0,DIF_3STATE,0,(char *)MSetAttrHidden,
  /* 07 */DI_CHECKBOX,5, 8,0,0,0,0,DIF_3STATE,0,(char *)MSetAttrSystem,
  /* 08 */DI_CHECKBOX,5, 9,0,0,0,0,DIF_3STATE,0,(char *)MSetAttrCompressed,
  /* 09 */DI_CHECKBOX,5,10,0,0,0,0,DIF_3STATE,0,(char *)MSetAttrEncrypted,
  /* 10 */DI_TEXT,3,11,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 11 */DI_CHECKBOX,5,12,0,0,0,0,DIF_DISABLE,0,(char *)MSetAttrSubfolders,
  /* 12 */DI_TEXT,3,13,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 13 */DI_TEXT,6,14,0,0,0,0,DIF_BOXCOLOR,0,(char *)MSetAttrFileTime,
  /* 14 */DI_TEXT,21,14,0,0,0,0,0,0,"",
  /* 15 */DI_TEXT,    5,15,0,0,0,0,0,0,(char *)MSetAttrModification,
  /* 16 */DI_FIXEDIT,21,15,30,15,0,0,DIF_MASKEDIT,0,"",
  /* 17 */DI_FIXEDIT,32,15,39,15,0,0,DIF_MASKEDIT,0,"",
  /* 18 */DI_TEXT,    5,16,0,0,0,0,0,0,(char *)MSetAttrCreation,
  /* 19 */DI_FIXEDIT,21,16,30,16,0,0,DIF_MASKEDIT,0,"",
  /* 20 */DI_FIXEDIT,32,16,39,16,0,0,DIF_MASKEDIT,0,"",
  /* 21 */DI_TEXT,    5,17,0,0,0,0,0,0,(char *)MSetAttrLastAccess,
  /* 22 */DI_FIXEDIT,21,17,30,17,0,0,DIF_MASKEDIT,0,"",
  /* 23 */DI_FIXEDIT,32,17,39,17,0,0,DIF_MASKEDIT,0,"",
  /* 24 */DI_BUTTON,19,18,0,0,0,0,0,0,(char *)MSetAttrCurrent,
  /* 25 */DI_BUTTON,31,18,0,0,0,0,0,0,(char *)MSetAttrBlank,
  /* 26 */DI_TEXT,3,19,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 27 */DI_BUTTON,0,20,0,0,0,0,DIF_CENTERGROUP,1,(char *)MSetAttrSet,
  /* 28 */DI_BUTTON,0,20,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel,
  /* 29 */DI_TEXT,-1,4,0,0,0,0,DIF_SHOWAMPERSAND,0,"",
  };
  MakeDialogItems(AttrDlgData,AttrDlg);
  int DlgCountItems=sizeof(AttrDlgData)/sizeof(AttrDlgData[0])-1;

  DWORD FileSystemFlags;
  int SelCount, I, J;

  if((SelCount=SrcPanel->GetSelCount())==0)
    return 0;

  if (SrcPanel->GetMode()==PLUGIN_PANEL)
  {
    struct OpenPluginInfo Info;
    HANDLE hPlugin=SrcPanel->GetPluginHandle();
    if(hPlugin == INVALID_HANDLE_VALUE)
      return 0;

    CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
    if(!(Info.Flags & OPIF_REALNAMES))
      return 0;
  }

  FileSystemFlags=0;
  if (GetVolumeInformation(NULL,NULL,0,NULL,NULL,&FileSystemFlags,NULL,0))
  {
    if (!(FileSystemFlags & FS_FILE_COMPRESSION))
      AttrDlg[8].Flags|=DIF_DISABLE;

    if (!IsCryptFileASupport || !(FileSystemFlags & FS_FILE_ENCRYPTION))
      AttrDlg[9].Flags|=DIF_DISABLE;
  }

  {
    char SelName[NM];
    int FileAttr;
    FILETIME LastWriteTime,CreationTime,LastAccessTime;
    int SetWriteTime,SetCreationTime,SetLastAccessTime;

    SaveScreen SaveScr;

    SrcPanel->GetSelName(NULL,FileAttr);
    SrcPanel->GetSelName(SelName,FileAttr);

    if (SelCount==0 || SelCount==1 && strcmp(SelName,"..")==0)
      return 0;

    int FocusPos;
    int NewAttr;
    int FolderPresent=FALSE, JunctionPresent=FALSE;
    char TimeText[6][100];

    int DateSeparator=GetDateSeparator();
    int TimeSeparator=GetTimeSeparator();
    static char DMask[20],TMask[20];

    sprintf(TMask,"99%c99%c99",TimeSeparator,TimeSeparator);
    switch(GetDateFormat())
    {
      case 0:
        sprintf(AttrDlg[14].Data,MSG(MSetAttrTimeTitle1),DateSeparator,DateSeparator,TimeSeparator,TimeSeparator);
        sprintf(DMask,"99%c99%c9999",DateSeparator,DateSeparator);
        break;
      case 1:
        sprintf(AttrDlg[14].Data,MSG(MSetAttrTimeTitle2),DateSeparator,DateSeparator,TimeSeparator,TimeSeparator);
        sprintf(DMask,"99%c99%c9999",DateSeparator,DateSeparator);
        break;
      default:
        sprintf(AttrDlg[14].Data,MSG(MSetAttrTimeTitle3),DateSeparator,DateSeparator,TimeSeparator,TimeSeparator);
        sprintf(DMask,"9999%c99%c99",DateSeparator,DateSeparator);
        break;
    }

    AttrDlg[16].Mask=DMask;
    AttrDlg[17].Mask=TMask;
    AttrDlg[19].Mask=DMask;
    AttrDlg[20].Mask=TMask;
    AttrDlg[22].Mask=DMask;
    AttrDlg[23].Mask=TMask;

    if (SelCount==1)
    {
      if((FileAttr & FA_DIREC))
      {
        AttrDlg[11].Flags&=~DIF_DISABLE;
        AttrDlg[11].Selected=Opt.SetAttrFolderRules == 1?0:1;
        if(Opt.SetAttrFolderRules)
        {
          FillingOfFields(SelName,FileAttr,AttrDlg,1);
          // убираем 3-State
          for(I=4; I <= 9; ++I)
            AttrDlg[I].Flags&=~DIF_3STATE;
        }
        FolderPresent=TRUE;

        // обработка случая, если ЭТО SymLink
        if(FileAttr&FILE_ATTRIBUTE_REPARSE_POINT)
        {
          char JuncName[NM*2];
          DWORD LenJunction=GetJunctionPointInfo(SelName,JuncName,sizeof(JuncName));
          //"\??\D:\Junc\Src\" или "\\?\Volume{..."

          AttrDlg[0].Y2++;
          for(I=3; I  < DlgCountItems; ++I)
          {
            AttrDlg[I].Y1++;
            AttrDlg[I].Y2++;
          }
          DlgCountItems++;
          JunctionPresent=TRUE;

          sprintf(AttrDlg[29].Data,MSG(MSetAttrJunction),
                (LenJunction?
                   TruncPathStr(JuncName+4,29):
                   MSG(MSetAttrUnknownJunction)));
        }
      }
      else
      {
        // убираем 3-State
        for(I=4; I <= 9; ++I)
          AttrDlg[I].Flags&=~DIF_3STATE;
      }

      strcpy(AttrDlg[2].Data,SelName);
      TruncStr(AttrDlg[2].Data,30);

      AttrDlg[4].Selected=(FileAttr & FA_RDONLY)!=0;
      AttrDlg[5].Selected=(FileAttr & FA_ARCH)!=0;
      AttrDlg[6].Selected=(FileAttr & FA_HIDDEN)!=0;
      AttrDlg[7].Selected=(FileAttr & FA_SYSTEM)!=0;
      AttrDlg[8].Selected=(FileAttr & FILE_ATTRIBUTE_COMPRESSED)!=0;
      AttrDlg[9].Selected=(FileAttr & FILE_ATTRIBUTE_ENCRYPTED)!=0;

      {
        HANDLE FindHandle;
        WIN32_FIND_DATA FindData;
        if ((FindHandle=FindFirstFile(SelName,&FindData))!=INVALID_HANDLE_VALUE)
        {
          FindClose(FindHandle);
          ConvertDate(&FindData.ftLastWriteTime,AttrDlg[16].Data,AttrDlg[17].Data,8,FALSE,FALSE,TRUE,TRUE);
          ConvertDate(&FindData.ftCreationTime,AttrDlg[19].Data,AttrDlg[20].Data,8,FALSE,FALSE,TRUE,TRUE);
          ConvertDate(&FindData.ftLastAccessTime,AttrDlg[22].Data,AttrDlg[23].Data,8,FALSE,FALSE,TRUE,TRUE);
        }
      }
      strcpy(TimeText[0],AttrDlg[16].Data);
      strcpy(TimeText[1],AttrDlg[17].Data);
      strcpy(TimeText[2],AttrDlg[19].Data);
      strcpy(TimeText[3],AttrDlg[20].Data);
      strcpy(TimeText[4],AttrDlg[22].Data);
      strcpy(TimeText[5],AttrDlg[23].Data);
    }
    else
    {
      EmptyDialog(AttrDlg,1,0);

      strcpy(AttrDlg[2].Data,MSG(MSetAttrSelectedObjects));
      // выставим -1 - потом учтем этот факт :-)
      for(I=4; I <= 9; ++I)
        AttrDlg[I].Selected=0;

      // проверка - есть ли среди выделенных - каталоги?
      // так же проверка на атрибуты
      J=0;
      SrcPanel->GetSelName(NULL,FileAttr);
      while (SrcPanel->GetSelName(SelName,FileAttr))
      {
        if(!J && (FileAttr & FA_DIREC))
        {
          FolderPresent=TRUE;
          AttrDlg[11].Flags&=~DIF_DISABLE;
          J++;
        }
        AttrDlg[4].Selected+=(FileAttr & FA_RDONLY)?1:0;
        AttrDlg[5].Selected+=(FileAttr & FA_ARCH)?1:0;
        AttrDlg[6].Selected+=(FileAttr & FA_HIDDEN)?1:0;
        AttrDlg[7].Selected+=(FileAttr & FA_SYSTEM)?1:0;
        AttrDlg[8].Selected+=(FileAttr & FILE_ATTRIBUTE_COMPRESSED)?1:0;
        AttrDlg[9].Selected+=(FileAttr & FILE_ATTRIBUTE_ENCRYPTED)?1:0;
      }
      SrcPanel->GetSelName(NULL,FileAttr);
      SrcPanel->GetSelName(SelName,FileAttr);
      // выставим "неопределенку" или то, что нужно
      for(I=4; I <= 9; ++I)
      {
        J=AttrDlg[I].Selected;
        // снимаем 3-state, если "есть все или нет ничего"
        // за исключением случая, если есть Фолдер среди объектов
        if((!J || J >= SelCount) && !FolderPresent)
          AttrDlg[I].Flags&=~DIF_3STATE;

        AttrDlg[I].Selected=(J >= SelCount)?1:(!J?0:2);
      }
    }

    // запомним состояние переключателей.
    for(I=4; I <= 9; ++I)
    {
      OriginalCBAttr0[I-4]=OriginalCBAttr[I-4]=AttrDlg[I].Selected;
      OriginalCBAttr2[I-4]=-1;
      OriginalCBFlag[I-4]=AttrDlg[I].Flags;
    }

    if (SelCount==1 && (FileAttr & FA_DIREC)==0)
    {
      int NewAttr;

      {
        Dialog Dlg(AttrDlg,DlgCountItems);
        Dlg.SetHelp("FileAttrDlg");
        Dlg.SetPosition(-1,-1,45,JunctionPresent?24:23);

        while (1)
        {
          Dlg.Show();
          while (!Dlg.Done())
          {
            Dlg.ReadInput();
            Dlg.ProcessInput();
            FocusPos=Dialog::SendDlgMessage((HANDLE)&Dlg,DM_GETFOCUS,0,0);
            if(((FileSystemFlags & (FS_FILE_COMPRESSION|FS_FILE_ENCRYPTION))==
                 (FS_FILE_COMPRESSION|FS_FILE_ENCRYPTION)) &&
               (FocusPos == 8 || FocusPos == 9))
            {
              IncludeExcludeAttrib(FocusPos,AttrDlg,8,9);
              Dlg.FastShow();
            }
          }
          Dlg.GetDialogObjectsData();

          int Code = Dlg.GetExitCode();

          if ((Code!=24) && (Code!=25))
            break;

          if (Code == 24)
          {
            FILETIME ft;
            GetSystemTimeAsFileTime(&ft);
            ConvertDate(&ft,AttrDlg[16].Data,AttrDlg[17].Data,8,FALSE,FALSE,TRUE,TRUE);
            ConvertDate(&ft,AttrDlg[19].Data,AttrDlg[20].Data,8,FALSE,FALSE,TRUE,TRUE);
            ConvertDate(&ft,AttrDlg[22].Data,AttrDlg[23].Data,8,FALSE,FALSE,TRUE,TRUE);
          }
          else
          {
            AttrDlg[16].Data[0]=AttrDlg[17].Data[0]=0;
            AttrDlg[19].Data[0]=AttrDlg[20].Data[0]=0;
            AttrDlg[22].Data[0]=AttrDlg[23].Data[0]=0;
          }
          Dlg.SendDlgMessage((HANDLE)&Dlg,DM_SETFOCUS,16,0);
          Dlg.ClearDone();
          Dlg.InitDialogObjects();
        }

        /* $ 08.04.2001 IS
             Акелла промахнулся на охоте...
        */
        if (Dlg.GetExitCode()!=27)
          return 0;
        /* IS $ */

        Dlg.Hide();
      }

      NewAttr=FileAttr & FA_DIREC;
      if (AttrDlg[4].Selected)        NewAttr|=FA_RDONLY;
      if (AttrDlg[5].Selected)        NewAttr|=FA_ARCH;
      if (AttrDlg[6].Selected)        NewAttr|=FA_HIDDEN;
      if (AttrDlg[7].Selected)        NewAttr|=FA_SYSTEM;
      if (AttrDlg[8].Selected)        NewAttr|=FILE_ATTRIBUTE_COMPRESSED;
      if (AttrDlg[9].Selected)        NewAttr|=FILE_ATTRIBUTE_ENCRYPTED;

      Message(0,0,MSG(MSetAttrTitle),MSG(MSetAttrSetting));

      SetWriteTime=ReadFileTime(0,SelName,FileAttr,&LastWriteTime,AttrDlg[16].Data,AttrDlg[17].Data);
      SetCreationTime=ReadFileTime(1,SelName,FileAttr,&CreationTime,AttrDlg[19].Data,AttrDlg[20].Data);
      SetLastAccessTime=ReadFileTime(2,SelName,FileAttr,&LastAccessTime,AttrDlg[22].Data,AttrDlg[23].Data);

      if(SetWriteTime || SetCreationTime || SetLastAccessTime)
        SetWriteTime=ESetFileTime(SelName,SetWriteTime ? &LastWriteTime:NULL,
                     SetCreationTime ? &CreationTime:NULL,
                     SetLastAccessTime ? &LastAccessTime:NULL,FileAttr);
      else
        SetWriteTime=TRUE;

//      if(NewAttr != (FileAttr & (~FA_DIREC))) // нужно ли что-нить менять???
      if(SetWriteTime) // если время удалось выставить...
      {
        if((NewAttr&FILE_ATTRIBUTE_COMPRESSED) && !(FileAttr&FILE_ATTRIBUTE_COMPRESSED))
          ESetFileCompression(SelName,1,FileAttr);
        else if(!(NewAttr&FILE_ATTRIBUTE_COMPRESSED) && (FileAttr&FILE_ATTRIBUTE_COMPRESSED))
          ESetFileCompression(SelName,0,FileAttr);
        else if((NewAttr&FILE_ATTRIBUTE_ENCRYPTED) && !(FileAttr&FILE_ATTRIBUTE_ENCRYPTED))
          ESetFileEncryption(SelName,1,FileAttr);
        else if(!(NewAttr&FILE_ATTRIBUTE_ENCRYPTED) && (FileAttr&FILE_ATTRIBUTE_ENCRYPTED))
          ESetFileEncryption(SelName,0,FileAttr);
        ESetFileAttributes(SelName,NewAttr&(~(FILE_ATTRIBUTE_ENCRYPTED|FILE_ATTRIBUTE_COMPRESSED)));
      }
    }

    /* Multi *********************************************************** */
    else
    {
      int SetAttr,ClearAttr,Cancel=0;
      char OldConsoleTitle[NM], OutFileName[72],TmpFileName[72];

//      EmptyDialog(AttrDlg,1,SelCount==1);

      {
        int RefreshNeed=FALSE;
        Dialog Dlg(AttrDlg,DlgCountItems,SetAttrDlgProc);
        Dlg.SetHelp("FileAttrDlg");
        Dlg.SetPosition(-1,-1,45,JunctionPresent?24:23);

        Dlg.Show();
        while (1)
        {
          int Sel11=AttrDlg[11].Selected;
          int Sel8=AttrDlg[8].Selected;
          int Sel9=AttrDlg[9].Selected;
          while (!Dlg.Done())
          {
            Dlg.ReadInput();
            Dlg.ProcessInput();
            FocusPos=Dialog::SendDlgMessage((HANDLE)&Dlg,DM_GETFOCUS,0,0);
            // отработаем взаимоисключения
            if(((FileSystemFlags & (FS_FILE_COMPRESSION|FS_FILE_ENCRYPTION))==
                 (FS_FILE_COMPRESSION|FS_FILE_ENCRYPTION)) &&
               (FocusPos == 8 || FocusPos == 9))
            {
              if(FocusPos == 8 && Sel8 != AttrDlg[8].Selected) // Состояние изменилось?
              {
                if(AttrDlg[8].Selected == 1 && AttrDlg[9].Selected)
                   AttrDlg[9].Selected=0;
                else if(AttrDlg[8].Selected == 2)
                   AttrDlg[9].Selected=2;
                RefreshNeed=TRUE;
              }
              else if(FocusPos == 9 && Sel9 != AttrDlg[9].Selected) // Состояние изменилось?
              {
                if(AttrDlg[9].Selected == 1 && AttrDlg[8].Selected)
                   AttrDlg[8].Selected=0;
                else if(AttrDlg[9].Selected == 2)
                   AttrDlg[8].Selected=2;
                RefreshNeed=TRUE;
              }
              Sel9=AttrDlg[9].Selected;
              Sel8=AttrDlg[8].Selected;
            }

            // если снимаем атрибуты для SubFolders
            // этот кусок всегда работает если есть хотя бы одна папка
            // иначе 11-й недоступен и всегда снят.
            if(FocusPos == 11)
            {
              if(SelCount==1) // каталог однозначно!
              {
                if(Sel11 != AttrDlg[11].Selected) // Состояние изменилось?
                {
//                  EmptyDialog(AttrDlg,1,1);
                  // убираем 3-State
                  for(I=4; I <= 9; ++I)
                  {
                    if(!AttrDlg[11].Selected) // сняли?
                    {
                      AttrDlg[I].Selected=OriginalCBAttr[I-4];
                      AttrDlg[I].Flags&=~DIF_3STATE;
                    }
                    else                      // установили?
                    {
                      AttrDlg[I].Flags|=DIF_3STATE;
                      if(OriginalCBAttr2[I-4] == -1)
                        AttrDlg[I].Selected=2;
                    }
                  }
                  if(!AttrDlg[11].Selected)
                    FillingOfFields(SelName,FileAttr,AttrDlg,0);
                  RefreshNeed=TRUE;
                }
              }
              else  // много объектов
              {
                if(Sel11 != AttrDlg[11].Selected) // Состояние изменилось?
                {
//                  EmptyDialog(AttrDlg,1,0);
                  for(I=4; I <= 9; ++I)
                  {
                    if(!AttrDlg[11].Selected) // сняли?
                    {
                      AttrDlg[I].Selected=OriginalCBAttr[I-4];
                      AttrDlg[I].Flags=OriginalCBFlag[I-4];
                    }
                    else                      // установили?
                    {
                      if(OriginalCBAttr2[I-4] == -1)
                      {
                        AttrDlg[I].Flags|=DIF_3STATE;
                        AttrDlg[I].Selected=2;
                      }
                    }
                  }
                  RefreshNeed=TRUE;
                }
              }
              Sel11=AttrDlg[11].Selected;
            }

            if(RefreshNeed)
            {
              RefreshNeed=FALSE;
              Dlg.InitDialogObjects();
              Dlg.Show();
            }
          }
          Dlg.GetDialogObjectsData();

          int Code = Dlg.GetExitCode();

          if ((Code!=24) && (Code!=25))
            break;

          if (Code == 24)
          {
            FILETIME ft;
            GetSystemTimeAsFileTime(&ft);
            ConvertDate(&ft,AttrDlg[16].Data,AttrDlg[17].Data,8,FALSE,FALSE,TRUE,TRUE);
            ConvertDate(&ft,AttrDlg[19].Data,AttrDlg[20].Data,8,FALSE,FALSE,TRUE,TRUE);
            ConvertDate(&ft,AttrDlg[22].Data,AttrDlg[23].Data,8,FALSE,FALSE,TRUE,TRUE);
          }
          else
          {
            AttrDlg[16].Data[0]=AttrDlg[17].Data[0]=0;
            AttrDlg[19].Data[0]=AttrDlg[20].Data[0]=0;
            AttrDlg[22].Data[0]=AttrDlg[23].Data[0]=0;
          }
          Dlg.SendDlgMessage((HANDLE)&Dlg,DM_SETFOCUS,16,0);
          Dlg.ClearDone();
          Dlg.InitDialogObjects();
          Dlg.Show();
        }

        /* $ 08.04.2001 IS
             Акелла промахнулся на охоте...
        */
        if (Dlg.GetExitCode()!=27)
          return 0;
        /* IS $ */
      }

      CtrlObject->Cp()->GetAnotherPanel(SrcPanel)->CloseFile();

      SetAttr=0;  ClearAttr=0;

      if (AttrDlg[4].Selected == 1)         SetAttr|=FA_RDONLY;
      else if (!AttrDlg[4].Selected)        ClearAttr|=FA_RDONLY;
      if (AttrDlg[5].Selected == 1)         SetAttr|=FA_ARCH;
      else if (!AttrDlg[5].Selected)        ClearAttr|=FA_ARCH;
      if (AttrDlg[6].Selected == 1)         SetAttr|=FA_HIDDEN;
      else if (!AttrDlg[6].Selected)        ClearAttr|=FA_HIDDEN;
      if (AttrDlg[7].Selected == 1)         SetAttr|=FA_SYSTEM;
      else if (!AttrDlg[7].Selected)        ClearAttr|=FA_SYSTEM;

      if (AttrDlg[8].Selected == 1)
      {
        SetAttr|=FILE_ATTRIBUTE_COMPRESSED;
        ClearAttr|=FILE_ATTRIBUTE_ENCRYPTED;
      }
      else if (!AttrDlg[8].Selected)
        ClearAttr|=FILE_ATTRIBUTE_COMPRESSED;
      else if (AttrDlg[9].Selected == 1)
      {
        SetAttr|=FILE_ATTRIBUTE_ENCRYPTED;
        ClearAttr|=FILE_ATTRIBUTE_COMPRESSED;
      }
      else if (!AttrDlg[9].Selected)
        ClearAttr|=FILE_ATTRIBUTE_ENCRYPTED;

      // добавим заголовок окна
      GetConsoleTitle(OldConsoleTitle,sizeof(OldConsoleTitle));
      SetFarTitle(MSG(MSetAttrTitle));

      SrcPanel->GetSelName(NULL,FileAttr);
      while (SrcPanel->GetSelName(SelName,FileAttr) && !Cancel)
      {
//_D(SysLog("SelName='%s'\n\tFileAttr =0x%08X\n\tSetAttr  =0x%08X\n\tClearAttr=0x%08X\n\tResult   =0x%08X",
// SelName,FileAttr,SetAttr,ClearAttr,((FileAttr|SetAttr)&(~ClearAttr))));
        Message(0,0,MSG(MSetAttrTitle),MSG(MSetAttrSetting),
          CenterStr(TruncPathStr(strcpy(OutFileName,SelName),40),OutFileName,44));

        if (CheckForEsc())
          break;

        SetWriteTime=ReadFileTime(0,SelName,FileAttr,&LastWriteTime,AttrDlg[16].Data,AttrDlg[17].Data);
        SetCreationTime=ReadFileTime(1,SelName,FileAttr,&CreationTime,AttrDlg[19].Data,AttrDlg[20].Data);
        SetLastAccessTime=ReadFileTime(2,SelName,FileAttr,&LastAccessTime,AttrDlg[22].Data,AttrDlg[23].Data);
        if(SetWriteTime || SetCreationTime || SetLastAccessTime)
          if (!ESetFileTime(SelName,SetWriteTime ? &LastWriteTime:NULL,
                       SetCreationTime ? &CreationTime:NULL,
                       SetLastAccessTime ? &LastAccessTime:NULL,FileAttr))
            break;

        if(((FileAttr|SetAttr)&(~ClearAttr)) != FileAttr)
        {
          if (AttrDlg[8].Selected == 1) // -E +C
          {
            if (!ESetFileCompression(SelName,1,FileAttr))
              break; // неудача сжать :-(
          }
          else if (AttrDlg[9].Selected == 1) // +E -C
          {
            if (!ESetFileEncryption(SelName,1,FileAttr))
              break; // неудача зашифровать :-(
          }
          else //???
          if (AttrDlg[8].Selected == 0) // -C ?E
          {
            if (!ESetFileCompression(SelName,0,FileAttr))
              break; // неудача разжать :-(
          }
          else if (AttrDlg[9].Selected == 0) // ?C -E
          {
            if (!ESetFileEncryption(SelName,0,FileAttr))
              break; // неудача разшифровать :-(
          }

          if (!ESetFileAttributes(SelName,(FileAttr|SetAttr)&(~ClearAttr)))
            break;
        }

        if ((FileAttr & FA_DIREC) && AttrDlg[11].Selected)
        {
          char FullName[NM];
          ScanTree ScTree(FALSE);
          WIN32_FIND_DATA FindData;

          ScTree.SetFindPath(SelName,"*.*");
          while (ScTree.GetNextName(&FindData,FullName))
          {
            Message(0,0,MSG(MSetAttrTitle),MSG(MSetAttrSetting),
              CenterStr(TruncPathStr(strcpy(OutFileName,FullName),40),OutFileName,44));
            if (CheckForEsc())
            {
              Cancel=1;
              break;
            }
            SetWriteTime=ReadFileTime(0,FullName,FindData.dwFileAttributes,&LastWriteTime,AttrDlg[16].Data,AttrDlg[17].Data);
            SetCreationTime=ReadFileTime(1,FullName,FindData.dwFileAttributes,&CreationTime,AttrDlg[19].Data,AttrDlg[20].Data);
            SetLastAccessTime=ReadFileTime(2,FullName,FindData.dwFileAttributes,&LastAccessTime,AttrDlg[22].Data,AttrDlg[23].Data);
            if (SetWriteTime || SetCreationTime || SetLastAccessTime)
            {
              if (!ESetFileTime(FullName,SetWriteTime ? &LastWriteTime:NULL,
                           SetCreationTime ? &CreationTime:NULL,
                           SetLastAccessTime ? &LastAccessTime:NULL,
                           FindData.dwFileAttributes))
              {
                Cancel=1;
                break;
              }
            }
            if(((FindData.dwFileAttributes|SetAttr)&(~ClearAttr)) !=
                 FindData.dwFileAttributes)
            {
              if (AttrDlg[8].Selected == 1) // -E +C
              {
                if (!ESetFileCompression(FullName,1,FindData.dwFileAttributes))
                {
                  Cancel=1;
                  break; // неудача сжать :-(
                }
              }
              else if (AttrDlg[9].Selected == 1) // +E -C
              {
                if (!ESetFileEncryption(FullName,1,FindData.dwFileAttributes))
                {
                  Cancel=1;
                  break; // неудача зашифровать :-(
                }
              }
              else //???
              if (!AttrDlg[8].Selected) // -C ?E
              {
                if (!ESetFileCompression(FullName,0,FindData.dwFileAttributes))
                {
                  Cancel=1;
                  break; // неудача разжать :-(
                }
              }
              else if (!AttrDlg[9].Selected) // ?C -E
              {
                if (!ESetFileEncryption(FullName,0,FindData.dwFileAttributes))
                {
                  Cancel=1;
                  break; // неудача разшифровать :-(
                }
              }
              if (!ESetFileAttributes(FullName,(FindData.dwFileAttributes|SetAttr)&(~ClearAttr)))
              {
                Cancel=1;
                break;
              }
            }
          }
        }
      } // END: while (SrcPanel->GetSelName(...))
      SetConsoleTitle(OldConsoleTitle);
    }
  }

  SrcPanel->SaveSelection();
  SrcPanel->Update(UPDATE_KEEP_SELECTION);
  SrcPanel->ClearSelection();
  SrcPanel->Redraw();
  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(SrcPanel);
  AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
  AnotherPanel->Redraw();
  return 1;
}
