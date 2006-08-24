/*
setattr.cpp

��������� ��������� ������

*/

/* Revision: 1.82 25.08.2006 $ */

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
#include "constitle.hpp"


#define DM_SETATTR      (DM_USER+1)

enum {
  SETATTR_TITLE=0,

  SETATTR_NAME=2,

  SETATTR_RO=4,
  SETATTR_ARCHIVE=5,
  SETATTR_HIDDEN=6,
  SETATTR_SYSTEM=7,
  SETATTR_COMPRESSED=8,
  SETATTR_ENCRYPTED=9,
  SETATTR_INDEXED=10,
  SETATTR_SPARSE=11,
  SETATTR_TEMP=12,

  SETATTR_SUBFOLDERS=14,

  SETATTR_TITLEDATE=16,
  SETATTR_MODIFICATION=17,
  SETATTR_MDATE=18,
  SETATTR_MTIME=19,
  SETATTR_CREATION=20,
  SETATTR_CDATE=21,
  SETATTR_CTIME=22,
  SETATTR_LASTACCESS=23,
  SETATTR_ADATE=24,
  SETATTR_ATIME=25,
  SETATTR_ORIGINAL=26,
  SETATTR_CURRENT=27,
  SETATTR_BLANK=28,

  SETATTR_SET=30,
  SETATTR_CANCEL=31,
  SETATTR_TITLELINK=32,
};

const wchar_t FmtMask1[]=L"99%c99%c99";
const wchar_t FmtMask2[]=L"99%c99%c9999";
const wchar_t FmtMask3[]=L"9999%c99%c99";

struct SetAttrDlgParam{
  BOOL  Plugin;
  DWORD FileSystemFlags;
  int ModeDialog;
  int DateFormat;
  string strSelName;
  int OriginalCBAttr[16]; // �������� CheckBox`�� �� ������ ������ �������
  int OriginalCBAttr2[16]; //
  DWORD OriginalCBFlag[16];
  int OState12, OState8, OState9;
  int OLastWriteTime,OCreationTime,OLastAccessTime;
  string strFSysName;
};

static int IsFileWritable(const wchar_t *Name, DWORD FileAttr, BOOL IsShowErrMsg, int Msg);
static int ReadFileTime(int Type,const wchar_t *Name,DWORD FileAttr,FILETIME *FileTime,
                       const wchar_t *OSrcDate, const wchar_t *OSrcTime);
static void PR_ShellSetFileAttributesMsg(void);
void ShellSetFileAttributesMsg(const wchar_t *Name);

// � Dst �������� ����� ��� ���� �������
static void GetFileDateAndTime(const wchar_t *Src,unsigned *Dst,int Separator)
{
  string strDigit;
  const wchar_t *PtrDigit;
  int I;

  Dst[0]=Dst[1]=Dst[2]=(unsigned)-1;
  I=0;
  const wchar_t *Ptr=Src;
  while((Ptr=GetCommaWordW(Ptr,strDigit,Separator)) != NULL)
  {
    PtrDigit=strDigit;
    while (*PtrDigit && !iswdigit(*PtrDigit))
      PtrDigit++;
    if(*PtrDigit)
      Dst[I]=_wtoi(PtrDigit);
    ++I;
  }
}

