#ifndef __FLINK_HPP__
#define __FLINK_HPP__

/*
flink.hpp

Заголовочный файл для работы с Hard & SymLink

*/

/* Revision: 1.06 29.05.2001 $ */

/*
Modify:
  29.05.2001 SVS
    + GetPathRootOne()
  28.04.2001 VVM
    + GetSubstName получает тип носителя
  25.04.2001 SVS
    + CreateVolumeMountPoint() - монтирование диска на файловую систему
  06.04.2001 SVS
    + CanCreateHardLinks() - проверка на вшивость.
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
BOOL  WINAPI CanCreateHardLinks(char *TargetFile,char *HardLinkName);
int   WINAPI GetNumberOfLinks(char *Name);
int   WINAPI CreateVolumeMountPoint(LPCTSTR SrcVolume,LPCTSTR LinkFolder);
BOOL  WINAPI CreateJunctionPoint(LPCTSTR szMountDir, LPCTSTR szDestDir);
BOOL  WINAPI DeleteJunctionPoint(LPCTSTR szMountDir);
DWORD WINAPI GetJunctionPointInfo(LPCTSTR szMountDir,
              LPTSTR  szDestBuff,
              DWORD   dwBuffSize);

BOOL GetSubstName(int DriveType,char *LocalName,char *SubstName,int SubstSize);
int DelSubstDrive(char *DosDeviceName);
void  WINAPI GetPathRoot(char *Path,char *Root);
void GetPathRootOne(char *Path,char *Root);

#endif // __FLINK_HPP__
