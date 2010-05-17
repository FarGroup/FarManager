/*
manager.cpp

Переключение между несколькими file panels, viewers, editors, dialogs

*/

#include "headers.hpp"
#pragma hdrstop

#include "manager.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "frame.hpp"
#include "vmenu.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "savescr.hpp"
#include "cmdline.hpp"
#include "ctrlobj.hpp"

Manager *FrameManager;

Manager::Manager()
{
	_MANAGERLOG(CleverSysLog Clev("Manager::Manager()"));
	FrameList=NULL;
	FrameCount=FrameListSize=0;
	FramePos=-1;
	ModalStack=NULL;
	FrameList=(Frame **)xf_realloc(FrameList,sizeof(*FrameList)*(FrameCount+1));
	ModalStack=NULL;
	ModalStackSize = ModalStackCount = 0;
	EndLoop = FALSE;
	RefreshedFrame=NULL;
	CurrentFrame  = NULL;
	InsertedFrame = NULL;
	DeletedFrame  = NULL;
	ActivatedFrame= NULL;
	DeactivatedFrame=NULL;
	ModalizedFrame=NULL;
	UnmodalizedFrame=NULL;
	ExecutedFrame=NULL;
	//SemiModalBackFrames=NULL; //Теперь это массив
	//SemiModalBackFramesCount=0;
	//SemiModalBackFramesSize=0;
	ModalEVCount=0;
	StartManager=FALSE;
}

Manager::~Manager()
{
	_MANAGERLOG(CleverSysLog Clev("Manager::~Manager()"));

	if (FrameList)
		xf_free(FrameList);

	if (ModalStack)
		xf_free(ModalStack);

	/*if (SemiModalBackFrames)
	  xf_free(SemiModalBackFrames);*/
}


