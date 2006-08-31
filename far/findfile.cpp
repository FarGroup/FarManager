/*
findfile.cpp

����� (Alt-F7)

*/

/* Revision: 1.207 01.09.2006 $ */


#include "headers.hpp"
#pragma hdrstop

#include "findfile.hpp"
#include "ClevMutex.hpp"
#include "plugin.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "flink.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "vmenu.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "editor.hpp"
#include "fileview.hpp"
#include "fileedit.hpp"
#include "filelist.hpp"
#include "cmdline.hpp"
#include "chgprior.hpp"
#include "namelist.hpp"
#include "scantree.hpp"
#include "savescr.hpp"
#include "manager.hpp"
#include "scrbuf.hpp"
#include "CFileMask.hpp"
#include "filefilter.hpp"
#include "farexcpt.hpp"

#define DLG_HEIGHT 23
#define DLG_WIDTH 74
#define PARTIAL_DLG_STR_LEN 30
#define CHAR_TABLE_SIZE 5

#define LIST_DELTA  64
static DWORD LIST_INDEX_NONE = (DWORD)-1;

// ������ ��������� ������. ������ �� ������ �������� � ����.
static LPFINDLIST  *FindList;
static DWORD       FindListCapacity;
static DWORD       FindListCount;
// ������ �������. ���� ���� ������ � ������, �� FindList->ArcIndex ��������� ����.
static LPARCLIST   *ArcList;
static DWORD       ArcListCapacity;
static DWORD       ArcListCount;

static CriticalSection ffCS; // ����� �� ��������� ������ ��������� ������, ���� �� �� ���� ������
static CriticalSection statusCS; // ����� �� ������ � FindMessage/FindFileCount/FindDirCount ���� �� ������ ������


static DWORD FindFileArcIndex;
// ������������ ��� �������� ������ �� ��������� ������.
// ������ �������� �������� � ������ � ���� ��� ��������.
static DWORD FindExitIndex;
enum {FIND_EXIT_NONE, FIND_EXIT_SEARCHAGAIN, FIND_EXIT_GOTO, FIND_EXIT_PANEL};
static int FindExitCode;
//static char FindFileArcName[NM];

static string strFindMask, strFindStr;
/* $ 30.07.2000 KM
   ��������� ���������� WholeWords ��� ������ �� ������� ����������
*/
static int SearchMode,CmpCase,WholeWords,SearchInArchives,SearchInSymLink,SearchHex;
/* KM $ */
static int FindFoldersChanged;
static int SearchFromChanged;
static int DlgWidth,DlgHeight;
static volatile int StopSearch,PauseSearch,SearchDone,LastFoundNumber,FindFileCount,FindDirCount,WriteDataUsed,PrepareFilesListUsed;
static string strFindMessage;
static string strLastDirName;
static int FindMessageReady,FindCountReady,FindPositionChanged;
static char PluginSearchPath[2*NM];
static HANDLE hDlg;
static int RecurseLevel;
static int BreakMainThread;
static int PluginMode;

static HANDLE hPluginMutex;

static int UseAllTables=FALSE,UseDecodeTable=FALSE,UseANSI=FALSE,UseUnicode=FALSE,TableNum=0,UseFilter=0;
static int SearchInFirstIndex=0,EnableSearchInFirst=FALSE;
static __int64 SearchInFirst=_i64(0);
static struct CharTableSet TableSet;

/* $ 01.07.2001 IS
   ������ "����� ������". ������ ��� ����� ������������ ��� �������� �����
   ����� �� ���������� � �������.
*/
static CFileMaskW FileMaskForFindFile;
/* IS $ */

/* $ 05.10.2003 KM
   ��������� �� ������ ������� ��������
*/
static FileFilter *Filter;
/* KM $*/

int _cdecl SortItems(const void *p1,const void *p2)
{
  PluginPanelItemW *Item1=(PluginPanelItemW *)p1;
  PluginPanelItemW *Item2=(PluginPanelItemW *)p2;
  string strN1, strN2;
  if (*Item1->FindData.lpwszFileName)
    strN1 = Item1->FindData.lpwszFileName;
  if (*Item2->FindData.lpwszFileName)
    strN2 = Item2->FindData.lpwszFileName;

  CutToSlashW(strN1);
  CutToSlashW(strN2);
  return LocalStricmpW(strN2,strN1);
}

long WINAPI FindFiles::MainDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  Dialog* Dlg=(Dialog*)hDlg;
  const wchar_t *FindText=UMSG(MFindFileText),*FindHex=UMSG(MFindFileHex),*FindCode=UMSG(MFindFileCodePage);
  string strDataStr;

  switch(Msg)
  {
    case DN_INITDIALOG:
    {
      /* $ 21.09.2003 KM
         ������������ ��������� ������ ����� �������� ������
         � ����������� �� Dlg->Item[13].Selected
      */
      if (Dlg->Item[13]->Selected) // [ ] Search for hexadecimal code
      {
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,5,FALSE);
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,6,TRUE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,8,FALSE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,11,FALSE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,12,FALSE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,15,FALSE);
      }
      else
      {
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,5,TRUE);
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,6,FALSE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,8,TRUE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,11,TRUE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,12,TRUE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,15,TRUE);
      }

      Dialog::SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,5,1);
      Dialog::SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,6,1);
      /* KM $ */

      unsigned int W=Dlg->Item[7]->X1-Dlg->Item[4]->X1-5;
      if (wcslen((Dlg->Item[13]->Selected?FindHex:FindText))>W)
      {
        strDataStr = Dlg->Item[13]->Selected?FindHex:FindText;
        TruncStrW (strDataStr, W);
      }
      else
        strDataStr = (Dlg->Item[13]->Selected?FindHex:FindText);

      Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,4,(long)(const wchar_t*)strDataStr);

      W=Dlg->Item[0]->X2-Dlg->Item[7]->X1-3;
      if (wcslen(FindCode)>W)
      {
        strDataStr = FindCode;
        TruncStrW (strDataStr, W);
      }
      else
        strDataStr = FindCode;
      Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,7,(long)(const wchar_t*)strDataStr);

      /* ��������� ����������� ����� ���������� */
      UseAllTables=Opt.CharTable.AllTables;
      UseANSI=Opt.CharTable.AnsiTable;
      UseUnicode=Opt.CharTable.UnicodeTable;
      UseDecodeTable=((UseAllTables==0) && (UseUnicode==0) && (UseANSI==0) && (Opt.CharTable.TableNum>0));
      if (UseDecodeTable)
        TableNum=Opt.CharTable.TableNum-1;
      else
        TableNum=0;
      /* -------------------------------------- */

      if (UseAllTables)
        xstrncpy(TableSet.TableName,MSG(MFindFileAllTables),sizeof(TableSet.TableName)-1);
      else if (UseUnicode)
        xstrncpy(TableSet.TableName,"Unicode",sizeof(TableSet.TableName)-1);
      /* $ 20.09.2003 KM
         ������� ��������� ANSI �������
      */
      else if (UseANSI)
      {
        GetTable(&TableSet,TRUE,TableNum,UseUnicode);
        xstrncpy(TableSet.TableName,MSG(MGetTableWindowsText),sizeof(TableSet.TableName)-1);
      }
      /* KM $ */
      else if (!UseDecodeTable)
        xstrncpy(TableSet.TableName,MSG(MGetTableNormalText),sizeof(TableSet.TableName)-1);
      else
        PrepareTable(&TableSet,TableNum,TRUE);
      RemoveChar(TableSet.TableName,'&',TRUE);

      string strTableName;
      strTableName.SetData (TableSet.TableName, CP_OEMCP); //BUGBUG

      Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,8,(long)(const wchar_t*)strTableName);

      FindFoldersChanged = FALSE;
      SearchFromChanged=FALSE;

      if (Dlg->Item[23]->Selected==1)
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,32,TRUE);
      else
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,32,FALSE);

      return TRUE;
    }
    case DN_LISTCHANGE:
    {
      if (Param1==8)
      {
        /* $ 20.09.2003 KM
           ������� ��������� ANSI �������
        */
        UseAllTables=(Param2==0);
        UseANSI=(Param2==3);
        UseUnicode=(Param2==4);
        UseDecodeTable=(Param2>=(CHAR_TABLE_SIZE+1));
        TableNum=Param2-(CHAR_TABLE_SIZE+1);
        if (UseAllTables)
          xstrncpy(TableSet.TableName,MSG(MFindFileAllTables),sizeof(TableSet.TableName)-1);
        else if (UseUnicode)
          xstrncpy(TableSet.TableName,"Unicode",sizeof(TableSet.TableName)-1);
        else if (UseANSI)
        {
          GetTable(&TableSet,TRUE,TableNum,UseUnicode);
          xstrncpy(TableSet.TableName,MSG(MGetTableWindowsText),sizeof(TableSet.TableName)-1);
        }
        else if (!UseDecodeTable)
          xstrncpy(TableSet.TableName,MSG(MGetTableNormalText),sizeof(TableSet.TableName)-1);
        else
          PrepareTable(&TableSet,TableNum,TRUE);
        /* KM $ */
      }
      return TRUE;
    }
    /* 22.11.2001 VVM
      ! ������������������� FindFolders ��� ����� ������.
        �� ������ ���� �� ������ ���� ��������� ������� */
    case DN_BTNCLICK:
    {
      Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
      string strSearchFromRoot;

      /* $ 23.11.2001 KM
         - ����! ������ ���� � ���� ������� :-(
           ��������� �������� �������� � �����������.
      */
      /* $ 22.11.2001 KM
         - ������ ������, ��� DN_BTNCLICK �������� �� ����� "�������������"
           ����������, ������ � �������� ������� ��-�������, ������� �������
           ������� ENTER �� ������� Find ��� Cancel �� � ���� �� ���������.
      */
      if (Param1==31 || Param1==35) // [ Find ] ��� [ Cancel ]
        return FALSE;
      else if (Param1==32) // [ Drive ]
      {
        IsRedrawFramesInProcess++;
        ActivePanel->ChangeDisk();
        // �� ��� �, ��� ����� ����� ������ ��������� ������
        // ����� ����� ��������.
        //FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
        FrameManager->ResizeAllFrame();
        IsRedrawFramesInProcess--;

        PrepareDriveNameStr(strSearchFromRoot, PARTIAL_DLG_STR_LEN);

        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,23,(long)(const wchar_t*)strSearchFromRoot);
        PluginMode=CtrlObject->Cp()->ActivePanel->GetMode()==PLUGIN_PANEL;
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,15,PluginMode?FALSE:TRUE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,20,PluginMode?FALSE:TRUE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,21,PluginMode?FALSE:TRUE);
      }
      else if (Param1==33) // Filter
        Filter->Configure();
      else if (Param1==34) // Advanced
        AdvancedDialog();
      else if (Param1>=20 && Param1<=26)
      {
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,32,Param1==23?TRUE:FALSE);
        SearchFromChanged=TRUE;
      }
      else if (Param1==15) // [ ] Search for folders
        FindFoldersChanged = TRUE;
      else if (Param1==13) // [ ] Search for hexadecimal code
      {
        Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);

        /* $ 21.09.2003 KM
           ������������ ��������� ������ ����� �������� ������
           � ����������� �� �������������� �������� hex mode
        */
        //int LenDataStr=sizeof(DataStr); //BUGBUG
        if (Param2)
        {
         // Transform((unsigned char *)DataStr,LenDataStr,Dlg->Item[5].Data,'X'); BUGBUG
          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,6,(long)(const wchar_t*)strDataStr);

          Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,5,FALSE);
          Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,6,TRUE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,8,FALSE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,11,FALSE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,12,FALSE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,15,FALSE);
        }
        else
        {
          //Transform((unsigned char *)DataStr,LenDataStr,Dlg->Item[6].Data,'S'); BUGBUG
          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,5,(long)(const wchar_t*)strDataStr);

          Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,5,TRUE);
          Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,6,FALSE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,8,TRUE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,11,TRUE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,12,TRUE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,15,TRUE);
        }
        /* KM $ */

        unsigned int W=Dlg->Item[7]->X1-Dlg->Item[4]->X1-5;
        if (wcslen((Param2?FindHex:FindText))>W)
        {
          strDataStr = (Param2?FindHex:FindText);
          TruncStrW (strDataStr, W);
        }
        else
          strDataStr = (Param2?FindHex:FindText);
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,4,(long)(const wchar_t*)strDataStr);

        if (strDataStr.GetLength()>0)
        {
          int UnchangeFlag=Dialog::SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,5,-1);
          Dialog::SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,6,UnchangeFlag);
        }

        Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
      }
      else if (Param1==29) // [ ] Advanced options
        EnableSearchInFirst=Param2;
      return TRUE;
      /* KM $ */
      /* KM $ */
    }
    case DN_EDITCHANGE:
    {
      FarDialogItem &Item=*reinterpret_cast<FarDialogItem*>(Param2);

      if (Param1==5)
      {
        if(!FindFoldersChanged)
        // ������ "���������� �����"
        {

          BOOL Checked = (Item.PtrData && *Item.PtrData)?FALSE:Opt.FindOpt.FindFolders;
          if (Checked)
            Dialog::SendDlgMessage(hDlg, DM_SETCHECK, 15, BSTATE_CHECKED);
          else
            Dialog::SendDlgMessage(hDlg, DM_SETCHECK, 15, BSTATE_UNCHECKED);
        }
      }
      return TRUE;
    }
    /* VVM $ */
    /* $ 15.10.2003 KM
        ���������� "�������" �������
    */
    case DN_HOTKEY:
    {
      if (Param1==4)
      {
        if (Dlg->Item[13]->Selected)
          Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,6,0);
        else
          Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,5,0);
        return FALSE;
      }
    }
    /* KM $ */
  }
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}


static void ShowTruncateMessage(int IDMField,int MaxSize)
{
  string strBuf1;
  string strBuf2;

  strBuf1 = L"\'";
  strBuf1 += UMSG(IDMField);
  strBuf1 += L"\'";
  RemoveHighlightsW(strBuf1);
  strBuf2.Format (UMSG(MEditInputSize2), MaxSize);
  MessageW(MSG_WARNING,1,UMSG(MWarning),UMSG(MEditInputSize1),strBuf1,strBuf2,UMSG(MOk));
}


