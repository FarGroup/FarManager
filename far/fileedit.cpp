/*
fileedit.cpp

Редактирование файла - надстройка над editor.cpp

*/

#include "headers.hpp"
#pragma hdrstop

#include "fileedit.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "macroopcode.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "poscache.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "dialog.hpp"
#include "fileview.hpp"
#include "help.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"
#include "namelist.hpp"
#include "history.hpp"
#include "cmdline.hpp"
#include "scrbuf.hpp"
#include "savescr.hpp"
#include "chgprior.hpp"
#include "filestr.hpp"




static HANDLE g_hDlg = NULL;
static int g_nID = 0;
static int g_nCodepage = 0;

BOOL __stdcall EnumCodePages (const wchar_t *lpwszCodePage)
{
    DWORD dwCP = _wtoi(lpwszCodePage);
    CPINFOEXW cpi;

    GetCPInfoExW (dwCP, 0, &cpi);

    if ( cpi.MaxCharSize == 1 )
    {
    	wchar_t *name = cpi.CodePageName;
    	wchar_t *p = wcschr(name, L'(');

    	if ( p )
    		name = p+1;

		p = wcsrchr(name, L')');

		if ( p )
			*p = 0;

    	FarListItemData data;

    	int index = Dialog::SendDlgMessage (g_hDlg, DM_LISTADDSTR, g_nID, (LONG_PTR)name);

    	data.Index = index;
    	data.DataSize = sizeof(dwCP);
    	data.Data = (void*)(DWORD_PTR)dwCP;

    	Dialog::SendDlgMessage (g_hDlg, DM_LISTSETDATA, g_nID, (LONG_PTR)&data);

    	if ( g_nCodepage == dwCP )
    	{
    		FarListPos pos;

    		pos.SelectPos = index;
    		pos.TopPos = -1;

    		Dialog::SendDlgMessage (g_hDlg, DM_LISTSETCURPOS, g_nID, (LONG_PTR)&pos);
    	}
	}

    return TRUE;
}


void AddCodepagesToList (HANDLE hDlg, int nID, int nCodepage, bool bAllowAuto = true)
{
	g_hDlg = hDlg;
	g_nID = nID;
	g_nCodepage = nCodepage;

  	FarListItem items[10];

  	memset(&items, 0, sizeof (items));

  	wcscpy(items[0].Text, L"Auto");

  	if ( nCodepage == CP_AUTODETECT ) //BUGBUG
	  	items[0].Flags |= LIF_SELECTED;

  	wcscpy(items[2].Text, L"OEM");

  	if ( nCodepage == CP_OEMCP ) //BUGBUG
	  	items[2].Flags |= LIF_SELECTED;

  	wcscpy(items[3].Text, L"ANSI");

  	if ( nCodepage == CP_ACP ) //BUGBUG
	  	items[3].Flags |= LIF_SELECTED;

  	wcscpy(items[5].Text, L"UTF-8");

  	if ( nCodepage == CP_UTF8 ) //BUGBUG
	  	items[5].Flags |= LIF_SELECTED;

  	wcscpy(items[6].Text, L"UTF-7");

  	if ( nCodepage == CP_UTF7 ) //BUGBUG
	  	items[6].Flags |= LIF_SELECTED;

  	wcscpy(items[7].Text, L"UNICODE");

  	if ( nCodepage == CP_UNICODE ) //BUGBUG
	  	items[7].Flags |= LIF_SELECTED;

  	wcscpy(items[8].Text, L"REVERSEBOM");

  	if ( nCodepage == CP_REVERSEBOM ) //BUGBUG
	  	items[8].Flags |= LIF_SELECTED;

	items[1].Flags = LIF_SEPARATOR;
  	items[4].Flags = LIF_SEPARATOR;
  	items[9].Flags = LIF_SEPARATOR;

  	FarList list;

  	list.Items = (FarListItem*)(bAllowAuto?&items[0]:&items[2]);
  	list.ItemsNumber = bAllowAuto?countof(items):countof(items)-2;

  	Dialog::SendDlgMessage (hDlg, DM_LISTADD, nID, (LONG_PTR)&list);

  	FarListItemData data;

  	int index = bAllowAuto?0:2;

  	//auto
  	data.Index = 0;
  	data.DataSize = 4;
  	data.Data = (void*)CP_AUTODETECT;

  	Dialog::SendDlgMessage (hDlg, DM_LISTSETDATA, nID, (LONG_PTR)&data);

  	//oem
  	data.Index = 2-index;
  	data.DataSize = 4;
  	data.Data = (void*)CP_OEMCP;

  	Dialog::SendDlgMessage (hDlg, DM_LISTSETDATA, nID, (LONG_PTR)&data);

  	//ansi
  	data.Index = 3-index;
  	data.DataSize = 4;
  	data.Data = (void*)CP_ACP;

  	Dialog::SendDlgMessage (hDlg, DM_LISTSETDATA, nID, (LONG_PTR)&data);


  	//utf-8
  	data.Index = 5-index;
  	data.DataSize = 4;
  	data.Data = (void*)CP_UTF8;

  	Dialog::SendDlgMessage (hDlg, DM_LISTSETDATA, nID, (LONG_PTR)&data);

  	//unicode
  	data.Index = 6-index;
  	data.DataSize = 4;
  	data.Data = (void*)CP_UNICODE;

  	Dialog::SendDlgMessage (hDlg, DM_LISTSETDATA, nID, (LONG_PTR)&data);

  	//reverse bom
  	data.Index = 7-index;
  	data.DataSize = 4;
  	data.Data = (void*)CP_REVERSEBOM;

  	Dialog::SendDlgMessage (hDlg, DM_LISTSETDATA, nID, (LONG_PTR)&data);

  	EnumSystemCodePagesW ((CODEPAGE_ENUMPROCW)EnumCodePages, CP_INSTALLED);
}


enum enumOpenEditor {
	ID_OE_TITLE,
	ID_OE_OPENFILETITLE,
	ID_OE_FILENAME,
	ID_OE_SEPARATOR1,
	ID_OE_CODEPAGETITLE,
	ID_OE_CODEPAGE,
	ID_OE_SEPARATOR2,
	ID_OE_OK,
	ID_OE_CANCEL,
	};


LONG_PTR __stdcall hndOpenEditor (
		HANDLE hDlg,
		int msg,
		int param1,
		LONG_PTR param2
		)
{
	if ( msg == DN_INITDIALOG )
	{
		int codepage = *(int*)Dialog::SendDlgMessage (hDlg, DM_GETDLGDATA, 0, 0);

		AddCodepagesToList (hDlg, ID_OE_CODEPAGE, codepage);
	}

	if ( msg == DN_CLOSE )
	{
		if ( param1 == ID_OE_OK )
		{
			int *param = (int*)Dialog::SendDlgMessage (hDlg, DM_GETDLGDATA, 0, 0);

			FarListPos pos;

			Dialog::SendDlgMessage (hDlg, DM_LISTGETCURPOS, ID_OE_CODEPAGE, (LONG_PTR)&pos);

			*param = Dialog::SendDlgMessage (hDlg, DM_LISTGETDATA, ID_OE_CODEPAGE, pos.SelectPos);

			return TRUE;
		}
	}

	return Dialog::DefDlgProc (hDlg, msg, param1, param2);
}




bool dlgOpenEditor (string &strFileName, int &codepage)
{
	const wchar_t *HistoryName=L"NewEdit";

	DialogDataEx EditDlgData[]=	{
		/* 00 */DI_DOUBLEBOX,3,1,72,8,0,0,0,0,(const wchar_t *)MEditTitle,
		/* 01 */DI_TEXT,     5,2, 0,2,0,0,0,0,(const wchar_t *)L"Open/create file:",
		/* 02 */DI_EDIT,     5,3,70,3,1,(DWORD_PTR)HistoryName,DIF_HISTORY,0,L"",
		/* 03 */DI_TEXT,     3,4, 0,4,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
		/* 04 */DI_TEXT,     5,5, 0,5,0,0,0,0,L"File codepage:",
		/* 05 */DI_COMBOBOX,25,5,70,5,0,0,DIF_DROPDOWNLIST,0,L"",
		/* 06 */DI_TEXT,     3,6, 0,6,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
		/* 07 */DI_BUTTON,   0,7, 0,7,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MOk,
		/* 08 */DI_BUTTON,   0,7, 0,7,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel,
	};

	MakeDialogItemsEx(EditDlgData,EditDlg);

	EditDlg[ID_OE_FILENAME].strData = strFileName;

	Dialog Dlg(EditDlg, countof(EditDlg), (FARWINDOWPROC)hndOpenEditor, (LONG_PTR)&codepage);

	Dlg.SetPosition(-1,-1,76,10);
	Dlg.SetHelp(L"FileSaveAs");

	Dlg.Process();

	if ( Dlg.GetExitCode() == ID_OE_OK )
	{
		strFileName = EditDlg[ID_OE_FILENAME].strData;
		return true;
	}

	return false;
}

enum enumSaveFileAs {
	ID_SF_TITLE,
	ID_SF_SAVEASFILETITLE,
	ID_SF_FILENAME,
	ID_SF_SEPARATOR1,
	ID_SF_CODEPAGETITLE,
	ID_SF_CODEPAGE,
	ID_SF_SEPARATOR2,
	ID_SF_SAVEASFORMATTITLE,
	ID_SF_DONOTCHANGE,
	ID_SF_DOS,
	ID_SF_UNIX,
	ID_SF_MAC,
	ID_SF_SEPARATOR3,
	ID_SF_OK,
	ID_SF_CANCEL,
	};

LONG_PTR __stdcall hndSaveFileAs (
		HANDLE hDlg,
		int msg,
		int param1,
		LONG_PTR param2
		)
{
	if ( msg == DN_INITDIALOG )
	{
		int codepage = *(int*)Dialog::SendDlgMessage (hDlg, DM_GETDLGDATA, 0, 0);
		AddCodepagesToList (hDlg, ID_SF_CODEPAGE, codepage, false);
	}

	if ( msg == DN_CLOSE )
	{
		if ( param1 == ID_SF_OK )
		{
			int *param = (int*)Dialog::SendDlgMessage (hDlg, DM_GETDLGDATA, 0, 0);

			FarListPos pos;

			Dialog::SendDlgMessage (hDlg, DM_LISTGETCURPOS, ID_SF_CODEPAGE, (LONG_PTR)&pos);

			*param = Dialog::SendDlgMessage (hDlg, DM_LISTGETDATA, ID_SF_CODEPAGE, pos.SelectPos);

			return TRUE;
		}
	}

	return Dialog::DefDlgProc (hDlg, msg, param1, param2);
}



