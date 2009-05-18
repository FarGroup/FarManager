/*
copy.cpp

Копирование файлов

*/

#include "headers.hpp"
#pragma hdrstop

#include "copy.hpp"
#include "global.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "colors.hpp"
#include "fn.hpp"
#include "flink.hpp"
#include "dialog.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "filelist.hpp"
#include "foldtree.hpp"
#include "treelist.hpp"
#include "chgprior.hpp"
#include "scantree.hpp"
#include "savescr.hpp"
#include "manager.hpp"
#include "constitle.hpp"
#include "lockscrn.hpp"
#include "filefilter.hpp"
#include "fileview.hpp"
#include "TPreRedrawFunc.hpp"
#include "TaskBar.hpp"

/* Общее время ожидания пользователя */
extern long WaitUserTime;
/* Длф того, что бы время при одижании пользователя тикало, а remaining/speed нет */
static long OldCalcTime;

/* Интервал для прорисовки прогресс-бара. */
#define COPY_TIMEOUT 200

// Высота и ширина диалога
#define DLG_HEIGHT 16
#define DLG_WIDTH 76

#define SDDATA_SIZE   64000

enum {COPY_BUFFER_SIZE  = 0x10000};

/* $ 30.01.2001 VVM
   + Константы для правил показа
   + Рабочие перменные */
enum {
  COPY_RULE_NUL    = 0x0001,
  COPY_RULE_FILES  = 0x0002,
};

static int TotalFilesToProcess;

static int ShowCopyTime;
static clock_t CopyStartTime;
static clock_t LastShowTime;
/* VVM $ */
static int OrigScrX,OrigScrY;

static DWORD WINAPI CopyProgressRoutine(LARGE_INTEGER TotalFileSize,
       LARGE_INTEGER TotalBytesTransferred,LARGE_INTEGER StreamSize,
       LARGE_INTEGER StreamBytesTransferred,DWORD dwStreamNumber,
       DWORD dwCallbackReason,HANDLE hSourceFile,HANDLE hDestinationFile,
       LPVOID lpData);

static int BarX,BarY,BarLength;

static unsigned __int64 TotalCopySize, TotalCopiedSize; // Общий индикатор копирования
static unsigned __int64 CurCopiedSize;                  // Текущий индикатор копирования
static unsigned __int64 TotalSkippedSize;               // Общий размер пропущенных файлов
static unsigned __int64 TotalCopiedSizeEx;
static int   CountTarget;                    // всего целей.
static int CopySecurityCopy=-1;
static int CopySecurityMove=-1;
static bool ShowTotalCopySize;
static int StaticMove;
static char TotalCopySizeText[32];
static ConsoleTitle *StaticCopyTitle=NULL;
static BOOL NT5, NT;
static bool CopySparse;

/* $ 15.04.2005 KM
   Указатель на объект фильтра операций
*/
static FileFilter *Filter;
static int UseFilter=FALSE;
/* KM $*/

static BOOL ZoomedState;
static BOOL IconicState;

struct CopyDlgParam {
  ShellCopy *thisClass;
  int AltF10;
  int FileAttr;
  int SelCount;
  int FolderPresent;
  int FilesPresent;
  int OnlyNewerFiles;
  int CopySecurity;
  char FSysNTFS;
  char PluginFormat[32]; // я думаю этого достаточно.
  DWORD FileSystemFlagsSrc;
  int IsDTSrcFixed;
  int IsDTDstFixed;
  int AskRO;
};

enum enumShellCopy {
  ID_SC_TITLE=0,
  ID_SC_TARGETTITLE,
  ID_SC_TARGETEDIT,
  ID_SC_SEPARATOR1,
  ID_SC_ACTITLE,
  ID_SC_ACLEAVE,
  ID_SC_ACCOPY,
  ID_SC_ACINHERIT,
  ID_SC_SEPARATOR2,
  ID_SC_COMBOTEXT,
  ID_SC_COMBO,
  ID_SC_COPYSYMLINK,
  ID_SC_MULTITARGET,
  ID_SC_SEPARATOR3,
  ID_SC_USEFILTER,
  ID_SC_SEPARATOR4,
  ID_SC_BTNCOPY,
  ID_SC_BTNTREE,
  ID_SC_BTNFILTER,
  ID_SC_BTNCANCEL,
  ID_SC_SOURCEFILENAME,
};

enum CopyMode
{
  CM_ASK,
  CM_OVERWRITE,
  CM_SKIP,
  CM_APPEND,
  CM_ONLYNEWER,
  CM_ASKRO,
};

ShellCopy::ShellCopy(Panel *SrcPanel,        // исходная панель (активная)
                     int Move,               // =1 - операция Move
                     int Link,               // =1 - Sym/Hard Link
                     int CurrentOnly,        // =1 - только текущий файл, под курсором
                     int Ask,                // =1 - выводить диалог?
                     int &ToPlugin,          // =?
                     char *PluginDestPath)   // =?
{
  _LOGCOPYR(CleverSysLog Clev("ShellCopy::ShellCopy()"));
  /* $ 15.08.2002 IS
     Запретим дубли в списке целей
  */
  DestList.SetParameters(0,0,ULF_UNIQUE);
  /* IS $ */
  struct CopyDlgParam CDP;

  char CopyStr[100];
  char SelNameShort[NM],SelName[NM];

  int DestPlugin;
  int AddSlash=FALSE;

  Filter=NULL;
  sddata=NULL;

  // ***********************************************************************
  // *** Предварительные проверки
  // ***********************************************************************
  // Сразу выциганим версию OS

  NT=WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT;
  NT5=NT && WinVer.dwMajorVersion >= 5;

  /* $ 06.06.2001 tran
     инициализация NULL должна быть ДО ЛЮБЫХ Return из конструктора... */
  CopyBuffer=NULL;

  if(Link && !NT) // Создание линков только под NT
  {
    Message(MSG_DOWN|MSG_WARNING,1,MSG(MWarning),
              MSG(MCopyNotSupportLink1),
              MSG(MCopyNotSupportLink2),
              MSG(MOk));
    _LOGCOPYR(SysLog("return -> %d NotSupportLink!!!",__LINE__));
    return;
  }

  memset(&CDP,0,sizeof(CDP));
  CDP.IsDTSrcFixed=-1;
  CDP.IsDTDstFixed=-1;

  if ((CDP.SelCount=SrcPanel->GetSelCount())==0)
  {
    _LOGCOPYR(SysLog("return -> %d SelCount == 0",__LINE__));
    return;
  }

  *SelName=*RenamedName=*CopiedName=0;

  if (CDP.SelCount==1)
  {
    SrcPanel->GetSelName(NULL,CDP.FileAttr);
    SrcPanel->GetSelName(SelName,CDP.FileAttr);
    if (TestParentFolderName(SelName))
    {
      _LOGCOPYR(SysLog("return -> %d TestParentFolderName('%s') != 0",__LINE__,SelName));
      return;
    }
  }

  RPT=RP_EXACTCOPY;

  ZoomedState=IsZoomed(hFarWnd);
  IconicState=IsIconic(hFarWnd);

  // Создадим объект фильтра
  Filter=new FileFilter(SrcPanel,FFT_COPY);

  sddata=new char[SDDATA_SIZE]; // Security 16000?

  /* $ 26.05.2001 OT Запретить перерисовку панелей во время копирования */
  _tran(SysLog("call (*FrameManager)[0]->LockRefresh()"));
  (*FrameManager)[0]->Lock();
  /* OT $ */

  // Размер буфера берется из реестра
  GetRegKey("System", "CopyBufferSize", CopyBufferSize, 0);
  if (CopyBufferSize == 0)
    CopyBufferSize = COPY_BUFFER_SIZE; //Макс. размер 64к
  if (CopyBufferSize < COPY_BUFFER_SIZE)
    CopyBufferSize = COPY_BUFFER_SIZE;

  CDP.thisClass=this;
  CDP.AltF10=0;
  CDP.FolderPresent=0;
  CDP.FilesPresent=0;

  ShellCopy::Flags=0;
  ShellCopy::Flags|=Move?FCOPY_MOVE:0;
  ShellCopy::Flags|=Link?FCOPY_LINK:0;
  ShellCopy::Flags|=CurrentOnly?FCOPY_CURRENTONLY:0;

  ShowTotalCopySize=Opt.CMOpt.CopyShowTotal != 0;

  *TotalCopySizeText=0;
  SelectedFolderNameLength=0;
  DestPlugin=ToPlugin;
  ToPlugin=FALSE;
  *SrcDriveRoot=0;
  *SrcFSName=0;
  SrcDriveType=0;
  StaticMove=Move;

  ShellCopy::SrcPanel=SrcPanel;
  DestPanel=CtrlObject->Cp()->GetAnotherPanel(SrcPanel);
  DestPanelMode=DestPlugin ? DestPanel->GetMode():NORMAL_PANEL;
  SrcPanelMode=SrcPanel->GetMode();

  int SizeBuffer=2048;
  if(DestPanelMode == PLUGIN_PANEL)
  {
    struct OpenPluginInfo Info;
    DestPanel->GetOpenPluginInfo(&Info);
    int LenCurDir=(int)strlen(NullToEmpty(Info.CurDir));
    if(SizeBuffer < LenCurDir)
      SizeBuffer=LenCurDir;
  }
  SizeBuffer+=NM; // добавка :-)

  /* $ 03.08.2001 IS
       CopyDlgValue - в этой переменной храним заветную строчку из диалога,
       именно эту переменную всячески измененяем, а CopyDlg[2].Data не трогаем.
  */
  char *CopyDlgValue=(char *)alloca(SizeBuffer);
  char *Dlg2Value=(char *)alloca(SizeBuffer);
  char *DestDir=(char *)alloca(SizeBuffer);
  char *InitDestDir=(char *)alloca(SizeBuffer);
  char *SrcDir=(char *)alloca(SizeBuffer);

  *Dlg2Value=0;

  // ***********************************************************************
  // *** Prepare Dialog Controls
  // ***********************************************************************
  const char *HistoryName="Copy";
  /* $ 03.08.2001 IS добавим новую опцию: мультикопирование */
  static struct DialogData CopyDlgData[]={
  /* 00 */  DI_DOUBLEBOX,3,1,DLG_WIDTH-4,DLG_HEIGHT-2,0,0,0,0,(char *)MCopyDlgTitle,
  /* 01 */  DI_TEXT,5,2,0,2,0,0,0,0,(char *)MCMLTargetTO,
  /* 02 */  DI_EDIT,5,3,70,3,1,(DWORD_PTR)HistoryName,DIF_HISTORY|DIF_VAREDIT|DIF_EDITEXPAND|DIF_USELASTHISTORY/*|DIF_EDITPATH*/,0,"",
  /* 03 */  DI_TEXT,3,4,0,4,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 04 */  DI_TEXT,5,5,0,5,0,0,0,0,(char *)MCopySecurity,
  /* 05 */  DI_RADIOBUTTON,5,5,0,5,0,0,DIF_GROUP,0,(char *)MCopySecurityLeave,
  /* 06 */  DI_RADIOBUTTON,5,5,0,5,0,0,0,0,(char *)MCopySecurityCopy,
  /* 07 */  DI_RADIOBUTTON,5,5,0,5,0,0,0,0,(char *)MCopySecurityInherit,
  /* 08 */  DI_TEXT,3,6,0,6,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 09 */  DI_TEXT,        5, 7, 0, 7,0,0,0,0,(char *)MCopyIfFileExist,
  /* 10 */  DI_COMBOBOX,   29, 7,70, 7,0,0,DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND|DIF_LISTWRAPMODE,0,"",
  /* 11 */  DI_CHECKBOX,5,8,0,8,0,0,0,0,(char *)MCopySymLinkContents,
  /* 12 */  DI_CHECKBOX,5,9,0,9,0,0,0,0,(char *)MCopyMultiActions,
  /* 13 */  DI_TEXT,3,10,0,10,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 14 */  DI_CHECKBOX,5,11,0,11,0,0,0,0,(char *)MCopyUseFilter,
  /* 15 */  DI_TEXT,3,12,0,12,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 16 */  DI_BUTTON,0,13,0,13,0,0,DIF_CENTERGROUP,1,(char *)MCopyDlgCopy,
  /* 17 */  DI_BUTTON,0,13,0,13,0,0,DIF_CENTERGROUP|DIF_BTNNOCLOSE,0,(char *)MCopyDlgTree,
  /* 18 */  DI_BUTTON,0,13,0,13,0,0,DIF_CENTERGROUP|DIF_BTNNOCLOSE,0,(char *)MCopySetFilter,
  /* 19 */  DI_BUTTON,0,13,0,13,0,0,DIF_CENTERGROUP,0,(char *)MCopyDlgCancel,
  /* 20 */  DI_TEXT,5,2,0,2,0,0,DIF_SHOWAMPERSAND,0,"",
  };
  MakeDialogItems(CopyDlgData,CopyDlg);

  CopyDlg[ID_SC_MULTITARGET].Selected=Opt.CMOpt.MultiCopy;

  // Использовать фильтр. KM
  CopyDlg[ID_SC_USEFILTER].Selected=UseFilter;

  CopyDlg[ID_SC_TARGETEDIT].Ptr.PtrData=Dlg2Value;
  CopyDlg[ID_SC_TARGETEDIT].Ptr.PtrLength=SizeBuffer;

  CopyDlg[ID_SC_ACLEAVE].X1 = CopyDlg[ID_SC_ACTITLE].X1 + (int)strlen(CopyDlg[ID_SC_ACTITLE].Data) - (strchr(CopyDlg[ID_SC_ACTITLE].Data, '&')?1:0) + 1;
  CopyDlg[ID_SC_ACCOPY].X1 = CopyDlg[ID_SC_ACLEAVE].X1 + (int)strlen(CopyDlg[ID_SC_ACLEAVE].Data) - (strchr(CopyDlg[ID_SC_ACLEAVE].Data, '&')?1:0) + 5;
  CopyDlg[ID_SC_ACINHERIT].X1 = CopyDlg[ID_SC_ACCOPY].X1 + (int)strlen(CopyDlg[ID_SC_ACCOPY].Data) - (strchr(CopyDlg[ID_SC_ACCOPY].Data, '&')?1:0) + 5;

  if(Link)
  {
    strcpy(CopyDlg[ID_SC_COMBOTEXT].Data,MSG(MLinkType));
    CopyDlg[ID_SC_COPYSYMLINK].Selected=0;
    CopyDlg[ID_SC_COPYSYMLINK].Flags|=DIF_DISABLE;
    CDP.CopySecurity=1;
  }
  else if(Move)  // секция про перенос
  {
    //   2 - Default
    //   1 - Copy access rights
    //   0 - Inherit access rights
    CDP.CopySecurity=2;

    // ставить опцию "Inherit access rights"?
    // CSO_MOVE_SETINHERITSECURITY - двухбитный флаг
    if((Opt.CMOpt.CopySecurityOptions&CSO_MOVE_SETINHERITSECURITY) == CSO_MOVE_SETINHERITSECURITY)
      CDP.CopySecurity=0;
    else if (Opt.CMOpt.CopySecurityOptions&CSO_MOVE_SETCOPYSECURITY)
      CDP.CopySecurity=1;

    // хотели сессионное запоминание?
    if(CopySecurityMove != -1 && Opt.CMOpt.CopySecurityOptions&CSO_MOVE_SESSIONSECURITY)
      CDP.CopySecurity=CopySecurityMove;
    else
      CopySecurityMove=CDP.CopySecurity;
  }
  else // секция про копирование
  {
    //   2 - Default
    //   1 - Copy access rights
    //   0 - Inherit access rights
    CDP.CopySecurity=2;

    // ставить опцию "Inherit access rights"?
    // CSO_COPY_SETINHERITSECURITY - двухбитный флаг
    if((Opt.CMOpt.CopySecurityOptions&CSO_COPY_SETINHERITSECURITY) == CSO_COPY_SETINHERITSECURITY)
      CDP.CopySecurity=0;
    else if (Opt.CMOpt.CopySecurityOptions&CSO_COPY_SETCOPYSECURITY)
      CDP.CopySecurity=1;

    // хотели сессионное запоминание?
    if(CopySecurityCopy != -1 && Opt.CMOpt.CopySecurityOptions&CSO_COPY_SESSIONSECURITY)
      CDP.CopySecurity=CopySecurityCopy;
    else
      CopySecurityCopy=CDP.CopySecurity;
  }

  // вот теперь выставляем
  if(CDP.CopySecurity)
  {
    if(CDP.CopySecurity == 1)
    {
      ShellCopy::Flags|=FCOPY_COPYSECURITY;
      CopyDlg[ID_SC_ACCOPY].Selected=1;
    }
    else
    {
      ShellCopy::Flags|=FCOPY_LEAVESECURITY;
      CopyDlg[ID_SC_ACLEAVE].Selected=1;
    }
  }
  else
  {
    ShellCopy::Flags&=~(FCOPY_COPYSECURITY|FCOPY_LEAVESECURITY);
    CopyDlg[ID_SC_ACINHERIT].Selected=1;
  }

  if (CDP.SelCount==1)
  { // SelName & FileAttr уже заполнены (см. в самом начале функции)
/*
    // Если каталог и он один, то предполагаем, что хотим создать симлинк
    if(Link && NT5 && (CDP.FileAttr&FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY && DestPanelMode == NORMAL_PANEL)
    {
      CDP.OnlyNewerFiles=CopyDlg[ID_SC_ONLYNEWER].Selected=1;
      CDP.FolderPresent=TRUE;
    }
    else
*/
      CDP.OnlyNewerFiles=0;

    if (SrcPanel->GetType()==TREE_PANEL)
    {
      char NewDir[NM],*ChPtr;
      strcpy(NewDir,SelName);
      if ((ChPtr=strrchr(NewDir,'\\'))!=0)
      {
        *ChPtr=0;
        if (ChPtr==NewDir || *(ChPtr-1)==':')
          strcat(NewDir,"\\");
        FarChDir(NewDir);
      }
    }
    // Урезаем имя до размеров диалога.
    sprintf(CopyStr,
            MSG(Move?MMoveFile:(Link?MLinkFile:MCopyFile)),
            TruncPathStr(strcpy(SelNameShort,SelName),33));

    // Если копируем одиночный файл, то запрещаем использовать фильтр
    if (!(CDP.FileAttr&FILE_ATTRIBUTE_DIRECTORY))
    {
      CopyDlg[ID_SC_USEFILTER].Selected=0;
      CopyDlg[ID_SC_USEFILTER].Flags|=DIF_DISABLE;
    }
  }
  else // Объектов несколько!
  {
    int NOper=MCopyFiles;
         if (Move) NOper=MMoveFiles;
    else if (Link) NOper=MLinkFiles;

    // коррекция языка - про окончания
    char StrItems[32];
    itoa(CDP.SelCount,StrItems,10);
    int LenItems=(int)strlen(StrItems);
    int NItems=MCMLItemsA;
    if((LenItems >= 2 && StrItems[LenItems-2] == '1') ||
        StrItems[LenItems-1] >= '5' ||
        StrItems[LenItems-1] == '0')
      NItems=MCMLItemsS;
    else if(StrItems[LenItems-1] == '1')
      NItems=MCMLItems0;
    sprintf(CopyStr,MSG(NOper),CDP.SelCount,MSG(NItems));
  }
  sprintf(CopyDlg[ID_SC_SOURCEFILENAME].Data,"%.65s",CopyStr);

  // заголовки контролов
  strcpy(CopyDlg[ID_SC_TITLE].Data,MSG(Move?MMoveDlgTitle :(Link?MLinkDlgTitle:MCopyDlgTitle)));
  strcpy(CopyDlg[ID_SC_BTNCOPY].Data,MSG(Move?MCopyDlgRename:(Link?MCopyDlgLink:MCopyDlgCopy)));

  if(DestPanelMode == PLUGIN_PANEL)
  {
    // Если противоположная панель - плагин, то дисаблим OnlyNewer //?????
    CDP.CopySecurity=2;
    CDP.OnlyNewerFiles=0;
    CopyDlg[ID_SC_ACCOPY].Selected=0;
    CopyDlg[ID_SC_ACINHERIT].Selected=0;
    CopyDlg[ID_SC_ACLEAVE].Selected=1;
    CopyDlg[ID_SC_ACCOPY].Flags|=DIF_DISABLE;
    CopyDlg[ID_SC_ACINHERIT].Flags|=DIF_DISABLE;
    CopyDlg[ID_SC_ACLEAVE].Flags|=DIF_DISABLE;
  }

  DestPanel->GetCurDir(DestDir);
  SrcPanel->GetCurDir(SrcDir);

  if (CurrentOnly)
  /* $ 23.03.2002 IS
     При копировании только элемента под курсором берем его имя в кавычки,
     если оно содержит разделители.
  */
  {
    strcpy((char *)CopyDlg[ID_SC_TARGETEDIT].Ptr.PtrData,SelName);
    if(strpbrk((char *)CopyDlg[ID_SC_TARGETEDIT].Ptr.PtrData,",;"))
    {
      Unquote((char *)CopyDlg[ID_SC_TARGETEDIT].Ptr.PtrData);     // уберем все лишние кавычки
      InsertQuote((char *)CopyDlg[ID_SC_TARGETEDIT].Ptr.PtrData); // возьмем в кавычки, т.к. могут быть разделители
    }
  }
  /* IS $ */
  else
    switch(DestPanelMode)
    {
      case NORMAL_PANEL:
        if ((*DestDir==0 || !DestPanel->IsVisible() || !stricmp(SrcDir,DestDir)) && CDP.SelCount==1)
          strcpy((char *)CopyDlg[ID_SC_TARGETEDIT].Ptr.PtrData,SelName);
        else
        {
          AddEndSlash(strcpy((char *)CopyDlg[ID_SC_TARGETEDIT].Ptr.PtrData,DestDir));
        }
        CDP.PluginFormat[0]=0;
        /* $ 19.07.2003 IS
           Если цель содержит разделители, то возьмем ее в кавычки, дабы не получить
           ерунду при F5, Enter в панелях, когда пользователь включит MultiCopy
        */
        if(strpbrk((char *)CopyDlg[ID_SC_TARGETEDIT].Ptr.PtrData,",;"))
        {
          Unquote((char *)CopyDlg[ID_SC_TARGETEDIT].Ptr.PtrData);     // уберем все лишние кавычки
          InsertQuote((char *)CopyDlg[ID_SC_TARGETEDIT].Ptr.PtrData); // возьмем в кавычки, т.к. могут быть разделители
        }
        /* IS $ */
        break;
      case PLUGIN_PANEL:
        {
          struct OpenPluginInfo Info;
          DestPanel->GetOpenPluginInfo(&Info);
          /* $ 14.08.2000 SVS
             Данные, усеченные до 40 символов... :-(
             А потом используются ((char *)CopyDlg[2].Ptr.PtrData) по полной программе...
             "%.40s:" -> "%s:"
          */
          sprintf((char *)CopyDlg[ID_SC_TARGETEDIT].Ptr.PtrData,"%s:",NullToEmpty(Info.Format));
          /* SVS $ */
          while (strlen((char *)CopyDlg[ID_SC_TARGETEDIT].Ptr.PtrData)<2)
            strcat((char *)CopyDlg[ID_SC_TARGETEDIT].Ptr.PtrData,":");
          strupr(xstrncpy(CDP.PluginFormat,(char *)CopyDlg[ID_SC_TARGETEDIT].Ptr.PtrData,sizeof(CDP.PluginFormat)-1));
        }
        break;
    }
  strcpy(InitDestDir,(char *)CopyDlg[ID_SC_TARGETEDIT].Ptr.PtrData);

  // Для фильтра
  WIN32_FIND_DATA fd;

  //TODO: проверка здесь достаточно бессмысленна - флаг может поменяться в диалоге
  SrcPanel->GetSelName(NULL,CDP.FileAttr);
  while(SrcPanel->GetSelName(SelName,CDP.FileAttr,NULL,&fd))
  {
    if (UseFilter)
    {
      if (!Filter->FileInFilter(&fd))
        continue;
    }

    if(CDP.FileAttr & FILE_ATTRIBUTE_DIRECTORY)
    {
      CDP.FolderPresent=TRUE;
      AddSlash=TRUE;
//      break;
    }
    else
      CDP.FilesPresent=TRUE;
  }

  if(Link) // рулесы по поводу линков (предварительные!)
  {
/*
    // задисаблим опцию про симлинк, если OS < NT2000.
    if(!NT5 || ((CurrentOnly || CDP.SelCount==1) && !(CDP.FileAttr&FILE_ATTRIBUTE_DIRECTORY)))
    {
      CopyDlg[ID_SC_ONLYNEWER].Flags|=DIF_DISABLE;
      CDP.OnlyNewerFiles=CopyDlg[ID_SC_ONLYNEWER].Selected=0;
    }
*/
    // задисаблим опцию про копирование права.
    CopyDlg[ID_SC_ACCOPY].Flags|=DIF_DISABLE;
    CopyDlg[ID_SC_ACINHERIT].Flags|=DIF_DISABLE;
    CopyDlg[ID_SC_ACLEAVE].Flags|=DIF_DISABLE;

/*
    int SelectedSymLink=CopyDlg[ID_SC_ONLYNEWER].Selected;
    if(CDP.SelCount > 1 && !CDP.FilesPresent && CDP.FolderPresent)
      SelectedSymLink=1;
    if(!LinkRules(&CopyDlg[ID_SC_BTNCOPY].Flags,
                  &CopyDlg[ID_SC_ONLYNEWER].Flags,
                  &SelectedSymLink,
                  SrcDir,
                  (char *)CopyDlg[ID_SC_TARGETEDIT].Data,
                  &CDP))
    {
      _LOGCOPYR(SysLog("return -> %d LinkRules() == 0",__LINE__));
      return;
    }
    CopyDlg[ID_SC_ONLYNEWER].Selected=SelectedSymLink;
*/
  }
  // корректирем позицию " to"
  CopyDlg[ID_SC_TARGETTITLE].X1=CopyDlg[ID_SC_TARGETTITLE].X2=CopyDlg[ID_SC_SOURCEFILENAME].X1+(int)strlen(RemoveTrailingSpaces(CopyDlg[ID_SC_SOURCEFILENAME].Data));

  /* $ 15.06.2002 IS
     Обработка копирования мышкой - в этом случае диалог не показывается,
     но переменные все равно инициализируются. Если произойдет неудачная
     компиляция списка целей, то покажем диалог.
  */
  if(!Ask)
  {
    strcpy(CopyDlgValue,(char *)CopyDlg[ID_SC_TARGETEDIT].Ptr.PtrData);
    Unquote(CopyDlgValue);
    InsertQuote(CopyDlgValue);
    if(!DestList.Set(CopyDlgValue))
      Ask=TRUE;
  }
  /* IS $ */
  // ***********************************************************************
  // *** Вывод и обработка диалога
  // ***********************************************************************
  if (Ask)
  {
    FarList ComboList;
    FarListItem LinkTypeItems[4],CopyModeItems[7];
    if(Link)
    {
      ComboList.ItemsNumber=sizeof(LinkTypeItems)/sizeof(LinkTypeItems[0]);
      ComboList.Items=LinkTypeItems;
      memset(ComboList.Items,0,sizeof(FarListItem)*ComboList.ItemsNumber);
      strcpy(ComboList.Items[0].Text,MSG(MLinkTypeHardlink));
      strcpy(ComboList.Items[1].Text,MSG(MLinkTypeJunction));
      strcpy(ComboList.Items[2].Text,MSG(MLinkTypeSymlinkFile));
      strcpy(ComboList.Items[3].Text,MSG(MLinkTypeSymlinkDirectory));

      if(CDP.FilesPresent)
        ComboList.Items[0].Flags|=LIF_SELECTED;
      else
        ComboList.Items[1].Flags|=LIF_SELECTED;

      if(WinVer.dwMajorVersion<6)
      {
        ComboList.Items[2].Flags|=LIF_DISABLE;
        ComboList.Items[3].Flags|=LIF_DISABLE;
      }
    }
    else
    {
      ComboList.ItemsNumber=sizeof(CopyModeItems)/sizeof(CopyModeItems[0]);
      ComboList.Items=CopyModeItems;
      memset(ComboList.Items,0,sizeof(FarListItem)*ComboList.ItemsNumber);
      strcpy(ComboList.Items[0].Text,MSG(MCopyAsk));
      strcpy(ComboList.Items[1].Text,MSG(MCopyOverwrite));
      strcpy(ComboList.Items[2].Text,MSG(MCopySkipOvr));
      strcpy(ComboList.Items[3].Text,MSG(MCopyAppend));
      strcpy(ComboList.Items[4].Text,MSG(MCopyOnlyNewerFiles));
      strcpy(ComboList.Items[6].Text,MSG(MCopyAskRO));

      ComboList.Items[0].Flags=LIF_SELECTED;
      ComboList.Items[5].Flags=LIF_SEPARATOR;
      ComboList.Items[6].Flags=LIF_CHECKED;
    }
    CopyDlg[ID_SC_COMBO].ListItems=&ComboList;

    Dialog Dlg(CopyDlg,sizeof(CopyDlg)/sizeof(CopyDlg[0]),CopyDlgProc,(LONG_PTR)&CDP);
    Dlg.SetHelp(Link?"HardSymLink":"CopyFiles");
    Dlg.SetPosition(-1,-1,DLG_WIDTH,DLG_HEIGHT);

//    Dlg.Show();
    /* $ 02.06.2001 IS
       + Проверим список целей и поднимем тревогу, если он содержит ошибки
    */
    int DlgExitCode;
    for(;;)
    {
      Dlg.ClearDone();
      Dlg.Process();
      DlgExitCode=Dlg.GetExitCode();

      //Рефреш текущему времени для фильтра сразу после выхода из диалога
      Filter->UpdateCurrentTime();

      if(DlgExitCode == ID_SC_BTNCOPY)
      {
        /* $ 03.08.2001 IS
           Запомним строчку из диалога и начинаем ее мучить в зависимости от
           состояния опции мультикопирования
        */
        strcpy(CopyDlgValue,(char *)CopyDlg[ID_SC_TARGETEDIT].Ptr.PtrData);
        Opt.CMOpt.MultiCopy=CopyDlg[ID_SC_MULTITARGET].Selected;
        if(!Opt.CMOpt.MultiCopy || !strpbrk((char *)CopyDlg[ID_SC_TARGETEDIT].Ptr.PtrData,",;")) // отключено multi*
        {
           // уберем пробелы, лишние кавычки
           RemoveTrailingSpaces((char *)CopyDlg[ID_SC_TARGETEDIT].Ptr.PtrData);
           Unquote((char *)CopyDlg[ID_SC_TARGETEDIT].Ptr.PtrData);
           RemoveTrailingSpaces(CopyDlgValue);
           Unquote(CopyDlgValue);

           // добавим кавычки, чтобы "список" удачно скомпилировался вне
           // зависимости от наличия разделителей в оном
           InsertQuote(CopyDlgValue);
        }
        /* IS $ */
        if(DestList.Set(CopyDlgValue) && !strpbrk(CopyDlgValue,ReservedFilenameSymbols))
        {
          // Запомнить признак использования фильтра. KM
          UseFilter=CopyDlg[ID_SC_USEFILTER].Selected;
          break;
        }
        else
          Message(MSG_DOWN|MSG_WARNING,1,MSG(MWarning),MSG(MCopyIncorrectTargetList), MSG(MOk));
      }
      else
        break;
    }

    /* IS $ */
    if(DlgExitCode == ID_SC_BTNCANCEL || DlgExitCode < 0 || (CopyDlg[ID_SC_BTNCOPY].Flags&DIF_DISABLE))
    {
      if (DestPlugin)
        ToPlugin=-1;
      _LOGCOPYR(SysLog("return -> %d",__LINE__));
      return;
    }
  }
  // ***********************************************************************
  // *** Стадия подготовки данных после диалога
  // ***********************************************************************
  ShellCopy::Flags&=~FCOPY_COPYPARENTSECURITY;
  if(CopyDlg[ID_SC_ACCOPY].Selected)
  {
    ShellCopy::Flags|=FCOPY_COPYSECURITY;
  }
  else if(CopyDlg[ID_SC_ACINHERIT].Selected)
  {
    ShellCopy::Flags&=~(FCOPY_COPYSECURITY|FCOPY_LEAVESECURITY);
  }
  else
  {
    ShellCopy::Flags|=FCOPY_LEAVESECURITY;
  }

  if(Opt.CMOpt.UseSystemCopy)
    ShellCopy::Flags|=FCOPY_USESYSTEMCOPY;
  else
    ShellCopy::Flags&=~FCOPY_USESYSTEMCOPY;

  if(!(ShellCopy::Flags&(FCOPY_COPYSECURITY|FCOPY_LEAVESECURITY)))
    ShellCopy::Flags|=FCOPY_COPYPARENTSECURITY;

  CDP.CopySecurity=ShellCopy::Flags&FCOPY_COPYSECURITY?1:(ShellCopy::Flags&FCOPY_LEAVESECURITY?2:0);

  // в любом случае сохраняем сессионное запоминание (не для Link, т.к. для Link временное состояние - "ВСЕГДА!")
  if(!Link)
  {
    if(Move)
      CopySecurityMove=CDP.CopySecurity;
    else
      CopySecurityCopy=CDP.CopySecurity;
  }

  ReadOnlyDelMode=ReadOnlyOvrMode=OvrMode=SkipEncMode=SkipMode=SkipDeleteMode=-1;

  if(Link)
  {
    switch(CopyDlg[ID_SC_COMBO].ListPos)
    {
      case 0:
        RPT=RP_HARDLINK;
        break;
      case 1:
        RPT=RP_JUNCTION;
        break;
      case 2:
        RPT=RP_SYMLINKFILE;
        break;
      case 3:
        RPT=RP_SYMLINKDIR;
        break;
    }
  }
  else
  {
    ReadOnlyOvrMode=CDP.AskRO?-1:1;
    switch(CopyDlg[ID_SC_COMBO].ListPos)
    {
      case CM_ASK:
        OvrMode=-1;
        break;
      case CM_OVERWRITE:
        OvrMode=1;
        break;
      case CM_SKIP:
        OvrMode=3;
        ReadOnlyOvrMode=CDP.AskRO?-1:3;
        break;
      case CM_APPEND:
        OvrMode=5;
        break;
      case CM_ONLYNEWER:
        ShellCopy::Flags|=FCOPY_ONLYNEWERFILES;
        break;
    }
  }
  ShellCopy::Flags|=CopyDlg[ID_SC_COPYSYMLINK].Selected?FCOPY_COPYSYMLINKCONTENTS:0;

  if (DestPlugin && !strcmp((char *)CopyDlg[ID_SC_TARGETEDIT].Ptr.PtrData,InitDestDir))
  {
    ToPlugin=1;
    _LOGCOPYR(SysLog("return -> %d",__LINE__));
    return;
  }

  int WorkMove=Move;

  _LOGCOPYR(SysLog("CopyDlg[ID_SC_TARGETEDIT].Ptr.PtrData='%s'",(char *)CopyDlg[ID_SC_TARGETEDIT].Ptr.PtrData));
  if(CheckNulOrCon((char *)CopyDlg[ID_SC_TARGETEDIT].Ptr.PtrData))
    ShellCopy::Flags|=FCOPY_COPYTONUL;
  if(ShellCopy::Flags&FCOPY_COPYTONUL)
  {
    ShellCopy::Flags&=~FCOPY_MOVE;
    StaticMove=WorkMove=0;
  }

  if(CDP.SelCount==1 || (ShellCopy::Flags&FCOPY_COPYTONUL))
    AddSlash=FALSE; //???

  if (DestPlugin==2)
  {
    if (PluginDestPath)
      strcpy(PluginDestPath,(char *)CopyDlg[ID_SC_TARGETEDIT].Ptr.PtrData);
    _LOGCOPYR(SysLog("return -> %d",__LINE__));
    return;
  }

  _tran(SysLog("[%p] ShellCopy::ShellCopy() 5",this));
  if (Opt.Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED && SrcPanel->IsDizDisplayed() ||
      Opt.Diz.UpdateMode==DIZ_UPDATE_ALWAYS)
  {
    CtrlObject->Cp()->LeftPanel->ReadDiz();
    CtrlObject->Cp()->RightPanel->ReadDiz();
  }

  CopyBuffer=new char[CopyBufferSize];
  DestPanel->CloseFile();
  *DestDizPath=0;
  SrcPanel->SaveSelection();

  // TODO: Posix - bugbug
  for (int I=0;CopyDlgValue[I]!=0;I++)
    if (CopyDlgValue[I]=='/')
      CopyDlgValue[I]='\\';

 // нужно ли показывать время копирования?
  ShowCopyTime = Opt.CMOpt.CopyTimeRule & ((ShellCopy::Flags&FCOPY_COPYTONUL)?COPY_RULE_NUL:COPY_RULE_FILES);

  // ***********************************************************************
  // **** Здесь все подготовительные операции закончены, можно приступать
  // **** к процессу Copy/Move/Link
  // ***********************************************************************

  int NeedDizUpdate=FALSE;
  int NeedUpdateAPanel=FALSE;

  TaskBar TB;
  // ПОКА! принудительно выставим обновление.
  // В последствии этот флаг будет выставляться в ShellCopy::CheckUpdatePanel()
  ShellCopy::Flags|=FCOPY_UPDATEPPANEL;

  /*
     ЕСЛИ ПРИНЯТЬ В КАЧЕСТВЕ РАЗДЕЛИТЕЛЯ ПУТЕЙ, НАПРИМЕР ';',
     то нужно парсить CopyDlgValue на предмет MultiCopy и
     вызывать CopyFileTree нужное количество раз.
  */
  /* $ 02.06.2001 IS
     + Коректная обработка списка целей с учетом кавычек
  */
  {
    ShellCopy::Flags&=~FCOPY_MOVE;
    if(DestList.Set(CopyDlgValue)) // если список успешно "скомпилировался"
    {
        const char *NamePtr;
        char *NameTmp=new char[SizeBuffer];

        // переинициализируем переменные в самом начале (BugZ#171)
//        CopyBufSize = COPY_BUFFER_SIZE; // Начинаем с 1к
        CopyBufSize = CopyBufferSize;

        // посчитаем количество целей.
        DestList.Reset();
        CountTarget=0;
        while(DestList.GetNext())
          CountTarget++;

        DestList.Reset();
        TotalFiles=0;
        TotalCopySize=TotalCopiedSize=TotalSkippedSize=0;
        // Запомним время начала
        if (ShowCopyTime)
        {
          CopyStartTime = clock();
          WaitUserTime = OldCalcTime = 0;
        }

        CopyTitle = new ConsoleTitle(NULL);
        StaticCopyTitle=CopyTitle;

        if(CountTarget > 1)
          StaticMove=WorkMove=0;

        while(NULL!=(NamePtr=DestList.GetNext()))
        {
          CurCopiedSize=0;
          CopyTitle->Set(Move ? MSG(MCopyMovingTitle):MSG(MCopyCopyingTitle));

          xstrncpy(NameTmp, NamePtr,SizeBuffer-1);
          _LOGCOPYR(SysLog("NamePtr='%s', Move=%d",NamePtr,WorkMove));

          if(isalpha(NameTmp[0]) && NameTmp[1]==':' && !NameTmp[2])
            PrepareDiskPath(NameTmp,SizeBuffer,true);

          if(!strcmp(NameTmp,"..") && IsLocalRootPath(SrcDir))
          {
            if(Message(MSG_WARNING,2,MSG(MError),MSG((!Move?MCannotCopyToTwoDot:MCannotMoveToTwoDot)),MSG(MCannotCopyMoveToTwoDot),MSG(MCopySkip),MSG(MCopyCancel)) == 0)
              continue;
            break;
          }

          if(CheckNulOrCon(NameTmp))
            ShellCopy::Flags|=FCOPY_COPYTONUL;
          else
            ShellCopy::Flags&=~FCOPY_COPYTONUL;

          if(ShellCopy::Flags&FCOPY_COPYTONUL)
          {
            ShellCopy::Flags&=~FCOPY_MOVE;
            StaticMove=WorkMove=0;
          }
//          else
//            StaticMove=WorkMove=Move;

          if(DestList.IsEmpty()) // нужно учесть моменты связанные с операцией Move.
          {
            StaticMove=WorkMove=Move;
            ShellCopy::Flags|=WorkMove?FCOPY_MOVE:0; // только для последней операции
            ShellCopy::Flags|=FCOPY_COPYLASTTIME;
          }

          // Если выделенных элементов больше 1 и среди них есть каталог, то всегда
          // делаем так, чтобы на конце был '\\'
          // деламем так не всегда, а только когда NameTmp не является маской.
          if (AddSlash && strpbrk(NameTmp,"*?")==NULL)
            AddEndSlash(NameTmp);

          // Для перемещение одного объекта скинем просчет "тотала"
          if (CDP.SelCount==1 && WorkMove && strpbrk(NameTmp,":\\")==NULL)
            ShowTotalCopySize=FALSE;

          if(WorkMove) // при перемещении "тотал" так же скидывается для "того же диска"
          {
            if (!UseFilter && IsSameDisk(SrcDir,NameTmp))
              ShowTotalCopySize=FALSE;
            if(CDP.SelCount==1 && CDP.FolderPresent && CheckUpdateAnotherPanel(SrcPanel,SelName))
            {
              NeedUpdateAPanel=TRUE;
            }
          }

          // Обнулим инфу про дизы
          *DestDizPath=0;
          ShellCopy::Flags&=~FCOPY_DIZREAD;

          // сохраним выделение
          SrcPanel->SaveSelection();

          *DestDriveRoot=0;
          *DestFSName=0;

          int OldCopySymlinkContents=ShellCopy::Flags&FCOPY_COPYSYMLINKCONTENTS;
          int I;

         // собственно - один проход копирования
          // Mantis#45: Необходимо привсти копирование ссылок на папки с NTFS на FAT к более логичному виду
          if(!CheckFileSystem(NameTmp))
            ShellCopy::Flags|=FCOPY_COPYSYMLINKCONTENTS;

          PreRedraw.Push(ShellCopy::PR_ShellCopyMsg);
          I=CopyFileTree(NameTmp);
          PreRedraw.Pop();

          if(OldCopySymlinkContents)
            ShellCopy::Flags|=FCOPY_COPYSYMLINKCONTENTS;
          else
            ShellCopy::Flags&=~FCOPY_COPYSYMLINKCONTENTS;

          if(I == COPY_CANCEL)
          {
            NeedDizUpdate=TRUE;
            break;
          }

          // если "есть порох в пороховницах" - восстановим выделение
          if(!DestList.IsEmpty())
            SrcPanel->RestoreSelection();

          // Позаботимся о дизах.
          if (!(ShellCopy::Flags&FCOPY_COPYTONUL) && *DestDizPath)
          {
            char DestDizName[NM*2];
            DestDiz.GetDizName(DestDizName);
            DWORD Attr=GetFileAttributes(DestDizName);
            int DestReadOnly=(Attr!=INVALID_FILE_ATTRIBUTES && (Attr & FA_RDONLY));
            if(DestList.IsEmpty()) // Скидываем только во время последней Op.
              if (WorkMove && !DestReadOnly)
                SrcPanel->FlushDiz();
            DestDiz.Flush(DestDizPath);
          }
        }
        StaticCopyTitle=NULL;
        delete CopyTitle;

        delete[] NameTmp;
    }
    _LOGCOPYR(else SysLog("Error: DestList.Set(CopyDlgValue) return FALSE"));
  }
  /* IS $ */

  // ***********************************************************************
  // *** заключительеая стадия процесса
  // *** восстанавливаем/дизим/редравим
  // ***********************************************************************

  if(NeedDizUpdate) // при мультикопировании может быть обрыв, но нам все
  {                 // равно нужно апдейтить дизы!
    if (!(ShellCopy::Flags&FCOPY_COPYTONUL) && *DestDizPath)
    {
      char DestDizName[NM+32];
      DestDiz.GetDizName(DestDizName);
      DWORD Attr=GetFileAttributes(DestDizName);
      int DestReadOnly=(Attr!=INVALID_FILE_ATTRIBUTES && (Attr & FA_RDONLY));
      if (Move && !DestReadOnly)
        SrcPanel->FlushDiz();
      DestDiz.Flush(DestDizPath);
    }
  }

#if 1
  SrcPanel->Update(UPDATE_KEEP_SELECTION);
  int LenRenamedName=0;
  if (CDP.SelCount==1 && *RenamedName)
  {
    LenRenamedName=(int)strlen(RenamedName);
    SrcPanel->GoToFile(RenamedName);
  }
#if 1
  if(NeedUpdateAPanel && CDP.FileAttr != -1 && (CDP.FileAttr&FILE_ATTRIBUTE_DIRECTORY) && DestPanelMode != PLUGIN_PANEL)
  {
    SrcPanel->GetCurDir(SrcDir);
    DestPanel->SetCurDir(SrcDir,FALSE);
  }
#else
  if(CDP.FileAttr != -1 && (CDP.FileAttr&FILE_ATTRIBUTE_DIRECTORY) && DestPanelMode != PLUGIN_PANEL)
  {
    // если SrcDir содержится в DestDir...
    DestPanel->GetCurDir(DestDir);
    SrcPanel->GetCurDir(SrcDir);

    // исправляем ошибку обновления пассивной панели.
//    int LenSrcDir=strlen(SrcDir);
//    int LenDstDir=strlen(DestDir);
//    if(CDP.SelCount == 1 && !LocalStrnicmp(DestDir,RenamedName,Min(LenDstDir,LenRenamedName)) ||
//      (LenDstDir > LenSrcDir && !LocalStrnicmp(DestDir,SrcDir,Min(LenDstDir,LenSrcDir)) && !IsLocalRootPath(SrcDir)))
    if(CheckUpdateAnotherPanel(SrcPanel,SrcDir))
    {
      // ...то просто.. ;-) выставим тот же каталог, что позволит обновить
      // панель.
      DestPanel->SetCurDir(DestDir,FALSE);
    }
  }
#endif
  // проверим "нужность" апдейта пассивной панели
  if(ShellCopy::Flags&FCOPY_UPDATEPPANEL)
  {
    DestPanel->SortFileList(TRUE);
    DestPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
  }

  if(SrcPanelMode == PLUGIN_PANEL)
    SrcPanel->SetPluginModified();

  CtrlObject->Cp()->Redraw();
#else

  SrcPanel->Update(UPDATE_KEEP_SELECTION);
  if (CDP.SelCount==1 && *RenamedName)
    SrcPanel->GoToFile(RenamedName);

  SrcPanel->Redraw();

  DestPanel->SortFileList(TRUE);
  DestPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
  DestPanel->Redraw();
#endif
}