FindFiles::FindFiles()
{
  _ALGO(CleverSysLog clv("FindFiles::FindFiles()"));
  static string strLastFindMask=L"*.*", strLastFindStr;
  // ����������� ��������� � ����������� ����������
  static string strSearchFromRoot;
  /* $ 30.07.2000 KM
     ��������� ���������� LastWholeWords ��� ������ �� ������� ����������
  */
  static int LastCmpCase=0,LastWholeWords=0,LastSearchInArchives=0,LastSearchInSymLink=-1,LastSearchHex=0;
  /* KM $ */
  int I;

  // �������� ������ �������
  Filter=new FileFilter;

  CmpCase=LastCmpCase;
  WholeWords=LastWholeWords;
  SearchInArchives=LastSearchInArchives;
  SearchHex=LastSearchHex;
  if(LastSearchInSymLink == -1)
    LastSearchInSymLink=Opt.ScanJunction;
  if (!RegVer)
    LastSearchInSymLink=0;
  SearchInSymLink=LastSearchInSymLink;
  SearchMode=Opt.FindOpt.FileSearchMode;

  // ������ ���� �������, � ������� ����������� ����������� ������ � �����
  SearchInFirstIndex=Opt.FindOpt.SearchInFirst;

  // ��-��������� �������������� ��������� �� ������������
  EnableSearchInFirst=FALSE;

  strFindMask = strLastFindMask;
  strFindStr = strLastFindStr;
  BreakMainThread=0;

  strSearchFromRoot = UMSG(MSearchFromRootFolder);

  FarList TableList;
  FarListItem *TableItem=(FarListItem *)xf_malloc(sizeof(FarListItem)*CHAR_TABLE_SIZE);
  TableList.Items=TableItem;
  TableList.ItemsNumber=CHAR_TABLE_SIZE;

  memset(TableItem,0,sizeof(FarListItem)*CHAR_TABLE_SIZE);
  xwcsncpy(TableItem[0].Text,UMSG(MFindFileAllTables),(sizeof(TableItem[0].Text)-1)/sizeof (wchar_t));
  TableItem[1].Flags=LIF_SEPARATOR;
  xwcsncpy(TableItem[2].Text,UMSG(MGetTableNormalText),(sizeof(TableItem[2].Text)-1)/sizeof (wchar_t));
  xwcsncpy(TableItem[3].Text,UMSG(MGetTableWindowsText),(sizeof(TableItem[3].Text)-1)/sizeof (wchar_t));
  xwcsncpy(TableItem[4].Text,L"Unicode",(sizeof(TableItem[4].Text)-1)/sizeof (wchar_t));

  for (I=0;;I++)
  {
    CharTableSet cts;
    int RetVal=FarCharTable(I,(char *)&cts,sizeof(cts));
    if (RetVal==-1)
      break;

    if (I==0)
    {
      TableItem=(FarListItem *)xf_realloc(TableItem,sizeof(FarListItem)*(CHAR_TABLE_SIZE+1));
      if (TableItem==NULL)
        return;
      memset(&TableItem[CHAR_TABLE_SIZE],0,sizeof(FarListItem));
      TableItem[CHAR_TABLE_SIZE].Flags=LIF_SEPARATOR;
      TableList.Items=TableItem;
      TableList.ItemsNumber++;
    }

    TableItem=(FarListItem *)xf_realloc(TableItem,sizeof(FarListItem)*(I+CHAR_TABLE_SIZE+2));
    if (TableItem==NULL)
      return;
    memset(&TableItem[I+CHAR_TABLE_SIZE+1],0,sizeof(FarListItem));

    MultiByteToWideChar(CP_OEMCP, 0, cts.TableName, -1, TableItem[I+CHAR_TABLE_SIZE+1].Text, (sizeof(TableItem[I+CHAR_TABLE_SIZE+1].Text)-1)/sizeof (wchar_t)); //BUGBUG

    ///RemoveCharW(TableItem[I+CHAR_TABLE_SIZE+1].Text,L'&',TRUE); //BUGBUG!!!
    TableList.Items=TableItem;
    TableList.ItemsNumber++;
  }

  FindList = NULL;
  ArcList = NULL;
  hPluginMutex=CreateMutexW(NULL,FALSE,NULL);

  do
  {
    ClearAllLists();

    Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
    PluginMode=ActivePanel->GetMode()==PLUGIN_PANEL && ActivePanel->IsVisible();

    PrepareDriveNameStr(strSearchFromRoot,PARTIAL_DLG_STR_LEN);

    const wchar_t *MasksHistoryName=L"Masks",*TextHistoryName=L"SearchText";
    const char *HexMask="HH HH HH HH HH HH HH HH HH HH HH"; // HH HH HH HH HH HH HH HH HH HH HH HH";

/*
    000000000011111111112222222222333333333344444444445555555555666666666677777777
    012345678901234567890123456789012345678901234567890123456789012345678901234567
00  """"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
01  "  +----------------------------- Find file ------------------------------+  "
02  "  � A file mask or several file masks:                                   �  "
03  "  � *.*#################################################################.�  "
04  "  �----------------------------------+-----------------------------------�  "
05  "  � Containing text:                 � Using character table:            �  "
06  "  � ################################.� Windows text#####################.�  "
07  "  �----------------------------------+-----------------------------------�  "
08  "  � [ ] Case sensitive               � [ ] Search in archives            �  "
09  "  � [ ] Whole words                  � [x] Search for folders            �  "
10  "  � [ ] Search for hex               � [x] Search in symbolic links      �  "
11  "  �----------------------------------+-----------------------------------�  "
12  "  � Select place to search                                               �  "
13  "  � ( ) In all non-removable drives    (.) From the current folder       �  "
14  "  � ( ) In all local drives            ( ) The current folder only       �  "
15  "  � ( ) In PATH folders                ( ) Selected folders              �  "
    "  � ( ) From the root of C:                                              �  "
16  "  �----------------------------------------------------------------------�  "
17  "  � [ ] Use filter                     [ ] Advanced options              �  "
18  "  �----------------------------------------------------------------------�  "
19  "  �      [ Find ]  [ Drive ]  [ Filter ]  [ Advanced ]  [ Cancel ]       �  "
20  "  +----------------------------------------------------------------------+  "
21  "                                                                            "
22  """"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
*/

    /* $ 30.07.2000 KM
       �������� ����� checkbox "Whole words" � ������ ������
    */
    static struct DialogDataEx FindAskDlgData[]=
    {
      /* 00 */DI_DOUBLEBOX,3,1,DLG_WIDTH,DLG_HEIGHT-2,0,0,0,0,(const wchar_t *)MFindFileTitle,
      /* 01 */DI_TEXT,5,2,0,0,0,0,0,0,(const wchar_t *)MFindFileMasks,
      /* 02 */DI_EDIT,5,3,72,3,1,(DWORD)MasksHistoryName,DIF_HISTORY|DIF_USELASTHISTORY,0,L"",
      /* 03 */DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR2,0,L"",
      /* 04 */DI_TEXT,5,5,0,0,0,0,0,0,L"",
      /* 05 */DI_EDIT,5,6,36,5,0,(DWORD)TextHistoryName,DIF_HISTORY,0,L"",
      /* 06 */DI_FIXEDIT,5,6,36,5,0,(DWORD)HexMask,DIF_MASKEDIT,0,L"",
      /* 07 */DI_TEXT,40,5,0,0,0,0,0,0,L"",
      /* 08 */DI_COMBOBOX,40,6,72,18,0,0,DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND,0,L"",
      /* 09 */DI_TEXT,3,7,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
      /* 10 */DI_VTEXT,38,4,0,0,0,0,DIF_BOXCOLOR,0,L"\x2564\x2502\x2502\x253C",
      /* 11 */DI_CHECKBOX,5,8,0,0,0,0,0,0,(const wchar_t *)MFindFileCase,
      /* 12 */DI_CHECKBOX,5,9,0,0,0,0,0,0,(const wchar_t *)MFindFileWholeWords,
      /* 13 */DI_CHECKBOX,5,10,0,0,0,0,0,0,(const wchar_t *)MSearchForHex,
      /* 14 */DI_CHECKBOX,40,8,0,0,0,0,0,0,(const wchar_t *)MFindArchives,
      /* 15 */DI_CHECKBOX,40,9,0,0,0,0,0,0,(const wchar_t *)MFindFolders,
      /* 16 */DI_CHECKBOX,40,10,0,0,0,0,0,0,(const wchar_t *)MFindSymLinks,
      /* 17 */DI_TEXT,3,11,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR2,0,L"",
        /* 18 */DI_VTEXT,38,7,0,0,0,0,DIF_BOXCOLOR,0,L"\x253C\x2502\x2502\x2502\x2567",
      /* 19 */DI_TEXT,5,12,0,0,0,0,0,0,(const wchar_t *)MSearchWhere,
      /* 20 */DI_RADIOBUTTON,5,13,0,0,0,0,DIF_GROUP,0,(const wchar_t *)MSearchAllDisks,
      /* 21 */DI_RADIOBUTTON,5,14,0,0,0,1,0,0,(const wchar_t *)MSearchAllButNetwork,
      /* 22 */DI_RADIOBUTTON,5,15,0,0,0,1,0,0,(const wchar_t *)MSearchInPATH,
      /* 23 */DI_RADIOBUTTON,5,16,0,0,0,1,0,0,L"",
      /* 24 */DI_RADIOBUTTON,40,13,0,0,0,0,0,0,(const wchar_t *)MSearchFromCurrent,
      /* 25 */DI_RADIOBUTTON,40,14,0,0,0,0,0,0,(const wchar_t *)MSearchInCurrent,
      /* 26 */DI_RADIOBUTTON,40,15,0,0,0,0,0,0,(const wchar_t *)MSearchInSelected,
      /* 27 */DI_TEXT,3,17,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
      /* 28 */DI_CHECKBOX,5,18,0,0,0,0,0,0,(const wchar_t *)MFindUseFilter,
      /* 29 */DI_CHECKBOX,40,18,0,0,0,0,0,0,(const wchar_t *)MFindAdvancedOptions,
      /* 30 */DI_TEXT,3,19,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
      /* 31 */DI_BUTTON,0,20,0,0,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MFindFileFind,
      /* 32 */DI_BUTTON,0,20,0,0,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MFindFileDrive,
      /* 33 */DI_BUTTON,0,20,0,0,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MFindFileSetFilter,
      /* 34 */DI_BUTTON,0,20,0,0,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MFindFileAdvanced,
      /* 35 */DI_BUTTON,0,20,0,0,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel,
    };
    /* KM $ */

    FindAskDlgData[23].Data = strSearchFromRoot; //BUGBUG

    MakeDialogItemsEx(FindAskDlgData,FindAskDlg);


    if ( strFindStr.IsEmpty() )
      FindAskDlg[15].Selected=Opt.FindOpt.FindFolders;
    for(I=20; I <= 26; ++I)
      FindAskDlg[I].Selected=0;
    FindAskDlg[20+SearchMode].Selected=1;

    {
      if (PluginMode)
      {
        struct OpenPluginInfoW Info;
        HANDLE hPlugin=ActivePanel->GetPluginHandle();
        CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
        /* $ 14.05.2001 DJ
           ���������, � �� ������
        */
        if ((Info.Flags & OPIF_REALNAMES)==0)
          FindAskDlg[14].Flags |= DIF_DISABLE;
        /* DJ $ */
        if (FindAskDlg[20].Selected || FindAskDlg[21].Selected)
        {
          FindAskDlg[20].Selected=FindAskDlg[21].Selected=0;
          FindAskDlg[23].Selected=1;
        }
        FindAskDlg[20].Flags=FindAskDlg[21].Flags|=DIF_DISABLE;
      }
    }

    if (!RegVer || PluginMode)
    {
      FindAskDlg[16].Selected=0;
      FindAskDlg[16].Flags|=DIF_DISABLE;
    }
    else
      FindAskDlg[16].Selected=SearchInSymLink;

    /* $ 14.05.2001 DJ
       �� �������� �������, ���� ������ ������ � �������
    */
    if (!(FindAskDlg[14].Flags & DIF_DISABLE))
      FindAskDlg[14].Selected=SearchInArchives;
    /* DJ $ */

    FindAskDlg[2].strData = strFindMask;

    /* $ 26.10.2003 KM */
    if (SearchHex)
      FindAskDlg[6].strData = strFindStr;
    else
      FindAskDlg[5].strData = strFindStr;
    /* KM $ */

    FindAskDlg[11].Selected=CmpCase;
    FindAskDlg[12].Selected=WholeWords;
    FindAskDlg[13].Selected=SearchHex;

    // ������������ ������. KM
    FindAskDlg[28].Selected=UseFilter;

    // �������� �������������� ����� ������. KM
    FindAskDlg[29].Selected=EnableSearchInFirst;

    while (1)
    {
      int ExitCode;
      {
        FindAskDlg[8].ListItems=&TableList;

        Dialog Dlg(FindAskDlg,sizeof(FindAskDlg)/sizeof(FindAskDlg[0]),MainDlgProc);

        Dlg.SetHelp(L"FindFile");
        Dlg.SetPosition(-1,-1,DLG_WIDTH+4,DLG_HEIGHT);
        Dlg.Process();
        ExitCode=Dlg.GetExitCode();
      }

      if (ExitCode!=31)
      {
        xf_free(TableItem);
        CloseHandle(hPluginMutex);
        return;
      }

      /* ����������� ������������� ���������� */
      Opt.CharTable.AllTables=UseAllTables;
      Opt.CharTable.AnsiTable=UseANSI;
      Opt.CharTable.UnicodeTable=UseUnicode;
      if (UseDecodeTable)
        Opt.CharTable.TableNum=TableNum+1;
      else
        Opt.CharTable.TableNum=0;
      /****************************************/

      /* $ 01.07.2001 IS
         �������� ����� �� ������������
      */
      if( FindAskDlg[2].strData.IsEmpty() )             // ���� ������ � ������� �����,
         FindAskDlg[2].strData = L"*"; // �� �������, ��� ����� ���� "*"

      if(FileMaskForFindFile.Set(FindAskDlg[2].strData, FMF_ADDASTERISK))
           break;
      /* IS $ */
    }

    CmpCase=FindAskDlg[11].Selected;
    /* $ 30.07.2000 KM
       ��������� ����������
    */
    WholeWords=FindAskDlg[12].Selected;
    /* KM $ */
    SearchHex=FindAskDlg[13].Selected;
    SearchInArchives=FindAskDlg[14].Selected;
    if (FindFoldersChanged)
      Opt.FindOpt.FindFolders=FindAskDlg[15].Selected;

    if (RegVer && !PluginMode)
      SearchInSymLink=FindAskDlg[16].Selected;

    // ��������� ������� ������������� �������. KM
    UseFilter=FindAskDlg[28].Selected;

    strFindMask = !FindAskDlg[2].strData.IsEmpty() ? FindAskDlg[2].strData:L"*";

    if (SearchHex)
    {
      strFindStr = FindAskDlg[6].strData;
      RemoveTrailingSpacesW(strFindStr);
    }
    else
      strFindStr = FindAskDlg[5].strData;

    if ( !strFindStr.IsEmpty())
    {
      UnicodeToAnsi(strFindStr, GlobalSearchString, sizeof (GlobalSearchString)-1);
      GlobalSearchCase=CmpCase;
      /* $ 30.07.2000 KM
         ��������� ����������
      */
      GlobalSearchWholeWords=WholeWords;
      /* KM $ */
      GlobalSearchHex=SearchHex;
    }
    if (FindAskDlg[20].Selected)
      SearchMode=SEARCH_ALL;
    if (FindAskDlg[21].Selected)
      SearchMode=SEARCH_ALL_BUTNETWORK;
    if (FindAskDlg[22].Selected)
      SearchMode=SEARCH_INPATH;
    if (FindAskDlg[23].Selected)
      SearchMode=SEARCH_ROOT;
    if (FindAskDlg[24].Selected)
      SearchMode=SEARCH_FROM_CURRENT;
    if (FindAskDlg[25].Selected)
      SearchMode=SEARCH_CURRENT_ONLY;
    if (FindAskDlg[26].Selected)
        SearchMode=SEARCH_SELECTED;
    if (SearchFromChanged)
    {
      Opt.FindOpt.FileSearchMode=SearchMode;
    }
    LastCmpCase=CmpCase;
    /* $ 30.07.2000 KM
       ��������� ����������
    */
    LastWholeWords=WholeWords;
    /* KM $ */
    LastSearchHex=SearchHex;
    LastSearchInArchives=SearchInArchives;
    LastSearchInSymLink=SearchInSymLink;
    strLastFindMask = strFindMask;
    strLastFindStr = strFindStr;
    if ( !strFindStr.IsEmpty() )
      Editor::SetReplaceMode(FALSE);
  } while (FindFilesProcess());
  CloseHandle(hPluginMutex);
  xf_free(TableItem);
}