bool dlgSaveFileAs (string &strFileName, int &TextFormat, int &codepage)
{
    const wchar_t *HistoryName=L"NewEdit";

    DialogDataEx EditDlgData[]=
    {
      /* 00 */ DI_DOUBLEBOX,3,1,72,14,0,0,0,0,(const wchar_t *)MEditTitle,
      /* 01 */ DI_TEXT,5,2,0,2,0,0,0,0,(const wchar_t *)MEditSaveAs,
      /* 02 */ DI_EDIT,5,3,70,3,1,(DWORD_PTR)HistoryName,DIF_HISTORY/*|DIF_EDITPATH*/,0,L"",
      /* 03 */ DI_TEXT,3,4,0,4,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
      /* 04 */ DI_TEXT,5,5,0,5,0,0,0,0,L"File codepage:",
      /* 05 */ DI_COMBOBOX,25,5,70,5,0,0,DIF_DROPDOWNLIST,0,L"",
      /* 06 */ DI_TEXT,3,6,0,6,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
      /* 07 */ DI_TEXT,5,7,0,7,0,0,0,0,(const wchar_t *)MEditSaveAsFormatTitle,
      /* 08 */ DI_RADIOBUTTON,5,8,0,8,0,0,DIF_GROUP,0,(const wchar_t *)MEditSaveOriginal,
      /* 09 */ DI_RADIOBUTTON,5,9,0,9,0,0,0,0,(const wchar_t *)MEditSaveDOS,
      /* 10 */ DI_RADIOBUTTON,5,10,0,10,0,0,0,0,(const wchar_t *)MEditSaveUnix,
      /* 11 */ DI_RADIOBUTTON,5,11,0,11,0,0,0,0,(const wchar_t *)MEditSaveMac,
      /* 12 */ DI_TEXT,3,12,0,12,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
      /* 13 */ DI_BUTTON,0,13,0,13,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MOk,
      /* 14 */ DI_BUTTON,0,13,0,13,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel,
    };

    MakeDialogItemsEx(EditDlgData,EditDlg);

    EditDlg[2].strData = (/*Flags.Check(FFILEEDIT_SAVETOSAVEAS)?strFullFileName:strFileName*/strFileName);

    wchar_t *PtrEditDlgData=EditDlg[ID_SF_FILENAME].strData.GetBuffer ();

    PtrEditDlgData = wcsstr (PtrEditDlgData, UMSG(MNewFileName));

    if(PtrEditDlgData)
      *PtrEditDlgData=0;

    EditDlg[2].strData.ReleaseBuffer();

    EditDlg[ID_SF_DONOTCHANGE].Selected = 0;
    EditDlg[ID_SF_DOS].Selected = 0;
    EditDlg[ID_SF_UNIX].Selected = 0;
    EditDlg[ID_SF_MAC].Selected=0;
    EditDlg[ID_SF_DONOTCHANGE+TextFormat].Selected = TRUE;

    Dialog Dlg(EditDlg, countof (EditDlg), (FARWINDOWPROC)hndSaveFileAs, (LONG_PTR)&codepage);
    Dlg.SetPosition(-1,-1,76,16);
    Dlg.SetHelp(L"FileSaveAs");
    Dlg.Process();

    if ( (Dlg.GetExitCode() == ID_SF_OK) && !EditDlg[ID_SF_FILENAME].strData.IsEmpty() )
    {
		strFileName = EditDlg[ID_SF_FILENAME].strData;

		if (EditDlg[ID_SF_DONOTCHANGE].Selected)
			TextFormat=0;

		if (EditDlg[ID_SF_DOS].Selected)
			TextFormat=1;

		if (EditDlg[ID_SF_UNIX].Selected)
			TextFormat=2;

		if (EditDlg[ID_SF_MAC].Selected)
			TextFormat=3;

       return true;
	}

    return false;
}


FileEditor::FileEditor(
		const wchar_t *Name,
		int codepage,
		DWORD InitFlags,
		int StartLine,
		int StartChar,
		const wchar_t *PluginData,
		int OpenModeExstFile
		)
{
  ScreenObject::SetPosition(0,0,ScrX,ScrY);
  Flags.Set(InitFlags);
  Flags.Set(FFILEEDIT_FULLSCREEN);
  Init(Name,codepage, NULL,InitFlags,StartLine,StartChar, PluginData,FALSE,OpenModeExstFile);
}


FileEditor::FileEditor(
		const wchar_t *Name,
		int codepage,
		DWORD InitFlags,
		int StartLine,
		int StartChar,
		const wchar_t *Title,
		int X1,
		int Y1,
		int X2,
		int Y2,
		int DeleteOnClose,
		int OpenModeExstFile
		)
{
  Flags.Set(InitFlags);
  /* $ 02.11.2001 IS
       отрицательные координаты левого верхнего угла заменяются на нулевые
  */
  if(X1 < 0)
    X1=0;
  if(X2 < 0 || X2 > ScrX)
    X2=ScrX;
  if(Y1 < 0)
    Y1=0;
  if(Y2 < 0 || Y2 > ScrY)
    Y2=ScrY;
  if(X1 >= X2)
  {
    X1=0;
    X2=ScrX;
  }
  if(Y1 >= Y2)
  {
    Y1=0;
    Y2=ScrY;
  }

  /* IS $ */
  ScreenObject::SetPosition(X1,Y1,X2,Y2);
  Flags.Change(FFILEEDIT_FULLSCREEN,(X1==0 && Y1==0 && X2==ScrX && Y2==ScrY));
  Init(Name,codepage, Title,InitFlags,StartLine,StartChar,L"",DeleteOnClose,OpenModeExstFile);
}

/* $ 07.05.2001 DJ
   в деструкторе грохаем EditNamesList, если он был создан, а в SetNamesList()
   создаем EditNamesList и копируем туда значения
*/
/*
  Вызов деструкторов идет так:
    FileEditor::~FileEditor()
    Editor::~Editor()
    ...
*/
FileEditor::~FileEditor()
{
  //AY: флаг оповещающий закрытие редактора.
  m_bClosing = true;

  if (m_editor->EdOpt.SavePos && CtrlObject!=NULL)
	SaveToCache ();

  BitFlags FEditFlags=m_editor->Flags;
  int FEditEditorID=m_editor->EditorID;

  if (bEE_READ_Sent)
  {
    FileEditor *save = CtrlObject->Plugins.CurEditor;
    CtrlObject->Plugins.CurEditor=this;
    CtrlObject->Plugins.ProcessEditorEvent(EE_CLOSE,&FEditEditorID);
    CtrlObject->Plugins.CurEditor = save;
  }

  if (!Flags.Check(FFILEEDIT_OPENFAILED))
  {
    /* $ 11.10.2001 IS
       Удалим файл вместе с каталогом, если это просится и файла с таким же
       именем не открыто в других фреймах.
    */
    /* $ 14.06.2001 IS
       Если установлен FEDITOR_DELETEONLYFILEONCLOSE и сброшен
       FEDITOR_DELETEONCLOSE, то удаляем только файл.
    */
    if ( Flags.Check(FFILEEDIT_DELETEONCLOSE|FFILEEDIT_DELETEONLYFILEONCLOSE) &&
       !FrameManager->CountFramesWithName(strFullFileName))
    {
       if( Flags.Check(FFILEEDIT_DELETEONCLOSE))
         DeleteFileWithFolderW(strFullFileName);
       else
       {
         SetFileAttributesW(strFullFileName,FILE_ATTRIBUTE_NORMAL);
         DeleteFileW(strFullFileName); //BUGBUG
       }
    }
    /* IS 14.06.2002 $ */
    /* IS 11.10.2001 $ */
  }

  if( m_editor )
    delete m_editor;

  m_editor=NULL;

  CurrentEditor=NULL;
  if (EditNamesList)
    delete EditNamesList;
}

void FileEditor::Init (
		const wchar_t *Name,
		int codepage,
		const wchar_t *Title,
		DWORD InitFlags,
		int StartLine,
		int StartChar,
		const wchar_t *PluginData,
		int DeleteOnClose,
		int OpenModeExstFile
		)
{
  SysErrorCode=0;
  int BlankFileName=!wcscmp(Name,UMSG(MNewFileName));

  //AY: флаг оповещающий закрытие редактора.
  m_bClosing = false;

  bEE_READ_Sent = false;

  m_editor = new Editor;

  if ( !m_editor )
  {
	ExitCode=XC_OPEN_ERROR;
	return;
  }

  m_codepage = codepage;

  m_editor->SetOwner (this);
  m_editor->SetCodePage (m_codepage);

  /* $ 19.02.2001 IS
       Я не учел, что для нового файла GetFileAttributes не вызывается...
  */
  *AttrStr=0;
  /* IS $ */
  CurrentEditor=this;
  FileAttributes=(DWORD)-1;
  FileAttributesModified=false;
  SetTitle(Title);
  /* $ 07.05.2001 DJ */
  EditNamesList = NULL;
  KeyBarVisible = Opt.EdOpt.ShowKeyBar;
  /* DJ $ */

  /* $ 17.08.2001 KM
    Добавлено для поиска по AltF7. При редактировании найденного файла из
    архива для клавиши F2 сделать вызов ShiftF2.
  */
  Flags.Change(FFILEEDIT_SAVETOSAVEAS,(BlankFileName?TRUE:FALSE));
  /* KM $ */

  if (*Name==0)
  {
    ExitCode=XC_OPEN_ERROR;
    return;
  }

  SetPluginData(PluginData);
  m_editor->SetHostFileEditor(this);
  SetCanLoseFocus(Flags.Check(FFILEEDIT_ENABLEF6));

  FarGetCurDirW (strStartDir);

  if(!SetFileName(Name))
  {
    ExitCode=XC_OPEN_ERROR;
    return;
  }

  /*$ 11.05.2001 OT */
  //int FramePos=FrameManager->FindFrameByFile(MODALTYPE_EDITOR,FullFileName);
  //if (FramePos!=-1)
  if (Flags.Check(FFILEEDIT_ENABLEF6))
  {
    //if (Flags.Check(FFILEEDIT_ENABLEF6))
    int FramePos=FrameManager->FindFrameByFile(MODALTYPE_EDITOR, strFullFileName);
    if (FramePos!=-1)
    {
      int SwitchTo=FALSE;
      int MsgCode=0;
      if (!(*FrameManager)[FramePos]->GetCanLoseFocus(TRUE) ||
          Opt.Confirm.AllowReedit)
      {
        if(OpenModeExstFile == FEOPMODE_QUERY)
        {
          string strMsgFullFileName;
          strMsgFullFileName = strFullFileName;
          SetMessageHelp(L"EditorReload");
          MsgCode=MessageW(0,3,UMSG(MEditTitle),
                TruncPathStrW(strMsgFullFileName,ScrX-16),
                UMSG(MAskReload),
                UMSG(MCurrent),UMSG(MNewOpen),UMSG(MReload));
        }
        else
        {
          MsgCode=(OpenModeExstFile==FEOPMODE_USEEXISTING)?0:
                        (OpenModeExstFile==FEOPMODE_NEWIFOPEN?1:-100);
        }
        switch(MsgCode)
        {
          case 0:         // Current
            SwitchTo=TRUE;
            FrameManager->DeleteFrame(this); //???
            break;
          case 1:         // NewOpen
            SwitchTo=FALSE;
            break;
          case 2:         // Reload
            FrameManager->DeleteFrame(FramePos);
            SetExitCode(-2);
            break;
          case -100:
            //FrameManager->DeleteFrame(this);  //???
            SetExitCode(XC_EXISTS);
            return;
          default:
            FrameManager->DeleteFrame(this);  //???
            SetExitCode(MsgCode == -100?XC_EXISTS:XC_QUIT);
            return;
        }
      }
      else
      {
        SwitchTo=TRUE;
      }
      if (SwitchTo)
      {
        FrameManager->ActivateFrame(FramePos);
        //FrameManager->PluginCommit();
        SetExitCode((OpenModeExstFile != FEOPMODE_QUERY)?XC_EXISTS:TRUE);
        return ;
      }
    }
  }
  /* 11.05.2001 OT $*/

  /* $ 29.11.2000 SVS
     Если файл имеет атрибут ReadOnly или System или Hidden,
     И параметр на запрос выставлен, то сначала спросим.
  */
  /* $ 03.12.2000 SVS
     System или Hidden - задаются отдельно
  */
  /* $ 15.12.2000 SVS
    - Shift-F4, новый файл. Выдает сообщение :-(
  */
  DWORD FAttr=::GetFileAttributesW(Name);
  /* $ 05.06.2001 IS
     + посылаем подальше всех, кто пытается отредактировать каталог
  */
  if(FAttr!=-1 && FAttr&FILE_ATTRIBUTE_DIRECTORY)
  {
    MessageW(MSG_WARNING,1,UMSG(MEditTitle),UMSG(MEditCanNotEditDirectory),UMSG(MOk));
    ExitCode=XC_OPEN_ERROR;
    return;
  }
  /* IS $ */
  if((m_editor->EdOpt.ReadOnlyLock&2) &&
     FAttr != -1 &&
     (FAttr &
        (FILE_ATTRIBUTE_READONLY|
           /* Hidden=0x2 System=0x4 - располагаются во 2-м полубайте,
              поэтому применяем маску 0110.0000 и
              сдвигаем на свое место => 0000.0110 и получаем
              те самые нужные атрибуты  */
           ((m_editor->EdOpt.ReadOnlyLock&0x60)>>4)
        )
     )
  )
  /* SVS $ */
  {
    if(MessageW(MSG_WARNING,2,UMSG(MEditTitle),Name,UMSG(MEditRSH),
                             UMSG(MEditROOpen),UMSG(MYes),UMSG(MNo)))
    {
      //SetLastError(ERROR_ACCESS_DENIED);
      ExitCode=XC_OPEN_ERROR;
      return;
    }
  }
  /* SVS 03.12.2000 $ */
  /* SVS $ */

  m_editor->SetPosition(X1,Y1,X2,Y2-1);
  m_editor->SetStartPos(StartLine,StartChar);
  SetDeleteOnClose(DeleteOnClose);
  int UserBreak;
  /* $ 06.07.2001 IS
     При создании файла с нуля так же посылаем плагинам событие EE_READ, дабы
     не нарушать однообразие.
  */
  if(FAttr == -1)
    Flags.Set(FFILEEDIT_NEW);

  if(BlankFileName || Flags.Check(FFILEEDIT_CANNEWFILE))
    Flags.Set(FFILEEDIT_NEW);

  if (!LoadFile(strFullFileName,UserBreak))
  {
    m_codepage = CP_OEMCP; //BUGBUG
    m_editor->SetCodePage (m_codepage);

    if(BlankFileName)
    {
      Flags.Clear(FFILEEDIT_OPENFAILED); //AY: ну так как редактор мы открываем то видимо надо и сбросить ошибку открытия
      UserBreak=0;
    }
    if(!Flags.Check(FFILEEDIT_NEW) || UserBreak)
    {
      if (UserBreak!=1)
      {
        SetLastError(SysErrorCode);
        MessageW(MSG_WARNING|MSG_ERRORTYPE,1,UMSG(MEditTitle),UMSG(MEditCannotOpen),strFileName,UMSG(MOk));
        ExitCode=XC_OPEN_ERROR;
      }
      else
      {
        ExitCode=XC_LOADING_INTERRUPTED;
      }
      //FrameManager->DeleteFrame(this); // BugZ#546 - Editor валит фар!
      //CtrlObject->Cp()->Redraw(); //AY: вроде как не надо, делает проблемы
                                    //    с проресовкой если в редакторе из истории
                                    //    попытаться выбрать несуществующий файл

      return;
    }
  }

  CtrlObject->Plugins.CurEditor=this;//&FEdit;
  CtrlObject->Plugins.ProcessEditorEvent(EE_READ,NULL);
  bEE_READ_Sent = true;

  /* IS $ */
  ShowConsoleTitle();
  EditKeyBar.SetOwner(this);
  EditKeyBar.SetPosition(X1,Y2,X2,Y2);

  /* $ 07.08.2000 SVS
    ! Код, касаемый KeyBar вынесен в отдельную функцию */
  InitKeyBar();
  /* SVS $*/

  if ( Opt.EdOpt.ShowKeyBar==0 )
    EditKeyBar.Hide0();

  MacroMode=MACRO_EDITOR;
  CtrlObject->Macro.SetMode(MACRO_EDITOR);

  if (Flags.Check(FFILEEDIT_ENABLEF6))
    FrameManager->InsertFrame(this);
  else
    FrameManager->ExecuteFrame(this);

}