LONG_PTR WINAPI ShellCopy::CopyDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
#define DM_CALLTREE (DM_USER+1)
#define DM_SWITCHRO (DM_USER+2)

  struct CopyDlgParam *DlgParam;
  DlgParam=(struct CopyDlgParam *)Dialog::SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);

  switch(Msg)
  {
  case DN_INITDIALOG:
    Dialog::SendDlgMessage(hDlg,DM_SETCOMBOBOXEVENT,ID_SC_COMBO,CBET_KEY|CBET_MOUSE);
    Dialog::SendDlgMessage(hDlg,DM_SETMOUSEEVENTNOTIFY,TRUE,0);
    break;
  case DM_SWITCHRO:
  {
    FarListGetItem LGI={6};
    Dialog::SendDlgMessage(hDlg,DM_LISTGETITEM,ID_SC_COMBO,(LONG_PTR)&LGI);
    if(LGI.Item.Flags&LIF_CHECKED)
      LGI.Item.Flags&=~LIF_CHECKED;
    else
      LGI.Item.Flags|=LIF_CHECKED;
    Dialog::SendDlgMessage(hDlg,DM_LISTUPDATE,ID_SC_COMBO,(LONG_PTR)&LGI);
    return TRUE;
  }
    case DN_BTNCLICK:
    {
      if (Param1==ID_SC_USEFILTER) // "Use filter"
      {
        UseFilter=(int)Param2;
        return TRUE;
      }
      if(Param1 == ID_SC_BTNTREE) // Tree
      {
        Dialog::SendDlgMessage(hDlg,DM_CALLTREE,0,0);
        return FALSE;
      }
      else if(Param1 == ID_SC_BTNCOPY)
      {
        Dialog::SendDlgMessage(hDlg,DM_CLOSE,ID_SC_BTNCOPY,0);
      }
      /*
      else if(Param1 == ID_SC_ONLYNEWER && ((DlgParam->thisClass->Flags)&FCOPY_LINK))
      {
        // подсократим код путем эмуляции телодвижений в строке ввода :-))
        struct FarDialogItem DItemTargetEdit;
        Dialog::SendDlgMessage(hDlg,DM_GETDLGITEM,ID_SC_TARGETEDIT,(LONG_PTR)&DItemTargetEdit);
        Dialog::SendDlgMessage(hDlg,DN_EDITCHANGE,ID_SC_TARGETEDIT,(LONG_PTR)&DItemTargetEdit);
      }
      */
      else if (Param1==ID_SC_BTNFILTER) // Filter
      {
        Filter->FilterEdit();
        return TRUE;
      }
      break;
    }

    case DM_KEY: // по поводу дерева!
    {
      if(Param2 == KEY_ALTF10 || Param2 == KEY_F10 || Param2 == KEY_SHIFTF10)
      {
        DlgParam->AltF10=Param2 == KEY_ALTF10?1:(Param2 == KEY_SHIFTF10?2:0);
        Dialog::SendDlgMessage(hDlg,DM_CALLTREE,DlgParam->AltF10,0);
        return TRUE;
      }
      if(Param1 == ID_SC_COMBO)
      {
        if(Param2==KEY_ENTER || Param2==KEY_NUMENTER || Param2==KEY_INS || Param2==KEY_NUMPAD0 || Param2==KEY_SPACE)
        {
          if(Dialog::SendDlgMessage(hDlg,DM_LISTGETCURPOS,ID_SC_COMBO,0)==6)
            return Dialog::SendDlgMessage(hDlg,DM_SWITCHRO,0,0);
        }
      }
    }
    break;
  case DN_MOUSEEVENT:
    if(Dialog::SendDlgMessage(hDlg,DM_GETDROPDOWNOPENED,ID_SC_COMBO,0))
    {
      MOUSE_EVENT_RECORD *mer=(MOUSE_EVENT_RECORD *)Param2;
      if(Dialog::SendDlgMessage(hDlg,DM_LISTGETCURPOS,ID_SC_COMBO,0)==6 && mer->dwButtonState && !mer->dwEventFlags)
      {
        Dialog::SendDlgMessage(hDlg,DM_SWITCHRO,0,0);
        return FALSE;
      }
    }
    break;

    case DN_EDITCHANGE:
      if(Param1 == 2)
      {
        struct FarDialogItem DItemACCopy,DItemACInherit,DItemACLeave,/*DItemOnlyNewer,*/DItemBtnCopy;
        int LenCurDirName=DlgParam->thisClass->SrcPanel->GetCurDir(NULL);
        char *SrcDir=(char *)alloca(LenCurDirName+16);
        DlgParam->thisClass->SrcPanel->GetCurDir(SrcDir);
        Dialog::SendDlgMessage(hDlg,DM_GETDLGITEM,ID_SC_ACCOPY,(LONG_PTR)&DItemACCopy);
        Dialog::SendDlgMessage(hDlg,DM_GETDLGITEM,ID_SC_ACINHERIT,(LONG_PTR)&DItemACInherit);
        Dialog::SendDlgMessage(hDlg,DM_GETDLGITEM,ID_SC_ACLEAVE,(LONG_PTR)&DItemACLeave);
        //Dialog::SendDlgMessage(hDlg,DM_GETDLGITEM,ID_SC_ONLYNEWER,(LONG_PTR)&DItemOnlyNewer);
        Dialog::SendDlgMessage(hDlg,DM_GETDLGITEM,ID_SC_BTNCOPY,(LONG_PTR)&DItemBtnCopy);

        // Это создание линка?
        if((DlgParam->thisClass->Flags)&FCOPY_LINK)
        {
/* пока отключим
          DlgParam->thisClass->LinkRules(&DItemBtnCopy.Flags,
                    &DItemOnlyNewer.Flags,
                    &DItemOnlyNewer.Param.Selected,
                    SrcDir,
                    ((struct FarDialogItem *)Param2)->Data.Ptr.PtrData,DlgParam);
*/
        }
        else // обычные Copy/Move
        {
          struct FarDialogItem *DItemTargetEdit=(struct FarDialogItem *)Param2;
          char *Buf=(char*)alloca(DItemTargetEdit->Data.Ptr.PtrLength);
          strupr(strcpy(Buf,DItemTargetEdit->Data.Ptr.PtrData));
          if(*DlgParam->PluginFormat && strstr(Buf,DlgParam->PluginFormat))
          {
            DItemACCopy.Flags|=DIF_DISABLE;
            DItemACInherit.Flags|=DIF_DISABLE;
            DItemACLeave.Flags|=DIF_DISABLE;
            //DItemOnlyNewer.Flags|=DIF_DISABLE;
            //DlgParam->OnlyNewerFiles=DItemOnlyNewer.Param.Selected;
            DlgParam->CopySecurity=0;
            if (DItemACCopy.Param.Selected)
              DlgParam->CopySecurity=1;
            else if (DItemACLeave.Param.Selected)
              DlgParam->CopySecurity=2;
            DItemACCopy.Param.Selected=0;
            DItemACInherit.Param.Selected=0;
            DItemACLeave.Param.Selected=1;
            //DItemOnlyNewer.Param.Selected=0;
          }
          else
          {
            DItemACCopy.Flags&=~DIF_DISABLE;
            DItemACInherit.Flags&=~DIF_DISABLE;
            DItemACLeave.Flags&=~DIF_DISABLE;
            //DItemOnlyNewer.Flags&=~DIF_DISABLE;
            //DItemOnlyNewer.Param.Selected=DlgParam->OnlyNewerFiles;
            DItemACCopy.Param.Selected=0;
            DItemACInherit.Param.Selected=0;
            DItemACLeave.Param.Selected=0;
            if (DlgParam->CopySecurity == 1)
            {
              DItemACCopy.Param.Selected=1;
            }
            else if (DlgParam->CopySecurity == 2)
            {
              DItemACLeave.Param.Selected=1;
            }
            else
              DItemACInherit.Param.Selected=1;
          }
        }

        Dialog::SendDlgMessage(hDlg,DM_SETDLGITEM,ID_SC_ACCOPY,(LONG_PTR)&DItemACCopy);
        Dialog::SendDlgMessage(hDlg,DM_SETDLGITEM,ID_SC_ACINHERIT,(LONG_PTR)&DItemACInherit);
        Dialog::SendDlgMessage(hDlg,DM_SETDLGITEM,ID_SC_ACLEAVE,(LONG_PTR)&DItemACLeave);
        //Dialog::SendDlgMessage(hDlg,DM_SETDLGITEM,ID_SC_ONLYNEWER,(LONG_PTR)&DItemOnlyNewer);
        Dialog::SendDlgMessage(hDlg,DM_SETDLGITEM,ID_SC_BTNCOPY,(LONG_PTR)&DItemBtnCopy);
      }
      break;

    case DM_CALLTREE:
    {
      /* $ 13.10.2001 IS
         + При мультикопировании добавляем выбранный в "дереве" каталог к уже
           существующему списку через точку с запятой.
         - Баг: при мультикопировании выбранный в "дереве" каталог не
           заключался в кавычки, если он содержал в своем
           имени символы-разделители.
         - Баг: неправильно работало Shift-F10, если строка ввода содержала
           слеш на конце.
         - Баг: неправильно работало Shift-F10 при мультикопировании -
           показывался корневой каталог, теперь показывается самый первый каталог
           в списке.
      */
      BOOL MultiCopy=Dialog::SendDlgMessage(hDlg,DM_GETCHECK,ID_SC_MULTITARGET,0)==BSTATE_CHECKED;
      struct FarDialogItem DItemTargetEdit;
      Dialog::SendDlgMessage(hDlg,DM_GETDLGITEM,ID_SC_TARGETEDIT,(LONG_PTR)&DItemTargetEdit);

      char *NewFolder=(char*)alloca(DItemTargetEdit.Data.Ptr.PtrLength);
      char *OldFolder=(char*)DItemTargetEdit.Data.Ptr.PtrData;

      *NewFolder = 0;
      if(DlgParam->AltF10 == 2)
      {
        strcpy(NewFolder, OldFolder);
        if(MultiCopy)
        {
          UserDefinedList DestList(0,0,ULF_UNIQUE);
          if(DestList.Set(OldFolder))
          {
            DestList.Reset();
            const char *NamePtr=DestList.GetNext();
            if(NamePtr)
              xstrncpy(NewFolder, NamePtr, DItemTargetEdit.Data.Ptr.PtrLength-1);
          }
        }
        if(*NewFolder == 0)
          DlgParam->AltF10=-1;
        else // убираем лишний слеш
          DeleteEndSlash(NewFolder);
      }

      if(DlgParam->AltF10 != -1)
      {
        {
          FolderTree Tree(NewFolder,
               (DlgParam->AltF10==1?MODALTREE_PASSIVE:
                  (DlgParam->AltF10==2?MODALTREE_FREE:
                     MODALTREE_ACTIVE)),
               FALSE,FALSE);
        }
        if (*NewFolder)
        {
          AddEndSlash(NewFolder);
          if(MultiCopy) // мультикопирование
          {
            // Добавим кавычки, если имя каталога содержит символы-разделители
            if(strpbrk(NewFolder,";,"))
              InsertQuote(NewFolder);

            int len=(int)strlen(OldFolder), newlen=(int)strlen(NewFolder), addSep=0;
            addSep=len>0;
            if(len+newlen+addSep < DItemTargetEdit.Data.Ptr.PtrLength)// контролируем длину строки
            {
              if(addSep)
                OldFolder[len++]=';'; // добавим разделитель к непустому списку
              strcpy(OldFolder+len, NewFolder);
            }
            strcpy(NewFolder, OldFolder);
          }
          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_SC_TARGETEDIT,(LONG_PTR)NewFolder);
          Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,ID_SC_TARGETEDIT,0);
        }
      }
      DlgParam->AltF10=0;
      return TRUE;
    }
  case DN_CLOSE:
  {
    if(Param1==ID_SC_BTNCOPY)
    {
      FarListGetItem LGI={6};
      Dialog::SendDlgMessage(hDlg,DM_LISTGETITEM,ID_SC_COMBO,(LONG_PTR)&LGI);
      if(LGI.Item.Flags&LIF_CHECKED)
        DlgParam->AskRO=TRUE;
    }
  }
  break;
  }
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

