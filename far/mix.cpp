/*
mix.cpp

Куча разных вспомогательных функций

*/

#include "headers.hpp"
#pragma hdrstop

#include "plugin.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "flink.hpp"
#include "treelist.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "savefpos.hpp"
#include "chgprior.hpp"
#include "filepanels.hpp"
#include "filelist.hpp"
#include "panel.hpp"
#include "scantree.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "CFileMask.hpp"
#include "constitle.hpp"
#include "udlist.hpp"
#include "manager.hpp"
#include "lockscrn.hpp"
#include "lasterror.hpp"
#include "RefreshFrameManager.hpp"
#include "filefilter.hpp"
#include "TPreRedrawFunc.hpp"
#include "TaskBar.hpp"

long filelen(FILE *FPtr)
{
	SaveFilePos SavePos(FPtr);
	fseek(FPtr,0,SEEK_END);
	return(ftell(FPtr));
}

__int64 filelen64(FILE *FPtr)
{
	SaveFilePos SavePos(FPtr);
	fseek64(FPtr,0,SEEK_END);
	return(ftell64(FPtr));
}



/* $ 14.01.2002 IS
     Установка нужного диска и каталога и установление соответствующей переменной
     окружения. В случае успеха возвращается не ноль.
   $ 22.01.2002 IS
     + Обработаем самостоятельно пути типа "буква:"
   $ 15.02.2002 IS
     + Новый параметр ChangeDir, если FALSE, то не меняем текущий диск, а только
       устанавливаем переменные окружения. По умолчанию - TRUE.
   $ 07.11.2002 IS
     + Принудительно меняем все / на \ (попытка побороть bugz#568)
*/
BOOL FarChDir(const char *NewDir, BOOL ChangeDir)
{
	_CHANGEDIR(CleverSysLog clv("FarChDir"));
	_CHANGEDIR(SysLog("(NewDir=\"%s\", ChangeDir=%d)",NewDir,ChangeDir));

	if (!NewDir || *NewDir == 0)
		return FALSE;

	BOOL rc=FALSE;
	char CurDir[NM*2];

	if (GetCurrentDirectory(sizeof(CurDir),CurDir) && !LocalStricmp(CurDir,NewDir))
	{
		_CHANGEDIR(SysLog("(CurDir == NewDir) ==> return TRUE;"));
		return TRUE;
	}

	char Drive[4]="=A:";

	if (isalpha(*NewDir) && NewDir[1]==':' && NewDir[2]==0)// если указана только
	{                                                     // буква диска, то путь
		Drive[1]=toupper(*NewDir);                          // возьмем из переменной

		if (GetEnvironmentVariable(Drive,CurDir,sizeof(CurDir))) // окружения
			FAR_CharToOem(CurDir,CurDir);
		else
		{
			sprintf(CurDir,"%s\\",NewDir); // при неудаче переключимся в корень диска
			char *Chr=CurDir;

			while (*Chr)
			{
				if (*Chr=='/') *Chr='\\';

				++Chr;
			}
		}

		*CurDir=toupper(*CurDir);

		if (ChangeDir)
			rc=SetCurrentDirectory(CurDir);
	}
	else
	{
		if (ChangeDir)
		{
			if (strcmp(NewDir,".."))
			{
				xstrncpy(CurDir,NewDir,sizeof(CurDir)-1);

				if (!strcmp(CurDir,"\\"))
					FarGetCurDir(sizeof(CurDir),CurDir); // здесь берем корень

				char *Chr=CurDir;

				while (*Chr)
				{
					if (*Chr=='/')
						*Chr='\\';

					++Chr;
				}

				DWORD att1=GetFileAttributes(NewDir);
				char *ptr;
				char FullDir[sizeof(CurDir)];
				GetFullPathName(NewDir,sizeof(FullDir),FullDir,&ptr);
				_CHANGEDIR(SysLog("[%d] FullDir=\"%s\"",__LINE__,FullDir));
				AddEndSlash(FullDir);
				DWORD att2=GetFileAttributes(FullDir);

				if (att2==INVALID_FILE_ATTRIBUTES)
				{
					_CHANGEDIR(SysLog("[%d] att2==INVALID_FILE_ATTRIBUTES, CALL PrepareDiskPath()",__LINE__));
					PrepareDiskPath(FullDir,sizeof(FullDir)-1);
					att2=GetFileAttributes(FullDir);
				}

				_SVS(SysLog("[%d] att1=0x%08X att2=0x%08X",__LINE__,att1,att2));

				if (att2==INVALID_FILE_ATTRIBUTES || (att1!=att2 && strcmp(NewDir,"..")))
					rc=SetCurrentDirectory(NewDir);
				else
					rc=SetCurrentDirectory(FullDir);
			}
			else
				rc=SetCurrentDirectory(NewDir);
		}
	}

	if (rc || !ChangeDir)
	{
		if ((!ChangeDir || GetCurrentDirectory(sizeof(CurDir),CurDir)) && isalpha(*CurDir) && CurDir[1]==':')
		{
			Drive[1]=toupper(*CurDir);
			FAR_OemToChar(CurDir,CurDir); // аргументы SetEnvironmentVariable должны быть ANSI
			SetEnvironmentVariable(Drive,CurDir);
			//    rc=0;
		}
	}

	return rc;
}

bool TestCurrentDirectory(const char *TestDir)
{
	char CurDir[NM*2];

	if (GetCurrentDirectory(sizeof(CurDir),CurDir) && !LocalStricmp(CurDir,TestDir))
		return true;

	return false;
}


/* $ 20.03.2002 SVS
 обертка вокруг функции получения текущего пути.
 для локального пути переводит букву диска в uppercase
*/
DWORD FarGetCurDir(DWORD Length,char *Buffer)
{
	DWORD Result=GetCurrentDirectory(Length,Buffer);

	/*
	баг в GetCurrentDirectory:
	если текущий каталог - "\\?\Volume{GUID}\",
	то в Buffer не попадает завершающий слеш
	и дальнейшая работа с таким путём обламывается.
	*/
	if (IsLocalVolumeRootPath(Buffer))
		AddEndSlash(Buffer);

	if (Result && isalpha(*Buffer) && Buffer[1]==':' && (Buffer[2]==0 || Buffer[2]== '\\'))
		*Buffer=toupper(*Buffer);

	return Result;
}
/* SVS 20.03.2002 $ */

int ToPercent(unsigned long N1,unsigned long N2)
{
	if (N1 > 10000)
	{
		N1/=100;
		N2/=100;
	}

	if (N2==0)
		return(0);

	if (N2<N1)
		return(100);

	return((int)(N1*100/N2));
}

int ToPercent64(__int64 N1,__int64 N2)
{
	if (N1 > _i64(10000))
	{
		N1/=_i64(100);
		N2/=_i64(100);
	}

	if (N2==_i64(0))
		return(_i64(0));

	if (N2<N1)
		return(100);

	return((int)(N1*_i64(100)/N2));
}



/* $ 09.10.2000 IS
    + Новая функция для обработки имени файла
*/
// обработать имя файла: сравнить с маской, масками, сгенерировать по маске
int WINAPI ProcessName(const char *param1, char *param2, DWORD flags)
{
	int skippath=flags&PN_SKIPPATH;
	flags &= ~PN_SKIPPATH;

	if (flags == PN_CMPNAME)
		return CmpName(param1, param2, skippath);

	if (flags == PN_CMPNAMELIST)
	{
		int Found=FALSE;
		char FileMask[NM];
		const char *MaskPtr;
		MaskPtr=param1;

		while ((MaskPtr=GetCommaWord(MaskPtr,FileMask))!=NULL)
		{
			if (CmpName(FileMask,param2,skippath))
			{
				Found=TRUE;
				break;
			}
		}

		return Found;
	}

	if (flags&PN_GENERATENAME)
		return ConvertWildcards(param1, param2, (flags&0xFF)|(skippath?PN_SKIPPATH:0));

	return FALSE;
}
/* IS $ */

int GetFileTypeByName(const char *Name)
{
	HANDLE hFile=FAR_CreateFile(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
	                            NULL,OPEN_EXISTING,0,NULL);

	if (hFile==INVALID_HANDLE_VALUE)
		return(FILE_TYPE_UNKNOWN);

	int Type=GetFileType(hFile);
	CloseHandle(hFile);
	return(Type);
}


