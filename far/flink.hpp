#ifndef __FLINK_HPP__
#define __FLINK_HPP__

/*
flink.hpp

«аголовочный файл дл€ работы с Hard & SymLink

*/

int   WINAPI MkLink(const wchar_t *Src,const wchar_t *Dest);
int   WINAPI FarMkLink(const wchar_t *Src,const wchar_t *Dest,DWORD Flags);

BOOL  WINAPI CanCreateHardLinks(const wchar_t *TargetFile,const wchar_t *HardLinkName);
int   WINAPI GetNumberOfLinks(const wchar_t *Name);
int   WINAPI CreateVolumeMountPoint(const wchar_t *SrcVolume, const wchar_t *LinkFolder);

BOOL  WINAPI CreateJunctionPoint(const wchar_t *szMountDir, const wchar_t *szDestDir);
BOOL  WINAPI DeleteJunctionPoint(const wchar_t *szMountDir);

DWORD WINAPI GetJunctionPointInfo(const wchar_t *szMountDir, string &szDestBuff);

//int   WINAPI FarGetReparsePointInfo(const char *Src,char *Dest,int DestSize);

BOOL GetSubstName(int DriveType,const wchar_t *LocalName,string &strSubstName);

int DelSubstDrive(const wchar_t *DosDeviceName);
void  WINAPI GetPathRoot(const wchar_t *Path, string &strRoot);
void GetPathRootOne(const wchar_t *Path, string &strRoot);

// перечисл€тель дл€ EnumNTFSStreams
// в параметре sid поле cStreamName не актуально, т.к. готовое им€ потока
//    передаетс€ в параметре StreamName
typedef BOOL (WINAPI *ENUMFILESTREAMS)(int Idx,const WCHAR *StreamName,const WIN32_STREAM_ID *sid);
int WINAPI EnumNTFSStreams(const char *FileName,ENUMFILESTREAMS fpEnum,__int64 *SizeStreams);

#endif // __FLINK_HPP__