BOOL ShellCopy::LinkRules(DWORD *Flags9,DWORD* Flags5,int* Selected5,
                         char *SrcDir,char *DstDir,struct CopyDlgParam *CDP)
{
// пока отключим
#if 0
  char Root[1024];
  *Flags9|=DIF_DISABLE; // дисаблим сразу!
  *Flags5|=DIF_DISABLE;

  if(DstDir && DstDir[0] == '\\' && DstDir[1] == '\\')
  {
    *Selected5=0;
    return TRUE;
  }
//// // _SVS(SysLog("\n---"));
  // получаем полную инфу о источнике и приемнике
  if(CDP->IsDTSrcFixed == -1)
  {
    char FSysNameSrc[NM];
    xstrncpy(Root,SrcDir,sizeof(Root));
    Unquote(Root);
    ConvertNameToFull(Root,Root, sizeof(Root));
    GetPathRoot(Root,Root);
//// // _SVS(SysLog("SrcDir=%s",SrcDir));
//// // _SVS(SysLog("Root=%s",Root));
    CDP->IsDTSrcFixed=FAR_GetDriveType(Root);
    CDP->IsDTSrcFixed=CDP->IsDTSrcFixed == DRIVE_FIXED ||
                      IsDriveTypeCDROM(CDP->IsDTSrcFixed) ||
                      (NT5 && WinVer.dwMinorVersion>0?DRIVE_REMOVABLE:0);
    GetVolumeInformation(Root,NULL,0,NULL,NULL,&CDP->FileSystemFlagsSrc,FSysNameSrc,sizeof(FSysNameSrc));
    CDP->FSysNTFS=!stricmp(FSysNameSrc,"NTFS")?TRUE:FALSE;
//// // _SVS(SysLog("FSysNameSrc=%s",FSysNameSrc));
  }

/*
С сетевого на локаль - имеем задисабленную [ ] Symbolic link.
С локали на сетевой  - происходит удачная попытка
*/
  // 1. если источник находится не на логическом диске
  if(CDP->IsDTSrcFixed || CDP->FSysNTFS)
  {
    char FSysNameDst[NM];
    DWORD FileSystemFlagsDst;

    xstrncpy(Root,DstDir,sizeof(Root));
    Unquote(Root);

    ConvertNameToFull(Root,Root, sizeof(Root));
    GetPathRoot(Root,Root);
    if(GetFileAttributes(Root) == -1)
      return TRUE;

    //GetVolumeInformation(Root,NULL,0,NULL,NULL,&FileSystemFlagsDst,FSysNameDst,sizeof(FSysNameDst));
    // 3. если приемник находится не на логическом диске
    CDP->IsDTDstFixed=FAR_GetDriveType(Root);
    CDP->IsDTDstFixed=CDP->IsDTDstFixed == DRIVE_FIXED || IsDriveTypeCDROM(CDP->IsDTSrcFixed);
    *FSysNameDst=0;
    GetVolumeInformation(Root,NULL,0,NULL,NULL,&FileSystemFlagsDst,FSysNameDst,sizeof(FSysNameDst));
    int SameDisk=IsSameDisk(SrcDir,DstDir);
    int IsHardLink=(!CDP->FolderPresent && CDP->FilesPresent && SameDisk && (CDP->IsDTDstFixed || !stricmp(FSysNameDst,"NTFS")));
    // 4. если источник находится на логическом диске, отличном от NTFS
    if(!IsHardLink && (CDP->IsDTDstFixed || !stricmp(FSysNameDst,"NTFS")) || IsHardLink)
    {
      if(CDP->SelCount == 1)
      {
        if(CDP->FolderPresent) // Folder?
        {
          // . если источник находится на логическом диске NTFS, но не поддерживающим repase point
          if(NT5 &&
//             (CDP->FileSystemFlagsSrc&FILE_SUPPORTS_REPARSE_POINTS) &&
             (FileSystemFlagsDst&FILE_SUPPORTS_REPARSE_POINTS) &&
//    ! заблокирована возможность создавать симлинки с локали на сеть -
//      все равно в такой каталог не ввалишь (то биш, бага была)
             CDP->IsDTDstFixed && CDP->IsDTSrcFixed)
          {
            *Flags5 &=~ DIF_DISABLE;
            // это проверка на вшивость, когда на делаем линк на каталог на другом
            // диске и... сняли галку с симлинка.
            if(*Selected5 || (!*Selected5 && SameDisk))
               *Flags9 &=~ DIF_DISABLE;

            if(!CDP->IsDTDstFixed && SameDisk)
            {
              *Selected5=0;
              *Flags5 |= DIF_DISABLE;
              *Flags9 &=~ DIF_DISABLE;
            }
          }
          else if(NT /* && !NT5 */ && SameDisk)
          {
            *Selected5=0;
            *Flags9 &=~ DIF_DISABLE;
          }
          else
          {
            *Selected5=0;
//            *Flags9 &=~ DIF_DISABLE;
          }
        }
        else if(SameDisk)// && CDP->FSysNTFS) // это файл!
        {
          *Selected5=0;
          *Flags9 &=~ DIF_DISABLE;
        }
      }
      else
      {
        if(CDP->FolderPresent)
        {
          if(NT5 && (FileSystemFlagsDst&FILE_SUPPORTS_REPARSE_POINTS))
          {
            *Flags5 &=~ DIF_DISABLE;
            if(!CDP->FilesPresent)
            {
              *Flags9 &=~ DIF_DISABLE;
            }

            if(!CDP->IsDTDstFixed && SameDisk)
            {
              *Selected5=0;
              *Flags5 |= DIF_DISABLE;
              *Flags9 &=~ DIF_DISABLE;
            }
          }
          else if(NT && !NT5 && SameDisk)
          {
            *Selected5=0;
            *Flags9 &=~ DIF_DISABLE;
          }

          if(CDP->FilesPresent && SameDisk)// && CDP->FSysNTFS)
          {
//            *Selected5=0;
            *Flags9 &=~ DIF_DISABLE;
          }
        }
        else if(SameDisk)// && CDP->FSysNTFS) // это файл!
        {
          *Selected5=0;
          *Flags9 &=~ DIF_DISABLE;
        }
      }
    }
  }
  else
    return FALSE;
#endif
  return TRUE;
}


ShellCopy::~ShellCopy()
{
  _tran(SysLog("[%p] ShellCopy::~ShellCopy(), CopyBufer=%p",this,CopyBuffer));
  if ( CopyBuffer )
    delete[] CopyBuffer;

  // $ 26.05.2001 OT Разрешить перерисовку панелей
  _tran(SysLog("call (*FrameManager)[0]->UnlockRefresh()"));
  (*FrameManager)[0]->Unlock();
  (*FrameManager)[0]->Refresh();

  if(sddata)
    delete[] sddata;

  if(Filter) // Уничтожим объект фильтра
    delete Filter;
}