void FileEditor::InitKeyBar(void)
{
  EditKeyBar.SetAllGroup (KBL_MAIN,      Opt.OnlyEditorViewerUsed?MSingleEditF1:MEditF1, 12);
  EditKeyBar.SetAllGroup (KBL_SHIFT,     Opt.OnlyEditorViewerUsed?MSingleEditShiftF1:MEditShiftF1, 12);
  EditKeyBar.SetAllGroup (KBL_ALT,       Opt.OnlyEditorViewerUsed?MSingleEditAltF1:MEditAltF1, 12);
  EditKeyBar.SetAllGroup (KBL_CTRL,      Opt.OnlyEditorViewerUsed?MSingleEditCtrlF1:MEditCtrlF1, 12);
  EditKeyBar.SetAllGroup (KBL_CTRLSHIFT, Opt.OnlyEditorViewerUsed?MSingleEditCtrlShiftF1:MEditCtrlShiftF1, 12);
  EditKeyBar.SetAllGroup (KBL_CTRLALT,   Opt.OnlyEditorViewerUsed?MSingleEditCtrlAltF1:MEditCtrlAltF1, 12);
  EditKeyBar.SetAllGroup (KBL_ALTSHIFT,  Opt.OnlyEditorViewerUsed?MSingleEditAltShiftF1:MEditAltShiftF1, 12);

  if(!GetCanLoseFocus())
    EditKeyBar.Change(KBL_SHIFT,L"",4-1);

  if(Flags.Check(FFILEEDIT_SAVETOSAVEAS))
    EditKeyBar.Change(KBL_MAIN,UMSG(MEditShiftF2),2-1);

  if(!Flags.Check(FFILEEDIT_ENABLEF6))
    EditKeyBar.Change(KBL_MAIN,L"",6-1);
  if(!GetCanLoseFocus())
    EditKeyBar.Change(KBL_MAIN,L"",12-1);

  if(!GetCanLoseFocus())
    EditKeyBar.Change(KBL_ALT,L"",11-1);
  if(!Opt.UsePrintManager || CtrlObject->Plugins.FindPlugin(SYSID_PRINTMANAGER))
    EditKeyBar.Change(KBL_ALT,L"",5-1);


/*  if (m_editor->AnsiText)
    EditKeyBar.Change(KBL_MAIN,UMSG(Opt.OnlyEditorViewerUsed?MSingleEditF8DOS:MEditF8DOS),7);
  else*/
    EditKeyBar.Change(KBL_MAIN,UMSG(Opt.OnlyEditorViewerUsed?MSingleEditF8:MEditF8),7);

  EditKeyBar.Show();
  m_editor->SetPosition(X1,Y1,X2,Y2-(Opt.EdOpt.ShowKeyBar?1:0));
  SetKeyBar(&EditKeyBar);
}

void FileEditor::SetNamesList (NamesList *Names)
{
  if (EditNamesList == NULL)
    EditNamesList = new NamesList;
  Names->MoveData (*EditNamesList);
}

void FileEditor::Show()
{
  if (Flags.Check(FFILEEDIT_FULLSCREEN))
  {
    if ( Opt.EdOpt.ShowKeyBar )
    {
       EditKeyBar.SetPosition(0,ScrY,ScrX,ScrY);
       EditKeyBar.Redraw();
    }
    ScreenObject::SetPosition(0,0,ScrX,ScrY-(Opt.EdOpt.ShowKeyBar?1:0));
    m_editor->SetPosition(0,0,ScrX,ScrY-(Opt.EdOpt.ShowKeyBar?1:0));
  }
  ScreenObject::Show();
}


void FileEditor::DisplayObject()
{
  if ( !m_editor->Locked() )
  {
    if(m_editor->Flags.Check(FEDITOR_ISRESIZEDCONSOLE))
    {
      m_editor->Flags.Clear(FEDITOR_ISRESIZEDCONSOLE);
      CtrlObject->Plugins.CurEditor=this;
      CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_CHANGE);//EEREDRAW_ALL);
    }
    m_editor->Show();
  }
}

bool IsUnicodeCP (int codepage) //BUGBUG
{
	return (codepage == CP_UNICODE) || (codepage == CP_UTF8) || (codepage == CP_UTF7) || (codepage == CP_REVERSEBOM);
}


int FileEditor::ProcessKey(int Key)
{
  return ReProcessKey(Key,FALSE);
}

