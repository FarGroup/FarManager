﻿#ifndef PANELMIX_HPP_AF7AAF02_56C0_4E41_B1D9_D1F1A5B4025D
#define PANELMIX_HPP_AF7AAF02_56C0_4E41_B1D9_D1F1A5B4025D
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

#include "panelfwd.hpp"

#include "platform.chrono.hpp"

struct column;

void ShellUpdatePanels(panel_ptr SrcPanel, bool NeedSetUpADir = false);
bool CheckUpdateAnotherPanel(panel_ptr SrcPanel,const string& SelName);

bool MakePath(const panel_ptr& SrcPanel, bool FilePath, bool RealName, bool ShortNameAsIs, string& strPathName);
bool MakePathForUI(DWORD Key, string &strPathName);

string FormatStr_Attribute(DWORD FileAttributes, size_t Width);
string FormatStr_DateTime(os::chrono::time_point FileTime, int ColumnType, unsigned long long Flags, int Width);
string FormatStr_Size(long long Size, const string& strName,
	DWORD FileAttributes, DWORD ShowFolderSize, DWORD ReparseTag, int ColumnType,
	unsigned long long Flags, int Width, const wchar_t* CurDir = nullptr);
std::vector<column> DeserialiseViewSettings(const string& ColumnTitles, const string& ColumnWidths);
std::pair<string, string> SerialiseViewSettings(const std::vector<column>& Columns);
int GetDefaultWidth(unsigned long long Type);

#endif // PANELMIX_HPP_AF7AAF02_56C0_4E41_B1D9_D1F1A5B4025D