// ���������� ������� - ���� ��� ����� ������� ������ ������.
long WINAPI SetAttrDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  int FocusPos,I;
  int State8, State9, State12;
  struct SetAttrDlgParam *DlgParam;

  DlgParam=(struct SetAttrDlgParam *)Dialog::SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  switch(Msg)
  {
    case DN_BTNCLICK:
      if(Param1 >= SETATTR_RO && Param1 <= SETATTR_TEMP || Param1 == SETATTR_SUBFOLDERS)
      {
        DlgParam->OriginalCBAttr[Param1-SETATTR_RO] = Param2;
        DlgParam->OriginalCBAttr2[Param1-SETATTR_RO] = 0;

        FocusPos=Dialog::SendDlgMessage(hDlg,DM_GETFOCUS,0,0);
        State8=Dialog::SendDlgMessage(hDlg,DM_GETCHECK,SETATTR_COMPRESSED,0);
        State9=Dialog::SendDlgMessage(hDlg,DM_GETCHECK,SETATTR_ENCRYPTED,0);
        State12=Dialog::SendDlgMessage(hDlg,DM_GETCHECK,SETATTR_SUBFOLDERS,0);

        if(!DlgParam->ModeDialog) // =0 - ���������
        {
          if(((DlgParam->FileSystemFlags & (FS_FILE_COMPRESSION|FS_FILE_ENCRYPTION))==
               (FS_FILE_COMPRESSION|FS_FILE_ENCRYPTION)) &&
             (FocusPos == SETATTR_COMPRESSED || FocusPos == SETATTR_ENCRYPTED))
          {
              if(FocusPos == SETATTR_COMPRESSED && /*State8 &&*/ State9)
                Dialog::SendDlgMessage(hDlg,DM_SETCHECK,SETATTR_ENCRYPTED,BSTATE_UNCHECKED);
              if(FocusPos == SETATTR_ENCRYPTED && /*State9 &&*/ State8)
                Dialog::SendDlgMessage(hDlg,DM_SETCHECK,SETATTR_COMPRESSED,BSTATE_UNCHECKED);
          }
        }
        else // =1|2 Multi
        {
          // ���������� ����������������
          if(((DlgParam->FileSystemFlags & (FS_FILE_COMPRESSION|FS_FILE_ENCRYPTION))==
               (FS_FILE_COMPRESSION|FS_FILE_ENCRYPTION)) &&
             (FocusPos == SETATTR_COMPRESSED || FocusPos == SETATTR_ENCRYPTED))
          {
            if(FocusPos == SETATTR_COMPRESSED && DlgParam->OState8 != State8) // ��������� ����������?
            {
              if(State8 == BSTATE_CHECKED && State9)
                Dialog::SendDlgMessage(hDlg,DM_SETCHECK,SETATTR_ENCRYPTED,BSTATE_UNCHECKED);
              else if(State8 == BSTATE_3STATE)
                Dialog::SendDlgMessage(hDlg,DM_SETCHECK,SETATTR_ENCRYPTED,BSTATE_3STATE);
            }
            else if(FocusPos == SETATTR_ENCRYPTED && DlgParam->OState9 != State9) // ��������� ����������?
            {
              if(State9 == BSTATE_CHECKED && State8)
                Dialog::SendDlgMessage(hDlg,DM_SETCHECK,SETATTR_COMPRESSED,BSTATE_UNCHECKED);
              else if(State9 == 2)
                Dialog::SendDlgMessage(hDlg,DM_SETCHECK,SETATTR_COMPRESSED,BSTATE_3STATE);
            }

            // ��� ���� ��������
            if(FocusPos == SETATTR_COMPRESSED && /* DlgParam->OState8 && */ State9)
              Dialog::SendDlgMessage(hDlg,DM_SETCHECK,SETATTR_ENCRYPTED,BSTATE_UNCHECKED);
            if(FocusPos == SETATTR_ENCRYPTED && /* DlgParam->OState9 && */ State8)
              Dialog::SendDlgMessage(hDlg,DM_SETCHECK,SETATTR_COMPRESSED,BSTATE_UNCHECKED);

            DlgParam->OState9=State9;
            DlgParam->OState8=State8;
          }

          // ���� ������� �������� ��� SubFolders
          // ���� ����� ������ �������� ���� ���� ���� �� ���� �����
          // ����� 12-� ���������� � ������ ����.
          if(FocusPos == SETATTR_SUBFOLDERS)
          {
            if(DlgParam->ModeDialog==1) // ������� ����������!
            {
              if(DlgParam->OState12 != State12) // ��������� ����������?
              {
                // ������� 3-State
                for(I=SETATTR_RO; I <= SETATTR_TEMP; ++I)
                {
                  if(!State12) // �����?
                  {
                    Dialog::SendDlgMessage(hDlg,DM_SET3STATE,I,FALSE);
                    Dialog::SendDlgMessage(hDlg,DM_SETCHECK,I,DlgParam->OriginalCBAttr[I-SETATTR_RO]);
                  }
                  else                      // ����������?
                  {
                    Dialog::SendDlgMessage(hDlg,DM_SET3STATE,I,TRUE);
                    if(DlgParam->OriginalCBAttr2[I-SETATTR_RO] == -1)
                      Dialog::SendDlgMessage(hDlg,DM_SETCHECK,I,BSTATE_3STATE);
                  }
                }
                if(Opt.SetAttrFolderRules)
                {
                  FAR_FIND_DATA_EX FindData;

                  if ( apiGetFindDataEx (DlgParam->strSelName,&FindData) )
                  {
                    if(!State12)
                    {
                      if(!DlgParam->OLastWriteTime)
                      {
                        Dialog::SendDlgMessage(hDlg,DM_SETATTR,SETATTR_MODIFICATION,(DWORD)&FindData.ftLastWriteTime);
                        DlgParam->OLastWriteTime=0;
                      }
                      if(!DlgParam->OCreationTime)
                      {
                        Dialog::SendDlgMessage(hDlg,DM_SETATTR,SETATTR_CREATION,(DWORD)&FindData.ftCreationTime);
                        DlgParam->OCreationTime=0;
                      }
                      if(!DlgParam->OLastAccessTime)
                      {
                        Dialog::SendDlgMessage(hDlg,DM_SETATTR,SETATTR_LASTACCESS,(DWORD)&FindData.ftLastAccessTime);
                        DlgParam->OLastAccessTime=0;
                      }
                    }
                    else
                    {
                      if(!DlgParam->OLastWriteTime)
                      {
                        Dialog::SendDlgMessage(hDlg,DM_SETATTR,SETATTR_MODIFICATION,0);
                        DlgParam->OLastWriteTime=0;
                      }
                      if(!DlgParam->OCreationTime)
                      {
                        Dialog::SendDlgMessage(hDlg,DM_SETATTR,SETATTR_CREATION,0);
                        DlgParam->OCreationTime=0;
                      }
                      if(!DlgParam->OLastAccessTime)
                      {
                        Dialog::SendDlgMessage(hDlg,DM_SETATTR,SETATTR_LASTACCESS,0);
                        DlgParam->OLastAccessTime=0;
                      }
                    }
                  }

                }
              }
            }
            else  // ����� ��������
            {
              if(DlgParam->OState12 != State12) // ��������� ����������?
              {
                for(I=SETATTR_RO; I <= SETATTR_TEMP; ++I)
                {
                  if(!State12) // �����?
                  {
                    Dialog::SendDlgMessage(hDlg,DM_SET3STATE,I,
                              ((DlgParam->OriginalCBFlag[I-SETATTR_RO]&DIF_3STATE)?TRUE:FALSE));
                    Dialog::SendDlgMessage(hDlg,DM_SETCHECK,I,DlgParam->OriginalCBAttr[I-SETATTR_RO]);
                  }
                  else                      // ����������?
                  {
                    if(DlgParam->OriginalCBAttr2[I-SETATTR_RO] == -1)
                    {
                      Dialog::SendDlgMessage(hDlg,DM_SET3STATE,I,TRUE);
                      Dialog::SendDlgMessage(hDlg,DM_SETCHECK,I,BSTATE_3STATE);
                    }
                  }
                }
              }
            }
            DlgParam->OState12=State12;
          }
        }
        return TRUE;
      }
      // Set Original? / Set All? / Clear All?
      else if(Param1 == SETATTR_ORIGINAL)
      {
        FAR_FIND_DATA_EX FindData;
        DlgParam=(struct SetAttrDlgParam *)Dialog::SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
        if ( apiGetFindDataEx (DlgParam->strSelName,&FindData) )
        {
          Dialog::SendDlgMessage(hDlg,DM_SETATTR,SETATTR_MODIFICATION,(DWORD)&FindData.ftLastWriteTime);
          Dialog::SendDlgMessage(hDlg,DM_SETATTR,SETATTR_CREATION,(DWORD)&FindData.ftCreationTime);
          Dialog::SendDlgMessage(hDlg,DM_SETATTR,SETATTR_LASTACCESS,(DWORD)&FindData.ftLastAccessTime);
          DlgParam->OLastWriteTime=DlgParam->OCreationTime=DlgParam->OLastAccessTime=0;
        }
        Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,SETATTR_MDATE,0);
        return TRUE;
      }
      else if(Param1 == SETATTR_CURRENT || Param1 == SETATTR_BLANK)
      {
        Dialog::SendDlgMessage(hDlg,DM_SETATTR,SETATTR_MODIFICATION,Param1 == SETATTR_CURRENT?-1:0);
        Dialog::SendDlgMessage(hDlg,DM_SETATTR,SETATTR_CREATION,Param1 == SETATTR_CURRENT?-1:0);
        Dialog::SendDlgMessage(hDlg,DM_SETATTR,SETATTR_LASTACCESS,Param1 == SETATTR_CURRENT?-1:0);
        DlgParam->OLastWriteTime=DlgParam->OCreationTime=DlgParam->OLastAccessTime=1;
        Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,SETATTR_MDATE,0);
        return TRUE;
      }
      break;

    case DN_MOUSECLICK:
     {
       //_SVS(SysLog("Msg=DN_MOUSECLICK Param1=%d Param2=%d",Param1,Param2));
       if(Param1 >= SETATTR_MODIFICATION && Param1 <= SETATTR_ATIME)
       {
         if(((MOUSE_EVENT_RECORD*)Param2)->dwEventFlags == DOUBLE_CLICK)
         {
           // ����� ��������� �������� "��������"
           Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
           Dialog::SendDlgMessage(hDlg,DM_SETATTR,Param1,-1);
         }
         if(Param1 == SETATTR_MODIFICATION || Param1 == SETATTR_CREATION || Param1 == SETATTR_LASTACCESS)
           Param1++;
         Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,Param1,0);
         return TRUE;
       }
     }
     break;

    case DN_EDITCHANGE:
    {
      if(Param1 >= SETATTR_MDATE && Param1 <= SETATTR_ATIME)
      {
             if(Param1 == SETATTR_MDATE || Param1 == SETATTR_MTIME) { DlgParam->OLastWriteTime=1;}
        else if(Param1 == SETATTR_CDATE || Param1 == SETATTR_CTIME) { DlgParam->OCreationTime=1;}
        else if(Param1 == SETATTR_ADATE || Param1 == SETATTR_ATIME) { DlgParam->OLastAccessTime=1;}
      }
      break;
    }

    case DM_SETATTR:
      {
        FILETIME ft;
        string strDate, strTime;
        int Set1, Set2;
        if(Param2) // Set?
        {
          if(Param2==-1)
            GetSystemTimeAsFileTime(&ft);
          else
            ft=*(FILETIME *)Param2;
          ConvertDateW(ft,strDate,strTime,8,FALSE,FALSE,TRUE,TRUE);
        }
        else if(!Param2) // Clear
        {
            strDate=strTime=L"";
        }

        // ������ �� �����, ��� ��� ����
             if(Param1 == SETATTR_MODIFICATION) { Set1=SETATTR_MDATE; Set2=SETATTR_MTIME; DlgParam->OLastWriteTime=1;}
        else if(Param1 == SETATTR_CREATION) { Set1=SETATTR_CDATE; Set2=SETATTR_CTIME; DlgParam->OCreationTime=1;}
        else if(Param1 == SETATTR_LASTACCESS) { Set1=SETATTR_ADATE; Set2=SETATTR_ATIME; DlgParam->OLastAccessTime=1;}
        else if(Param1 == SETATTR_MDATE || Param1 == SETATTR_CDATE || Param1 == SETATTR_ADATE) { Set1=Param1; Set2=-1; }
        else { Set1=-1; Set2=Param1; }

        if(Set1 != -1)
          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,Set1,(long)(const wchar_t*)strDate);
        if(Set2 != -1)
          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,Set2,(long)(const wchar_t*)strTime);
        return TRUE;
      }
  }
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

