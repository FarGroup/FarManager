/*
setattr.cpp

Установка атрибутов файлов

*/

/* Revision: 1.45 17.12.2001 $ */

/*
Modify:
  17.12.2001 SVS
    - Бага, аднака - для одиночного файлового объекта месаг процесса установки
      атрибутов был пустым.
    ! Часть кода по разбору даты и времени вынесена в функцию GetFileDateAndTime()
  08.11.2001 SVS
    ! Уточнение ширины - прагала падлюка.
  06.11.2001 SVS
    ! Ширина месага при удалении файлов и выставлении атрибутов динамически
      меняется в сторону увеличения (с подачи SF), начиная с min 30 символов.
  24.10.2001 SVS
    - Уберем флаг MSG_KEEPBACKGROUND для месага.
  23.10.2001 SVS
    - неверное выставлялись атрибуты для нескольких объектов
    ! немного уточнений по поводу вывода текущего обрабатываемого файла
    - Артефакт с прорисовкой после внедрения CALLBACK-функции (когда 1 панель
      погашена - остается кусок месагбокса)
  21.10.2001 SVS
    + CALLBACK-функция для избавления от BugZ#85
  12.10.2001 SVS
    ! Ну охренеть (Opt.FolderSetAttr165!!!) - уже и так есть то, что надо:
      Opt.SetAttrFolderRules!
    + кнопка "Original" - исходное значение файловых времен.
  11.10.2001 SVS
    + Opt.FolderSetAttr165; // поведение для каталогов как у 1.65
  27.09.2001 IS
    - Левый размер при использовании strncpy
  24.09.2001 SVS
    - В деревяхе Ctrl-A неверно отображало инфу для симлинков.
  11.09.2001 SVS
    + для "volume mount point" укажем что это именно монтированный том и по
      возможности (если имя диска есть) покажем букву диска. Для прочих
      символических связей оставим "Link"
  11.09.2001 SVS
    - BugZ#7: Уточнение по поводу слинкованной файловой системы отличной от NTFS.
  17.07.2001 SKV
    - баг в ReadFileTime. Псевдопотенциальный, в debug срабатывал всегда.
    ! закомментировал несколько неюзаемых переменных, сократив кол-во варнингов
  25.06.2001 IS
    ! Внедрение const
  10.06.2001 SVS
    - ошибка в логике при работе с атрибутами для каталога
  20.05.2001 SVS
    - воздействие на кнопки "текущее" и "пусто" приводило к закрытию дислога
    + реакция на клик мыши на лейбах Modification, Creation и Last access
      перемещает фокус ввода на ближайшее поле ввода.
  18.05.2001 SVS
    ! Вся логика работы диалога перенесена на уровень диалогой процедуры.
      Осталось и инициализирующую часть впихнуть по назначению ;-)
  14.05.2001 SVS
    - Проблемы с выставлением времени файла
    + DOUBLE_CLICK: ;-)
      1) на лейбаках: Modification, Creation & Last access
         выставляет текущую дату для всей строки (дата и время)
      2) в отдельных едитах - только в них.
    ! Логика выставления времени перенесена на уровень функции-обработчика
      диалога
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
#include "ctrlobj.hpp"


#define DM_SETATTR      (DM_USER+1)

const char FmtMask1[]="99%c99%c99";
const char FmtMask2[]="99%c99%c9999";
const char FmtMask3[]="9999%c99%c99";

struct SetAttrDlgParam{
  DWORD FileSystemFlags;
  int ModeDialog;
  int DateFormat;
  char *SelName;
  int OriginalCBAttr[16]; // значения CheckBox`ов на момент старта диалога
  int OriginalCBAttr2[16]; //
  DWORD OriginalCBFlag[16];
  int OState11, OState8, OState9;
};

static int ReadFileTime(int Type,char *Name,DWORD FileAttr,FILETIME *FileTime,
                       char *OSrcDate,char *OSrcTime);
static void PR_ShellSetFileAttributesMsg(void);
void ShellSetFileAttributesMsg(char *Name);

// В Dst получить числа для даты времени
static void GetFileDateAndTime(const char *Src,unsigned *Dst,int Separator)
{
  char Temp[32], Digit[16],*PtrDigit;
  int I;

  strncpy(Temp,Src,sizeof(Temp)-1);
  Dst[0]=Dst[1]=Dst[2]=(unsigned)-1;
  I=0;
  const char *Ptr=Temp;
  while((Ptr=GetCommaWord(Ptr,Digit,Separator)) != NULL)
  {
    PtrDigit=Digit;
    while (*PtrDigit && !isdigit(*PtrDigit))
      PtrDigit++;
    if(*PtrDigit)
      Dst[I]=atoi(PtrDigit);
    ++I;
  }
}

// обработчик диалога - пока это отлов нажатий нужных кнопок.
long WINAPI SetAttrDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  int FocusPos,I;
  int State8, State9, State11;
  struct SetAttrDlgParam *DlgParam;

  switch(Msg)
  {
    case DN_BTNCLICK:
      if(Param1 >= 4 && Param1 <= 9 || Param1 == 11)
      {
        DlgParam=(struct SetAttrDlgParam *)Dialog::SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
        DlgParam->OriginalCBAttr[Param1-4] = Param2;
        DlgParam->OriginalCBAttr2[Param1-4] = 0;

        FocusPos=Dialog::SendDlgMessage(hDlg,DM_GETFOCUS,0,0);
        State8=Dialog::SendDlgMessage(hDlg,DM_GETCHECK,8,0);
        State9=Dialog::SendDlgMessage(hDlg,DM_GETCHECK,9,0);
        State11=Dialog::SendDlgMessage(hDlg,DM_GETCHECK,11,0);

        if(!DlgParam->ModeDialog) // =0 - одиночный
        {
          if(((DlgParam->FileSystemFlags & (FS_FILE_COMPRESSION|FS_FILE_ENCRYPTION))==
               (FS_FILE_COMPRESSION|FS_FILE_ENCRYPTION)) &&
             (FocusPos == 8 || FocusPos == 9))
          {
              if(FocusPos == 8 && State8 && State9)
                Dialog::SendDlgMessage(hDlg,DM_SETCHECK,9,BSTATE_UNCHECKED);
              if(FocusPos == 9 && State9 && State8)
                Dialog::SendDlgMessage(hDlg,DM_SETCHECK,8,BSTATE_UNCHECKED);
          }
        }
        else // =1|2 Multi
        {
          // отработаем взаимоисключения
          if(((DlgParam->FileSystemFlags & (FS_FILE_COMPRESSION|FS_FILE_ENCRYPTION))==
               (FS_FILE_COMPRESSION|FS_FILE_ENCRYPTION)) &&
             (FocusPos == 8 || FocusPos == 9))
          {
            if(FocusPos == 8 && DlgParam->OState8 != State8) // Состояние изменилось?
            {
              if(State8 == BSTATE_CHECKED && State9)
                Dialog::SendDlgMessage(hDlg,DM_SETCHECK,9,BSTATE_UNCHECKED);
              else if(State8 == BSTATE_3STATE)
                Dialog::SendDlgMessage(hDlg,DM_SETCHECK,9,BSTATE_3STATE);
            }
            else if(FocusPos == 9 && DlgParam->OState9 != State9) // Состояние изменилось?
            {
              if(State9 == BSTATE_CHECKED && State8)
                Dialog::SendDlgMessage(hDlg,DM_SETCHECK,8,BSTATE_UNCHECKED);
              else if(State9 == 2)
                Dialog::SendDlgMessage(hDlg,DM_SETCHECK,8,BSTATE_3STATE);
            }

            // еще одна проверка
            if(FocusPos == 8 && State8 && State9)
              Dialog::SendDlgMessage(hDlg,DM_SETCHECK,9,BSTATE_UNCHECKED);
            if(FocusPos == 9 && State9 && State8)
              Dialog::SendDlgMessage(hDlg,DM_SETCHECK,8,BSTATE_UNCHECKED);

            DlgParam->OState9=State9;
            DlgParam->OState8=State8;
          }

          // если снимаем атрибуты для SubFolders
          // этот кусок всегда работает если есть хотя бы одна папка
          // иначе 11-й недоступен и всегда снят.
          if(FocusPos == 11)
          {
            if(DlgParam->ModeDialog==1) // каталог однозначно!
            {
              if(DlgParam->OState11 != State11) // Состояние изменилось?
              {
                // убираем 3-State
                for(I=4; I <= 9; ++I)
                {
                  if(!State11) // сняли?
                  {
                    Dialog::SendDlgMessage(hDlg,DM_SET3STATE,I,FALSE);
                    Dialog::SendDlgMessage(hDlg,DM_SETCHECK,I,DlgParam->OriginalCBAttr[I-4]);
                  }
                  else                      // установили?
                  {
                    Dialog::SendDlgMessage(hDlg,DM_SET3STATE,I,TRUE);
                    if(DlgParam->OriginalCBAttr2[I-4] == -1)
                      Dialog::SendDlgMessage(hDlg,DM_SETCHECK,I,BSTATE_3STATE);
                  }
                }
                if(!State11 && Opt.SetAttrFolderRules)
                {
                  HANDLE FindHandle;
                  WIN32_FIND_DATA FindData;
                  if ((FindHandle=FindFirstFile(DlgParam->SelName,&FindData))!=INVALID_HANDLE_VALUE)
                  {
                    FindClose(FindHandle);
                    Dialog::SendDlgMessage(hDlg,DM_SETATTR,15,(DWORD)&FindData.ftLastWriteTime);
                    Dialog::SendDlgMessage(hDlg,DM_SETATTR,18,(DWORD)&FindData.ftCreationTime);
                    Dialog::SendDlgMessage(hDlg,DM_SETATTR,21,(DWORD)&FindData.ftLastAccessTime);
                  }
                }
              }
            }
            else  // много объектов
            {
              if(DlgParam->OState11 != State11) // Состояние изменилось?
              {
                for(I=4; I <= 9; ++I)
                {
                  if(!State11) // сняли?
                  {
                    Dialog::SendDlgMessage(hDlg,DM_SET3STATE,I,
                              ((DlgParam->OriginalCBFlag[I-4]&DIF_3STATE)?TRUE:FALSE));
                    Dialog::SendDlgMessage(hDlg,DM_SETCHECK,I,DlgParam->OriginalCBAttr[I-4]);
                  }
                  else                      // установили?
                  {
                    if(DlgParam->OriginalCBAttr2[I-4] == -1)
                    {
                      Dialog::SendDlgMessage(hDlg,DM_SET3STATE,I,TRUE);
                      Dialog::SendDlgMessage(hDlg,DM_SETCHECK,I,BSTATE_3STATE);
                    }
                  }
                }
              }
            }
            DlgParam->OState11=State11;
          }
        }
        return TRUE;
      }
      // Set Original? / Set All? / Clear All?
      else if(Param1 == 24)
      {
        HANDLE FindHandle;
        WIN32_FIND_DATA FindData;
        DlgParam=(struct SetAttrDlgParam *)Dialog::SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
        if ((FindHandle=FindFirstFile(DlgParam->SelName,&FindData))!=INVALID_HANDLE_VALUE)
        {
          FindClose(FindHandle);
          Dialog::SendDlgMessage(hDlg,DM_SETATTR,15,(DWORD)&FindData.ftLastWriteTime);
          Dialog::SendDlgMessage(hDlg,DM_SETATTR,18,(DWORD)&FindData.ftCreationTime);
          Dialog::SendDlgMessage(hDlg,DM_SETATTR,21,(DWORD)&FindData.ftLastAccessTime);
        }
        Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,16,0);
        return TRUE;
      }
      else if(Param1 == 25 || Param1 == 26)
      {
        Dialog::SendDlgMessage(hDlg,DM_SETATTR,15,Param1 == 25?-1:0);
        Dialog::SendDlgMessage(hDlg,DM_SETATTR,18,Param1 == 25?-1:0);
        Dialog::SendDlgMessage(hDlg,DM_SETATTR,21,Param1 == 25?-1:0);
        Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,16,0);
        return TRUE;
      }
      break;

    case DN_MOUSECLICK:
     {
       //_SVS(SysLog("Msg=DN_MOUSECLICK Param1=%d Param2=%d",Param1,Param2));
       if(Param1 >= 15 && Param1 <= 23)
       {
         if(((MOUSE_EVENT_RECORD*)Param2)->dwEventFlags == DOUBLE_CLICK)
         {
           // Дадим Менеджеру диалогов "попотеть"
           Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
           Dialog::SendDlgMessage(hDlg,DM_SETATTR,Param1,-1);
         }
         if(Param1 == 15 || Param1 == 18 || Param1 == 21)
           Param1++;
         Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,Param1,0);
         return TRUE;
       }
     }
     break;

    case DM_SETATTR:
      {
        FILETIME ft;
        char Date[16],Time[16];
        int Set1, Set2;
        if(Param2) // Set?
        {
          if(Param2==-1)
            GetSystemTimeAsFileTime(&ft);
          else
            ft=*(FILETIME *)Param2;
          ConvertDate(&ft,Date,Time,8,FALSE,FALSE,TRUE,TRUE);
        }
        else if(!Param2) // Clear
        {
           Date[0]=Time[0]=0;
        }

        // Глянем на место, где был клик
             if(Param1 == 15) { Set1=16; Set2=17; }
        else if(Param1 == 18) { Set1=19; Set2=20; }
        else if(Param1 == 21) { Set1=22; Set2=23; }
        else if(Param1 == 16 || Param1 == 19 || Param1 == 22) { Set1=Param1; Set2=-1; }
        else { Set1=-1; Set2=Param1; }

        if(Set1 != -1)
          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,Set1,(long)Date);
        if(Set2 != -1)
          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,Set2,(long)Time);
        return TRUE;
      }
  }
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

static void PR_ShellSetFileAttributesMsg(void)
{
  ShellSetFileAttributesMsg((char *)PreRedrawParam.Param1);
}

void ShellSetFileAttributesMsg(char *Name)
{
  static int Width=30;
  int WidthTemp;
  char OutFileName[NM];

  if(Name && *Name)
    WidthTemp=Max((int)strlen(Name),(int)30);
  else
    Width=WidthTemp=30;

  if(WidthTemp > WidthNameForMessage)
    WidthTemp=WidthNameForMessage; // ширина месага - 38%
  if(WidthTemp >= sizeof(OutFileName)-4)
    WidthTemp=sizeof(OutFileName)-5;

  if(Width < WidthTemp)
    Width=WidthTemp;

  if(Name && *Name)
  {
    strncpy(OutFileName,Name,sizeof(OutFileName)-1);
    TruncPathStr(OutFileName,Width);
    CenterStr(OutFileName,OutFileName,Width+4);
  }
  else
  {
    *OutFileName=0;
    CenterStr(OutFileName,OutFileName,Width+4); // подготавливаем нужную ширину (вид!)
  }
  Message(0,0,MSG(MSetAttrTitle),MSG(MSetAttrSetting),OutFileName);
  PreRedrawParam.Param1=Name;
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
19   | [ Original ]  [ Current ] [ Blank ] |   19
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
  /* 24 */DI_BUTTON, 5,18,0,0,0,0,DIF_BTNNOCLOSE,0,(char *)MSetAttrOriginal,
  /* 25 */DI_BUTTON,19,18,0,0,0,0,DIF_BTNNOCLOSE,0,(char *)MSetAttrCurrent,
  /* 26 */DI_BUTTON,31,18,0,0,0,0,DIF_BTNNOCLOSE,0,(char *)MSetAttrBlank,
  /* 27 */DI_TEXT,3,19,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 28 */DI_BUTTON,0,20,0,0,0,0,DIF_CENTERGROUP,1,(char *)MSetAttrSet,
  /* 29 */DI_BUTTON,0,20,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel,
  /* 30 */DI_TEXT,-1,4,0,0,0,0,DIF_SHOWAMPERSAND,0,"",
  };
  MakeDialogItems(AttrDlgData,AttrDlg);
  int DlgCountItems=sizeof(AttrDlgData)/sizeof(AttrDlgData[0])-1;

  int SelCount, I, J;
  struct SetAttrDlgParam DlgParam;

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

  char FullName[NM];

  FullName[strlen(FullName)+1]=0;
  memset(&DlgParam,0,sizeof(DlgParam));

  DlgParam.FileSystemFlags=0;
  if (GetVolumeInformation(NULL,NULL,0,NULL,NULL,&DlgParam.FileSystemFlags,NULL,0))
  {
    if (!(DlgParam.FileSystemFlags & FS_FILE_COMPRESSION))
      AttrDlg[8].Flags|=DIF_DISABLE;

    if (!IsCryptFileASupport || !(DlgParam.FileSystemFlags & FS_FILE_ENCRYPTION))
      AttrDlg[9].Flags|=DIF_DISABLE;
  }

  {
    char SelName[NM];
    int FileAttr;
    FILETIME LastWriteTime,CreationTime,LastAccessTime;
    int SetWriteTime,SetCreationTime,SetLastAccessTime;

    //SaveScreen SaveScr;

    SrcPanel->GetSelName(NULL,FileAttr);
    SrcPanel->GetSelName(SelName,FileAttr);
    if (SelCount==0 || SelCount==1 && strcmp(SelName,"..")==0)
      return 0;

//    int NewAttr;
    int FolderPresent=FALSE, JunctionPresent=FALSE;

    int DateSeparator=GetDateSeparator();
    int TimeSeparator=GetTimeSeparator();
    static char DMask[20],TMask[20];

    sprintf(TMask,FmtMask1,TimeSeparator,TimeSeparator);
    switch(DlgParam.DateFormat=GetDateFormat())
    {
      case 0:
        sprintf(AttrDlg[14].Data,MSG(MSetAttrTimeTitle1),DateSeparator,DateSeparator,TimeSeparator,TimeSeparator);
        sprintf(DMask,FmtMask2,DateSeparator,DateSeparator);
        break;
      case 1:
        sprintf(AttrDlg[14].Data,MSG(MSetAttrTimeTitle2),DateSeparator,DateSeparator,TimeSeparator,TimeSeparator);
        sprintf(DMask,FmtMask2,DateSeparator,DateSeparator);
        break;
      default:
        sprintf(AttrDlg[14].Data,MSG(MSetAttrTimeTitle3),DateSeparator,DateSeparator,TimeSeparator,TimeSeparator);
        sprintf(DMask,FmtMask3,DateSeparator,DateSeparator);
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
        if(SelName[strlen(SelName)-1] != '\\')
        {
          AddEndSlash(SelName);
          FileAttr=GetFileAttributes(SelName);
          SelName[strlen(SelName)-1]=0;
        }
        //_SVS(SysLog("SelName=%s  FileAttr=0x%08X",SelName,FileAttr));
        AttrDlg[11].Flags&=~DIF_DISABLE;
        AttrDlg[11].Selected=Opt.SetAttrFolderRules == 1?0:1;
        if(Opt.SetAttrFolderRules)
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
          AttrDlg[4].Selected=(FileAttr & FA_RDONLY)!=0;
          AttrDlg[5].Selected=(FileAttr & FA_ARCH)!=0;
          AttrDlg[6].Selected=(FileAttr & FA_HIDDEN)!=0;
          AttrDlg[7].Selected=(FileAttr & FA_SYSTEM)!=0;
          AttrDlg[8].Selected=(FileAttr & FILE_ATTRIBUTE_COMPRESSED)!=0;
          AttrDlg[9].Selected=(FileAttr & FILE_ATTRIBUTE_ENCRYPTED)!=0;

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

          int ID_Msg, Width;
          if(!strncmp(JuncName+4,"Volume{",7))
          {
            char JuncRoot[NM];
            JuncRoot[0]=JuncRoot[1]=0;
            GetPathRootOne(JuncName+4,JuncRoot);
            if(JuncRoot[1] == ':')
              strcpy(JuncName+4,JuncRoot);
            ID_Msg=MSetAttrVolMount;
            Width=14;
          }
          else
          {
            ID_Msg=MSetAttrJunction;
            Width=28;
          }

          sprintf(AttrDlg[30].Data,MSG(ID_Msg),
                (LenJunction?
                   TruncPathStr(JuncName+4,Width):
                   MSG(MSetAttrUnknownJunction)));

          /* $ 11.09.2001 SVS
             Уточнение по поводу слинкованной файловой системы отличной от
             NTFS.
          */
          DlgParam.FileSystemFlags=0;
          GetPathRoot(SelName,JuncName);
          if (GetVolumeInformation(JuncName,NULL,0,NULL,NULL,&DlgParam.FileSystemFlags,NULL,0))
          {
            if (!(DlgParam.FileSystemFlags & FS_FILE_COMPRESSION))
              AttrDlg[8].Flags|=DIF_DISABLE;

            if (!IsCryptFileASupport || !(DlgParam.FileSystemFlags & FS_FILE_ENCRYPTION))
              AttrDlg[9].Flags|=DIF_DISABLE;
          }
          /* SVS $ */
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
    }
    else
    {
      AttrDlg[4].Selected=AttrDlg[5].Selected=AttrDlg[6].Selected=
      AttrDlg[7].Selected=AttrDlg[8].Selected=AttrDlg[9].Selected=2;
      AttrDlg[16].Data[0]=AttrDlg[17].Data[0]=AttrDlg[19].Data[0]=
      AttrDlg[20].Data[0]=AttrDlg[22].Data[0]=AttrDlg[23].Data[0]='\0';

      AttrDlg[24].Flags|=DIF_HIDDEN;

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

    // поведение для каталогов как у 1.65?
    if(FolderPresent && !Opt.SetAttrFolderRules)
    {
      AttrDlg[11].Selected=1;
      AttrDlg[16].Data[0]=AttrDlg[17].Data[0]=AttrDlg[19].Data[0]=
      AttrDlg[20].Data[0]=AttrDlg[22].Data[0]=AttrDlg[23].Data[0]='\0';
      for(I=4; I <= 9; ++I)
      {
        AttrDlg[I].Selected=2;
        AttrDlg[I].Flags|=DIF_3STATE;
      }
    }

    // запомним состояние переключателей.
    for(I=4; I <= 9; ++I)
    {
      DlgParam.OriginalCBAttr[I-4]=AttrDlg[I].Selected;
      DlgParam.OriginalCBAttr2[I-4]=-1;
      DlgParam.OriginalCBFlag[I-4]=AttrDlg[I].Flags;
    }

    DlgParam.ModeDialog=((SelCount==1 && (FileAttr & FA_DIREC)==0)?0:(SelCount==1?1:2));
    DlgParam.SelName=SelName;
    DlgParam.OState11=AttrDlg[11].Selected;
    DlgParam.OState8=AttrDlg[8].Selected;
    DlgParam.OState9=AttrDlg[9].Selected;

    // <Dialog>
    {
      Dialog Dlg(AttrDlg,DlgCountItems,SetAttrDlgProc,(DWORD)&DlgParam);
      Dlg.SetHelp("FileAttrDlg");                 //  ^ - это одиночный диалог!
      Dlg.SetPosition(-1,-1,45,JunctionPresent?24:23);
      Dlg.Process();
      if (Dlg.GetExitCode()!=28)
        return 0;
    }
    // </Dialog>

    SetPreRedrawFunc(PR_ShellSetFileAttributesMsg);
    ShellSetFileAttributesMsg(SelCount==1?SelName:NULL);

    if (SelCount==1 && (FileAttr & FA_DIREC)==0)
    {
      int NewAttr;
      NewAttr=FileAttr & FA_DIREC;
      if (AttrDlg[4].Selected)        NewAttr|=FA_RDONLY;
      if (AttrDlg[5].Selected)        NewAttr|=FA_ARCH;
      if (AttrDlg[6].Selected)        NewAttr|=FA_HIDDEN;
      if (AttrDlg[7].Selected)        NewAttr|=FA_SYSTEM;
      if (AttrDlg[8].Selected)        NewAttr|=FILE_ATTRIBUTE_COMPRESSED;
      if (AttrDlg[9].Selected)        NewAttr|=FILE_ATTRIBUTE_ENCRYPTED;

      SetWriteTime=ReadFileTime(0,SelName,FileAttr,&LastWriteTime,AttrDlg[16].Data,AttrDlg[17].Data);
      SetCreationTime=ReadFileTime(1,SelName,FileAttr,&CreationTime,AttrDlg[19].Data,AttrDlg[20].Data);
      SetLastAccessTime=ReadFileTime(2,SelName,FileAttr,&LastAccessTime,AttrDlg[22].Data,AttrDlg[23].Data);
//_SVS(SysLog("\n\tSetWriteTime=%d\n\tSetCreationTime=%d\n\tSetLastAccessTime=%d",SetWriteTime,SetCreationTime,SetLastAccessTime));
      if(SetWriteTime || SetCreationTime || SetLastAccessTime)
        SetWriteTime=ESetFileTime(SelName,
                     (SetWriteTime ? &LastWriteTime:NULL),
                     (SetCreationTime ? &CreationTime:NULL),
                     (SetLastAccessTime ? &LastAccessTime:NULL),
                     FileAttr);
      else
        SetWriteTime=TRUE;

//      if(NewAttr != (FileAttr & (~FA_DIREC))) // нужно ли что-нить менять???
      if(SetWriteTime) // если время удалось выставить...
      {
        if((NewAttr&FILE_ATTRIBUTE_COMPRESSED) && !(FileAttr&FILE_ATTRIBUTE_COMPRESSED))
          ESetFileCompression(SelName,1,FileAttr);
        else if(!(NewAttr&FILE_ATTRIBUTE_COMPRESSED) && (FileAttr&FILE_ATTRIBUTE_COMPRESSED))
          ESetFileCompression(SelName,0,FileAttr);

        if((NewAttr&FILE_ATTRIBUTE_ENCRYPTED) && !(FileAttr&FILE_ATTRIBUTE_ENCRYPTED))
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
      char OldConsoleTitle[NM];//,TmpFileName[72];
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

      if (AttrDlg[9].Selected == 1)
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
//_SVS(SysLog("SelName='%s'\n\tFileAttr =0x%08X\n\tSetAttr  =0x%08X\n\tClearAttr=0x%08X\n\tResult   =0x%08X",
//    SelName,FileAttr,SetAttr,ClearAttr,((FileAttr|SetAttr)&(~ClearAttr))));
        ShellSetFileAttributesMsg(SelName);

        if (CheckForEsc())
          break;

        SetWriteTime=ReadFileTime(0,SelName,FileAttr,&LastWriteTime,AttrDlg[16].Data,AttrDlg[17].Data);
        SetCreationTime=ReadFileTime(1,SelName,FileAttr,&CreationTime,AttrDlg[19].Data,AttrDlg[20].Data);
        SetLastAccessTime=ReadFileTime(2,SelName,FileAttr,&LastAccessTime,AttrDlg[22].Data,AttrDlg[23].Data);
        if(SetWriteTime || SetCreationTime || SetLastAccessTime)
          if (!ESetFileTime(SelName,
                 (SetWriteTime ? &LastWriteTime:NULL),
                 (SetCreationTime ? &CreationTime:NULL),
                 (SetLastAccessTime ? &LastAccessTime:NULL),
                 FileAttr))
            break;

        if(((FileAttr|SetAttr)&(~ClearAttr)) != FileAttr)
        {
          if (AttrDlg[8].Selected != 2)
          {
            if (!ESetFileCompression(SelName,AttrDlg[8].Selected,FileAttr))
              break; // неудача сжать :-(
          }
          if (AttrDlg[9].Selected != 2) // +E -C
          {
            if(AttrDlg[8].Selected != 1)
              if (!ESetFileEncryption(SelName,AttrDlg[9].Selected,FileAttr))
                break; // неудача зашифровать :-(
          }

          if (!ESetFileAttributes(SelName,((FileAttr|SetAttr)&(~ClearAttr))))
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
            ShellSetFileAttributesMsg(FullName);
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
              if (AttrDlg[8].Selected != 2)
              {
                if (!ESetFileCompression(FullName,1,FindData.dwFileAttributes))
                {
                  Cancel=1;
                  break; // неудача сжать :-(
                }
              }
              else if (AttrDlg[9].Selected != 2) // +E -C
              {
                if(AttrDlg[8].Selected != 1)
                  if (!ESetFileEncryption(FullName,1,FindData.dwFileAttributes))
                  {
                    Cancel=1;
                    break; // неудача зашифровать :-(
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
    SetPreRedrawFunc(NULL);
  }

  SrcPanel->SaveSelection();
  SrcPanel->Update(UPDATE_KEEP_SELECTION);
  SrcPanel->ClearSelection();
  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(SrcPanel);
  AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
  CtrlObject->Cp()->Redraw();
  return 1;
}

static int ReadFileTime(int Type,char *Name,DWORD FileAttr,FILETIME *FileTime,
                       char *OSrcDate,char *OSrcTime)
{
  FILETIME ft, oft;
  SYSTEMTIME st, ost;
  unsigned DateN[3],TimeN[3];
  int DigitCount,I;
  int /*SetTime,*/GetTime;
  FILETIME *OriginalFileTime, OFTModify, OFTCreate, OFTLast;
  char SrcDate[32], SrcTime[32], Digit[16],*PtrDigit;
  const char *Ptr;


  /*$ 17.07.2001 SKV
    от греха подальше, занулим.
  */
  ZeroMemory(&st,sizeof(st));
  /* SKV$*/

  // ****** ОБРАБОТКА ДАТЫ ******** //
  GetFileDateAndTime(OSrcDate,DateN,GetDateSeparator());
  // ****** ОБРАБОТКА ВРЕМЕНИ ******** //
  GetFileDateAndTime(OSrcTime,TimeN,GetTimeSeparator());

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
    GetTime=GetFileTime(hFile,&OFTCreate,&OFTLast,&OFTModify);
    CloseHandle(hFile);

    if(!GetTime)
      return(FALSE);

    switch(Type)
    {
      case 0: // Modif
        OriginalFileTime=&OFTModify;
        break;
      case 1: // Creat
        OriginalFileTime=&OFTCreate;
        break;
      case 2: // Last
        OriginalFileTime=&OFTLast;
        break;
    }

    // конвертнем в локальное время.
    FileTimeToLocalFileTime(OriginalFileTime,&oft);
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
    return (!CompareFileTime(FileTime,OriginalFileTime))?FALSE:TRUE;
  return TRUE;
}