static void DrawGetDirInfoMsg(char *Title,const char *Name)
{
	Message(0,0,Title,MSG(MScanningFolder),Name);
	PreRedrawItem preRedrawItem=PreRedraw.Peek();
	preRedrawItem.Param.Param1=Title;
	preRedrawItem.Param.Param2=Name;
	PreRedraw.SetParam(preRedrawItem.Param);
}

static void PR_DrawGetDirInfoMsg(void)
{
	PreRedrawItem preRedrawItem=PreRedraw.Peek();
	DrawGetDirInfoMsg((char *)preRedrawItem.Param.Param1,(char *)preRedrawItem.Param.Param2);
}

int GetDirInfo(char *Title,
               const char *DirName,
               unsigned long &DirCount,
               unsigned long &FileCount,
               unsigned __int64 &FileSize,
               unsigned __int64 &CompressedFileSize,
               unsigned __int64 &RealSize,
               unsigned long &ClusterSize,
               clock_t MsgWaitTime,
               FileFilter *Filter,
               DWORD Flags)
{
	DirCount=FileCount=0;
	FileSize=_i64(0);
	CompressedFileSize=_i64(0);
	RealSize=_i64(0);
	char FullDirName[NM],DriveRoot[NM];
	char FullName[NM],CurDirName[NM],LastDirName[NM];

	if (ConvertNameToFull(DirName,FullDirName, sizeof(FullDirName)) >= sizeof(FullDirName))
		return -1;

	unsigned __int64 CurSize;
	SaveScreen SaveScr;
	UndoGlobalSaveScrPtr UndSaveScr(&SaveScr);
	TPreRedrawFuncGuard preRedrawFuncGuard(PR_DrawGetDirInfoMsg);
	TaskBar TB;
	ScanTree ScTree(FALSE,TRUE,
	                (Flags&GETDIRINFO_SCANSYMLINKDEF?(DWORD)-1:(Flags&GETDIRINFO_SCANSYMLINK)),
	                Flags&GETDIRINFO_USEDALTFOLDERNAME);
	WIN32_FIND_DATA FindData;
	int MsgOut=0;
	clock_t StartTime=clock();
	SetCursorType(FALSE,0);
	GetPathRoot(FullDirName,DriveRoot);
	/* $ 20.03.2002 DJ
	   для . - покажем имя родительского каталога
	*/
	const char *ShowDirName = DirName;

	if (DirName[0] == '.' && DirName[1] == 0)
	{
		char *p = strrchr(FullDirName, '\\');

		if (p)
			ShowDirName = p + 1;
	}

	ConsoleTitle OldTitle;
	RefreshFrameManager frref(ScrX,ScrY,MsgWaitTime,Flags&GETDIRINFO_DONTREDRAWFRAME);

	if ((ClusterSize=GetClusterSize(DriveRoot))==0)
	{
		DWORD SectorsPerCluster=0,BytesPerSector=0,FreeClusters=0,Clusters=0;

		if (GetDiskFreeSpace(DriveRoot,&SectorsPerCluster,&BytesPerSector,&FreeClusters,&Clusters))
			ClusterSize=SectorsPerCluster*BytesPerSector;
	}

	// Временные хранилища имён каталогов
	*LastDirName=0;
	*CurDirName=0;
	ScTree.SetFindPath(DirName,"*.*");

	while (ScTree.GetNextName(&FindData,FullName, sizeof(FullName)-1))
	{
		if (!CtrlObject->Macro.IsExecuting())
		{
			INPUT_RECORD rec;

			switch (PeekInputRecord(&rec))
			{
				case 0:
				case KEY_IDLE:
					break;
				case KEY_NONE:
				case KEY_ALT:
				case KEY_CTRL:
				case KEY_SHIFT:
				case KEY_RALT:
				case KEY_RCTRL:
					GetInputRecord(&rec);
					break;
				case KEY_ESC:
				case KEY_BREAK:
					GetInputRecord(&rec);
					return(0);
				default:

					if (Flags&GETDIRINFO_ENHBREAK)
					{
						return(-1);
					}

					GetInputRecord(&rec);
					break;
			}
		}

		if (!MsgOut && MsgWaitTime!=0xffffffff && clock()-StartTime > MsgWaitTime)
		{
			OldTitle.Set("%s %s",MSG(MScanningFolder), ShowDirName); // покажем заголовок консоли
			SetCursorType(FALSE,0);
			DrawGetDirInfoMsg(Title,ShowDirName);
			MsgOut=1;
		}

		if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// Счётчик каталогов наращиваем только если не включен фильтр,
			// в противном случае это будем делать в подсчёте количества файлов
			if (!(Flags&GETDIRINFO_USEFILTER))
				DirCount++;
			else
			{
				// Если каталог не попадает под фильтр то его надо полностью
				// пропустить - иначе при включенном подсчёте total
				// он учтётся (mantiss 551)
				if (!Filter->FileInFilter(&FindData))
					ScTree.SkipDir();
			}
		}
		else
		{
			/* $ 17.04.2005 KM
			   Проверка попадания файла в условия фильра
			*/
			if ((Flags&GETDIRINFO_USEFILTER))
			{
				if (!Filter->FileInFilter(&FindData))
					continue;
			}

			// Наращиваем счётчик каталогов при включенном фильтре только тогда,
			// когда в таком каталоге найден файл, удовлетворяющий условиям
			// фильтра.
			if ((Flags&GETDIRINFO_USEFILTER))
			{
				char *p1=strrchr(FullName,'\\');

				if (p1)
					xstrncpy(CurDirName,FullName,p1-FullName);
				else
					xstrncpy(CurDirName,FullName,sizeof(CurDirName)-1);

				if (LocalStricmp(CurDirName,LastDirName)!=0)
				{
					DirCount++;
					xstrncpy(LastDirName,CurDirName,sizeof(LastDirName)-1);
				}
			}

			FileCount++;
			CurSize=MKUINT64(FindData.nFileSizeHigh,FindData.nFileSizeLow);
			FileSize+=CurSize;

			if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) || (FindData.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE))
			{
				DWORD CompressedSize,CompressedSizeHigh;
				CompressedSize=GetCompressedFileSize(FullName,&CompressedSizeHigh);

				if (CompressedSize!=INVALID_FILE_SIZE || GetLastError()==NO_ERROR)
					CurSize=MKUINT64(CompressedSizeHigh,CompressedSize);
			}

			CompressedFileSize+=CurSize;

			if (ClusterSize>0)
			{
				RealSize+=CurSize;
				long Slack=(long)(CurSize%ClusterSize);

				if (Slack>0)
					RealSize+=ClusterSize-Slack;
			}
		}
	}

	return(1);
}


int GetPluginDirInfo(HANDLE hPlugin,char *DirName,unsigned long &DirCount,
                     unsigned long &FileCount,
                     unsigned __int64 &FileSize,
                     unsigned __int64 &CompressedFileSize)
{
	struct PluginPanelItem *PanelItem=NULL;
	int ItemsNumber,ExitCode;
	DirCount=FileCount=0;
	FileSize=CompressedFileSize=0;

	if ((ExitCode=FarGetPluginDirList(((struct PluginHandle *)hPlugin)->PluginNumber,
	                                  ((struct PluginHandle *)hPlugin)->InternalHandle,DirName,
	                                  &PanelItem,&ItemsNumber))==TRUE)
	{
		for (int I=0; I<ItemsNumber; I++)
		{
			if (PanelItem[I].FindData.dwFileAttributes & FA_DIREC)
				DirCount++;
			else
			{
				FileCount++;
				__int64 CurSize=MKUINT64(PanelItem[I].FindData.nFileSizeHigh,PanelItem[I].FindData.nFileSizeLow);
				FileSize+=CurSize;

				if (PanelItem[I].PackSize==0 && PanelItem[I].PackSizeHigh==0)
					CompressedFileSize+=CurSize;
				else
				{
					__int64 AddSize=MKUINT64(PanelItem[I].PackSizeHigh,PanelItem[I].PackSize);
					CompressedFileSize+=AddSize;
				}
			}
		}
	}

	if (PanelItem!=NULL)
		FarFreeDirList(PanelItem);

	return(ExitCode);
}