COPY_CODES ShellCopy::CopyFileTree(char *Dest)
{
  _LOGCOPYR(CleverSysLog Clev("ShellCopy::CopyFileTree()"));
  _LOGCOPYR(SysLog("Params: Dest='%s'",Dest));
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  //SaveScreen SaveScr;
  DWORD DestAttr=(DWORD)-1;
  _tran(SysLog("[%p] ShellCopy::CopyFileTree()",this));

  char SelName[NM],SelShortName[NM];
  int Length,FileAttr;

  if ((Length=(int)strlen(Dest))==0 || strcmp(Dest,".")==0)
  {
    _LOGCOPYR(SysLog("return COPY_FAILURE -> strlen('%s')=%d",Dest,Length));
    return COPY_FAILURE; //????
  }

  SetCursorType(FALSE,0);

  ShellCopy::Flags&=~(FCOPY_STREAMSKIP|FCOPY_STREAMALL);

  if(TotalCopySize == 0)
  {
    *TotalCopySizeText=0;
    /* $ 19.12.2001 VVM
      ! Не сканируем каталоги при создании линков */
    if (ShowTotalCopySize && !(ShellCopy::Flags&FCOPY_LINK) && !CalcTotalSize())
    {
      _LOGCOPYR(SysLog("return COPY_FAILURE -> if (ShowTotalCopySize && !(ShellCopy::Flags&FCOPY_LINK) && !CalcTotalSize())"));
      return COPY_FAILURE;
    }
  }
  else
    CurCopiedSize=0;

  ShellCopyMsg("","",MSG_LEFTALIGN);

  LastShowTime = 0;

  // Создание структуры каталогов в месте назначения
  if(!(ShellCopy::Flags&FCOPY_COPYTONUL))
  {
    //if (Length > 1 && Dest[Length-1]=='\\' && Dest[Length-2]!=':') //??????????
    {
      char NewPath[NM*3];
      xstrncpy(NewPath,Dest,sizeof(NewPath)-1);
      char *Ptr=strrchr(NewPath,'\\');
      if(!Ptr)
        Ptr=strrchr(NewPath,'/');

      if(Ptr)
      {
        *Ptr=0;
        if (Opt.CreateUppercaseFolders && !IsCaseMixed(NewPath))
          LocalStrupr(NewPath);

        DWORD Attr=GetFileAttributes(NewPath);
        if (Attr==INVALID_FILE_ATTRIBUTES)
        {
          if (CreateDirectory(NewPath,NULL))
            TreeList::AddTreeName(NewPath);
          else
            CreatePath(NewPath);
        }
        else if ((Attr & FILE_ATTRIBUTE_DIRECTORY)==0)
        {
          Message(MSG_DOWN|MSG_WARNING,1,MSG(MError),MSG(MCopyCannotCreateFolder),NewPath,MSG(MOk));
          _LOGCOPYR(SysLog("return COPY_FAILURE -> %d (CannotCreateFolder)",__LINE__));
          return COPY_FAILURE;
        }
      }
    }
    DestAttr=GetFileAttributes(Dest);
  }


  // Выставим признак "Тот же диск"
  int SameDisk=FALSE;
  if (ShellCopy::Flags&FCOPY_MOVE)
  {
    char SrcDir[NM];
    SrcPanel->GetCurDir(SrcDir);
    SameDisk=IsSameDisk(SrcDir,Dest);
  }

  // Основной цикл копирования одной порции.
  SrcPanel->GetSelName(NULL,FileAttr);
  {
  _LOGCOPYR(CleverSysLog Clev("Run process copy"));
  while (SrcPanel->GetSelName(SelName,FileAttr,SelShortName))
  {
    _LOGCOPYR(SysLog("SelName='%s', FileAttr=0x%08X, SelShortName='%s'",SelName,FileAttr,SelShortName));
    if (!(ShellCopy::Flags&FCOPY_COPYTONUL))
    {
      char FullDest[NM];
      xstrncpy(FullDest,Dest,sizeof(FullDest)-1);
      if(strpbrk(Dest,"*?")!=NULL)
        ShellCopyConvertWildcards(SelName,FullDest);
      DestAttr=GetFileAttributes(FullDest);
      // получим данные о месте назначения
      if (*DestDriveRoot==0)
      {
        GetPathRoot(FullDest,DestDriveRoot);
        DestDriveType=FAR_GetDriveType(strchr(FullDest,'\\')!=NULL ? DestDriveRoot:NULL);
        if(GetFileAttributes(DestDriveRoot) != -1)
          if(!GetVolumeInformation(DestDriveRoot,NULL,0,NULL,NULL,&DestFSFlags,DestFSName,sizeof(DestFSName)))
            *DestFSName=0;
        _LOGCOPYR(SysLog("DestDriveRoot='%s', DestDriveType=%d, DestFSFlags=0x%08X, DestFSName='%s'",DestDriveRoot,DestDriveType,DestFSFlags,DestFSName));
      }
    }

    char DestPath[NM];
    xstrncpy(DestPath,Dest,sizeof(DestPath)-1);
    WIN32_FIND_DATA SrcData;
    int CopyCode=COPY_SUCCESS,KeepPathPos;

    ShellCopy::Flags&=~FCOPY_OVERWRITENEXT;

    if (*SrcDriveRoot==0 || LocalStrnicmp(SelName,SrcDriveRoot,(int)strlen(SrcDriveRoot))!=0)
    {
      GetPathRoot(SelName,SrcDriveRoot);
      SrcDriveType=FAR_GetDriveType(strchr(SelName,'\\')!=NULL ? SrcDriveRoot:NULL);
      if(GetFileAttributes(SrcDriveRoot) != -1)
        if(!GetVolumeInformation(SrcDriveRoot,NULL,0,NULL,NULL,&SrcFSFlags,SrcFSName,sizeof(SrcFSName)))
          *SrcFSName=0;
      _LOGCOPYR(SysLog("SrcDriveRoot='%s', SrcDriveType=%d, SrcFSFlags=0x%08X, SrcFSName='%s'",SrcDriveRoot,SrcDriveType,SrcFSFlags,SrcFSName));
    }

    if (FileAttr & FILE_ATTRIBUTE_DIRECTORY)
      SelectedFolderNameLength=(int)strlen(SelName);
    else
      SelectedFolderNameLength=0;

    // "замочим" к едрене фени симлинк - копируем полный контент, независимо от опции
    // (но не для случая переименования линка по сети)
    if((DestDriveType == DRIVE_REMOTE || SrcDriveType == DRIVE_REMOTE) && stricmp(SrcDriveRoot,DestDriveRoot))
      ShellCopy::Flags|=FCOPY_COPYSYMLINKCONTENTS;

    KeepPathPos=(int)(PointToName(SelName)-SelName);
    _LOGCOPYR(SysLog("%d KeepPathPos=%d",__LINE__,KeepPathPos));
    if(!stricmp(SrcDriveRoot,SelName) && (RPT==RP_JUNCTION || RPT==RP_SYMLINKDIR)) // но сначала посмотрим на "это корень диска?"
    {
      _LOGCOPYR(SysLog("%d if(!stricmp(SrcDriveRoot,SelName) &&...",__LINE__));
      SrcData.dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;
      //// // _SVS(SysLog("Dest='%s'",Dest));
    }
    else
    {
      // проверка на вшивость ;-)
      if(!GetFileWin32FindData(SelName,&SrcData))
      {
        strcat(DestPath,SelName);
        ShellCopy::ShellCopyMsg(SelName,DestPath,MSG_LEFTALIGN|MSG_KEEPBACKGROUND);
        if (Message(MSG_DOWN|MSG_WARNING,2,MSG(MError),MSG(MCopyCannotFind),
                SelName,MSG(MSkip),MSG(MCancel))==1)
        {
          _LOGCOPYR(SysLog("return COPY_FAILURE -> MCopyCannotFind"));
          return COPY_FAILURE;
        }
        /* $ 23.03.2002 VVM
          ! Уберем это, т.к. состояние SrcData неизвестно */
  //      unsigned __int64 SubSize=MKUINT64(SrcData.nFileSizeHigh,SrcData.nFileSizeLow);
  //      TotalCopySize-=SubSize;
        /* VVM $ */
        _LOGCOPYR(SysLog("%d continue;",__LINE__));
        continue;
      }
    }

    // Если это каталог и трэба создать связь...
    if(RPT==RP_SYMLINKFILE || ((SrcData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (RPT==RP_JUNCTION || RPT==RP_SYMLINKDIR)))
    {
      /*
      ЭТОТ кусок, если хотим не делать ссылку на ссылку!
      char SrcRealName[NM*2];
      ConvertNameToReal(SelName,SrcRealName,sizeof(SrcRealName));
      switch(MkSymLink(SrcRealName,Dest,ShellCopy::Flags))
      */
      switch(MkSymLink(SelName,Dest,RPT,ShellCopy::Flags))
      {
        case 2:
          break;
        case 1:
            // Отметим (Ins) несколько каталогов, ALT-F6 Enter - выделение с папок не снялось.
            if ((!(ShellCopy::Flags&FCOPY_CURRENTONLY)) && (ShellCopy::Flags&FCOPY_COPYLASTTIME))
              SrcPanel->ClearLastGetSelection();
            _LOGCOPYR(SysLog("%d continue;",__LINE__));
            continue;
        case 0:
          _LOGCOPYR(SysLog("return COPY_FAILURE -> %d",__LINE__));
          return COPY_FAILURE;
      }
    }

    //KeepPathPos=PointToName(SelName)-SelName;

    // Мувим?
    if ((ShellCopy::Flags&FCOPY_MOVE))
    {
      // Тыкс, а как на счет "тот же диск"?
      if (KeepPathPos && PointToName(Dest)==Dest)
      {
        strcpy(DestPath,SelName);
        strcpy(DestPath+KeepPathPos,Dest);
        SameDisk=TRUE;
      }

      if ((UseFilter ||  !SameDisk) || (SrcData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT) && (ShellCopy::Flags&FCOPY_COPYSYMLINKCONTENTS))
        CopyCode=COPY_FAILURE;
      else
      {
        CopyCode=ShellCopyOneFile(SelName,SrcData,DestPath,KeepPathPos,1);
        if (CopyCode==COPY_SUCCESS_MOVE)
        {
          if (*DestDizPath)
          {
            if (*RenamedName)
            {
              DestDiz.DeleteDiz(SelName,SelShortName);
              SrcPanel->CopyDiz(SelName,SelShortName,RenamedName,RenamedName,&DestDiz);
            }
            else
            {
              if (*CopiedName==0)
                strcpy(CopiedName,SelName);
              SrcPanel->CopyDiz(SelName,SelShortName,CopiedName,CopiedName,&DestDiz);
              SrcPanel->DeleteDiz(SelName,SelShortName);
            }
          }
          continue;
        }

        if (CopyCode==COPY_CANCEL)
        {
          _LOGCOPYR(SysLog("return COPY_CANCEL -> %d",__LINE__));
          return COPY_CANCEL;
        }

        if (CopyCode==COPY_NEXT)
        {
          unsigned __int64 CurSize=MKUINT64(SrcData.nFileSizeHigh,SrcData.nFileSizeLow);
          TotalCopiedSize = TotalCopiedSize - CurCopiedSize + CurSize;
          TotalSkippedSize = TotalSkippedSize + CurSize - CurCopiedSize;
//          TotalCopySize-=SubSize;
          continue;
        }

        if (!(ShellCopy::Flags&FCOPY_MOVE) || CopyCode==COPY_FAILURE)
          ShellCopy::Flags|=FCOPY_OVERWRITENEXT;
      }
    }

    if (!(ShellCopy::Flags&FCOPY_MOVE) || CopyCode==COPY_FAILURE)
    {
      _LOGCOPYR(SysLog("%d Call ShellCopyOneFile('%s',%p,'%s',%d,0)",__LINE__,NullToEmpty(SelName),&SrcData,NullToEmpty(Dest),KeepPathPos));
      CopyCode=ShellCopyOneFile(SelName,SrcData,Dest,KeepPathPos,0);
      _LOGCOPYR(SysLog("return %d",CopyCode));
      ShellCopy::Flags&=~FCOPY_OVERWRITENEXT;

      if (CopyCode==COPY_CANCEL)
      {
        _LOGCOPYR(SysLog("%d CopyCode==COPY_CANCEL",__LINE__));
        return COPY_CANCEL;
      }

      if (CopyCode!=COPY_SUCCESS)
      {
        unsigned __int64 CurSize=MKUINT64(SrcData.nFileSizeHigh,SrcData.nFileSizeLow);
        if (CopyCode != COPY_NOFILTER) //????
          TotalCopiedSize = TotalCopiedSize - CurCopiedSize + CurSize;
        if (CopyCode == COPY_NEXT)
          TotalSkippedSize = TotalSkippedSize + CurSize - CurCopiedSize;
        continue;
      }
    }

    if (CopyCode==COPY_SUCCESS && !(ShellCopy::Flags&FCOPY_COPYTONUL) && *DestDizPath)
    {
      if (*CopiedName==0)
        strcpy(CopiedName,SelName);
      SrcPanel->CopyDiz(SelName,SelShortName,CopiedName,CopiedName,&DestDiz);
    }
#if 0
    // Если [ ] Copy contents of symbolic links
    if((SrcData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT) && !(ShellCopy::Flags&FCOPY_COPYSYMLINKCONTENTS))
    {
      //создать симлинк
      switch(MkSymLink(SelName,Dest,FCOPY_LINK/*|FCOPY_NOSHOWMSGLINK*/))
      {
        case 2:
          break;
        case 1:
            // Отметим (Ins) несколько каталогов, ALT-F6 Enter - выделение с папок не снялось.
            if ((!(ShellCopy::Flags&FCOPY_CURRENTONLY)) && (ShellCopy::Flags&FCOPY_COPYLASTTIME))
              SrcPanel->ClearLastGetSelection();
            _LOGCOPYR(SysLog("%d continue;",__LINE__));
            continue;
        case 0:
          _LOGCOPYR(SysLog("return COPY_FAILURE -> %d",__LINE__));
          return COPY_FAILURE;
      }
      continue;
    }
#endif

    // Mantis#44 - Потеря данных при копировании ссылок на папки
    // если каталог (или нужно копировать симлинк) - придется рекурсивно спускаться...
    if (RPT!=RP_SYMLINKFILE && (SrcData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
        (
          !(SrcData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT) ||
          (SrcData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT) && (ShellCopy::Flags&FCOPY_COPYSYMLINKCONTENTS)
        )
       )
    {
      _LOGCOPYR(CleverSysLog Clev("if(SrcData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)"));

      // Признак, по которому в дальнейшем узнаем: было копирование одного пустого
      // каталога при включенном фильтре или было копирование каталога с содержимым.
      int TryToCopyTree=FALSE,FilesInDir=0;

      int SubCopyCode;
      char SubName[NM],FullName[NM];
      ScanTree ScTree(TRUE,TRUE,ShellCopy::Flags&FCOPY_COPYSYMLINKCONTENTS);

      strcpy(SubName,SelName);
      strcat(SubName,"\\");
      if (DestAttr==(DWORD)-1)
        KeepPathPos=(int)strlen(SubName);

      int NeedRename=!((SrcData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT) && (ShellCopy::Flags&FCOPY_COPYSYMLINKCONTENTS) && (ShellCopy::Flags&FCOPY_MOVE));
      int AttemptToMove=FALSE;
      int SaveOvrMode=OvrMode;

      ScTree.SetFindPath(SubName,"*.*",FSCANTREE_FILESFIRST);

      while (ScTree.GetNextName(&SrcData,FullName, sizeof (FullName)-1))
      {
        _LOGCOPYR(SysLog("FullName='%s', SameDisk=%d",FullName,SameDisk));

        // Была попытка скопировать каталог с содержимым при включенном фильтре
        TryToCopyTree=TRUE;

        /* 23.04.2005 KM
           Находясь в фильтре каталоги не копируем, ибо скопируешь его, а файл
           из-за действия фильтра не скопируется и получится, что у нас останется
           пустой и никому не нужный каталог. А создавать каталоги будем в ShellCopyOneFile,
           вырезанием пути из имени попавшего в фильтр файла.
        */
        if (UseFilter && (SrcData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
          // Просто пропустить каталог недостаточно - если каталог помечен в
          // фильтре как некопируемый, то следует пропускать и его и всё его
          // содержимое.
          if (!Filter->FileInFilter((WIN32_FIND_DATA *) &SrcData))
          {
            ScTree.SkipDir();
            continue;
          }
          else
          {
            // Из-за этих пропусков при Move полностью скопированный каталог не
            // удаляется обычным методом, а пустые каталоги не копируются даже
            // в случае когда фильтр это разрешает
            if (!(ShellCopy::Flags&FCOPY_MOVE) || (!UseFilter && SameDisk) || !ScTree.IsDirSearchDone())
              continue;
            if(FilesInDir) goto remove_moved_directory;
          }
        }
        /* KM $ */

        AttemptToMove=FALSE;
        if ((ShellCopy::Flags&FCOPY_MOVE) && (!UseFilter && SameDisk) && (SrcData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0)
        {
          AttemptToMove=TRUE;

          switch(ShellCopyOneFile(FullName,SrcData,Dest,KeepPathPos,NeedRename)) // 1
          {
            case COPY_CANCEL:
              _LOGCOPYR(SysLog("return COPY_CANCEL -> %d",__LINE__));
              return COPY_CANCEL;

            case COPY_NEXT:
            {
              unsigned __int64 CurSize=MKUINT64(SrcData.nFileSizeHigh,SrcData.nFileSizeLow);
              TotalCopiedSize = TotalCopiedSize - CurCopiedSize + CurSize;
              TotalSkippedSize = TotalSkippedSize + CurSize - CurCopiedSize;
              continue;
            }

            case COPY_SUCCESS_MOVE:
            {
              FilesInDir++;
              continue;
            }

            case COPY_SUCCESS:
              if(!NeedRename) // вариант при перемещении содержимого симлика с опцией "копировать содержимое сим..."
              {
                unsigned __int64 CurSize=MKUINT64(SrcData.nFileSizeHigh,SrcData.nFileSizeLow);
                TotalCopiedSize = TotalCopiedSize - CurCopiedSize + CurSize;
                TotalSkippedSize = TotalSkippedSize + CurSize - CurCopiedSize;
                FilesInDir++;
                continue;     // ...  т.к. мы ЭТО не мувили, а скопировали, то все, на этом закончим бадаться с этим файлов
              }
          }
        }

        SaveOvrMode=OvrMode;

        if (AttemptToMove)
          OvrMode=1;

        _LOGCOPYR(SysLog("%d Call ShellCopyOneFile('%s',%p,'%s',%d,0)",__LINE__,FullName,&SrcData,Dest,KeepPathPos));
        SubCopyCode=ShellCopyOneFile(FullName,SrcData,Dest,KeepPathPos,0);
        _LOGCOPYR(SysLog("SubCopyCode=%d",SubCopyCode));

        if (AttemptToMove)
          OvrMode=SaveOvrMode;

        if (SubCopyCode==COPY_CANCEL)
        {
          _LOGCOPYR(SysLog("return COPY_CANCEL -> %d",__LINE__));
          return COPY_CANCEL;
        }

        if (SubCopyCode==COPY_NEXT)
        {
          unsigned __int64 CurSize=MKUINT64(SrcData.nFileSizeHigh,SrcData.nFileSizeLow);
          TotalCopiedSize = TotalCopiedSize - CurCopiedSize + CurSize;
          TotalSkippedSize = TotalSkippedSize + CurSize - CurCopiedSize;
        }

        if (SubCopyCode==COPY_SUCCESS)
        {
          if(ShellCopy::Flags&FCOPY_MOVE)
          {
            if (SrcData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
              _LOGCOPYR(SysLog("************* %d (%s) ******************",__LINE__,FullName));
              if (ScTree.IsDirSearchDone() ||
                  ((SrcData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && !(ShellCopy::Flags&FCOPY_COPYSYMLINKCONTENTS)))
              {
                if (SrcData.dwFileAttributes & FA_RDONLY)
                  SetFileAttributes(FullName,FILE_ATTRIBUTE_NORMAL);
remove_moved_directory:
                _LOGCOPYR(SysLog("************* %d (%s) Pred FAR_RemoveDirectory ******************",__LINE__,FullName));
                if (FAR_RemoveDirectory(FullName))
                  TreeList::DelTreeName(FullName);
                _LOGCOPYR(else SysLog("************* %d (%s) ******************",__LINE__,FullName));
              }
            }
            // здесь нужны проверка на FSCANTREE_INSIDEJUNCTION, иначе
            // при мовинге будет удаление файла, что крайне неправильно!
            else if(!ScTree.InsideJunction())
            {
              if (DeleteAfterMove(FullName,SrcData.dwFileAttributes)==COPY_CANCEL)
              {
                _LOGCOPYR(SysLog("return COPY_CANCEL -> %d DeleteAfterMove('%s',0x%08X) == COPY_CANCEL",__LINE__,FullName,SrcData.dwFileAttributes));
                return COPY_CANCEL;
              }
            }
          }

          FilesInDir++;
        }
      }

      if ((ShellCopy::Flags&FCOPY_MOVE) && CopyCode==COPY_SUCCESS)
      {
        if (FileAttr & FA_RDONLY)
          SetFileAttributes(SelName,FILE_ATTRIBUTE_NORMAL);

        if (FAR_RemoveDirectory(SelName))
        {
          TreeList::DelTreeName(SelName);

          if (*DestDizPath)
            SrcPanel->DeleteDiz(SelName,SelShortName);
        }
      }

      /* $ 23.04.2005 KM
         Была проведена попытка копирования каталога с содержимым при
         включенном фильтре, но не было скопировано ни одного файла,
         поэтому удалим SelName в каталоге-приёмнике.
      */
      if (UseFilter && TryToCopyTree && !FilesInDir)
      {
        strcat(DestPath,SelName);
        FAR_RemoveDirectory(DestPath);
      }
      /* KM $ */
    }
    else if ((ShellCopy::Flags&FCOPY_MOVE) && CopyCode==COPY_SUCCESS)
    {
      int DeleteCode;
      if ((DeleteCode=DeleteAfterMove(SelName,FileAttr))==COPY_CANCEL)
      {
        _LOGCOPYR(SysLog("return COPY_CANCEL -> %d DeleteAfterMove('%s',0x%08X) == COPY_CANCEL",__LINE__,SelName,FileAttr));
        return COPY_CANCEL;
      }

      if (DeleteCode==COPY_SUCCESS && *DestDizPath)
        SrcPanel->DeleteDiz(SelName,SelShortName);
    }

    if ((!(ShellCopy::Flags&FCOPY_CURRENTONLY)) && (ShellCopy::Flags&FCOPY_COPYLASTTIME))
    {
      SrcPanel->ClearLastGetSelection();
    }
  }
  }

  _tran(SysLog("[%p] ShellCopy::CopyFileTree() end",this));
  return COPY_SUCCESS; //COPY_SUCCESS_MOVE???
}

COPY_CODES ShellCopy::ShellCopyOneFile(const char *Src,
                                       const WIN32_FIND_DATA &SrcData,
                                       const char *Dest, int KeepPathPos,
                                       int Rename)
{
  _LOGCOPYR(CleverSysLog Clev("ShellCopy::ShellCopyOneFile()"));
  _LOGCOPYR(SysLog("Params: Src='%s', Dest='%s', KeepPathPos=%d, Rename=%d",Src, Dest,KeepPathPos,Rename));
  char DestPath[2*NM];
  DWORD DestAttr=(DWORD)-1;
  WIN32_FIND_DATA DestData={0};
  /* $ 25.05.2002 IS
     + RenameToShortName - дополняет SameName и становится больше нуля тогда,
       когда объект переименовывается в его же _короткое_ имя.
  */
  int SameName=0,
      RenameToShortName=0,
      Append=0;
  /* IS $ */

  *RenamedName=*CopiedName=0;

  CurCopiedSize = 0; // Сбросить текущий прогресс

  int IsSetSecuty=FALSE;

  if (CheckForEscSilent())
  {
    if (ConfirmAbortOp())
    {
      _LOGCOPYR(SysLog("return COPY_CANCEL -> %d Pressed ESC",__LINE__));
      return(COPY_CANCEL);
    }
  }

  if (UseFilter)
  {
    if (!Filter->FileInFilter((WIN32_FIND_DATA *) &SrcData))
      return COPY_NOFILTER;
  }

  xstrncpy(DestPath,Dest,sizeof(DestPath)-1);

  ShellCopyConvertWildcards(Src,DestPath);

  char *NamePtr=PointToName(DestPath);

  DestAttr=(DWORD)-1;

  if (DestPath[0]=='\\' && DestPath[1]=='\\')
  {
    char Root[NM];
    GetPathRoot(DestPath,Root);
    int RootLength=(int)strlen(Root);

    if (RootLength>0 && Root[RootLength-1]=='\\')
      Root[RootLength-1]=0;

    if (strcmp(DestPath,Root)==0)
      DestAttr=FILE_ATTRIBUTE_DIRECTORY;
  }

  _LOGCOPYR(SysLog("NamePtr='%s', TestParentFolderName()=>%d",NamePtr,TestParentFolderName(NamePtr)));
  if (*NamePtr==0 || TestParentFolderName(NamePtr))
    DestAttr=FILE_ATTRIBUTE_DIRECTORY;

  *DestData.cFileName=0;

  if (DestAttr==(DWORD)-1)
  {
    GetFileWin32FindData(DestPath,&DestData,false);
    DestAttr=DestData.dwFileAttributes;
  }
  _LOGCOPYR(SysLog("%d DestAttr=0x%08X",__LINE__,DestAttr));

  if (DestAttr!=(DWORD)-1 && (DestAttr & FILE_ATTRIBUTE_DIRECTORY))
  {
    int CmpCode=CmpFullNames(Src,DestPath);
    if(CmpCode==1) // TODO: error check
    {
      _LOGCOPYR(SysLog("%d CmpCode=%d, CmpFullNames('%s','%s')",__LINE__,CmpCode,Src,DestPath));
      SameName=1;
      /* $ 25.05.2002 IS
         Проверим ту ситуацию, когда переименовывается _каталог_ в свое же
         _короткое_ имя
      */
      if(Rename)
      {
         CmpCode=!strcmp(PointToName(Src),PointToName(DestPath));
         if(!CmpCode)
           RenameToShortName = !LocalStricmp(DestPath,SrcData.cAlternateFileName);
      }
      /* IS $ */
      if (CmpCode==1)
      {
        SetMessageHelp("ErrCopyItSelf");
        Message(MSG_DOWN|MSG_WARNING,1,MSG(MError),MSG(MCannotCopyFolderToItself1),
                Src,MSG(MCannotCopyFolderToItself2),MSG(MOk));
        _LOGCOPYR(SysLog("return COPY_CANCEL -> %d",__LINE__));
        return(COPY_CANCEL);
      }
    }
    _LOGCOPYR(else SysLog("%d CmpFullNames() == 0",__LINE__));

    if (!SameName)
    {
      int Length=(int)strlen(DestPath);

      if (DestPath[Length-1]!='\\' && DestPath[Length-1]!=':')
        strcat(DestPath,"\\");

      const char *PathPtr=Src+KeepPathPos;

      if (*PathPtr && KeepPathPos==0 && PathPtr[1]==':')
        PathPtr+=2;

      if (*PathPtr=='\\')
        PathPtr++;

      /* $ 23.04.2005 KM
          Поскольку находясь в фильтре приходится пропускать копирование каталогов,
          чтобы не плодить пустые каталоги из-за непопавших в фильтр файлов,
          то создание каталога будем производить при копировании файла, попавшего
          в фильтр, а для этого возьмём атрибуты копируемого каталога и установим
          их на создаваемый каталог.
      */
      if (UseFilter)
      {
        char OldPath[2*NM],NewPath[2*NM];
        const char *path=PathPtr,*p1=NULL;

        while (p1=strchr(path,'\\'))
        {
          DWORD FileAttr=(DWORD)-1;
          WIN32_FIND_DATA FileData={0};

          xstrncpy(OldPath,Src,p1-Src);

          GetFileWin32FindData(OldPath,&FileData);
          FileAttr=FileData.dwFileAttributes;
          #if 0
          // Поищем старый каталог и возьмём его атрибуты
          if (!GetFileWin32FindData(OldPath,&FileData)))
            // Однако это не нормально, не нашли каталог, который уже скопировали
            // (этот код по хорошему и не должен срабатывать)
            FileAttr=(DWORD)-1;
          else
          {
            // Есть файл, а также его атрибуты
            FileAttr=FileData.dwFileAttributes;
          }
          #endif

          // Создадим имя каталога, который теперь нужно создать, если конечно его ещё нет
          xstrncpy(NewPath,DestPath,sizeof(NewPath)-1);
          xstrncpy(NewPath+strlen(DestPath),PathPtr,p1-PathPtr);

          // Такого каталога ещё нет, создадим его
          if (!GetFileWin32FindData(NewPath,&FileData))
          {
            int CopySecurity = ShellCopy::Flags&FCOPY_COPYSECURITY;
            SECURITY_ATTRIBUTES sa;

            if ((CopySecurity) && !GetSecurity(OldPath,sa))
              CopySecurity = FALSE;

            // Собственно создание каталога
            if (CreateDirectory(NewPath,CopySecurity?&sa:NULL))
            {
              // Нормально, создали каталог
              if (FileAttr!=(DWORD)-1)
                // Теперь установим атрибуты. Взаимоисключающие атрибуты не проверяем
                // ибо эти атрибуты "живые", то есть получены у реального каталога
                ShellSetAttr(NewPath,FileAttr);
            }
            else
            {
              // Ай-ай-ай. Каталог не смогли создать! Значит будем ругаться!
              int MsgCode;
              MsgCode=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,3,MSG(MError),
                              MSG(MCopyCannotCreateFolder),NewPath,MSG(MCopyRetry),
                              MSG(MCopySkip),MSG(MCopyCancel));

              if (MsgCode!=0)
              {
                // Ну что ж, раз дали отмену или пропуск создания каталога, выходим отсюда
                _LOGCOPYR(SysLog("return %d -> %d",((MsgCode==-2 || MsgCode==2) ? COPY_CANCEL:COPY_NEXT),__LINE__));
                return((MsgCode==-2 || MsgCode==2) ? COPY_CANCEL:COPY_NEXT);
              }

              // Сказали хотим продолжить попытку создания каталога. Ну тогда в добрый путь
              continue;
            }
          }

          // Мы стоим на обратном слэше
          if (*p1=='\\')
            p1++;

          // Возьмём следующий адрес в имени каталога за обратным слэшем,
          // для того чтобы проверить и, возможно, создать следующий каталог,
          // находящийся в копируемом имени файла.
          path=p1;
        }
      }
      /* KM $ */

      strcat(DestPath,PathPtr);

      if(!GetFileWin32FindData(DestPath,&DestData))
        DestAttr=(DWORD)-1;
      else
        DestAttr=DestData.dwFileAttributes;
      _LOGCOPYR(SysLog("%d DestPath='%s', DestAttr=0x%08X",__LINE__,DestPath,DestAttr));
    }
  }

  /* $ 25.05.2002 IS
     Забыли в проверке считать nul==con, вместо того, чтобы дописать в условие
     проверку на con, напишу просто проверку на FCOPY_COPYTONUL
  */
  if (!(ShellCopy::Flags&FCOPY_COPYTONUL) && stricmp(DestPath,"prn")!=0)
  /* IS $ */
    SetDestDizPath(DestPath);

  ShellCopyMsg(Src,DestPath,MSG_LEFTALIGN|MSG_KEEPBACKGROUND);

  if(!(ShellCopy::Flags&FCOPY_COPYTONUL))
  {
    // проверка очередного монстрика на потоки
    switch(CheckStreams(Src,DestPath))
    {
      case COPY_NEXT:
        _LOGCOPYR(SysLog("return COPY_NEXT -> %d CheckStreams('%s','%s')",__LINE__,Src,DestPath));
        return COPY_NEXT;
      case COPY_CANCEL:
        _LOGCOPYR(SysLog("return COPY_CANCEL -> %d CheckStreams('%s','%s')",__LINE__,Src,DestPath));
        return COPY_CANCEL;
    }

    if(SrcData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY ||
      (SrcData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT && RPT==RP_EXACTCOPY && !(ShellCopy::Flags&FCOPY_COPYSYMLINKCONTENTS)))
    {
      _LOGCOPYR(CleverSysLog Clev("if (SrcData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)"));
      if (!Rename)
        strcpy(CopiedName,PointToName(DestPath));

      /* $ 25.05.2002 IS
         Не выполняем лиших операций, когда RenameToShortName.
      */
      if (DestAttr!=(DWORD)-1 && !RenameToShortName)
      /* IS $ */
      {
        if ((DestAttr & FILE_ATTRIBUTE_DIRECTORY) && !SameName)
        {
          DWORD SetAttr=SrcData.dwFileAttributes;
          if (IsDriveTypeCDROM(SrcDriveType) && Opt.ClearReadOnly && (SetAttr & FA_RDONLY))
            SetAttr&=~FA_RDONLY;

          _LOGCOPYR(SysLog("%d SetAttr=0x%08X, DestAttr=0x%08X",__LINE__,SetAttr,DestAttr));

          if (SetAttr!=DestAttr)
            ShellSetAttr(DestPath,SetAttr);

          char SrcFullName[NM];
          if (ConvertNameToFull(Src,SrcFullName, sizeof(SrcFullName)) >= sizeof(SrcFullName))
          {
            _LOGCOPYR(SysLog("return COPY_NEXT -> %d ConvertNameToFull()-but",__LINE__));
            return(COPY_NEXT);
          }
          _LOGCOPYR(SysLog("%d SrcFullName='%s', DestPath='%s', return ??;",__LINE__,SrcFullName,DestPath));
          return(strcmp(DestPath,SrcFullName)==0 ? COPY_NEXT:COPY_SUCCESS);
        }

        int Type=GetFileTypeByName(DestPath);
        if (Type==FILE_TYPE_CHAR || Type==FILE_TYPE_PIPE)
        {
          _LOGCOPYR(SysLog("return %d -> %d if (Type==FILE_TYPE_CHAR || Type==FILE_TYPE_PIPE)",(Rename ? COPY_NEXT:COPY_SUCCESS),__LINE__));
          return(Rename ? COPY_NEXT:COPY_SUCCESS);
        }
      }

      if (Rename)
      {
        _LOGCOPYR(SysLog("%d Rename",__LINE__));
        char SrcFullName[NM*2],DestFullName[NM*2];

        if (ConvertNameToFull(Src,SrcFullName, sizeof(SrcFullName)) >= sizeof(SrcFullName))
        {
          _LOGCOPYR(SysLog("return COPY_NEXT -> %d ConvertNameToFull()-but",__LINE__));
          return(COPY_NEXT);
        }

        SECURITY_ATTRIBUTES sa;
        _LOGCOPYR(ConvertNameToFull(Dest,DestFullName, sizeof(DestFullName)));
        _LOGCOPYR(SysLog("%d call GetSecurity ('%s')",__LINE__,Dest));
        _LOGCOPYR(SysLog("%d DestFullName='%s'",__LINE__,DestFullName));
        _LOGCOPYR(SysLog("%d SrcFullName ='%s'",__LINE__,SrcFullName));
        //_LOGCOPYR(SysLog("%d CmpFullPath => %d",__LINE__,CmpFullPath(SrcFullName,DestFullName)));
        _LOGCOPYR(SysLog("%d CmpFullPath => %d",__LINE__,CmpFullPath(Src,Dest)));

        // для Move нам необходимо узнать каталог родитель, чтобы получить его секьюрити
        if (!(ShellCopy::Flags&(FCOPY_COPYSECURITY|FCOPY_LEAVESECURITY)))
        {
          IsSetSecuty=FALSE;
          if(CmpFullPath(Src,Dest)) // в пределах одного каталога ничего не меняем
            IsSetSecuty=FALSE;
          else if(GetFileAttributes(Dest) == (DWORD)-1) // если каталога нет...
          {
            // ...получаем секьюрити родителя
            if(GetSecurity(GetParentFolder(Dest,DestFullName,sizeof(DestFullName)-1),sa))
              IsSetSecuty=TRUE;
          }
          else if(GetSecurity(Dest,sa)) // иначе получаем секьюрити Dest`а
            IsSetSecuty=TRUE;
        }

        /* $ 18.07.2001 VVM
          + Пытаемся переименовать, пока не отменят */
        while (1)
        {
          // $ 25.05.2002 IS Отдельная обработка RenameToShortName для каталога.
          _LOGCOPYR(SysLog("%d Move '%s' => '%s'",__LINE__,Src,DestPath));
          BOOL SuccessMove=RenameToShortName?MoveFileThroughTemp(Src,DestPath):FAR_MoveFile(Src,DestPath);
          _LOGCOPYR(SysLog("%d SuccessMove=%d",__LINE__,SuccessMove));

          if (SuccessMove)
          {
            if(IsSetSecuty)// && WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT && !strcmp(DestFSName,"NTFS"))
              //if(!(ShellCopy::Flags&FCOPY_CURRENTONLY))
                SetRecursiveSecurity(DestPath,sa);

            if (PointToName(DestPath)==DestPath)
              strcpy(RenamedName,DestPath);
            else
              strcpy(CopiedName,PointToName(DestPath));
//          ConvertNameToFull(DestPath,DestFullName, sizeof(DestFullName));

            if (ConvertNameToFull(Dest,DestFullName, sizeof(DestFullName)) >= sizeof(DestFullName))
            {
              _LOGCOPYR(SysLog("return COPY_NEXT -> %d ConvertNameToFull()-but",__LINE__));
              return(COPY_NEXT);
            }

            TreeList::RenTreeName(SrcFullName,DestFullName);
            _LOGCOPYR(SysLog("return %d -> %d",(SameName ? COPY_NEXT:COPY_SUCCESS_MOVE),__LINE__));
            return(SameName ? COPY_NEXT:COPY_SUCCESS_MOVE);
          }
          else // $ 18.07.2001 VVM
          {    //  Спросить, что делать, если не смогли переименовать каталог
            switch (Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,3,MSG(MError),
                            MSG(MCopyCannotRenameFolder),Src,MSG(MCopyRetry),
                            MSG(MCopyIgnore),MSG(MCopyCancel)))
            {
              case 0:  continue;
              case 1:
              {
                // если сказали "ignore" - попытаемся создать пустой и перенести туда содержимое
                int CopySecurity = ShellCopy::Flags&FCOPY_COPYSECURITY;
                SECURITY_ATTRIBUTES sa;
                if ((CopySecurity) && !GetSecurity(Src,sa))
                  CopySecurity = FALSE;
                if (CreateDirectory(DestPath,CopySecurity?&sa:NULL))
                {
                  if (PointToName(DestPath)==DestPath)
                    strcpy(RenamedName,DestPath);
                  else
                    strcpy(CopiedName,PointToName(DestPath));
//                  if (ConvertNameToFull(Dest,DestFullName, sizeof(DestFullName)) >= sizeof(DestFullName))
//                    return(COPY_NEXT);
//                  TreeList::RenTreeName(SrcFullName,DestFullName);
#if 0
                  // для источника, имеющего суть симлинка - создадим симлинк
                  if(SrcData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
                  {
                    char SrcRealName[NM*2];
                    ConvertNameToReal(Src,SrcRealName,sizeof(SrcRealName));
                    switch(MkSymLink(SrcRealName,DestPath,FCOPY_LINK))
                    {
                      case 2:
                        _LOGCOPYR(SysLog("return COPY_CANCEL -> %d",__LINE__));
                        return COPY_CANCEL;
                      case 1: break;
                      case 0:
                        _LOGCOPYR(SysLog("return COPY_FAILURE -> %d",__LINE__));
                        return COPY_FAILURE;
                    }
                  }
#endif
                  TreeList::AddTreeName(DestPath);
                  _LOGCOPYR(SysLog("return COPY_SUCCESS -> %d",__LINE__));
                  return(COPY_SUCCESS);
                }
              }
              default:
                _LOGCOPYR(SysLog("return COPY_CANCEL -> %d",__LINE__));
                return (COPY_CANCEL);
            } /* switch */
          } /* else */
        } /* while */
        /* VVM $ */
      }

      SECURITY_ATTRIBUTES sa;
      if ((ShellCopy::Flags&FCOPY_COPYSECURITY) && !GetSecurity(Src,sa))
      {
        _LOGCOPYR(SysLog("return COPY_CANCEL -> %d GetSecurity()==0",__LINE__));
        return COPY_CANCEL;
      }

      if(RPT!=RP_SYMLINKFILE && SrcData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY){
      while (!CreateDirectory(DestPath,(ShellCopy::Flags&FCOPY_COPYSECURITY) ? &sa:NULL))
      {
        int MsgCode;
        MsgCode=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,3,MSG(MError),
                        MSG(MCopyCannotCreateFolder),DestPath,MSG(MCopyRetry),
                        MSG(MCopySkip),MSG(MCopyCancel));

        if (MsgCode!=0)
        {
          _LOGCOPYR(SysLog("return %d -> %d",((MsgCode==-2 || MsgCode==2) ? COPY_CANCEL:COPY_NEXT),__LINE__));
          return((MsgCode==-2 || MsgCode==2) ? COPY_CANCEL:COPY_NEXT);
        }
      }

      DWORD SetAttr=SrcData.dwFileAttributes;

      if (IsDriveTypeCDROM(SrcDriveType) && Opt.ClearReadOnly && (SetAttr & FA_RDONLY))
        SetAttr&=~FA_RDONLY;

      if((SetAttr & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
      {
        // не будем выставлять компрессию, если мылимся в каталог
        // с выставленным FILE_ATTRIBUTE_ENCRYPTED (а он уже будет выставлен после CreateDirectory)
        // т.с. пропускаем лишний ход.
        if(GetFileAttributes(DestPath)&FILE_ATTRIBUTE_ENCRYPTED)
          SetAttr&=~FILE_ATTRIBUTE_COMPRESSED;

        if(SetAttr&FILE_ATTRIBUTE_COMPRESSED)
        {
          while(1)
          {
            int MsgCode=ESetFileCompression(DestPath,1,0,SkipMode);
            if(MsgCode)
            {
              if(MsgCode == SETATTR_RET_SKIP)
                ShellCopy::Flags|=FCOPY_SKIPSETATTRFLD;
              else if(MsgCode == SETATTR_RET_SKIPALL)
              {
                ShellCopy::Flags|=FCOPY_SKIPSETATTRFLD;
                this->SkipMode=SETATTR_RET_SKIP;
              }
              break;
            }
            if(MsgCode != SETATTR_RET_OK)
              return (MsgCode==SETATTR_RET_SKIP || MsgCode==SETATTR_RET_SKIPALL) ? COPY_NEXT:COPY_CANCEL;
          }
        }
#if 0
        else if(SetAttr&FILE_ATTRIBUTE_ENCRYPTED)
        {
          while(1)
          {
            int MsgCode=ESetFileEncryption(DestPath,1,0);
            if(MsgCode)
            {
              if(MsgCode == 2)
                ShellCopy::Flags|=FCOPY_SKIPSETATTRFLD;
              break;
            }
            if(MsgCode != 1)
              return (MsgCode==2) ? COPY_NEXT:COPY_CANCEL;
          }
        }
#endif

        while(!ShellSetAttr(DestPath,SetAttr))
        {
          int MsgCode;
          MsgCode=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),
                          MSG(MCopyCannotChangeFolderAttr),DestPath,
                          MSG(MCopyRetry),MSG(MCopySkip),MSG(MCopySkipAll),MSG(MCopyCancel));

          if (MsgCode!=0)
          {
            if(MsgCode==1)
              break;
            if(MsgCode==2)
            {
              ShellCopy::Flags|=FCOPY_SKIPSETATTRFLD;
              break;
            }
            FAR_RemoveDirectory(DestPath);
            _LOGCOPYR(SysLog("return COPY_CANCEL -> %d ShellSetAttr('%s', 0x%08X)==0",__LINE__,DestPath,SetAttr));
            return((MsgCode==-2 || MsgCode==3) ? COPY_CANCEL:COPY_NEXT);
          }
        }
      }
      else if( !(ShellCopy::Flags & FCOPY_SKIPSETATTRFLD) && ((SetAttr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY))
      {
        while(!ShellSetAttr(DestPath,SetAttr))
        {
          int MsgCode;
          MsgCode=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),
                          MSG(MCopyCannotChangeFolderAttr),DestPath,
                          MSG(MCopyRetry),MSG(MCopySkip),MSG(MCopySkipAll),MSG(MCopyCancel));

          if (MsgCode!=0)
          {
            if(MsgCode==1)
              break;
            if(MsgCode==2)
            {
              ShellCopy::Flags|=FCOPY_SKIPSETATTRFLD;
              break;
            }
            FAR_RemoveDirectory(DestPath);
            _LOGCOPYR(SysLog("return COPY_CANCEL -> %d ShellSetAttr('%s', 0x%08X)==0",__LINE__,DestPath,SetAttr));
            return((MsgCode==-2 || MsgCode==3) ? COPY_CANCEL:COPY_NEXT);
          }
        }
      }}
#if 1
      // для источника, имеющего суть симлинка - создадим симлинк
      // Если [ ] Copy contents of symbolic links
      if(SrcData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT && !(ShellCopy::Flags&FCOPY_COPYSYMLINKCONTENTS) && RPT==RP_EXACTCOPY)
      {
        char SrcRealName[NM*2];
        ConvertNameToFull(Src,SrcRealName,sizeof(SrcRealName));
        switch(MkSymLink(SrcRealName,DestPath,RPT,0))
        {
          case 2:
            _LOGCOPYR(SysLog("return COPY_CANCEL -> %d",__LINE__));
            return COPY_CANCEL;
          case 1:
            break;
          case 0:
            _LOGCOPYR(SysLog("return COPY_FAILURE -> %d",__LINE__));
            return COPY_FAILURE;
        }
      }
#endif
      TreeList::AddTreeName(DestPath);
      _LOGCOPYR(SysLog("return COPY_SUCCESS -> %d",__LINE__));
      return COPY_SUCCESS;
    }

    if (DestAttr!=(DWORD)-1 && (DestAttr & FILE_ATTRIBUTE_DIRECTORY)==0)
    {
      _LOGCOPYR(SysLog("%d enter to !FILE_ATTRIBUTE_DIRECTORY",__LINE__));
      /* $ 25.05.2002 IS
         + Не проверяем RenameToShortName второй раз
         + Чтобы не делать лишнюю работу, проверяем сначала совпадение размера
           файлов, в т.ч. и nFileSizeHigh
      */
      if(!RenameToShortName)
      {
        if (SrcData.nFileSizeHigh==DestData.nFileSizeHigh &&
            SrcData.nFileSizeLow==DestData.nFileSizeLow)
      /* IS $ */
        {
          int CmpCode=CmpFullNames(Src,DestPath);
          if(CmpCode==1) // TODO: error check
          {
            SameName=1;
            /* $ 25.05.2002 IS
               Проверим ту ситуацию, когда переименовывается _файл_ в свое же
               _короткое_ имя
            */
            if(Rename)
            {
               CmpCode=!strcmp(PointToName(Src),PointToName(DestPath));
               if(!CmpCode)
               {
                 RenameToShortName = !LocalStricmp(DestPath,SrcData.cAlternateFileName);
                 _LOGCOPYR(SysLog("%d RenameToShortName=%d (LocalStricmp('%s','%s')=%d)",__LINE__,RenameToShortName,DestData.cAlternateFileName,SrcData.cFileName,LocalStricmp(DestData.cAlternateFileName,SrcData.cFileName)));
               }
            }
            /* IS $ */
            if(CmpCode==1)
            {
              Message(MSG_DOWN|MSG_WARNING,1,MSG(MError),MSG(MCannotCopyFileToItself1),
                      Src,MSG(MCannotCopyFileToItself2),MSG(MOk));
              _LOGCOPYR(SysLog("return COPY_CANCEL -> %d",__LINE__));
              return(COPY_CANCEL);
            }
          }
        }

        int RetCode;
        if (!AskOverwrite(SrcData,Src,DestPath,DestAttr,SameName,Rename,((ShellCopy::Flags&FCOPY_LINK)?0:1),Append,RetCode))
        {
          _LOGCOPYR(SysLog("return RetCode=%d -> %d if (!AskCode)",RetCode,__LINE__));
          return((COPY_CODES)RetCode);
        }
      }
    }
  }
  else
  {
    if (SrcData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
      _LOGCOPYR(SysLog("return return COPY_SUCCESS -> %d if (SrcData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)",__LINE__));
      return COPY_SUCCESS;
    }
/*
    {
      char SrcFullName[NM];
      if (ConvertNameToFull(Src,SrcFullName, sizeof(SrcFullName)) >= sizeof(SrcFullName))
      {
        return(COPY_NEXT);
      }
    }
*/
  }

  int NWFS_Attr=(Opt.Nowell.MoveRO && !strcmp(DestFSName,"NWFS"))?TRUE:FALSE;
  {

  _LOGCOPYR(CleverSysLog Clev("while (1)"));
  while (1)
  {
    int CopyCode=0;
//    unsigned __int64 SaveCopiedSize=CurCopiedSize;
    unsigned __int64 SaveTotalSize=TotalCopiedSize;
    if (!(ShellCopy::Flags&FCOPY_COPYTONUL) && Rename)
    {
      int MoveCode=FALSE,AskDelete;

      if ((WinVer.dwPlatformId!=VER_PLATFORM_WIN32_NT || !strcmp(DestFSName,"NWFS")) && !Append &&
          DestAttr!=(DWORD)-1 && !SameName &&
          !RenameToShortName) // !!!
      {
        remove(DestPath);
      }

      if (!Append)
      {
        char SrcFullName[NM*2];
        ConvertNameToFull(Src,SrcFullName, sizeof(SrcFullName));

        if (NWFS_Attr)
          SetFileAttributes(SrcFullName,SrcData.dwFileAttributes&(~FA_RDONLY));

        SECURITY_ATTRIBUTES sa;
        IsSetSecuty=FALSE;

        // для Move нам необходимо узнать каталог родитель, чтобы получить его секьюрити
        if (Rename && !(ShellCopy::Flags&(FCOPY_COPYSECURITY|FCOPY_LEAVESECURITY)))
        {
          if(CmpFullPath(Src,Dest)) // в пределах одного каталога ничего не меняем
            IsSetSecuty=FALSE;
          else if(GetFileAttributes(Dest) == (DWORD)-1) // если каталога нет...
          {
            char DestFullName[NM*2];
            // ...получаем секьюрити родителя
            if(GetSecurity(GetParentFolder(Dest,DestFullName,sizeof(DestFullName)-1),sa))
              IsSetSecuty=TRUE;
          }
          else if(GetSecurity(Dest,sa)) // иначе получаем секьюрити Dest`а
            IsSetSecuty=TRUE;
        }


        /* $ 25.05.2002 IS
           Отдельная обработка RenameToShortName для каталога.
        */
        if(RenameToShortName)
          MoveCode=MoveFileThroughTemp(SrcFullName, DestPath);
        else
        {
          if (WinVer.dwPlatformId!=VER_PLATFORM_WIN32_NT || !strcmp(DestFSName,"NWFS"))
            MoveCode=FAR_MoveFile(SrcFullName,DestPath);
          else
            MoveCode=FAR_MoveFileEx(SrcFullName,DestPath,SameName ? MOVEFILE_COPY_ALLOWED:MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING);
        }

        if (!MoveCode)
        {
          int MoveLastError=GetLastError();
          if (NWFS_Attr)
            SetFileAttributes(SrcFullName,SrcData.dwFileAttributes);

          if(MoveLastError==ERROR_NOT_SAME_DEVICE)
          {
            _LOGCOPYR(SysLog("return COPY_FAILURE -> %d if(MoveLastError==ERROR_NOT_SAME_DEVICE)",__LINE__));
            return COPY_FAILURE;
          }

          SetLastError(MoveLastError);
        }
        else
        {
          if (IsSetSecuty)
            SetSecurity(DestPath,sa);
        }

        if (NWFS_Attr)
          SetFileAttributes(DestPath,SrcData.dwFileAttributes);

        if (ShowTotalCopySize && MoveCode)
        {
          unsigned __int64 AddSize=MKUINT64(SrcData.nFileSizeHigh,SrcData.nFileSizeLow);
          TotalCopiedSize+=AddSize;
          ShowBar(TotalCopiedSize,TotalCopySize,true);
          ShowTitle(FALSE);
        }
        AskDelete=0;
      }
      else
      {
        _LOGCOPYR(SysLog("%d call ShellCopyFile()",__LINE__));
        CopyCode=ShellCopyFile(Src,SrcData,DestPath,(DWORD)-1,Append);
        _LOGCOPYR(SysLog("%d CopyCode=%d",__LINE__,CopyCode));

        switch(CopyCode)
        {
          case COPY_SUCCESS:
            MoveCode=TRUE;
            break;
          case COPY_FAILUREREAD:
          case COPY_FAILURE:
            MoveCode=FALSE;
            break;
          case COPY_CANCEL:
            _LOGCOPYR(SysLog("return COPY_CANCEL -> %d ",__LINE__));
            return COPY_CANCEL;
          case COPY_NEXT:
            _LOGCOPYR(SysLog("return COPY_NEXT -> %d ",__LINE__));
            return COPY_NEXT;
        }
        AskDelete=1;
      }

      if (MoveCode)
      {
        if (DestAttr==(DWORD)-1 || (DestAttr & FILE_ATTRIBUTE_DIRECTORY)==0)
        {
          if (PointToName(DestPath)==DestPath)
            strcpy(RenamedName,DestPath);
          else
            strcpy(CopiedName,PointToName(DestPath));
        }

        if (IsDriveTypeCDROM(SrcDriveType) && Opt.ClearReadOnly &&
            (SrcData.dwFileAttributes & FA_RDONLY))
          ShellSetAttr(DestPath,SrcData.dwFileAttributes & (~FA_RDONLY));

        TotalFiles++;
        if (AskDelete && DeleteAfterMove(Src,SrcData.dwFileAttributes)==COPY_CANCEL)
        {
          _LOGCOPYR(SysLog("return COPY_CANCEL -> %d DeleteAfterMove()==COPY_CANCEL",__LINE__));
          return COPY_CANCEL;
        }

        _LOGCOPYR(SysLog("return COPY_SUCCESS_MOVE -> %d",__LINE__));
        return(COPY_SUCCESS_MOVE);
      }
    }
    else
    {
      _LOGCOPYR(SysLog("%d call ShellCopyFile('%s',%p,'%s',0x%08X,%d)",__LINE__,Src,&SrcData,DestPath,DestAttr,Append));
      CopyCode=ShellCopyFile(Src,SrcData,DestPath,DestAttr,Append);
      _LOGCOPYR(SysLog("%d CopyCode=%d",__LINE__,CopyCode));

      if (CopyCode==COPY_SUCCESS)
      {
        strcpy(CopiedName,PointToName(DestPath));
        if(!(ShellCopy::Flags&FCOPY_COPYTONUL))
        {
          if (IsDriveTypeCDROM(SrcDriveType) && Opt.ClearReadOnly &&
              (SrcData.dwFileAttributes & FA_RDONLY))
            ShellSetAttr(DestPath,SrcData.dwFileAttributes & ~FA_RDONLY);

          if (DestAttr!=(DWORD)-1 && LocalStricmp(CopiedName,DestData.cFileName)==0 &&
              strcmp(CopiedName,DestData.cFileName)!=0)
            FAR_MoveFile(DestPath,DestPath);
        }

        TotalFiles++;
        if(DestAttr!=-1 && Append)
          SetFileAttributes(DestPath,DestAttr);

        _LOGCOPYR(SysLog("return COPY_SUCCESS -> %d",__LINE__));
        return COPY_SUCCESS;
      }
      else if (CopyCode==COPY_CANCEL || CopyCode==COPY_NEXT)
      {
        if(DestAttr!=-1 && Append)
          SetFileAttributes(DestPath,DestAttr);
        _LOGCOPYR(SysLog("return CopyCode [%d] -> %d",CopyCode,__LINE__));
        return((COPY_CODES)CopyCode);
      }
      else if(CopyCode == COPY_FAILURE)
      {
        SkipEncMode=-1;
      }

      if(DestAttr!=-1 && Append)
        SetFileAttributes(DestPath,DestAttr);
    }
    //????
    if(CopyCode == COPY_FAILUREREAD)
    {
      _LOGCOPYR(SysLog("return COPY_FAILURE -> %d if(CopyCode == COPY_FAILUREREAD)",__LINE__));
      return COPY_FAILURE;
    }
    //????

    char Msg1[2*NM],Msg2[2*NM];
    int MsgMCannot=(ShellCopy::Flags&FCOPY_LINK) ? MCannotLink: (ShellCopy::Flags&FCOPY_MOVE) ? MCannotMove: MCannotCopy;
    InsertQuote(strcpy(Msg1,Src));
    InsertQuote(strcpy(Msg2,DestPath));
    {
      int MsgCode;
      if((SrcData.dwFileAttributes&FILE_ATTRIBUTE_ENCRYPTED))
      {
//        if (SkipMode!=-1)
//          MsgCode=SkipMode;
        if (SkipEncMode!=-1)// && SkipMode!=-1)
        {
          MsgCode=SkipEncMode;
          if(SkipEncMode == 1)
            ShellCopy::Flags|=FCOPY_DECRYPTED_DESTINATION;
        }
        else
        {
          if(_localLastError == 5)
          {
            #define ERROR_EFS_SERVER_NOT_TRUSTED     6011L
            ;//SetLastError(_localLastError=(DWORD)0x80090345L);//SEC_E_DELEGATION_REQUIRED);
            SetLastError(_localLastError=ERROR_EFS_SERVER_NOT_TRUSTED);
          }
          MsgCode=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,5,MSG(MError),
                          MSG(MsgMCannot),
                          Msg1,
                          MSG(MCannotCopyTo),
                          Msg2,
                          MSG(MCopyDecrypt),
                          MSG(MCopyDecryptAll),
                          MSG(MCopySkip),
                          MSG(MCopySkipAll),
                          MSG(MCopyCancel));

          switch(MsgCode)
          {
            case  0:
              _LOGCOPYR(SysLog("return COPY_NEXT -> %d",__LINE__));
              ShellCopy::Flags|=FCOPY_DECRYPTED_DESTINATION;
              break;//return COPY_NEXT;
            case  1:
              SkipEncMode=1;
              ShellCopy::Flags|=FCOPY_DECRYPTED_DESTINATION;
              _LOGCOPYR(SysLog("return COPY_NEXT -> %d",__LINE__));
              break;//return COPY_NEXT;
            case  2:
              _LOGCOPYR(SysLog("return COPY_NEXT -> %d",__LINE__));
              return COPY_NEXT;
            case  3:
              SkipMode=1;
              _LOGCOPYR(SysLog("return COPY_NEXT -> %d",__LINE__));
              return COPY_NEXT;
            case -1:
            case -2:
            case  4:
              _LOGCOPYR(SysLog("return COPY_CANCEL -> %d",__LINE__));
              return COPY_CANCEL;
          }
        }
      }
      else
      {
        if (SkipMode!=-1)
          MsgCode=SkipMode;
        else
        {
          MsgCode=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),
                          MSG(MsgMCannot),
                          Msg1,
                          MSG(MCannotCopyTo),
                          Msg2,
                          MSG(MCopyRetry),
                          MSG(MCopySkip),
                          MSG(MCopySkipAll),
                          MSG(MCopyCancel));
        }

        switch(MsgCode)
        {
          case -1:
          case  1:
            _LOGCOPYR(SysLog("return COPY_NEXT -> %d",__LINE__));
            return COPY_NEXT;
          case  2:
            SkipMode=1;
            _LOGCOPYR(SysLog("return COPY_NEXT -> %d",__LINE__));
            return COPY_NEXT;
          case -2:
          case  3:
            _LOGCOPYR(SysLog("return COPY_CANCEL -> %d",__LINE__));
            return COPY_CANCEL;
        }
      }
    }

//    CurCopiedSize=SaveCopiedSize;
    TotalCopiedSize=SaveTotalSize;
    int RetCode;
    if (!AskOverwrite(SrcData,Src,DestPath,DestAttr,SameName,Rename,((ShellCopy::Flags&FCOPY_LINK)?0:1),Append,RetCode))
    {
      _LOGCOPYR(SysLog("return RetCode=%d -> %d if (!AskCode)",RetCode,__LINE__));
      return((COPY_CODES)RetCode);
    }
  }
  }
}