FindFiles::~FindFiles()
{
  /* $ 02.07.2001 IS
     ��������� ������.
  */
  FileMaskForFindFile.Free();
  /* IS $ */
  ClearAllLists();
  ScrBuf.ResetShadow();

  // ��������� ������ �������
  if(Filter)
    delete Filter;
}

long WINAPI FindFiles::AdvancedDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  switch(Msg)
  {
    case DN_INITDIALOG:
    {
      Dialog::SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,2,1);
    }
    case DN_LISTCHANGE:
    {
      /* $ 11.12.2005 KM
        �������� ������ ���� �������
      */
      if (Param1==3)
      {
        SearchInFirstIndex=Param2;
      }
      return TRUE;
    }
  }
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

void FindFiles::AdvancedDialog()
{
  /* $ 10.04.2005 KM
     ������ ������ ���������, ��������� ������� ���������� �����
     � ������ ������ (����������, ���������� ��� ����������) ������
  */
  FarList SizeList;
  FarListItem *SizeItem=(FarListItem *)xf_malloc(sizeof(FarListItem)*4);
  SizeList.Items=SizeItem;
  SizeList.ItemsNumber=4;

  memset(SizeItem,0,sizeof(FarListItem)*4);
  xwcsncpy(SizeItem[0].Text,UMSG(MFindFileSearchInBytes),(sizeof(SizeItem[0].Text)-1)/sizeof (wchar_t));
  xwcsncpy(SizeItem[1].Text,UMSG(MFindFileSearchInKBytes),(sizeof(SizeItem[2].Text)-1)/sizeof (wchar_t));
  xwcsncpy(SizeItem[2].Text,UMSG(MFindFileSearchInMBytes),(sizeof(SizeItem[3].Text)-1)/sizeof (wchar_t));
  xwcsncpy(SizeItem[3].Text,UMSG(MFindFileSearchInGBytes),(sizeof(SizeItem[4].Text)-1)/sizeof (wchar_t));
  /* KM $ */

  const wchar_t *DigitMask=L"99999999999999999999";

  static struct DialogDataEx AdvancedDlgData[]=
  {
    /* 00 */DI_DOUBLEBOX,3,1,38,6,0,0,0,0,(const wchar_t *)MFindFileAdvancedTitle,
    /* 01 */DI_TEXT,5,2,0,0,0,0,0,0,(const wchar_t *)MFindFileSearchFirst,
    /* 02 */DI_FIXEDIT,5,3,24,3,0,(DWORD)DigitMask,DIF_MASKEDIT,0,L"",
    /* 03 */DI_COMBOBOX,26,3,36,13,0,0,DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND,0,L"",
    /* 04 */DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
    /* 05 */DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MOk,
    /* 06 */DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel,
  };

  MakeDialogItemsEx(AdvancedDlgData,AdvancedDlg);

  // ��������� ������, ������� ����� ����������� ��� ����������� ������
  // ������ � ������ ������ (����������, ���������� ��� ����������) � ������.
  AdvancedDlg[2].strData = Opt.FindOpt.strSearchInFirstSize;

  // ��������� ������� ���������, � ������� ����� ��������� ������ ����������� ������
  // ������ � ������ ������ (����������, ���������� ��� ����������) � ������.
  AdvancedDlg[3].strData = SizeItem[Opt.FindOpt.SearchInFirst].Text;

  AdvancedDlg[3].ListItems=&SizeList;

  Dialog Dlg(AdvancedDlg,sizeof(AdvancedDlg)/sizeof(AdvancedDlg[0]),AdvancedDlgProc);

  Dlg.SetHelp(L"FindFileAdvanced");
  Dlg.SetPosition(-1,-1,38+4,6+2);
  Dlg.Process();
  int ExitCode=Dlg.GetExitCode();

  if (ExitCode==5) // OK
  {
    /* $ 11.04.2005 KM
      �������� ���������, ����������� � ������ � ������
      ������ (��,�� ��� ��) ����� ������.
    */
    Opt.FindOpt.SearchInFirst=SearchInFirstIndex;

    Opt.FindOpt.strSearchInFirstSize = AdvancedDlg[2].strData;

    // ������� ������������ ������ ������ �� ����� ��� ������
    SearchInFirst=GetSearchInFirstW(AdvancedDlg[2].strData);
    /* KM $ */
  }

  xf_free(SizeItem);
}

int FindFiles::GetPluginFile(DWORD ArcIndex, struct PluginPanelItemW *PanelItem,
                             const wchar_t *DestPath, string &strResultName)
{
  _ALGO(CleverSysLog clv("FindFiles::GetPluginFile()"));
  HANDLE hPlugin = ArcList[ArcIndex]->hPlugin;
  string strSaveDir;
  struct OpenPluginInfoW Info;
  CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);

  strSaveDir = Info.CurDir;
  AddEndSlashW(strSaveDir);

  CtrlObject->Plugins.SetDirectory(hPlugin,L"\\",OPM_SILENT|OPM_FIND);

  string strFileName;

  strFileName = ArcList[ArcIndex]->strRootPath;
  SetPluginDirectory(strFileName,hPlugin);

  strFileName = PanelItem->FindData.lpwszFileName;
  SetPluginDirectory(strFileName,hPlugin);

  int nItemsNumber;
  struct PluginPanelItemW *pItems;
  int nResult = 0;

  wchar_t *lpFileNameToFind = _wcsdup (PanelItem->FindData.lpwszFileName);
  wchar_t *lpFileNameToFindShort = _wcsdup (PanelItem->FindData.lpwszAlternateFileName);

  lpFileNameToFind = (wchar_t*)PointToNameW(RemovePseudoBackSlashW(lpFileNameToFind));
  lpFileNameToFindShort = (wchar_t*)PointToNameW(RemovePseudoBackSlashW(lpFileNameToFindShort));

  if ( CtrlObject->Plugins.GetFindData (
      hPlugin,
      &pItems,
      &nItemsNumber,
      OPM_FIND
      ) )
  {
    for (int i = 0; i < nItemsNumber; i++)
    {
      struct PluginPanelItemW *pItem = &pItems[i];

      wchar_t *lpwszFileName = _wcsdup(pItem->FindData.lpwszFileName);
      xf_free (pItem->FindData.lpwszFileName);

      pItem->FindData.lpwszFileName = _wcsdup (PointToNameW(RemovePseudoBackSlashW(lpwszFileName)));
      xf_free (lpwszFileName);

      lpwszFileName = _wcsdup(pItem->FindData.lpwszAlternateFileName);
      xf_free (pItem->FindData.lpwszAlternateFileName);

      pItem->FindData.lpwszAlternateFileName = _wcsdup (PointToNameW(RemovePseudoBackSlashW(lpwszFileName)));
      xf_free (lpwszFileName);

      if ( !wcscmp (lpFileNameToFind, pItem->FindData.lpwszFileName) &&
           !wcscmp (lpFileNameToFindShort, pItem->FindData.lpwszAlternateFileName) )
      {
        nResult = CtrlObject->Plugins.GetFile (
                            hPlugin,
                            pItem,
                            DestPath,
                            strResultName,
                            OPM_SILENT|OPM_FIND
                            );

        xf_free (pItem->FindData.lpwszFileName);
        xf_free (pItem->FindData.lpwszAlternateFileName);

        break;
      }
      xf_free (pItem->FindData.lpwszFileName);
      xf_free (pItem->FindData.lpwszAlternateFileName);

    }

    CtrlObject->Plugins.FreeFindData (hPlugin, pItems, nItemsNumber);
  }

  free (lpFileNameToFind);
  free (lpFileNameToFindShort);

  CtrlObject->Plugins.SetDirectory(hPlugin,L"\\",OPM_SILENT|OPM_FIND);
//  SetPluginDirectory(ArcList[ArcIndex].RootPath,hPlugin);
  SetPluginDirectory(strSaveDir,hPlugin);

  return nResult;
}

long WINAPI FindFiles::FindDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  int Result;
  Dialog* Dlg=(Dialog*)hDlg;
  VMenu *ListBox=Dlg->Item[1]->ListPtr;

  CriticalSectionLock Lock(Dlg->CS);

  switch(Msg)
  {
    case DN_DRAWDIALOGDONE:
    {
      Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
      // ���������� ����� �� ������ [Go To]
      if((FindDirCount || FindFileCount) && !FindPositionChanged)
      {
        FindPositionChanged=TRUE;
        Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,6/* [Go To] */,0);
      }
//      else
//        ScrBuf.Flush();
      return TRUE;
    }
