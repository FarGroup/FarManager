#pragma once

/*
copy.hpp

class ShellCopy - Копирование файлов
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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

class Panel;
class Dialog;

ENUM(COPY_CODES);
ENUM(ReparsePointTypes);

class ShellCopy
{
public:
	ShellCopy(Panel *SrcPanel,int Move,int Link,int CurrentOnly,int Ask, int &ToPlugin, const wchar_t* PluginDestPath, bool ToSubdir=false);
	~ShellCopy();

private:
	COPY_CODES CopyFileTree(const string&  Dest);
	COPY_CODES ShellCopyOneFile(const string& Src, const FAR_FIND_DATA_EX &SrcData, string &strDest, int KeepPathPos, int Rename);
	COPY_CODES CheckStreams(const string& Src,const string& DestPath);
	int ShellCopyFile(const string& SrcName,const FAR_FIND_DATA_EX &SrcData, string &strDestName,DWORD &DestAttr,int Append);
	int ShellSystemCopy(const string& SrcName,const string& DestName,const FAR_FIND_DATA_EX &SrcData);
	int DeleteAfterMove(const string& Name,DWORD Attr);
	int AskOverwrite(const FAR_FIND_DATA_EX &SrcData,const string& SrcName,const string& DestName, DWORD DestAttr,int SameName,int Rename,int AskAppend, int &Append,string &strNewName,int &RetCode);
	int GetSecurity(const string& FileName, FAR_SECURITY_DESCRIPTOR_EX& sd);
	int SetSecurity(const string& FileName,const FAR_SECURITY_DESCRIPTOR_EX& sd);
	int SetRecursiveSecurity(const string& FileName,const FAR_SECURITY_DESCRIPTOR_EX& sd);
	bool CalcTotalSize();
	bool ShellSetAttr(const string& Dest,DWORD Attr);
	void SetDestDizPath(const string& DestPath);
	void CheckUpdatePanel(); // выставляет флаг FCOPY_UPDATEPPANEL
	intptr_t WarnDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2);
	intptr_t CopyDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2);


	DWORD Flags;
	Panel *SrcPanel,*DestPanel;
	int SrcPanelMode,DestPanelMode;
	int SrcDriveType,DestDriveType;
	string strSrcDriveRoot;
	string strDestDriveRoot;
	string strDestFSName;
	DizList DestDiz;
	string strDestDizPath;
	char *CopyBuffer;
	size_t CopyBufferSize;
	string strCopiedName;
	string strRenamedName;
	string strRenamedFilesPath;
	int OvrMode;
	int ReadOnlyOvrMode;
	int ReadOnlyDelMode;
	int SkipMode;          // ...для пропуска при копировании залоченных файлов.
	int SkipEncMode;
	int SkipDeleteMode;
	int SelectedFolderNameLength;
	UserDefinedList DestList;
	// тип создаваемого репарспоинта.
	// при AltF6 будет то, что выбрал юзер в диалоге,
	// в остальных случаях - RP_EXACTCOPY - как у источника
	ReparsePointTypes RPT;
	string strPluginFormat;
	int AltF10;
	int CopySecurity;
	size_t SelCount;
	DWORD FileAttr;
	bool FolderPresent;
	bool FilesPresent;
	bool AskRO;
};