// проверка очередного монстрика на потоки
COPY_CODES ShellCopy::CheckStreams(const char *Src,const char *DestPath)
{
#if 0
  int AscStreams=(ShellCopy::Flags&FCOPY_STREAMSKIP)?2:((ShellCopy::Flags&FCOPY_STREAMALL)?0:1);
  if(!(ShellCopy::Flags&FCOPY_USESYSTEMCOPY) && NT && AscStreams)
  {
    int CountStreams=EnumNTFSStreams(Src,NULL,NULL);
    if(CountStreams > 1 ||
       (CountStreams >= 1 && (GetFileAttributes(Src)&FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY))
    {
      if(AscStreams == 2)
      {
        return(COPY_NEXT);
      }

      SetMessageHelp("WarnCopyStream");
      //char SrcFullName[NM];
      //ConvertNameToFull(Src,SrcFullName, sizeof(SrcFullName));
      //TruncPathStr(SrcFullName,ScrX-16);
      int MsgCode=Message(MSG_DOWN|MSG_WARNING,5,MSG(MWarning),
              MSG(MCopyStream1),
              MSG(CanCreateHardLinks(DestPath,NULL)?MCopyStream2:MCopyStream3),
              MSG(MCopyStream4),"\1",//SrcFullName,"\1",
              MSG(MCopyResume),MSG(MCopyOverwriteAll),MSG(MCopySkipOvr),MSG(MCopySkipAllOvr),MSG(MCopyCancelOvr));
      switch(MsgCode)
      {
        case 0: break;
        case 1: ShellCopy::Flags|=FCOPY_STREAMALL; break;
        case 2: return(COPY_NEXT);
        case 3: ShellCopy::Flags|=FCOPY_STREAMSKIP; return(COPY_NEXT);
        default:
          return COPY_CANCEL;
      }
    }
  }
#endif
  return COPY_SUCCESS;
}

void ShellCopy::PR_ShellCopyMsg(void)
{
  // // _LOGCOPYR(CleverSysLog clv("PR_ShellCopyMsg"));
  // // _LOGCOPYR(SysLog("2='%s'/0x%08X  3='%s'/0x%08X  Flags=0x%08X",(char*)PreRedrawParam.Param2,PreRedrawParam.Param2,(char*)PreRedrawParam.Param3,PreRedrawParam.Param3,PreRedrawParam.Flags));
  LastShowTime = 0;
  PreRedrawItem preRedrawItem=PreRedraw.Peek();
  if(preRedrawItem.Param.Param1)
    ((ShellCopy*)preRedrawItem.Param.Param1)->ShellCopyMsg((char*)preRedrawItem.Param.Param2,(char*)preRedrawItem.Param.Param3,preRedrawItem.Param.Flags&(~MSG_KEEPBACKGROUND));
}

void ShellCopy::ShellCopyMsg(const char *Src,const char *Dest,int Flags)
{
  // // _LOGCOPYR(CleverSysLog clv("ShellCopyMsg"));
  char FilesStr[100],BarStr[100],SrcName[NM],DestName[NM];

  //// // _LOGCOPYR(SysLog("[%p] ShellCopy::ShellCopyMsg('%s','%s',%x)",this,Src,Dest,Flags));
  #define BAR_SIZE  46

  strcpy(BarStr,"\x1");

  if (ShowTotalCopySize)
  {
    char TotalMsg[100];
    if (*TotalCopySizeText)
      sprintf(TotalMsg," %s: %s ",MSG(MCopyDlgTotal),TotalCopySizeText);
    else
      sprintf(TotalMsg," %s ",MSG(MCopyDlgTotal));
    strcat(BarStr,TotalMsg);
//    *FilesStr=0;

    sprintf (FilesStr, MSG(MCopyProcessedTotal),TotalFiles, TotalFilesToProcess);
  }
  else
  {
    sprintf(FilesStr,MSG(MCopyProcessed),TotalFiles);
    /* $ 30.01.2001 VVM
        + Запомнить время начала.
          Работает для каждого файла при выключенном ShowTotalIndicator  */
    if ((Src!=NULL) && (ShowCopyTime))
    {
      CopyStartTime = clock();
      LastShowTime = 0;
      WaitUserTime = OldCalcTime = 0;
    }
    /* VVM $ */
  }

  if (Src!=NULL)
  {
    _snprintf(SrcName,sizeof(SrcName),"%-46s",Src);
    TruncPathStr(SrcName,46);
  }
  _snprintf(DestName,sizeof(DestName),"%-46s",Dest);
  TruncPathStr(DestName,46);

  SetMessageHelp("CopyFiles");
  if (Src==NULL)
    Message(Flags,0,(ShellCopy::Flags&FCOPY_MOVE) ? MSG(MMoveDlgTitle):
                       MSG(MCopyDlgTitle),
                       "",MSG(MCopyScanning),
                       DestName,"","",BarStr,"");
  else
  {
    int Move = ShellCopy::Flags&FCOPY_MOVE;

    if ( ShowTotalCopySize )
    {
      if ( ShowCopyTime )
        Message(Flags, 0, MSG(Move?MMoveDlgTitle:MCopyDlgTitle),MSG(Move?MCopyMoving:MCopyCopying),SrcName,MSG(MCopyTo),DestName,"",BarStr,"","\x1",FilesStr,"\x1","");
      else
        Message(Flags, 0, MSG(Move?MMoveDlgTitle:MCopyDlgTitle),MSG(Move?MCopyMoving:MCopyCopying),SrcName,MSG(MCopyTo),DestName,"",BarStr,"","\x1",FilesStr);
    }
    else
    {
      if ( ShowCopyTime )
        Message(Flags, 0, MSG(Move?MMoveDlgTitle:MCopyDlgTitle),MSG(Move?MCopyMoving:MCopyCopying),SrcName,MSG(MCopyTo),DestName,"",BarStr,FilesStr,"\x1","");
      else
        Message(Flags, 0, MSG(Move?MMoveDlgTitle:MCopyDlgTitle),MSG(Move?MCopyMoving:MCopyCopying),SrcName,MSG(MCopyTo),DestName,"",BarStr,FilesStr);
    }
  }

  int MessageX1,MessageY1,MessageX2,MessageY2;
  GetMessagePosition(MessageX1,MessageY1,MessageX2,MessageY2);
  BarX=MessageX1+5;
  BarY=MessageY1+6;
  BarLength=MessageX2-MessageX1-9-5; //-5 для процентов

  if (Src!=NULL)
  {
    // // _LOGCOPYR(SysLog(" ******************  ShowTotalCopySize=%d",ShowTotalCopySize));
    ShowBar(0,0,false);
    if (ShowTotalCopySize)
    {
      ShowBar(TotalCopiedSize,TotalCopySize,true);
      ShowTitle(FALSE);
    }
  }

  PreRedrawItem preRedrawItem=PreRedraw.Peek();
  preRedrawItem.Param.Flags=Flags;
  preRedrawItem.Param.Param1=this;
  preRedrawItem.Param.Param2=Src;
  preRedrawItem.Param.Param3=Dest;
  PreRedraw.SetParam(preRedrawItem.Param);
}


int ShellCopy::ShellCopyConvertWildcards(const char *SrcName,char *Dest)
{
  char WildName[2*NM],*CurWildPtr,*DestNamePtr,*SrcNamePtr;
  /* $ 25.05.2002 IS
     Скопируем SrcName во внутренний буфер, т.к. нам надо переменную изменить
  */
  char Src[2*NM];
  xstrncpy(Src,SrcName,sizeof(Src)-1);
  /* IS $ */
  char PartBeforeName[NM],PartAfterFolderName[NM];
  DestNamePtr=PointToName(Dest);
  strcpy(WildName,DestNamePtr);
  if (strchr(WildName,'*')==NULL && strchr(WildName,'?')==NULL)
    return(FALSE);

  if (SelectedFolderNameLength!=0)
  {
    strcpy(PartAfterFolderName,Src+SelectedFolderNameLength);
    Src[SelectedFolderNameLength]=0;
  }

  SrcNamePtr=PointToName(Src);

  int BeforeNameLength=DestNamePtr==Dest ? (int)(SrcNamePtr-Src):0;
  xstrncpy(PartBeforeName,Src,BeforeNameLength);
  PartBeforeName[BeforeNameLength]=0;

  char *SrcNameDot=strrchr(SrcNamePtr,'.');
  CurWildPtr=WildName;
  while (*CurWildPtr)
    switch(*CurWildPtr)
    {
      case '?':
        CurWildPtr++;
        if (*SrcNamePtr)
          *(DestNamePtr++)=*(SrcNamePtr++);
        break;
      case '*':
        CurWildPtr++;
        while (*SrcNamePtr)
        {
          if (*CurWildPtr=='.' && SrcNameDot!=NULL && strchr(CurWildPtr+1,'.')==NULL)
          {
            if (SrcNamePtr==SrcNameDot)
              break;
          }
          else
            if (*SrcNamePtr==*CurWildPtr)
              break;
          *(DestNamePtr++)=*(SrcNamePtr++);
        }
        break;
      case '.':
        CurWildPtr++;
        *(DestNamePtr++)='.';
        if (strpbrk(CurWildPtr,"*?")!=NULL)
          while (*SrcNamePtr)
            if (*(SrcNamePtr++)=='.')
              break;
        break;
      default:
        *(DestNamePtr++)=*(CurWildPtr++);
        if (*SrcNamePtr && *SrcNamePtr!='.')
          SrcNamePtr++;
        break;
    }

  *DestNamePtr=0;
  if (DestNamePtr!=Dest && *(DestNamePtr-1)=='.')
    *(DestNamePtr-1)=0;
  if (*PartBeforeName)
  {
    strcat(PartBeforeName,Dest);
    strcpy(Dest,PartBeforeName);
  }
  if (SelectedFolderNameLength!=0)
    strcat(Src,PartAfterFolderName);
  return(TRUE);
}

int ShellCopy::DeleteAfterMove(const char *Name,int Attr)
{
  if (Attr & FA_RDONLY)
  {
    int MsgCode;
    if (ReadOnlyDelMode!=-1)
      MsgCode=ReadOnlyDelMode;
    else
      MsgCode=Message(MSG_DOWN|MSG_WARNING,5,MSG(MWarning),
              MSG(MCopyFileRO),Name,MSG(MCopyAskDelete),
              MSG(MCopyDeleteRO),MSG(MCopyDeleteAllRO),
              MSG(MCopySkipRO),MSG(MCopySkipAllRO),MSG(MCopyCancelRO));
    switch(MsgCode)
    {
      case 1:
        ReadOnlyDelMode=1;
        break;
      case 2:
        return(COPY_NEXT);
      case 3:
        ReadOnlyDelMode=3;
        return(COPY_NEXT);
      case -1:
      case -2:
      case 4:
        return(COPY_CANCEL);
    }
    SetFileAttributes(Name,FILE_ATTRIBUTE_NORMAL);
  }

  while((Attr&FA_DIREC)?!FAR_RemoveDirectory(Name):remove(Name)!=0)
  {
    int MsgCode;
    if(SkipDeleteMode!=-1)
      MsgCode=SkipDeleteMode;
    else
      MsgCode=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),MSG(MCannotDeleteFile),Name,
        MSG(MDeleteRetry),MSG(MDeleteSkip),MSG(MDeleteSkipAll),MSG(MDeleteCancel));
    switch(MsgCode)
    {
      case 1:
        return COPY_NEXT;
      case 2:
        SkipDeleteMode=1;
        return COPY_NEXT;
      case -1:
      case -2:
      case 3:
        return(COPY_CANCEL);
    }
  }
  return(COPY_SUCCESS);
}


