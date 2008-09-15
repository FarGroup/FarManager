#ifndef __FLINK_HPP__
#define __FLINK_HPP__

/*
flink.hpp

«аголовочный файл дл€ работы с Hard & SymLink

*/

int   WINAPI MkLink(const char *Src,const char *Dest);
int   WINAPI FarMkLink(const char *Src,const char *Dest,DWORD Flags);
BOOL  WINAPI CanCreateHardLinks(const char *TargetFile,const char *HardLinkName);
int   WINAPI GetNumberOfLinks(const char *Name);
int   WINAPI CreateVolumeMountPoint(LPCTSTR SrcVolume,LPCTSTR LinkFolder);
BOOL  WINAPI CreateReparsePoint(LPCTSTR szMountDir, LPCTSTR szDestDir,DWORD Type=RP_JUNCTION);
BOOL  WINAPI DeleteReparsePoint(LPCTSTR szMountDir);
DWORD WINAPI GetReparsePointInfo(LPCTSTR szMountDir,LPTSTR szDestBuff,DWORD dwBuffSize,LPDWORD lpReparseTag=NULL);
int   WINAPI FarGetReparsePointInfo(const char *Src,char *Dest,int DestSize);

BOOL GetSubstName(int DriveType,char *LocalName,char *SubstName,int SubstSize);
int DelSubstDrive(char *DosDeviceName);
void  WINAPI GetPathRoot(const char *Path,char *Root);
void GetPathRootOne(const char *Path,char *Root);

// перечисл€тель дл€ EnumNTFSStreams
// в параметре sid поле cStreamName не актуально, т.к. готовое им€ потока
//    передаетс€ в параметре StreamName
typedef BOOL (WINAPI *ENUMFILESTREAMS)(int Idx,const WCHAR *StreamName,const WIN32_STREAM_ID *sid);
int WINAPI EnumNTFSStreams(const char *FileName,ENUMFILESTREAMS fpEnum,__int64 *SizeStreams);

#endif // __FLINK_HPP__