static void PR_ShellSetFileAttributesMsg(void)
{
  ShellSetFileAttributesMsg((wchar_t *)PreRedrawParam.Param1);
}

void ShellSetFileAttributesMsg(const wchar_t *Name)
{
  static int Width=54;
  int WidthTemp;
  string strOutFileName;

  if(Name && *Name)
    WidthTemp=Max((int)wcslen(Name),(int)54);
  else
    Width=WidthTemp=54;

  if(WidthTemp > WidthNameForMessage)
    WidthTemp=WidthNameForMessage; // ������ ������ - 38%

  if(Width < WidthTemp)
    Width=WidthTemp;

  if(Name && *Name)
  {
    strOutFileName = Name;
    TruncPathStrW(strOutFileName,Width);
    CenterStrW(strOutFileName,strOutFileName,Width+4);
  }
  else
  {
    strOutFileName=L"";
    CenterStrW(strOutFileName,strOutFileName,Width+4); // �������������� ������ ������ (���!)
  }
  MessageW(0,0,UMSG(MSetAttrTitle),UMSG(MSetAttrSetting),(const wchar_t*)strOutFileName);
  PreRedrawParam.Param1=(void*)Name;
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
12   | [ ] Indexed                         |   12
13   +-------------------------------------+   13
14   | [x] Process subfolders              |   14
15   +-------------------------------------+   15
16   |  File time      DD.MM.YYYY hh:mm:ss |   16
17   | Modification      .  .       :  :   |   17
18   | Creation          .  .       :  :   |   18
19   | Last access       .  .       :  :   |   19
20   | [ Original ]  [ Current ] [ Blank ] |   10
21   +-------------------------------------+   21
22   |         [ Set ]  [ Cancel ]         |   22
23   +-------------------------------------+   23
24                                             24
*/
  static struct DialogDataEx AttrDlgData[]={
  /* 00 */DI_DOUBLEBOX,3,1,65,20,0,0,0,0,(wchar_t *)MSetAttrTitle,
  /* 01 */DI_TEXT,-1,2,0,0,0,0,0,0,(wchar_t *)MSetAttrFor,
  /* 02 */DI_TEXT,-1,3,0,0,0,0,DIF_SHOWAMPERSAND,0,L"",
  /* 03 */DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 04 */DI_CHECKBOX,5, 5,0,0,1,0,DIF_3STATE,0,(wchar_t *)MSetAttrRO,
  /* 05 */DI_CHECKBOX,5, 6,0,0,0,0,DIF_3STATE,0,(wchar_t *)MSetAttrArchive,
  /* 06 */DI_CHECKBOX,5, 7,0,0,0,0,DIF_3STATE,0,(wchar_t *)MSetAttrHidden,
  /* 07 */DI_CHECKBOX,5, 8,0,0,0,0,DIF_3STATE,0,(wchar_t *)MSetAttrSystem,
  /* 08 */DI_CHECKBOX,5, 9,0,0,0,0,DIF_3STATE,0,(wchar_t *)MSetAttrCompressed,
  /* 09 */DI_CHECKBOX,35, 5,0,0,0,0,DIF_3STATE,0,(wchar_t *)MSetAttrEncrypted,
  /* 10 */DI_CHECKBOX,35, 6,0,0,0,0,DIF_3STATE|DIF_DISABLE,0,(wchar_t *)MSetAttrNotIndexed,
  /* 11 */DI_CHECKBOX,35, 7,0,0,0,0,DIF_3STATE|DIF_DISABLE,0,(wchar_t *)MSetAttrSparse,
  /* 12 */DI_CHECKBOX,35, 8,0,0,0,0,DIF_3STATE|DIF_DISABLE,0,(wchar_t *)MSetAttrTemp,
  /* 13 */DI_TEXT,3,10,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 14 */DI_CHECKBOX,5,11,0,0,0,0,DIF_DISABLE,0,(wchar_t *)MSetAttrSubfolders,
  /* 15 */DI_TEXT,3,12,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 16 */DI_TEXT,45,13,0,0,0,0,0,0,L"",
  /* 17 */DI_TEXT,    5,14,0,0,0,0,0,0,(wchar_t *)MSetAttrModification,
  /* 18 */DI_FIXEDIT,45,14,54,14,0,0,DIF_MASKEDIT,0,L"",
  /* 19 */DI_FIXEDIT,56,14,63,14,0,0,DIF_MASKEDIT,0,L"",
  /* 20 */DI_TEXT,    5,15,0,0,0,0,0,0,(wchar_t *)MSetAttrCreation,
  /* 21 */DI_FIXEDIT,45,15,54,15,0,0,DIF_MASKEDIT,0,L"",
  /* 22 */DI_FIXEDIT,56,15,63,15,0,0,DIF_MASKEDIT,0,L"",
  /* 23 */DI_TEXT,    5,16,0,0,0,0,0,0,(wchar_t *)MSetAttrLastAccess,
  /* 24 */DI_FIXEDIT,45,16,54,16,0,0,DIF_MASKEDIT,0,L"",
  /* 25 */DI_FIXEDIT,56,16,63,16,0,0,DIF_MASKEDIT,0,L"",
  /* 26 */DI_BUTTON,0,17,0,0,0,0,DIF_CENTERGROUP|DIF_BTNNOCLOSE,0,(wchar_t *)MSetAttrOriginal,
  /* 27 */DI_BUTTON,0,17,0,0,0,0,DIF_CENTERGROUP|DIF_BTNNOCLOSE,0,(wchar_t *)MSetAttrCurrent,
  /* 28 */DI_BUTTON,0,17,0,0,0,0,DIF_CENTERGROUP|DIF_BTNNOCLOSE,0,(wchar_t *)MSetAttrBlank,
  /* 29 */DI_TEXT,3,18,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 30 */DI_BUTTON,0,19,0,0,0,0,DIF_CENTERGROUP,1,(wchar_t *)MSetAttrSet,
  /* 31 */DI_BUTTON,0,19,0,0,0,0,DIF_CENTERGROUP,0,(wchar_t *)MCancel,
  /* 32 */DI_TEXT,-1,4,0,0,0,0,DIF_SHOWAMPERSAND,0,L"",
  };
  MakeDialogItemsEx(AttrDlgData,AttrDlg);
  int DlgCountItems=sizeof(AttrDlgData)/sizeof(AttrDlgData[0])-1;

  int SelCount, I, J;
  struct SetAttrDlgParam DlgParam;

  if((SelCount=SrcPanel->GetSelCount())==0)
    return 0;

  memset(&DlgParam,0,sizeof(DlgParam));

  if (SrcPanel->GetMode()==PLUGIN_PANEL)
  {
    struct OpenPluginInfoW Info;
    HANDLE hPlugin=SrcPanel->GetPluginHandle();
    if(hPlugin == INVALID_HANDLE_VALUE)
      return 0;

    CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
    if(!(Info.Flags & OPIF_REALNAMES))
    {
      AttrDlg[SETATTR_SET].Flags|=DIF_DISABLE;
      DlgParam.Plugin=TRUE;
    }
  }

  DlgParam.FileSystemFlags=0;

  if(DlgParam.Plugin)
  {
    AttrDlg[SETATTR_COMPRESSED].Flags|=DIF_DISABLE;
    AttrDlg[SETATTR_ENCRYPTED].Flags|=DIF_DISABLE;
  }
  else
  {
    string strFSysName;
    if (apiGetVolumeInformation(NULL,NULL,NULL,NULL,&DlgParam.FileSystemFlags,&strFSysName))
    {
      if (!(DlgParam.FileSystemFlags & FS_FILE_COMPRESSION))
        AttrDlg[SETATTR_COMPRESSED].Flags|=DIF_DISABLE;

      if (!IsCryptFileASupport || !(DlgParam.FileSystemFlags & FS_FILE_ENCRYPTION))
        AttrDlg[SETATTR_ENCRYPTED].Flags|=DIF_DISABLE;

      if(!LocalStricmpW(strFSysName,L"NTFS"))
        AttrDlg[SETATTR_INDEXED].Flags&=~DIF_DISABLE;
    }
    DlgParam.strFSysName=strFSysName;
  }

  {
    string strSelName;
    int FileAttr;
    FILETIME LastWriteTime,CreationTime,LastAccessTime;
    int SetWriteTime,SetCreationTime,SetLastAccessTime;
    int SetWriteTimeRetCode=TRUE;
    FAR_FIND_DATA_EX FindData;

    //SaveScreen SaveScr;

    SrcPanel->GetSelNameW(NULL,FileAttr);
    SrcPanel->GetSelNameW(&strSelName,FileAttr,NULL,&FindData);
    if (SelCount==0 || SelCount==1 && TestParentFolderNameW(strSelName))
      return 0;

//    int NewAttr;
    int FolderPresent=FALSE, JunctionPresent=FALSE;

    int DateSeparator=GetDateSeparator();
    int TimeSeparator=GetTimeSeparator();
    string strDMask, strTMask;

    strTMask.Format (FmtMask1,TimeSeparator,TimeSeparator);

    string strAttr;

    switch(DlgParam.DateFormat=GetDateFormat())
    {
      case 0:
        strAttr = AttrDlg[SETATTR_TITLEDATE].strData;
        strAttr.Format (UMSG(MSetAttrTimeTitle1),DateSeparator,DateSeparator,TimeSeparator,TimeSeparator);
        strDMask.Format (FmtMask2,DateSeparator,DateSeparator);
        break;
      case 1:
        strAttr = AttrDlg[SETATTR_TITLEDATE].strData;
        strAttr.Format (UMSG(MSetAttrTimeTitle2),DateSeparator,DateSeparator,TimeSeparator,TimeSeparator);
        strDMask.Format (FmtMask2,DateSeparator,DateSeparator);
        break;
      default:
        strAttr = AttrDlg[SETATTR_TITLEDATE].strData;
        strAttr.Format (UMSG(MSetAttrTimeTitle3),DateSeparator,DateSeparator,TimeSeparator,TimeSeparator);
        strDMask.Format (FmtMask3,DateSeparator,DateSeparator);
        break;
    }

    AttrDlg[SETATTR_MDATE].Mask=strDMask;
    AttrDlg[SETATTR_MTIME].Mask=strTMask;
    AttrDlg[SETATTR_CDATE].Mask=strDMask;
    AttrDlg[SETATTR_CTIME].Mask=strTMask;
    AttrDlg[SETATTR_ADATE].Mask=strDMask;
    AttrDlg[SETATTR_ATIME].Mask=strTMask;

    if (SelCount==1)
    {
      if((FileAttr & FA_DIREC))
      {
        if(strSelName.At(strSelName.GetLength()-1) != L'\\')
        {
          string strCopy = strSelName;
          AddEndSlashW(strCopy);
          FileAttr=GetFileAttributesW(strCopy);
        }
        //_SVS(SysLog("SelName=%s  FileAttr=0x%08X",SelName,FileAttr));
        AttrDlg[SETATTR_SUBFOLDERS].Flags&=~DIF_DISABLE;
        AttrDlg[SETATTR_SUBFOLDERS].Selected=Opt.SetAttrFolderRules == 1?0:1;
        if(Opt.SetAttrFolderRules)
        {
          if ( apiGetFindDataEx (strSelName,&FindData) )
          {
            ConvertDateW(FindData.ftLastWriteTime, AttrDlg[SETATTR_MDATE].strData,AttrDlg[SETATTR_MTIME].strData,8,FALSE,FALSE,TRUE,TRUE);
            ConvertDateW(FindData.ftCreationTime,  AttrDlg[SETATTR_CDATE].strData,AttrDlg[SETATTR_CTIME].strData,8,FALSE,FALSE,TRUE,TRUE);
            ConvertDateW(FindData.ftLastAccessTime,AttrDlg[SETATTR_ADATE].strData,AttrDlg[SETATTR_ATIME].strData,8,FALSE,FALSE,TRUE,TRUE);
          }
          AttrDlg[SETATTR_RO].Selected=(FileAttr & FA_RDONLY)!=0;
          AttrDlg[SETATTR_ARCHIVE].Selected=(FileAttr & FA_ARCH)!=0;
          AttrDlg[SETATTR_HIDDEN].Selected=(FileAttr & FA_HIDDEN)!=0;
          AttrDlg[SETATTR_SYSTEM].Selected=(FileAttr & FA_SYSTEM)!=0;
          AttrDlg[SETATTR_COMPRESSED].Selected=(FileAttr & FILE_ATTRIBUTE_COMPRESSED)!=0;
          AttrDlg[SETATTR_ENCRYPTED].Selected=(FileAttr & FILE_ATTRIBUTE_ENCRYPTED)!=0;
          AttrDlg[SETATTR_INDEXED].Selected=(FileAttr & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)!=0;
          AttrDlg[SETATTR_SPARSE].Selected=(FileAttr & FILE_ATTRIBUTE_SPARSE_FILE)!=0;
          AttrDlg[SETATTR_TEMP].Selected=(FileAttr & FILE_ATTRIBUTE_TEMPORARY)!=0;

          // ������� 3-State
          for(I=SETATTR_RO; I <= SETATTR_TEMP; ++I)
            AttrDlg[I].Flags&=~DIF_3STATE;
        }
        FolderPresent=TRUE;

        // ��������� ������, ���� ��� SymLink
        if(FileAttr&FILE_ATTRIBUTE_REPARSE_POINT)
        {
          string strJuncName;
          DWORD LenJunction=GetJunctionPointInfoW(strSelName, strJuncName);
          //"\??\D:\Junc\Src\" ��� "\\?\Volume{..."

          AttrDlg[SETATTR_TITLE].Y2++;
          for(I=3; I  < DlgCountItems; ++I)
          {
            AttrDlg[I].Y1++;
            if (AttrDlg[I].Y2)
              AttrDlg[I].Y2++;
          }
          DlgCountItems++;
          JunctionPresent=TRUE;

          int ID_Msg, Width;
          if(!wcsncmp((const wchar_t*)strJuncName+4,L"Volume{",7))
          {
            string strJuncRoot;
            GetPathRootOneW((const wchar_t*)strJuncName+4,strJuncRoot);

            wchar_t *lpwszJunc = strJuncName.GetBuffer (strJuncRoot.GetLength()+4);

            if(strJuncRoot.At(1) == L':')
              wcscpy(lpwszJunc+4,strJuncRoot);

            strJuncName.ReleaseBuffer ();

            ID_Msg=MSetAttrVolMount;
            Width=38;
          }
          else
          {
            ID_Msg=MSetAttrJunction;
            Width=52;
          }

          string strJuncTemp = (const wchar_t*)strJuncName+4;


          AttrDlg[SETATTR_TITLELINK].strData.Format (UMSG(ID_Msg),
                (LenJunction?
                   (const wchar_t *)TruncPathStrW(strJuncTemp,Width):
                   UMSG(MSetAttrUnknownJunction)));

          /* $ 11.09.2001 SVS
             ��������� �� ������ ������������ �������� ������� �������� ��
             NTFS.
          */
          DlgParam.FileSystemFlags=0;
          GetPathRootW(strSelName,strJuncName);
          if (apiGetVolumeInformation (strJuncName,NULL,NULL,NULL,&DlgParam.FileSystemFlags,NULL))
          {
            if (!(DlgParam.FileSystemFlags & FS_FILE_COMPRESSION))
              AttrDlg[SETATTR_COMPRESSED].Flags|=DIF_DISABLE;

            if (!IsCryptFileASupport || !(DlgParam.FileSystemFlags & FS_FILE_ENCRYPTION))
              AttrDlg[SETATTR_ENCRYPTED].Flags|=DIF_DISABLE;
          }
          /* SVS $ */
        }
      }
      else
      {
        // ������� 3-State
        for(I=SETATTR_RO; I <= SETATTR_TEMP; ++I)
          AttrDlg[I].Flags&=~DIF_3STATE;
      }

      AttrDlg[SETATTR_NAME].strData = strSelName;
      TruncStrW(AttrDlg[SETATTR_NAME].strData,54);

      AttrDlg[SETATTR_RO].Selected=(FileAttr & FA_RDONLY)!=0;
      AttrDlg[SETATTR_ARCHIVE].Selected=(FileAttr & FA_ARCH)!=0;
      AttrDlg[SETATTR_HIDDEN].Selected=(FileAttr & FA_HIDDEN)!=0;
      AttrDlg[SETATTR_SYSTEM].Selected=(FileAttr & FA_SYSTEM)!=0;
      AttrDlg[SETATTR_COMPRESSED].Selected=(FileAttr & FILE_ATTRIBUTE_COMPRESSED)!=0;
      AttrDlg[SETATTR_ENCRYPTED].Selected=(FileAttr & FILE_ATTRIBUTE_ENCRYPTED)!=0;
      AttrDlg[SETATTR_INDEXED].Selected=(FileAttr & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)!=0;
      AttrDlg[SETATTR_SPARSE].Selected=(FileAttr & FILE_ATTRIBUTE_SPARSE_FILE)!=0;
      AttrDlg[SETATTR_TEMP].Selected=(FileAttr & FILE_ATTRIBUTE_TEMPORARY)!=0;

      if(DlgParam.Plugin)
      {
        ConvertDateW(FindData.ftLastWriteTime,AttrDlg[SETATTR_MDATE].strData,AttrDlg[SETATTR_MTIME].strData,8,FALSE,FALSE,TRUE,TRUE);
        ConvertDateW(FindData.ftCreationTime,AttrDlg[SETATTR_CDATE].strData,AttrDlg[SETATTR_CTIME].strData,8,FALSE,FALSE,TRUE,TRUE);
        ConvertDateW(FindData.ftLastAccessTime,AttrDlg[SETATTR_ADATE].strData,AttrDlg[SETATTR_ATIME].strData,8,FALSE,FALSE,TRUE,TRUE);
      }
      else
      {
        if ( apiGetFindDataEx (strSelName,&FindData))
        {
          ConvertDateW(FindData.ftLastWriteTime,AttrDlg[SETATTR_MDATE].strData,AttrDlg[SETATTR_MTIME].strData,8,FALSE,FALSE,TRUE,TRUE);
          ConvertDateW(FindData.ftCreationTime,AttrDlg[SETATTR_CDATE].strData,AttrDlg[SETATTR_CTIME].strData,8,FALSE,FALSE,TRUE,TRUE);
          ConvertDateW(FindData.ftLastAccessTime,AttrDlg[SETATTR_ADATE].strData,AttrDlg[SETATTR_ATIME].strData,8,FALSE,FALSE,TRUE,TRUE);
        }
      }
    }
    else
    {
      AttrDlg[SETATTR_RO].Selected=AttrDlg[SETATTR_ARCHIVE].Selected=AttrDlg[SETATTR_HIDDEN].Selected=
      AttrDlg[SETATTR_SYSTEM].Selected=AttrDlg[SETATTR_COMPRESSED].Selected=AttrDlg[SETATTR_ENCRYPTED].Selected=
      AttrDlg[SETATTR_INDEXED].Selected=AttrDlg[SETATTR_SPARSE].Selected=AttrDlg[SETATTR_TEMP].Selected=2;
      AttrDlg[SETATTR_MDATE].strData=AttrDlg[SETATTR_MTIME].strData=AttrDlg[SETATTR_CDATE].strData=
      AttrDlg[SETATTR_CTIME].strData=AttrDlg[SETATTR_ADATE].strData=AttrDlg[SETATTR_ATIME].strData=L"";

      AttrDlg[SETATTR_ORIGINAL].Flags|=DIF_HIDDEN;
      AttrDlg[SETATTR_ORIGINAL].Flags&=~DIF_CENTERGROUP;

      AttrDlg[SETATTR_NAME].strData = UMSG(MSetAttrSelectedObjects);
      // �������� -1 - ����� ����� ���� ���� :-)
      for(I=SETATTR_RO; I <= SETATTR_TEMP; ++I)
        AttrDlg[I].Selected=0;

      // �������� - ���� �� ����� ���������� - ��������?
      // ��� �� �������� �� ��������
      J=0;
      SrcPanel->GetSelNameW(NULL,FileAttr);
      while (SrcPanel->GetSelNameW(&strSelName,FileAttr,NULL,&FindData))
      {
        if(!J && (FileAttr & FA_DIREC))
        {
          FolderPresent=TRUE;
          AttrDlg[SETATTR_SUBFOLDERS].Flags&=~DIF_DISABLE;
          J++;
        }
        AttrDlg[SETATTR_RO].Selected+=(FileAttr & FA_RDONLY)?1:0;
        AttrDlg[SETATTR_ARCHIVE].Selected+=(FileAttr & FA_ARCH)?1:0;
        AttrDlg[SETATTR_HIDDEN].Selected+=(FileAttr & FA_HIDDEN)?1:0;
        AttrDlg[SETATTR_SYSTEM].Selected+=(FileAttr & FA_SYSTEM)?1:0;
        AttrDlg[SETATTR_COMPRESSED].Selected+=(FileAttr & FILE_ATTRIBUTE_COMPRESSED)?1:0;
        AttrDlg[SETATTR_ENCRYPTED].Selected+=(FileAttr & FILE_ATTRIBUTE_ENCRYPTED)?1:0;
        AttrDlg[SETATTR_INDEXED].Selected+=(FileAttr & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)?1:0;
        AttrDlg[SETATTR_SPARSE].Selected+=(FileAttr & FILE_ATTRIBUTE_SPARSE_FILE)?1:0;
        AttrDlg[SETATTR_TEMP].Selected+=(FileAttr & FILE_ATTRIBUTE_TEMPORARY)?1:0;
      }
      SrcPanel->GetSelNameW(NULL,FileAttr);
      SrcPanel->GetSelNameW(&strSelName,FileAttr,NULL,&FindData);
      // �������� "�������������" ��� ��, ��� �����
      for(I=SETATTR_RO; I <= SETATTR_TEMP; ++I)
      {
        J=AttrDlg[I].Selected;
        // ������� 3-state, ���� "���� ��� ��� ��� ������"
        // �� ����������� ������, ���� ���� ������ ����� ��������
        if((!J || J >= SelCount) && !FolderPresent)
          AttrDlg[I].Flags&=~DIF_3STATE;

        AttrDlg[I].Selected=(J >= SelCount)?1:(!J?0:2);
      }
    }

    // ��������� ��� ��������� ��� � 1.65?
    if(FolderPresent && !Opt.SetAttrFolderRules)
    {
      AttrDlg[SETATTR_SUBFOLDERS].Selected=1;
      AttrDlg[SETATTR_MDATE].strData=AttrDlg[SETATTR_MTIME].strData=AttrDlg[SETATTR_CDATE].strData=
      AttrDlg[SETATTR_CTIME].strData=AttrDlg[SETATTR_ADATE].strData=AttrDlg[SETATTR_ATIME].strData=L"";
      for(I=SETATTR_RO; I <= SETATTR_TEMP; ++I)
      {
        AttrDlg[I].Selected=2;
        AttrDlg[I].Flags|=DIF_3STATE;
      }
    }

    // �������� ��������� ��������������.
    for(I=SETATTR_RO; I <= SETATTR_TEMP; ++I)
    {
      DlgParam.OriginalCBAttr[I-SETATTR_RO]=AttrDlg[I].Selected;
      DlgParam.OriginalCBAttr2[I-SETATTR_RO]=-1;
      DlgParam.OriginalCBFlag[I-SETATTR_RO]=AttrDlg[I].Flags;
    }

    DlgParam.ModeDialog=((SelCount==1 && (FileAttr & FA_DIREC)==0)?0:(SelCount==1?1:2));
    DlgParam.strSelName=strSelName;
    DlgParam.OState12=AttrDlg[SETATTR_SUBFOLDERS].Selected;
    DlgParam.OState8=AttrDlg[SETATTR_COMPRESSED].Selected;
    DlgParam.OState9=AttrDlg[SETATTR_ENCRYPTED].Selected;

    // <Dialog>
    {
      Dialog Dlg(AttrDlg,DlgCountItems,SetAttrDlgProc,(DWORD)&DlgParam);
      Dlg.SetHelp(L"FileAttrDlg");                 //  ^ - ��� ��������� ������!
      Dlg.SetPosition(-1,-1,69,JunctionPresent?23:22);
      Dlg.Process();
      if (Dlg.GetExitCode()!=SETATTR_SET)
        return 0;
    }
    // </Dialog>

    SetPreRedrawFunc(PR_ShellSetFileAttributesMsg);
    ShellSetFileAttributesMsg(SelCount==1?(const wchar_t*)strSelName:NULL);

    if (SelCount==1 && (FileAttr & FA_DIREC)==0)
    {
      if(IsFileWritable(strSelName,FileAttr,TRUE,MSetAttrCannotFor) == 1)
      {
        int NewAttr;
        NewAttr=FileAttr & FA_DIREC;
        if (AttrDlg[SETATTR_RO].Selected)         NewAttr|=FA_RDONLY;
        if (AttrDlg[SETATTR_ARCHIVE].Selected)    NewAttr|=FA_ARCH;
        if (AttrDlg[SETATTR_HIDDEN].Selected)     NewAttr|=FA_HIDDEN;
        if (AttrDlg[SETATTR_SYSTEM].Selected)     NewAttr|=FA_SYSTEM;
        if (AttrDlg[SETATTR_COMPRESSED].Selected) NewAttr|=FILE_ATTRIBUTE_COMPRESSED;
        if (AttrDlg[SETATTR_ENCRYPTED].Selected)  NewAttr|=FILE_ATTRIBUTE_ENCRYPTED;
        if (AttrDlg[SETATTR_INDEXED].Selected)    NewAttr|=FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;

        /*
        AY: �� � ����� ���������� �� ��������
        if(!(AttrDlg[SETATTR_SPARSE].Flags&DIF_DISABLE))
          if (AttrDlg[SETATTR_SPARSE].Selected)
            NewAttr|=FILE_ATTRIBUTE_SPARSE_FILE;
        if(!(AttrDlg[SETATTR_TEMP].Flags&DIF_DISABLE))
          if (AttrDlg[SETATTR_TEMP].Selected)
            NewAttr|=FILE_ATTRIBUTE_TEMPORARY;
        */

        SetWriteTime=     DlgParam.OLastWriteTime  && ReadFileTime(0,strSelName,FileAttr,&LastWriteTime,AttrDlg[SETATTR_MDATE].strData,AttrDlg[SETATTR_MTIME].strData);
        SetCreationTime=  DlgParam.OCreationTime   && ReadFileTime(1,strSelName,FileAttr,&CreationTime,AttrDlg[SETATTR_CDATE].strData,AttrDlg[SETATTR_CTIME].strData);
        SetLastAccessTime=DlgParam.OLastAccessTime && ReadFileTime(2,strSelName,FileAttr,&LastAccessTime,AttrDlg[SETATTR_ADATE].strData,AttrDlg[SETATTR_ATIME].strData);
  //_SVS(SysLog("\n\tSetWriteTime=%d\n\tSetCreationTime=%d\n\tSetLastAccessTime=%d",SetWriteTime,SetCreationTime,SetLastAccessTime));
        if(SetWriteTime || SetCreationTime || SetLastAccessTime)
          SetWriteTimeRetCode=ESetFileTimeW(strSelName,
                                           (SetWriteTime ? &LastWriteTime:NULL),
                                           (SetCreationTime ? &CreationTime:NULL),
                                           (SetLastAccessTime ? &LastAccessTime:NULL),
                                           FileAttr);
        else
          SetWriteTimeRetCode=TRUE;

  //      if(NewAttr != (FileAttr & (~FA_DIREC))) // ����� �� ���-���� ������???
        if(SetWriteTimeRetCode == 1) // ���� ����� ������� ���������...
        {
          if((NewAttr&FILE_ATTRIBUTE_COMPRESSED) && !(FileAttr&FILE_ATTRIBUTE_COMPRESSED))
            ESetFileCompressionW(strSelName,1,FileAttr);
          else if(!(NewAttr&FILE_ATTRIBUTE_COMPRESSED) && (FileAttr&FILE_ATTRIBUTE_COMPRESSED))
            ESetFileCompressionW(strSelName,0,FileAttr);

          if((NewAttr&FILE_ATTRIBUTE_ENCRYPTED) && !(FileAttr&FILE_ATTRIBUTE_ENCRYPTED))
            ESetFileEncryptionW(strSelName,1,FileAttr);
          else if(!(NewAttr&FILE_ATTRIBUTE_ENCRYPTED) && (FileAttr&FILE_ATTRIBUTE_ENCRYPTED))
            ESetFileEncryptionW(strSelName,0,FileAttr);

          ESetFileAttributesW(strSelName,NewAttr&(~(FILE_ATTRIBUTE_ENCRYPTED|FILE_ATTRIBUTE_COMPRESSED)));
        }
      }
    }

    /* Multi *********************************************************** */
    else
    {
      int RetCode=1;
      ConsoleTitle *SetAttrTitle= new ConsoleTitle(UMSG(MSetAttrTitle));
      int SetAttr,ClearAttr,Cancel=0;
      CtrlObject->Cp()->GetAnotherPanel(SrcPanel)->CloseFile();

      SetAttr=0;  ClearAttr=0;

      if (AttrDlg[SETATTR_RO].Selected == 1)         SetAttr|=FA_RDONLY;
      else if (!AttrDlg[SETATTR_RO].Selected)        ClearAttr|=FA_RDONLY;
      if (AttrDlg[SETATTR_ARCHIVE].Selected == 1)    SetAttr|=FA_ARCH;
      else if (!AttrDlg[SETATTR_ARCHIVE].Selected)   ClearAttr|=FA_ARCH;
      if (AttrDlg[SETATTR_HIDDEN].Selected == 1)     SetAttr|=FA_HIDDEN;
      else if (!AttrDlg[SETATTR_HIDDEN].Selected)    ClearAttr|=FA_HIDDEN;
      if (AttrDlg[SETATTR_SYSTEM].Selected == 1)     SetAttr|=FA_SYSTEM;
      else if (!AttrDlg[SETATTR_SYSTEM].Selected)    ClearAttr|=FA_SYSTEM;

      if (AttrDlg[SETATTR_COMPRESSED].Selected == 1)
      {
        SetAttr|=FILE_ATTRIBUTE_COMPRESSED;
        ClearAttr|=FILE_ATTRIBUTE_ENCRYPTED;
      }
      else if (!AttrDlg[SETATTR_COMPRESSED].Selected)
        ClearAttr|=FILE_ATTRIBUTE_COMPRESSED;

      if (AttrDlg[SETATTR_ENCRYPTED].Selected == 1)
      {
        SetAttr|=FILE_ATTRIBUTE_ENCRYPTED;
        ClearAttr|=FILE_ATTRIBUTE_COMPRESSED;
      }
      else if (!AttrDlg[SETATTR_ENCRYPTED].Selected)
        ClearAttr|=FILE_ATTRIBUTE_ENCRYPTED;

      if (AttrDlg[SETATTR_INDEXED].Selected == 1)        SetAttr|=FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
      else if (!AttrDlg[SETATTR_INDEXED].Selected)       ClearAttr|=FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;

      /*
      AY: �� � ����� ���������� �� ��������
      if(!(AttrDlg[SETATTR_SPARSE].Flags&DIF_DISABLE))
      {
        if (AttrDlg[SETATTR_SPARSE].Selected == 1)        SetAttr|=FILE_ATTRIBUTE_SPARSE_FILE;
        else if (!AttrDlg[SETATTR_SPARSE].Selected)       ClearAttr|=FILE_ATTRIBUTE_SPARSE_FILE;
      }

      if(!(AttrDlg[SETATTR_TEMP].Flags&DIF_DISABLE))
      {
        if (AttrDlg[SETATTR_TEMP].Selected == 1)        SetAttr|=FILE_ATTRIBUTE_TEMPORARY;
        else if (!AttrDlg[SETATTR_TEMP].Selected)       ClearAttr|=FILE_ATTRIBUTE_TEMPORARY;
      }
      */

      SrcPanel->GetSelNameW(NULL,FileAttr);

      while (SrcPanel->GetSelNameW(&strSelName,FileAttr,NULL,&FindData) && !Cancel)
      {
//_SVS(SysLog("SelName='%s'\n\tFileAttr =0x%08X\n\tSetAttr  =0x%08X\n\tClearAttr=0x%08X\n\tResult   =0x%08X",
//    SelName,FileAttr,SetAttr,ClearAttr,((FileAttr|SetAttr)&(~ClearAttr))));
        ShellSetFileAttributesMsg(strSelName);

        if (CheckForEsc())
          break;

        RetCode=IsFileWritable(strSelName,FileAttr,TRUE,MSetAttrCannotFor);
        if(!RetCode)
          break;
        if(RetCode == 2)
          continue;

        SetWriteTime=     DlgParam.OLastWriteTime  && ReadFileTime(0,strSelName,FileAttr,&LastWriteTime,AttrDlg[SETATTR_MDATE].strData,AttrDlg[SETATTR_MTIME].strData);
        SetCreationTime=  DlgParam.OCreationTime   && ReadFileTime(1,strSelName,FileAttr,&CreationTime,AttrDlg[SETATTR_CDATE].strData,AttrDlg[SETATTR_CTIME].strData);
        SetLastAccessTime=DlgParam.OLastAccessTime && ReadFileTime(2,strSelName,FileAttr,&LastAccessTime,AttrDlg[SETATTR_ADATE].strData,AttrDlg[SETATTR_ATIME].strData);
        if(!(FileAttr&FILE_ATTRIBUTE_REPARSE_POINT) && (SetWriteTime || SetCreationTime || SetLastAccessTime))
        {
          if(StrstriW(DlgParam.strFSysName,L"FAT") && (FileAttr&FA_DIREC))
            RetCode=1;
          else
            RetCode=ESetFileTimeW(strSelName,
                 (SetWriteTime ? &LastWriteTime:NULL),
                 (SetCreationTime ? &CreationTime:NULL),
                 (SetLastAccessTime ? &LastAccessTime:NULL),
                 FileAttr);
          if(!RetCode)
            break;
          if(RetCode == 2)
            continue;
        }
        if(((FileAttr|SetAttr)&(~ClearAttr)) != FileAttr)
        {
          if (AttrDlg[SETATTR_COMPRESSED].Selected != 2)
          {
            RetCode=ESetFileCompressionW(strSelName,AttrDlg[SETATTR_COMPRESSED].Selected,FileAttr);
            if(!RetCode) // ������� ����� :-(
              break;
            if(RetCode == 2)
              continue;
          }
          if (AttrDlg[SETATTR_ENCRYPTED].Selected != 2) // +E -C
          {
            if(AttrDlg[SETATTR_COMPRESSED].Selected != 1)
            {
              RetCode=ESetFileEncryptionW(strSelName,AttrDlg[SETATTR_ENCRYPTED].Selected,FileAttr);
              if(!RetCode) // ������� ����������� :-(
                break;
              if(RetCode == 2)
                continue;
            }
          }
          RetCode=ESetFileAttributesW(strSelName,((FileAttr|SetAttr)&(~ClearAttr)));
          if(!RetCode)
            break;
          if(RetCode == 2)
            continue;
        }

        if ((FileAttr & FA_DIREC) && AttrDlg[SETATTR_SUBFOLDERS].Selected)
        {
          string strFullName;
          ScanTree ScTree(FALSE);

          ScTree.SetFindPathW(strSelName,L"*.*");
          while (ScTree.GetNextNameW(&FindData,strFullName))
          {
            ShellSetFileAttributesMsg(strFullName);
            if (CheckForEsc())
            {
              Cancel=1;
              break;
            }

            RetCode=IsFileWritable(strFullName,FindData.dwFileAttributes,TRUE,MSetAttrCannotFor);
            if(!RetCode)
            {
              Cancel=1;
              break;
            }
            if(RetCode == 2)
              continue;

            SetWriteTime=     DlgParam.OLastWriteTime  && ReadFileTime(0,strFullName,FindData.dwFileAttributes,&LastWriteTime,AttrDlg[SETATTR_MDATE].strData,AttrDlg[SETATTR_MTIME].strData);
            SetCreationTime=  DlgParam.OCreationTime   && ReadFileTime(1,strFullName,FindData.dwFileAttributes,&CreationTime,AttrDlg[SETATTR_CDATE].strData,AttrDlg[SETATTR_CTIME].strData);
            SetLastAccessTime=DlgParam.OLastAccessTime && ReadFileTime(2,strFullName,FindData.dwFileAttributes,&LastAccessTime,AttrDlg[SETATTR_ADATE].strData,AttrDlg[SETATTR_ATIME].strData);
            if(!(FindData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT) && (SetWriteTime || SetCreationTime || SetLastAccessTime))
            {
              if(StrstriW(DlgParam.strFSysName,L"FAT") && (FileAttr&FA_DIREC))
                RetCode=1;
              else
                RetCode=ESetFileTimeW(strFullName,SetWriteTime ? &LastWriteTime:NULL,
                           SetCreationTime ? &CreationTime:NULL,
                           SetLastAccessTime ? &LastAccessTime:NULL,
                           FindData.dwFileAttributes);
              if(RetCode == 0)
              {
                Cancel=1;
                break;
              }
              if(RetCode == 2)
                continue;
            }
            if(((FindData.dwFileAttributes|SetAttr)&(~ClearAttr)) !=
                 FindData.dwFileAttributes)
            {
              if (AttrDlg[SETATTR_COMPRESSED].Selected != 2)
              {
                RetCode=ESetFileCompressionW(strFullName,AttrDlg[SETATTR_COMPRESSED].Selected,FindData.dwFileAttributes);
                if(RetCode == 0)
                {
                  Cancel=1;
                  break; // ������� ����� :-(
                }
                if(RetCode == 2)
                  continue;
              }
              if (AttrDlg[SETATTR_ENCRYPTED].Selected != 2) // +E -C
              {
                if(AttrDlg[SETATTR_COMPRESSED].Selected != 1)
                {
                  RetCode=ESetFileEncryptionW(strFullName,AttrDlg[SETATTR_ENCRYPTED].Selected,FindData.dwFileAttributes);
                  if (RetCode == 0)
                  {
                    Cancel=1;
                    break; // ������� ����������� :-(
                  }
                  if(RetCode == 2)
                    continue;
                }
              }
              RetCode=ESetFileAttributesW(strFullName,(FindData.dwFileAttributes|SetAttr)&(~ClearAttr));
              if (RetCode == 0)
              {
                Cancel=1;
                break;
              }
              if(RetCode == 2)
                continue;
            }
          }
        }
      } // END: while (SrcPanel->GetSelName(...))
      delete SetAttrTitle;
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

static int ReadFileTime(int Type,const wchar_t *Name,DWORD FileAttr,FILETIME *FileTime,
                        const wchar_t *OSrcDate, const wchar_t *OSrcTime)
{
  FILETIME ft, oft;
  SYSTEMTIME st, ost;
  unsigned DateN[3],TimeN[3];
  int DigitCount;
  int /*SetTime,*/GetTime;
  FILETIME *OriginalFileTime=0, OFTModify, OFTCreate, OFTLast;

  /*$ 17.07.2001 SKV
    �� ����� ��������, �������.
  */
  ZeroMemory(&st,sizeof(st));
  /* SKV$*/

  // ****** ��������� ���� ******** //
  GetFileDateAndTime(OSrcDate,DateN,GetDateSeparator());
  // ****** ��������� ������� ******** //
  GetFileDateAndTime(OSrcTime,TimeN,GetTimeSeparator());

  // ��������� ������ ������������
  if(DateN[0] == -1 || DateN[1] == -1 || DateN[2] == -1 ||
     TimeN[0] == -1 || TimeN[1] == -1 || TimeN[2] == -1)
  {
    // �������� ���� ��� ������������ ���� � ����� �����.
    HANDLE hFile=FAR_CreateFileW(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
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

    // ���������� � ��������� �����.
    FileTimeToLocalFileTime(OriginalFileTime,&oft);
    FileTimeToSystemTime(&oft,&ost);
    DigitCount=TRUE;
  }
  else
    DigitCount=FALSE;

  // "�������"
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

  if(TimeN[0]==(unsigned)-1 && TimeN[1]==(unsigned)-1 && TimeN[2]==(unsigned)-1)
  {
    st.wMilliseconds=ost.wMilliseconds;
    // ��� ������������ ����������� wDayOfWeek
    //SystemTimeToFileTime(&st,&ft);
    //FileTimeToSystemTime(&ft,&st);
  }

  // �������������� � "������������" ������
  SystemTimeToFileTime(&st,&ft);
  LocalFileTimeToFileTime(&ft,FileTime);
  if(DigitCount)
    return (CompareFileTime(FileTime,OriginalFileTime) == 0)?FALSE:TRUE;
  return TRUE;
}

// ���������� 0 - ������, 1 - ��, 2 - Skip
static int IsFileWritable(const wchar_t *Name, DWORD FileAttr, BOOL IsShowErrMsg, int Msg)
{
  if ((FileAttr & FA_DIREC) && WinVer.dwPlatformId!=VER_PLATFORM_WIN32_NT)
    return 1;

  while (1)
  {
    if (FileAttr & FA_RDONLY)
      SetFileAttributesW(Name,FileAttr & ~FA_RDONLY);

    HANDLE hFile=FAR_CreateFileW(Name,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,(FileAttr & FA_DIREC) ? FILE_FLAG_BACKUP_SEMANTICS:0,NULL);
    BOOL Writable=TRUE;
    if(hFile == INVALID_HANDLE_VALUE)
      Writable=FALSE;
    else
      CloseHandle(hFile);

    DWORD LastError=GetLastError();
    if (FileAttr & FA_RDONLY)
      SetFileAttributesW(Name,FileAttr);
    SetLastError(LastError);

    if (Writable)
      break;

    int Code;
    if(IsShowErrMsg)
        Code=MessageW(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,3,UMSG(MError),
                     UMSG(Msg),Name,
                     UMSG(MHRetry),UMSG(MHSkip),UMSG(MHCancel));
    else
       return 0;

    if (Code<0)
      return 0;
    if(Code == 1)
      return 2;
    if(Code == 2)
      return 0;
  }
  return 1;
}