int ShellCopy::ShellCopyFile(const char *SrcName,const WIN32_FIND_DATA &SrcData,
                             const char *DestName,DWORD DestAttr,int Append)
{
  _LOGCOPYR(CleverSysLog Clev("ShellCopy::ShellCopyFile()"));
  _LOGCOPYR(SysLog("Params: SrcName='%s', DestName='%s', DestAttr=0x%08X",SrcName, DestName,DestAttr));
  OrigScrX=ScrX;
  OrigScrY=ScrY;

  if ((ShellCopy::Flags&FCOPY_LINK))
  {
    if(RPT==RP_HARDLINK)
    {
      FAR_DeleteFile(DestName);
      return(MkHardLink(SrcName,DestName) ? COPY_SUCCESS:COPY_FAILURE);
    }
    else
    {
      return(MkSymLink(SrcName,DestName,RPT,0) ? COPY_SUCCESS:COPY_FAILURE);
    }
  }

  if((SrcData.dwFileAttributes&FILE_ATTRIBUTE_ENCRYPTED) &&
     !CheckDisksProps(SrcName,DestName,CHECKEDPROPS_ISDST_ENCRYPTION)
    )
  {
    int MsgCode;
    if (SkipEncMode!=-1)
    {
      MsgCode=SkipEncMode;
      if(SkipEncMode == 1)
        ShellCopy::Flags|=FCOPY_DECRYPTED_DESTINATION;
    }
    else
    {
      char Msg1[2*NM];
      SetMessageHelp("WarnCopyEncrypt");
      InsertQuote(xstrncpy(Msg1,SrcName,sizeof(Msg1)-1));
      MsgCode=Message(MSG_DOWN|MSG_WARNING,3,MSG(MWarning),
                      MSG(MCopyEncryptWarn1),
                      Msg1,
                      MSG(MCopyEncryptWarn2),
                      MSG(MCopyEncryptWarn3),
                      MSG(MCopyIgnore),MSG(MCopyIgnoreAll),MSG(MCopyCancel));
    }

    switch(MsgCode)
    {
      case  0:
        _LOGCOPYR(SysLog("return COPY_NEXT -> %d",__LINE__));
        ShellCopy::Flags|=FCOPY_DECRYPTED_DESTINATION;
        break;//return COPY_NEXT;
      case  1:
        SkipEncMode=1;
        ShellCopy::Flags|=FCOPY_DECRYPTED_DESTINATION;
        _LOGCOPYR(SysLog("return COPY_NEXT -> %d",__LINE__));
        break;//return COPY_NEXT;
      case -1:
      case -2:
      case  2:
        _LOGCOPYR(SysLog("return COPY_CANCEL -> %d",__LINE__));
        return COPY_CANCEL;
    }
  }

  if (!(ShellCopy::Flags&FCOPY_COPYTONUL) && (ShellCopy::Flags&FCOPY_USESYSTEMCOPY) && !Append)
  {
    //if(!(WinVer.dwMajorVersion >= 5 && WinVer.dwMinorVersion > 0) && (ShellCopy::Flags&FCOPY_DECRYPTED_DESTINATION))
    if(!(SrcData.dwFileAttributes&FILE_ATTRIBUTE_ENCRYPTED) ||
        (SrcData.dwFileAttributes&FILE_ATTRIBUTE_ENCRYPTED) &&
          (WinVer.dwMajorVersion >= 5 && WinVer.dwMinorVersion > 0 ||
          !(ShellCopy::Flags&(FCOPY_DECRYPTED_DESTINATION)))
      )
    {
      if (!Opt.CMOpt.CopyOpened)
      {
        HANDLE SrcHandle=FAR_CreateFile (
            SrcName,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_SEQUENTIAL_SCAN,
            NULL
            );

        if (SrcHandle==INVALID_HANDLE_VALUE)
        {
          _LOGCOPYR(SysLog("return COPY_FAILURE -> %d if (SrcHandle==INVALID_HANDLE_VALUE)",__LINE__));
          return COPY_FAILURE;
        }
        CloseHandle(SrcHandle);
      }
      _LOGCOPYR(SysLog("call ShellSystemCopy('%s','%s',%p)",SrcName,DestName,SrcData));
      return(ShellSystemCopy(SrcName,DestName,SrcData));
    }
  }

  SECURITY_ATTRIBUTES sa;
  if ((ShellCopy::Flags&FCOPY_COPYSECURITY) && !GetSecurity(SrcName,sa))
  {
    _LOGCOPYR(SysLog("return COPY_CANCEL -> %d GetSecurity() == 0",__LINE__));
    return COPY_CANCEL;
  }
  int OpenMode=FILE_SHARE_READ;
  if (Opt.CMOpt.CopyOpened)
    OpenMode|=FILE_SHARE_WRITE;
  HANDLE SrcHandle= FAR_CreateFile (
      SrcName,
      GENERIC_READ,
      OpenMode,
      NULL,
      OPEN_EXISTING,
      FILE_FLAG_SEQUENTIAL_SCAN,
      NULL
      );
  if (SrcHandle==INVALID_HANDLE_VALUE && Opt.CMOpt.CopyOpened)
  {
    _localLastError=GetLastError();
    SetLastError(_localLastError);
    if ( _localLastError == ERROR_SHARING_VIOLATION )
    {
      SrcHandle = FAR_CreateFile (
          SrcName,
          GENERIC_READ,
          FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
          NULL,
          OPEN_EXISTING,
          FILE_FLAG_SEQUENTIAL_SCAN,
          NULL
          );
    }
  }
  if (SrcHandle == INVALID_HANDLE_VALUE )
  {
    _localLastError=GetLastError();
    SetLastError(_localLastError);
    _LOGCOPYR(SysLog("return COPY_FAILURE -> %d if (SrcHandle==INVALID_HANDLE_VALUE)",__LINE__));
    return COPY_FAILURE;
  }

  HANDLE DestHandle=INVALID_HANDLE_VALUE;
  LARGE_INTEGER AppendPos={0};

  if(!(ShellCopy::Flags&FCOPY_COPYTONUL))
  {
    //if (DestAttr!=(DWORD)-1 && !Append) //вот это портит копирование поверх хардлинков
      //remove(DestName);
    DestHandle=FAR_CreateFile (
        DestName,
        GENERIC_WRITE,
        FILE_SHARE_READ,
        (ShellCopy::Flags&FCOPY_COPYSECURITY) ? &sa:NULL,
        (Append ? OPEN_EXISTING:CREATE_ALWAYS),
        SrcData.dwFileAttributes&(~((ShellCopy::Flags&(FCOPY_DECRYPTED_DESTINATION))?FILE_ATTRIBUTE_ENCRYPTED|FILE_FLAG_SEQUENTIAL_SCAN:FILE_FLAG_SEQUENTIAL_SCAN)),
        NULL
        );

    ShellCopy::Flags&=~FCOPY_DECRYPTED_DESTINATION;
    if (DestHandle==INVALID_HANDLE_VALUE)
    {
      _localLastError=GetLastError();
      CloseHandle(SrcHandle);
      SetLastError(_localLastError);
      _LOGCOPYR(SysLog("return COPY_FAILURE -> %d CreateFile=-1, LastError=%d (0x%08X)",__LINE__,_localLastError,_localLastError));
      return COPY_FAILURE;
    }

    char DriveRoot[NM];
    GetPathRoot(SrcName,DriveRoot);
    DWORD VolFlags=0;
    GetVolumeInformation(DriveRoot,NULL,0,NULL,NULL,&VolFlags,NULL,0);
    CopySparse=((VolFlags&FILE_SUPPORTS_SPARSE_FILES)==FILE_SUPPORTS_SPARSE_FILES);
    if(CopySparse)
    {
      GetPathRoot(DestName,DriveRoot);
      VolFlags=0;
      GetVolumeInformation(DriveRoot,NULL,0,NULL,NULL,&VolFlags,NULL,0);
      CopySparse=((VolFlags&FILE_SUPPORTS_SPARSE_FILES)==FILE_SUPPORTS_SPARSE_FILES);
      if(CopySparse)
      {
        BY_HANDLE_FILE_INFORMATION bhfi;
        GetFileInformationByHandle(SrcHandle, &bhfi);
        CopySparse=((bhfi.dwFileAttributes&FILE_ATTRIBUTE_SPARSE_FILE)==FILE_ATTRIBUTE_SPARSE_FILE);
        if(CopySparse)
        {
          DWORD Temp;
          if(!DeviceIoControl(DestHandle,FSCTL_SET_SPARSE,NULL,0,NULL,0,&Temp,NULL))
            CopySparse=false;
        }
      }
    }

    if (Append)
    {
      LARGE_INTEGER Pos={0};
      if(!FAR_SetFilePointerEx(DestHandle,Pos,&AppendPos,FILE_END))
      {
        _localLastError=GetLastError();
        CloseHandle(SrcHandle);
        CloseHandle(DestHandle);
        SetLastError(_localLastError);
        _LOGCOPYR(SysLog("return COPY_FAILURE -> %d FAR_SetFilePointerEx() == -1, LastError=%d (0x%08X)",__LINE__,_localLastError,_localLastError));
        return COPY_FAILURE;
      }
    }

    // если места в приёмнике хватает - займём сразу.
    UINT64 FreeBytes=0;
    LARGE_INTEGER SrcSize;
    SrcSize.u.LowPart=SrcData.nFileSizeLow;
    SrcSize.u.HighPart=SrcData.nFileSizeHigh;
    if(GetDiskSize(DriveRoot,NULL,NULL,&FreeBytes))
    {
      if(FreeBytes>(UINT64)SrcSize.QuadPart)
      {
        LARGE_INTEGER CurPtr={0};
        CurPtr.u.LowPart=SetFilePointer(DestHandle,0,&CurPtr.u.HighPart,FILE_CURRENT);
        if(FAR_SetFilePointerEx(DestHandle,SrcSize,NULL,FILE_CURRENT) && SetEndOfFile(DestHandle))
            FAR_SetFilePointerEx(DestHandle,CurPtr,NULL,FILE_BEGIN);
      }
    }
  }

//  unsigned __int64 WrittenSize=0;
  int   AbortOp = FALSE;
  //UINT  OldErrMode=SetErrorMode(SEM_NOOPENFILEERRORBOX|SEM_NOGPFAULTERRORBOX|SEM_FAILCRITICALERRORS);
  unsigned __int64 FileSize=MKUINT64(SrcData.nFileSizeHigh,SrcData.nFileSizeLow);

  BOOL SparseQueryResult=TRUE;
  LARGE_INTEGER iFileSize;
  iFileSize.QuadPart=FileSize;
  FILE_ALLOCATED_RANGE_BUFFER queryrange;
  FILE_ALLOCATED_RANGE_BUFFER ranges[1024];
  queryrange.FileOffset.QuadPart = 0;
  queryrange.Length = iFileSize;

  do
  {
    DWORD n=0,nbytes=0;
    if(CopySparse)
    {
      SparseQueryResult=DeviceIoControl(SrcHandle,FSCTL_QUERY_ALLOCATED_RANGES,&queryrange,sizeof(queryrange),ranges,sizeof(ranges),&nbytes,NULL);
      if(!SparseQueryResult && GetLastError()!=ERROR_MORE_DATA)
        break;
      n=nbytes/sizeof(FILE_ALLOCATED_RANGE_BUFFER);
    }

    for(DWORD i=0;i<(CopySparse?n:i+1);i++)
    {
      LARGE_INTEGER iSize;
      if(CopySparse)
      {
        iSize=ranges[i].Length;
        FAR_SetFilePointerEx(SrcHandle,ranges[i].FileOffset,NULL,FILE_BEGIN);
        LARGE_INTEGER DestPos=ranges[i].FileOffset;
        if(Append)
          DestPos.QuadPart+=AppendPos.QuadPart;
        FAR_SetFilePointerEx(DestHandle,DestPos,NULL,FILE_BEGIN);
      }
      DWORD BytesRead,BytesWritten;
      while (CopySparse?(iSize.QuadPart>0):true)
      {
        BOOL IsChangeConsole=OrigScrX != ScrX || OrigScrY != ScrY;

        if (CheckForEscSilent())
        {
          AbortOp = ConfirmAbortOp();
          IsChangeConsole=TRUE; // !!! Именно так; для того, чтобы апдейтить месаг
        }

        IsChangeConsole=ShellCopy::CheckAndUpdateConsole(IsChangeConsole);

        if(IsChangeConsole)
        {
          OrigScrX=ScrX;
          OrigScrY=ScrY;
          ShellCopy::PR_ShellCopyMsg();

          ShowBar(CurCopiedSize,FileSize,false);
          if (ShowTotalCopySize)
          {
            ShowBar(TotalCopiedSize,TotalCopySize,true);
            ShowTitle(FALSE);
          }
        }

        if (AbortOp)
        {
          CloseHandle(SrcHandle);
          if(!(ShellCopy::Flags&FCOPY_COPYTONUL))
          {
            if (Append)
            {
              FAR_SetFilePointerEx(DestHandle,AppendPos,NULL,FILE_BEGIN);
              SetEndOfFile(DestHandle);
            }
            CloseHandle(DestHandle);
            if (!Append)
            {
              SetFileAttributes(DestName,FILE_ATTRIBUTE_NORMAL);
              FAR_DeleteFile(DestName);
            }
          }
          _LOGCOPYR(SysLog("return COPY_CANCEL -> %d",__LINE__));
          //SetErrorMode(OldErrMode);
          return COPY_CANCEL;
        }

      /* $ 23.10.2000 VVM
         + Динамический буфер копирования */

      /* $ 25.04.2003 VVM
         - Отменим пока это буфер */
//      if (CopyBufSize < CopyBufferSize)
//        StartTime=clock();
        while (!ReadFile(SrcHandle,CopyBuffer,(CopySparse?(DWORD)Min((LONGLONG)CopyBufSize,iSize.QuadPart):CopyBufSize),&BytesRead,NULL))
        {
          int MsgCode = Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),
                              MSG(MCopyReadError),SrcName,
                              MSG(MRetry),MSG(MCancel));
          ShellCopy::PR_ShellCopyMsg();
          if (MsgCode==0)
            continue;
          DWORD LastError=GetLastError();
          CloseHandle(SrcHandle);
          if(!(ShellCopy::Flags&FCOPY_COPYTONUL))
          {
            if (Append)
            {
              FAR_SetFilePointerEx(DestHandle,AppendPos,NULL,FILE_BEGIN);
              SetEndOfFile(DestHandle);
            }
            CloseHandle(DestHandle);
            if (!Append)
            {
              SetFileAttributes(DestName,FILE_ATTRIBUTE_NORMAL);
              FAR_DeleteFile(DestName);
            }
          }
          ShowBar(0,0,false);
          ShowTitle(FALSE);
          //SetErrorMode(OldErrMode);
          SetLastError(_localLastError=LastError);
          // return COPY_FAILUREREAD;
          _LOGCOPYR(SysLog("return COPY_FAILURE -> %d",__LINE__));
          CurCopiedSize = 0; // Сбросить текущий прогресс
          return COPY_FAILURE;
        }
        if (BytesRead==0)
        {
          SparseQueryResult=FALSE;
          break;
        }

        if(!(ShellCopy::Flags&FCOPY_COPYTONUL))
        {
          while (!WriteFile(DestHandle,CopyBuffer,BytesRead,&BytesWritten,NULL))
          {
            DWORD LastError=GetLastError();
            int Split=FALSE,SplitCancelled=FALSE,SplitSkipped=FALSE;
            if ((LastError==ERROR_DISK_FULL || LastError==ERROR_HANDLE_DISK_FULL) &&
                DestName[0]!=0 && DestName[1]==':')
            {
              char DriveRoot[NM];
              GetPathRoot(DestName,DriveRoot);

              UINT64 FreeSize=0;
              if(GetDiskSize(DriveRoot,NULL,NULL,&FreeSize))
              {
                if (FreeSize<BytesRead &&
                    WriteFile(DestHandle,CopyBuffer,(DWORD)FreeSize,&BytesWritten,NULL) &&
                    SetFilePointer(SrcHandle,(DWORD)FreeSize-BytesRead,NULL,FILE_CURRENT)!=INVALID_SET_FILE_POINTER)
                {
                  CloseHandle(DestHandle);
                  SetMessageHelp("CopyFiles");
                  int MsgCode=Message(MSG_DOWN|MSG_WARNING,4,MSG(MError),
                                      MSG(MErrorInsufficientDiskSpace),DestName,
                                      MSG(MSplit),MSG(MSkip),MSG(MRetry),MSG(MCancel));
                  ShellCopy::PR_ShellCopyMsg();
                  if (MsgCode==2)
                  {
                    CloseHandle(SrcHandle);
                    if (!Append)
                    {
                      SetFileAttributes(DestName,FILE_ATTRIBUTE_NORMAL);
                      FAR_DeleteFile(DestName);
                    }
                    _LOGCOPYR(SysLog("return COPY_FAILURE -> %d",__LINE__));
                    //SetErrorMode(OldErrMode);
                    return COPY_FAILURE;
                  }
                  if (MsgCode==0)
                  {
                    Split=TRUE;
                    while (1)
                    {
                      if(GetDiskSize(DriveRoot,NULL,NULL,&FreeSize))
                        if (FreeSize<BytesRead)
                        {
                          int MsgCode = Message(MSG_DOWN|MSG_WARNING,2,MSG(MWarning),
                                                MSG(MCopyErrorDiskFull),DestName,
                                                MSG(MRetry),MSG(MCancel));
                          ShellCopy::PR_ShellCopyMsg();
                          if (MsgCode!=0)
                          {
                            Split=FALSE;
                            SplitCancelled=TRUE;
                          }
                          else
                            continue;
                        }
                      break;
                    }
                  }
                  if (MsgCode==1)
                    SplitSkipped=TRUE;
                  if (MsgCode==-1 || MsgCode==3)
                    SplitCancelled=TRUE;
                }
              }
            }
            if (Split)
            {
              LARGE_INTEGER FilePtr={0};
              FilePtr.u.LowPart=SetFilePointer(SrcHandle,0,&FilePtr.u.HighPart,FILE_CURRENT);
              WIN32_FIND_DATA SplitData=SrcData;
              LARGE_INTEGER nFileSize;
              nFileSize.u.LowPart=SplitData.nFileSizeLow;
              nFileSize.u.HighPart=SplitData.nFileSizeHigh;
              nFileSize.QuadPart-=FilePtr.QuadPart;
              SplitData.nFileSizeHigh=nFileSize.u.HighPart;
              SplitData.nFileSizeLow=nFileSize.u.LowPart;
              int RetCode;
              if (!AskOverwrite(SplitData,SrcName,DestName,0xFFFFFFFF,FALSE,((ShellCopy::Flags&FCOPY_MOVE)?TRUE:FALSE),((ShellCopy::Flags&FCOPY_LINK)?0:1),Append,RetCode))
              {
                CloseHandle(SrcHandle);
                //SetErrorMode(OldErrMode);
                _LOGCOPYR(SysLog("return COPY_CANCEL -> %d",__LINE__));
                return(COPY_CANCEL);
              }
              char DestDir[NM],*ChPtr;
              strcpy(DestDir,DestName);
              if ((ChPtr=strrchr(DestDir,'\\'))!=NULL)
              {
                *ChPtr=0;
                CreatePath(DestDir);
              }
              DestHandle=FAR_CreateFile (
                  DestName,
                  GENERIC_WRITE,
                  FILE_SHARE_READ,
                  NULL,
                  (Append ? OPEN_EXISTING:CREATE_ALWAYS),
                  SrcData.dwFileAttributes|FILE_FLAG_SEQUENTIAL_SCAN,
                  NULL
                  );

              if (DestHandle==INVALID_HANDLE_VALUE ||
                  Append && SetFilePointer(DestHandle,0,NULL,FILE_END)==INVALID_SET_FILE_POINTER)
              {
                DWORD LastError=GetLastError();
                CloseHandle(SrcHandle);
                CloseHandle(DestHandle);
                //SetErrorMode(OldErrMode);
                SetLastError(_localLastError=LastError);
                _LOGCOPYR(SysLog("return COPY_FAILURE -> %d",__LINE__));
                return COPY_FAILURE;
              }
            }
            else
            {
              if (!SplitCancelled && !SplitSkipped &&
                  Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),
                  MSG(MCopyWriteError),DestName,MSG(MRetry),MSG(MCancel))==0)
              {
                continue;
              }
              CloseHandle(SrcHandle);
              if (Append)
              {
                FAR_SetFilePointerEx(DestHandle,AppendPos,NULL,FILE_BEGIN);
                SetEndOfFile(DestHandle);
              }
              CloseHandle(DestHandle);
              if (!Append)
              {
                SetFileAttributes(DestName,FILE_ATTRIBUTE_NORMAL);
                FAR_DeleteFile(DestName);
              }
              ShowBar(0,0,false);
              ShowTitle(FALSE);
              //SetErrorMode(OldErrMode);
              SetLastError(_localLastError=LastError);
              if (SplitSkipped)
              {
                _LOGCOPYR(SysLog("return COPY_NEXT -> %d",__LINE__));
                return COPY_NEXT;
              }
              _LOGCOPYR(SysLog("return (%d ? COPY_CANCEL:COPY_FAILURE) -> %d",SplitCancelled,__LINE__));
              return(SplitCancelled ? COPY_CANCEL:COPY_FAILURE);
            }
            break;
          }
        }
        else
        {
          BytesWritten=BytesRead; // не забудем приравнять количество записанных байт
        }

//    if ((CopyBufSize < CopyBufferSize) && (BytesWritten==CopyBufSize))
//   {
//      StopTime=clock();
//      if ((StopTime - StartTime) < 250)
//      {
//        CopyBufSize*=2;
//        if (CopyBufSize > CopyBufferSize)
//          CopyBufSize=CopyBufferSize;
//      }
//    } /* if */
    /* VVM $ */
    /* VVM $ */

        CurCopiedSize+=BytesWritten;
        if (ShowTotalCopySize)
          TotalCopiedSize+=BytesWritten;

        /* $ 14.09.2002 VVM
          + Показывать прогресс не чаще 5 раз в секунду */
        if ((CurCopiedSize == FileSize) || (clock() - LastShowTime > COPY_TIMEOUT))
        {
          ShowBar(CurCopiedSize,FileSize,false);
          if (ShowTotalCopySize)
          {
            ShowBar(TotalCopiedSize,TotalCopySize,true);
            ShowTitle(FALSE);
          }
        }
        /* VVM $ */
        if(CopySparse)
          iSize.QuadPart -= BytesRead;
      }
      if(!CopySparse || !SparseQueryResult)
        break;
    } /* for */
    if(!SparseQueryResult)
      break;
    if(CopySparse)
    {
      if(!SparseQueryResult && n>0)
      {
        queryrange.FileOffset.QuadPart=ranges[n-1].FileOffset.QuadPart+ranges[n-1].Length.QuadPart;
        queryrange.Length.QuadPart = iFileSize.QuadPart-queryrange.FileOffset.QuadPart;
      }
    }
  }
  while(!SparseQueryResult && CopySparse);
  //SetErrorMode(OldErrMode);

  if(!(ShellCopy::Flags&FCOPY_COPYTONUL))
  {
    SetFileTime(DestHandle,NULL,NULL,&SrcData.ftLastWriteTime);
    CloseHandle(SrcHandle);
    if(CopySparse)
    {
      LARGE_INTEGER Pos;
      Pos.QuadPart=iFileSize.QuadPart;
      if(Append)
        Pos.QuadPart+=AppendPos.QuadPart;
      FAR_SetFilePointerEx(DestHandle,Pos,NULL,FILE_BEGIN);
      SetEndOfFile(DestHandle);
    }
    CloseHandle(DestHandle);

    // TODO: ЗДЕСЯ СТАВИТЬ Compressed???
    if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS &&
        (SrcData.dwFileAttributes & (FA_HIDDEN|FA_SYSTEM|FA_RDONLY)))
      ShellSetAttr(DestName,SrcData.dwFileAttributes&(~((ShellCopy::Flags&(FCOPY_DECRYPTED_DESTINATION))?FILE_ATTRIBUTE_ENCRYPTED:0)));
    ShellCopy::Flags&=~FCOPY_DECRYPTED_DESTINATION;
  }
  else
    CloseHandle(SrcHandle);

  _LOGCOPYR(SysLog("return COPY_SUCCESS -> %d",__LINE__));
  return COPY_SUCCESS;
}

/* $ 30.01.2001 VVM
    + Перевод секунд в текст */
static void GetTimeText(DWORD Time, char *TimeText)
{
  DWORD Sec = Time;
  DWORD Min = Sec/60;
  Sec-=(Min * 60);
  DWORD Hour = Min/60;
  Min-=(Hour*60);
  sprintf(TimeText,"%02u:%02u:%02u",Hour,Min,Sec);
}

/* $ 30.04.2003 VVM
  + Функция возвращает TRUE, если что-то нарисовала, иначе FALSE */
