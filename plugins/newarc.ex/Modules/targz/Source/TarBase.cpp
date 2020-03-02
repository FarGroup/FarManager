#include <Rtl.Base.h>
#include <FarPluginBase.hpp>
#include "Archive.hpp"
#include "../../module.hpp"
#include "unixutils.hpp"
#include "fileutils.hpp"
#include <stdio.h>

bool ArchiveDetect::Read(unsigned char *Data,DWORD Size)
{
  DWORD ReadSize;
  if(read(Data,Size,&ReadSize)&&(ReadSize==Size)) return true;
  return false;
}

ArchiveOne::ArchiveOne(const char *ArcName)
{
  GenerateName(ArcName,ZipName);
  Handle=CreateFile(ArcName,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  Processed=false;
}

ArchiveOne::~ArchiveOne()
{
  CloseHandle(Handle);
}

int ArchiveOne::Next(PluginPanelItem *data)
{
  if(Processed) return E_EOF;
  data->PackSize=data->FindData.nFileSizeLow=GetFileSize(Handle,NULL);
  strcpy(data->FindData.cFileName,ZipName);
  Processed=true;
  return E_SUCCESS;
}

bool ArchiveOne::Extract(PluginPanelItem *pItem,char *destination)
{
  return false;
}

int ArchiveGzip::Next(PluginPanelItem *data)
{
  int res=ArchiveOne::Next(data);
  if(res==E_SUCCESS)
  {
#if defined(__BORLANDC__)
  #pragma option -a2
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(2)
  #if defined(__LCC__)
    #define _export __declspec(dllexport)
  #endif
#else
  #pragma pack(push,2)
  #if _MSC_VER
    #define _export
  #endif
#endif
    struct GZHeader
    {
      BYTE Mark[2];
      BYTE Method;
      BYTE Flags;
      DWORD FileTime;
      BYTE ExtraFlags;
      BYTE HostOS;
    } Header;
#if defined(__BORLANDC__)
  #pragma option -a.
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack()
#else
  #pragma pack(pop)
#endif
    DWORD ReadSize;
    if(!ReadFile(Handle,&Header,sizeof(Header),&ReadSize,NULL)) return E_READ_ERROR;
    if(Header.Flags&2) SetFilePointer(Handle,2,NULL,FILE_CURRENT);
    if(Header.Flags&4)
    {
      DWORD ExtraLength;
      if(!ReadFile(Handle,&ExtraLength,sizeof(ExtraLength),&ReadSize,NULL)) return(E_READ_ERROR);
      SetFilePointer(Handle,ExtraLength,NULL,FILE_CURRENT);
    }
    if(Header.Flags&8)
      if(!ReadFile(Handle,data->FindData.cFileName,sizeof(data->FindData.cFileName),&ReadSize,NULL))
        return(E_READ_ERROR);
//    if(*Item->FindData.cFileName==0) //FIXME
//      strcpy(Item->FindData.cFileName,ZipName);
    UnixTimeToFileTime(Header.FileTime,&data->FindData.ftLastWriteTime);
//    Info->Comment=(Header.Flags&16)!=0;
//    Info->Encrypted=(Header.Flags & 32)!=0;
    SetFilePointer(Handle,-4,NULL,FILE_END);
    if(!ReadFile(Handle,&data->FindData.nFileSizeLow,sizeof(data->FindData.nFileSizeLow),&ReadSize,NULL))
      return(E_READ_ERROR);
  }
  return res;
}

TarBase::TarBase()
{
  firsttime=true;
  valid=false;
  NextPosition=0;
  Exist=etAsk;
  Retry=rtAsk;
}

int TarBase::Next(PluginPanelItem *data)
{
  valid=false;
  if(firsttime)
  {
    firsttime=false;
    if(error()) return E_UNEXPECTED_EOF;
  }
  struct posix_header Header;
  DWORD ReadSize;
  BOOL SkipItem=FALSE;
  char *LongName=NULL;
  memset(&Item,0,sizeof(Item));
  do
  {
    NextPosition=seek(NextPosition);
    if(NextPosition==0xFFFFFFFF) return E_READ_ERROR;
#if 0
    if(NextPosition>FileSize) return(E_UNEXPECTED_EOF); //FIXME
#endif
    if(!read(&Header,sizeof(Header),&ReadSize)) return(E_READ_ERROR);
    if(ReadSize==0||*Header.name==0) return(E_EOF);

    if(Header.typeflag==GNUTYPE_LONGLINK||Header.typeflag==GNUTYPE_LONGNAME) SkipItem=TRUE;
    else
    {
      SkipItem=FALSE;
      char *EndPos;
      if(LongName!=NULL)
        EndPos=AdjustTARFileName(LongName);
      else
        EndPos=AdjustTARFileName(Header.name);
      strncpy(Item.FindData.cFileName,EndPos,sizeof(Item.FindData.cFileName)-1);
      Item.FindData.dwFileAttributes=(GetOctal(Header.mode)&0x4000)||((Header.typeflag-'0')&4)?FILE_ATTRIBUTE_DIRECTORY:0;
      UnixTimeToFileTime(GetOctal(Header.mtime),&Item.FindData.ftLastWriteTime);
      Item.FindData.nFileSizeHigh=0;
      Item.FindData.dwReserved0=NextPosition+sizeof(Header); //WARN: we use reserved field for our purporses.
                                                             //FIXME
    }
    Item.PackSize=Item.FindData.nFileSizeLow=GetOctal(Header.size);
    DWORD PrevPosition=NextPosition;
    NextPosition+=sizeof(Header)+Item.PackSize;
    if(NextPosition&511)
      NextPosition+=512-(NextPosition&511);
    if(PrevPosition>=NextPosition)
      return(E_BROKEN);
    if(Header.typeflag==GNUTYPE_LONGNAME)
    {
      seek(PrevPosition+sizeof(Header)); //FIXME
      // we can't have two LONGNAME records in a row without a file between them
      if(LongName!=NULL) return E_BROKEN;
      LongName=(char *)HeapAlloc(GetProcessHeap(),0,Item.PackSize); //FIXME
      DWORD BytesRead;
      read(LongName,Item.PackSize,&BytesRead); //FIXME
      if(BytesRead!=Item.PackSize)
      {
        HeapFree(GetProcessHeap(),0,LongName);
        return E_BROKEN;
      }
    }
  } while(SkipItem);
  if (LongName)
    HeapFree(GetProcessHeap(),0,LongName);
  valid=true;
  *data=Item;
  return E_SUCCESS;
}

bool TarBase::Extract(PluginPanelItem *pItem,char *destination) //FIXME: result must be int
{
//  if(!valid) return false;
  if(!destination) return false;
  seek(pItem->FindData.dwReserved0);
  DWORD Remaining=pItem->FindData.nFileSizeLow,BytesRead,BytesWritten;
  HANDLE out; char Buffer[BLOCKSIZE];
  CreateFileEx(&out,destination,&Exist,&Retry,pItem->FindData.dwFileAttributes,pItem->FindData.nFileSizeLow,pItem->FindData.ftLastWriteTime);
  if(out==INVALID_HANDLE_VALUE) return false;
  bool res=true;
  //{FILE *log; log=fopen("c:\\plugins.log","at"); fprintf(log, "rem: %ld\n",Remaining); fclose(log);}
  while(Remaining)
  {
    DWORD Bytes=(Remaining>BLOCKSIZE)?BLOCKSIZE:Remaining;
    if((!read(Buffer,sizeof(Buffer),&BytesRead))||(BytesRead!=sizeof(Buffer)))
    {
      res=false;
      break;
    }
    if(!WriteFile(out,&Buffer,Bytes,&BytesWritten,NULL))
    {
      res=false;
      break;
    }
    Remaining-=Bytes;
  }
  CloseHandle(out);
  return res;
}

CpioBase::CpioBase()
{
  firsttime=true;
  valid=false;
  NextPosition=0;
  Exist=etAsk;
  Retry=rtAsk;
}

#define READ_ASCII {if(!read(&buffer,8,&ReadSize)) return E_READ_ERROR;if(ReadSize==0) return E_BROKEN;buffer[8]=0;}

int CpioBase::Next(PluginPanelItem *data)
{
  valid=false;
  if(firsttime)
  {
    firsttime=false;
    if(error()) return E_UNEXPECTED_EOF;
  }
  DWORD ReadSize;
  BOOL SkipItem=FALSE;
  //char *LongName=NULL;
  memset(&Item,0,sizeof(Item));
  do
  {
    NextPosition=seek(NextPosition);
    if(NextPosition==0xFFFFFFFF) return E_READ_ERROR;
#if 0
    if(NextPosition>FileSize) return(E_UNEXPECTED_EOF); //FIXME
#endif
//    if(!read(&Header,sizeof(Header),&ReadSize)) return(E_READ_ERROR);
//    if(ReadSize==0||*Header.name==0) return(E_EOF);

    {
      char buffer[9],*filename=NULL;
      unsigned long namesize;/* bool abort=false;*/
      if(!read(&buffer,6,&ReadSize)) return E_READ_ERROR;
      if(ReadSize==0) return E_EOF;
      buffer[6]=0;
      if(strcmp(buffer,"070701")) return E_BROKEN;
      READ_ASCII; //c_ino
      READ_ASCII; //c_mode
      Item.FindData.dwFileAttributes=(GetHex(buffer)&0x4000)?FILE_ATTRIBUTE_DIRECTORY:0;
      READ_ASCII; //c_uid
      READ_ASCII; //c_gid
      READ_ASCII; //c_nlink
      READ_ASCII; //c_mtime
      UnixTimeToFileTime(GetHex(buffer),&Item.FindData.ftLastWriteTime);
      READ_ASCII; //c_filesize
      Item.PackSizeHigh=Item.FindData.nFileSizeHigh=0;
      Item.PackSize=Item.FindData.nFileSizeLow=GetHex(buffer);
      READ_ASCII; //c_maj
      READ_ASCII; //c_min
      READ_ASCII; //c_rmaj
      READ_ASCII; //c_rmin
      READ_ASCII; //c_namesize
      namesize=GetHex(buffer);
      READ_ASCII; //c_chksum
      NextPosition+=110+namesize;
      NextPosition+=(4-(NextPosition%4))%4;
      NextPosition+=Item.FindData.nFileSizeLow;
      NextPosition+=(4-(NextPosition%4))%4;
      filename=(char *)HeapAlloc(GetProcessHeap(),0,namesize);
      if(filename) //FIXME
      {
        if(read(filename,namesize,&ReadSize))
        {
          char *EndPos;
          EndPos=AdjustTARFileName(filename);
          if(strlen(EndPos)>=MAX_PATH)
          {
            strncpy(Item.FindData.cFileName,EndPos,MAX_PATH-1);
            Item.FindData.cFileName[MAX_PATH-1]=0;
          }
          else strcpy(Item.FindData.cFileName,EndPos);
        }
        HeapFree(GetProcessHeap(),0,filename);
      }
      if(!strcmp(Item.FindData.cFileName,"TRAILER!!!")) return E_EOF;
    }
  } while(SkipItem);
  valid=true;
  *data=Item;
  return E_SUCCESS;
}
#undef READ_ASCII

bool CpioBase::Extract(PluginPanelItem *pItem,char *destination) //FIXME: result must be int
{
  if(!valid) return false;
  if(!destination) return false;
  DWORD Remaining=pItem->FindData.nFileSizeLow,BytesRead,BytesWritten;
  HANDLE out; char Buffer[BLOCKSIZE];
  CreateFileEx(&out,destination,&Exist,&Retry,pItem->FindData.dwFileAttributes,pItem->FindData.nFileSizeLow,pItem->FindData.ftLastWriteTime);
  if(out==INVALID_HANDLE_VALUE) return false;
  bool res=true;
  while(Remaining)
  {
    DWORD Bytes=(Remaining>BLOCKSIZE)?BLOCKSIZE:Remaining;
    if((!read(Buffer,sizeof(Buffer),&BytesRead))||(BytesRead!=sizeof(Buffer)))
    {
      res=false;
      break;
    }
    if(!WriteFile(out,&Buffer,Bytes,&BytesWritten,NULL))
    {
      res=false;
      break;
    }
    Remaining-=Bytes;
  }
  CloseHandle(out);
  return res;
}