/*
  Функция CheckFolder возвращает одно состояний тестируемого каталога:

    CHKFLD_NOTFOUND   (2) - нет такого
    CHKFLD_NOTEMPTY   (1) - не пусто
    CHKFLD_EMPTY      (0) - пусто
    CHKFLD_NOTACCESS (-1) - нет доступа
    CHKFLD_ERROR     (-2) - ошибка (параметры - дерьмо или нехватило памяти для выделения промежуточных буферов)
*/
int CheckFolder(const char *Path)
{
	if (!(Path && *Path)) // проверка на вшивость
		return CHKFLD_ERROR;

	int LenFindPath=Max((int)strlen(Path),2048)+8;
	char *FindPath=(char *)alloca(LenFindPath); // здесь alloca - чтобы _точно_ хватило на все про все.

	if (!FindPath)
		return CHKFLD_ERROR;

	HANDLE FindHandle;
	WIN32_FIND_DATA fdata;
	int Done=FALSE;
	// сообразим маску для поиска.
	strcpy(FindPath,Path);
	AddEndSlash(FindPath);
	strcat(FindPath,"*.*");

	// первая проверка - че-нить считать можем?
	if ((FindHandle=FAR_FindFirstFile(FindPath,&fdata)) == INVALID_HANDLE_VALUE)
	{
		GuardLastError lstError;

		if (lstError.Get() == ERROR_FILE_NOT_FOUND)
			return CHKFLD_EMPTY;

		// собственно... не факт, что диск не читаем, т.к. на чистом диске в корне нету даже "."
		// поэтому посмотрим на Root
		GetPathRootOne(Path,FindPath);

		if (!strcmp(Path,FindPath))
		{
			// проверка атрибутов гарантировано скажет - это бага BugZ#743 или пустой корень диска.
			if (GetFileAttributes(FindPath)!=INVALID_FILE_ATTRIBUTES)
			{
				if (lstError.Get() == ERROR_ACCESS_DENIED)
					return CHKFLD_NOTACCESS;

				return CHKFLD_EMPTY;
			}
		}

		strcpy(FindPath,Path);

		if (CheckShortcutFolder(FindPath,LenFindPath,FALSE,TRUE))
		{
			if (strcmp(Path,FindPath))
				return CHKFLD_NOTFOUND;

#if 0
			DWORD Attr=GetFileAttributes(Path);

			if (Attr != INVALID_FILE_ATTRIBUTES && (Attr&FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT)
			{
				ConvertNameToFull(Path,FindPath,LenFindPath);
				GetPathRoot(FindPath,FindPath);

				// проверка атрибутов гарантировано скажет - это бага BugZ#743 или пустой корень диска.
				if (GetFileAttributes(FindPath)!=INVALID_FILE_ATTRIBUTES)
					return CHKFLD_EMPTY;
			}

#endif
		}

		return CHKFLD_NOTACCESS;
	}

	// Ок. Что-то есть. Попробуем ответить на вопрос "путой каталог?"
	while (!Done)
	{
		if (fdata.cFileName[0] == '.' && (fdata.cFileName[1] == 0 || fdata.cFileName[1] == '.' && fdata.cFileName[2] == 0))
			; // игнорируем "." и ".."
		else
		{
			// что-то есть, отличное от "." и ".." - каталог не пуст
			FAR_FindClose(FindHandle);
			return CHKFLD_NOTEMPTY;
		}

		Done=!FAR_FindNextFile(FindHandle,&fdata);
	}

	// однозначно каталог пуст
	FAR_FindClose(FindHandle);
	return CHKFLD_EMPTY;
}

char* FarMSG(int MsgID)
{
	return(Lang.GetMsg(MsgID));
}


BOOL GetDiskSize(char *Root,unsigned __int64 *TotalSize,unsigned __int64 *TotalFree,unsigned __int64 *UserFree)
{
	typedef BOOL (WINAPI *GETDISKFREESPACEEX)(
	    LPCTSTR lpDirectoryName,
	    PULARGE_INTEGER lpFreeBytesAvailableToCaller,
	    PULARGE_INTEGER lpTotalNumberOfBytes,
	    PULARGE_INTEGER lpTotalNumberOfFreeBytes
	);
	static GETDISKFREESPACEEX pGetDiskFreeSpaceEx=NULL;
	static int LoadAttempt=FALSE;
	int ExitCode=0;
#if 0
	// пока оставим
	ULARGE_INTEGER uiTotalSize,uiTotalFree,uiUserFree;
	uiUserFree.u.LowPart=uiUserFree.u.HighPart=0;
	uiTotalSize.u.LowPart=uiTotalSize.u.HighPart=0;
	uiTotalFree.u.LowPart=uiTotalFree.u.HighPart=0;

	if (!LoadAttempt && pGetDiskFreeSpaceEx==NULL)
	{
		HMODULE hKernel=GetModuleHandle("kernel32.dll");

		if (hKernel!=NULL)
			pGetDiskFreeSpaceEx=(GETDISKFREESPACEEX)GetProcAddress(hKernel,"GetDiskFreeSpaceExA");

		LoadAttempt=TRUE;
	}

	if (pGetDiskFreeSpaceEx!=NULL)
	{
		ExitCode=pGetDiskFreeSpaceEx(Root,&uiUserFree,&uiTotalSize,&uiTotalFree);

		if (uiUserFree.u.HighPart>uiTotalFree.u.HighPart)
			uiUserFree.u=uiTotalFree.u;
	}

	if (pGetDiskFreeSpaceEx==NULL || ExitCode==0 ||
	        uiTotalSize.u.HighPart==0 && uiTotalSize.u.LowPart==0)
	{
		DWORD SectorsPerCluster,BytesPerSector,FreeClusters,Clusters;
		ExitCode=GetDiskFreeSpace(Root,&SectorsPerCluster,&BytesPerSector,
		                          &FreeClusters,&Clusters);
		uiTotalSize.u.LowPart=SectorsPerCluster*BytesPerSector*Clusters;
		uiTotalSize.u.HighPart=0;
		uiTotalFree.u.LowPart=SectorsPerCluster*BytesPerSector*FreeClusters;
		uiTotalFree.u.HighPart=0;
		uiUserFree.u=uiTotalFree.u;
	}

	*TotalSize=MKUINT64(uiTotalSize.u.HighPart,uiTotalSize.u.LowPart);
	*TotalFree=MKUINT64(uiTotalFree.u.HighPart,uiTotalFree.u.LowPart);
	*UserFree=MKUINT64(uiUserFree.u.HighPart,uiUserFree.u.LowPart);
#else
	unsigned __int64 uiTotalSize,uiTotalFree,uiUserFree;
	uiUserFree=_i64(0);
	uiTotalSize=_i64(0);
	uiTotalFree=_i64(0);

	if (!LoadAttempt && pGetDiskFreeSpaceEx==NULL)
	{
		HMODULE hKernel=GetModuleHandle("kernel32.dll");

		if (hKernel!=NULL)
			pGetDiskFreeSpaceEx=(GETDISKFREESPACEEX)GetProcAddress(hKernel,"GetDiskFreeSpaceExA");

		LoadAttempt=TRUE;
	}

	if (pGetDiskFreeSpaceEx!=NULL)
		ExitCode=pGetDiskFreeSpaceEx(Root,(PULARGE_INTEGER)&uiUserFree,(PULARGE_INTEGER)&uiTotalSize,(PULARGE_INTEGER)&uiTotalFree);

	if (pGetDiskFreeSpaceEx==NULL || ExitCode==0 || uiTotalSize == _i64(0))
	{
		DWORD SectorsPerCluster,BytesPerSector,FreeClusters,Clusters;
		ExitCode=GetDiskFreeSpace(Root,&SectorsPerCluster,&BytesPerSector,&FreeClusters,&Clusters);
		uiTotalSize=(unsigned __int64)SectorsPerCluster*(unsigned __int64)BytesPerSector*(unsigned __int64)Clusters;
		uiTotalFree=(unsigned __int64)SectorsPerCluster*(unsigned __int64)BytesPerSector*(unsigned __int64)FreeClusters;
		uiUserFree=uiTotalFree;
	}

	if (TotalSize)
		*TotalSize=uiTotalSize;

	if (TotalFree)
		*TotalFree=uiTotalFree;

	if (UserFree)
		*UserFree=uiUserFree;

#endif
	return(ExitCode);
}

int GetClusterSize(char *Root)
{
#ifndef _WIN64
	struct ExtGetDskFreSpc
	{
		WORD ExtFree_Size;
		WORD ExtFree_Level;
		DWORD ExtFree_SectorsPerCluster;
		DWORD ExtFree_BytesPerSector;
		DWORD ExtFree_AvailableClusters;
		DWORD ExtFree_TotalClusters;
		DWORD ExtFree_AvailablePhysSectors;
		DWORD ExtFree_TotalPhysSectors;
		DWORD ExtFree_AvailableAllocationUnits;
		DWORD ExtFree_TotalAllocationUnits;
		DWORD ExtFree_Rsvd[2];
	} DiskInfo;
	struct _DIOC_REGISTERS
	{
		DWORD reg_EBX;
		DWORD reg_EDX;
		DWORD reg_ECX;
		DWORD reg_EAX;
		DWORD reg_EDI;
		DWORD reg_ESI;
		DWORD reg_Flags;
	} reg;
	BOOL fResult;
	DWORD cb;

	if (WinVer.dwPlatformId!=VER_PLATFORM_WIN32_WINDOWS ||
	        WinVer.dwBuildNumber<0x04000457)
		return(0);

	HANDLE hDevice = FAR_CreateFile("\\\\.\\vwin32", 0, 0, NULL, 0,
	                                FILE_FLAG_DELETE_ON_CLOSE, NULL);

	if (hDevice==INVALID_HANDLE_VALUE)
		return(0);

	DiskInfo.ExtFree_Level=0;
	reg.reg_EAX = 0x7303;
	reg.reg_EDX = (DWORD)(DWORD_PTR)Root;
	reg.reg_EDI = (DWORD)(DWORD_PTR)&DiskInfo;
	reg.reg_ECX = sizeof(DiskInfo);
	reg.reg_Flags = 0x0001;
	fResult=DeviceIoControl(hDevice,6,&reg,sizeof(reg),&reg,sizeof(reg),&cb,0);
	CloseHandle(hDevice);

	if (!fResult || (reg.reg_Flags & 0x0001))
		return(0);

	return(DiskInfo.ExtFree_SectorsPerCluster*DiskInfo.ExtFree_BytesPerSector);
#else
	return 0;
#endif
}



/* $ 02.03.2001 IS
   Расширение переменных среды
   Вынесена в качестве самостоятельной вместо прямого вызова
     ExpandEnvironmentStrings.
   src и dest могут быть одинаковыми
*/
DWORD WINAPI ExpandEnvironmentStr(const char *src, char *dest, size_t size)
{
	DWORD ret=0;

	if (size && src && dest)
	{
		char *tmpDest=(char *)alloca(size),
		              *tmpSrc=(char *)alloca(strlen(src)+1);

		if (tmpDest && tmpSrc)
		{
			FAR_OemToChar(src, tmpSrc);
			DWORD Len = ExpandEnvironmentStrings(tmpSrc,tmpDest,(DWORD)size);

			if (Len && Len <= size)
			{
				const char *ptrsrc=src;
				char *ptrdest=tmpDest;

				//AY: Обрабатываем каждый %var% отдельно дабы не делать лишних OEM2ANSI
				//    и назад перекодировок.
				//    Проверки на size не нужны так как за нас это уже
				//    проверил ExpandEnvironmentStrings().
				while (1)
				{
					while (*ptrsrc && *ptrsrc!='%')
						*(ptrdest++) = *(ptrsrc++);

					if (*ptrsrc == '%')
					{
						const char *ptrnext=strchr(ptrsrc+1,'%');

						if (ptrnext)
						{
							Len = (DWORD)(ptrnext-ptrsrc+1);
							FAR_OemToCharBuff(ptrsrc,tmpSrc,Len);
							tmpSrc[Len]=0;
							Len = ExpandEnvironmentStrings(tmpSrc,ptrdest,(DWORD)(size-(ptrdest-tmpDest)));

							//AY: Был ли это реальный %var%?
							if (Len && strcmp(tmpSrc,ptrdest))
							{
								FAR_CharToOem(ptrdest, ptrdest);
								ptrdest += strlen(ptrdest);
								ptrsrc = ptrnext+1;
							}
							else
								*(ptrdest++) = *(ptrsrc++);
						}
						else
							*(ptrdest++) = *(ptrsrc++);
					}

					if (!*ptrsrc)
					{
						*ptrdest = 0;
						break;
					}
				}

				xstrncpy(dest, tmpDest, size-1);
			}
			else
			{
				xstrncpy(dest, src, size-1);
			}

			ret=(DWORD)strlen(dest);
		}
	}

	return ret;
}

#if 0
/*
In: "C:\WINNT\SYSTEM32\FOO.TXT", "%SystemRoot%"
Out: "%SystemRoot%\SYSTEM32\FOO.TXT"
*/
BOOL UnExpandEnvString(const char *Path, const char *EnvVar, char* Dest, int DestSize)
{
	int I;
	char Temp[NM*2];
	Temp[0] = 0;
	ExpandEnvironmentStr(EnvVar, Temp, sizeof(Temp));
	I = strlen(Temp);

	if (CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, Temp, I, Path, I) == 2)
	{
		if (strlen(Path)-I+strlen(EnvVar) < DestSize)
		{
			xstrncpy(Dest, EnvVar, DestSize-1);
			xstrncat(Dest, Path + I, DestSize-1);
			return TRUE;
		}
	}

	return FALSE;
}

