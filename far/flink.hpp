#ifndef __FLINK_HPP__
#define __FLINK_HPP__

/*
flink.hpp

Заголовочный файл для работы с Hard & SymLink

*/

/* Revision: 1.00 04.01.2001 $ */

/*
Modify:
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


#endif // __FLINK_HPP__
