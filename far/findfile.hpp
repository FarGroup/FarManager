#pragma once

/*
findfile.hpp

Поиск (Alt-F7)
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

enum
{
	FFSEARCH_ALL,
	FFSEARCH_ALL_BUTNETWORK,
	FFSEARCH_INPATH,
	FFSEARCH_ROOT,
	FFSEARCH_FROM_CURRENT,
	FFSEARCH_CURRENT_ONLY,
	FFSEARCH_SELECTED,
};

class FindFiles
{
	private:
		bool FindFilesProcess();
		static LONG_PTR WINAPI FindDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2);
		static void DoScanTree(HANDLE hDlg,string& strRoot);
		static void DoPrepareFileList(HANDLE hDlg);
		static DWORD WINAPI PrepareFilesList(void *Param);
		static void DoPreparePluginList(HANDLE hDlg,bool Internal);
		static DWORD WINAPI PreparePluginList(void* Param);
		static void DoWriteDialogData(HANDLE hDlg);
		static DWORD WINAPI WriteDialogData(void *Param);
		static void ScanPluginTree(HANDLE hDlg,HANDLE hPlugin, DWORD Flags);
		static void ArchiveSearch(HANDLE hDlg,const wchar_t *ArcName);
		static void AddMenuRecord(HANDLE hDlg,const wchar_t *FullName, FAR_FIND_DATA_EX *FindData);
		static void AddMenuRecord(HANDLE hDlg,const wchar_t *FullName, FAR_FIND_DATA *FindData);

	public:
		FindFiles();
		~FindFiles();
};