int FileEditor::ReProcessKey(int Key,int CalledFromControl)
{
  DWORD FNAttr;

  if (Flags.Check(FFILEEDIT_REDRAWTITLE) && ((Key & 0x00ffffff) < KEY_END_FKEY))
    ShowConsoleTitle();

  // BugZ#488 - Shift=enter
  if(ShiftPressed && Key == KEY_ENTER && CtrlObject->Macro.IsExecuting() == MACROMODE_NOMACRO)
  {
    Key=KEY_SHIFTENTER;
  }

  // Все сотальные необработанные клавиши пустим далее
  /* $ 28.04.2001 DJ
     не передаем KEY_MACRO* плагину - поскольку ReadRec в этом случае
     никак не соответствует обрабатываемой клавише, возникают разномастные
     глюки
  */
  if(Key >= KEY_MACRO_BASE && Key <= KEY_MACRO_ENDBASE) // исключаем MACRO
  {
    if(Key == MCODE_V_EDITORSTATE)
    {
      DWORD MacroEditState=0;
      MacroEditState|=Flags.Flags&FFILEEDIT_NEW?0x00000001:0;
      MacroEditState|=Flags.Flags&FFILEEDIT_ENABLEF6?0x00000002:0;
      MacroEditState|=Flags.Flags&FFILEEDIT_DELETEONCLOSE?0x00000004:0;
      MacroEditState|=m_editor->Flags.Flags&FEDITOR_MODIFIED?0x00000008:0;
      MacroEditState|=m_editor->BlockStart?0x00000010:0;
      MacroEditState|=m_editor->VBlockStart?0x00000020:0;
      MacroEditState|=m_editor->Flags.Flags&FEDITOR_WASCHANGED?0x00000040:0;
      MacroEditState|=m_editor->Flags.Flags&FEDITOR_OVERTYPE?0x00000080:0;
      MacroEditState|=m_editor->Flags.Flags&FEDITOR_CURPOSCHANGEDBYPLUGIN?0x00000100:0;
      MacroEditState|=m_editor->Flags.Flags&FEDITOR_LOCKMODE?0x00000200:0;
      MacroEditState|=m_editor->EdOpt.PersistentBlocks?0x00000400:0;
      return MacroEditState;
    }

    if(Key == MCODE_V_EDITORCURPOS)
      return m_editor->CurLine->GetTabCurPos()+1;
    if(Key == MCODE_V_EDITORCURLINE)
      return m_editor->NumLine+1;
    if(Key == MCODE_V_ITEMCOUNT || Key == MCODE_V_EDITORLINES)
      return m_editor->NumLastLine;

    return(m_editor->ProcessKey(Key));
  }
  /* DJ $ */

  switch(Key)
  {
    /* $ 27.09.2000 SVS
       Печать файла/блока с использованием плагина PrintMan
    */
    case KEY_ALTF5:
    {
      if(Opt.UsePrintManager && CtrlObject->Plugins.FindPlugin(SYSID_PRINTMANAGER))
      {
        CtrlObject->Plugins.CallPlugin(SYSID_PRINTMANAGER,OPEN_EDITOR,0); // printman
        return TRUE;
      }
      break; // отдадим Alt-F5 на растерзание плагинам, если не установлен PrintMan
    }
    /* SVS $*/

    case KEY_F6:
    {
      /* $ 10.05.2001 DJ
         используем EnableF6
      */
      if (Flags.Check(FFILEEDIT_ENABLEF6))
      {
        int FirstSave=1, NeedQuestion=1;
        // проверка на "а может это говно удалили уже?"
        // возможно здесь она и не нужна!
        // хотя, раз уж были изменени, то
        if(m_editor->IsFileChanged() &&  // в текущем сеансе были изменения?
           ::GetFileAttributesW(strFullFileName) == -1) // а файл еще существует?
        {
          switch(MessageW(MSG_WARNING,2,UMSG(MEditTitle),
                         UMSG(MEditSavedChangedNonFile),
                         UMSG(MEditSavedChangedNonFile2),
                         UMSG(MEditSave),UMSG(MCancel)))
          {
            case 0:
              if(ProcessKey(KEY_F2))
              {
                FirstSave=0;
                break;
              }
            default:
              return FALSE;
          }
        }

        if(!FirstSave || m_editor->IsFileChanged() || ::GetFileAttributesW (strFullFileName)!=-1)
        {
          long FilePos=m_editor->GetCurPos();
          /* $ 01.02.2001 IS
             ! Открываем вьюер с указанием длинного имени файла, а не короткого
          */
          if (ProcessQuitKey(FirstSave,NeedQuestion))
          {
            /* $ 11.10.200 IS
               не будем удалять файл, если было включено удаление, но при этом
               пользователь переключился во вьюер
            */
            SetDeleteOnClose(0);
            /* IS $ */
            /* $ 06.05.2001 DJ
               обработка F6 под NWZ
            */
            /* $ 07.05.2001 DJ
               сохраняем NamesList
            */


            FileViewer *Viewer = new FileViewer (strFullFileName, GetCanLoseFocus(), FALSE,
               FALSE, FilePos, NULL, EditNamesList, Flags.Check(FFILEEDIT_SAVETOSAVEAS));
            /* DJ $ */
  //OT          FrameManager->InsertFrame (Viewer);
            /* DJ $ */
          }
          /* IS $ */
          ShowTime(2);
        }
        return(TRUE);
      }
      break; // отдадим F6 плагинам, если есть запрет на переключение
      /* DJ $ */
    }

    /* $ 10.05.2001 DJ
       Alt-F11 - показать view/edit history
    */
    case KEY_ALTF11:
    {
      if (GetCanLoseFocus())
      {
        CtrlObject->CmdLine->ShowViewEditHistory();
        return TRUE;
      }
      break; // отдадим Alt-F11 на растерзание плагинам, если редактор модальный
    }
    /* DJ $ */
  }

#if 1
  BOOL ProcessedNext=TRUE;

  _SVS(if(Key=='n' || Key=='m'))
    _SVS(SysLog(L"%d Key='%c'",__LINE__,Key));

  if(!CalledFromControl && (CtrlObject->Macro.IsRecording() == MACROMODE_RECORDING_COMMON || CtrlObject->Macro.IsExecuting() == MACROMODE_EXECUTING_COMMON || CtrlObject->Macro.GetCurRecord(NULL,NULL) == MACROMODE_NOMACRO))
  {
    _SVS(if(CtrlObject->Macro.IsRecording() == MACROMODE_RECORDING_COMMON || CtrlObject->Macro.IsExecuting() == MACROMODE_EXECUTING_COMMON))
      _SVS(SysLog(L"%d !!!! CtrlObject->Macro.GetCurRecord(NULL,NULL) != MACROMODE_NOMACRO !!!!",__LINE__));
    ProcessedNext=!ProcessEditorInput(FrameManager->GetLastInputRecord());
  }

  if (ProcessedNext)
#else
  if (!CalledFromControl && //CtrlObject->Macro.IsExecuting() || CtrlObject->Macro.IsRecording() || // пусть доходят!
    !ProcessEditorInput(FrameManager->GetLastInputRecord()))
#endif
  {
    switch(Key)
    {
      case KEY_F1:
      {
        Help Hlp (L"Editor");
        return(TRUE);
      }

      /* $ 25.04.2001 IS
           ctrl+f - вставить в строку полное имя редактируемого файла
      */
      case KEY_CTRLF:
      {
        if (!m_editor->Flags.Check(FEDITOR_LOCKMODE))
        {
          m_editor->Pasting++;
          m_editor->TextChanged(1);
          BOOL IsBlock=m_editor->VBlockStart || m_editor->BlockStart;
          if (!m_editor->EdOpt.PersistentBlocks && IsBlock)
          {
            m_editor->Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);
            m_editor->DeleteBlock();
          }
          //AddUndoData(CurLine->EditLine.GetStringAddr(),NumLine,
          //                CurLine->EditLine.GetCurPos(),UNDO_EDIT);
          m_editor->Paste(strFullFileName); //???
          //if (!EdOpt.PersistentBlocks)
          m_editor->UnmarkBlock();
          m_editor->Pasting--;
          m_editor->Show(); //???
        }
        return (TRUE);
      }
      /* IS $ */
      /* $ 24.08.2000 SVS
         + Добавляем реакцию показа бакграунда на клавишу CtrlAltShift
      */
      case KEY_CTRLO:
      {
        if(!Opt.OnlyEditorViewerUsed)
        {
          m_editor->Hide();  // $ 27.09.2000 skv - To prevent redraw in macro with Ctrl-O
          if(FrameManager->ShowBackground())
          {
            SetCursorType(FALSE,0);
            WaitKey();
          }
          Show();
        }
        return(TRUE);
      }
  /* $ KEY_CTRLALTSHIFTPRESS унесено в manager OT */

      case KEY_F2:
      case KEY_SHIFTF2:
      {
        BOOL Done=FALSE;

        string strOldCurDir;
        FarGetCurDirW (strOldCurDir);

        wchar_t *lpwszPtr;
        wchar_t wChr;

        while(!Done) // бьемся до упора
        {
          // проверим путь к файлу, может его уже снесли...
          lpwszPtr = strFullFileName.GetBuffer ();

          lpwszPtr=wcsrchr(lpwszPtr,L'\\');
          if(lpwszPtr)
          {
            wChr=*lpwszPtr;
            *lpwszPtr=0;
            // В корне?
            if (!(LocalIsalphaW(strFullFileName.At(0)) && (strFullFileName.At(1)==L':') && !strFullFileName.At(2)))
            {
              // а дальше? каталог существует?
              if((FNAttr=::GetFileAttributesW(strFullFileName)) == -1 ||
                                !(FNAttr&FILE_ATTRIBUTE_DIRECTORY)
                  //|| LocalStricmp(OldCurDir,FullFileName)  // <- это видимо лишнее.
                )
                Flags.Set(FFILEEDIT_SAVETOSAVEAS);
            }
            *lpwszPtr=wChr;
          }
          strFullFileName.ReleaseBuffer ();


          if(Key == KEY_F2 &&
             (FNAttr=::GetFileAttributesW(strFullFileName)) != -1 &&
             !(FNAttr&FILE_ATTRIBUTE_DIRECTORY))
              Flags.Clear(FFILEEDIT_SAVETOSAVEAS);

          static int TextFormat=0;

		  int codepage = m_codepage;

          bool SaveAs = Key==KEY_SHIFTF2 || Flags.Check(FFILEEDIT_SAVETOSAVEAS);

          int NameChanged=FALSE;

          string strFullSaveAsName = strFullFileName;

          if ( SaveAs )
          {
            string strSaveAsName = Flags.Check(FFILEEDIT_SAVETOSAVEAS)?strFullFileName:strFileName;

            if ( !dlgSaveFileAs (strSaveAsName, TextFormat, codepage) )
            	return FALSE;

            apiExpandEnvironmentStrings (strSaveAsName, strSaveAsName);

            RemoveTrailingSpacesW(strSaveAsName);
            UnquoteW(strSaveAsName);

            NameChanged=LocalStricmpW(strSaveAsName, (Flags.Check(FFILEEDIT_SAVETOSAVEAS)?strFullFileName:strFileName))!=0;

            if( !NameChanged )
              FarChDirW (strStartDir); // ПОЧЕМУ? А нужно ли???

            FNAttr=::GetFileAttributesW(strSaveAsName);
            if (NameChanged && FNAttr != -1)
            {
              if (MessageW(MSG_WARNING,2,UMSG(MEditTitle),strSaveAsName,UMSG(MEditExists),
                           UMSG(MEditOvr),UMSG(MYes),UMSG(MNo))!=0)
              {
                FarChDirW(strOldCurDir);
                return(TRUE);
              }
              Flags.Set(FFILEEDIT_SAVEWQUESTIONS);
            }

            ConvertNameToFullW (strSaveAsName, strFullSaveAsName); //BUGBUG, не проверяем имя на правильность

            //это не про нас, про нас ниже, все куда страшнее

            /*string strFileNameTemp = strSaveAsName;

            if(!SetFileName(strFileNameTemp))
            {
              SetLastError(ERROR_INVALID_NAME);
              MessageW(MSG_WARNING|MSG_ERRORTYPE,1,UMSG(MEditTitle),strFileNameTemp,UMSG(MOk));
              if(!NameChanged)
                FarChDirW(strOldCurDir);
              continue;
              //return FALSE;
            } */

            if(!NameChanged)
              FarChDirW(strOldCurDir);
          }
          ShowConsoleTitle();

          FarChDirW (strStartDir); //???

          if(SaveFile(strFullSaveAsName, 0, SaveAs, TextFormat, codepage) == SAVEFILE_ERROR)
          {
            SetLastError(SysErrorCode);
            if (MessageW(MSG_WARNING|MSG_ERRORTYPE,2,UMSG(MEditTitle),UMSG(MEditCannotSave),
                        strFileName,UMSG(MRetry),UMSG(MCancel))!=0)
            {
              Done=TRUE;
              break;
            }
          }
          else
          {
            //здесь идет полная жопа, проверка на ошибки вообще пока отсутствует
            {
            	bool bInPlace = (!IsUnicodeCP(m_codepage) && !IsUnicodeCP(codepage)) || (m_codepage == codepage);

            	if ( !bInPlace )
            		m_editor->FreeAllocatedData ();

            	SetFileName (strFullSaveAsName);

            	SetCodePage (codepage); //

            	if ( !bInPlace )
            	{
					MessageW (MSG_WARNING, 1, L"WARNING!", L"Editor will be reopened with new file!", UMSG(MOk));

					int UserBreak;
					LoadFile (strFullSaveAsName, UserBreak);

					Show();//!!! BUGBUG
				}
            }

            Done=TRUE;
          }
        }
        return(TRUE);
      }

      // $ 30.05.2003 SVS - Shift-F4 в редакторе/вьювере позволяет открывать другой редактор/вьювер (пока только редактор)
      case KEY_SHIFTF4:
      {
        if(!Opt.OnlyEditorViewerUsed && GetCanLoseFocus())
          CtrlObject->Cp()->ActivePanel->ProcessKey(Key);
        return TRUE;
      }

      /*$ 21.07.2000 SKV
          + выход с позиционированием на редактируемом файле по CTRLF10
      */
      case KEY_CTRLF10:
      {
        {
          if (isTemporary())
          {
            return(TRUE);
          }

          string strFullFileNameTemp = strFullFileName;
          /* 26.11.2001 VVM
            ! Использовать полное имя файла */
          /* $ 28.12.2001 DJ
             вынесем код в общую функцию
          */
          if(::GetFileAttributesW(strFullFileName) == -1) // а сам файл то еще на месте?
          {
              if(!CheckShortcutFolderW(&strFullFileNameTemp,-1,FALSE))
              return FALSE;
            strFullFileNameTemp += L"\\."; // для вваливания внутрь :-)
          }

          if(Flags.Check(FFILEEDIT_NEW))
          {
            UpdateFileList();
            Flags.Clear(FFILEEDIT_NEW);
          }

          {
            SaveScreen Sc;
            CtrlObject->Cp()->GoToFileW (strFullFileNameTemp);
            Flags.Set(FFILEEDIT_REDRAWTITLE);
          }
          /* DJ $ */
          /* VVM $ */
        }
        return (TRUE);
      }
      /* SKV $*/

      case KEY_CTRLB:
      {
        Opt.EdOpt.ShowKeyBar=!Opt.EdOpt.ShowKeyBar;
        if ( Opt.EdOpt.ShowKeyBar )
          EditKeyBar.Show();
        else
          EditKeyBar.Hide0(); // 0 mean - Don't purge saved screen
        Show();
        KeyBarVisible = Opt.EdOpt.ShowKeyBar;
        return (TRUE);
      }

      case KEY_SHIFTF10:
        if(!ProcessKey(KEY_F2)) // учтем факт того, что могли отказаться от сохранения
          return FALSE;
      case KEY_ESC:
      case KEY_F10:
      {
        int FirstSave=1, NeedQuestion=1;
        if(Key != KEY_SHIFTF10)    // KEY_SHIFTF10 не учитываем!
        {
          int FilePlased=::GetFileAttributesW (strFullFileName) == -1 && !Flags.Check(FFILEEDIT_NEW);
          if(m_editor->IsFileChanged() ||  // в текущем сеансе были изменения?
             FilePlased) // а сам файл то еще на месте?
          {
            int Res;
            if(m_editor->IsFileChanged() && FilePlased)
                Res=MessageW(MSG_WARNING,3,UMSG(MEditTitle),
                           UMSG(MEditSavedChangedNonFile),
                           UMSG(MEditSavedChangedNonFile2),
                           UMSG(MEditSave),UMSG(MEditNotSave),UMSG(MEditContinue));
            else if(!m_editor->IsFileChanged() && FilePlased)
                Res=MessageW(MSG_WARNING,3,UMSG(MEditTitle),
                           UMSG(MEditSavedChangedNonFile1),
                           UMSG(MEditSavedChangedNonFile2),
                           UMSG(MEditSave),UMSG(MEditNotSave),UMSG(MEditContinue));
            else
               Res=100;
            switch(Res)
            {
              case 0:
                if(!ProcessKey(KEY_F2))  // попытка сначала сохранить
                  NeedQuestion=0;
                FirstSave=0;
                break;
              case 1:
                NeedQuestion=0;
                FirstSave=0;
                break;
              case 100:
                FirstSave=NeedQuestion=1;
                break;
              case 2:
              default:
                return FALSE;
            }
          }
          else if(!m_editor->Flags.Check(FEDITOR_MODIFIED)) //????
            NeedQuestion=0;

        }
        if(!ProcessQuitKey(FirstSave,NeedQuestion))
          return FALSE;
        return(TRUE);
      }

		case KEY_SHIFTF8:
		{
			if ( !IsUnicodeCP(m_codepage) )
			{
				int codepage = GetTableEx ();

				if ( codepage != -1 )
					SetCodePage (codepage);
			}

			return TRUE;
		}

      /* $ 19.12.2000 SVS
         Вызов диалога настроек (с подачи IS)
      */
      case KEY_ALTSHIFTF9:
      {
        /* $ 26.02.2001 IS
             Работа с локальной копией EditorOptions
        */
        struct EditorOptions EdOpt;
        GetEditorOptions(EdOpt);

        EditorConfig(EdOpt,1); // $ 27.11.2001 DJ - Local в EditorConfig
        EditKeyBar.Show(); //???? Нужно ли????

        SetEditorOptions(EdOpt);

        /* IS $ */
        if ( Opt.EdOpt.ShowKeyBar )
          EditKeyBar.Show();

        m_editor->Show();
        return TRUE;
      }
      /* SVS $ */

      default:
      {
        if (Flags.Check(FFILEEDIT_FULLSCREEN) && CtrlObject->Macro.IsExecuting() == MACROMODE_NOMACRO)
          if ( Opt.EdOpt.ShowKeyBar )
            EditKeyBar.Show();
        /* SVS $ */
        if (!EditKeyBar.ProcessKey(Key))
          return(m_editor->ProcessKey(Key));
      }
    }
  }
  return(TRUE);
}