/*
    case DN_HELP: // � ������ ������ �������� ����� ������, ���... ��������� � �����������
    {
      return !SearchDone?NULL:Param2;
    }
*/
    case DN_MOUSECLICK:
    {
      /* $ 07.04.2003 VVM
         ! ���� �� ����� ������ ������� ��������� ���� ����� - ��������.
         �� ���� �������� ��������� �� �������� �������. ��� �� � ����... */
      SMALL_RECT drect,rect;
      int Ret = FALSE;
      Dialog::SendDlgMessage(hDlg,DM_GETDLGRECT,0,(long)&drect);
      Dialog::SendDlgMessage(hDlg,DM_GETITEMPOSITION,1,(long)&rect);
      if (Param1==1 && ((MOUSE_EVENT_RECORD *)Param2)->dwMousePosition.X<drect.Left+rect.Right)
      {
        Ret = FindDlgProc(hDlg,DN_BTNCLICK,6,0); // emulates a [ Go to ] button pressing
      }
      /* VVM $ */
      return Ret;
    }

    case DN_KEY:
    {
      if (!StopSearch && ((Param2==KEY_ESC) || (Param2 == KEY_F10)) )
      {
        PauseSearch=TRUE;
        IsProcessAssignMacroKey++; // �������� ���� �������
                                   // �.�. � ���� ������� ������ ������ Alt-F9!
        int LocalRes=TRUE;
        if (Opt.Confirm.Esc)
          LocalRes=AbortMessage();
        IsProcessAssignMacroKey--;
        PauseSearch=FALSE;
        StopSearch=LocalRes;

        return TRUE;
      }

      if (!ListBox)
      {
        return TRUE;
      }

      if(Param2 == KEY_LEFT || Param2 == KEY_RIGHT)
        FindPositionChanged = TRUE;

      // �������� ����.������� ����� �����������.
      if(Param2 == KEY_CTRLALTSHIFTPRESS || Param2 == KEY_ALTF9)
      {
        IsProcessAssignMacroKey--;
        FrameManager->ProcessKey(Param2);
        IsProcessAssignMacroKey++;
        return TRUE;
      }

      if (Param1==9 && (Param2==KEY_RIGHT || Param2==KEY_TAB)) // [ Stop ] button
      {
        FindPositionChanged = TRUE;
        Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,5/* [ New search ] */,0);
        return TRUE;
      }
      else if (Param1==5 && (Param2==KEY_LEFT || Param2==KEY_SHIFTTAB)) // [ New search ] button
      {
        FindPositionChanged = TRUE;
        Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,9/* [ Stop ] */,0);
        return TRUE;
      }
      else if (Param2==KEY_UP || Param2==KEY_DOWN || Param2==KEY_PGUP ||
               Param2==KEY_PGDN || Param2==KEY_HOME || Param2==KEY_END ||
               Param2==KEY_MSWHEEL_UP || Param2==KEY_MSWHEEL_DOWN)
      {
        ListBox->ProcessKey(Param2);
        return TRUE;
      }
      else if (Param2==KEY_F3 || Param2==KEY_NUMPAD5 || Param2==KEY_SHIFTNUMPAD5 || Param2==KEY_F4 ||
               Param2==KEY_ENTER && Dialog::SendDlgMessage(hDlg,DM_GETFOCUS,0,0) == 7
              )
      {
        if (ListBox->GetItemCount()==0)
        {
          return TRUE;
        }

        if(Param2==KEY_ENTER && Dialog::SendDlgMessage(hDlg,DM_GETFOCUS,0,0) == 7)
          Param2=KEY_F3;

        ffCS.Enter ();

        DWORD ItemIndex = (DWORD)ListBox->GetUserData(NULL, 0);
        if (ItemIndex != LIST_INDEX_NONE)
        {
          int RemoveTemp=FALSE;
          int ClosePlugin=FALSE; // ������� ���� ���������, ���� �������.
          string strSearchFileName;
          string strTempDir;

          if (FindList[ItemIndex]->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
          {
            ffCS.Leave ();

            return TRUE;
          }
          string strFileName=FindList[ItemIndex]->FindData.strFileName;
          // FindFileArcIndex ������ ����� ������������
          // �� ����� ���� ��� ������.
          if ((FindList[ItemIndex]->ArcIndex != LIST_INDEX_NONE) &&
              (!(ArcList[FindList[ItemIndex]->ArcIndex]->Flags & OPIF_REALNAMES)))
          {
            string strFindArcName = ArcList[FindList[ItemIndex]->ArcIndex]->strArcName;
            if (ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin == INVALID_HANDLE_VALUE)
            {
              char *Buffer=new char[Opt.PluginMaxReadData];
              FILE *ProcessFile=_wfopen(strFindArcName,L"rb");
              if (ProcessFile==NULL)
              {
                delete[] Buffer;

                ffCS.Leave ();
                return TRUE;
              }
              int ReadSize=fread(Buffer,1,Opt.PluginMaxReadData,ProcessFile);
              fclose(ProcessFile);

              int SavePluginsOutput=DisablePluginsOutput;
              DisablePluginsOutput=TRUE;

              WaitForSingleObject(hPluginMutex,INFINITE);

              ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin = CtrlObject->Plugins.OpenFilePlugin(strFindArcName,(unsigned char *)Buffer,ReadSize);

              ReleaseMutex(hPluginMutex);

              DisablePluginsOutput=SavePluginsOutput;

              delete[] Buffer;

              if (ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin == (HANDLE)-2 ||
                  ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin == INVALID_HANDLE_VALUE)
              {
                ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin = INVALID_HANDLE_VALUE;

                ffCS.Leave ();
                return TRUE;
              }
              ClosePlugin = TRUE;
            }

            PluginPanelItemW FileItem;
            memset(&FileItem,0,sizeof(FileItem));

            apiFindDataExToData(&FindList[ItemIndex]->FindData, &FileItem.FindData);

            FarMkTempExW(strTempDir); // � �������� �� NULL???
            CreateDirectoryW(strTempDir, NULL);

//            if (!CtrlObject->Plugins.GetFile(ArcList[FindList[ItemIndex].ArcIndex].hPlugin,&FileItem,TempDir,SearchFileName,OPM_SILENT|OPM_FIND))
            WaitForSingleObject(hPluginMutex,INFINITE);
            if (!GetPluginFile(FindList[ItemIndex]->ArcIndex,&FileItem,strTempDir,strSearchFileName))
            {
              apiFreeFindData(&FileItem.FindData);

              FAR_RemoveDirectoryW(strTempDir);
              if (ClosePlugin)
              {
                CtrlObject->Plugins.ClosePlugin(ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin);
                ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin = INVALID_HANDLE_VALUE;
              }
              ReleaseMutex(hPluginMutex);

              ffCS.Leave ();
              return FALSE;
            }
            else
            {
              apiFreeFindData(&FileItem.FindData);

              if (ClosePlugin)
              {
                CtrlObject->Plugins.ClosePlugin(ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin);
                ArcList[FindList[ItemIndex]->ArcIndex]->hPlugin = INVALID_HANDLE_VALUE;
              }
              ReleaseMutex(hPluginMutex);
            }
            RemoveTemp=TRUE;
          }
          else
          {
            strSearchFileName = FindList[ItemIndex]->FindData.strFileName;
            if(GetFileAttributesW(strSearchFileName) == (DWORD)-1 && GetFileAttributesW(FindList[ItemIndex]->FindData.strAlternateFileName) != (DWORD)-1)
              strSearchFileName = FindList[ItemIndex]->FindData.strAlternateFileName;
          }

          DWORD FileAttr;
          if ((FileAttr=GetFileAttributesW(strSearchFileName))!=(DWORD)-1)
          {
            string strOldTitle;
            apiGetConsoleTitle (strOldTitle);

            if (Param2==KEY_F3 || Param2==KEY_NUMPAD5 || Param2==KEY_SHIFTNUMPAD5)
            {
              int ListSize=ListBox->GetItemCount();
              NamesList ViewList;
              // ������� ��� �����, ������� ����� �������� �����...
              if(Opt.FindOpt.CollectFiles)
              {
                DWORD Index;
                for (int I=0;I<ListSize;I++)
                {
                  Index = (DWORD)ListBox->GetUserData(NULL, 0, I);
                  LPFINDLIST PtrFindList=FindList[Index];
                  if ((Index != LIST_INDEX_NONE) &&
                      ((PtrFindList->ArcIndex == LIST_INDEX_NONE) ||
                       (ArcList[PtrFindList->ArcIndex]->Flags & OPIF_REALNAMES)))
                  {
                    // �� ��������� ����� � ������� � OPIF_REALNAMES
                    if ( !PtrFindList->FindData.strFileName.IsEmpty() && !(PtrFindList->FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
                        ViewList.AddName(PtrFindList->FindData.strFileName, PtrFindList->FindData.strAlternateFileName);
                  } /* if */
                } /* for */

                string strCurDir = FindList[ItemIndex]->FindData.strFileName;

                ViewList.SetCurName(strCurDir);
              }
              Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,FALSE,0);
              Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
              {
                FileViewer ShellViewer (strSearchFileName,FALSE,FALSE,FALSE,-1,NULL,(FindList[ItemIndex]->ArcIndex != LIST_INDEX_NONE)?NULL:(Opt.FindOpt.CollectFiles?&ViewList:NULL));
                ShellViewer.SetDynamicallyBorn(FALSE);
                ShellViewer.SetEnableF6(TRUE);
                // FindFileArcIndex ������ ����� ������������
                // �� ����� ���� ��� ������.
                if ((FindList[ItemIndex]->ArcIndex != LIST_INDEX_NONE) &&
                    (!(ArcList[FindList[ItemIndex]->ArcIndex]->Flags & OPIF_REALNAMES)))
                  ShellViewer.SetSaveToSaveAs(TRUE);
                //!Mutex.Unlock();

                ffCS.Leave (); // ����� ����� �����������, ���� �� ��� ������ �����

                IsProcessVE_FindFile++;
                FrameManager->EnterModalEV();
                FrameManager->ExecuteModal();
                FrameManager->ExitModalEV();
                IsProcessVE_FindFile--;

                ffCS.Enter ();

                // ���������� ���������� �����
                FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
              }

              Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
              Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,TRUE,0);
            }
            else
            {
              Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,FALSE,0);
              Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
              {
/* $ 14.08.2002 VVM
  ! ����-��� �������� �� ������ ������������� � �������� ��������.
    � ���������, ������� �� ��� �� �������� ������
                int FramePos=FrameManager->FindFrameByFile(MODALTYPE_EDITOR,SearchFileName);
                int SwitchTo=FALSE;
                if (FramePos!=-1)
                {
                  if (!(*FrameManager)[FramePos]->GetCanLoseFocus(TRUE) ||
                      Opt.Confirm.AllowReedit)
                  {
                    char MsgFullFileName[NM];
                    xstrncpy(MsgFullFileName,SearchFileName,sizeof(MsgFullFileName)-1);
                    int MsgCode=Message(0,2,MSG(MFindFileTitle),
                          TruncPathStr(MsgFullFileName,ScrX-16),
                          MSG(MAskReload),
                          MSG(MCurrent),MSG(MNewOpen));
                    if (MsgCode==0)
                    {
                      SwitchTo=TRUE;
                    }
                    else if (MsgCode==1)
                    {
                      SwitchTo=FALSE;
                    }
                    else
                    {
                      Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
                      Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,TRUE,0);
                      return TRUE;
                    }
                  }
                  else
                  {
                    SwitchTo=TRUE;
                  }
                }
                if (SwitchTo)
                {
                  (*FrameManager)[FramePos]->SetCanLoseFocus(FALSE);
                  (*FrameManager)[FramePos]->SetDynamicallyBorn(FALSE);
                  FrameManager->ActivateFrame(FramePos);
                  IsProcessVE_FindFile++;
                  FrameManager->EnterModalEV();
                  FrameManager->ExecuteModal ();
                  FrameManager->ExitModalEV();
//                  FrameManager->ExecuteNonModal();
                  IsProcessVE_FindFile--;
                  // ���������� ���������� �����
                  FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
                }
                else
*/
                {
                  FileEditor ShellEditor (strSearchFileName,FALSE,FALSE);
                  ShellEditor.SetDynamicallyBorn(FALSE);
                  ShellEditor.SetEnableF6 (TRUE);
                  // FindFileArcIndex ������ ����� ������������
                  // �� ����� ���� ��� ������.
                  if ((FindList[ItemIndex]->ArcIndex != LIST_INDEX_NONE) &&
                      (!(ArcList[FindList[ItemIndex]->ArcIndex]->Flags & OPIF_REALNAMES)))
                    ShellEditor.SetSaveToSaveAs(TRUE);

                  ffCS.Leave (); // ����� ����� �����������, ���� �� ��� ������ �����
                  IsProcessVE_FindFile++;
                  FrameManager->EnterModalEV();
                  FrameManager->ExecuteModal ();
                  FrameManager->ExitModalEV();
                  IsProcessVE_FindFile--;

                  ffCS.Enter ();
                  // ���������� ���������� �����
                  FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
                }
              }
              Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
              Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,TRUE,0);
            }
            SetConsoleTitleW (strOldTitle);
          }
          if (RemoveTemp)
          {
            DeleteFileWithFolderW(strSearchFileName);
          }
        }

        ffCS.Leave ();

        return TRUE;
      }
      Result = Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
      return Result;
    }
    case DN_BTNCLICK:
    {

      FindPositionChanged = TRUE;

      if (Param1==5) // [ New search ] button pressed
      {
        StopSearch=TRUE;
        FindExitCode=FIND_EXIT_SEARCHAGAIN;
        return FALSE;
      }
      else if (Param1==9) // [ Stop ] button pressed
      {
        if (StopSearch)
          return FALSE;
        StopSearch=TRUE;
        return TRUE;
      }
      else if (Param1==6) // [ Goto ] button pressed
      {
        if (!ListBox)
        {
          return FALSE;
        }

        // ������� ����� ������ ��� �� ����� ������ �� �������.
        // ������� ������ ��� [ Panel ]
        if (ListBox->GetItemCount()==0)
        {
          return (TRUE);
        }
        FindExitIndex = (DWORD)ListBox->GetUserData(NULL, 0);
        if (FindExitIndex != LIST_INDEX_NONE)
          FindExitCode = FIND_EXIT_GOTO;
        return (FALSE);
      }
      else if (Param1==7) // [ View ] button pressed
      {
        FindDlgProc(hDlg,DN_KEY,1,KEY_F3);
        return TRUE;
      }
      else if (Param1==8) // [ Panel ] button pressed
      {
        if (!ListBox)
        {
          return FALSE;
        }

        // �� ������ ����� �������� �� � �������, � �����.
        // ����� ��������� ������. ����� �������� ��������, ����� ��
        // ���� �� ������, ����� �� ������� � ������� ����� (� �����-��
        // ����!) � � ���������� ��� ���������.
        if (ListBox->GetItemCount()==0)
        {
          return (TRUE);
        }
        FindExitCode = FIND_EXIT_PANEL;
        FindExitIndex = (DWORD)ListBox->GetUserData(NULL, 0);
        return (FALSE);
      }
    }
    case DN_CTLCOLORDLGLIST:
    {
      if (Param2)
        ((struct FarListColors *)Param2)->Colors[VMenuColorDisabled]=FarColorToReal(COL_DIALOGLISTTEXT);
      return TRUE;
    }
    case DN_CLOSE:
    {
      StopSearch=TRUE;
      return TRUE;
    }
    /* 10.08.2001 KM
       ��������� �������� ������� ������ ��� ��������� �������� �������.
    */
    case DN_RESIZECONSOLE:
    {
      COORD coord=(*(COORD*)Param2);
      SMALL_RECT rect;
      int IncY=coord.Y-DlgHeight-3;
      int I;

      Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
      for (I=0;I<10;I++)
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,I,FALSE);

      Dialog::SendDlgMessage(hDlg,DM_GETDLGRECT,0,(long)&rect);
      coord.X=rect.Right-rect.Left+1;
      DlgHeight+=IncY;
      coord.Y=DlgHeight;

      if (IncY>0)
        Dialog::SendDlgMessage(hDlg,DM_RESIZEDIALOG,0,(long)&coord);

      for (I=0;I<2;I++)
      {
        Dialog::SendDlgMessage(hDlg,DM_GETITEMPOSITION,I,(long)&rect);
        rect.Bottom+=(short)IncY;
        Dialog::SendDlgMessage(hDlg,DM_SETITEMPOSITION,I,(long)&rect);
      }

      for (I=2;I<10;I++)
      {
        Dialog::SendDlgMessage(hDlg,DM_GETITEMPOSITION,I,(long)&rect);
        if (I==2)
          rect.Left=-1;
        rect.Top+=(short)IncY;
        Dialog::SendDlgMessage(hDlg,DM_SETITEMPOSITION,I,(long)&rect);
      }

      if (!(IncY>0))
        Dialog::SendDlgMessage(hDlg,DM_RESIZEDIALOG,0,(long)&coord);

      for (I=0;I<10;I++)
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,I,TRUE);
      Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);

      return TRUE;
    }
    /* KM $ */
  }
  Result = Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
  return Result;
}

int FindFiles::FindFilesProcess()
{
  _ALGO(CleverSysLog clv("FindFiles::FindFilesProcess()"));
  // � ����������� ��������� ����� � ����������� ����������
  static string strTitle = L"";
  static string strSearchStr;

  /* $ 05.10.2003 KM
     ���� ������������ ������ ��������, �� �� ����� ������ �������� �� ����
  */
  if ( !strFindMask.IsEmpty() )
    if (UseFilter)
      strTitle.Format (L"%s: %s (%s)",UMSG(MFindFileTitle),(const wchar_t*)strFindMask,UMSG(MFindUsingFilter));
    else
      strTitle.Format (L"%s: %s",UMSG(MFindFileTitle),(const wchar_t*)strFindMask);
  else
    if (UseFilter)
      strTitle.Format (L"%s (%s)",UMSG(MFindFileTitle),UMSG(MFindUsingFilter));
    else
      strTitle.Format (L"%s", UMSG(MFindFileTitle));

  if ( !strFindStr.IsEmpty() )
  {
    string strTemp, strFStr;

    strFStr = strFindStr;

    TruncStrFromEndW(strFStr,10);
    strTemp.Format (L" \"%s\"", (const wchar_t*)strFStr);
    strSearchStr.Format(UMSG(MFindSearchingIn), (const wchar_t*)strTemp);
  }
  else
    strSearchStr.Format (UMSG(MFindSearchingIn), L"");

  struct DialogDataEx FindDlgData[]={
  /* 00 */DI_DOUBLEBOX,3,1,DLG_WIDTH,DLG_HEIGHT-4,0,0,DIF_SHOWAMPERSAND,0,strTitle,
  /* 01 */DI_LISTBOX,4,2,73,DLG_HEIGHT-9,0,0,DIF_LISTNOBOX,0,(const wchar_t*)0,
  /* 02 */DI_TEXT,-1,DLG_HEIGHT-8,0,DLG_HEIGHT-8,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 03 */DI_TEXT,5,DLG_HEIGHT-7,0,DLG_HEIGHT-7,0,0,DIF_SHOWAMPERSAND,0,strSearchStr,
  /* 04 */DI_TEXT,3,DLG_HEIGHT-6,0,DLG_HEIGHT-6,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 05 */DI_BUTTON,0,DLG_HEIGHT-5,0,DLG_HEIGHT-5,1,0,DIF_CENTERGROUP,1,(const wchar_t *)MFindNewSearch,
  /* 06 */DI_BUTTON,0,DLG_HEIGHT-5,0,DLG_HEIGHT-5,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MFindGoTo,
  /* 07 */DI_BUTTON,0,DLG_HEIGHT-5,0,DLG_HEIGHT-5,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MFindView,
  /* 08 */DI_BUTTON,0,DLG_HEIGHT-5,0,DLG_HEIGHT-5,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MFindPanel,
  /* 09 */DI_BUTTON,0,DLG_HEIGHT-5,0,DLG_HEIGHT-5,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MFindStop
  };

  MakeDialogItemsEx(FindDlgData,FindDlg);

  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  DlgHeight=DLG_HEIGHT-2;
  DlgWidth=FindDlg[0].X2-FindDlg[0].X1-4;

  int IncY=ScrY>24 ? ScrY-24:0;
  FindDlg[0].Y2+=IncY;
  FindDlg[1].Y2+=IncY;
  FindDlg[2].Y1+=IncY;
  FindDlg[3].Y1+=IncY;
  FindDlg[4].Y1+=IncY;
  FindDlg[5].Y1+=IncY;
  FindDlg[6].Y1+=IncY;
  FindDlg[7].Y1+=IncY;
  FindDlg[8].Y1+=IncY;
  FindDlg[9].Y1+=IncY;

  DlgHeight+=IncY;

  if (PluginMode)
  {
    Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
    HANDLE hPlugin=ActivePanel->GetPluginHandle();
    struct OpenPluginInfoW Info;
    CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);

    FindFileArcIndex = AddArcListItem(Info.HostFile, hPlugin, Info.Flags, Info.CurDir);

    if (FindFileArcIndex == LIST_INDEX_NONE)
      return(FALSE);

    if ((Info.Flags & OPIF_REALNAMES)==0)
    {
      FindDlg[8].Type=DI_TEXT;
      FindDlg[8].strData=L"";
    }
  }

  Dialog Dlg=Dialog(FindDlg,sizeof(FindDlg)/sizeof(FindDlg[0]),FindDlgProc);
  hDlg=(HANDLE)&Dlg;