BOOL PathUnExpandEnvStr(const char *Path, char* Dest, int DestSize)
{
	const char *StdEnv[]=
	{
		"%TEMP%",               // C:\Documents and Settings\Administrator\Local Settings\Temp
		"%APPDATA%",            // C:\Documents and Settings\Administrator\Application Data
		"%USERPROFILE%",        // C:\Documents and Settings\Administrator
		"%ALLUSERSPROFILE%",    // C:\Documents and Settings\All Users
		"%CommonProgramFiles%", // C:\Program Files\Common Files
		"%ProgramFiles%",       // C:\Program Files
		"%SystemRoot%",         // C:\WINNT
	};

	for (int I=0; I < sizeof(StdEnv)/sizeof(StdEnv[0]); ++I)
	{
		if (UnExpandEnvironmentString(Path, StdEnv[I], Dest, DestSize))
			return TRUE;
	}

	xstrncpy(Dest, Path, DestSize-1);
	return FALSE;
}
#endif

/* $ 10.09.2000 tran
   FSF/FarRecurseSearch */
/* $ 30.07.2001 IS
     1. Проверяем правильность параметров.
     2. Теперь обработка каталогов не зависит от маски файлов
     3. Маска может быть стандартного фаровского вида (со скобками,
        перечислением и пр.). Может быть несколько масок файлов, разделенных
        запятыми или точкой с запятой, можно указывать маски исключения,
        можно заключать маски в кавычки. Короче, все как и должно быть :-)
*/
void WINAPI FarRecursiveSearch(const char *InitDir,const char *Mask,FRSUSERFUNC Func,DWORD Flags,void *Param)
{
	if (Func && InitDir && *InitDir && Mask && *Mask)
	{
		CFileMask FMask;

		if (!FMask.Set(Mask, FMF_SILENT)) return;

		Flags=Flags&0x000000FF; // только младший байт!
		ScanTree ScTree(Flags & FRS_RETUPDIR,Flags & FRS_RECUR, Flags & FRS_SCANSYMLINK);
		WIN32_FIND_DATA FindData;
		char FullName[NM];
		ScTree.SetFindPath(InitDir,"*");

		while (ScTree.GetNextName(&FindData,FullName, sizeof(FullName)-1))
		{
			if ((FMask.Compare(FindData.cFileName) || FMask.Compare(FindData.cAlternateFileName)) &&
			        Func(&FindData,FullName,Param) == 0)
				break;
		}
	}
}
/* IS $ */
/* tran 10.09.2000 $ */

/* $ 14.09.2000 SVS
 + Функция FarMkTemp - получение имени временного файла с полным путем.
    Dest - приемник результата (должен быть достаточно большим, например NM
    Template - шаблон по правилам функции mktemp, например "FarTmpXXXXXX"
   Вернет либо NULL, либо указатель на Dest.
*/
/* $ 18.09.2000 SVS
  Не ту функцию впихнул :-)))
*/
/* $ 25.10.2000 IS
 ! Заменил mktemp на вызов соответствующей апишной функции, т.к. предыдущий
   вариант приводил к ошибке (заметили на Multiarc'е)
   Параметр Prefix - строка, указывающая на первые символы имени временного
   файла. Используются только первые 3 символа из этой строки.
*/
char* WINAPI FarMkTemp(char *Dest, const char *Prefix)
{
	return FarMkTempEx(Dest,Prefix,TRUE);
}