int FileEditor::ProcessQuitKey(int FirstSave,BOOL NeedQuestion)
{
  string strOldCurDir;
  FarGetCurDirW (strOldCurDir);
  while (1)
  {
    FarChDirW(strStartDir); // ПОЧЕМУ? А нужно ли???
    int SaveCode=SAVEFILE_SUCCESS;
    if(NeedQuestion)
    {
      SaveCode=SaveFile(strFullFileName,FirstSave,0,FALSE);
    }
    if (SaveCode==SAVEFILE_CANCEL)
      break;
    if (SaveCode==SAVEFILE_SUCCESS)
    {
      /* $ 09.02.2002 VVM
        + Обновить панели, если писали в текущий каталог */
      if (NeedQuestion)
      {
        UpdateFileList();
      }
      /* VVM $ */

      FrameManager->DeleteFrame();
      SetExitCode (XC_QUIT);
      break;
    }
    if(!wcscmp(strFileName,UMSG(MNewFileName)))
      if(!ProcessKey(KEY_SHIFTF2))
      {
        FarChDirW(strOldCurDir);
        return FALSE;
      }
      else
        break;
    SetLastError(SysErrorCode);
    if (MessageW(MSG_WARNING|MSG_ERRORTYPE,2,UMSG(MEditTitle),UMSG(MEditCannotSave),
              strFileName,UMSG(MRetry),UMSG(MCancel))!=0)
        break;
    FirstSave=0;
  }

  FarChDirW(strOldCurDir);
  return GetExitCode() == XC_QUIT;
}


// сюды плавно переносить код из Editor::ReadFile()
int FileEditor::LoadFile(const wchar_t *Name,int &UserBreak)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

	int LastLineCR = 0, Count = 0, MessageShown=FALSE;;
	EditorCacheParams cp;

	UserBreak = 0;

	FILE *EditFile;

	HANDLE hEdit = FAR_CreateFileW (
			Name,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_SEQUENTIAL_SCAN,
			NULL
			);

	if ( hEdit == INVALID_HANDLE_VALUE )
	{
		int LastError=GetLastError();
		SetLastError(LastError);

		if ( (LastError != ERROR_FILE_NOT_FOUND) && (LastError != ERROR_PATH_NOT_FOUND) )
		{
			UserBreak = -1;
			Flags.Set(FFILEEDIT_OPENFAILED);
		}

		return FALSE;
	}

	int EditHandle=_open_osfhandle((intptr_t)hEdit,O_BINARY);

	if ( EditHandle == -1 )
		return FALSE;

	if ( (EditFile=fdopen(EditHandle,"rb")) == NULL )
		return FALSE;

	if ( GetFileType(hEdit) != FILE_TYPE_DISK )
	{
		fclose(EditFile);
		SetLastError(ERROR_INVALID_NAME);

		UserBreak=-1;
		Flags.Set(FFILEEDIT_OPENFAILED);
		return FALSE;
	}


	m_editor->FreeAllocatedData ();

	bool bCached = LoadFromCache (&cp);

	GetFileString GetStr(EditFile);

    *m_editor->GlobalEOL=0; //BUGBUG???

	wchar_t *Str;
	int StrLength,GetCode;

	clock_t StartTime=clock();

	int dcp = GetFileFormat (EditFile, &m_bSignatureFound);

	if ( m_codepage == CP_AUTODETECT )
	{
		if ( bCached )
			m_codepage = cp.Table;

		if ( !bCached || (m_codepage == 0) )
			m_codepage = dcp;
	}

	m_editor->SetCodePage (m_codepage); //BUGBUG

	while ((GetCode=GetStr.GetStringW(&Str, m_codepage, StrLength))!=0)
	{
		if ( GetCode == -1 )
		{
			fclose(EditFile);
			SetPreRedrawFunc(NULL);
			return FALSE;
		}

		LastLineCR=0;

		if ( (++Count & 0xfff) == 0 && (clock()-StartTime > 500) )
		{
			if ( CheckForEsc() )
			{
				UserBreak = 1;
				fclose(EditFile);
				SetPreRedrawFunc(NULL);

				return FALSE;
			}
/*
			if (!MessageShown)
			{
				SetCursorType(FALSE,0);
				SetPreRedrawFunc(Editor::PR_EditorShowMsg);
				EditorShowMsg(UMSG(MEditTitle),UMSG(MEditReading),Name);
				MessageShown=TRUE;
			}
			*/
		}

		const wchar_t *CurEOL;

		if ( !LastLineCR &&
			 ((CurEOL = wmemchr(Str,L'\r',StrLength)) != NULL ||
			  (CurEOL=wmemchr(Str,L'\n',StrLength))!=NULL) )
		{
			xwcsncpy(m_editor->GlobalEOL,CurEOL,(sizeof(m_editor->GlobalEOL)-1)/sizeof(wchar_t));
			m_editor->GlobalEOL[sizeof(m_editor->GlobalEOL)-1]=0;
			LastLineCR=1;
		}

		if( !m_editor->InsertString (Str, StrLength) )
		{
			fclose(EditFile);
			SetPreRedrawFunc(NULL);
			return(FALSE);
		}
	}

	SetPreRedrawFunc(NULL);

	if ( LastLineCR || (Count == 0) )
		m_editor->InsertString (L"", 0);

	fclose (EditFile);

	if ( bCached )
		m_editor->SetCacheParams (&cp);

	SysErrorCode=GetLastError();
	apiGetFindDataEx (Name,&FileInfo);

	return TRUE;
}

#define SIGN_UNICODE 0xFEFF
#define SIGN_REVERSEBOM 0xFFFE
#define SIGN_UTF8 0xBFBBEF

//TextFormat и Codepage используются ТОЛЬКО, если bSaveAs = true!