/* $ 29.12.2000 IS
  Аналог CloseAll, но разрешает продолжение полноценной работы в фаре,
  если пользователь продолжил редактировать файл.
  Возвращает TRUE, если все закрыли и можно выходить из фара.
*/
BOOL Manager::ExitAll()
{
	int i;

	for (i=this->ModalStackCount-1; i>=0; i--)
	{
		Frame *iFrame=this->ModalStack[i];

		if (!iFrame->GetCanLoseFocus(TRUE))
		{
			int PrevFrameCount=ModalStackCount;
			iFrame->ProcessKey(KEY_ESC);
			Commit();

			if (PrevFrameCount==ModalStackCount)
			{
				return FALSE;
			}
		}
	}

	for (i=FrameCount-1; i>=0; i--)
	{
		Frame *iFrame=FrameList[i];

		if (!iFrame->GetCanLoseFocus(TRUE))
		{
			ActivateFrame(iFrame);
			Commit();
			int PrevFrameCount=FrameCount;
			iFrame->ProcessKey(KEY_ESC);
			Commit();

			if (PrevFrameCount==FrameCount)
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

void Manager::CloseAll()
{
	int i;
	Frame *iFrame;

	for (i=ModalStackCount-1; i>=0; i--)
	{
		iFrame=ModalStack[i];
		DeleteFrame(iFrame);
		DeleteCommit();
		DeletedFrame=NULL;
	}

	for (i=FrameCount-1; i>=0; i--)
	{
		iFrame=(*this)[i];
		DeleteFrame(iFrame);
		DeleteCommit();
		DeletedFrame=NULL;
	}

	xf_free(FrameList);
	FrameList=NULL;
	FrameCount=FramePos=0;
}

BOOL Manager::IsAnyFrameModified(int Activate)
{
	for (int I=0; I<FrameCount; I++)
		if (FrameList[I]->IsFileModified())
		{
			if (Activate)
			{
				ActivateFrame(I);
				Commit();
			}

			return(TRUE);
		}

	return(FALSE);
}

void Manager::InsertFrame(Frame *Inserted, int Index)
{
	_MANAGERLOG(CleverSysLog Clev("Manager::InsertFrame(Frame *Inserted, int Index)"));
	_MANAGERLOG(SysLog("Inserted=%p (%s), Index=%i",Inserted,(Inserted?Inserted->GetTypeName():NULL), Index));

	if (Index==-1)
		Index=FramePos;

	InsertedFrame=Inserted;
}

void Manager::DeleteFrame(Frame *Deleted)
{
	_MANAGERLOG(CleverSysLog Clev("Manager::DeleteFrame(Frame *Deleted)"));
	_MANAGERLOG(SysLog("Deleted=%p (%s)",Deleted,(Deleted?Deleted->GetTypeName():NULL)));

	for (int i=0; i<FrameCount; i++)
	{
		Frame *iFrame=FrameList[i];

		if (iFrame->RemoveModal(Deleted))
		{
			return;
		}
	}

	if (!Deleted)
	{
		DeletedFrame=CurrentFrame;
	}
	else
	{
		DeletedFrame=Deleted;
	}
}

void Manager::DeleteFrame(int Index)
{
	_MANAGERLOG(CleverSysLog Clev("Manager::DeleteFrame(int Index)"));
	_MANAGERLOG(SysLog("Index=%i",Index));
	DeleteFrame(this->operator[](Index));
}


void Manager::ModalizeFrame(Frame *Modalized, int Mode)
{
	_MANAGERLOG(CleverSysLog Clev("Manager::ModalizeFrame (Frame *Modalized, int Mode)"));
	_MANAGERLOG(SysLog("Modalized=%p (%s), Mode=%d",Modalized,(Modalized?Modalized->GetTypeName():NULL),Mode));
	ModalizedFrame=Modalized;
	ModalizeCommit();
}

void Manager::UnmodalizeFrame(Frame *Unmodalized)
{
	_MANAGERLOG(CleverSysLog Clev("Manager::UnmodalizeFrame (Frame *Unmodalized)"));
	_MANAGERLOG(SysLog("Unmodalized=%p (%s)",Unmodalized,(Unmodalized?Unmodalized->GetTypeName():NULL)));
	UnmodalizedFrame=Unmodalized;
	UnmodalizeCommit();
}

void Manager::ExecuteNonModal()
{
	_MANAGERLOG(CleverSysLog Clev("Manager::ExecuteNonModal ()"));
	_MANAGERLOG(SysLog("ExecutedFrame=%p (%s), InsertedFrame=%p (%s), DeletedFrame=%p (%s)",ExecutedFrame,(ExecutedFrame?ExecutedFrame->GetTypeName():NULL), InsertedFrame,(InsertedFrame?InsertedFrame->GetTypeName():NULL), DeletedFrame,(DeletedFrame?DeletedFrame->GetTypeName():NULL)));
	Frame *NonModal=InsertedFrame?InsertedFrame:(ExecutedFrame?ExecutedFrame:ActivatedFrame);

	if (!NonModal)
	{
		return;
	}

	/* $ 14.05.2002 SKV
	  Положим текущий фрэйм в список "родителей" полумодальных фрэймов
	*/
	//Frame *SaveFrame=CurrentFrame;
	//AddSemiModalBackFrame(SaveFrame);
	/* SKV $ */
	int NonModalIndex=IndexOf(NonModal);

	if (-1==NonModalIndex)
	{
		InsertedFrame=NonModal;
		ExecutedFrame=NULL;
		InsertCommit();
		InsertedFrame=NULL;
	}
	else
	{
		ActivateFrame(NonModalIndex);
	}

	//Frame* ModalStartLevel=NonModal;
	while (1)
	{
		Commit();

		if (CurrentFrame!=NonModal)
		{
			break;
		}

		ProcessMainLoop();
	}

	//ExecuteModal(NonModal);
	/* $ 14.05.2002 SKV
	  ... и уберём его же.
	*/
	//RemoveSemiModalBackFrame(SaveFrame);
	/* SKV $ */
}

void Manager::ExecuteModal(Frame *Executed)
{
	_MANAGERLOG(CleverSysLog Clev("Manager::ExecuteModal (Frame *Executed)"));
	_MANAGERLOG(SysLog("Executed=%p (%s), ExecutedFrame=%p (%s)",Executed,(Executed?Executed->GetTypeName():NULL),ExecutedFrame,(ExecutedFrame?ExecutedFrame->GetTypeName():NULL)));

	if (!Executed && !ExecutedFrame)
	{
		return;
	}

	if (Executed)
	{
		if (ExecutedFrame)
		{
			_MANAGERLOG(SysLog("Попытка в одном цикле запустить в модальном режиме два фрейма. Executed=%p, ExecitedFrame=%p",Executed, ExecutedFrame));
			return;// NULL; //?? Определить, какое значение правильно возвращать в этом случае
		}
		else
		{
			ExecutedFrame=Executed;
		}
	}

	int ModalStartLevel=ModalStackCount;
	int OriginalStartManager=StartManager;
	StartManager=TRUE;

	while (1)
	{
		Commit();

		if (ModalStackCount<=ModalStartLevel)
		{
			break;
		}

		ProcessMainLoop();
	}

	StartManager=OriginalStartManager;
	return;// GetModalExitCode();
}

int Manager::GetModalExitCode()
{
	return ModalExitCode;
}

/* $ 11.10.2001 IS
   Подсчитать количество фреймов с указанным именем.
*/
int Manager::CountFramesWithName(const char *Name, BOOL IgnoreCase)
{
	int Counter=0;
	typedef int (__cdecl *cmpfunc_t)(const char *s1, const char *s2);
	cmpfunc_t cmpfunc=IgnoreCase?(cmpfunc_t)LocalStricmp:(cmpfunc_t)strcmp;
	char Type[200],curName[NM];

	for (int I=0; I<FrameCount; I++)
	{
		FrameList[I]->GetTypeAndName(Type,curName);

		if (!cmpfunc(Name, curName)) ++Counter;
	}

	return Counter;
}
/* IS $ */

/*!
  \return Возвращает NULL если нажат "отказ" или если нажат текущий фрейм.
  Другими словами, если немодальный фрейм не поменялся.
  Если же фрейм поменялся, то тогда функция должна возвратить
  указатель на предыдущий фрейм.
*/
Frame *Manager::FrameMenu()
{
	/* $ 28.04.2002 KM
	    Флаг для определения того, что меню переключения
	    экранов уже активировано.
	*/
	static int AlreadyShown=FALSE;

	if (AlreadyShown)
		return NULL;

	/* KM $ */
	int ExitCode, CheckCanLoseFocus=CurrentFrame->GetCanLoseFocus();
	{
		struct MenuItem ModalMenuItem;
		memset(&ModalMenuItem,0,sizeof(ModalMenuItem));
		VMenu ModalMenu(MSG(MScreensTitle),NULL,0,ScrY-4);
		ModalMenu.SetHelp("ScrSwitch");
		ModalMenu.SetFlags(VMENU_WRAPMODE);
		ModalMenu.SetPosition(-1,-1,0,0);

		if (!CheckCanLoseFocus)
			ModalMenuItem.SetDisable(TRUE);

		for (int I=0; I<FrameCount; I++)
		{
			char Type[200],Name[NM*2],NumText[100];
			FrameList[I]->GetTypeAndName(Type,Name);

			/* $ 07.07.2001 IS
			   Если фреймов больше 10, то используем для горячих клавиш буквы
			   латинского алфавита, т.о. получаем всего не 10, а 36 горячих клавиш.
			*/
			if (I<10)
				sprintf(NumText,"&%d. ",I);
			else if (I<36)
				sprintf(NumText,"&%c. ",I+55); // 55='A'-10
			else
				strcpy(NumText,"&   ");

			/* IS $ */
			/* $ 28.07.2000 tran
			   файл усекает по ширине экрана */
			TruncPathStr(Name,Min((int)ScrX,(int)(sizeof(ModalMenuItem.Name)-1))-24);
			ReplaceStrings(Name,"&","&&",-1);
			/*  добавляется "*" если файл изменен */
			sprintf(ModalMenuItem.Name,"%s%-10.10s %c %s",NumText,Type,(FrameList[I]->IsFileModified()?'*':' '),Name);
			/* tran 28.07.2000 $ */
			ModalMenuItem.SetSelect(I==FramePos);
			ModalMenu.AddItem(&ModalMenuItem);
		}

		/* $ 28.04.2002 KM */
		AlreadyShown=TRUE;
		ModalMenu.Process();
		AlreadyShown=FALSE;
		/* KM $ */
		ExitCode=ModalMenu.Modal::GetExitCode();
	}

	if (CheckCanLoseFocus)
	{
		if (ExitCode>=0)
		{
			ActivateFrame(ExitCode);
			return (ActivatedFrame==CurrentFrame || !CurrentFrame->GetCanLoseFocus()?NULL:CurrentFrame);
		}

		return (ActivatedFrame==CurrentFrame?NULL:CurrentFrame);
	}

	return NULL;
}


int Manager::GetFrameCountByType(int Type)
{
	int ret=0;

	for (int I=0; I<FrameCount; I++)
	{
		/* $ 10.05.2001 DJ
		   не учитываем фрейм, который собираемся удалять
		*/
		if (FrameList[I] == DeletedFrame || FrameList [I]->GetExitCode() == XC_QUIT)
			continue;

		/* DJ $ */
		if (FrameList[I]->GetType()==Type)
			ret++;
	}

	return ret;
}

void Manager::SetFramePos(int NewPos)
{
	_MANAGERLOG(CleverSysLog Clev("Manager::SetFramePos(int NewPos)"));
	_MANAGERLOG(SysLog("NewPos=%i",NewPos));
	FramePos=NewPos;
}

/*$ 11.05.2001 OT Теперь можно искать файл не только по полному имени, но и отдельно - путь, отдельно имя */
int  Manager::FindFrameByFile(int ModalType,char *FileName,char *Dir)
{
	_MANAGERLOG(CleverSysLog Clev("Manager::FindFrameByFile(int ModalType,char *FileName,char *Dir)"));
	_MANAGERLOG(SysLog("ModalType=%d ,FileName='%s', Dir='%s'",ModalType,FileName,Dir));
	char bufFileName[NM*2];
	char *FullFileName=FileName;

	if (Dir)
	{
		strcpy(bufFileName,Dir);
		AddEndSlash(bufFileName);
		strcat(bufFileName,FileName);
		FullFileName=bufFileName;
	}

	for (int I=0; I<FrameCount; I++)
	{
		char Type[200],Name[NM];

		// Mantis#0000469 - получать Name будем только при совпадении ModalType
		if (FrameList[I]->GetType()==ModalType)
		{
			FrameList[I]->GetTypeAndName(Type,Name);

			if (LocalStricmp(Name,FullFileName)==0)
				return(I);
		}
	}

	return(-1);
}
/* 11.05.2001 OT $*/

BOOL Manager::ShowBackground()
{
	_MANAGERLOG(CleverSysLog Clev("Manager::ShowBackground()"));

	CtrlObject->CmdLine->ShowBackground();
	return TRUE;
}


void Manager::ActivateFrame(Frame *Activated)
{
	_MANAGERLOG(CleverSysLog Clev("Manager::ActivateFrame(Frame *Activated)"));
	_MANAGERLOG(SysLog("Activated=%p (%s)",Activated,(Activated?Activated->GetTypeName():NULL)));

	if (IndexOf(Activated)==-1 && IndexOfStack(Activated)==-1)
		return;

	if (!ActivatedFrame)
	{
		ActivatedFrame=Activated;
	}
}

void Manager::ActivateFrame(int Index)
{
	_MANAGERLOG(CleverSysLog Clev("Manager::ActivateFrame(int Index)"));
	_MANAGERLOG(SysLog("Index=%i",Index));
	ActivateFrame((*this)[Index]);
}

void Manager::DeactivateFrame(Frame *Deactivated,int Direction)
{
	_MANAGERLOG(CleverSysLog Clev("Manager::DeactivateFrame (Frame *Deactivated,int Direction)"));
	_MANAGERLOG(SysLog("Deactivated=%p (%s), Direction=%d",Deactivated,(Deactivated?Deactivated->GetTypeName():NULL),Direction));

	if (Direction)
	{
		FramePos+=Direction;

		if (Direction>0)
		{
			if (FramePos>=FrameCount)
			{
				FramePos=0;
			}
		}
		else
		{
			if (FramePos<0)
			{
				FramePos=FrameCount-1;
			}
		}

		ActivateFrame(FramePos);
	}
	else
	{
		// Direction==0
		// Direct access from menu or (in future) from plugin
	}

	DeactivatedFrame=Deactivated;
}

void Manager::SwapTwoFrame(int Direction)
{
	_MANAGERLOG(CleverSysLog Clev("Manager::SwapTwoFrame (int Direction)"));
	_MANAGERLOG(SysLog("Direction=%d",Direction));

	if (Direction)
	{
		int OldFramePos=FramePos;
		FramePos+=Direction;

		if (Direction>0)
		{
			if (FramePos>=FrameCount)
			{
				FramePos=0;
			}
		}
		else
		{
			if (FramePos<0)
			{
				FramePos=FrameCount-1;
			}
		}

		Frame *TmpFrame=FrameList[OldFramePos];
		FrameList[OldFramePos]=FrameList[FramePos];
		FrameList[FramePos]=TmpFrame;
		ActivateFrame(OldFramePos);
	}

	DeactivatedFrame=CurrentFrame;
}

void Manager::RefreshFrame(Frame *Refreshed)
{
	_MANAGERLOG(CleverSysLog Clev("Manager::RefreshFrame(Frame *Refreshed)"));
	_MANAGERLOG(SysLog("Refreshed=%p (%s)",Refreshed,(Refreshed?Refreshed->GetTypeName():NULL)));

	if (ActivatedFrame)
		return;

	if (Refreshed)
	{
		RefreshedFrame=Refreshed;
	}
	else
	{
		RefreshedFrame=CurrentFrame;
	}

	if (IndexOf(Refreshed)==-1 && IndexOfStack(Refreshed)==-1)
		return;

	/* $ 13.04.2002 KM
	  - Вызываем принудительный Commit() для фрейма имеющего члена
	    NextModal, это означает что активным сейчас является
	    VMenu, а значит Commit() сам не будет вызван после возврата
	    из функции.
	    Устраняет ещё один момент неперерисовки, когда один над
	    другим находится несколько объектов VMenu. Пример:
	    настройка цветов. Теперь AltF9 в диалоге настройки
	    цветов корректно перерисовывает меню.
	*/
	if (RefreshedFrame && RefreshedFrame->NextModal)
		Commit();

	/* KM $ */
}

void Manager::RefreshFrame(int Index)
{
	_MANAGERLOG(CleverSysLog Clev("Manager::RefreshFrame(int Index)"));
	_MANAGERLOG(SysLog("Index=%d",Index));
	RefreshFrame((*this)[Index]);
}

void Manager::ExecuteFrame(Frame *Executed)
{
	_MANAGERLOG(CleverSysLog Clev("Manager::ExecuteFrame(Frame *Executed)"));
	_MANAGERLOG(SysLog("Executed=%p (%s)",Executed,(Executed?Executed->GetTypeName():NULL)));
	ExecutedFrame=Executed;
}


/* $ 10.05.2001 DJ
   переключается на панели (фрейм с номером 0)
*/

void Manager::SwitchToPanels()
{
	_MANAGERLOG(CleverSysLog Clev("Manager::SwitchToPanels()"));
	ActivateFrame(0);
}

/* DJ $ */


int Manager::HaveAnyFrame()
{
	if (FrameCount || InsertedFrame || DeletedFrame || ActivatedFrame || RefreshedFrame ||
	        ModalizedFrame || DeactivatedFrame || ExecutedFrame || CurrentFrame)
		return 1;

	return 0;
}

void Manager::EnterMainLoop()
{
	_MANAGERLOG(CleverSysLog Clev("Manager::EnterMainLoop()"));
	WaitInFastFind=0;
	StartManager=TRUE;

	while (1)
	{
		Commit();

		if (EndLoop || !HaveAnyFrame())
		{
			break;
		}

		ProcessMainLoop();
	}
}

void Manager::ProcessMainLoop()
{
	CtrlObject->Macro.SetMode(CurrentFrame->GetMacroMode());
	// Mantis#0000073: Не работает автоскролинг в QView
	WaitInMainLoop=IsPanelsActive() && ((FilePanels*)CurrentFrame)->ActivePanel->GetType()!=QVIEW_PANEL;
	//WaitInFastFind++;
	int Key=GetInputRecord(&LastInputRecord);
	//WaitInFastFind--;
	WaitInMainLoop=FALSE;

	if (EndLoop)
		return;

	if (LastInputRecord.EventType==MOUSE_EVENT)
		ProcessMouse(&LastInputRecord.Event.MouseEvent);
	else
		ProcessKey(Key);
}

void Manager::ExitMainLoop(int Ask)
{
	if (CloseFAR)
	{
		CloseFAR=FALSE;
		CloseFARMenu=TRUE;
	};

	if (!Ask || !Opt.Confirm.Exit || Message(0,2,MSG(MQuit),MSG(MAskQuit),MSG(MYes),MSG(MNo))==0)

		/* $ 29.12.2000 IS
		   + Проверяем, сохранены ли все измененные файлы. Если нет, то не выходим
		     из фара.
		*/
		if (ExitAll())
		{
			//TODO: при закрытии по x нужно делать форсированный выход. Иначе могут быть
			//     глюки, например, при перезагрузке
			/* IS $ */
			FilePanels *cp;

			if ((cp = CtrlObject->Cp()) == NULL
			        || (!cp->LeftPanel->ProcessPluginEvent(FE_CLOSE,NULL) && !cp->RightPanel->ProcessPluginEvent(FE_CLOSE,NULL)))
				EndLoop=TRUE;
		}
		else
		{
			CloseFARMenu=FALSE;
		}
}

#if defined(FAR_ALPHA_VERSION)
#include <float.h>
#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4717)
//#ifdef __cplusplus
//#if defined(_MSC_VER < 1500) // TODO: See REMINDER file, section intrin.h
#ifndef _M_IA64
extern "C" void __ud2(void);
#else
extern "C" void __setReg(int, unsigned __int64);
#endif
//#endif                       // TODO: See REMINDER file, section intrin.h
//#endif
#endif
static void Test_EXCEPTION_STACK_OVERFLOW(char* target)
{
	char Buffer[1024]; /* чтобы быстрее рвануло */
	strcpy(Buffer, "zzzz");
	Test_EXCEPTION_STACK_OVERFLOW(Buffer);
}
#if defined(_MSC_VER)
#pragma warning( pop )
#endif
#endif


int  Manager::ProcessKey(DWORD Key)
{
	int ret=FALSE;

	if (CurrentFrame)
	{
		int i=0;
		DWORD KeyM=(Key&(~KEY_CTRLMASK));

		if (!(KeyM >= KEY_MACRO_BASE && KeyM <= KEY_MACRO_ENDBASE || KeyM >= KEY_OP_BASE && KeyM <= KEY_OP_ENDBASE)) // пропустим макро-коды
		{
			switch (CurrentFrame->GetType())
			{
				case MODALTYPE_PANELS:
				{
					_ALGO(CleverSysLog clv("Manager::ProcessKey()"));
					_ALGO(SysLog("Key=%s",_FARKEY_ToName(Key)));

					if (CtrlObject->Cp()->ActivePanel->SendKeyToPlugin(Key,TRUE))
						return TRUE;

					break;
				}
				case MODALTYPE_VIEWER:
					//if(((FileViewer*)CurrentFrame)->ProcessViewerInput(FrameManager->GetLastInputRecord()))
					//  return TRUE;
					break;
				case MODALTYPE_EDITOR:
					//if(((FileEditor*)CurrentFrame)->ProcessEditorInput(FrameManager->GetLastInputRecord()))
					//  return TRUE;
					break;
				case MODALTYPE_DIALOG:
					//((Dialog*)CurrentFrame)->CallDlgProc(DN_KEY,((Dialog*)CurrentFrame)->GetDlgFocusPos(),Key);
					break;
				case MODALTYPE_VMENU:
				case MODALTYPE_HELP:
				case MODALTYPE_COMBOBOX:
				case MODALTYPE_USER:
				case MODALTYPE_FINDFOLDER:
				default:
					break;
			}
		}

#if defined(FAR_ALPHA_VERSION)

// сей код для проверки исключатора, просьба не трогать :-)
		if (Key == (KEY_APPS|KEY_CTRL|KEY_ALT) && GetRegKey("System\\Exception","Used",0))
		{
			struct __ECODE
			{
				DWORD Code;
				char *Name;
			} ECode[]=
			{
				{EXCEPTION_ACCESS_VIOLATION,"Access Violation (Read)"},
				{EXCEPTION_ACCESS_VIOLATION,"Access Violation (Write)"},
				{EXCEPTION_INT_DIVIDE_BY_ZERO,"Divide by zero"},
				{EXCEPTION_ILLEGAL_INSTRUCTION,"Illegal instruction"},
				{EXCEPTION_STACK_OVERFLOW,"Stack Overflow"},
				{EXCEPTION_FLT_DIVIDE_BY_ZERO,"Floating-point divide by zero"},
#ifdef _M_IA64
				{EXCEPTION_DATATYPE_MISALIGNMENT,"Alignment fault (IA64 specific)",},
#endif
				/*
				        {EXCEPTION_FLT_OVERFLOW,"EXCEPTION_FLT_OVERFLOW"},
				        {EXCEPTION_BREAKPOINT,"EXCEPTION_BREAKPOINT",},
				        {EXCEPTION_SINGLE_STEP,"EXCEPTION_SINGLE_STEP",},
				        {EXCEPTION_ARRAY_BOUNDS_EXCEEDED,"EXCEPTION_ARRAY_BOUNDS_EXCEEDED",},
				        {EXCEPTION_FLT_DENORMAL_OPERAND,"EXCEPTION_FLT_DENORMAL_OPERAND",},
				        {EXCEPTION_FLT_INEXACT_RESULT,"EXCEPTION_FLT_INEXACT_RESULT",},
				        {EXCEPTION_FLT_INVALID_OPERATION,"EXCEPTION_FLT_INVALID_OPERATION",},
				        {EXCEPTION_FLT_STACK_CHECK,"EXCEPTION_FLT_STACK_CHECK",},
				        {EXCEPTION_FLT_UNDERFLOW,"EXCEPTION_FLT_UNDERFLOW",},
				        {EXCEPTION_INT_OVERFLOW,"EXCEPTION_INT_OVERFLOW",0},
				        {EXCEPTION_PRIV_INSTRUCTION,"EXCEPTION_PRIV_INSTRUCTION",0},
				        {EXCEPTION_IN_PAGE_ERROR,"EXCEPTION_IN_PAGE_ERROR",0},
				        {EXCEPTION_NONCONTINUABLE_EXCEPTION,"EXCEPTION_NONCONTINUABLE_EXCEPTION",0},
				        {EXCEPTION_INVALID_DISPOSITION,"EXCEPTION_INVALID_DISPOSITION",0},
				        {EXCEPTION_GUARD_PAGE,"EXCEPTION_GUARD_PAGE",0},
				        {EXCEPTION_INVALID_HANDLE,"EXCEPTION_INVALID_HANDLE",0},
				*/
			};
			static union
			{
				int     i;
				int     *iptr;
				double  d;
			} zero_const, refers;
			zero_const.i=0L;
			struct MenuItem ModalMenuItem;
			memset(&ModalMenuItem,0,sizeof(ModalMenuItem));
			VMenu ModalMenu("Test Exceptions",NULL,0,ScrY-4);
			ModalMenu.SetFlags(VMENU_WRAPMODE);
			ModalMenu.SetPosition(-1,-1,0,0);

			for (int I=0; I<sizeof(ECode)/sizeof(ECode[0]); I++)
			{
				strcpy(ModalMenuItem.Name,ECode[I].Name);
				ModalMenu.AddItem(&ModalMenuItem);
			}

			ModalMenu.Process();
			int ExitCode=ModalMenu.Modal::GetExitCode();

			switch (ExitCode)
			{
				case -1:
					return TRUE;
				case 0:
					zero_const.i=*zero_const.iptr;
					break;
				case 1:
					*zero_const.iptr = 0;
					break;
				case 2:
					zero_const.i=1/zero_const.i;
					break;
				case 3:
#if defined(_MSC_VER)
#ifdef _M_IA64
#define __REG_IA64_IntR0 1024
					__setReg(__REG_IA64_IntR0, 666);
#else
					__ud2();
#endif
#elif defined(__GNUC__)
					asm("ud2");
#elif defined(__BORLANDC__)
					__emit__(0xF, 0xB);
#else
#error "Unsupported compiler"
#endif
					break;
				case 4:
					Test_EXCEPTION_STACK_OVERFLOW(NULL);
					break;
				case 5:
					refers.d = 1 / zero_const.d;
#ifdef _M_IA64
					break;
				case 6:
				{
					char temp[10];
					memset(temp, 0, 10);
					double* val;
					val = (double*)(&temp[3]);
					printf("%lf\n", *val);
				}
#endif
			}

			Message(MSG_WARNING, 1, "Test Exceptions failed", "", ECode[ExitCode].Name, "", MSG(MOk));
			return TRUE;
		}

#endif

		/*** БЛОК ПРИВЕЛЕГИРОВАННЫХ КЛАВИШ ! ***/

		/***   КОТОРЫЕ НЕЛЬЗЯ НАМАКРОСИТЬ    ***/
		switch (Key)
		{
			case(KEY_ALT|KEY_NUMPAD0):
			case(KEY_ALT|KEY_INS):
			{
				RunGraber();
				return TRUE;
			}
			case KEY_CONSOLE_BUFFER_RESIZE:
				Sleep(1);
				ResizeAllFrame();
				return TRUE;
		}

		/*** А вот здесь - все остальное! ***/
		if (!IsProcessAssignMacroKey || IsProcessVE_FindFile)
			// в любом случае если кому-то ненужны все клавиши или
		{
			/* ** Эти клавиши разрешены для работы вьювера/редактора
			      во время вызова онных из поиска файлов ** */
			switch (Key)
			{
				case KEY_CTRLW:
					ShowProcessList();
					return(TRUE);
				case KEY_F11:
					PluginsMenu();
					FrameManager->RefreshFrame();
					_OT(SysLog(-1));
					return TRUE;
				case KEY_ALTF9:
				{
					//_SVS(SysLog(1,"Manager::ProcessKey, KEY_ALTF9 pressed..."));
					Sleep(1);
					SetVideoMode(FarAltEnter(-2));
					Sleep(1);

					/* В процессе исполнения Alt-F9 (в нормальном режиме) в очередь
					   консоли попадает WINDOW_BUFFER_SIZE_EVENT, формируется в
					   ChangeVideoMode().
					   В режиме исполнения макросов ЭТО не происходит по вполне понятным
					   причинам.
					*/
					if (CtrlObject->Macro.IsExecuting())
					{
						int PScrX=ScrX;
						int PScrY=ScrY;
						Sleep(1);
						GetVideoMode(CurScreenBufferInfo);

						if (PScrX+1 == CurScreenBufferInfo.dwSize.X &&
						        PScrY+1 == CurScreenBufferInfo.dwSize.Y)
						{
							//_SVS(SysLog(-1,"GetInputRecord(WINDOW_BUFFER_SIZE_EVENT); return KEY_NONE"));
							return TRUE;
						}
						else
						{
							PrevScrX=PScrX;
							PrevScrY=PScrY;
							//_SVS(SysLog(-1,"GetInputRecord(WINDOW_BUFFER_SIZE_EVENT); return KEY_CONSOLE_BUFFER_RESIZE"));
							Sleep(1);
							return ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
						}
					}

					//_SVS(SysLog(-1));
					return TRUE;
				}
				case KEY_F12:
				{
					int TypeFrame=FrameManager->GetCurrentFrame()->GetType();

					if (TypeFrame != MODALTYPE_HELP && TypeFrame != MODALTYPE_DIALOG)
					{
						DeactivateFrame(FrameMenu(),0);
						_OT(SysLog(-1));
						return TRUE;
					}

					break; // отдадим F12 дальше по цепочке
				}
			}

			// а здесь то, что может быть запрещено везде :-)
			if (!IsProcessVE_FindFile)
			{
				switch (Key)
				{
					case KEY_CTRLALTSHIFTPRESS:
					case KEY_RCTRLALTSHIFTPRESS:
					{
						if (!(Opt.CASRule&1) && Key == KEY_CTRLALTSHIFTPRESS)
							break;

						if (!(Opt.CASRule&2) && Key == KEY_RCTRLALTSHIFTPRESS)
							break;

						if (!NotUseCAS)
						{
							if (CurrentFrame->FastHide())
							{
								int isPanelFocus=CurrentFrame->GetType() == MODALTYPE_PANELS;

								if (isPanelFocus)
								{
									int LeftVisible=CtrlObject->Cp()->LeftPanel->IsVisible();
									int RightVisible=CtrlObject->Cp()->RightPanel->IsVisible();
									int CmdLineVisible=CtrlObject->CmdLine->IsVisible();
									int KeyBarVisible=CtrlObject->Cp()->MainKeyBar.IsVisible();
									CtrlObject->CmdLine->ShowBackground();
									CtrlObject->Cp()->LeftPanel->Hide0();
									CtrlObject->Cp()->RightPanel->Hide0();

									switch (Opt.PanelCtrlAltShiftRule)
									{
										case 0:
											CtrlObject->CmdLine->Show();
											CtrlObject->Cp()->MainKeyBar.Show();
											break;
										case 1:
											CtrlObject->Cp()->MainKeyBar.Show();
											break;
									}

									WaitKey(Key==KEY_CTRLALTSHIFTPRESS?KEY_CTRLALTSHIFTRELEASE:KEY_RCTRLALTSHIFTRELEASE);

									if (LeftVisible)      CtrlObject->Cp()->LeftPanel->Show();

									if (RightVisible)     CtrlObject->Cp()->RightPanel->Show();

									if (CmdLineVisible)   CtrlObject->CmdLine->Show();

									if (KeyBarVisible)    CtrlObject->Cp()->MainKeyBar.Show();
								}
								else
								{
									ImmediateHide();
									WaitKey(Key==KEY_CTRLALTSHIFTPRESS?KEY_CTRLALTSHIFTRELEASE:KEY_RCTRLALTSHIFTRELEASE);
								}

								FrameManager->RefreshFrame();
							}

							return TRUE;
						}

						break;
					}
					case KEY_CTRLTAB:
					case KEY_CTRLSHIFTTAB:

						if (CurrentFrame->GetCanLoseFocus())
						{
							DeactivateFrame(CurrentFrame,Key==KEY_CTRLTAB?1:-1);
						}

						_OT(SysLog(-1));
						return TRUE;
				}
			}
		}

		CurrentFrame->UpdateKeyBar();
		CurrentFrame->ProcessKey(Key);
	}

	_OT(SysLog(-1));
	return ret;
}

int  Manager::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	// При каптюренной мыши отдаем управление заданному объекту
//    if (ScreenObject::CaptureMouseObject)
//      return ScreenObject::CaptureMouseObject->ProcessMouse(MouseEvent);
	int ret=FALSE;

//    _D(SysLog(1,"Manager::ProcessMouse()"));
	if (CurrentFrame)
		ret=CurrentFrame->ProcessMouse(MouseEvent);

//    _D(SysLog("Manager::ProcessMouse() ret=%i",ret));
	_OT(SysLog(-1));
	return ret;
}

void Manager::PluginsMenu()
{
	_OT(SysLog(1));
	int curType = CurrentFrame->GetType();

	if (curType == MODALTYPE_PANELS || curType == MODALTYPE_EDITOR || curType == MODALTYPE_VIEWER || curType == MODALTYPE_DIALOG)
	{
		/* 02.01.2002 IS
		   ! Вывод правильной помощи по Shift-F1 в меню плагинов в редакторе/вьюере/диалоге
		   ! Если на панели QVIEW или INFO открыт файл, то считаем, что это
		     полноценный вьюер и запускаем с соответствующим параметром плагины
		*/
		if (curType==MODALTYPE_PANELS)
		{
			int pType=CtrlObject->Cp()->ActivePanel->GetType();

			if (pType==QVIEW_PANEL || pType==INFO_PANEL)
			{
				char CurFileName[NM]="";
				CtrlObject->Cp()->GetTypeAndName(NULL,CurFileName);

				if (*CurFileName)
				{
					DWORD Attr=GetFileAttributes(CurFileName);

					// интересуют только обычные файлы
					if (Attr!=INVALID_FILE_ATTRIBUTES && !(Attr&FILE_ATTRIBUTE_DIRECTORY))
						curType=MODALTYPE_VIEWER;
				}
			}
		}

		// в редакторе, вьюере или диалоге покажем свою помощь по Shift-F1
		const char *Topic=curType==MODALTYPE_EDITOR?"Editor":
		                  curType==MODALTYPE_VIEWER?"Viewer":
		                  curType==MODALTYPE_DIALOG?"Dialog":NULL;
		CtrlObject->Plugins.CommandsMenu(curType,0,Topic);
		/* IS $ */
	}

	_OT(SysLog(-1));
}

BOOL Manager::IsPanelsActive()
{
	if (FramePos>=0)
	{
		return CurrentFrame?CurrentFrame->GetType() == MODALTYPE_PANELS:FALSE;
	}
	else
	{
		return FALSE;
	}
}

Frame *Manager::operator[](int Index)
{
	if (Index<0||Index>=FrameCount ||FrameList==0)
	{
		return NULL;
	}

	return FrameList[Index];
}

int Manager::IndexOfStack(Frame *Frame)
{
	int Result=-1;

	for (int i=0; i<ModalStackCount; i++)
	{
		if (Frame==ModalStack[i])
		{
			Result=i;
			break;
		}
	}

	return Result;
}

int Manager::IndexOf(Frame *Frame)
{
	int Result=-1;

	for (int i=0; i<FrameCount; i++)
	{
		if (Frame==FrameList[i])
		{
			Result=i;
			break;
		}
	}

	return Result;
}

BOOL Manager::Commit()
{
	_MANAGERLOG(CleverSysLog Clev("Manager::Commit()"));
	int Result = false;

	if (DeletedFrame && (InsertedFrame||ExecutedFrame))
	{
		UpdateCommit();
		DeletedFrame = NULL;
		InsertedFrame = NULL;
		ExecutedFrame=NULL;
		Result=true;
	}
	else if (ExecutedFrame)
	{
		ExecuteCommit();
		ExecutedFrame=NULL;
		Result=true;
	}
	else if (DeletedFrame)
	{
		DeleteCommit();
		DeletedFrame = NULL;
		Result=true;
	}
	else if (InsertedFrame)
	{
		InsertCommit();
		InsertedFrame = NULL;
		Result=true;
	}
	else if (DeactivatedFrame)
	{
		DeactivateCommit();
		DeactivatedFrame=NULL;
		Result=true;
	}
	else if (ActivatedFrame)
	{
		ActivateCommit();
		ActivatedFrame=NULL;
		Result=true;
	}
	else if (RefreshedFrame)
	{
		RefreshCommit();
		RefreshedFrame=NULL;
		Result=true;
	}
	else if (ModalizedFrame)
	{
		ModalizeCommit();
//    ModalizedFrame=NULL;
		Result=true;
	}
	else if (UnmodalizedFrame)
	{
		UnmodalizeCommit();
//    UnmodalizedFrame=NULL;
		Result=true;
	}

	if (Result)
	{
		Result=Commit();
	}

	return Result;
}

void Manager::DeactivateCommit()
{
	_MANAGERLOG(CleverSysLog Clev("Manager::DeactivateCommit()"));
	_MANAGERLOG(SysLog("DeactivatedFrame=%p (%s)",DeactivatedFrame,(DeactivatedFrame?DeactivatedFrame->GetTypeName():NULL)));

	/*$ 18.04.2002 skv
	  Если нечего активировать, то в общем-то не надо и деактивировать.
	*/
	if (!DeactivatedFrame || !ActivatedFrame)
	{
		return;
	}

	if (!ActivatedFrame)
	{
		_MANAGERLOG(SysLog("if (!ActivatedFrame) ==> WARNING!!!"));
	}

	if (DeactivatedFrame)
	{
		DeactivatedFrame->OnChangeFocus(0);
	}

	int modalIndex=IndexOfStack(DeactivatedFrame);

	if (-1 != modalIndex && modalIndex== ModalStackCount-1)
	{
		/*if (IsSemiModalBackFrame(ActivatedFrame))
		{ // Является ли "родителем" полумодального фрэйма?
		  ModalStackCount--;
		}
		else
		{*/
		if (IndexOfStack(ActivatedFrame)==-1)
		{
			ModalStack[ModalStackCount-1]=ActivatedFrame;
		}
		else
		{
			ModalStackCount--;
		}

//    }
	}
}


void Manager::ActivateCommit()
{
	_MANAGERLOG(CleverSysLog Clev("Manager::ActivateCommit()"));
	_MANAGERLOG(SysLog("ActivatedFrame=%p (%s)",ActivatedFrame,(ActivatedFrame?ActivatedFrame->GetTypeName():NULL)));

	if (CurrentFrame==ActivatedFrame)
	{
		RefreshedFrame=ActivatedFrame;
		return;
	}

	int FrameIndex=IndexOf(ActivatedFrame);

	if (-1!=FrameIndex)
	{
		FramePos=FrameIndex;
	}

	/* 14.05.2002 SKV
	  Если мы пытаемся активировать полумодальный фрэйм,
	  то надо его вытащит на верх стэка модалов.
	*/

	for (int I=0; I<ModalStackCount; I++)
	{
		if (ModalStack[I]==ActivatedFrame)
		{
			Frame *tmp=ModalStack[I];
			ModalStack[I]=ModalStack[ModalStackCount-1];
			ModalStack[ModalStackCount-1]=tmp;
			break;
		}
	}

	/* SKV $ */
	RefreshedFrame=CurrentFrame=ActivatedFrame;
}

void Manager::UpdateCommit()
{
	_MANAGERLOG(CleverSysLog Clev("Manager::UpdateCommit()"));
	_MANAGERLOG(SysLog("DeletedFrame=%p (%s), InsertedFrame=%p (%s), ExecutedFrame=%p (%s)",DeletedFrame,(DeletedFrame?DeletedFrame->GetTypeName():NULL),InsertedFrame,(InsertedFrame?InsertedFrame->GetTypeName():NULL), ExecutedFrame,(ExecutedFrame?ExecutedFrame->GetTypeName():NULL)));

	if (ExecutedFrame)
	{
		DeleteCommit();
		ExecuteCommit();
		return;
	}

	int FrameIndex=IndexOf(DeletedFrame);

	if (-1!=FrameIndex)
	{
		ActivateFrame(FrameList[FrameIndex] = InsertedFrame);
		ActivatedFrame->FrameToBack=CurrentFrame;
		DeleteCommit();
	}
	else
	{
		_MANAGERLOG(SysLog("ОШИБКА! Не найден удаляемый фрейм"));
	}
}

//! Удаляет DeletedFrame изо всех очередей!
//! Назначает следующий активный, (исходя из своих представлений)
//! Но только в том случае, если активный фрейм еще не назначен заранее.
void Manager::DeleteCommit()
{
	_MANAGERLOG(CleverSysLog Clev("Manager::DeleteCommit()"));
	_MANAGERLOG(SysLog("DeletedFrame=%p (%s)",DeletedFrame,(DeletedFrame?DeletedFrame->GetTypeName():NULL)));

	if (!DeletedFrame)
	{
		return;
	}

	// <ifDoubleInstance>
	//BOOL ifDoubI=ifDoubleInstance(DeletedFrame);
	// </ifDoubleInstance>
	int ModalIndex=IndexOfStack(DeletedFrame);

	if (ModalIndex!=-1)
	{
		/* $ 14.05.2002 SKV
		  Надёжнее найти и удалить именно то, что
		  нужно, а не просто верхний.
		*/
		for (int i=0; i<ModalStackCount; i++)
		{
			if (ModalStack[i]==DeletedFrame)
			{
				for (int j=i+1; j<ModalStackCount; j++)
				{
					ModalStack[j-1]=ModalStack[j];
				}

				ModalStackCount--;
				break;
			}
		}

		/* SKV $ */
		if (ModalStackCount)
		{
			ActivateFrame(ModalStack[ModalStackCount-1]);
		}
	}

	for (int i=0; i<FrameCount; i++)
	{
		if (FrameList[i]->FrameToBack==DeletedFrame)
		{
			FrameList[i]->FrameToBack=CtrlObject->Cp();
		}
	}

	int FrameIndex=IndexOf(DeletedFrame);

	if (-1!=FrameIndex)
	{
		DeletedFrame->DestroyAllModal();

		for (int j=FrameIndex; j<FrameCount-1; j++)
		{
			FrameList[j]=FrameList[j+1];
		}

		FrameCount--;

		if (FramePos >= FrameCount)
		{
			FramePos=0;
		}

		if (DeletedFrame->FrameToBack==CtrlObject->Cp())
		{
			ActivateFrame(FrameList[FramePos]);
		}
		else
		{
			ActivateFrame(DeletedFrame->FrameToBack);
		}
	}

	/* $ 14.05.2002 SKV
	  Долго не мог понять, нужен всё же этот код или нет.
	  Но вроде как нужен.
	  SVS> Когда понадобится - в некоторых местах расскомментить куски кода
	       помеченные скобками <ifDoubleInstance>

	if (ifDoubI && IsSemiModalBackFrame(ActivatedFrame)){
	  for(int i=0;i<ModalStackCount;i++)
	  {
	    if(ModalStack[i]==ActivatedFrame)
	    {
	      break;
	    }
	  }

	  if(i==ModalStackCount)
	  {
	    if (ModalStackCount == ModalStackSize){
	      ModalStack = (Frame **) xf_realloc (ModalStack, ++ModalStackSize * sizeof (Frame *));
	    }
	    ModalStack[ModalStackCount++]=ActivatedFrame;
	  }
	}
	*/
	/* SKV $ */
	DeletedFrame->OnDestroy();

	if (DeletedFrame->GetDynamicallyBorn())
	{
		_MANAGERLOG(SysLog("delete DeletedFrame %p (%s), CurrentFrame=%p (%s)",DeletedFrame,(DeletedFrame?DeletedFrame->GetTypeName():NULL),CurrentFrame,(CurrentFrame?CurrentFrame->GetTypeName():NULL)));

		if (CurrentFrame==DeletedFrame)
			CurrentFrame=0;

		/* $ 14.05.2002 SKV
		  Так как в деструкторе фрэйма неявно может быть
		  вызван commit, то надо подстраховаться.
		*/
		Frame *tmp=DeletedFrame;
		DeletedFrame=NULL;
		delete tmp;
		/* SKV $ */
	}

	// Полагаемся на то, что в ActevateFrame не будет переписан уже
	// присвоенный  ActivatedFrame
	if (ModalStackCount)
	{
		ActivateFrame(ModalStack[ModalStackCount-1]);
	}
	else
	{
		ActivateFrame(FramePos);
	}
}

void Manager::InsertCommit()
{
	_MANAGERLOG(CleverSysLog Clev("Manager::InsertCommit()"));
	_MANAGERLOG(SysLog("InsertedFrame=%p (%s)",InsertedFrame,(InsertedFrame?InsertedFrame->GetTypeName():NULL)));

	if (InsertedFrame)
	{
		if (FrameListSize <= FrameCount)
		{
			FrameList=(Frame **)xf_realloc(FrameList,sizeof(*FrameList)*(FrameCount+1));
			FrameListSize++;
		}

		InsertedFrame->FrameToBack=CurrentFrame;
		FrameList[FrameCount]=InsertedFrame;

		if (!ActivatedFrame)
		{
			ActivatedFrame=InsertedFrame;
		}

		FrameCount++;
	}
}

void Manager::RefreshCommit()
{
	_MANAGERLOG(CleverSysLog Clev("Manager::RefreshCommit()"));
	_MANAGERLOG(SysLog("RefreshedFrame=%p (%s)",RefreshedFrame,(RefreshedFrame?RefreshedFrame->GetTypeName():NULL)));

	if (!RefreshedFrame)
	{
		_MANAGERLOG(SysLog("[%d] return ==> RefreshedFrame=NULL",__LINE__));
		return;
	}

	if (IndexOf(RefreshedFrame)==-1 && IndexOfStack(RefreshedFrame)==-1)
	{
		_MANAGERLOG(SysLog("[%d] return ==> IndexOf=-1 and IndexOfStack=-1",__LINE__));
		return;
	}

	if (!RefreshedFrame->Locked())
	{
		if (!IsRedrawFramesInProcess)
		{
			_MANAGERLOG(SysLog("[%d] call RefreshedFrame->ShowConsoleTitle()",__LINE__));
			RefreshedFrame->ShowConsoleTitle();
		}

		_MANAGERLOG(SysLog("[%d] RefreshedFrame=%p %s",__LINE__,RefreshedFrame,(!RefreshedFrame?"ERROR!!!":"")));

		if (RefreshedFrame)
			RefreshedFrame->Refresh();

		if (!RefreshedFrame)
		{
			_MANAGERLOG(SysLog("[%d] return ==> RefreshedFrame=NULL",__LINE__));
			return;
		}

		CtrlObject->Macro.SetMode(RefreshedFrame->GetMacroMode());
	}

	if (Opt.ViewerEditorClock &&
	        (RefreshedFrame->GetType() == MODALTYPE_EDITOR ||
	         RefreshedFrame->GetType() == MODALTYPE_VIEWER)
	        || WaitInMainLoop && Opt.Clock)
	{
		_MANAGERLOG(SysLog("[%d] call ShowTime(1)",__LINE__));
		ShowTime(1);
	}

	_MANAGERLOG(SysLog("[%d] normal return",__LINE__));
}

void Manager::ExecuteCommit()
{
	_MANAGERLOG(CleverSysLog Clev("Manager::ExecuteCommit()"));
	_MANAGERLOG(SysLog("ExecutedFrame=%p (%s)",ExecutedFrame,(ExecutedFrame?ExecutedFrame->GetTypeName():NULL)));

	if (!ExecutedFrame)
	{
		return;
	}

	if (ModalStackCount == ModalStackSize)
	{
		ModalStack = (Frame **) xf_realloc(ModalStack, ++ModalStackSize * sizeof(Frame *));
	}

	ModalStack [ModalStackCount++] = ExecutedFrame;
	ActivatedFrame=ExecutedFrame;
}

/*$ 26.06.2001 SKV
  Для вызова из плагинов посредством ACTL_COMMIT
*/
BOOL Manager::PluginCommit()
{
	return Commit();
}
/* SKV$*/

/* $ Введена для нужд CtrlAltShift OT */
void Manager::ImmediateHide()
{
	if (FramePos<0)
		return;

	// Сначала проверяем, есть ли у прятываемого фрейма SaveScreen
	if (CurrentFrame->HasSaveScreen())
	{
		CurrentFrame->Hide();
		return;
	}

	// Фреймы перерисовываются, значит для нижних
	// не выставляем заголовок консоли, чтобы не мелькал.
	if (ModalStackCount>0)
	{
		/* $ 28.04.2002 KM
		    Проверим, а не модальный ли редактор или вьювер на вершине
		    модального стека? И если да, покажем User screen.
		*/
		if (ModalStack[ModalStackCount-1]->GetType()==MODALTYPE_EDITOR ||
		        ModalStack[ModalStackCount-1]->GetType()==MODALTYPE_VIEWER)
		{
			CtrlObject->CmdLine->ShowBackground();
		}
		else
		{
			int UnlockCount=0;
			/* $ 07.04.2002 KM */
			IsRedrawFramesInProcess++;
			/* KM $ */

			while ((*this)[FramePos]->Locked())
			{
				(*this)[FramePos]->Unlock();
				UnlockCount++;
			}

			RefreshFrame((*this)[FramePos]);
			Commit();

			for (int i=0; i<UnlockCount; i++)
			{
				(*this)[FramePos]->Lock();
			}

			if (ModalStackCount>1)
			{
				for (int i=0; i<ModalStackCount-1; i++)
				{
					if (!(ModalStack[i]->FastHide() & CASR_HELP))
					{
						RefreshFrame(ModalStack[i]);
						Commit();
					}
					else
					{
						break;
					}
				}
			}

			/* $ 04.04.2002 KM
			   Перерисуем заголовок только у активного фрейма.
			   Этим мы предотвращаем мелькание заголовка консоли
			   при перерисовке всех фреймов.
			*/
			IsRedrawFramesInProcess--;
			CurrentFrame->ShowConsoleTitle();
			/* KM $ */
		}

		/* KM $ */
	}
	else
	{
		CtrlObject->CmdLine->ShowBackground();
	}
}

void Manager::ModalizeCommit()
{
	CurrentFrame->Push(ModalizedFrame);
	ModalizedFrame=NULL;
}

void Manager::UnmodalizeCommit()
{
	int i;
	Frame *iFrame;

	for (i=0; i<FrameCount; i++)
	{
		iFrame=FrameList[i];

		if (iFrame->RemoveModal(UnmodalizedFrame))
		{
			break;
		}
	}

	for (i=0; i<ModalStackCount; i++)
	{
		iFrame=ModalStack[i];

		if (iFrame->RemoveModal(UnmodalizedFrame))
		{
			break;
		}
	}

	UnmodalizedFrame=NULL;
}
/* OT $*/

/* $ 15.05.2002 SKV
  Чуток подправим логику.
*/

BOOL Manager::ifDoubleInstance(Frame *frame)
{
	// <ifDoubleInstance>
	/*
	  if (ModalStackCount<=0)
	    return FALSE;
	  if(IndexOfStack(frame)==-1)
	    return FALSE;
	  if(IndexOf(frame)!=-1)
	    return TRUE;
	*/
	// </ifDoubleInstance>
	return FALSE;
}

/* SKV $ */

/*  Вызов ResizeConsole для всех NextModal у
    модального фрейма. KM
*/
void Manager::ResizeAllModal(Frame *ModalFrame)
{
	if (!ModalFrame->NextModal)
		return;

	Frame *iModal=ModalFrame->NextModal;

	while (iModal)
	{
		iModal->ResizeConsole();
		iModal=iModal->NextModal;
	}
}
/* KM $ */

void Manager::ResizeAllFrame()
{
	int I;

	for (I=0; I < FrameCount; I++)
	{
		FrameList[I]->ResizeConsole();
	}

	for (I=0; I < ModalStackCount; I++)
	{
		ModalStack[I]->ResizeConsole();
		/* $ 13.04.2002 KM
		  - А теперь проресайзим все NextModal...
		*/
		ResizeAllModal(ModalStack[I]);
		/* KM $ */
	}

	ImmediateHide();
	FrameManager->RefreshFrame();
	//RefreshFrame();
}

void Manager::InitKeyBar(void)
{
	for (int I=0; I < FrameCount; I++)
		FrameList[I]->InitKeyBar();
}

/*void Manager::AddSemiModalBackFrame(Frame* frame)
{
  if(SemiModalBackFramesCount>=SemiModalBackFramesSize)
  {
    SemiModalBackFramesSize+=4;
    SemiModalBackFrames=
      (Frame**)xf_realloc(SemiModalBackFrames,sizeof(Frame*)*SemiModalBackFramesSize);

  }
  SemiModalBackFrames[SemiModalBackFramesCount]=frame;
  SemiModalBackFramesCount++;
}

BOOL Manager::IsSemiModalBackFrame(Frame *frame)
{
  if(!SemiModalBackFrames)return FALSE;
  for(int i=0;i<SemiModalBackFramesCount;i++)
  {
    if(SemiModalBackFrames[i]==frame)return TRUE;
  }
  return FALSE;
}

void Manager::RemoveSemiModalBackFrame(Frame* frame)
{
  if(!SemiModalBackFrames)return;
  for(int i=0;i<SemiModalBackFramesCount;i++)
  {
    if(SemiModalBackFrames[i]==frame)
    {
      for(int j=i+1;j<SemiModalBackFramesCount;j++)
      {
        SemiModalBackFrames[j-1]=SemiModalBackFrames[j];
      }
      SemiModalBackFramesCount--;
      return;
    }
  }
}
*/

// возвращает top-модал или сам фрейм, если у фрейма нету модалов
Frame* Manager::GetTopModal()
{
	Frame *f=CurrentFrame, *fo=NULL;

	while (f)
	{
		fo=f;
		f=f->GetTopModal();
	}

	if (!f)
		f=fo;

	return f;
}