/*
             v - точка
   prefXXX X X XXX
       \ / ^   ^^^\ PID + TID
        |  \------/
        |
        +---------- [0A-Z]
*/
char* FarMkTempEx(char *Dest, const char *Prefix, BOOL WithPath)
{
	if (Dest)
	{
		if (!(Prefix && *Prefix))
			Prefix="FTMP";

		size_t pathlen = 0;
		char TempName[NM];
		TempName[0]=0;

		if (WithPath)
			pathlen = strlen(strcpy(TempName,Opt.TempPath));

		if (pathlen < sizeof(TempName)-12)  // else - invalid name!
		{
			strcpy(&TempName[pathlen], "0000XXXXXXXX");
			memcpy(&TempName[pathlen], Prefix, Min((int)strlen(Prefix),4));

			if (farmktemp(TempName)!=NULL)
			{
				strcpy(Dest,strupr(TempName));
				return Dest;
			}
		}
	}

	*Dest = 0;  // PARANOYA
	return NULL;
}
/* IS $ */
/* SVS 18.09.2000 $ */
/* SVS $ */

//Сделано для удаления возвращенного PasteFromClipboard
void WINAPI DeleteBuffer(void *Buffer)
{
	if (Buffer) xf_free(Buffer);
}

// Получить из имени диска RemoteName
char* DriveLocalToRemoteName(int DriveType,char Letter,char *Dest)
{
	int NetPathShown=FALSE, IsOK=FALSE;
	char LocalName[8]=" :\0\0\0",RemoteName[NM];
	DWORD RemoteNameSize=sizeof(RemoteName);
	*LocalName=Letter;
	*Dest=0;

	if (DriveType == DRIVE_UNKNOWN)
	{
		LocalName[2]='\\';
		DriveType = FAR_GetDriveType(LocalName);
		LocalName[2]=0;
	}

	if (DriveType==DRIVE_REMOTE)
	{
		SetFileApisTo(APIS2ANSI);

		if (WNetGetConnection(LocalName,RemoteName,&RemoteNameSize)==NO_ERROR)
		{
			NetPathShown=TRUE;
			IsOK=TRUE;
		}

		SetFileApisTo(APIS2OEM);
	}

	if (!NetPathShown)
		if (GetSubstName(DriveType,LocalName,RemoteName,sizeof(RemoteName)))
			IsOK=TRUE;

	if (IsOK)
	{
		FAR_CharToOem(RemoteName,RemoteName);
		strcpy(Dest,RemoteName);
	}

	return Dest;
}

/*
  FarGetLogicalDrives
  оболочка вокруг GetLogicalDrives, с учетом скрытых логических дисков
  HKCU\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer
  NoDrives:DWORD
    Последние 26 бит определяют буквы дисков от A до Z (отсчет справа налево).
    Диск виден при установленном 0 и скрыт при значении 1.
    Диск A представлен правой последней цифрой при двоичном представлении.
    Например, значение 00000000000000000000010101(0x7h)
    скрывает диски A, C, и E
*/
DWORD WINAPI FarGetLogicalDrives(void)
{
	static DWORD LogicalDrivesMask = 0;
	DWORD NoDrives=0;

	if ((!Opt.RememberLogicalDrives) || (LogicalDrivesMask==0))
		LogicalDrivesMask=GetLogicalDrives();

	if (!Opt.Policies.ShowHiddenDrives)
	{
		HKEY hKey;

		if (RegOpenKeyEx(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS && hKey)
		{
			int ExitCode;
			DWORD Type,Size=sizeof(NoDrives);
			ExitCode=RegQueryValueEx(hKey,"NoDrives",0,&Type,(BYTE *)&NoDrives,&Size);
			RegCloseKey(hKey);

			if (ExitCode != ERROR_SUCCESS)
				NoDrives=0;
		}
	}

	return LogicalDrivesMask&(~NoDrives);
}

/* $ 13.10.2002 IS
   Переписано заново с учетом того, чтобы избавиться от strstr и
   GetCommaWord - от них только проблемы, в частности, не работала раскраска
   по маске "%pathext%,*.lnk,*.pif,*.awk,*.pln", если %pathext% содержала
   ".pl", т.к. эта подстрока входила в "*.pln"
*/

// Преобразование корявого формата PATHEXT в ФАРовский :-)
// Функции передается нужные расширения, она лишь добавляет то, что есть
// в %PATHEXT%
// IS: Сравнений на совпадение очередной маски с тем, что имеется в Dest
// IS: не делается, т.к. дубли сами уберутся при компиляции маски
char *Add_PATHEXT(char *Dest)
{
	char Buf[1024];
	int curpos=(int)strlen(Dest)-1, l;
	UserDefinedList MaskList(0,0,ULF_UNIQUE);

	if (GetEnvironmentVariable("PATHEXT",Buf,sizeof(Buf)) && MaskList.Set(Buf))
	{
		/* $ 13.10.2002 IS проверка на '|' (маски исключения) */
		if (*Dest && Dest[curpos]!=',' && Dest[curpos]!='|')
		{
			Dest[curpos+1]=',';
			Dest[curpos+2]=0;
			++curpos;
		}

		/* IS $ */
		++curpos;
		const char *Ptr;
		MaskList.Reset();

		while (NULL!=(Ptr=MaskList.GetNext()))
		{
			Dest[curpos]='*';
			++curpos;
			l=(int)strlen(Ptr);
			memcpy(Dest+curpos,Ptr,l);
			curpos+=l;
			Dest[curpos]=',';
			++curpos;
			Dest[curpos]=0;
		}

		--curpos;
	}

	// лишняя запятая - в морг!
	/* $ 13.10.2002 IS Оптимизация по скорости */
	if (Dest[curpos] == ',')
		Dest[curpos]=0;

	/* IS $ */
	return Dest;
}
/* IS 13.10.2002 $ */

void CreatePath(char *Path)
{
	char *ChPtr = Path;
	char *DirPart = Path;
	BOOL bEnd = FALSE;

	while (TRUE)
	{
		if ((*ChPtr == 0) || (*ChPtr == '\\'))
		{
			if (*ChPtr == 0)
				bEnd = TRUE;

			*ChPtr = 0;

			if (Opt.CreateUppercaseFolders && !IsCaseMixed(DirPart) && GetFileAttributes(Path) == (DWORD)-1)
				LocalStrupr(DirPart);

			if (CreateDirectory(Path, NULL))
				TreeList::AddTreeName(Path);

			if (bEnd)
				break;

			*ChPtr = '\\';
			DirPart = ChPtr+1;
		}

		ChPtr++;
	}
}

int PathMayBeAbsolute(const char *Path)
{
	return (Path &&
	        (
//             (isalpha(*Path) && Path[1]==':' && Path[2]) ||
//           Абсолютный путь не обязательно после ":" содержит еще что-то.
//           Бывают случаи и "а:"
	            (isalpha(*Path) && Path[1]==':') ||
	            (Path[0]=='\\'  && Path[1]=='\\') ||
	            (Path[0]=='/'   && Path[1]=='/')
	        )
	       );
}

BOOL IsNetworkPath(const char *Path)
{
	return (Path && Path[0] == '\\' && Path[1] == '\\' && Path[2] != '\\' && strchr(Path+2,'\\'));
}

BOOL IsLocalPath(const char *Path)
{
	return (Path && isalpha(*Path) && Path[1]==':' && Path[2]);
}

BOOL IsLocalRootPath(const char *Path)
{
	return (Path && isalpha(*Path) && Path[1]==':' && Path[2] == '\\' && !Path[3]);
}

bool PathPrefix(const char *Path)
{
	/*
	  \\?\
	  \\.\
	  \??\
	*/
	return Path && Path[0] == '\\' && (Path[1] == '\\' || Path[1] == '?') && (Path[2] == '?' || Path[2] == '.') && Path[3] == '\\';
}

BOOL IsLocalPrefixPath(const char *Path)
{
	return PathPrefix(Path) && isalpha(Path[4]) && Path[5] == ':' && Path[6] == '\\';
}

BOOL IsLocalVolumePath(const char *Path)
{
	return PathPrefix(Path) && !strnicmp(&Path[4],"Volume{",7) && Path[47] == '}';
}

BOOL IsLocalVolumeRootPath(const char *Path)
{
	return IsLocalVolumePath(Path) && !Path[48];
}

// Косметические преобразования строки пути.
// CheckFullPath используется в FCTL_SET[ANOTHER]PANELDIR
char* PrepareDiskPath(char *Path,int MaxSize,BOOL CheckFullPath)
{
	_CHANGEDIR(CleverSysLog clv("PrepareDiskPath"));
	_CHANGEDIR(SysLog("(Path=\"%s\", MaxSize=%d, CheckFullPath=%d)",Path,MaxSize,CheckFullPath));

	if (Path && *Path)
	{
		if (((isalpha(Path[0]) && Path[1]==':') || (Path[0]=='\\' && Path[1]=='\\')))
		{
			if (CheckFullPath)
			{
				char NPath[1024];
				*NPath=0;
				_CHANGEDIR(SysLog("CALL RawConvertShortNameToLongName()"));
				RawConvertShortNameToLongName(Path,NPath,sizeof(NPath));

				if (*NPath)
					xstrncpy(Path,NPath,MaxSize);
			}

			if (Path[0]=='\\' && Path[1]=='\\')
			{
				if (IsLocalPrefixPath(Path))
					Path[4] = toupper(Path[4]);
				else
				{
					char *ptr=&Path[2];

					while (*ptr && *ptr!='\\')
					{
						*ptr=toupper(*ptr);
						ptr++;
					}
				}
			}
			else
				Path[0]=toupper(Path[0]);
		}
	}

	return Path;
}

/*
   Проверка пути или хост-файла на существование
   Если идет проверка пути (IsHostFile=FALSE), то будет
   предпринята попытка найти ближайший путь. Результат попытки
   возвращается в переданном TestPath.

   Return: 0 - бЯда.
           1 - ОБИ!,
          -1 - Почти что ОБИ, но ProcessPluginEvent вернул TRUE
   TestPath может быть пустым, тогда просто исполним ProcessPluginEvent()

*/
int CheckShortcutFolder(char *TestPath,int LengthPath,int IsHostFile, BOOL Silent)
{
	if (TestPath && *TestPath && GetFileAttributes(TestPath) == -1)
	{
		char Target[NM];
		int FoundPath=0;
		xstrncpy(Target, TestPath, sizeof(Target)-1);
		TruncPathStr(Target, ScrX-16);

		if (IsHostFile)
		{
			SetLastError(ERROR_FILE_NOT_FOUND);

			if (!Silent)
				Message(MSG_WARNING | MSG_ERRORTYPE, 1, MSG(MError), Target, MSG(MOk));
		}
		else // попытка найти!
		{
			SetLastError(ERROR_PATH_NOT_FOUND);

			if (Silent || Message(MSG_WARNING | MSG_ERRORTYPE, 2, MSG(MError), Target, MSG(MNeedNearPath), MSG(MHYes),MSG(MHNo)) == 0)
			{
				char *Ptr;
				char TestPathTemp[1024];
				xstrncpy(TestPathTemp,TestPath,sizeof(TestPathTemp)-1);

				while ((Ptr=strrchr(TestPathTemp,'\\')) != NULL)
				{
					*Ptr=0;

					if (GetFileAttributes(TestPathTemp) != -1)
					{
						int ChkFld=CheckFolder(TestPathTemp);

						if (ChkFld > CHKFLD_ERROR && ChkFld < CHKFLD_NOTFOUND)
						{
							if (!(TestPath[0] == '\\' && TestPath[1] == '\\' && TestPathTemp[1] == 0))
							{
								xstrncpy(TestPath,TestPathTemp,LengthPath);

								if (strlen(TestPath) == 2) // для случая "C:", иначе попадем в текущий каталог диска C:
									AddEndSlash(TestPath);

								FoundPath=1;
							}

							break;
						}
					}
				}
			}
		}

		if (!FoundPath)
			return 0;
	}

	if (CtrlObject->Cp()->ActivePanel->ProcessPluginEvent(FE_CLOSE,NULL))
		return -1;

	return 1;
}

BOOL IsDiskInDrive(const char *Root)
{
	char   VolName[256];
	char   Drive[NM];
	DWORD  MaxComSize;
	DWORD  Flags;
	char   FS[256];
	strcpy(Drive,Root);
	AddEndSlash(Drive);
	//UINT ErrMode = SetErrorMode ( SEM_FAILCRITICALERRORS );
	//если не сделать SetErrorMode - выскочит стандартное окошко "Drive Not Ready"
	BOOL Res = GetVolumeInformation(Drive, VolName, sizeof(VolName), NULL, &MaxComSize, &Flags, FS, sizeof(FS));
	//SetErrorMode(ErrMode);
	return Res;
}

void ShellUpdatePanels(Panel *SrcPanel,BOOL NeedSetUpADir)
{
	if (!SrcPanel)
		SrcPanel=CtrlObject->Cp()->ActivePanel;

	Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(SrcPanel);

	switch (SrcPanel->GetType())
	{
		case QVIEW_PANEL:
		case INFO_PANEL:
			SrcPanel=CtrlObject->Cp()->GetAnotherPanel(AnotherPanel=SrcPanel);
	}

	int AnotherType=AnotherPanel->GetType();

	if (AnotherType!=QVIEW_PANEL && AnotherType!=INFO_PANEL)
	{
		if (NeedSetUpADir)
		{
			char CurDir[2048];
			SrcPanel->GetCurDir(CurDir);
			AnotherPanel->SetCurDir(CurDir,TRUE);
			AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
		}
		else
		{
			// TODO: ???
			//if(AnotherPanel->NeedUpdatePanel(SrcPanel))
			//  AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
			//else
			{
				// Сбросим время обновления панели. Если там есть нотификация - обновится сама.
				if (AnotherType==FILE_PANEL)
					((FileList *)AnotherPanel)->ResetLastUpdateTime();

				AnotherPanel->UpdateIfChanged(UIC_UPDATE_NORMAL);
			}
		}
	}

	SrcPanel->Update(UPDATE_KEEP_SELECTION);

	if (AnotherType==QVIEW_PANEL)
		AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);

	CtrlObject->Cp()->Redraw();
}