int ShellCopy::ShowBar(unsigned __int64 WrittenSize,unsigned __int64 TotalSize,bool TotalBar)
{
  if(ShowTotalCopySize == TotalBar)
    TBC.SetProgressValue(WrittenSize,TotalSize);
  //_LOGCOPYR(CleverSysLog clv("ShellCopy::ShowBar"));
  //_LOGCOPYR(SysLog("WrittenSize=%Ld ,TotalSize=%Ld, TotalBar=%d",WrittenSize,TotalSize,TotalBar));
  if (!ShowTotalCopySize || TotalBar)
    LastShowTime = clock();

  unsigned __int64 OldWrittenSize = WrittenSize;
  unsigned __int64 OldTotalSize = TotalSize;
  //WrittenSize=WrittenSize>>8;
  //TotalSize=TotalSize>>8;

  int Length;
  if (WrittenSize>TotalSize)
    WrittenSize=TotalSize;
  if (!TotalSize)
    Length=BarLength;
  else
    Length=(int)(WrittenSize*BarLength/TotalSize);

  char ProgressBar[100];
  memset(ProgressBar,0x0B0,BarLength);
  ProgressBar[BarLength]=0;
  if (TotalSize)
    memset(ProgressBar,0x0DB,Length);
  SetColor(COL_DIALOGTEXT);
  GotoXY(BarX,BarY+(TotalBar ? 2:0));
  Text(ProgressBar);

  GotoXY(BarX+BarLength,BarY+(TotalBar ? 2:0));

  char Percents[32];
  sprintf (Percents, "%4d%%", ToPercent64 (WrittenSize, TotalSize));

  Text (Percents);

/* $ 30.01.2001 VVM
    + Показывает время копирования,оставшееся время и среднюю скорость. */
  // // _LOGCOPYR(SysLog("!!!!!!!!!!!!!! ShowCopyTime=%d ,ShowTotalCopySize=%d, TotalBar=%d",ShowCopyTime,ShowTotalCopySize,TotalBar));
  if (ShowCopyTime && (!ShowTotalCopySize || TotalBar))
  {
    unsigned long WorkTime = clock() - CopyStartTime;
    unsigned __int64 SizeLeft = OldTotalSize - OldWrittenSize;

    if (SizeLeft < 0)
      SizeLeft = 0;

    long CalcTime = OldCalcTime;
    if (WaitUserTime != -1) // -1 => находимся в процессе ожидания ответа юзера
      OldCalcTime = CalcTime = WorkTime - WaitUserTime;
    WorkTime /= 1000;
    CalcTime /= 1000;

    char TimeStr[100];
    char c[2];
    c[1]=0;

    if (OldTotalSize == 0 || WorkTime == 0)
      sprintf(TimeStr,MSG(MCopyTimeInfo), " ", " ", 0, " ");
    else
    {
      if (TotalBar)
        OldWrittenSize = OldWrittenSize - TotalSkippedSize;
      unsigned long CPS = static_cast<unsigned long>(CalcTime?OldWrittenSize/CalcTime:0);
      unsigned long TimeLeft = static_cast<unsigned long>((CPS)?SizeLeft/CPS:0);

      c[0]=' ';
      if (CPS > 99999) {
        c[0]='K';
        CPS = CPS/1024;
      }
      if (CPS > 99999) {
        c[0]='M';
        CPS = CPS/1024;
      }
      if (CPS > 99999) {
        c[0]='G';
        CPS = CPS/1024;
      }
      char WorkTimeStr[32];
      char TimeLeftStr[32];
      GetTimeText(WorkTime, WorkTimeStr);
      GetTimeText(TimeLeft, TimeLeftStr);
      sprintf(TimeStr,MSG(MCopyTimeInfo), WorkTimeStr, TimeLeftStr, CPS, c);
    }
    GotoXY(BarX,BarY+(TotalBar?6:4));
    Text(TimeStr);
  }
  return (TRUE);
}


void ShellCopy::SetDestDizPath(const char *DestPath)
{
  if (!(ShellCopy::Flags&FCOPY_DIZREAD))
  {
    strcpy(DestDizPath,DestPath);
    char *ChPtr=PointToName(DestDizPath);
    *(ChPtr--)=0;
    if (*DestDizPath==0)
      strcpy(DestDizPath,".");
    if (ChPtr>DestDizPath && *ChPtr!=':' && *(ChPtr-1)!=':')
      *ChPtr=0;
    if (Opt.Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED && !SrcPanel->IsDizDisplayed() ||
        Opt.Diz.UpdateMode==DIZ_NOT_UPDATE)
      *DestDizPath=0;
    if (*DestDizPath)
      DestDiz.Read(DestDizPath);
    ShellCopy::Flags|=FCOPY_DIZREAD;
  }
}

#define WARN_DLG_HEIGHT 13
#define WARN_DLG_WIDTH 68

enum WarnDlgItems
{
  WDLG_BORDER,
  WDLG_TEXT,
  WDLG_FILENAME,
  WDLG_SEPARATOR,
  WDLG_SRCFILEBTN,
  WDLG_DSTFILEBTN,
  WDLG_SEPARATOR2,
  WDLG_CHECKBOX,
  WDLG_SEPARATOR3,
  WDLG_OVERWRITE,
  WDLG_SKIP,
  WDLG_APPEND,
  WDLG_CANCEL,

};

#define DM_OPENVIEWER DM_USER+33

LONG_PTR WINAPI WarnDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  switch(Msg)
  {
  case DM_OPENVIEWER:
    {
      LPCSTR ViewName=NULL;
      LPCSTR* WFN=(LPCSTR*)Dialog::SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
      if(WFN)
      {
        switch(Param1)
        {
        case WDLG_SRCFILEBTN:
          ViewName=WFN[0];
          break;
        case WDLG_DSTFILEBTN:
          ViewName=WFN[1];
          break;
        }
        FileViewer Viewer(ViewName,FALSE,FALSE,TRUE,-1,NULL,NULL,FALSE);

        // а этот трюк не даст пользователю сменить текущий каталог по CtrlF10 и этим ввести в заблуждение копир:
        Viewer.SetTempViewName("nul",FALSE);

        Viewer.SetDynamicallyBorn(FALSE);
        FrameManager->EnterModalEV();
        FrameManager->ExecuteModal();
        FrameManager->ExitModalEV();
        FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
      }
    }
    break;
  case DN_CTLCOLORDLGITEM:
    {
      if(Param1==WDLG_FILENAME)
      {
        FarDialogItem di;
        Dialog::SendDlgMessage(hDlg,DM_GETDLGITEM,Param1,(LONG_PTR)&di);
        int Color=FarColorToReal(COL_WARNDIALOGTEXT)&0xFF;
        return ((Param2&0xFF00FF00)|(Color<<16)|Color);
      }
    }
    break;
  case DN_BTNCLICK:
    {
      if(Param1==WDLG_SRCFILEBTN || Param1==WDLG_DSTFILEBTN)
      {
        Dialog::SendDlgMessage(hDlg,DM_OPENVIEWER,Param1,NULL);
      }
    }
    break;
  case DN_KEY:
    {
      if((Param1==WDLG_SRCFILEBTN || Param1==WDLG_DSTFILEBTN) && Param2==KEY_F3)
      {
        Dialog::SendDlgMessage(hDlg,DM_OPENVIEWER,Param1,NULL);
      }
    }
    break;

  }
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

int ShellCopy::AskOverwrite(const WIN32_FIND_DATA &SrcData,
               const char *SrcName,
               const char *DestName, DWORD DestAttr,
               int SameName,int Rename,int AskAppend,
               int &Append,int &RetCode)
{
  DialogData WarnCopyDlgData[]=
  {
    /* 00 */  DI_DOUBLEBOX,3,1,WARN_DLG_WIDTH-4,WARN_DLG_HEIGHT-2,0,0,0,0,MSG(MWarning),
    /* 01 */  DI_TEXT,5,2,WARN_DLG_WIDTH-6,2,0,0,DIF_CENTERTEXT,0,MSG(MCopyFileExist),
    /* 02 */  DI_EDIT,5,3,WARN_DLG_WIDTH-6,3,0,0,DIF_READONLY,0,(char*)DestName,
    /* 03 */  DI_TEXT,3,4,0,4,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",

    /* 04 */  DI_BUTTON,5,5,WARN_DLG_WIDTH-6,5,0,0,DIF_BTNNOCLOSE|DIF_NOBRACKETS,0,"",
    /* 05 */  DI_BUTTON,5,6,WARN_DLG_WIDTH-6,6,0,0,DIF_BTNNOCLOSE|DIF_NOBRACKETS,0,"",

    /* 06 */  DI_TEXT,3,7,0,7,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",

    /* 07 */  DI_CHECKBOX,5,8,0,8,1,0,0,0,MSG(MCopyRememberChoice),
    /* 08 */  DI_TEXT,3,9,0,9,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",

    /* 09 */  DI_BUTTON,0,10,0,10,0,0,DIF_CENTERGROUP,1,MSG(MCopyOverwrite),
    /* 10 */  DI_BUTTON,0,10,0,10,0,0,DIF_CENTERGROUP,0,MSG(MCopySkipOvr),
    /* 11 */  DI_BUTTON,0,10,0,10,0,0,DIF_CENTERGROUP|(AskAppend?0:(DIF_DISABLE|DIF_HIDDEN)),0,MSG(MCopyAppend),
    /* 12 */  DI_BUTTON,0,10,0,10,0,0,DIF_CENTERGROUP,0,MSG(MCopyCancelOvr),
  };

  WIN32_FIND_DATA DestData={0};
  int DestDataFilled=FALSE;

  int MsgCode;

  Append=FALSE;

  if((ShellCopy::Flags&FCOPY_COPYTONUL))
  {
    RetCode=COPY_NEXT;
    return TRUE;
  }

  if (DestAttr==INVALID_FILE_ATTRIBUTES)
    if ((DestAttr=GetFileAttributes(DestName))==INVALID_FILE_ATTRIBUTES)
      return(TRUE);

  if (DestAttr & FILE_ATTRIBUTE_DIRECTORY)
    return(TRUE);

  if (OvrMode!=-1)
    MsgCode=OvrMode;
  else
  {
    int Type;
    if (!Opt.Confirm.Copy && !Rename || !Opt.Confirm.Move && Rename ||
        SameName || (Type=GetFileTypeByName(DestName))==FILE_TYPE_CHAR ||
        Type==FILE_TYPE_PIPE || (ShellCopy::Flags&FCOPY_OVERWRITENEXT))
      MsgCode=1;
    else
    {
      memset(&DestData,0,sizeof(DestData));
      GetFileWin32FindData(DestName,&DestData);
      DestDataFilled=TRUE;
      /* $ 04.08.2000 SVS
         Опция "Only newer file(s)"
      */
      if((ShellCopy::Flags&FCOPY_ONLYNEWERFILES))
      {
        // сравним время
        __int64 RetCompare=*(__int64*)&DestData.ftLastWriteTime - *(__int64*)&SrcData.ftLastWriteTime;
        if(RetCompare < 0)
          MsgCode=0;
        else
          MsgCode=2;
      }
      else
      {
        char SrcFileStr[512],DestFileStr[512];
        unsigned __int64 SrcSize=MKUINT64(SrcData.nFileSizeHigh,SrcData.nFileSizeLow);
        char SrcSizeText[20];
        _i64toa(SrcSize,SrcSizeText,10);
        unsigned __int64 DestSize=MKUINT64(DestData.nFileSizeHigh,DestData.nFileSizeLow);
        char DestSizeText[20];
        _i64toa(DestSize,DestSizeText,10);

        char DateText[20],TimeText[20];
        ConvertDate(SrcData.ftLastWriteTime,DateText,TimeText,8,FALSE,FALSE,TRUE,TRUE);
        sprintf(SrcFileStr,"%-17s %20.20s %s %s",MSG(MCopySource),SrcSizeText,DateText,TimeText);
        ConvertDate(DestData.ftLastWriteTime,DateText,TimeText,8,FALSE,FALSE,TRUE,TRUE);
        sprintf(DestFileStr,"%-17s %20.20s %s %s",MSG(MCopyDest),DestSizeText,DateText,TimeText);

    WarnCopyDlgData[WDLG_SRCFILEBTN].Data=SrcFileStr;
    WarnCopyDlgData[WDLG_DSTFILEBTN].Data=DestFileStr;

    MakeDialogItems(WarnCopyDlgData,WarnCopyDlg);
    char FullSrcName[NM*2];
    ConvertNameToFull(SrcName,FullSrcName,sizeof(FullSrcName));
    LPCSTR WFN[2]={FullSrcName,DestName};
    Dialog WarnDlg(WarnCopyDlg,sizeof(WarnCopyDlg)/sizeof(WarnCopyDlg[0]),WarnDlgProc,(LONG_PTR)&WFN);
    WarnDlg.SetDialogMode(DMODE_WARNINGSTYLE);
    WarnDlg.SetPosition(-1,-1,WARN_DLG_WIDTH,WARN_DLG_HEIGHT);
    WarnDlg.SetHelp("CopyAskOverwrite");
    WarnDlg.Process();

    switch(WarnDlg.GetExitCode())
    {
    case WDLG_OVERWRITE:
      MsgCode=WarnCopyDlg[WDLG_CHECKBOX].Selected?1:0;
      break;
    case WDLG_SKIP:
      MsgCode=WarnCopyDlg[WDLG_CHECKBOX].Selected?3:2;
      break;
    case WDLG_APPEND:
      MsgCode=WarnCopyDlg[WDLG_CHECKBOX].Selected?5:4;
      break;
    case -1:
    case -2:
    case WDLG_CANCEL:
      MsgCode=6;
      break;
    }
      }
    }
  }

  switch(MsgCode)
  {
    case 1:
      OvrMode=1;
      break;
    case 2:
      RetCode=COPY_NEXT;
      return(FALSE);
    case 3:
      OvrMode=3;
      RetCode=COPY_NEXT;
      return(FALSE);
    case 4:
      Append=TRUE;
      break;
    case 5:
      Append=TRUE;
      OvrMode=5;
      RetCode=COPY_NEXT;
      break;
    case -1:
    case -2:
    case 6:
      RetCode=COPY_CANCEL;
      return(FALSE);
  }
  if ((DestAttr & FA_RDONLY) && !(ShellCopy::Flags&FCOPY_OVERWRITENEXT))
  {
    int MsgCode;
    if (SameName)
      MsgCode=0;
    else
      if (ReadOnlyOvrMode!=-1)
        MsgCode=ReadOnlyOvrMode;
      else
      {
        if (!DestDataFilled)
        {
          memset(&DestData,0,sizeof(DestData));
          GetFileWin32FindData(DestName,&DestData);
        }
        char SrcFileStr[512],DestFileStr[512],DateText[20],TimeText[20];

        unsigned __int64 SrcSize=MKUINT64(SrcData.nFileSizeHigh,SrcData.nFileSizeLow);
        char SrcSizeText[20];
        _i64toa(SrcSize,SrcSizeText,10);
        unsigned __int64 DestSize=MKUINT64(DestData.nFileSizeHigh,DestData.nFileSizeLow);
        char DestSizeText[20];
        _i64toa(DestSize,DestSizeText,10);

        ConvertDate(SrcData.ftLastWriteTime,DateText,TimeText,8,FALSE,FALSE,TRUE,TRUE);
        sprintf(SrcFileStr,"%-17s %20.20s %s %s",MSG(MCopySource),SrcSizeText,DateText,TimeText);
        ConvertDate(DestData.ftLastWriteTime,DateText,TimeText,8,FALSE,FALSE,TRUE,TRUE);
        sprintf(DestFileStr,"%-17s %20.20s %s %s",MSG(MCopyDest),DestSizeText,DateText,TimeText);

    WarnCopyDlgData[WDLG_SRCFILEBTN].Data=SrcFileStr;
    WarnCopyDlgData[WDLG_DSTFILEBTN].Data=DestFileStr;

    WarnCopyDlgData[WDLG_TEXT].Data=MSG(MCopyFileRO);
    WarnCopyDlgData[WDLG_OVERWRITE].Data=MSG(MCopyContinue);
    WarnCopyDlgData[WDLG_APPEND].Flags|=DIF_DISABLE|DIF_HIDDEN;

    MakeDialogItems(WarnCopyDlgData,WarnCopyDlg);
    char SrcName[2*NM];
    ConvertNameToFull(SrcData.cFileName,SrcName,sizeof(SrcName));
    LPCSTR WFN[2]={SrcName,DestName};
    Dialog WarnDlg(WarnCopyDlg,sizeof(WarnCopyDlg)/sizeof(WarnCopyDlg[0]),WarnDlgProc,(LONG_PTR)&WFN);
    WarnDlg.SetDialogMode(DMODE_WARNINGSTYLE);
    WarnDlg.SetPosition(-1,-1,WARN_DLG_WIDTH,WARN_DLG_HEIGHT);
    WarnDlg.SetHelp("CopyFiles");
    WarnDlg.Process();

    switch(WarnDlg.GetExitCode())
    {
    case WDLG_OVERWRITE:
      MsgCode=WarnCopyDlg[WDLG_CHECKBOX].Selected?1:0;
      break;
    case WDLG_SKIP:
      MsgCode=WarnCopyDlg[WDLG_CHECKBOX].Selected?3:2;
      break;
    case -1:
    case -2:
    case WDLG_CANCEL:
      MsgCode=6;
      break;
    }
      }
    switch(MsgCode)
    {
      case 1:
        ReadOnlyOvrMode=1;
        break;
      case 2:
        RetCode=COPY_NEXT;
        return(FALSE);
      case 3:
        ReadOnlyOvrMode=3;
        RetCode=COPY_NEXT;
        return(FALSE);
      case -1:
      case -2:
      case 6:
        RetCode=COPY_CANCEL;
        return(FALSE);
    }
  }
  if (!SameName && (DestAttr & (FA_RDONLY|FA_HIDDEN|FA_SYSTEM)))
    SetFileAttributes(DestName,FILE_ATTRIBUTE_NORMAL);
  return(TRUE);
}


int ShellCopy::GetSecurity(const char *FileName,SECURITY_ATTRIBUTES &sa)
{
  _LOGCOPYR(CleverSysLog Clev("ShellCopy::GetSecurity()"));
  _LOGCOPYR(SysLog("Params: FileName='%s'",FileName));
  char AnsiName[NM];
  SECURITY_INFORMATION si=DACL_SECURITY_INFORMATION;
  SECURITY_DESCRIPTOR *sd=(SECURITY_DESCRIPTOR *)sddata;
  DWORD Needed;
  SetFileApisTo(APIS2ANSI);
  FAR_OemToChar(FileName,AnsiName);
  BOOL RetSec=GetFileSecurity(AnsiName,si,sd,SDDATA_SIZE,&Needed);
  int LastError=GetLastError();
  _LOGCOPYR(SysLog("LastError=%u Attr=0x%08X",LastError,GetFileAttributes(AnsiName)));
  SetFileApisTo(APIS2OEM);
  if (!RetSec)
  {
    sd=NULL;
    if (LastError!=ERROR_SUCCESS && LastError!=ERROR_FILE_NOT_FOUND &&
        LastError!=ERROR_CALL_NOT_IMPLEMENTED &&
        Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),
                MSG(MCannotGetSecurity),FileName,MSG(MOk),MSG(MCancel))==1)
      return(FALSE);
  }
  sa.nLength=sizeof(SECURITY_ATTRIBUTES);
  sa.lpSecurityDescriptor=sd;
  sa.bInheritHandle=FALSE;
  return(TRUE);
}


int ShellCopy::SetSecurity(const char *FileName,const SECURITY_ATTRIBUTES &sa)
{
  _LOGCOPYR(CleverSysLog Clev("ShellCopy::SetSecurity()"));
  _LOGCOPYR(SysLog("Params: FileName='%s'",FileName));
  char AnsiName[NM];
  SECURITY_INFORMATION si=DACL_SECURITY_INFORMATION;
  SetFileApisTo(APIS2ANSI);
  FAR_OemToChar(FileName,AnsiName);
  BOOL RetSec=SetFileSecurity(AnsiName,si,(PSECURITY_DESCRIPTOR)sa.lpSecurityDescriptor);
  int LastError=GetLastError();
  _LOGCOPYR(SysLog("LastError=%u",LastError));
  SetFileApisTo(APIS2OEM);
  if (!RetSec)
  {
    if (LastError!=ERROR_SUCCESS && LastError!=ERROR_FILE_NOT_FOUND &&
        LastError!=ERROR_CALL_NOT_IMPLEMENTED &&
        Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),
                MSG(MCannotSetSecurity),FileName,MSG(MOk),MSG(MCancel))==1)
      return(FALSE);
  }
  return(TRUE);
}


BOOL ShellCopySecuryMsg(const char *Name)
{
  static clock_t PrepareSecuryStartTime=0;
  static int Width=30;
  int WidthTemp;
  char OutFileName[NM];

  if (Name == NULL || *Name == 0 || (static_cast<DWORD>(clock() - PrepareSecuryStartTime) > Opt.ShowTimeoutDACLFiles))
  {
    if(Name && *Name)
    {
      PrepareSecuryStartTime = clock();     // Первый файл рисуется всегда
      WidthTemp=Max((int)strlen(Name),(int)30);
    }
    else
      Width=WidthTemp=30;

    if(WidthTemp > WidthNameForMessage)
      WidthTemp=WidthNameForMessage; // ширина месага - 38%
    if(WidthTemp >= sizeof(OutFileName)-4)
      WidthTemp=sizeof(OutFileName)-5;
    if(Width < WidthTemp)
      Width=WidthTemp;
#if 0
    if(Name) //???
    {
      xstrncpy(OutFileName,Name,sizeof(OutFileName)-1);
      TruncPathStr(OutFileName,Width);
      CenterStr(OutFileName,OutFileName,Width+4);

      Message(0,0,MSG(MMoveDlgTitle),MSG(MCopyPrepareSecury),OutFileName);
    }
#else
    xstrncpy(OutFileName,NullToEmpty(Name),sizeof(OutFileName)-1);
    TruncPathStr(OutFileName,Width);
    CenterStr(OutFileName,OutFileName,Width+4);

    Message(0,0,MSG(MMoveDlgTitle),MSG(MCopyPrepareSecury),OutFileName);
#endif

    if(CheckForEscSilent())
    {
      if(ConfirmAbortOp())
        return FALSE;
    }
  }
  PreRedrawItem preRedrawItem=PreRedraw.Peek();
  preRedrawItem.Param.Param1=static_cast<void*>(const_cast<char*>(Name));
  PreRedraw.SetParam(preRedrawItem.Param);
  return TRUE;
}

static void PR_ShellCopySecuryMsg(void)
{
  PreRedrawItem preRedrawItem=PreRedraw.Peek();
  ShellCopySecuryMsg(static_cast<const char*>(preRedrawItem.Param.Param1));
}

int ShellCopy::SetRecursiveSecurity(const char *FileName,const SECURITY_ATTRIBUTES &sa)
{
  _LOGCOPYR(CleverSysLog Clev("ShellCopy::SetRecursiveSecurity()"));
  _LOGCOPYR(SysLog("Params: FileName='%s'",FileName));
  if(SetSecurity(FileName,sa))
  {
    if(::GetFileAttributes(FileName)&FILE_ATTRIBUTE_DIRECTORY)
    {
      //SaveScreen SaveScr; //????
      //SetCursorType(FALSE,0);
      //ShellCopySecuryMsg("");

      char FullName[NM*2];
      WIN32_FIND_DATA SrcData;
      ScanTree ScTree(TRUE,TRUE,ShellCopy::Flags&FCOPY_COPYSYMLINKCONTENTS);
      ScTree.SetFindPath(FileName,"*.*",FSCANTREE_FILESFIRST);
      while (ScTree.GetNextName(&SrcData,FullName, sizeof (FullName)-1))
      {
        if(!ShellCopySecuryMsg(FullName))
          break;
        if(!SetSecurity(FullName,sa))
        {
          return FALSE;
        }
      }
    }
    return TRUE;
  }
  _LOGCOPYR(else SysLog("[%d] SetSecurity failed",__LINE__));
  return FALSE;
}


int ShellCopy::ShellSystemCopy(const char *SrcName,const char *DestName,const WIN32_FIND_DATA &SrcData)
{
  _LOGCOPYR(CleverSysLog Clev("ShellCopy::ShellSystemCopy()"));
  _LOGCOPYR(SysLog("Params: SrcName='%s', DestName='%s'",SrcName, DestName));
  _LOGCOPYR(WIN32_FIND_DATA_Dump("",SrcData));
  {
    _LOGCOPYR(static char Root[1024]);
    _LOGCOPYR(char lpRootPathName[NM]="");
    _LOGCOPYR(char lpVolumeNameBuffer[NM]="");
    _LOGCOPYR(char lpFileSystemNameBuffer[NM]="");
    _LOGCOPYR(DWORD lpVolumeSerialNumber=0);
    _LOGCOPYR(DWORD lpMaximumComponentLength=0);
    _LOGCOPYR(DWORD lpFileSystemFlags=0);
    _LOGCOPYR(GetPathRoot(SrcName,Root));
    _LOGCOPYR(GetVolumeInformation(Root,lpVolumeNameBuffer,sizeof(lpVolumeNameBuffer),
                                             &lpVolumeSerialNumber,&lpMaximumComponentLength,&lpFileSystemFlags,
                                             lpFileSystemNameBuffer,sizeof(lpFileSystemNameBuffer)));
    _LOGCOPYR(GetVolumeInformation_Dump("Src",Root,lpVolumeNameBuffer,sizeof(lpVolumeNameBuffer),
                                             lpVolumeSerialNumber,lpMaximumComponentLength,lpFileSystemFlags,
                                             lpFileSystemNameBuffer,sizeof(lpFileSystemNameBuffer),NULL));
    _LOGCOPYR(GetPathRoot(DestName,Root));
    _LOGCOPYR(GetVolumeInformation(Root,lpVolumeNameBuffer,sizeof(lpVolumeNameBuffer),
                                             &lpVolumeSerialNumber,&lpMaximumComponentLength,&lpFileSystemFlags,
                                             lpFileSystemNameBuffer,sizeof(lpFileSystemNameBuffer)));
    _LOGCOPYR(GetVolumeInformation_Dump("Dest",Root,lpVolumeNameBuffer,sizeof(lpVolumeNameBuffer),
                                             lpVolumeSerialNumber,lpMaximumComponentLength,lpFileSystemFlags,
                                             lpFileSystemNameBuffer,sizeof(lpFileSystemNameBuffer),NULL));
  }

  SECURITY_ATTRIBUTES sa;
  if ((ShellCopy::Flags&FCOPY_COPYSECURITY) && !GetSecurity(SrcName,sa))
    return(COPY_CANCEL);

  //// // _LOGCOPYR(SysLog("[%p] ShellCopy::ShellSystemCopy('%s','%s',..)",this,SrcName,DestName));
  ShellCopyMsg(SrcName,DestName,MSG_LEFTALIGN|MSG_KEEPBACKGROUND);

  if (Init_CopyFileEx())
  {
    BOOL Cancel=0;
    TotalCopiedSizeEx=TotalCopiedSize;
#ifndef COPY_FILE_ALLOW_DECRYPTED_DESTINATION
#define COPY_FILE_ALLOW_DECRYPTED_DESTINATION 0x00000008
#endif
    // COPY_FILE_ALLOW_DECRYPTED_DESTINATION
    if (!FAR_CopyFileEx(SrcName,DestName,(void *)CopyProgressRoutine,NULL,&Cancel,
         ShellCopy::Flags&(FCOPY_DECRYPTED_DESTINATION)?COPY_FILE_ALLOW_DECRYPTED_DESTINATION:0))
    {
      ShellCopy::Flags&=~FCOPY_DECRYPTED_DESTINATION;
      return (_localLastError=GetLastError())==ERROR_REQUEST_ABORTED ? COPY_CANCEL:COPY_FAILURE;
    }
    ShellCopy::Flags&=~FCOPY_DECRYPTED_DESTINATION;
  }
  else
  {
    if (ShowTotalCopySize)
    {
      unsigned __int64 AddSize=MKUINT64(SrcData.nFileSizeHigh,SrcData.nFileSizeLow);
      TotalCopiedSize += AddSize;
      CurCopiedSize = AddSize;
      ShowBar(TotalCopiedSize,TotalCopySize,true);
      ShowTitle(FALSE);
    }
    // Здесь ХЗ... могут быть траблы там, где по каким то причинам нету CopyFileExA
    if (!FAR_CopyFile(SrcName,DestName,FALSE))
      return COPY_FAILURE;
  }

  if ((ShellCopy::Flags&FCOPY_COPYSECURITY) && !SetSecurity(DestName,sa))
    return(COPY_CANCEL);
  return(COPY_SUCCESS);
}


#define PROGRESS_CONTINUE  0
#define PROGRESS_CANCEL    1
#if defined(__BORLANDC__)
#pragma warn -par
#endif

