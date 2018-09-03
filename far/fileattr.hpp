﻿#ifndef FILEATTR_HPP_1920BF1F_BD95_4A22_B3D9_33F2544760D1
#define FILEATTR_HPP_1920BF1F_BD95_4A22_B3D9_33F2544760D1
#pragma once

/*
fileattr.hpp

Работа с атрибутами файлов
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

#include "platform.chrono.hpp"

enum SETATTR_RET_CODES
{
	SETATTR_RET_ERROR,
	SETATTR_RET_OK,
	SETATTR_RET_SKIP,
	SETATTR_RET_SKIPALL,
};

int ESetFileAttributes(const string& Name,DWORD Attr,int SkipMode=-1);
int ESetFileCompression(const string& Name,int State,DWORD FileAttr,int SkipMode=-1);
int ESetFileEncryption(const string& Name,bool State,DWORD FileAttr,int SkipMode=-1,int Silent=0);
int ESetFileSparse(const string& Name,bool State,DWORD FileAttr,int SkipMode=-1);
int ESetFileTime(const string& Name, const os::chrono::time_point* LastWriteTime, const os::chrono::time_point* CreationTime, const os::chrono::time_point* LastAccessTime, const os::chrono::time_point* ChangeTime, DWORD FileAttr, int SkipMode=-1);
int ESetFileOwner(const string& Name, const string& Owner,int SkipMode=-1);
int EDeleteReparsePoint(const string& Name, DWORD FileAttr, int SkipMode=-1);

void enum_attributes(const std::function<bool(DWORD, wchar_t)>& Pred);


#endif // FILEATTR_HPP_1920BF1F_BD95_4A22_B3D9_33F2544760D1
