#ifndef __FLINK_HPP__
#define __FLINK_HPP__

/*
flink.hpp

Заголовочный файл для работы с Hard & SymLink

*/

/* Revision: 1.02 13.03.2001 $ */

/*
Modify:
  13.03.2001 SVS
    ! GetPathRoot переехала в fn.hpp :-)
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
void  WINAPI GetPathRoot(char *Path,char *Root);

#endif // __FLINK_HPP__