//  pDlg->SetDynamicallyBorn();
  Dlg.SetHelp(L"FindFileResult");
  Dlg.SetPosition(-1,-1,DLG_WIDTH+4,DLG_HEIGHT-2+IncY);
  // ���� �� �������� ������, � �� ������������� ��������� �����������
  // ������ ��� ������ � ������ �������� �� �����������
  Dlg.InitDialog();
  Dlg.Show();

  LastFoundNumber=0;
  SearchDone=FALSE;
  StopSearch=FALSE;
  PauseSearch=FALSE;
  WriteDataUsed=FALSE;
  PrepareFilesListUsed=0;
  FindFileCount=FindDirCount=0;
  FindExitIndex = LIST_INDEX_NONE;
  FindExitCode = FIND_EXIT_NONE;
  FindMessageReady=FindCountReady=FindPositionChanged=0;

  if (PluginMode)
  {
    if (_beginthread(PreparePluginList,0,NULL)==(unsigned long)-1)
      return FALSE;
  }
  else
  {
    if (_beginthread(PrepareFilesList,0,NULL)==(unsigned long)-1)
      return FALSE;
  }

  // ����� ��� ������ � ������� ���������� � ���� ������
  if (_beginthread(WriteDialogData,0,NULL)==(unsigned long)-1)
    return FALSE;

  IsProcessAssignMacroKey++; // �������� ��� ����. �������
  Dlg.Process();
  IsProcessAssignMacroKey--;

  while(WriteDataUsed || PrepareFilesListUsed > 0)
    Sleep(10);

  ::hDlg=NULL;

  switch (FindExitCode)
  {
    case FIND_EXIT_SEARCHAGAIN:
    {
      return TRUE;
    }
    case FIND_EXIT_PANEL:
    // ���������� ���������� �� ��������� ������
    {
      int ListSize = FindListCount;
      PluginPanelItemW *PanelItems=new PluginPanelItemW[ListSize];
      if (PanelItems==NULL)
        ListSize=0;
      int ItemsNumber=0;
      for (int i=0;i<ListSize;i++)
      {
        if (wcslen(FindList[i]->FindData.strFileName)>0 && FindList[i]->Used)
        // ��������� ������, ���� ��� ������
        {
          // ��� �������� � ������������ ������� ������� ��� ����� �� ��� ������.
          // ������ ���� ������ ������ �����.
          int IsArchive = ((FindList[i]->ArcIndex != LIST_INDEX_NONE) &&
                          !(ArcList[FindList[i]->ArcIndex]->Flags&OPIF_REALNAMES));

          // ��������� ������ ����� ��� ����� ������� ��� ����� ����� �������
          if (IsArchive || (Opt.FindOpt.FindFolders && !SearchHex) ||
              !(FindList[i]->FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
          {
            if (IsArchive)
              FindList[i]->FindData.strFileName = ArcList[FindList[i]->ArcIndex]->strArcName;

            PluginPanelItemW *pi=&PanelItems[ItemsNumber++];
            memset(pi,0,sizeof(*pi));

            apiFindDataExToData (&FindList[i]->FindData, &pi->FindData);

            if (IsArchive)
              pi->FindData.dwFileAttributes = 0;
            /* $ 21.03.2002 VVM
              + �������� ����� ��������� ��� ��������������� "\" */
            if (pi->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
              int Length = wcslen(pi->FindData.lpwszFileName);
              if ((Length) && (pi->FindData.lpwszFileName[Length-1]=='\\'))
                pi->FindData.lpwszFileName[Length-1] = 0;
            }
            /* VVM $ */
          }
        } /* if */
      } /* for */

      HANDLE hNewPlugin=CtrlObject->Plugins.OpenFindListPlugin(PanelItems,ItemsNumber);
      if (hNewPlugin!=INVALID_HANDLE_VALUE)
      {
        Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
        Panel *NewPanel=CtrlObject->Cp()->ChangePanel(ActivePanel,FILE_PANEL,TRUE,TRUE);
        NewPanel->SetPluginMode(hNewPlugin,"");
        NewPanel->SetVisible(TRUE);
        NewPanel->Update(0);
//        if (FindExitIndex != LIST_INDEX_NONE)
//          NewPanel->GoToFile(FindList[FindExitIndex].FindData.cFileName);
        NewPanel->Show();
        NewPanel->SetFocus();
      }

      for (int i = 0; i < ItemsNumber; i++)
          apiFreeFindData (&PanelItems[i].FindData);

      delete[] PanelItems;
      break;
    } /* case FIND_EXIT_PANEL */
    case FIND_EXIT_GOTO:
    {
      string strFileName=FindList[FindExitIndex]->FindData.strFileName;
      Panel *FindPanel=CtrlObject->Cp()->ActivePanel;

      if ((FindList[FindExitIndex]->ArcIndex != LIST_INDEX_NONE) &&
          (!(ArcList[FindList[FindExitIndex]->ArcIndex]->Flags & OPIF_REALNAMES)))
      {
        HANDLE hPlugin = ArcList[FindList[FindExitIndex]->ArcIndex]->hPlugin;
        if (hPlugin == INVALID_HANDLE_VALUE)
        {
          string strArcName, strArcPath;

          strArcName = ArcList[FindList[FindExitIndex]->ArcIndex]->strArcName;
          if (FindPanel->GetType()!=FILE_PANEL)
            FindPanel=CtrlObject->Cp()->ChangePanel(FindPanel,FILE_PANEL,TRUE,TRUE);

          strArcPath = strArcName;

          CutToSlashW (strArcPath);
          FindPanel->SetCurDirW(strArcPath,TRUE);
          hPlugin=((FileList *)FindPanel)->OpenFilePlugin(strArcName,FALSE);
          if (hPlugin==(HANDLE)-2)
            hPlugin = INVALID_HANDLE_VALUE;
        } /* if */
        if (hPlugin != INVALID_HANDLE_VALUE)
        {
          struct OpenPluginInfoW Info;
          CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);

          /* $ 19.01.2003 KM
             ��������� �������� � ������ ������� �������.
          */
          if (SearchMode==SEARCH_ROOT ||
              SearchMode==SEARCH_ALL ||
              SearchMode==SEARCH_ALL_BUTNETWORK ||
              SearchMode==SEARCH_INPATH)
            CtrlObject->Plugins.SetDirectory(hPlugin,L"\\",OPM_FIND);

          SetPluginDirectory(strFileName,hPlugin,TRUE);
          /* KM $ */
        }
      } /* if */
      else
      {
        string strSetName;
        int Length;
        if ((Length=strFileName.GetLength())==0)
          break;

        wchar_t *lpwszFileName = strFileName.GetBuffer();

        if (Length>1 && lpwszFileName[Length-1]==L'\\' && lpwszFileName[Length-2]!=L':')
          lpwszFileName[Length-1]=0;

        strFileName.ReleaseBuffer();

        #if !defined(INVALID_FILE_ATTRIBUTES)
        #define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
        #endif
        if ( (GetFileAttributesW(strFileName)==INVALID_FILE_ATTRIBUTES) && (GetLastError () != ERROR_ACCESS_DENIED))
          break;
        {
          wchar_t *NamePtr = strFileName.GetBuffer();
          NamePtr=(wchar_t*)PointToNameW(NamePtr);

          strSetName = NamePtr;
          *NamePtr=0;

          strFileName.ReleaseBuffer();

          Length=strFileName.GetLength();

          lpwszFileName = strFileName.GetBuffer();

          if (Length>1 && lpwszFileName[Length-1]==L'\\' && lpwszFileName[Length-2]!=L':')
            lpwszFileName[Length-1]=0;

          strFileName.ReleaseBuffer();
        }
        if ( strFileName.IsEmpty() )
          break;
        if (FindPanel->GetType()!=FILE_PANEL &&
            CtrlObject->Cp()->GetAnotherPanel(FindPanel)->GetType()==FILE_PANEL)
          FindPanel=CtrlObject->Cp()->GetAnotherPanel(FindPanel);
        if ((FindPanel->GetType()!=FILE_PANEL) || (FindPanel->GetMode()!=NORMAL_PANEL))
        // ������ ������ �� ������� ��������...
        {
          FindPanel=CtrlObject->Cp()->ChangePanel(FindPanel,FILE_PANEL,TRUE,TRUE);
          FindPanel->SetVisible(TRUE);
          FindPanel->Update(0);
        }
        /* $ 09.06.2001 IS
           ! �� ������ �������, ���� �� ��� � ��� ���������. ��� �����
             ���������� ����, ��� ��������� � ��������� ������ �� ������������.
        */
        {
          string strDirTmp;
          FindPanel->GetCurDirW(strDirTmp);
          Length=strDirTmp.GetLength();

          wchar_t *lpwszDirTmp = strDirTmp.GetBuffer();

          if (Length>1 && lpwszDirTmp[Length-1]==L'\\' && lpwszDirTmp[Length-2]!=L':')
            lpwszDirTmp[Length-1]=0;

          strDirTmp.ReleaseBuffer();

          if(0!=LocalStricmpW(strFileName, strDirTmp))
            FindPanel->SetCurDirW(strFileName,TRUE);
        }
        /* IS $ */
        if ( !strSetName.IsEmpty() )
          FindPanel->GoToFileW(strSetName);
        FindPanel->Show();
        FindPanel->SetFocus();
      }
      break;
    } /* case FIND_EXIT_GOTO */
  } /* switch */

  return FALSE;
}


void FindFiles::SetPluginDirectory(const wchar_t *DirName,HANDLE hPlugin,int UpdatePanel)
{
  string strName;
  wchar_t *StartName,*EndName;
  int IsPluginDir;

  /* $ 19.01.2003 KM
     ����������� ��������� �� 4 ����. ���� � DirName ����
     ������ '\x1' ������ ��� ���� �� �������. ����� �������
     ����� ���������� ���������� ���� �, ��������������,
     ������� ���������� �������.
  */
  if ( wcslen(DirName)>0 )
    IsPluginDir=wcschr(DirName,L'\x1')!=NULL;
  else
    IsPluginDir=FALSE;

  strName = DirName;
  StartName=strName.GetBuffer();
  while(IsPluginDir?(EndName=wcschr(StartName,L'\x1'))!=NULL:(EndName=wcschr(StartName,L'\\'))!=NULL)
  /* KM $ */
  {
    *EndName=0;
    // RereadPlugin
    {
      int FileCount=0;
      PluginPanelItemW *PanelData=NULL;
      if (CtrlObject->Plugins.GetFindData(hPlugin,&PanelData,&FileCount,OPM_FIND))
        CtrlObject->Plugins.FreeFindData(hPlugin,PanelData,FileCount);
    }
    CtrlObject->Plugins.SetDirectory(hPlugin,StartName,OPM_FIND);
    StartName=EndName+1;
  }
  /* $ 19.01.2003 KM
     �������� ������ ��� �������������.
  */
  if (UpdatePanel)
  {
    CtrlObject->Cp()->ActivePanel->Update(UPDATE_KEEP_SELECTION);
    if (!CtrlObject->Cp()->ActivePanel->GoToFileW(StartName))
      CtrlObject->Cp()->ActivePanel->GoToFileW(DirName);
    CtrlObject->Cp()->ActivePanel->Show();
  }

  strName.ReleaseBuffer();
  /* KM $ */
}


