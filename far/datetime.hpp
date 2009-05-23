#ifndef __DATETIME_HPP__
#define __DATETIME_HPP__
/*
datetime.hpp

Функции для работы с датой и временем
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

int GetDateFormat();
wchar_t GetDateSeparator();
wchar_t GetTimeSeparator();

__int64 FileTimeDifference(const FILETIME *a, const FILETIME* b);
unsigned __int64 FileTimeToUI64(const FILETIME *ft);

void GetFileDateAndTime(const wchar_t *Src,unsigned *Dst,int Separator);
void StrToDateTime(const wchar_t *CDate, const wchar_t *CTime, FILETIME &ft, int DateFormat, int DateSeparator, int TimeSeparator, bool bRelative=false);
void ConvertDate(const FILETIME &ft,string &strDateText, string &strTimeText,int TimeLength, int Brief=FALSE,int TextMonth=FALSE,int FullYear=FALSE,int DynInit=FALSE);
void ConvertRelativeDate(const FILETIME &ft,string &strDaysText,string &strTimeText);

void PrepareStrFTime();
size_t WINAPI StrFTime(string &strDest, const wchar_t *Format,const tm *t);
size_t MkStrFTime(string &strDest, const wchar_t *Fmt=NULL);

#endif // __DATETIME_HPP__