int CheckUpdateAnotherPanel(Panel *SrcPanel,const char *SelName)
{
	if (!SrcPanel)
		SrcPanel=CtrlObject->Cp()->ActivePanel;

	Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(SrcPanel);
	AnotherPanel->CloseFile();

	if (AnotherPanel->GetMode() == NORMAL_PANEL)
	{
		char AnotherCurDir[2048];
		char FullName[2058];
		AnotherPanel->GetCurDir(AnotherCurDir);
		AddEndSlash(AnotherCurDir);

		if (ConvertNameToFull(SelName,FullName, sizeof(FullName)) >= sizeof(FullName))
			return -1;

		AddEndSlash(FullName);

		if (strstr(AnotherCurDir,FullName))
		{
			((FileList*)AnotherPanel)->CloseChangeNotification();
			return TRUE;
		}
	}

	return FALSE;
}

/* $ 26.10.2003 KM
   Исправление и изменение внутренней логики
*/
/* $ 21.09.2003 KM
   Трансформация строки по заданному типу.
*/
void Transform(unsigned char *Buffer,int &BufLen,const char *ConvStr,char TransformType)
{
	int I,J,L,N;
	char *stop,HexNum[3];

	switch (TransformType)
	{
		case 'X': // Convert common string to hexadecimal string representation
		{
			*(char *)Buffer=0;
			L=(int)strlen(ConvStr);
			N=Min((BufLen-1)/2,L);

			for (I=0,J=0; I<N; I++,J+=2)
			{
				// "%02X" - два выходящих символа на каждый один входящий
				sprintf((char *)Buffer+J,"%02X",ConvStr[I]);
				BufLen=J+1;
			}

			RemoveTrailingSpaces((char *)Buffer);
			break;
		}
		case 'S': // Convert hexadecimal string representation to common string
		{
			*(char *)Buffer=0;
			L=(int)strlen(ConvStr);
			char *NewStr=new char[L+1];

			if (NewStr==NULL)
				return;

			// Подготовка временной строки
			memset(NewStr,0,L+1);

			// Обработка hex-строки: убираем пробелы между байтами.
			for (I=0,J=0; ConvStr[I]; ++I)
			{
				if (ConvStr[I]==' ')
					continue;

				NewStr[J]=ConvStr[I];
				++J;
			}

			L=(int)strlen(NewStr);
			N=Min(BufLen-1,L);

			for (I=0,J=0; I<N; I+=2,J++)
			{
				// "HH" - два входящих символа на каждый один выходящий
				xstrncpy(HexNum,&NewStr[I],2);
				HexNum[2]=0;
				unsigned long value=strtoul(HexNum,&stop,16);
				Buffer[J]=static_cast<unsigned char>(value);
				BufLen=J+1;
			}

			Buffer[J]=0;
			delete []NewStr;
			break;
		}
		default:
			break;
	}
}

