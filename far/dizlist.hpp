#ifndef __DIZLIST_HPP__
#define __DIZLIST_HPP__
/*
dizlist.hpp

Описания файлов

*/

/* Revision: 1.04 07.07.2006 $ */

#include "farconst.hpp"
#include "UnicodeString.hpp"

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
    static void PR_ReadingMsg(void);
};


#endif  // __DIZLIST_HPP__