#if defined(__BORLANDC__)
#pragma warn -par
#endif
void FindFiles::DoScanTree(string& strRoot, FAR_FIND_DATA_EX& FindData, string& strFullName)
{
      {
        ScanTree ScTree(FALSE,!(SearchMode==SEARCH_CURRENT_ONLY||SearchMode==SEARCH_INPATH),SearchInSymLink);

        string strSelName;
        int FileAttr;
        if (SearchMode==SEARCH_SELECTED)
          CtrlObject->Cp()->ActivePanel->GetSelNameW(NULL,FileAttr);

        while (1)
        {
          string strCurRoot;
          if (SearchMode==SEARCH_SELECTED)
          {
            if (!CtrlObject->Cp()->ActivePanel->GetSelNameW(&strSelName,FileAttr))
              break;
            if ((FileAttr & FILE_ATTRIBUTE_DIRECTORY)==0 || TestParentFolderNameW(strSelName) ||
                wcscmp(strSelName,L".")==0)
              continue;


            strCurRoot = strRoot;
            AddEndSlashW(strCurRoot);
            strCurRoot += strSelName;
          }
          else
            strCurRoot = strRoot;

          ScTree.SetFindPathW(strCurRoot,L"*.*");

          statusCS.Enter ();

          strFindMessage = strCurRoot;
          FindMessageReady=TRUE;

          statusCS.Leave ();

          while (!StopSearch && ScTree.GetNextNameW(&FindData,strFullName))
          {
            while (PauseSearch)
              Sleep(10);

            /* $ 30.09.2003 KM
              ����������� ����� �� ���������� � ����������� ������
            */
            int IsFile;
            if (UseFilter)
              IsFile=Filter->FileInFilter(&FindData);
            else
              IsFile=TRUE;

            if (IsFile)
            {
              /* $ 14.06.2004 KM
                ��������� �������� ��� ��������� ���������
              */
              if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
              {
                statusCS.Enter();

                strFindMessage = strFullName;
                FindMessageReady=TRUE;

                statusCS.Leave();
              }
              /* KM $ */

              if (IsFileIncluded(NULL,strFullName,FindData.dwFileAttributes))
                AddMenuRecord(strFullName,&FindData);

              if (SearchInArchives)
                ArchiveSearch(strFullName);
            }
            /* KM $ */
          }
          if (SearchMode!=SEARCH_SELECTED)
            break;
        }
      }
}
void _cdecl FindFiles::DoPrepareFileList(string& strRoot, FAR_FIND_DATA_EX& FindData, string& strFullName)
{
  TRY {
    wchar_t *PathEnv=NULL, *Ptr; //BUGBUG
      PrepareFilesListUsed++;
    DWORD DiskMask=FarGetLogicalDrives();

    //string strRoot; //BUGBUG
    CtrlObject->CmdLine->GetCurDirW(strRoot);

    if(SearchMode==SEARCH_INPATH)
    {
      DWORD SizeStr=GetEnvironmentVariableW(L"PATH",NULL,0);
      if((PathEnv=(wchar_t *)alloca((SizeStr+2)*sizeof (wchar_t))) != NULL)
      {
        GetEnvironmentVariableW(L"PATH",PathEnv,SizeStr+1);
        PathEnv[wcslen(PathEnv)]=0;
        Ptr=PathEnv;
        while(*Ptr)
        {
          if(*Ptr==L';')
            *Ptr=0;
          ++Ptr;
        }
      }
      Ptr=PathEnv;
    }

    for (int CurrentDisk=0;;CurrentDisk++,DiskMask>>=1)
    {
      if (SearchMode==SEARCH_ALL ||
          SearchMode==SEARCH_ALL_BUTNETWORK)
      {
        if(DiskMask==0)
          break;

        if ((DiskMask & 1)==0)
          continue;

        strRoot.Format (L"%c:\\", L'A'+CurrentDisk);
        int DriveType=FAR_GetDriveTypeW(strRoot);
        if (DriveType==DRIVE_REMOVABLE || IsDriveTypeCDROM(DriveType) ||
           (DriveType==DRIVE_REMOTE && SearchMode==SEARCH_ALL_BUTNETWORK))
          if (DiskMask==1)
            break;
          else
            continue;
      }
      else if (SearchMode==SEARCH_ROOT)
        GetPathRootOneW(strRoot,strRoot);
      else if(SearchMode==SEARCH_INPATH)
      {
        if(!*Ptr)
          break;
        strRoot = Ptr;
        Ptr+=wcslen(Ptr)+1;
      }

      DoScanTree(strRoot, FindData, strFullName);

      if (SearchMode!=SEARCH_ALL && SearchMode!=SEARCH_ALL_BUTNETWORK && SearchMode!=SEARCH_INPATH)
        break;
    }

    while (!StopSearch && FindMessageReady)
      Sleep(10);
  //  sprintf(FindMessage,MSG(MFindDone),FindFileCount,FindDirCount);

    statusCS.Enter ();

    strFindMessage.Format (UMSG(MFindDone),FindFileCount,FindDirCount);

    SetFarTitleW (strFindMessage);

    SearchDone=TRUE;
    FindMessageReady=TRUE;

    statusCS.Leave ();

    PrepareFilesListUsed--;
  }
  EXCEPT (xfilter((int)INVALID_HANDLE_VALUE,GetExceptionInformation(),NULL,1))
  {
    TerminateProcess( GetCurrentProcess(), 1);
  }
}
void _cdecl FindFiles::PrepareFilesList(void *Param)
{
  FAR_FIND_DATA_EX FindData;
  string strFullName;
  string strRoot;

  DoPrepareFileList(strRoot, FindData, strFullName);

}
#if defined(__BORLANDC__)
#pragma warn +par
#endif


void FindFiles::ArchiveSearch(const wchar_t *ArcName)
{
  _ALGO(CleverSysLog clv("FindFiles::ArchiveSearch()"));
  _ALGO(SysLog("ArcName='%s'",(ArcName?ArcName:"NULL")));
  char *Buffer=new char[Opt.PluginMaxReadData];
  FILE *ProcessFile=_wfopen(ArcName,L"rb");
  if (ProcessFile==NULL)
  {
    /* $ 13.07.2000 SVS
       ������������ new[]
    */
    delete[] Buffer;
    /* SVS $ */
    return;
  }
  int ReadSize=fread(Buffer,1,Opt.PluginMaxReadData,ProcessFile);
  fclose(ProcessFile);

  int SavePluginsOutput=DisablePluginsOutput;
  DisablePluginsOutput=TRUE;

  string strArcName = ArcName;

  HANDLE hArc=CtrlObject->Plugins.OpenFilePlugin(strArcName,(unsigned char *)Buffer,ReadSize);
  /* $ 01.10.2001 VVM */
  DisablePluginsOutput=SavePluginsOutput;
  /* VVM $ */


  /* $ 13.07.2000 SVS
     ������������ new[]
  */
  delete[] Buffer;
  /* SVS $ */

  if (hArc==(HANDLE)-2)
  {
    BreakMainThread=TRUE;
    return;
  }
  if (hArc==INVALID_HANDLE_VALUE)
    return;

  int SaveSearchMode=SearchMode;
  DWORD SaveArcIndex = FindFileArcIndex;
  {
    SearchMode=SEARCH_FROM_CURRENT;
    struct OpenPluginInfoW Info;
    CtrlObject->Plugins.GetOpenPluginInfo(hArc,&Info);

    FindFileArcIndex = AddArcListItem(ArcName, hArc, Info.Flags, Info.CurDir);
    /* $ 11.12.2001 VVM
      - �������� ������� ����� ������� � ������.
        � ���� ������ �� ����� - �� ������ ��� ����� */
    {
      char SaveSearchPath[2*NM];
      string strSaveDirName;
      int SaveListCount = FindListCount;
      /* $ 19.01.2003 KM
         �������� ���� ������ � �������, ��� ����� ����������.
      */
      xstrncpy(SaveSearchPath,PluginSearchPath,2*NM);
      /* KM $ */

      strSaveDirName = strLastDirName;
      strLastDirName = L"";
      PreparePluginList((void *)1);
      xstrncpy(PluginSearchPath,SaveSearchPath,2*NM);
      WaitForSingleObject(hPluginMutex,INFINITE);
      CtrlObject->Plugins.ClosePlugin(ArcList[FindFileArcIndex]->hPlugin);
      ArcList[FindFileArcIndex]->hPlugin = INVALID_HANDLE_VALUE;
      ReleaseMutex(hPluginMutex);
      if (SaveListCount == FindListCount)
        strLastDirName = strSaveDirName;
    }
    /* VVM $ */
  }
  FindFileArcIndex = SaveArcIndex;
  SearchMode=SaveSearchMode;
}

/* $ 01.07.2001 IS
   ���������� FileMaskForFindFile ������ GetCommaWord
*/
int FindFiles::IsFileIncluded(PluginPanelItemW *FileItem,const wchar_t *FullName,DWORD FileAttr)
{
  CriticalSectionLock Lock (ffCS);

  int FileFound=FileMaskForFindFile.Compare(FullName);
  HANDLE hPlugin=INVALID_HANDLE_VALUE;
  if (FindFileArcIndex != LIST_INDEX_NONE)
    hPlugin = ArcList[FindFileArcIndex]->hPlugin;
  while(FileFound)
  {
    /* $ 24.09.2003 KM
       ���� ������� ����� ������ hex-�����, ����� ����� � ����� �� ��������
    */
    /* $ 17.01.2002 VVM
      ! ��������� ������ � ������� � ������ ������� � ������ -
        ���� � ������� ������� ���� ������������ */
    if ((FileAttr & FILE_ATTRIBUTE_DIRECTORY) && ((Opt.FindOpt.FindFolders==0) || SearchHex))
//        ((hPlugin == INVALID_HANDLE_VALUE) ||
//        (ArcList[FindFileArcIndex].Flags & OPIF_FINDFOLDERS)==0))
      return FALSE;
    /* VVM $ */
    /* KM $ */

    if ( !strFindStr.IsEmpty() && FileFound)
    {
      FileFound=FALSE;
      if (FileAttr & FILE_ATTRIBUTE_DIRECTORY)
        break;
      string strSearchFileName;
      int RemoveTemp=FALSE;
      if ((hPlugin != INVALID_HANDLE_VALUE) && (ArcList[FindFileArcIndex]->Flags & OPIF_REALNAMES)==0)
      {
        string strTempDir;
        FarMkTempExW(strTempDir); // � �������� �� NULL???
        CreateDirectoryW(strTempDir,NULL);
        WaitForSingleObject(hPluginMutex,INFINITE);
        if (!CtrlObject->Plugins.GetFile(hPlugin,FileItem,strTempDir,strSearchFileName,OPM_SILENT|OPM_FIND))
        {
          ReleaseMutex(hPluginMutex);
          FAR_RemoveDirectoryW(strTempDir);
          break;
        }
        else
          ReleaseMutex(hPluginMutex);
        RemoveTemp=TRUE;
      }
      else
        strSearchFileName = FullName;

      if (LookForString(strSearchFileName))
        FileFound=TRUE;

      if (RemoveTemp)
      {
        DeleteFileWithFolderW(strSearchFileName);
      }
    }
    break;
  }
  return(FileFound);
}
/* IS $ */


void FindFiles::AddMenuRecord(const wchar_t *FullName, FAR_FIND_DATA *FindData)
{
    FAR_FIND_DATA_EX fdata;

    apiFindDataToDataEx (FindData, &fdata);

    AddMenuRecord (FullName, &fdata);
}