int FileEditor::SaveFile(const wchar_t *Name,int Ask, bool bSaveAs, int TextFormat, int Codepage)
{



  if (m_editor->Flags.Check(FEDITOR_LOCKMODE) && !m_editor->Flags.Check(FEDITOR_MODIFIED) && !bSaveAs)
    return SAVEFILE_SUCCESS;

  if (Ask)
  {
    if(!m_editor->Flags.Check(FEDITOR_MODIFIED))
      return SAVEFILE_SUCCESS;

    if (Ask)
    {
      switch (MessageW(MSG_WARNING,3,UMSG(MEditTitle),UMSG(MEditAskSave),
              UMSG(MEditSave),UMSG(MEditNotSave),UMSG(MEditContinue)))
      {
        case -1:
        case -2:
        case 2:  // Continue Edit
          return SAVEFILE_CANCEL;
        case 0:  // Save
          break;
        case 1:  // Not Save
          m_editor->TextChanged(0); // 10.08.2000 skv: TextChanged() support;
          return SAVEFILE_SUCCESS;
      }
    }
  }

  int NewFile=TRUE;
  FileAttributesModified=false;
  if ((FileAttributes=::GetFileAttributesW(Name))!=-1)
  {
    // Проверка времени модификации...
    if(!Flags.Check(FFILEEDIT_SAVEWQUESTIONS))
    {
      FAR_FIND_DATA_EX FInfo;
      if( apiGetFindDataEx (Name,&FInfo) && !FileInfo.strFileName.IsEmpty())
      {
        __int64 RetCompare=*(__int64*)&FileInfo.ftLastWriteTime - *(__int64*)&FInfo.ftLastWriteTime;
        if(RetCompare || !(FInfo.nFileSize == FileInfo.nFileSize))
        {
          SetMessageHelp(L"WarnEditorSavedEx");
          switch (MessageW(MSG_WARNING,3,UMSG(MEditTitle),UMSG(MEditAskSaveExt),
                  UMSG(MEditSave),UMSG(MEditBtnSaveAs),UMSG(MEditContinue)))
          {
            case -1:
            case -2:
            case 2:  // Continue Edit
              return SAVEFILE_CANCEL;
            case 1:  // Save as
              if(ProcessKey(KEY_SHIFTF2))
                return SAVEFILE_SUCCESS;
              else
                return SAVEFILE_CANCEL;
            case 0:  // Save
              break;
          }
        }
      }
    }
    Flags.Clear(FFILEEDIT_SAVEWQUESTIONS);

    NewFile=FALSE;
    if (FileAttributes & FA_RDONLY)
    {
        //BUGBUG
      int AskOverwrite=MessageW(MSG_WARNING,2,UMSG(MEditTitle),Name,UMSG(MEditRO),
                           UMSG(MEditOvr),UMSG(MYes),UMSG(MNo));
      if (AskOverwrite!=0)
        return SAVEFILE_CANCEL;

      SetFileAttributesW(Name,FileAttributes & ~FA_RDONLY); // сняты атрибуты
      FileAttributesModified=true;
    }

    if (FileAttributes & (FA_HIDDEN|FA_SYSTEM))
    {
      SetFileAttributesW(Name,FILE_ATTRIBUTE_NORMAL);
      FileAttributesModified=true;
    }
  }
  else
  {
    // проверим путь к файлу, может его уже снесли...
    string strCreatedPath = Name;

    const wchar_t *Ptr = wcsrchr (strCreatedPath, L'\\');

    if ( Ptr )
    {
      CutToSlashW (strCreatedPath, true);
      DWORD FAttr=0;
      if(::GetFileAttributesW(strCreatedPath) == -1)
      {
        // и попробуем создать.
        // Раз уж
        CreatePathW(strCreatedPath);
        FAttr=::GetFileAttributesW(strCreatedPath);
      }

      if(FAttr == -1)
        return SAVEFILE_ERROR;
    }
  }

  int RetCode=SAVEFILE_SUCCESS;

  if (TextFormat!=0)
    m_editor->Flags.Set(FEDITOR_WASCHANGED);

  switch(TextFormat)
  {
    case 1:
      wcscpy(m_editor->GlobalEOL,DOS_EOL_fmtW);
      break;
    case 2:
      wcscpy(m_editor->GlobalEOL,UNIX_EOL_fmtW);
      break;
    case 3:
      wcscpy(m_editor->GlobalEOL,MAC_EOL_fmtW);
      break;
  }

  if(::GetFileAttributesW(Name) == -1)
    Flags.Set(FFILEEDIT_NEW);

  {
    FILE *EditFile;
    //SaveScreen SaveScr;
    /* $ 11.10.2001 IS
       Если было произведено сохранение с любым результатом, то не удалять файл
    */
    Flags.Clear(FFILEEDIT_DELETEONCLOSE|FFILEEDIT_DELETEONLYFILEONCLOSE);
    /* IS $ */
    CtrlObject->Plugins.CurEditor=this;
//_D(SysLog(L"%08d EE_SAVE",__LINE__));
    CtrlObject->Plugins.ProcessEditorEvent(EE_SAVE,NULL);

    HANDLE hEdit = FAR_CreateFileW (
    		Name,
    		GENERIC_WRITE,
    		FILE_SHARE_READ,
    		NULL,
    		CREATE_ALWAYS,
    		FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN,
    		NULL
    		);

    if (hEdit==INVALID_HANDLE_VALUE)
    {
      //_SVS(SysLogLastError();SysLog(L"Name='%s',FileAttributes=%d",Name,FileAttributes));
      RetCode=SAVEFILE_ERROR;
      SysErrorCode=GetLastError();
      goto end;
    }
    int EditHandle=_open_osfhandle((intptr_t)hEdit,O_BINARY);
    if (EditHandle==-1)
    {
      RetCode=SAVEFILE_ERROR;
      SysErrorCode=GetLastError();
      goto end;
    }
    if ((EditFile=fdopen(EditHandle,"wb"))==NULL)
    {
      RetCode=SAVEFILE_ERROR;
      SysErrorCode=GetLastError();
      goto end;
    }

    m_editor->UndoSavePos=m_editor->UndoDataPos;
    m_editor->Flags.Clear(FEDITOR_UNDOOVERFLOW);

//    ConvertNameToFull(Name,FileName, sizeof(FileName));
/*
    if (ConvertNameToFull(Name,m_editor->FileName, sizeof(m_editor->FileName)) >= sizeof(m_editor->FileName))
    {
      m_editor->Flags.Set(FEDITOR_OPENFAILED);
      RetCode=SAVEFILE_ERROR;
      goto end;
    }
*/
    SetCursorType(FALSE,0);
    SetPreRedrawFunc(Editor::PR_EditorShowMsg);
    Editor::EditorShowMsg(UMSG(MEditTitle),UMSG(MEditSaving),Name);

    Edit *CurPtr=m_editor->TopList;

    int codepage;
    bool signature_found = m_bSignatureFound;

    if ( bSaveAs )
    {
    	signature_found = true; //BUGBUG;
		codepage = Codepage;
	}
	else
		codepage = m_editor->GetCodePage (); //???

    if ( signature_found )
    {
	    bool bSignError = false;
	    DWORD dwSignature = 0;

		if ( (codepage == CP_UNICODE) || (codepage == CP_REVERSEBOM) )
		{
    		dwSignature = SIGN_UNICODE;
    		if ( fwrite (&dwSignature, 1, 2, EditFile) != 2 )
    			bSignError = true;
		}

		if ( codepage == CP_UTF8 )
		{
    		dwSignature = SIGN_UTF8;
    		if ( fwrite (&dwSignature, 1, 3, EditFile) != 3 )
    			bSignError = true;
		}

		if ( bSignError )
		{
			fclose(EditFile);
			_wremove(Name);

			RetCode=SAVEFILE_ERROR;
			goto end;
		}
	}

    while (CurPtr!=NULL)
    {
      const wchar_t *SaveStr, *EndSeq;
      int Length;
      CurPtr->GetBinaryStringW(&SaveStr,&EndSeq,Length);
      if (*EndSeq==0 && CurPtr->m_next!=NULL)
        EndSeq=*m_editor->GlobalEOL ? m_editor->GlobalEOL:DOS_EOL_fmtW;
      if (TextFormat!=0 && *EndSeq!=0)
      {
        if (TextFormat==1)
          EndSeq=DOS_EOL_fmtW;
        else if (TextFormat==2)
          EndSeq=UNIX_EOL_fmtW;
        else
          EndSeq=MAC_EOL_fmtW;
        CurPtr->SetEOLW(EndSeq);
      }

      int EndLength=wcslen(EndSeq);
      bool bError = false;

		if ( (codepage == CP_UNICODE) || (codepage == CP_REVERSEBOM) ) //BUGBUG, wrong revbom!!!
		{
			if ( (fwrite(SaveStr, sizeof (wchar_t), Length, EditFile) != Length) ||
				 (fwrite(EndSeq, sizeof (wchar_t), EndLength, EditFile) != EndLength) )
				bError = true;
		}
		else
		{
			int length = WideCharToMultiByte (codepage, 0, SaveStr, Length, NULL, 0, NULL, NULL);

            char *SaveStrCopy = new char[length];

            if ( SaveStrCopy )
            {
	            int endlength = WideCharToMultiByte (codepage, 0, EndSeq, EndLength, NULL, 0, NULL, NULL);

				char *EndSeqCopy = new char[endlength];

				if ( EndSeqCopy )
				{
					WideCharToMultiByte (codepage, 0, SaveStr, Length, SaveStrCopy, length, NULL, NULL);
					WideCharToMultiByte (codepage, 0, EndSeq, EndLength, EndSeqCopy, endlength, NULL, NULL);

					if ( (fwrite (SaveStrCopy,1,length,EditFile) != length) ||
						 (fwrite (EndSeqCopy,1,endlength,EditFile) != endlength) )
						bError = true;

					delete EndSeqCopy;
				}
				else
					bError = true;

				delete SaveStrCopy;
			}
			else
				bError = true;
		}

		if ( bError )
		{
			fclose(EditFile);
			_wremove(Name);

			RetCode=SAVEFILE_ERROR;
			goto end;

		}

		CurPtr=CurPtr->m_next;
    }
    if (fflush(EditFile)==EOF)
    {
      fclose(EditFile);
      _wremove(Name);
      RetCode=SAVEFILE_ERROR;
      goto end;
    }
    SetEndOfFile(hEdit);
    fclose(EditFile);
  }

end:
  SetPreRedrawFunc(NULL);

  if (FileAttributes!=-1 && FileAttributesModified)
  {
    SetFileAttributesW(Name,FileAttributes|FA_ARCH);
  }

  apiGetFindDataEx (strFullFileName,&FileInfo);

  if (m_editor->Flags.Check(FEDITOR_MODIFIED) || NewFile)
    m_editor->Flags.Set(FEDITOR_WASCHANGED);

  /* Этот кусок раскомметировать в том случае, если народ решит, что
     для если файл был залочен и мы его переписали под други именем...
     ...то "лочка" должна быть снята.
  */
//  if(SaveAs)
//    Flags.Clear(FEDITOR_LOCKMODE);


  /*$ 10.08.2000 skv
    Modified->TextChanged
  */
  /* 28.12.2001 VVM
    ! Проверить на успешную запись */
  if (RetCode==SAVEFILE_SUCCESS)
    m_editor->TextChanged(0);

  if(GetDynamicallyBorn()) // принудительно сбросим Title // Flags.Check(FFILEEDIT_SAVETOSAVEAS) ????????
    strTitle = L"";

  Show();

  // ************************************
  Flags.Clear(FFILEEDIT_NEW);

  return RetCode;
}

int FileEditor::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  if (!EditKeyBar.ProcessMouse(MouseEvent))
    if (!ProcessEditorInput(FrameManager->GetLastInputRecord()))
      if (!m_editor->ProcessMouse(MouseEvent))
        return(FALSE);
  return(TRUE);
}


int FileEditor::GetTypeAndName(string &strType, string &strName)
{
  strType = UMSG(MScreensEdit);
  strName = strFullFileName;

  return(MODALTYPE_EDITOR);
}


void FileEditor::ShowConsoleTitle()
{
    string strTitle;

    strTitle.Format (UMSG(MInEditor), PointToNameW (strFileName));
    SetFarTitleW (strTitle);

    Flags.Clear(FFILEEDIT_REDRAWTITLE);
}


/* $ 28.06.2000 tran
 (NT Console resize)
 resize editor */
void FileEditor::SetScreenPosition()
{
  if (Flags.Check(FFILEEDIT_FULLSCREEN))
  {
    SetPosition(0,0,ScrX,ScrY);
  }
}
/* tran $ */

/* $ 10.05.2001 DJ
   добавление в view/edit history
*/

void FileEditor::OnDestroy()
{
  _OT(SysLog(L"[%p] FileEditor::OnDestroy()",this));
  if (!Flags.Check(FFILEEDIT_DISABLEHISTORY) && _wcsicmp(strFileName,UMSG(MNewFileName)))
    CtrlObject->ViewHistory->AddToHistory(strFullFileName,UMSG(MHistoryEdit),
                  (m_editor->Flags.Check(FEDITOR_LOCKMODE)?4:1));
  /* $ 19.10.2001 OT
  */
  if (CtrlObject->Plugins.CurEditor==this)//&this->FEdit)
  {
    CtrlObject->Plugins.CurEditor=NULL;
  }
}

int FileEditor::GetCanLoseFocus(int DynamicMode)
{
  if (DynamicMode)
  {
    if (m_editor->IsFileModified())
    {
      return FALSE;
    }
  }
  else
  {
    return CanLoseFocus;
  }
  return TRUE;
}

