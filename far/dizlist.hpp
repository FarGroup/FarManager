#ifndef __DIZLIST_HPP__
#define __DIZLIST_HPP__
/*
dizlist.hpp

Описания файлов

*/

/* Revision: 1.01 06.05.2001 $ */

/*
Modify:
  06.05.2001 DJ
    ! перетрях #include
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "farconst.hpp"

class DizList
{
  private:
    int GetDizPos(char *Name,char *ShortName,int *TextPos);
    int GetDizPosEx(char *Name,char *ShortName,int *TextPos);
    void AddRecord(char *DizText);
    void BuildIndex();
    char DizFileName[NM];
    struct DizRecord *DizData;
    int DizCount;
    int *IndexData;
    int IndexCount;
  public:
    DizList();
    ~DizList();
    void Read(char *Path,char *DizName=NULL);
    void Reset();
    char* GetDizTextAddr(char *Name,char *ShortName,DWORD FileSize);
    int DeleteDiz(char *Name,char *ShortName);
    int Flush(char *Path,char *DizName=NULL);
    void AddDiz(char *Name,char *ShortName,char *DizText);
    int CopyDiz(char *Name,char *ShortName,char *DestName,
                 char *DestShortName,DizList *DestDiz);
    void GetDizName(char *DizName);
};


#endif	// __DIZLIST_HPP__