void FindFiles::AddMenuRecord(const wchar_t *FullName, FAR_FIND_DATA_EX *FindData)
{
  string strMenuText, strFileText, strSizeText;
  string strAttr;
  string strDateStr, strTimeStr, strDate;
  MenuItemEx ListItem;

  Dialog* Dlg=(Dialog*)hDlg;
  if(!Dlg)
  {
    return;
  }

  VMenu *ListBox=Dlg->Item[1]->ListPtr;

  if (!ListBox)
  {
    return;
  }

  ListItem.Clear ();

  if (FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    strSizeText.Format (L"%13s", UMSG(MFindFileFolder));
  else
  {
    wchar_t *wszSizeText = strSizeText.GetBuffer ();

    _ui64tow(FindData->nFileSize, wszSizeText,10);

    strSizeText.ReleaseBuffer ();
  }
  const wchar_t *DisplayName=FindData->strFileName;
  /* $ 24.03.2002 KM
     � �������� ������������� �������� ��������� � ����� �� ���
     ��� ����������� ��� ����������� � ������, �������� ����,
     �.�. ��������� ������� ���������� ��� ������ � ������ ����,
     � ������� ��������� ������.
  */
  if (FindFileArcIndex != LIST_INDEX_NONE)
    DisplayName=PointToNameW(DisplayName);
  /* KM $ */

  strFileText.Format (L" %-26.26s %13.13s",DisplayName,(const wchar_t*)strSizeText);


  /* $ 05.10.2003 KM
     ��� ������������� ������� �� ���� ����� ���������� ��� ���
     ����, ������� ����� � �������.
  */
  if (UseFilter && Filter->GetParams().FDate.Used)
  {
    switch(Filter->GetParams().FDate.DateType)
    {
      case FDATE_MODIFIED:
        // ���������� ���� ���������� ���������
        ConvertDateW(FindData->ftLastWriteTime,strDateStr,strTimeStr,5);
        break;
      case FDATE_CREATED:
        // ���������� ���� ��������
        ConvertDateW(FindData->ftCreationTime,strDateStr,strTimeStr,5);
        break;
      case FDATE_OPENED:
        // ���������� ���� ���������� �������
        ConvertDateW(FindData->ftLastAccessTime,strDateStr,strTimeStr,5);
        break;
    }
  }
  else
    // ���������� ���� ���������� ���������
    ConvertDateW(FindData->ftLastWriteTime,strDateStr,strTimeStr,5);
  /* KM $ */
  strDate.Format (L"  %s  %s", (const wchar_t*)strDateStr, (const wchar_t*)strTimeStr);
  strFileText += strDate;

  /* $ 05.10.2003 KM
     ��������� � ������ ������ �������� ��������� ������
  */
  wchar_t AttrStr[16];
  DWORD FileAttr=FindData->dwFileAttributes;
  AttrStr[0]=(FileAttr & FILE_ATTRIBUTE_READONLY) ? L'R':L' ';
  AttrStr[1]=(FileAttr & FILE_ATTRIBUTE_SYSTEM) ? L'S':L' ';
  AttrStr[2]=(FileAttr & FILE_ATTRIBUTE_HIDDEN) ? L'H':L' ';
  AttrStr[3]=(FileAttr & FILE_ATTRIBUTE_ARCHIVE) ? L'A':L' ';
  AttrStr[4]=(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT) ? L'L' : ((FileAttr & FILE_ATTRIBUTE_SPARSE_FILE) ? L'$':L' ');
  AttrStr[5]=(FileAttr & FILE_ATTRIBUTE_COMPRESSED) ? L'C':((FileAttr & FILE_ATTRIBUTE_ENCRYPTED)?L'E':L' ');
  AttrStr[6]=(FileAttr & FILE_ATTRIBUTE_TEMPORARY) ? L'T':' ';
  AttrStr[7]=(FileAttr & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED) ? L'I':L' ';
  AttrStr[8]=0;

  strAttr.Format (L" %s", AttrStr);
  strFileText += strAttr;
  /* KM $ */
  strMenuText.Format (L" %-*.*s",DlgWidth-3,DlgWidth-3, (const wchar_t*)strFileText);

  string strPathName;
  strPathName = FullName;

  RemovePseudoBackSlashW(strPathName);

  CutToSlashW(strPathName);

  if ( strPathName.IsEmpty() )
      strPathName = L".\\";

  AddEndSlashW(strPathName);

  if ( LocalStricmpW(strPathName,strLastDirName)!=0)
  {
    if ( !strLastDirName.IsEmpty() )
    {
      /* $ 12.05.2001 DJ
         ������ �� ��������������� �� ������ ������� ����� ����������
      */
      ListItem.Flags|=LIF_DISABLE;
      // � ������ VVM ������� ���������� � ������ ������� LIST_INDEX_NONE �� ������ �������
      ListBox->SetUserData((void*)LIST_INDEX_NONE,sizeof(LIST_INDEX_NONE),ListBox->AddItemW(&ListItem));
      ListItem.Flags&=~LIF_DISABLE;
      /* DJ $ */
    }
    ffCS.Enter ();

    strLastDirName = strPathName;

    if ((FindFileArcIndex != LIST_INDEX_NONE) &&
        (!(ArcList[FindFileArcIndex]->Flags & OPIF_REALNAMES)) &&
        (!ArcList[FindFileArcIndex]->strArcName.IsEmpty()))
    {
      string strArcPathName;

      const wchar_t *ArcName = ArcList[FindFileArcIndex]->strArcName;

      strArcPathName.Format (L"%s:%s", ArcName, strPathName.At(0)==L'.' ? L"\\":(const wchar_t*)strPathName);
      strPathName = strArcPathName;
    }
    strSizeText = UMSG(MFindFileFolder);

    TruncPathStrW(strPathName,50);

    strFileText.Format (L"%-50.50s  <%6.6s>", (const wchar_t*)strPathName, (const wchar_t*)strSizeText);
    ListItem.strName.Format (L"%-*.*s",DlgWidth-2,DlgWidth-2, (const wchar_t*)strFileText);

    DWORD ItemIndex = AddFindListItem(FindData);
    if (ItemIndex != LIST_INDEX_NONE)
    {
      // ������� ������ � FindData. ��� ��� �� �����
      memset(&FindList[ItemIndex]->FindData,0,sizeof(FindList[ItemIndex]->FindData));
      // ���������� LastDirName, �.�. PathName ��� ����� ���� ��������

      FindList[ItemIndex]->FindData.strFileName = strLastDirName;
      /* $ 07.04.2002 KM
        - ������ ������� ����� ���������� ����, � ���������
          ������ �� �������� ������� �� ������� �� ������.
          Used=0 - ��� �� �������� �� ��������� ������.
      */
      FindList[ItemIndex]->Used=0;
      /* KM $ */
      // �������� ������� � ��������, ���-�� �� �� ��� ������ :)
      FindList[ItemIndex]->FindData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
      if (FindFileArcIndex != LIST_INDEX_NONE)
        FindList[ItemIndex]->ArcIndex = FindFileArcIndex;

      ListBox->SetUserData((void*)ItemIndex,sizeof(ItemIndex),
                           ListBox->AddItemW(&ListItem));
    }

    ffCS.Leave ();
  }

  /* $ 24.03.2002 KM
     ������������� ��������� � ������ ����������
     �����. ��� ���� � 1.65 � � 3 ����, �� ���
     ������ ��������� ������ � ��� ���-�� �������,
     ������ ��������� �� �����.
  */

  ffCS.Enter ();

  DWORD ItemIndex = AddFindListItem(FindData);
  if (ItemIndex != LIST_INDEX_NONE)
  {

    FindList[ItemIndex]->FindData.strFileName = FullName;
    /* $ 07.04.2002 KM
       Used=1 - ��� �������� �� ��������� ������.
    */
    FindList[ItemIndex]->Used=1;
    /* KM $ */
    if (FindFileArcIndex != LIST_INDEX_NONE)
      FindList[ItemIndex]->ArcIndex = FindFileArcIndex;
  }
  ListItem.strName = strMenuText;

  ffCS.Leave ();
  /* $ 17.01.2002 VVM
    ! �������� ����� �� � ���������, � � ������. ���� �� �������� ��������� */
//  ListItem.SetSelect(!FindFileCount);

  int ListPos = ListBox->AddItemW(&ListItem);
  ListBox->SetUserData((void*)ItemIndex,sizeof(ItemIndex), ListPos);
  // ������� ��� �������� - � ������.
  if (!FindFileCount && !FindDirCount)
    ListBox->SetSelectPos(ListPos, -1);
  /* VVM $ */

  statusCS.Enter();

  if (FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    FindDirCount++;
  else
    FindFileCount++;

  statusCS.Leave();
  /* KM $ */

  LastFoundNumber++;
  FindCountReady=TRUE;

}


int FindFiles::LookForString(const wchar_t *Name)
{
  FILE *SrcFile;

  char FindStr[512];

  UnicodeToAnsi(strFindStr, FindStr, 512);

  char Buf[32768],SaveBuf[32768],CmpStr[sizeof(FindStr)];
  int Length,ReadSize,SaveReadSize;
  if ((Length=strlen(FindStr))==0)
    return(TRUE);
  HANDLE FileHandle=FAR_CreateFileW(Name,GENERIC_READ|GENERIC_WRITE,
         FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
  if (FileHandle==INVALID_HANDLE_VALUE)
    FileHandle=FAR_CreateFileW(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
                          NULL,OPEN_EXISTING,0,NULL);
  if (FileHandle==INVALID_HANDLE_VALUE)
    return(FALSE);
  int Handle=_open_osfhandle((long)FileHandle,O_BINARY);
  if (Handle==-1)
    return(FALSE);
  if ((SrcFile=fdopen(Handle,"rb"))==NULL)
    return(FALSE);

  FILETIME LastAccess;
  int TimeRead=GetFileTime(FileHandle,NULL,&LastAccess,NULL);

  /* $ 26.10.2003 KM */
  if (SearchHex)
  {
    int LenCmpStr=sizeof(CmpStr);

    Transform((unsigned char *)CmpStr,LenCmpStr,(char *)FindStr,'S');
    Length=LenCmpStr;
  }
  else
    xstrncpy(CmpStr,FindStr,sizeof(CmpStr)-1);
  /* KM $ */

  if (!CmpCase && !SearchHex)
    LocalStrupr(CmpStr);
  /* $ 30.07.2000 KM
     ���������� ����������
  */
  int FirstIteration=TRUE;
  /* KM $ */
  int ReverseBOM=FALSE;
  int IsFirst=FALSE;

    // ��� ������� �� �����. ������������ ��� ���������
  // � ������������ ��������, � ������� ������������ �����
  __int64 AlreadyRead=_i64(0);

  while (!StopSearch && (ReadSize=fread(Buf,1,sizeof(Buf),SrcFile))>0)
  {
    /* $ 12.04.2005 KM
       ���� ������������ ����������� �� ������ �� ������ ������ �� �����,
       �������� �� ������� �� �� ��� ������� ����������� ����������� �������
    */
    if (EnableSearchInFirst && SearchInFirst)
    {
      if (AlreadyRead+ReadSize>SearchInFirst)
      {
        ReadSize=static_cast<int>(SearchInFirst-AlreadyRead);
      }

      if (ReadSize<=0)
        break;

      AlreadyRead+=ReadSize;
    }
    /* KM $ */

    /* $ 11.09.2005 KM
       - Bug #1377
    */
    int DecodeTableNum=0;
    int ANSISearch=UseANSI;
    int UnicodeSearch=UseUnicode;
    int RealReadSize=ReadSize;

    // ������� � ������ ���������� ���������� ���������: OEM, ANSI � Unicode
    int UseInnerTables=true;

    /* $ 22.09.2003 KM
       ����� �� hex-�����
    */
    if (!SearchHex)
    {
      if (UseAllTables || UseUnicode)
      {
        // ��� ����� ��� �� ���� ���������� ��� � �������,
        // �������� ��������� ������ ������
        SaveReadSize=ReadSize;

        // � ����� �������� ��������� �����.
        memcpy(SaveBuf,Buf,ReadSize);

        /* $ 21.10.2000 SVS
           ������� ����������, � ��� ��� �������, �� ���� � FFFE-������
        */
        if(!IsFirst)
        {
          IsFirst=TRUE;
          if(*(WORD*)Buf == 0xFFFE) // The text contains the Unicode
             ReverseBOM=TRUE;       // byte-reversed byte-order mark
                                    // (Reverse BOM) 0xFFFE as its first character.
        }
        /* SVS $ */
      }
    }
    /* KM $ */

    while (1)
    {
      /* $ 22.09.2003 KM
         ����� �� hex-�����
      */
      if (!SearchHex)
      {
        if (UnicodeSearch)
        {
          char BOMBuf[sizeof(SaveBuf)];
          char *BufPtr=SaveBuf;

          if(ReverseBOM)
          {
            BufPtr=BOMBuf;

            for(int I=0; I < SaveReadSize; I+=2)
            {
              BOMBuf[I]=SaveBuf[I+1];
              BOMBuf[I+1]=SaveBuf[I];
            }
          }

          WideCharToMultiByte(CP_OEMCP,0,(LPCWSTR)BufPtr,SaveReadSize/2,Buf,sizeof(Buf),NULL,NULL);
          ReadSize=SaveReadSize/2;
        }
        else
        {
          // ���� ����� ��� �� ���� ����������, ����������� ����������� �����
          // � ��� ������ ��� ������ � ������������ ��������� ������ � ������ ���������
          if (UseAllTables)
          {
            ReadSize=SaveReadSize;
            memcpy(Buf,SaveBuf,ReadSize);
          }

          /* $ 20.09.2003 KM
             ������� ��������� ANSI �������
          */
          if (UseDecodeTable || DecodeTableNum>0 || ANSISearch)
          {
            // ��� ��������� �� ������ � ANSI ���������, ���������� ������� �������������
            if (ANSISearch)
              GetTable(&TableSet,TRUE,TableNum,UseUnicode);

            for (int I=0;I<ReadSize;I++)
              Buf[I]=TableSet.DecodeTable[Buf[I]];
          }
          /* KM $ */
        }
        if (!CmpCase)
          LocalUpperBuf(Buf,ReadSize);
      }
      /* KM $ */

      int CheckSize=ReadSize-Length+1;
      /* $ 30.07.2000 KM
         ��������� "Whole words" � ������
      */
      /* $ 26.05.2002 KM
          ���������� ������ � ������ �� ����� ������.
      */
      for (int I=0;I<CheckSize;I++)
      {
        int cmpResult;
        int locResultLeft=FALSE;
        int locResultRight=FALSE;

        /* $ 22.09.2003 KM
           ����� �� hex-�����
        */

        char *lpWordDiv = UnicodeToAnsi (Opt.strWordDiv);

        if (WholeWords && !SearchHex)
        {
          if (!FirstIteration)
          {
            if (IsSpace(Buf[I-1]) || IsEol(Buf[I-1]) ||
               (strchr(lpWordDiv,Buf[I-1])!=NULL))
              locResultLeft=TRUE;
          }
          else
          {
            FirstIteration=FALSE;
            locResultLeft=TRUE;
          }

          if (RealReadSize!=sizeof(Buf) && I+Length>=RealReadSize)
            locResultRight=TRUE;
          else
            if (I+Length<RealReadSize &&
               (IsSpace(Buf[I+Length]) || IsEol(Buf[I+Length]) ||
               (strchr(lpWordDiv,Buf[I+Length])!=NULL)))
              locResultRight=TRUE;
        }
        /* KM $ */
        else
        {
          locResultLeft=TRUE;
          locResultRight=TRUE;
        }

        xf_free (lpWordDiv);

        cmpResult=locResultLeft && locResultRight && CmpStr[0]==Buf[I] &&
          (Length==1 || CmpStr[1]==Buf[I+1] &&
          (Length==2 || memcmp(CmpStr+2,&Buf[I+2],Length-2)==0));

        if (cmpResult)
        {
          if (TimeRead)
            SetFileTime(FileHandle,NULL,&LastAccess,NULL);
          fclose(SrcFile);
          return(TRUE);
        }
      }
      /* KM $ */
      /* KM $ */
      /* $ 22.09.2003 KM
         ����� �� hex-�����
      */
      if (UseAllTables && !SearchHex)
      {
        if (!ANSISearch && !UnicodeSearch && UseInnerTables)
        {
          // ��� �� ����� � OEM ���������, ������ ������ � ANSI.
          ANSISearch=true;
        }
        else
        {
          if (!UnicodeSearch && UseInnerTables)
          {
            // �� ����� � ANSI ���������, ������ ������ � Unicode.
            UnicodeSearch=true;
            ANSISearch=false;
          }
          else
          {
            // ���������� ������� ��� ������, ������ �������� ������ � ����������������
            UseInnerTables=false;

            UnicodeSearch=false;
            ANSISearch=false;

            // �� ����� �� � OEM, �� � ANSI, �� � Unicode, ������ ����� ������
            // � ������������� ���������������� ����������.
            if (PrepareTable(&TableSet,DecodeTableNum,TRUE))
            {
              DecodeTableNum++;
              xstrncpy(CmpStr,FindStr,sizeof(CmpStr)-1);
              if (!CmpCase)
                LocalStrupr(CmpStr);
            }
            else
              break;
          }
        }
      }
      else
        break;
      /* KM $ */
    }
    /* KM $ */

    if (RealReadSize==sizeof(Buf))
    {
      /* $ 22.09.2003 KM
         ����� �� hex-�����
      */
      /* $ 30.07.2000 KM
         ��������� offset ��� ������ ������ ����� � ������ WordDiv
      */
      int NewPos;
      //��� ������ �� ���� �������� �� �� ���� ��� ����� ���������� ����� � � �������
      //����� �� �������� FileSize/sizeof(Buf)*(Length+1) ���� ����� �������
      //�� ���� ��� �� ������ �� ��� ������ �� ���� �������� � ������� �� �����
      //��������� ���� ���������� ������.
      if ((UseAllTables || UnicodeSearch) && !SearchHex)
        NewPos=ftell(SrcFile)-2*(Length+1);
      else
        NewPos=ftell(SrcFile)-(Length+1);
      fseek(SrcFile,Max(NewPos,0),SEEK_SET);
      /* KM $ */
      /* KM $ */
    }
  }
  if (TimeRead)
    SetFileTime(FileHandle,NULL,&LastAccess,NULL);
  fclose(SrcFile);
  return(FALSE);
}

void FindFiles::DoPreparePluginList(void* Param, string& strSaveDir)
{
  TRY {

    Sleep(200);
    *PluginSearchPath=0;
    Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
    /* $ 15.10.2001 VVM */
    HANDLE hPlugin=ArcList[FindFileArcIndex]->hPlugin;
    struct OpenPluginInfoW Info;
    CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
    strSaveDir = Info.CurDir;
    WaitForSingleObject(hPluginMutex,INFINITE);
    if (SearchMode==SEARCH_ROOT ||
        SearchMode==SEARCH_ALL ||
        SearchMode==SEARCH_ALL_BUTNETWORK ||
        SearchMode==SEARCH_INPATH)
      CtrlObject->Plugins.SetDirectory(hPlugin,L"\\",OPM_FIND);
    ReleaseMutex(hPluginMutex);
    RecurseLevel=0;
    ScanPluginTree(hPlugin,ArcList[FindFileArcIndex]->Flags);
    /* VVM $ */
    WaitForSingleObject(hPluginMutex,INFINITE);
    if (SearchMode==SEARCH_ROOT ||
        SearchMode==SEARCH_ALL ||
        SearchMode==SEARCH_ALL_BUTNETWORK ||
        SearchMode==SEARCH_INPATH)
      CtrlObject->Plugins.SetDirectory(hPlugin,strSaveDir,OPM_FIND);
    ReleaseMutex(hPluginMutex);
    while (!StopSearch && FindMessageReady)
      Sleep(10);
    if (Param==NULL)
    {
      statusCS.Enter();

      strFindMessage.Format(UMSG(MFindDone),FindFileCount,FindDirCount);
      FindMessageReady=TRUE;
      SearchDone=TRUE;

      statusCS.Leave();
    }
  }
  EXCEPT (xfilter((int)INVALID_HANDLE_VALUE,GetExceptionInformation(),NULL,1))
  {
    TerminateProcess( GetCurrentProcess(), 1);
  }
}
#if defined(__BORLANDC__)
#pragma warn -par
#endif
void _cdecl FindFiles::PreparePluginList(void *Param)
{
  string strSaveDir;

  DoPreparePluginList(Param, strSaveDir);
}
#if defined(__BORLANDC__)
#pragma warn +par
#endif

void FindFiles::ScanPluginTree(HANDLE hPlugin, DWORD Flags)
{
  PluginPanelItemW *PanelData=NULL;
  int ItemCount=0;

  WaitForSingleObject(hPluginMutex,INFINITE);
  if (StopSearch || !CtrlObject->Plugins.GetFindData(hPlugin,&PanelData,&ItemCount,OPM_FIND))
  {
    ReleaseMutex(hPluginMutex);
    return;
  }
  else
    ReleaseMutex(hPluginMutex);
  RecurseLevel++;

  /* $ 19.01.2003 KM
     ������� ���� ����������, ���-�� ���������� ����������
     �� ������ ��, ������� � ������.
  */
  /* $ 24.03.2002 KM
     ���������� � �������� �� ������ ����� ������ � ������,
     ���� ��� ����� �������� � ���� ����, � �� ������ ���
     OPIF_REALNAMES, � ��������� ������ � ����������� ������
     ���������� ���������� ���������� ������ ��������������
     ���������� ����� �� ������.
  */
//  if (PanelData && strlen(PointToName(PanelData->FindData.cFileName))>0)
//    far_qsort((void *)PanelData,ItemCount,sizeof(*PanelData),SortItems);
  /* KM $ */
  /* KM $ */

  if (SearchMode!=SEARCH_SELECTED || RecurseLevel!=1)
  {
    for (int I=0;I<ItemCount && !StopSearch;I++)
    {
      while (PauseSearch)
        Sleep(10);

      PluginPanelItemW *CurPanelItem=PanelData+I;
      string strCurName=CurPanelItem->FindData.lpwszFileName;
      string strFullName;
      if (wcscmp(strCurName,L".")==0 || TestParentFolderNameW(strCurName))
        continue;
//      char AddPath[2*NM];
      if (Flags & OPIF_REALNAMES)
      {
        strFullName = strCurName;
//        strcpy(AddPath,CurName);
//        *PointToName(AddPath)=0;
      }
      else
      {
        strFullName.SetData (PluginSearchPath, CP_OEMCP); //BUGBUG
        strFullName += strCurName;
//        strcpy(AddPath,PluginSearchPath);
      }

      /* $ 30.09.2003 KM
        ����������� ����� �� ���������� � ����������� ������
      */
      int IsFile;
      if (UseFilter)
        IsFile=Filter->FileInFilter(&CurPanelItem->FindData);
      else
        IsFile=TRUE;

      if (IsFile)
      {
        /* $ 14.06.2004 KM
          ��������� �������� ��� ��������� ���������
        */
        if (CurPanelItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
          statusCS.Enter();

          strFindMessage = strFullName;
          RemovePseudoBackSlashW(strFindMessage);
          FindMessageReady=TRUE;

          statusCS.Leave();

        }
        /* KM $ */

        if (IsFileIncluded(CurPanelItem,strCurName,CurPanelItem->FindData.dwFileAttributes))
          AddMenuRecord(strFullName,&CurPanelItem->FindData);

        if (SearchInArchives && (hPlugin != INVALID_HANDLE_VALUE) && (Flags & OPIF_REALNAMES))
          ArchiveSearch(strFullName);
      }
      /* KM $ */
    }
  }
  if (SearchMode!=SEARCH_CURRENT_ONLY)
  {
    for (int I=0;I<ItemCount && !StopSearch;I++)
    {
      PluginPanelItemW *CurPanelItem=PanelData+I;
      string strCurName=CurPanelItem->FindData.lpwszFileName;
      if ((CurPanelItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
          wcscmp(strCurName,L".")!=0 && !TestParentFolderNameW(strCurName) &&
          (SearchMode!=SEARCH_SELECTED || RecurseLevel!=1 ||
          CtrlObject->Cp()->ActivePanel->IsSelectedW(strCurName)))
      {
        WaitForSingleObject(hPluginMutex,INFINITE);

        if (wcschr(strCurName,L'\x1')==NULL && CtrlObject->Plugins.SetDirectory(hPlugin,strCurName,OPM_FIND))
        {
          ReleaseMutex(hPluginMutex);
          /* $ 19.01.2003 KM
             ���� ����� � ��� �������� �� ������������
             PluginSearchPath, �� ������ ������������ ������
             ����� � ���� � ����� ����������, ��� �� ���� ���.
          */
          int SearchPathLen=strlen(PluginSearchPath);
          int CurNameLen=strCurName.GetLength();
          if (SearchPathLen+CurNameLen<NM-2)
          {
              char szCurName[NM];

              UnicodeToAnsi(strCurName, szCurName, sizeof (szCurName)-1); //BUGBUG

            strcat(PluginSearchPath,szCurName);
            strcat(PluginSearchPath,"\x1");
            ScanPluginTree(hPlugin, Flags);
            char *Ptr=strrchr(PluginSearchPath,'\x1');
            if (Ptr!=NULL)
              *Ptr=0;
          }
          /* KM $ */
          char *NamePtr=strrchr(PluginSearchPath,'\x1');
          if (NamePtr!=NULL)
            *(NamePtr+1)=0;
          else
            *PluginSearchPath=0;
          WaitForSingleObject(hPluginMutex,INFINITE);
          if (!CtrlObject->Plugins.SetDirectory(hPlugin,L"..",OPM_FIND))
            StopSearch=TRUE;
          ReleaseMutex(hPluginMutex);
          if (StopSearch) break;
        }
        else
          ReleaseMutex(hPluginMutex);
      }
    }
  }
  CtrlObject->Plugins.FreeFindData(hPlugin,PanelData,ItemCount);
  RecurseLevel--;
}

void _cdecl FindFiles::WriteDialogData(void *Param)
{
  string strDataStr;

  WriteDataUsed=TRUE;

  while( true )
  {
    Dialog* Dlg=(Dialog*)hDlg;

    if( !Dlg )
      break;

    VMenu *ListBox=Dlg->Item[1]->ListPtr;

    if (ListBox && !PauseSearch && !ScreenSaverActive)
    {
      if (BreakMainThread)
        StopSearch=TRUE;

      if (FindCountReady)
      {
        statusCS.Enter ();

        strDataStr.Format (L" %s: %d ", UMSG(MFindFound),FindFileCount+FindDirCount);

        statusCS.Leave ();

        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,2,(long)(const wchar_t*)strDataStr);

        FindCountReady=FALSE;
      }

      if (FindMessageReady)
      {
        string strSearchStr;

        if ( !strFindStr.IsEmpty() )
        {
          string strTemp, strFStr;

          strFStr = strFindStr;

          TruncStrFromEndW(strFStr,10);

          strTemp.Format (L" \"%s\"", (const wchar_t*)strFStr);
          strSearchStr.Format (UMSG(MFindSearchingIn), (const wchar_t*)strTemp);
        }
        else
          strSearchStr.Format (UMSG(MFindSearchingIn), L"");

        int Wid1=strSearchStr.GetLength();
        int Wid2=DlgWidth-strSearchStr.GetLength()-1;

        if (SearchDone)
        {
          Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);

          strDataStr = UMSG(MFindCancel);
          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,9,(long)(const wchar_t*)strDataStr);

          statusCS.Enter ();
          strDataStr.Format (L"%-*.*s",DlgWidth,DlgWidth, (const wchar_t*)strFindMessage);
          statusCS.Leave ();

          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,3,(long)(const wchar_t*)strDataStr);

          strDataStr = L"";
          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,2,(long)(const wchar_t*)strDataStr);

          Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);

          SetFarTitleW(strFindMessage);
          StopSearch=TRUE;
        }
        else
        {
          statusCS.Enter ();

          TruncPathStrW(strFindMessage, Wid2);

          strDataStr.Format(L"%-*.*s %-*.*s",Wid1,Wid1, (const wchar_t*)strSearchStr,Wid2,Wid2,(const wchar_t*)strFindMessage);
          statusCS.Leave ();

          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,3,(long)(const wchar_t*)strDataStr);
        }

        FindMessageReady=FALSE;
      }

      if (LastFoundNumber && ListBox)
      {
        LastFoundNumber=0;

        if ( ListBox->UpdateRequired () )
          Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,1,1);
      }
    }

    if (StopSearch && SearchDone && !FindMessageReady && !FindCountReady && !LastFoundNumber)
        break;

    Sleep(20);
  }

  WriteDataUsed=FALSE;
}

