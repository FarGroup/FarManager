#ifndef __HISTORY_HPP__
#define __HISTORY_HPP__
/*
history.hpp

История (Alt-F8, Alt-F11, Alt-F12)
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

#include "farconst.hpp"
#include "UnicodeString.hpp"

enum{
  HISTORYTYPE_CMD,
  HISTORYTYPE_FOLDER,
  HISTORYTYPE_VIEW,
};

#define HISTORY_TITLESIZE 32

struct HistoryRecord
{
  int   Type;
  wchar_t  Title[HISTORY_TITLESIZE]; //BUGBUG
  wchar_t *Name;
};

class History
{
  private:
    string strRegKey;
    unsigned int LastPtr,CurLastPtr;
    unsigned int LastPtr0,CurLastPtr0;
    int EnableAdd,RemoveDups,KeepSelectedPos;
    int TypeHistory;
    int HistoryCount;
    const int *EnableSave;
    int SaveTitle,SaveType;
    int LastSimilar;
    int ReturnSimilarTemplate;
    HistoryRecord *LastStr;

  private:
    void AddToHistoryLocal(const wchar_t *Str,const wchar_t *Title,int Type);
    void FreeHistory();
    BOOL EqualType(int Type1, int Type2);

  public:
    History(int TypeHistory,int HistoryCount,const wchar_t *RegKey,const int *EnableSave,int SaveTitle,int SaveType);
   ~History();

  public:
    void AddToHistory(const wchar_t *Str,const wchar_t *Title=NULL,int Type=0,int SaveForbid=0);
    BOOL ReadHistory();
    BOOL SaveHistory();
    int  Select(const wchar_t *Title,const wchar_t *HelpTopic, string &strStr,int &Type, string *strItemTitle = NULL);
    void GetPrev(string &strStr);
    void GetNext(string &strStr);
    void SetFirst() {LastPtr=LastPtr0;CurLastPtr=CurLastPtr0;}
    void GetSimilar(string &strStr,int LastCmdPartLength);
    void SetAddMode(int EnableAdd,int RemoveDups,int KeepSelectedPos);
    void ReloadTitle();
};


#endif  // __HISTORY_HPP__
