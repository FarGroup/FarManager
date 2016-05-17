#ifndef COPY_HPP_741275F9_6A07_4F73_9674_D8464C559194
#define COPY_HPP_741275F9_6A07_4F73_9674_D8464C559194
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
#include "panel.hpp"

class Dialog;
class copy_progress;
class FileFilter;

enum COPY_CODES: int;
enum ReparsePointTypes: int;

class ShellCopy: noncopyable
{
public:
	ShellCopy(panel_ptr SrcPanel, int Move, int Link, int CurrentOnly, int Ask, int &ToPlugin, const wchar_t* PluginDestPath, bool ToSubdir = false);
	~ShellCopy();
	DWORD CopyProgressRoutine(uint64_t TotalFileSize, uint64_t TotalBytesTransferred, uint64_t StreamSize, uint64_t StreamBytesTransferred, DWORD StreamNumber, DWORD CallbackReason, HANDLE SourceFile, HANDLE DestinationFile);

private:
	COPY_CODES CopyFileTree(const string&  Dest);
	COPY_CODES ShellCopyOneFile(const string& Src, const os::FAR_FIND_DATA &SrcData, string &strDest, int KeepPathPos, int Rename);
	COPY_CODES CheckStreams(const string& Src,const string& DestPath);
	int ShellCopyFile(const string& SrcName,const os::FAR_FIND_DATA &SrcData, string &strDestName,DWORD &DestAttr,int Append);
	int ShellSystemCopy(const string& SrcName,const string& DestName,const os::FAR_FIND_DATA &SrcData);
	int DeleteAfterMove(const string& Name,DWORD Attr);
	int AskOverwrite(const os::FAR_FIND_DATA &SrcData,const string& SrcName,const string& DestName, DWORD DestAttr,int SameName,int Rename,int AskAppend, int &Append,string &strNewName,int &RetCode);
	bool GetSecurity(const string& FileName, os::FAR_SECURITY_DESCRIPTOR& sd);
	bool SetSecurity(const string& FileName, const os::FAR_SECURITY_DESCRIPTOR& sd);
	int SetRecursiveSecurity(const string& FileName,const os::FAR_SECURITY_DESCRIPTOR& sd);
	bool CalcTotalSize() const;
	bool ShellSetAttr(const string& Dest,DWORD Attr);
	void SetDestDizPath(const string& DestPath);
	static intptr_t WarnDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2);
	intptr_t CopyDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2);

	std::unique_ptr<copy_progress> CP;
	std::unique_ptr<FileFilter> m_Filter;
	DWORD Flags;
	panel_ptr SrcPanel, DestPanel;
	panel_mode SrcPanelMode,DestPanelMode;
	int SrcDriveType,DestDriveType;
	string strSrcDriveRoot;
	string strDestDriveRoot;
	string strDestFSName;
	DizList DestDiz;
	string strDestDizPath;
	char_ptr CopyBuffer;
	const size_t CopyBufferSize;
	string strCopiedName;
	string strRenamedName;
	string strRenamedFilesPath;
	int OvrMode;
	int ReadOnlyOvrMode;
	int ReadOnlyDelMode;
	int SkipMode;          // ...для пропуска при копировании залоченных файлов.
	int SkipEncMode;
	int SkipDeleteMode;
	bool SkipSecurityErrors;
	int SelectedFolderNameLength;
	std::vector<string> m_DestList;
	// тип создаваемого репарспоинта.
	// при AltF6 будет то, что выбрал юзер в диалоге,
	// в остальных случаях - RP_EXACTCOPY - как у источника
	ReparsePointTypes RPT;
	string strPluginFormat;
	int AltF10;
	int m_CopySecurity;
	size_t SelCount;
	DWORD m_FileAttr;
	bool FolderPresent;
	bool FilesPresent;
	bool AskRO;
	bool m_UseFilter;
	HANDLE m_FileHandleForStreamSizeFix;
	size_t m_NumberOfTargets;
};

#endif // COPY_HPP_741275F9_6A07_4F73_9674_D8464C559194
