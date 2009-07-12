#ifndef __DIZLIST_HPP__
#define __DIZLIST_HPP__
/*
dizlist.hpp

Описания файлов
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

struct DizRecord
{
  wchar_t *DizText;
  int Deleted;
};

class DizList
{
  private:
    string strDizFileName;
    DizRecord *DizData;
    int DizCount;
    int *IndexData;
    int IndexCount;

  private:
    int GetDizPos(const wchar_t *Name,const wchar_t *ShortName,int *TextPos);
    int GetDizPosEx(const wchar_t *Name,const wchar_t *ShortName,int *TextPos);
    void AddRecord(const wchar_t *DizText);
    void BuildIndex();

  public:
    DizList();
    ~DizList();

  public:
    void Read(const wchar_t *Path,const wchar_t *DizName=NULL);
    void Reset();
    const wchar_t* GetDizTextAddr(const wchar_t *Name,const wchar_t *ShortName, const __int64 &FileSize);
    int DeleteDiz(const wchar_t *Name,const wchar_t *ShortName);
    int Flush(const wchar_t *Path,const wchar_t *DizName=NULL);
    void AddDiz(const wchar_t *Name,const wchar_t *ShortName,const wchar_t *DizText);
    int CopyDiz(const wchar_t *Name,const wchar_t *ShortName,const wchar_t *DestName,
                 const wchar_t *DestShortName,DizList *DestDiz);
    void GetDizName(string &strDizName);
		static void PR_ReadingMsg();
};


#endif  // __DIZLIST_HPP__