void FileEditor::SetLockEditor(BOOL LockMode)
{
  if(LockMode)
    m_editor->Flags.Set(FEDITOR_LOCKMODE);
  else
    m_editor->Flags.Clear(FEDITOR_LOCKMODE);
}

int FileEditor::FastHide()
{
  return Opt.AllCtrlAltShiftRule & CASR_EDITOR;
}

BOOL FileEditor::isTemporary()
{
  return (!GetDynamicallyBorn());
}

void FileEditor::ResizeConsole()
{
  m_editor->PrepareResizedConsole();
}

int FileEditor::ProcessEditorInput(INPUT_RECORD *Rec)
{
  int RetCode;

  CtrlObject->Plugins.CurEditor=this;
  RetCode=CtrlObject->Plugins.ProcessEditorInput(Rec);

  return RetCode;
}

void FileEditor::SetPluginTitle(const wchar_t *PluginTitle)
{
    if ( !PluginTitle )
        strPluginTitle = L"";
    else
        strPluginTitle = PluginTitle;
}

BOOL FileEditor::SetFileName(const wchar_t *NewFileName)
{
  strFileName = NewFileName;

  if( wcscmp (strFileName,UMSG(MNewFileName)))
  {
    if ( wcspbrk (strFileName, ReservedFilenameSymbolsW) )
        return FALSE;

    ConvertNameToFullW (strFileName, strFullFileName);

    //Дабы избежать бардака, развернём слэшики...

    wchar_t *lpwszChar = strFullFileName.GetBuffer ();

    while ( *lpwszChar )
    {
        if ( *lpwszChar == L'/' )
            *lpwszChar = L'\\';

        lpwszChar++;
    }

    strFullFileName.ReleaseBuffer ();
  }
  else
  {
    strFullFileName = strStartDir;
    AddEndSlashW (strFullFileName);

    strFullFileName += strFileName;
  }

  return TRUE;
}

void FileEditor::SetTitle(const wchar_t *Title)
{
	strTitle = NullToEmptyW(Title);
}

void FileEditor::ChangeEditKeyBar()
{
/*  if (m_editor->AnsiText)
    EditKeyBar.Change(UMSG(Opt.OnlyEditorViewerUsed?MSingleEditF8DOS:MEditF8DOS),7);
  else*/
    EditKeyBar.Change(UMSG(Opt.OnlyEditorViewerUsed?MSingleEditF8:MEditF8),7);

  EditKeyBar.Redraw();
}

void FileEditor::GetTitle(string &strLocalTitle,int SubLen,int TruncSize)
{
  if ( !strPluginTitle.IsEmpty () )
    strLocalTitle = strPluginTitle;
  else
  {
    if ( !strTitle.IsEmpty () )
      strLocalTitle = strTitle;
    else
      strLocalTitle = strFullFileName;
  }
}

void FileEditor::ShowStatus()
{
  if ( m_editor->Locked () )
    return;

  SetColor(COL_EDITORSTATUS);

  GotoXY(X1,Y1); //??

  string strStatus, strLineStr;

  string strLocalTitle;
  GetTitle(strLocalTitle);

  int NameLength = Opt.ViewerEditorClock && Flags.Check(FFILEEDIT_FULLSCREEN) ? 19:25;

  if ( X2 > 80)
     NameLength += (X2-80);

  if ( !strPluginTitle.IsEmpty () || !strTitle.IsEmpty ())
    TruncPathStrW (strLocalTitle, (ObjWidth<NameLength?ObjWidth:NameLength));
  else
    TruncPathStrW (strLocalTitle, NameLength);

  //предварительный расчет
  strLineStr.Format (L"%d/%d", m_editor->NumLastLine, m_editor->NumLastLine);

  int SizeLineStr = strLineStr.GetLength();

  if( SizeLineStr > 12 )
    NameLength -= (SizeLineStr-12);
  else
    SizeLineStr = 12;

  strLineStr.Format (L"%d/%d", m_editor->NumLine+1, m_editor->NumLastLine);

  string strAttr;
  strAttr.SetData (AttrStr, CP_OEMCP);

  strStatus.Format(
        L"%-*s %c%c%c%d %7s %*.*s %5s %-4d %3s",
        NameLength,
        (const wchar_t*)strLocalTitle,
        (m_editor->Flags.Check(FEDITOR_MODIFIED) ? L'*':L' '),
        (m_editor->Flags.Check(FEDITOR_LOCKMODE) ? L'-':L' '),
        (m_editor->Flags.Check(FEDITOR_PROCESSCTRLQ) ? L'"':L' '),
        m_codepage,
        UMSG(MEditStatusLine),
        SizeLineStr,
        SizeLineStr,
        (const wchar_t*)strLineStr,
        UMSG(MEditStatusCol),
        m_editor->CurLine->GetTabCurPos()+1,
        (const wchar_t*)strAttr
        );

  int StatusWidth=ObjWidth - (Opt.ViewerEditorClock && Flags.Check(FFILEEDIT_FULLSCREEN)?5:0);

  if (StatusWidth<0)
    StatusWidth=0;

  mprintfW (L"%-*.*s", StatusWidth, StatusWidth, (const wchar_t*)strStatus);

  {
    const wchar_t *Str;
    int Length;
    m_editor->CurLine->GetBinaryStringW(&Str,NULL,Length);
    int CurPos=m_editor->CurLine->GetCurPos();
    if (CurPos<Length)
    {
      GotoXY(X2-(Opt.ViewerEditorClock && Flags.Check(FFILEEDIT_FULLSCREEN) ? 11:4),Y1);
      SetColor(COL_EDITORSTATUS);
      /* $ 27.02.2001 SVS
      Показываем в зависимости от базы */
      static wchar_t *FmtCharCode[3]={L"%05o",L"%5d",L"%04Xh"};
      mprintfW(FmtCharCode[m_editor->EdOpt.CharCodeBase%3],(wchar_t)Str[CurPos]);
      /* SVS $ */
    }
  }

  if (Opt.ViewerEditorClock && Flags.Check(FFILEEDIT_FULLSCREEN))
    ShowTime(FALSE);
}

/* $ 13.02.2001
     Узнаем атрибуты файла и заодно сформируем готовую строку атрибутов для
     статуса.
*/
DWORD FileEditor::GetFileAttributes(const wchar_t *Name)
{
  FileAttributes=::GetFileAttributesW(Name);
  int ind=0;
  if(0xFFFFFFFF!=FileAttributes)
  {
     if(FileAttributes&FILE_ATTRIBUTE_READONLY) AttrStr[ind++]='R';
     if(FileAttributes&FILE_ATTRIBUTE_SYSTEM) AttrStr[ind++]='S';
     if(FileAttributes&FILE_ATTRIBUTE_HIDDEN) AttrStr[ind++]='H';
  }
  AttrStr[ind]=0;
  return FileAttributes;
}
/* IS $ */

/* Return TRUE - панель обовили
*/
BOOL FileEditor::UpdateFileList()
{
  Panel *ActivePanel = CtrlObject->Cp()->ActivePanel;
  const wchar_t *FileName = PointToNameW(strFullFileName);
  string strFilePath, strPanelPath;

  wchar_t *lpwszFilePath = strFilePath.GetBuffer();

  xwcsncpy(lpwszFilePath, strFullFileName, (FileName - (const wchar_t*)strFullFileName)/sizeof(wchar_t));

  strFilePath.ReleaseBuffer();

  ActivePanel->GetCurDirW(strPanelPath);
  AddEndSlashW(strPanelPath);
  AddEndSlashW(strFilePath);
  if (!wcscmp(strPanelPath, strFilePath))
  {
    ActivePanel->Update(UPDATE_KEEP_SELECTION|UPDATE_DRAW_MESSAGE);
    return TRUE;
  }
  return FALSE;
}

void FileEditor::SetPluginData(const wchar_t *PluginData)
{
  FileEditor::strPluginData = NullToEmptyW(PluginData);
}

/* $ 14.06.2002 IS
   DeleteOnClose стал int:
     0 - не удалять ничего
     1 - удалять файл и каталог
     2 - удалять только файл
*/
void FileEditor::SetDeleteOnClose(int NewMode)
{
  Flags.Clear(FFILEEDIT_DELETEONCLOSE|FFILEEDIT_DELETEONLYFILEONCLOSE);
  if(NewMode==1)
    Flags.Set(FFILEEDIT_DELETEONCLOSE);
  else if(NewMode==2)
    Flags.Set(FFILEEDIT_DELETEONLYFILEONCLOSE);
}
/* IS $ */

void FileEditor::GetEditorOptions(struct EditorOptions& EdOpt)
{
  m_editor->EdOpt.CopyTo(EdOpt);
}

void FileEditor::SetEditorOptions(struct EditorOptions& EdOpt)
{
  m_editor->SetTabSize(EdOpt.TabSize);
  m_editor->SetConvertTabs(EdOpt.ExpandTabs);
  m_editor->SetPersistentBlocks(EdOpt.PersistentBlocks);
  m_editor->SetDelRemovesBlocks(EdOpt.DelRemovesBlocks);
  m_editor->SetAutoIndent(EdOpt.AutoIndent);
  m_editor->SetAutoDetectTable(EdOpt.AutoDetectTable);
  m_editor->SetCursorBeyondEOL(EdOpt.CursorBeyondEOL);
  m_editor->SetCharCodeBase(EdOpt.CharCodeBase);
  m_editor->SetSavePosMode(EdOpt.SavePos, EdOpt.SaveShortPos);
  //m_editor->SetBSLikeDel(EdOpt.BSLikeDel);
}

