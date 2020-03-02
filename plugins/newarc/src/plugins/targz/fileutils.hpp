#ifndef __FILEUTILS_HPP__
#define __FILEUTILS_HPP__

enum
{
  etAsk=0,
  etOverwrite,
  etSkip,
  etAppend,
  etCancel
};

enum
{
  rtAsk=0,
  rtRetry,
  rtSkip,
  rtCancel
};

enum
{
  aotOverwrite=0,
  aotOverwriteAll,
  aotSkip,
  aotSkipAll,
  aotAppend,
  aotCancel,
};

enum
{
  aorRetry=0,
  aorSkip,
  aorSkipAll,
  aorCancel
};

extern bool CreateFileEx(HANDLE *handle,char *filename,int *exist,int *retry,DWORD attributes,DWORD size,FILETIME lastwrite);
extern bool CreateDirEx(char *DestDir);
extern char *GetFilePtr(char *path);
extern void GenerateName(const char *Name, char *ZipName);

#endif