/*
 возвращает PipeFound
*/
int PartCmdLine(const char *CmdStr,char *NewCmdStr,int SizeNewCmdStr,char *NewCmdPar,int SizeNewCmdPar)
{
	int PipeFound = FALSE;
	int QuoteFound = FALSE;
	ExpandEnvironmentStr(CmdStr, NewCmdStr, SizeNewCmdStr);
	RemoveExternalSpaces(NewCmdStr);
	char *CmdPtr = NewCmdStr;
	char *ParPtr = NULL;
	// Разделим собственно команду для исполнения и параметры.
	// При этом заодно определим наличие символов переопределения потоков
	// Работаем с учетом кавычек. Т.е. пайп в кавычках - не пайп.

	while (*CmdPtr)
	{
		if (*CmdPtr == '"')
			QuoteFound = !QuoteFound;

		if (!QuoteFound && *CmdPtr == '^') // для "^>"
		{
			CmdPtr++;
		}
		else if (!QuoteFound && CmdPtr != NewCmdStr)
		{
			if (*CmdPtr == '>' || *CmdPtr == '<' ||
			        *CmdPtr == '|' || *CmdPtr == ' ' ||
			        *CmdPtr == '/' ||      // вариант "far.exe/?"
			        (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT && *CmdPtr == '&') // Для НТ/2к обработаем разделитель команд
			   )
			{
				if (!ParPtr)
					ParPtr = CmdPtr;

				if (*CmdPtr != ' ' && *CmdPtr != '/')
					PipeFound = TRUE;
			}
		}

		if (ParPtr && PipeFound)
			// Нам больше ничего не надо узнавать
			break;

		CmdPtr++;
	}

	if (ParPtr) // Мы нашли параметры и отделяем мух от котлет
	{
		/* $ 05.04.2005 AY: это вроде как остатки прошлого, нафиг нам это надо */
		//if (*ParPtr=='/')
		//{
		//*NewCmdPar=0x20;
		//xstrncpy(NewCmdPar+1, ParPtr, SizeNewCmdPar-2);
		//}
		//else
		/* AY $ */
		if (*ParPtr == 0x20) //AY: первый пробел между командой и параметрами не нужен,
			*(ParPtr++)=0;     //    он добавляется заново в Execute.

		xstrncpy(NewCmdPar, ParPtr, SizeNewCmdPar-1);
		*ParPtr = 0;
	}

	Unquote(NewCmdStr);
	//RemoveExternalSpaces(NewCmdStr);
	//AY: Вполне легальное имя файла " .exe"
	//    а вот в конце пробелы уже быть не могут.
	//RemoveTrailingSpaces(NewCmdStr);
	//AY: это является лишней интеллигенцией, например файл "ааа .exe" могут запустить как "ааа ".
	return PipeFound;
}


BOOL ProcessOSAliases(char *Str,int SizeStr)
{
	if (WinVer.dwPlatformId != VER_PLATFORM_WIN32_NT)
		return FALSE;

	typedef DWORD (WINAPI *PGETCONSOLEALIAS)(
	    LPTSTR lpSource,
	    LPTSTR lpTargetBuffer,
	    DWORD TargetBufferLength,
	    LPTSTR lpExeName
	);
	static PGETCONSOLEALIAS GetConsoleAlias=NULL;

	if (!GetConsoleAlias)
	{
		GetConsoleAlias = (PGETCONSOLEALIAS)GetProcAddress(GetModuleHandle("kernel32"),"GetConsoleAliasA");

		if (!GetConsoleAlias)
			return FALSE;
	}

	char NewCmdStr[4096];
	char NewCmdPar[4096];
	*NewCmdStr=0;
	*NewCmdPar=0;
	PartCmdLine(Str,NewCmdStr,sizeof(NewCmdStr),NewCmdPar,sizeof(NewCmdPar));
	char ModuleName[NM];
	GetModuleFileName(NULL,ModuleName,sizeof(ModuleName));
	char* ExeName=PointToName(ModuleName);
	int ret=GetConsoleAlias(NewCmdStr,NewCmdStr,sizeof(NewCmdStr),ExeName);

	if (!ret)
	{
		if (ExpandEnvironmentStr("%COMSPEC%",ModuleName))
		{
			ExeName=PointToName(ModuleName);
			ret=GetConsoleAlias(NewCmdStr,NewCmdStr,sizeof(NewCmdStr),ExeName);
		}
	}

	if (!ret)
		return FALSE;

	if (!ReplaceStrings(NewCmdStr,"$*",NewCmdPar))
	{
		xstrncat(NewCmdStr," ",sizeof(NewCmdStr)-1);
		xstrncat(NewCmdStr,NewCmdPar,sizeof(NewCmdStr)-1);
	}

	xstrncpy(Str,NewCmdStr,SizeStr-1);
	return TRUE;
}

/*
  Например, код для [A|P]Panel.UNCPath
  _MakePath1(CheckCode == MCODE_V_APANEL_UNCPATH?KEY_ALTSHIFTBRACKET:KEY_ALTSHIFTBACKBRACKET,FileName,sizeof(FileName)-1,"")
*/
int _MakePath1(DWORD Key,char *PathName,int PathNameSize, const char *Param2,int ShortNameAsIs)
{
	int RetCode=FALSE;
	int NeedRealName=FALSE;
	*PathName=0;

	switch (Key)
	{
		case KEY_CTRLALTBRACKET:       // Вставить сетевое (UNC) путь из левой панели
		case KEY_CTRLALTBACKBRACKET:   // Вставить сетевое (UNC) путь из правой панели
		case KEY_ALTSHIFTBRACKET:      // Вставить сетевое (UNC) путь из активной панели
		case KEY_ALTSHIFTBACKBRACKET:  // Вставить сетевое (UNC) путь из пассивной панели
			NeedRealName=TRUE;
		case KEY_CTRLBRACKET:          // Вставить путь из левой панели
		case KEY_CTRLBACKBRACKET:      // Вставить путь из правой панели
		case KEY_CTRLSHIFTBRACKET:     // Вставить путь из активной панели
		case KEY_CTRLSHIFTBACKBRACKET: // Вставить путь из пассивной панели
		case KEY_CTRLSHIFTNUMENTER:       // Текущий файл с пасс.панели
		case KEY_SHIFTNUMENTER:           // Текущий файл с актив.панели
		case KEY_CTRLSHIFTENTER:       // Текущий файл с пасс.панели
		case KEY_SHIFTENTER:           // Текущий файл с актив.панели
		{
			Panel *SrcPanel=NULL;
			FilePanels *Cp=CtrlObject->Cp();

			switch (Key)
			{
				case KEY_CTRLALTBRACKET:
				case KEY_CTRLBRACKET:
					SrcPanel=Cp->LeftPanel;
					break;
				case KEY_CTRLALTBACKBRACKET:
				case KEY_CTRLBACKBRACKET:
					SrcPanel=Cp->RightPanel;
					break;
				case KEY_SHIFTNUMENTER:
				case KEY_SHIFTENTER:
				case KEY_ALTSHIFTBRACKET:
				case KEY_CTRLSHIFTBRACKET:
					SrcPanel=Cp->ActivePanel;
					break;
				case KEY_CTRLSHIFTNUMENTER:
				case KEY_CTRLSHIFTENTER:
				case KEY_ALTSHIFTBACKBRACKET:
				case KEY_CTRLSHIFTBACKBRACKET:
					SrcPanel=Cp->GetAnotherPanel(Cp->ActivePanel);
					break;
			}

			if (SrcPanel!=NULL)
			{
				char ShortFileName[NM];

				if (Key == KEY_SHIFTENTER || Key == KEY_CTRLSHIFTENTER || Key == KEY_SHIFTNUMENTER || Key == KEY_CTRLSHIFTNUMENTER)
				{
					SrcPanel->GetCurName(PathName,ShortFileName);

					if (SrcPanel->GetShowShortNamesMode()) // учтем короткость имен :-)
						strcpy(PathName,ShortFileName);
				}
				else
				{
					/* TODO: Здесь нужно учесть, что у TreeList тоже есть путь :-) */
					if (!(SrcPanel->GetType()==FILE_PANEL || SrcPanel->GetType()==TREE_PANEL))
						return(FALSE);

					SrcPanel->GetCurDir(PathName);

					if (SrcPanel->GetMode()!=PLUGIN_PANEL)
					{
						FileList *SrcFilePanel=(FileList *)SrcPanel;
						SrcFilePanel->GetCurDir(PathName);

						if (NeedRealName)
							SrcFilePanel->CreateFullPathName(PathName,ShortFileName,FA_DIREC,PathName,PathNameSize,TRUE,ShortNameAsIs);

						if (SrcFilePanel->GetShowShortNamesMode() && ShortNameAsIs)
							ConvertNameToShort(PathName,PathName,PathNameSize-1);
					}
					else
					{
						FileList *SrcFilePanel=(FileList *)SrcPanel;
						struct OpenPluginInfo Info;
						CtrlObject->Plugins.GetOpenPluginInfo(SrcFilePanel->GetPluginHandle(),&Info);
						FileList::AddPluginPrefix(SrcFilePanel,PathName);
						xstrncat(PathName,NullToEmpty(Info.CurDir),PathNameSize);
					}

					AddEndSlash(PathName);
				}

				if (Opt.QuotedName&QUOTEDNAME_INSERT)
					QuoteSpace(PathName);

				xstrncat(PathName,NullToEmpty(Param2),PathNameSize);
				RetCode=TRUE;
			}
		}
		break;
	}

	return RetCode;
}