BOOL FindFiles::FindListGrow()
{
  DWORD Delta = (FindListCapacity < 256)?LIST_DELTA:FindListCapacity/2;
  LPFINDLIST* NewList = (LPFINDLIST*)xf_realloc(FindList, (FindListCapacity + Delta) * 4);
  if (NewList)
  {
    FindList = NewList;
    FindListCapacity+= Delta;
    return(TRUE);
  }
  return(FALSE);
}

BOOL FindFiles::ArcListGrow()
{
  DWORD Delta = (ArcListCapacity < 256)?LIST_DELTA:ArcListCapacity/2;
  LPARCLIST* NewList = (LPARCLIST*)xf_realloc(ArcList, (ArcListCapacity + Delta) * 4);
  if (NewList)
  {
    ArcList = NewList;
    ArcListCapacity+= Delta;
    return(TRUE);
  }
  return(FALSE);
}

DWORD FindFiles::AddFindListItem(FAR_FIND_DATA_EX *FindData)
{
  if ((FindListCount == FindListCapacity) &&
      (!FindListGrow()))
    return(LIST_INDEX_NONE);

  FindList[FindListCount] = new _FINDLIST;

  FindList[FindListCount]->FindData = *FindData;
  FindList[FindListCount]->ArcIndex = LIST_INDEX_NONE;
  return(FindListCount++);
}

DWORD FindFiles::AddArcListItem(const wchar_t *ArcName, HANDLE hPlugin,
                                DWORD dwFlags, const wchar_t *RootPath)
{
  if ((ArcListCount == ArcListCapacity) &&
      (!ArcListGrow()))
    return(LIST_INDEX_NONE);

  ArcList[ArcListCount] = new _ARCLIST;

  ArcList[ArcListCount]->strArcName = NullToEmptyW(ArcName);
  ArcList[ArcListCount]->hPlugin = hPlugin;
  ArcList[ArcListCount]->Flags = dwFlags;
  ArcList[ArcListCount]->strRootPath =  NullToEmptyW(RootPath);
  AddEndSlashW(ArcList[ArcListCount]->strRootPath);
  return(ArcListCount++);
}

void FindFiles::ClearAllLists()
{
  CriticalSectionLock Lock(ffCS);

  FindFileArcIndex = LIST_INDEX_NONE;

  if (FindList)
  {
      for (unsigned int i = 0; i < FindListCount; i++)
          delete FindList[i];

    xf_free(FindList);
  }
  FindList = NULL;
  FindListCapacity = FindListCount = 0;

  if (ArcList)
  {
      for (unsigned int i = 0; i < ArcListCount; i++)
          delete ArcList[i];

    xf_free(ArcList);
  }
  ArcList = NULL;
  ArcListCapacity = ArcListCount = 0;
}

string &FindFiles::PrepareDriveNameStr(string &strSearchFromRoot, size_t sz)
{
  string strCurDir, strMsgStr, strMsgStr1;
  int MsgLen,DrvLen,MsgLenDiff;

  Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
  PluginMode=ActivePanel->GetMode()==PLUGIN_PANEL && ActivePanel->IsVisible();

  CtrlObject->CmdLine->GetCurDirW(strCurDir);

  GetPathRootOneW(strCurDir, strCurDir);

  wchar_t *CurDir = strCurDir.GetBuffer ();

  if (CurDir[wcslen(CurDir)-1]==L'\\')
    CurDir[wcslen(CurDir)-1]=0;

  strCurDir.ReleaseBuffer ();

  if (*CurDir==0 || PluginMode)
  {
    strSearchFromRoot = UMSG(MSearchFromRootFolder);
    strMsgStr1 = strSearchFromRoot;

    RemoveHighlightsW(strMsgStr1);
    MsgLen=strMsgStr1.GetLength();

    // ������� � ����� ����� � '&' � ���. ����� ��� �����������
    // ����� ������ ����� ������, ��� ����� '&'
    MsgLenDiff=strSearchFromRoot.GetLength()-MsgLen;
  }
  else
  {
    strMsgStr = UMSG(MSearchFromRootOfDrive);
    strMsgStr1 = strMsgStr;

    RemoveHighlightsW(strMsgStr1);
    MsgLen=strMsgStr1.GetLength();

    // ������� � ����� ����� � '&' � ���. ����� ��� �����������
    // ����� ������ ����� ������, ��� ����� '&'
    MsgLenDiff=strMsgStr.GetLength()-MsgLen;

    DrvLen=sz-MsgLen-1-MsgLenDiff; // -1 - ��� ������ ����� ������� � ������, -MsgLenDiff - ��� ���� ������� '&'
    if (DrvLen<7)
    {
      DrvLen=7; // ������� ����������� ������ ����� ����� 7 ��������
                // (���� ������ TruncPathStr � UNC, ����� ���� ���-�� ���� �����)
      MsgLen=sz-DrvLen-1-MsgLenDiff;
    }

    TruncStrFromEndW(strMsgStr,MsgLen+MsgLenDiff);
    TruncPathStrW(strCurDir,DrvLen);

    strSearchFromRoot.Format (L"%s %s", (const wchar_t*)strMsgStr, (const wchar_t*)strCurDir);
  }

  //SearchFromRoot[sz-1+MsgLenDiff]=0; //BUGBUG

  return strSearchFromRoot;
}

char *FindFiles::RemovePseudoBackSlash(char *FileName)
{
  for (int i=0;FileName[i]!=0;i++)
  {
    if (FileName[i]=='\x1')
      FileName[i]='\\';
  }
  return FileName;
}

wchar_t *FindFiles::RemovePseudoBackSlashW(wchar_t *FileName)
{
  for (int i=0;FileName[i]!=0;i++)
  {
    if (FileName[i]==L'\x1')
      FileName[i]=L'\\';
  }
  return FileName;
}

string &FindFiles::RemovePseudoBackSlashW(string &strFileName)
{
    wchar_t *lpwszFileName = strFileName.GetBuffer ();

    RemovePseudoBackSlashW (lpwszFileName);

    strFileName.ReleaseBuffer ();

    return strFileName;
}


__int64 __fastcall FindFiles::GetSearchInFirstW (const wchar_t *DigitStr)
{
    __int64 LocalSize=_i64(0);

  if (*DigitStr)
  {
    switch(SearchInFirstIndex)
    {
      case FSIZE_INBYTES:
        LocalSize=_wtoi64(DigitStr);
        break;
      case FSIZE_INKBYTES:
        // ������ ����� � ����������, �������� ��� � �����.
        LocalSize=_wtoi64(DigitStr)*_i64(1024);
        break;
      case FSIZE_INMBYTES:
        // ������ ����� � ����������, �������� ��� � �����.
        LocalSize=_wtoi64(DigitStr)*_i64(1024)*_i64(1024);
        break;
      case FSIZE_INGBYTES:
        // ������ ����� � ����������, �������� ��� � �����.
        LocalSize=_wtoi64(DigitStr)*_i64(1024)*_i64(1024)*_i64(1024);
        break;
      default:
        break;
    }
  }
  else
  {
    LocalSize=_i64(0);
  }

  return LocalSize;
}

/* KM $ */