DWORD WINAPI CopyProgressRoutine(LARGE_INTEGER TotalFileSize,
      LARGE_INTEGER TotalBytesTransferred,LARGE_INTEGER StreamSize,
      LARGE_INTEGER StreamBytesTransferred,DWORD dwStreamNumber,
      DWORD dwCallbackReason,HANDLE hSourceFile,HANDLE hDestinationFile,
      LPVOID lpData)
{
  // // _LOGCOPYR(CleverSysLog clv("CopyProgressRoutine"));
  // // _LOGCOPYR(SysLog("dwStreamNumber=%d",dwStreamNumber));

  unsigned __int64 TransferredSize=MKUINT64(TotalBytesTransferred.u.HighPart,TotalBytesTransferred.u.LowPart);
  unsigned __int64 TotalSize=MKUINT64(TotalFileSize.u.HighPart,TotalFileSize.u.LowPart);

  int AbortOp = FALSE;
  BOOL IsChangeConsole=OrigScrX != ScrX || OrigScrY != ScrY;
  if (CheckForEscSilent())
  {
    AbortOp = ConfirmAbortOp();
    IsChangeConsole=TRUE; // !!! Именно так; для того, чтобы апдейтить месаг
  }

  IsChangeConsole=ShellCopy::CheckAndUpdateConsole(IsChangeConsole);

  if(IsChangeConsole)
  {
    OrigScrX=ScrX;
    OrigScrY=ScrY;
    // // _LOGCOPYR(SysLog("IsChangeConsole 1"));
    ShellCopy::PR_ShellCopyMsg();
  }

  CurCopiedSize = TransferredSize;

  if (IsChangeConsole || (CurCopiedSize == TotalSize) || (clock() - LastShowTime > COPY_TIMEOUT))
  {
    ShellCopy::ShowBar(TransferredSize,TotalSize,FALSE);
    if (ShowTotalCopySize && dwStreamNumber==1)
    {
      TotalCopiedSize=TotalCopiedSizeEx+CurCopiedSize;
      ShellCopy::ShowBar(TotalCopiedSize,TotalCopySize,true);
      ShellCopy::ShowTitle(FALSE);
    }
  }
  return(AbortOp ? PROGRESS_CANCEL:PROGRESS_CONTINUE);
}
#if defined(__BORLANDC__)
#pragma warn +par
#endif


int ShellCopy::IsSameDisk(const char *SrcPath,const char *DestPath)
{
  return CheckDisksProps(SrcPath,DestPath,CHECKEDPROPS_ISSAMEDISK);
}


bool ShellCopy::CalcTotalSize()
{
  char SelName[NM],SelShortName[NM];
  int FileAttr;
  unsigned __int64 FileSize;

  PreRedraw.Push(ShellCopy::PR_ShellCopyMsg);

  // Для фильтра
  WIN32_FIND_DATA fd;

  TotalCopySize=CurCopiedSize=0;
  TotalFilesToProcess = 0;

  ShellCopyMsg(NULL,"",MSG_LEFTALIGN);

  SrcPanel->GetSelName(NULL,FileAttr);
  while (SrcPanel->GetSelName(SelName,FileAttr,SelShortName,&fd))
  {
    if ((FileAttr&FILE_ATTRIBUTE_REPARSE_POINT) && !(ShellCopy::Flags&FCOPY_COPYSYMLINKCONTENTS))
        continue;

    if (FileAttr & FILE_ATTRIBUTE_DIRECTORY)
    {
      {
        unsigned long DirCount,FileCount,ClusterSize;
        unsigned __int64 CompressedSize,RealFileSize;
        ShellCopyMsg(NULL,SelName,MSG_LEFTALIGN|MSG_KEEPBACKGROUND);
        int __Ret=GetDirInfo("",SelName,DirCount,FileCount,FileSize,CompressedSize,
                        RealFileSize,ClusterSize,0xffffffff,
                        Filter,
                        (ShellCopy::Flags&FCOPY_COPYSYMLINKCONTENTS?GETDIRINFO_SCANSYMLINK:0)|
                        (UseFilter?GETDIRINFO_USEFILTER:0));
        if (__Ret <= 0)
        {
          ShowTotalCopySize=false;
          PreRedraw.Pop();
          return(false);
        }

        if(FileCount > 0)
        {
          TotalCopySize+=FileSize;
          TotalFilesToProcess += FileCount;
        }
      }
    }
    else
    {
      if (UseFilter)
      {
        if (!Filter->FileInFilter(&fd))
          continue;
      }


      if ((FileSize=SrcPanel->GetLastSelectedSize()) != (unsigned __int64)-1)
      {
        TotalCopySize+=FileSize;
        TotalFilesToProcess++;
      }
    }
  }
  // INFO: Это для варианта, когда "ВСЕГО = общий размер * количество целей"
  TotalCopySize=TotalCopySize*(__int64)CountTarget;

  InsertCommas(TotalCopySize,TotalCopySizeText,sizeof(TotalCopySizeText));
  PreRedraw.Pop();
  return(true);
}


void ShellCopy::ShowTitle(int FirstTime)
{
  if (ShowTotalCopySize && !FirstTime)
  {
    StaticCopyTitle->Set("{%d%%} %s",ToPercent64(TotalCopiedSize,TotalCopySize),StaticMove ? MSG(MCopyMovingTitle):MSG(MCopyCopyingTitle));
  }
}


/* $ 25.05.2002 IS
 + Всегда работаем с реальными _длинными_ именами, в результате чего
   отлавливается ситуация, когда
   Src="D:\Program Files\filename"
   Dest="D:\PROGRA~1\filename"
   ("D:\PROGRA~1" - короткое имя для "D:\Program Files")
   считается, что имена тоже одинаковые, а раньше считалось,
   что они разные (функция не знала, что и в первом, и во втором случае
   путь один и тот же)
 ! Оптимизация - "велосипед" заменен на DeleteEndSlash
 ! Убираем всю самодеятельность по проверке имен с разным
   регистром из функции прочь, потому что это нужно делать только при
   переименовании, а функция вызывается и при копировании тоже. Все это
   должно обрабатываться не в ней, а там же, где и RenameToShortName.
   Теперь функция вернет 1, для случая имен src=path\filename,
   dest=path\filename (раньше возвращала 2 - т.е. сигнал об ошибке).
*/
int ShellCopy::CmpFullNames(const char *Src,const char *Dest)
{
  _LOGCOPYR(CleverSysLog Clev("ShellCopy::CmpFullNames()"));
  _LOGCOPYR(SysLog("Params: Src='%s', Dest='%s'",Src, Dest));
  char SrcFullName[1024],DestFullName[1024];
  int I;

  // получим полные пути с учетом символических связей
  if (ConvertNameToReal(Src,SrcFullName, sizeof(SrcFullName)) >= sizeof(SrcFullName))
    return 2;
  if (ConvertNameToReal(Dest,DestFullName, sizeof(DestFullName)) >= sizeof(DestFullName))
    return 2;

  // уберем мусор из имен
  for (I=(int)strlen(SrcFullName)-1;I>0 && SrcFullName[I]=='.';I--)
    SrcFullName[I]=0;
  DeleteEndSlash(SrcFullName);
  _LOGCOPYR(SysLog("SrcFullName='%s'",SrcFullName));
  for (I=(int)strlen(DestFullName)-1;I>0 && DestFullName[I]=='.';I--)
    DestFullName[I]=0;
  DeleteEndSlash(DestFullName);
  _LOGCOPYR(SysLog("DestFullName='%s'",DestFullName));

/*
  if (LocalStricmp(SrcFullName,DestFullName)!=0)
    return(0);

  return(strcmp(PointToName(SrcFullName),PointToName(DestFullName))==0 ? 2:1);
*/
  // избавимся от коротких имен
  if(IsLocalPath(SrcFullName))
  {
    I=RawConvertShortNameToLongName(SrcFullName,SrcFullName,sizeof(SrcFullName));
    _LOGCOPYR(SysLog("RawConvertShortNameToLongName() -> SrcFullName='%s'",SrcFullName));
    if(!I || I>=sizeof(SrcFullName))
      return 2;
  }
  if(IsLocalPath(DestFullName))
  {
    I=RawConvertShortNameToLongName(DestFullName,DestFullName,sizeof(DestFullName));
    _LOGCOPYR(SysLog("RawConvertShortNameToLongName() -> DestFullName='%s'",DestFullName));
    if(!I || I>=sizeof(DestFullName))
      return 2;
  }

  _LOGCOPYR(SysLog("return LocalStricmp(SrcFullName,DestFullName)='%d'",LocalStricmp(SrcFullName,DestFullName)));
  return LocalStricmp(SrcFullName,DestFullName)==0;
}
/* IS $ */

int ShellCopy::CmpFullPath(const char *Src,const char *Dest)
{
  _LOGCOPYR(CleverSysLog Clev("ShellCopy::CmpFullPath()"));
  _LOGCOPYR(SysLog("Params: Src='%s', Dest='%s'",Src, Dest));
  char SrcFullName[1024],DestFullName[1024];
  int I;

  GetParentFolder(Src,SrcFullName, sizeof(SrcFullName));
  GetParentFolder(Dest,DestFullName, sizeof(DestFullName));

  // уберем мусор из имен
  for (I=(int)strlen(SrcFullName)-1;I>0 && SrcFullName[I]=='.';I--)
    SrcFullName[I]=0;
  DeleteEndSlash(SrcFullName);
  _LOGCOPYR(SysLog("SrcFullName='%s'",SrcFullName));
  for (I=(int)strlen(DestFullName)-1;I>0 && DestFullName[I]=='.';I--)
    DestFullName[I]=0;
  DeleteEndSlash(DestFullName);
  _LOGCOPYR(SysLog("DestFullName='%s'",DestFullName));

/*
  if (LocalStricmp(SrcFullName,DestFullName)!=0)
    return(0);

  return(strcmp(PointToName(SrcFullName),PointToName(DestFullName))==0 ? 2:1);
*/
  // избавимся от коротких имен
  if(IsLocalPath(SrcFullName))
  {
    I=RawConvertShortNameToLongName(SrcFullName,SrcFullName,sizeof(SrcFullName));
    _LOGCOPYR(SysLog("RawConvertShortNameToLongName() -> SrcFullName='%s'",SrcFullName));
    if(!I || I>=sizeof(SrcFullName))
      return 2;
  }
  if(IsLocalPath(DestFullName))
  {
    I=RawConvertShortNameToLongName(DestFullName,DestFullName,sizeof(DestFullName));
    _LOGCOPYR(SysLog("RawConvertShortNameToLongName() -> DestFullName='%s'",DestFullName));
    if(!I || I>=sizeof(DestFullName))
      return 2;
  }

  _LOGCOPYR(SysLog("return LocalStricmp(SrcFullName,DestFullName)='%d'",LocalStricmp(SrcFullName,DestFullName)));
  return LocalStricmp(SrcFullName,DestFullName)==0;
}

char *ShellCopy::GetParentFolder(const char *Src, char *Dest, int LenDest)
{
  char DestFullName[2048];
  if (ConvertNameToReal(Src,DestFullName, sizeof(DestFullName)) >= sizeof(DestFullName))
    return NULL;
  char *Ptr=strrchr(DestFullName,'\\');
  if(Ptr)
    *Ptr=0;
  xstrncpy(Dest,DestFullName,LenDest);
  return Dest;
}

// Кусок для создания SymLink для каталогов.
int ShellCopy::MkSymLink(const char *SelName,const char *Dest,ReparsePointTypes LinkType,DWORD Flags)
{
  _LOGCOPYR(CleverSysLog Clev("ShellCopy::MkSymLink()"));
  _LOGCOPYR(SysLog("Params: SelName='%s', Dest='%s', Flags=0x%08X",SelName,Dest,Flags));
  if(SelName && *SelName && Dest && *Dest)
  {
    char SrcFullName[NM], DestFullName[NM], SelOnlyName[NM];
    char MsgBuf[NM],MsgBuf2[NM];

    // выделим имя
    xstrncpy(SelOnlyName,SelName,sizeof(SelOnlyName)-1);
    if(SelOnlyName[strlen(SelOnlyName)-1] == '\\')
      SelOnlyName[strlen(SelOnlyName)-1]=0;
    char *PtrSelName=strrchr(SelOnlyName,'\\');
    if(!PtrSelName)
      PtrSelName=SelOnlyName;
    else
      ++PtrSelName;

    if(SelName[1] == ':' && (SelName[2] == 0 || SelName[2] == '\\' && SelName[3] == 0)) // C: или C:/
    {
//      if(Flags&FCOPY_VOLMOUNT)
      {
        strcpy(SrcFullName,SelName);
        AddEndSlash(SrcFullName);
      }
/*
    Вот здесь - ну очень умное поведение!
    Т.е. если в качестве SelName передали "C:", то в этом куске происходит
    коррекция типа линка - с symlink`а на volmount
*/
      LinkType=RP_VOLMOUNT;
      _LOGCOPYR(SysLog("Flags=0x%08X (transfer SymLink to VolMount)",Flags));
    }
    else
      if (ConvertNameToFull(SelName,SrcFullName, sizeof(SrcFullName)) >= sizeof(SrcFullName))
        return 0;
    _LOGCOPYR(SysLog("Src: ConvertNameToReal('%s','%s')",SelName,SrcFullName));

    if (ConvertNameToFull(Dest,DestFullName, sizeof(DestFullName)) >= sizeof(DestFullName))
      return 0;
    _LOGCOPYR(SysLog("Dst: ConvertNameToFull('%s','%s')",Dest,DestFullName));

//    char *EndDestFullName=DestFullName+strlen(DestFullName);
    if(DestFullName[strlen(DestFullName)-1] == '\\')
    {
      if(LinkType!=RP_VOLMOUNT)
      {
        // AddEndSlash(DestFullName);
        xstrncat(DestFullName,PtrSelName,sizeof(DestFullName)-1);
      }
      else
      {
        // если таржед не задан - применяется стд. имя "Disk_%c"
        sprintf(DestFullName+strlen(DestFullName),"Disk_%c",*SelName);
      }
    }

    if(LinkType==RP_VOLMOUNT)
    {
      AddEndSlash(SrcFullName);
      AddEndSlash(DestFullName);
    }

    int JSAttr=GetFileAttributes(DestFullName);
    _LOGCOPYR(SysLog("%d DestFullName='%s' JSAttr=0x%08X",__LINE__,DestFullName,JSAttr));
    if(JSAttr != -1) // Существует такой?
    {
      if((JSAttr&FILE_ATTRIBUTE_DIRECTORY)!=FILE_ATTRIBUTE_DIRECTORY)
      {
        if(!(Flags&FCOPY_NOSHOWMSGLINK))
        {
          Message(MSG_DOWN|MSG_WARNING,1,MSG(MError),
                MSG(MCopyCannotCreateJunctionToFile),
                DestFullName,MSG(MOk));
        }
        return 0;
      }

      if(CheckFolder(DestFullName) == CHKFLD_NOTEMPTY) // а пустой?
      {
        _LOGCOPYR(SysLog("CheckFolder('%s') == CHKFLD_NOTEMPTY",DestFullName));
        // не пустой, ну что же, тогда пробуем сделать dest\srcname
        AddEndSlash(DestFullName);
        if(LinkType==RP_VOLMOUNT)
        {
          char TmpName[NM];
          sprintf(TmpName,MSG(MCopyMountName),*SelName);
          strcat(DestFullName,TmpName);
          AddEndSlash(DestFullName);
        }
        else
          strcat(DestFullName,PtrSelName);

        int JSAttr=GetFileAttributes(DestFullName);
         _LOGCOPYR(SysLog("%d DestFullName='%s' JSAttr=0x%08X",__LINE__,DestFullName,JSAttr));
        if(JSAttr != -1) // И такой тоже есть???
        {
          _LOGCOPYR(SysLog("Ops! Folder '%s' Exist!",DestFullName));
          if(CheckFolder(DestFullName) == CHKFLD_NOTEMPTY) // а пустой?
          {
            _LOGCOPYR(SysLog("%d CheckFolder('%s') == CHKFLD_NOTEMPTY",__LINE__,DestFullName));
            if(!(Flags&FCOPY_NOSHOWMSGLINK))
            {
              if(LinkType==RP_VOLMOUNT)
              {
                sprintf(MsgBuf,MSG(MCopyMountVolFailed), SelName);
                sprintf(MsgBuf2,MSG(MCopyMountVolFailed2), DestFullName);
                Message(MSG_DOWN|MSG_WARNING,1,MSG(MError),
                   MsgBuf,
                   MsgBuf2,
                   MSG(MCopyFolderNotEmpty),
                   MSG(MOk));
              }
              else
                Message(MSG_DOWN|MSG_WARNING,1,MSG(MError),
                      MSG(MCopyCannotCreateLink),DestFullName,
                      MSG(MCopyFolderNotEmpty),MSG(MOk));
            }
            _LOGCOPYR(SysLog("return 0 -> %d Unequivocally into mortuary",__LINE__));
            return 0; // однозначно в морг
          }
        }
        else // создаем.
        {
           _LOGCOPYR(SysLog("%d CreateDirectory('%s')",__LINE__,DestFullName));
          if (CreateDirectory(DestFullName,NULL))
            TreeList::AddTreeName(DestFullName);
          else
            CreatePath(DestFullName);
        }
        if(GetFileAttributes(DestFullName) == -1) // так, все очень даже плохо.
        {
          if(!(Flags&FCOPY_NOSHOWMSGLINK))
          {
            Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),
                      MSG(MCopyCannotCreateFolder),
                      DestFullName,MSG(MOk));
          }
          _LOGCOPYR(SysLog("return 0 -> %d So, all very much even is bad: GetFileAttributes('%s') == -1",__LINE__,DestFullName));
          return 0;
        }
      }
    }
    else
    {
      if(LinkType==RP_SYMLINKFILE || LinkType==RP_SYMLINKDIR)
      {
        // в этом случае создается путь, но не сам каталог
        char Path[1024];
        strcpy(Path,DestFullName);
        char *tmp=PointToName(Path);
        if(tmp)
        {
          *tmp=0;
          if(GetFileAttributes(Path)==INVALID_FILE_ATTRIBUTES)
            CreatePath(Path);
        }
      }
      else
      {
        bool CreateDir=true;
        if(LinkType==RP_EXACTCOPY)
        {
          // в этом случае создается или каталог, или пустой файл
          DWORD dwSrcAttr=GetFileAttributes(SrcFullName);
          if(dwSrcAttr!=INVALID_FILE_ATTRIBUTES && !(dwSrcAttr&FILE_ATTRIBUTE_DIRECTORY))
            CreateDir=false;
        }
        if(CreateDir)
        {
          if (CreateDirectory(DestFullName,NULL))
            TreeList::AddTreeName(DestFullName);
          else
            CreatePath(DestFullName);
        }
        else
        {
          char Path[1024];
          strcpy(Path,DestFullName);
          char *tmp=PointToName(Path);
          if(tmp)
          {
            *tmp=0;
            // создаём
            if(GetFileAttributes(Path)==INVALID_FILE_ATTRIBUTES)
              CreatePath(Path);
            HANDLE hFile=FAR_CreateFile(DestFullName,0,0,0,CREATE_NEW,GetFileAttributes(SrcFullName),0);
            if(hFile!=INVALID_HANDLE_VALUE)
            {
              CloseHandle(hFile);
            }
          }
        }
        if(GetFileAttributes(DestFullName) == INVALID_FILE_ATTRIBUTES) // так. все очень даже плохо.
        {
          if(!(Flags&FCOPY_NOSHOWMSGLINK))
          {
            Message(MSG_DOWN|MSG_WARNING,1,MSG(MError),
                    MSG(MCopyCannotCreateLink),DestFullName,MSG(MOk));
          }
          return 0;
        }
      }
    }
    if(LinkType!=RP_VOLMOUNT)
    {
      if(CreateReparsePoint(SrcFullName,DestFullName,LinkType))
      {
        _LOGCOPYR(SysLog("Ok: CreateReparsePoint('%s','%s')",SrcFullName,DestFullName));
        return 1;
      }
      else
      {
        if(!(Flags&FCOPY_NOSHOWMSGLINK))
        {
          Message(MSG_DOWN|MSG_WARNING,1,MSG(MError),
                 MSG(MCopyCannotCreateLink),DestFullName,MSG(MOk));
        }
        _LOGCOPYR(SysLog("Fail: CreateReparsePoint('%s','%s')",SrcFullName,DestFullName));
        return 0;
      }
    }
    else
    {
      int ResultVol=CreateVolumeMountPoint(SrcFullName,DestFullName);
      if(!ResultVol)
      {
        _LOGCOPYR(SysLog("Ok: CreateVolumeMountPoint('%s','%s')",SrcFullName,DestFullName));
        return 1;
      }
      else
      {
        if(!(Flags&FCOPY_NOSHOWMSGLINK))
        {
          switch(ResultVol)
          {
            case 1:
              sprintf(MsgBuf,MSG(MCopyRetrVolFailed), SelName);
              break;
            case 2:
              sprintf(MsgBuf,MSG(MCopyMountVolFailed), SelName);
              sprintf(MsgBuf2,MSG(MCopyMountVolFailed2), DestFullName);
              break;
            case 3:
              strcpy(MsgBuf,MSG(MCopyCannotSupportVolMount));
              break;
          }

          if(ResultVol == 2)
            Message(MSG_DOWN|MSG_WARNING,1,MSG(MError),
              MsgBuf,
              MsgBuf2,
              MSG(MOk));
          else
            Message(MSG_DOWN|MSG_WARNING,1,MSG(MError),
              MSG(MCopyCannotCreateVolMount),
              MsgBuf,
              MSG(MOk));
        }
        _LOGCOPYR(SysLog("Fail: CreateVolumeMountPoint('%s','%s')",SrcFullName,DestFullName));
        return 0;
      }
    }
  }
  _LOGCOPYR(SysLog("return 2 -> %d",__LINE__));
  return 2;
}

/*
  Оболочка вокруг SetFileAttributes() для
  корректного выставления атрибутов
*/
int ShellCopy::ShellSetAttr(const char *Dest,DWORD Attr)
{
  _LOGCOPYR(CleverSysLog Clev("ShellCopy::ShellSetAttr()"));
  _LOGCOPYR(SysLog("Params: Dest='%s', Attr=0x%08X",Dest,Attr));
  char Root[1024];
  char FSysNameDst[NM];
  DWORD FileSystemFlagsDst;

  ConvertNameToFull(Dest,Root, sizeof(Root));
  GetPathRoot(Root,Root);
  if(GetFileAttributes(Root) == -1) // Неудача, когда сетевой путь, да еще и симлинк
  { // ... в этом случае проверим AS IS
    ConvertNameToFull(Dest,Root, sizeof(Root));
    GetPathRootOne(Root,Root);
    if(GetFileAttributes(Root) == -1)
    {
      _LOGCOPYR(SysLog("return 0 -> %d GetFileAttributes('%s') == -1",__LINE__,Root));
      return FALSE;
    }
  }

  /* 18.06.2002 VVM
    ! Даже если не смогли получить информацию о томе - попытаемся выставить атрибуты
      У меня на новеловском томе при UNC-пути почему-то обламывается GetVolumeInformation() */
  _LOGCOPYR(SysLog("%d 0x%08X Dest='%s' Root='%s'",__LINE__,Attr,Dest,Root));
  int GetInfoSuccess = GetVolumeInformation(Root,NULL,0,NULL,NULL,&FileSystemFlagsDst,FSysNameDst,sizeof(FSysNameDst));
  if (GetInfoSuccess)
  {
     _LOGCOPYR(SysLog("GetVolumeInformation -> FS='%s' (Flags=0x%08X) %c%c",FSysNameDst,FileSystemFlagsDst,(FileSystemFlagsDst&FS_FILE_COMPRESSION?'C':'.'),(FileSystemFlagsDst&FILE_SUPPORTS_ENCRYPTION?'E':'.')));
     if(!(FileSystemFlagsDst&FS_FILE_COMPRESSION))
       Attr&=~FILE_ATTRIBUTE_COMPRESSED;
     if(!(FileSystemFlagsDst&FILE_SUPPORTS_ENCRYPTION))
       Attr&=~FILE_ATTRIBUTE_ENCRYPTED;
  }
  if (!SetFileAttributes(Dest,Attr))
  {
    _LOGCOPYR(SysLog("return 0 -> %d SetFileAttributes('%s',0x%08X) == 0",__LINE__,Dest,Attr));
    return FALSE;
  }
  if((Attr&FILE_ATTRIBUTE_COMPRESSED) && !(Attr&FILE_ATTRIBUTE_ENCRYPTED))
  {
    int Ret=ESetFileCompression(Dest,1,Attr&(~FILE_ATTRIBUTE_COMPRESSED),SkipMode);
    if(Ret==SETATTR_RET_ERROR)
    {
      _LOGCOPYR(SysLog("return 0 -> %d ESetFileCompression('%s',1,0) == 0",__LINE__,Dest));
      return FALSE;
    }
    else if(Ret==SETATTR_RET_SKIPALL)
      this->SkipMode=SETATTR_RET_SKIP;
  }
  // При копировании/переносе выставляем FILE_ATTRIBUTE_ENCRYPTED
  // для каталога, если он есть
  if (GetInfoSuccess && (FileSystemFlagsDst&FILE_SUPPORTS_ENCRYPTION) &&
     (Attr&(FILE_ATTRIBUTE_ENCRYPTED|FILE_ATTRIBUTE_DIRECTORY)) == (FILE_ATTRIBUTE_ENCRYPTED|FILE_ATTRIBUTE_DIRECTORY))
  {
    int Ret=ESetFileEncryption(Dest,1,0,SkipMode);
    if (Ret==SETATTR_RET_ERROR)
    {
      _LOGCOPYR(SysLog("return 0 -> %d ESetFileEncryption('%s',1,0) == 0",__LINE__,Dest));
      return FALSE;
    }
    else if(Ret==SETATTR_RET_SKIPALL)
      SkipMode=SETATTR_RET_SKIP;
  }
  _LOGCOPYR(SysLog("return 1 -> %d",__LINE__));
  return TRUE;
  /* VVM $ */
}

BOOL ShellCopy::CheckNulOrCon(const char *Src)
{
  // Выставляем признак копирования в NUL
  /* $ 21.12.2002 IS
     Признак копирования в nul выставим в случаях, когда цель назначения:
     - начинается с "nul\", "\\.\nul\" или "con\"
     - равна "nul", "\\.\nul" или "con"
  */
  if(!LocalStricmp (Src,"nul")             ||
     !LocalStrnicmp(Src,"nul\\", 4)        ||
     !LocalStrnicmp(Src,"\\\\.\\nul", 7)   ||
     !LocalStrnicmp(Src,"\\\\.\\nul\\", 8) ||
     !LocalStricmp (Src,"con")             ||
     !LocalStrnicmp(Src,"con\\", 4)
    )
    return TRUE;
  return FALSE;
}

void ShellCopy::CheckUpdatePanel() // выставляет флаг FCOPY_UPDATEPPANEL
{
}

BOOL ShellCopy::CheckAndUpdateConsole(BOOL IsChangeConsole)
{
  BOOL curZoomedState=IsZoomed(hFarWnd);
  BOOL curIconicState=IsIconic(hFarWnd);
  if(ZoomedState!=curZoomedState && IconicState==curIconicState)
  {
    ZoomedState=curZoomedState;
    ChangeVideoMode(ZoomedState);
    Frame *frame=FrameManager->GetBottomFrame();
    int LockCount=-1;
    while(frame->Locked())
    {
      LockCount++;
      frame->Unlock();
    }
    FrameManager->ResizeAllFrame();
    FrameManager->PluginCommit();
    /*
    while(LockCount > 0)
    {
      frame->Lock();
      LockCount--;
    }
    */
    IsChangeConsole=TRUE;
  }
  return IsChangeConsole;
}
