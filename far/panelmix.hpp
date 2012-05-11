#pragma once

/*
panelmix.hpp

Misc functions for processing of path names
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

class Panel;

void ShellUpdatePanels(Panel *SrcPanel,BOOL NeedSetUpADir=FALSE);
int  CheckUpdateAnotherPanel(Panel *SrcPanel,const wchar_t *SelName);

int _MakePath1(DWORD Key,string &strPathName, const wchar_t *Param2,int ShortNameAsIs=TRUE);

const string FormatStr_Attribute(DWORD FileAttributes,int Width=-1);
const string FormatStr_DateTime(const FILETIME *FileTime,int ColumnType,unsigned __int64 Flags,int Width);
const string FormatStr_Size(__int64 FileSize, __int64 AllocationSize, __int64 StreamsSize, const string& strName,
						DWORD FileAttributes,DWORD ShowFolderSize,DWORD ReparseTag,int ColumnType,
						unsigned __int64 Flags,int Width,const wchar_t *CurDir=nullptr);
void TextToViewSettings(const wchar_t *ColumnTitles,const wchar_t *ColumnWidths,
						unsigned __int64 *ViewColumnTypes,int *ViewColumnWidths,int *ViewColumnWidthsTypes,int &ColumnCount);
void ViewSettingsToText(unsigned __int64 *ViewColumnTypes,int *ViewColumnWidths,int *ViewColumnWidthsTypes,int ColumnCount,
						string &strColumnTitles,string &strColumnWidths);
