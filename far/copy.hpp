#pragma once

/*
copy.hpp

class ShellCopy -  опирование файлов
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "dizlist.hpp"
#include "udlist.hpp"
#include "flink.hpp"

class Panel;

enum COPY_CODES
{
	COPY_CANCEL,
	COPY_NEXT,
	COPY_NOFILTER,                              // не считать размеры, т.к. файл не прошел по фильтру
	COPY_FAILURE,
	COPY_FAILUREREAD,
	COPY_SUCCESS,
	COPY_SUCCESS_MOVE,
	COPY_RETRY,
};

enum COPY_FLAGS
{
	FCOPY_COPYTONUL               = 0x00000001, // ѕризнак копировани€ в NUL
	FCOPY_CURRENTONLY             = 0x00000002, // “олько текщий?
	FCOPY_ONLYNEWERFILES          = 0x00000004, // Copy only newer files
	FCOPY_OVERWRITENEXT           = 0x00000008, // Overwrite all
	FCOPY_LINK                    = 0x00000010, // создание линков
	FCOPY_MOVE                    = 0x00000040, // перенос/переименование
	FCOPY_DIZREAD                 = 0x00000080, //
	FCOPY_COPYSECURITY            = 0x00000100, // [x] Copy access rights
	FCOPY_NOSHOWMSGLINK           = 0x00000200, // не показывать месаги при ликовании
	FCOPY_VOLMOUNT                = 0x00000400, // операци€ монтированни€ тома
	FCOPY_STREAMSKIP              = 0x00000800, // потоки
	FCOPY_STREAMALL               = 0x00001000, // потоки
	FCOPY_SKIPSETATTRFLD          = 0x00002000, // больше не пытатьс€ ставить атрибуты дл€ каталогов - когда нажали Skip All
	FCOPY_COPYSYMLINKCONTENTS     = 0x00004000, //  опировать содержимое симолических св€зей?
	FCOPY_COPYPARENTSECURITY      = 0x00008000, // Ќакладывать родительские права, в случае если мы не копируем права доступа
	FCOPY_LEAVESECURITY           = 0x00010000, // Move: [?] Ќичего не делать с правами доступа
	FCOPY_DECRYPTED_DESTINATION   = 0x00020000, // дл€ криптованных файлов - расшифровывать...
	FCOPY_USESYSTEMCOPY           = 0x00040000, // использовать системную функцию копировани€
	FCOPY_COPYLASTTIME            = 0x10000000, // ѕри копировании в несколько каталогов устанавливаетс€ дл€ последнего.
	FCOPY_UPDATEPPANEL            = 0x80000000, // необходимо обновить пассивную панель
};

class ShellCopy
{
		DWORD Flags;
		Panel *SrcPanel,*DestPanel;
		int SrcPanelMode,DestPanelMode;
		int SrcDriveType,DestDriveType;
		string strSrcDriveRoot;
		string strDestDriveRoot;
		string strDestFSName;
		char   *sddata; // Security
		DizList DestDiz;
		string strDestDizPath;
		char *CopyBuffer;
		int CopyBufferSize;
		string strCopiedName;
		string strRenamedName;
		string strRenamedFilesPath;
		int OvrMode;
		int ReadOnlyOvrMode;
		int ReadOnlyDelMode;
		int SkipMode;          // ...дл€ пропуска при копировании залоченных файлов.
		int SkipEncMode;
		int SkipDeleteMode;
		int SelectedFolderNameLength;
		UserDefinedList DestList;
		// тип создаваемого репарспоинта.
		// при AltF6 будет то, что выбрал юзер в диалоге,
		// в остальных случа€х - RP_EXACTCOPY - как у источника
		ReparsePointTypes RPT;

		COPY_CODES CopyFileTree(const wchar_t *Dest);
		COPY_CODES ShellCopyOneFile(const wchar_t *Src,
		                            const FAR_FIND_DATA_EX &SrcData,
		                            string &strDest,
		                            int KeepPathPos, int Rename);
		COPY_CODES CheckStreams(const wchar_t *Src,const wchar_t *DestPath);
		int  ShellCopyFile(const wchar_t *SrcName,const FAR_FIND_DATA_EX &SrcData,
		                   string &strDestName,DWORD &DestAttr,int Append);
		int  ShellSystemCopy(const wchar_t *SrcName,const wchar_t *DestName,const FAR_FIND_DATA_EX &SrcData);
		int  DeleteAfterMove(const wchar_t *Name,DWORD Attr);
		void SetDestDizPath(const wchar_t *DestPath);
		int  AskOverwrite(const FAR_FIND_DATA_EX &SrcData,const wchar_t *SrcName,const wchar_t *DestName,
		                  DWORD DestAttr,int SameName,int Rename,int AskAppend,
		                  int &Append,string &strNewName,int &RetCode);
		int  GetSecurity(const wchar_t *FileName,SECURITY_ATTRIBUTES &sa);
		int  SetSecurity(const wchar_t *FileName,const SECURITY_ATTRIBUTES &sa);
		int  SetRecursiveSecurity(const wchar_t *FileName,const SECURITY_ATTRIBUTES &sa);
		bool CalcTotalSize();
		bool ShellSetAttr(const wchar_t *Dest,DWORD Attr);
		void CheckUpdatePanel(); // выставл€ет флаг FCOPY_UPDATEPPANEL
	public:
		ShellCopy(Panel *SrcPanel,int Move,int Link,int CurrentOnly,int Ask,
		          int &ToPlugin, const wchar_t *PluginDestPath);
		~ShellCopy();
		static LONG_PTR WINAPI CopyDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2);
};
