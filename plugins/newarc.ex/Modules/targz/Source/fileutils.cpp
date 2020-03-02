#include <Rtl.Base.h>
#include <FarPluginBase.hpp>
#include <stdio.h>
#include "fileutils.hpp"

extern PluginStartupInfo Info; //FIXME

static int AskRetry(char *filename,int *retry)
{
  int localretry=rtSkip;
  if(*retry==rtAsk)
  {
    const char *MsgItems[]={"Error","Cannot copy",filename,"Retry","Skip","Skip all","Cancel"};
    int code=Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),4);
    if(code<aorRetry) code=aorCancel;
    switch(code)
    {
      case aorRetry:
        localretry=rtRetry;
        break;
      case aorSkipAll:
        *retry=rtSkip;
      case aorSkip:
        localretry=rtSkip;
        break;
      case aorCancel:
        localretry=rtCancel;
        break;
    }
  }
  else localretry=*retry;
  return localretry;
}

static void GetFileAttr(char *file,unsigned long *size,SYSTEMTIME *mod)
{
  *size=0;
  memset(mod,0,sizeof(SYSTEMTIME));
  WIN32_FIND_DATA find; HANDLE hFind;
  hFind=FindFirstFile(file,&find);
  if(hFind!=INVALID_HANDLE_VALUE)
  {
    FindClose(hFind);
//    *size=(unsigned long long)find.nFileSizeLow+(unsigned long long)find.nFileSizeHigh*4294967296ULL;
    *size=find.nFileSizeLow;
    FILETIME local;
    FileTimeToLocalFileTime(&find.ftLastWriteTime,&local);
    FileTimeToSystemTime(&local,mod);
  }
}

static int AskOverwrite(char *filename,DWORD size,FILETIME lastwrite)
{
  char new_title[64],old_title[64];
  const char *MsgItems[]={"Warning","File already exists",filename,"\001",new_title,old_title,"\001","Overwrite","All","Skip","Skip all","Append","Cancel"};
  SYSTEMTIME new_mod;
  FileTimeToSystemTime(&lastwrite,&new_mod);
  sprintf(new_title,"New                %10ld %02d.%02d.%02d %02d:%02d:%02d",size,new_mod.wDay,new_mod.wMonth,new_mod.wYear,new_mod.wHour,new_mod.wMinute,new_mod.wSecond);
  DWORD oldsize; SYSTEMTIME old_mod;
  GetFileAttr(filename,&oldsize,&old_mod);
  sprintf(old_title,"Existing           %10ld %02d.%02d.%02d %02d:%02d:%02d",oldsize,old_mod.wDay,old_mod.wMonth,old_mod.wYear,old_mod.wHour,old_mod.wMinute,old_mod.wSecond);
  int res=Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),6);
  if(res<aotOverwrite) res=aotCancel;
  return res;
}

bool CreateFileEx(HANDLE *handle,char *filename,int *exist,int *retry,DWORD attributes,DWORD size,FILETIME lastwrite)
{
  bool domain=true,result=true;
  while(domain)
  {
    domain=false;
    *handle=CreateFile(filename,GENERIC_WRITE,0,NULL,CREATE_NEW,attributes|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
    if((*handle==INVALID_HANDLE_VALUE)&&(GetLastError()==ERROR_FILE_EXISTS))
    {
      int localexist=etSkip;
      if(*exist==etAsk)
      {
        int msgcode=AskOverwrite(filename,size,lastwrite);
        switch(msgcode)
        {
          case aotOverwriteAll:
            *exist=etOverwrite;
          case aotOverwrite:
            localexist=etOverwrite;
            break;
          case aotSkipAll:
            *exist=etSkip;
          case aotSkip:
            localexist=etSkip;
            break;
          case aotAppend:
            localexist=etAppend;
            break;
          case aotCancel:
            localexist=etCancel;
            break;
        }
      }
      else localexist=*exist;
      bool dolocal=true;
      switch(localexist)
      {
        case etOverwrite:
          while(dolocal)
          {
            dolocal=false;
            *handle=CreateFile(filename,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,attributes|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
            if(*handle==INVALID_HANDLE_VALUE)
            {
              int localretry=AskRetry(filename,retry);
              switch(localretry)
              {
                case rtRetry:
                  dolocal=true;
                  break;
                case rtCancel:
                  result=false;
                  break;
              }
            }
          }
          break;
        case etSkip:
          break;
        case etAppend:
          while(dolocal)
          {
            dolocal=false;
            *handle=CreateFile(filename,GENERIC_WRITE,0,NULL,OPEN_ALWAYS,attributes|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
            if(*handle==INVALID_HANDLE_VALUE)
            {
              int localretry=AskRetry(filename,retry);
              switch(localretry)
              {
                case rtRetry:
                  dolocal=true;
                  break;
                case rtCancel:
                  result=false;
                  break;
              }
            }
            else
            {
              long HighPointer=0;
              SetFilePointer(*handle,0,&HighPointer,FILE_END); //!!!
            }
          }
          break;
        case etCancel:
          result=false;
          break;
      }
    }
    else if(*handle==INVALID_HANDLE_VALUE)
    {
      int localretry=AskRetry(filename,retry);
      switch(localretry)
      {
        case rtRetry:
          domain=true;
          break;
        case rtCancel:
          result=false;
          break;
      }
    }
  }
  return result;
}

bool CreateDirEx(char *DestDir)
{
  char Dir[MAX_PATH],*DirEnd;
  if((strlen(DestDir)>2)/*&&(!CheckSystemFile(DestDir))*/) //FIXME
  {
    char CreateDest[MAX_PATH];
    DirEnd=DestDir+3;
    do
    {
      DirEnd=strchr(DirEnd,'\\');
      if(DirEnd)
      {
        strncpy(Dir,DestDir,DirEnd-DestDir); Dir[DirEnd-DestDir]=0;
        DirEnd++;
        CreateDirectory(Dir,NULL);
      }
    } while(DirEnd);
    strcpy(CreateDest,Dir);
    strcat(CreateDest,"\\*");
    WIN32_FIND_DATA find;
    SetLastError(0);
    HANDLE hFind=FindFirstFile(CreateDest,&find);
    if(hFind!=INVALID_HANDLE_VALUE)
    {
      FindClose(hFind);
      return true;
    }
  }
  return false;
}

char *GetFilePtr(char *path)
{
  char *result=path;
  while(*path)
  {
    if((*path=='\\')||(*path=='/')) result=path+1;
    path++;
  }
  return result;
}

void GenerateName(const char *Name,char *ZipName)
{
  const char *NamePtr=(const char *)strrchr(Name,'\\');
  NamePtr=(NamePtr==NULL)?Name:NamePtr+1;
  strcpy(ZipName,NamePtr);
  const char *Dot=(const char *)strrchr(NamePtr,'.');
  if(Dot!=NULL)
  {
    Dot++;
    if (_strcmpi(Dot,"tgz")==0 || _strcmpi(Dot,"taz")==0)
      strcpy(&ZipName[Dot-NamePtr],"tar");
    else
      ZipName[Dot-NamePtr-1]=0;
  }
}
