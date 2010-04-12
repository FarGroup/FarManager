#ifndef __DIZLIST_HPP__
#define __DIZLIST_HPP__
/*
dizlist.hpp

Описания файлов

*/

#include "farconst.hpp"

class DizList
{
	private:
		char DizFileName[NM];
		struct DizRecord *DizData;
		int DizCount;
		int *IndexData;
		int IndexCount;
		bool Modified;

	private:
		int GetDizPos(char *Name,char *ShortName,int *TextPos);
		int GetDizPosEx(char *Name,char *ShortName,int *TextPos);
		void AddRecord(char *DizText);
		void BuildIndex();

	public:
		DizList();
		~DizList();

	public:
		void Read(char *Path,char *DizName=NULL);
		void Reset();
		char* GetDizTextAddr(char *Name,char *ShortName,unsigned __int64 FileSize);
		int DeleteDiz(char *Name,char *ShortName);
		int Flush(char *Path,char *DizName=NULL);
		void AddDiz(char *Name,char *ShortName,char *DizText);
		int CopyDiz(char *Name,char *ShortName,char *DestName,
		            char *DestShortName,DizList *DestDiz);
		void GetDizName(char *DizName);
		static void PR_ReadingMsg(void);
};


#endif	// __DIZLIST_HPP__
