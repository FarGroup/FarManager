#ifndef __FLINK_HPP__
#define __FLINK_HPP__

/*
flink.hpp

Заголовочный файл для работы с Hard & SymLink

*/

/* Revision: 1.01 05.01.2001 $ */

/*
Modify:
  05.01.2001 SVS
    + Функция DelSubstDrive - удаление Subst драйвера
    + Функция GetSubstName переехала из fh.hpp
  04.01.2001 SVS
    + Создан.
    + Описания MkLink, GetNumberOfLinks переехали из fn.hpp

*/

int   WINAPI MkLink(char *Src,char *Dest);
int   WINAPI GetNumberOfLinks(char *Name);
BOOL  WINAPI CreateJunctionPoint(LPCTSTR szMountDir, LPCTSTR szDestDir);
BOOL  WINAPI DeleteJunctionPoint(LPCTSTR szMountDir);
DWORD WINAPI GetJunctionPointInfo(LPCTSTR szMountDir,
              LPTSTR  szDestBuff,
              DWORD   dwBuffSize);

BOOL GetSubstName(char *LocalName,char *SubstName,int SubstSize);
int DelSubstDrive(char *DosDeviceName);

#endif // __FLINK_HPP__