int FileEditor::EditorControl(int Command, void *Param)
{
#if defined(SYSLOG_KEYMACRO)
  _KEYMACRO(CleverSysLog SL(L"FileEditor::EditorControl()"));
  if(Command == ECTL_READINPUT || Command == ECTL_PROCESSINPUT)
  {
    _KEYMACRO(SysLog(L"(Command=%s, Param=[%d/0x%08X]) Macro.IsExecuting()=%d",_ECTL_ToName(Command),(int)Param,Param,CtrlObject->Macro.IsExecuting()));
  }
#else
  _ECTLLOG(CleverSysLog SL(L"FileEditor::EditorControl()"));
  _ECTLLOG(SysLog(L"(Command=%s, Param=[%d/0x%08X])",_ECTL_ToName(Command),(int)Param,Param));
#endif
	if ( m_bClosing && (Command != ECTL_GETINFO) && (Command != ECTL_GETBOOKMARKS) && (Command != ECTL_FREEINFO) )
		return FALSE;


	switch ( Command )
	{
		case ECTL_GETINFO:
		{
			if ( m_editor->EditorControl(Command, Param) )
			{
				EditorInfo *Info = (EditorInfo*)Param;
				Info->FileName = _wcsdup(strFullFileName); //BUGBUG
				return TRUE;
			}

			return FALSE;
		}

		case ECTL_FREEINFO:
		{
			EditorInfo *Info = (EditorInfo*)Param;
			xf_free ((void*)Info->FileName);

			return TRUE;
		}

		case ECTL_GETBOOKMARKS:
		{
			if( !Flags.Check(FFILEEDIT_OPENFAILED) && Param && !IsBadReadPtr(Param, sizeof(EditorBookMarks)) )
			{
				EditorBookMarks *ebm = (EditorBookMarks*)Param;

				if ( ebm->Line && !IsBadWritePtr(ebm->Line, BOOKMARK_COUNT*sizeof(long)) )
					memcpy(ebm->Line, m_editor->SavePos.Line, BOOKMARK_COUNT*sizeof(long));

				if ( ebm->Cursor && !IsBadWritePtr(ebm->Cursor, BOOKMARK_COUNT*sizeof(long)) )
					memcpy(ebm->Cursor,m_editor->SavePos.Cursor,BOOKMARK_COUNT*sizeof(long));

				if ( ebm->ScreenLine && !IsBadWritePtr(ebm->ScreenLine, BOOKMARK_COUNT*sizeof(long)) )
					memcpy(ebm->ScreenLine, m_editor->SavePos.ScreenLine, BOOKMARK_COUNT*sizeof(long));

				if ( ebm->LeftPos && !IsBadWritePtr(ebm->LeftPos, BOOKMARK_COUNT*sizeof(long)) )
					memcpy(ebm->LeftPos, m_editor->SavePos.LeftPos, BOOKMARK_COUNT*sizeof(long));

				return TRUE;
			}

			return FALSE;
		}

		case ECTL_SETTITLE:
		{
			strPluginTitle = NullToEmptyW((const wchar_t*)Param);

			ShowStatus();
			ScrBuf.Flush(); //???
			return TRUE;
		}

		case ECTL_EDITORTOOEM:
		{
			if(!Param || IsBadReadPtr(Param,sizeof(struct EditorConvertText)))
				return FALSE;

			/*struct EditorConvertText *ect=(struct EditorConvertText *)Param;
			_ECTLLOG(SysLog(L"struct EditorConvertText{"));
			_ECTLLOG(SysLog(L"  Text       ='%s'",ect->Text));
			_ECTLLOG(SysLog(L"  TextLength =%d",ect->TextLength));
			_ECTLLOG(SysLog(L"}"));
			if (m_editor->UseDecodeTable && ect->Text)
			{
				DecodeString(ect->Text,(unsigned char *)m_editor->TableSet.DecodeTable,ect->TextLength);
				_ECTLLOG(SysLog(L"DecodeString -> ect->Text='%s'",ect->Text));
			}*/ //BUGBUG
			return TRUE;
		}

		case ECTL_OEMTOEDITOR:
		{
			if(!Param || IsBadReadPtr(Param,sizeof(struct EditorConvertText)))
				return FALSE;

			/*struct EditorConvertText *ect=(struct EditorConvertText *)Param;
			_ECTLLOG(SysLog(L"struct EditorConvertText{"));
			_ECTLLOG(SysLog(L"  Text       ='%s'",ect->Text));
			_ECTLLOG(SysLog(L"  TextLength =%d",ect->TextLength));
			_ECTLLOG(SysLog(L"}"));
			if (m_editor->UseDecodeTable && ect->Text)
			{
				EncodeString(ect->Text,(unsigned char *)m_editor->TableSet.EncodeTable,ect->TextLength);
				_ECTLLOG(SysLog(L"EncodeString -> ect->Text='%s'",ect->Text));
			}*/ //BUGBUG
			return TRUE;
		}

		case ECTL_REDRAW:
		{
			FileEditor::DisplayObject();
			ScrBuf.Flush();

			return TRUE;
		}

		/*
			Функция установки Keybar Labels
			Param = NULL - восстановить, пред. значение
			Param = -1   - обновить полосу (перерисовать)
			Param = KeyBarTitles
		*/
		case ECTL_SETKEYBAR:
		{
			KeyBarTitles *Kbt = (KeyBarTitles*)Param;

            if ( !Kbt ) //восстановить изначальное
				InitKeyBar();
			else
			{
				if ( ((LONG_PTR)Param != (LONG_PTR)-1) && !IsBadReadPtr(Param, sizeof(KeyBarTitles)) ) // не только перерисовать?
				{
					for (int I = 0; I < 12; ++I)
					{
						if(Kbt->Titles[I])
							EditKeyBar.Change(KBL_MAIN,Kbt->Titles[I],I);

						if(Kbt->CtrlTitles[I])
							EditKeyBar.Change(KBL_CTRL,Kbt->CtrlTitles[I],I);

						if(Kbt->AltTitles[I])
							EditKeyBar.Change(KBL_ALT,Kbt->AltTitles[I],I);

						if(Kbt->ShiftTitles[I])
							EditKeyBar.Change(KBL_SHIFT,Kbt->ShiftTitles[I],I);

						if(Kbt->CtrlShiftTitles[I])
							EditKeyBar.Change(KBL_CTRLSHIFT,Kbt->CtrlShiftTitles[I],I);

						if(Kbt->AltShiftTitles[I])
							EditKeyBar.Change(KBL_ALTSHIFT,Kbt->AltShiftTitles[I],I);

						if(Kbt->CtrlAltTitles[I])
							EditKeyBar.Change(KBL_CTRLALT,Kbt->CtrlAltTitles[I],I);
					}
				}

				EditKeyBar.Show();
			}
			return TRUE;
		}
    /* SVS $ */

    case ECTL_SAVEFILE:
    {
      EditorSaveFile *esf=(EditorSaveFile *)Param;
      string strName = strFullFileName;
      int EOL=0;
      if (esf && !IsBadReadPtr(esf,sizeof(EditorSaveFile)))
      {
        if (*esf->FileName)
          strName=esf->FileName;
        if (esf->FileEOL!=NULL)
        {
          if (wcscmp(esf->FileEOL,DOS_EOL_fmtW)==0)
            EOL=1;
          if (wcscmp(esf->FileEOL,UNIX_EOL_fmtW)==0)
            EOL=2;
          if (wcscmp(esf->FileEOL,MAC_EOL_fmtW)==0)
            EOL=3;
        }
      }

      {
        string strOldFullFileName = strFullFileName;

        if(SetFileName(strName))
          return SaveFile(strName,FALSE,EOL,!LocalStricmpW(strName, strOldFullFileName));
      }
      return FALSE;
    }

    case ECTL_QUIT:
    {
      FrameManager->DeleteFrame(this);
      SetExitCode(SAVEFILE_ERROR); // что-то меня терзают смутные сомнения ...???
      return(TRUE);
    }

    case ECTL_READINPUT:
    {
      if(CtrlObject->Macro.IsRecording() == MACROMODE_RECORDING || CtrlObject->Macro.IsExecuting() == MACROMODE_EXECUTING)
      {
//        return FALSE;
      }

      if(!Param || IsBadReadPtr(Param,sizeof(INPUT_RECORD)))
        return FALSE;
      else
      {
        INPUT_RECORD *rec=(INPUT_RECORD *)Param;
        DWORD Key;
        while(1)
        {
          Key=GetInputRecord(rec);
          if((!rec->EventType || rec->EventType == KEY_EVENT || rec->EventType == FARMACRO_KEY_EVENT) && Key >= KEY_MACRO_BASE && Key <= KEY_MACRO_ENDBASE) // исключаем MACRO
             ReProcessKey(Key);
          else
            break;
        }
        //if(Key==KEY_CONSOLE_BUFFER_RESIZE) //????
        //  Show();                          //????
#if defined(SYSLOG_KEYMACRO)
        if(rec->EventType == KEY_EVENT)
        {
          SysLog(L"ECTL_READINPUT={%s,{%d,%d,Vk=0x%04X,0x%08X}}",
                             (rec->EventType == FARMACRO_KEY_EVENT?"FARMACRO_KEY_EVENT":"KEY_EVENT"),
                             rec->Event.KeyEvent.bKeyDown,
                             rec->Event.KeyEvent.wRepeatCount,
                             rec->Event.KeyEvent.wVirtualKeyCode,
                             rec->Event.KeyEvent.dwControlKeyState);
        }
#endif
      }
      return(TRUE);
    }

    case ECTL_PROCESSINPUT:
    {
      if(!Param || IsBadReadPtr(Param,sizeof(INPUT_RECORD)))
        return FALSE;
      else
      {
        INPUT_RECORD *rec=(INPUT_RECORD *)Param;
        if (ProcessEditorInput(rec))
          return(TRUE);
        if (rec->EventType==MOUSE_EVENT)
          ProcessMouse(&rec->Event.MouseEvent);
        else
        {
#if defined(SYSLOG_KEYMACRO)
          if(!rec->EventType || rec->EventType == KEY_EVENT || rec->EventType == FARMACRO_KEY_EVENT)
          {
            SysLog(L"ECTL_PROCESSINPUT={%s,{%d,%d,Vk=0x%04X,0x%08X}}",
                             (rec->EventType == FARMACRO_KEY_EVENT?"FARMACRO_KEY_EVENT":"KEY_EVENT"),
                             rec->Event.KeyEvent.bKeyDown,
                             rec->Event.KeyEvent.wRepeatCount,
                             rec->Event.KeyEvent.wVirtualKeyCode,
                             rec->Event.KeyEvent.dwControlKeyState);
          }
#endif
          int Key=CalcKeyCode(rec,FALSE);
          ReProcessKey(Key);
        }
      }
      return(TRUE);
    }

    case ECTL_PROCESSKEY:
    {
      ReProcessKey((int)(INT_PTR)Param);
      return TRUE;
    }

  }

  return m_editor->EditorControl(Command,Param);
}

bool FileEditor::LoadFromCache (EditorCacheParams *pp)
{
	memset (pp, 0, sizeof (EditorCacheParams));

	string strCacheName;

	if ( *GetPluginData())
		strCacheName.Format (L"%s%s", GetPluginData(), (const wchar_t*)PointToNameW(strFullFileName));
	else
	{
		strCacheName = strFullFileName;

		wchar_t *lpwszCacheName = strCacheName.GetBuffer();

		for(int i=0;lpwszCacheName[i];i++)
		{
			if(lpwszCacheName[i]==L'/')
				lpwszCacheName[i]=L'\\';
		}

		strCacheName.ReleaseBuffer();
	}


	unsigned int Table;

	TPosCache32 PosCache={0};

	if ( CtrlObject->EditorPosCache->GetPosition(
			strCacheName,
			&PosCache
			) )
	{
		pp->Line=PosCache.Param[0];
		pp->ScreenLine=PosCache.Param[1];
		pp->LinePos=PosCache.Param[2];
		pp->LeftPos=PosCache.Param[3];
		pp->Table=PosCache.Param[4];

		if((int)pp->Line < 0) pp->Line=0;
		if((int)pp->ScreenLine < 0) pp->ScreenLine=0;
		if((int)pp->LinePos < 0) pp->LinePos=0;
		if((int)pp->LeftPos < 0) pp->LeftPos=0;
		if((int)pp->Table < 0) pp->Table=0;

		return true;
	}

	return false;
}

void FileEditor::SaveToCache ()
{
	EditorCacheParams cp;

	m_editor->GetCacheParams (&cp);

	string strCacheName;

	if ( !strPluginData.IsEmpty() )
		strCacheName.Format (L"%s%s",(const wchar_t*)strPluginData,PointToNameW(strFullFileName));
	else
		strCacheName = strFullFileName;

    if ( !Flags.Check(FFILEEDIT_OPENFAILED) ) //????
	{
		TPosCache32 PosCache = {0};

		PosCache.Param[0] = cp.Line;
		PosCache.Param[1] = cp.ScreenLine;
		PosCache.Param[2] = cp.LinePos;
		PosCache.Param[3] = cp.LeftPos;
		PosCache.Param[4] = cp.Table;

		//if no position saved these are nulls
		PosCache.Position[0] = cp.SavePos.Line;
		PosCache.Position[1] = cp.SavePos.Cursor;
		PosCache.Position[2] = cp.SavePos.ScreenLine;
		PosCache.Position[3] = cp.SavePos.LeftPos;

		CtrlObject->EditorPosCache->AddPosition(strCacheName, &PosCache);
	}
}

void FileEditor::SetCodePage(int codepage)
{
	if ( codepage != m_codepage )
	{
		m_codepage = codepage;

		if ( m_editor )
		{
			m_bSignatureFound = m_editor->SetCodePage (m_codepage);
			ChangeEditKeyBar(); //???
		}
	}
}
