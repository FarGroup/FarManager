#ifndef __FLINK_HPP__
#define __FLINK_HPP__

/*
flink.hpp

«аголовочный файл дл€ работы с Hard & SymLink

*/

/* Revision: 1.16 17.03.2006 $ */

int   WINAPI MkLinkW(const wchar_t *Src,const wchar_t *Dest);
int   WINAPI FarMkLinkW(const wchar_t *Src,const wchar_t *Dest,DWORD Flags);

BOOL  WINAPI CanCreateHardLinksW(const wchar_t *TargetFile,const wchar_t *HardLinkName);
int   WINAPI GetNumberOfLinksW(const wchar_t *Name);
int   WINAPI CreateVolumeMountPointW(const wchar_t *SrcVolume, const wchar_t *LinkFolder);

BOOL  WINAPI CreateJunctionPoint(LPCTSTR szMountDir, LPCTSTR szDestDir);
BOOL  WINAPI CreateJunctionPointW(const wchar_t *szMountDir, const wchar_t *szDestDir);
BOOL  WINAPI DeleteJunctionPointW(const wchar_t *szMountDir);

DWORD WINAPI GetJunctionPointInfoW(const wchar_t *szMountDir, string &szDestBuff);

//int   WINAPI FarGetReparsePointInfo(const char *Src,char *Dest,int DestSize);

BOOL GetSubstNameW(int DriveType,const wchar_t *LocalName,string &strSubstName);

int DelSubstDrive(const wchar_t *DosDeviceName);
void  WINAPI GetPathRootW(const wchar_t *Path, string &strRoot);
void GetPathRootOneW(const wchar_t *Path, string &strRoot);

// перечисл€тель дл€ EnumNTFSStreams
// в параметре sid поле cStreamName не актуально, т.к. готовое им€ потока
//    передаетс€ в параметре StreamName
typedef BOOL (WINAPI *ENUMFILESTREAMS)(int Idx,const WCHAR *StreamName,const WIN32_STREAM_ID *sid);
int WINAPI EnumNTFSStreams(const char *FileName,ENUMFILESTREAMS fpEnum,__int64 *SizeStreams);

#endif // __FLINK_HPP__