const char *CurPath2ComputerName(const char *CurDir, char *ComputerName,int SizeName)
{
	char NetDir[NM*3];
	*ComputerName=0;
	*NetDir=0;

	if (CurDir[0]=='\\' && CurDir[1]=='\\')
		strcpy(NetDir,CurDir);
	else
	{
		char LocalName[3];
		DWORD RemoteNameSize=sizeof(NetDir);
		xstrncpy(LocalName,CurDir,2);
		LocalName[sizeof(LocalName)-1]=0;
		SetFileApisTo(APIS2ANSI);

		if (WNetGetConnection(LocalName,NetDir,&RemoteNameSize)==NO_ERROR)
			FAR_CharToOem(NetDir,NetDir);

		SetFileApisTo(APIS2OEM);
	}

	if (NetDir[0]=='\\' && NetDir[1]=='\\')
	{
		xstrncpy(ComputerName,NetDir+2,SizeName);
		char *EndSlash=strchr(ComputerName,'\\');

		if (EndSlash==NULL)
			*ComputerName=0;
		else
			*EndSlash=0;
	}

	return ComputerName;
}

int CheckDisksProps(const char *SrcPath,const char *DestPath,int CheckedType)
{
	char SrcRoot[NM],DestRoot[NM];
	int SrcDriveType, DestDriveType;
	DWORD SrcVolumeNumber=0, DestVolumeNumber=0;
	char SrcVolumeName[NM], DestVolumeName[NM];
	char  SrcFileSystemName[NM], DestFileSystemName[NM];
	DWORD SrcFileSystemFlags, DestFileSystemFlags;
	DWORD SrcMaximumComponentLength, DestMaximumComponentLength;
	xstrncpy(SrcRoot,SrcPath,sizeof(SrcRoot)-1);
	xstrncpy(DestRoot,DestPath,sizeof(DestRoot)-1);
	ConvertNameToUNC(SrcRoot,sizeof(SrcRoot)-1);
	ConvertNameToUNC(DestRoot,sizeof(DestRoot)-1);
	GetPathRoot(SrcRoot,SrcRoot);
	GetPathRoot(DestRoot,DestRoot);
	SrcDriveType=FAR_GetDriveType(SrcRoot,NULL,TRUE);
	DestDriveType=FAR_GetDriveType(DestRoot,NULL,TRUE);

	if (!GetVolumeInformation(SrcRoot,SrcVolumeName,sizeof(SrcVolumeName),&SrcVolumeNumber,&SrcMaximumComponentLength,&SrcFileSystemFlags,SrcFileSystemName,sizeof(SrcFileSystemName)))
		return(FALSE);

	if (!GetVolumeInformation(DestRoot,DestVolumeName,sizeof(DestVolumeName),&DestVolumeNumber,&DestMaximumComponentLength,&DestFileSystemFlags,DestFileSystemName,sizeof(DestFileSystemName)))
		return(FALSE);

	if (CheckedType == CHECKEDPROPS_ISSAMEDISK)
	{
		if (strpbrk(DestPath,"\\:")==NULL)
			return TRUE;

		if ((SrcRoot[0]=='\\' && SrcRoot[1]=='\\' || DestRoot[0]=='\\' && DestRoot[1]=='\\') &&
		        LocalStricmp(SrcRoot,DestRoot)!=0)
			return FALSE;

		if (*SrcPath==0 || *DestPath==0 || (SrcPath[1]!=':' && DestPath[1]!=':')) //????
			return TRUE;

		if (toupper(DestRoot[0])==toupper(SrcRoot[0]))
			return TRUE;

		unsigned __int64 SrcTotalSize,SrcTotalFree,SrcUserFree;
		unsigned __int64 DestTotalSize,DestTotalFree,DestUserFree;

		if (!GetDiskSize(SrcRoot,&SrcTotalSize,&SrcTotalFree,&SrcUserFree))
			return FALSE;

		if (!GetDiskSize(DestRoot,&DestTotalSize,&DestTotalFree,&DestUserFree))
			return FALSE;

		if (!(SrcVolumeNumber!=0 &&
		        SrcVolumeNumber==DestVolumeNumber &&
		        strcmp(SrcVolumeName,DestVolumeName)==0 &&
		        SrcTotalSize==DestTotalSize))
			return FALSE;
	}
	else if (CheckedType == CHECKEDPROPS_ISDST_ENCRYPTION)
	{
		if (!(DestFileSystemFlags&FILE_SUPPORTS_ENCRYPTION))
			return FALSE;

		if (!(DestDriveType==DRIVE_REMOVABLE || DestDriveType==DRIVE_FIXED || DestDriveType==DRIVE_REMOTE))
			return FALSE;
	}

	return TRUE;
}

void ConvertNameToUNC(char *FileName, int Size)
{
	char Temp[4906];
	ConvertNameToFull(FileName,FileName,Size);
	// Посмотрим на тип файловой системы
	char FileSystemName[NM];
	GetPathRoot(FileName,Temp);

	if (!GetVolumeInformation(Temp,NULL,0,NULL,NULL,NULL,FileSystemName,sizeof(FileSystemName)))
		*FileSystemName=0;

	DWORD uniSize = sizeof(Temp);

	// применяем WNetGetUniversalName для чего угодно, только не для Novell`а
	if (stricmp(FileSystemName,"NWFS") != 0 &&
	        WNetGetUniversalName(FileName, UNIVERSAL_NAME_INFO_LEVEL, &Temp, &uniSize) == NO_ERROR)
	{
		xstrncpy(FileName,((UNIVERSAL_NAME_INFO*)Temp)->lpUniversalName,Size-1);
		FAR_CharToOem(FileName,FileName);
	}
	else if (FileName[1] == ':')
	{
		// BugZ#449 - Неверная работа CtrlAltF с ресурсами Novell DS
		// Здесь, если не получилось получить UniversalName и если это
		// мапленный диск - получаем как для меню выбора дисков
		if (*DriveLocalToRemoteName(DRIVE_UNKNOWN,*FileName,Temp) != 0)
		{
			const char *NamePtr;

			if ((NamePtr=strchr(FileName, '/')) == NULL)
				NamePtr=strchr(FileName, '\\');

			if (NamePtr != NULL)
			{
				AddEndSlash(Temp);
				strcat(Temp,&NamePtr[1]);
			}

			xstrncpy(FileName, Temp, Size-1);
		}
	}

	ConvertNameToReal(FileName,FileName, Size);
}

// Проверка типа файловой системы. Если TargetFS не задан (NULL или ""), то проверяем на "NTFS"
BOOL CheckFileSystem(const char *CurDir,const char *TargetFS)
{
	BOOL Is_FS_Target=FALSE;
	char FileSysName[NM],RootDir[NM*2];
	ConvertNameToFull(CurDir,RootDir, sizeof(RootDir));
	GetPathRoot(RootDir,RootDir);

	if (GetVolumeInformation(RootDir,NULL,0,NULL,NULL,NULL,FileSysName,sizeof(FileSysName)))
		Is_FS_Target=!stricmp(FileSysName,(TargetFS && *TargetFS?TargetFS:"NTFS"))?TRUE:FALSE;

	return Is_FS_Target;
}
