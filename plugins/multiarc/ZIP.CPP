#include <windows.h>
#include <string.h>
#include <dos.h>
#include "plugin.hpp"
#include "fmt.hpp"

#if defined(__BORLANDC__)
  #pragma option -a1
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(1)
  #if defined(__LCC__)
    #define _export __declspec(dllexport)
  #endif
#else
  #pragma pack(push,1)
  #if _MSC_VER
    #define _export
  #endif
#endif

#ifdef _MSC_VER
#pragma comment(linker, "/ignore:4078")
#pragma comment(linker, "/merge:.data=.")
#pragma comment(linker, "/merge:.rdata=.")
#pragma comment(linker, "/merge:.text=.")
#endif

static HANDLE ArcHandle;
static DWORD SFXSize,NextPosition,FileSize;
static int ArcComment,Truncated,FirstRecord;

struct ZipHeader
{
  DWORD Signature;
  WORD VerToExtract;
  WORD BitFlag;
  WORD Method;
  WORD LastModTime;
  WORD LastModDate;
  DWORD Crc32;
  DWORD SizeCompr;
  DWORD SizeUncompr;
  WORD FileNameLen;
  WORD ExtraFieldLen;
  // FileName[];
  // ExtraField[];
};

const size_t MIN_HEADER_LEN=sizeof(ZipHeader);

inline BOOL IsValidHeader(const unsigned char *Data, const unsigned char *DataEnd)
{
  ZipHeader* pHdr=(ZipHeader*)Data;
  const WORD Zip64=45;
  return (0x04034b50==pHdr->Signature
    && pHdr->Method<15
    && Data+MIN_HEADER_LEN+pHdr->FileNameLen+pHdr->ExtraFieldLen<DataEnd);
}

BOOL WINAPI _export IsArchive(const char *Name,const unsigned char *Data,int DataSize)
{
  if (Data[0]=='P' && Data[1]=='K' && Data[2]==5 && Data[3]==6)
  {
    SFXSize=0;
    return(TRUE);
  }
  if (DataSize<MIN_HEADER_LEN) return FALSE;
  const unsigned char *MaxData=Data+DataSize-MIN_HEADER_LEN;
  const unsigned char *DataEnd=Data+DataSize;
  for (const unsigned char *CurData=Data; CurData<MaxData; CurData++)
  {
    if (IsValidHeader(CurData, DataEnd))
    {
      SFXSize=CurData-Data;
      return(TRUE);
    }
  }
  return(FALSE);
}

BOOL WINAPI _export OpenArchive(const char *Name,int *Type)
{
  ArcHandle=CreateFile(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
                       NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if (ArcHandle==INVALID_HANDLE_VALUE)
    return(FALSE);

  *Type=0;

  ArcComment=FALSE;
  FirstRecord=TRUE;

  FileSize=GetFileSize(ArcHandle,NULL);

  char ReadBuf[1024];
  DWORD CurPos,ReadSize;
  int Buf,Found=0;
  CurPos=NextPosition=SetFilePointer(ArcHandle,0,NULL,FILE_END);
  if (CurPos<sizeof(ReadBuf)-18)
    CurPos=0;
  else
    CurPos-=sizeof(ReadBuf)-18;
  for (Buf=0;Buf<64 && !Found;Buf++)
  {
    SetFilePointer(ArcHandle,CurPos,NULL,FILE_BEGIN);
    ReadFile(ArcHandle,ReadBuf,sizeof(ReadBuf),&ReadSize,NULL);
    for (int I=ReadSize-4;I>=0;I--)
      if (ReadBuf[I]==0x50 && ReadBuf[I+1]==0x4b && ReadBuf[I+2]==0x05 &&
          ReadBuf[I+3]==0x06)
      {
        SetFilePointer(ArcHandle,CurPos+I+16,NULL,FILE_BEGIN);
        ReadFile(ArcHandle,&NextPosition,sizeof(NextPosition),&ReadSize,NULL);
        Found=TRUE;
        break;
      }
    if (CurPos==0)
      break;
    if (CurPos<sizeof(ReadBuf)-4)
      CurPos=0;
    else
      CurPos-=sizeof(ReadBuf)-4;
  }
  Truncated=!Found;
  if (Truncated)
    NextPosition=SFXSize;
  return(TRUE);
}


int WINAPI _export GetArcItem(struct PluginPanelItem *Item,struct ArcItemInfo *Info)
{
  struct ZipHd1
  {
    DWORD Mark;
    BYTE UnpVer;
    BYTE UnpOS;
    WORD Flags;
    WORD Method;
    DWORD ftime;
    DWORD CRC;
    DWORD PackSize;
    DWORD UnpSize;
    WORD NameLen;
    WORD AddLen;
  } ZipHd1;

  struct ZipHd2
  {
    DWORD Mark;
    BYTE PackVer;
    BYTE PackOS;
    BYTE UnpVer;
    BYTE UnpOS;
    WORD Flags;
    WORD Method;
    DWORD ftime;
    DWORD CRC;
    DWORD PackSize;
    DWORD UnpSize;
    WORD NameLen;
    WORD AddLen;
    WORD CommLen;
    WORD DiskNum;
    WORD ZIPAttr;
    DWORD Attr;
    DWORD Offset;
  } ZipHeader;

  DWORD ReadSize;

  NextPosition=SetFilePointer(ArcHandle,NextPosition,NULL,FILE_BEGIN);
  if (NextPosition==0xFFFFFFFF)
    return(GETARC_READERROR);
  if (NextPosition>FileSize)
    return(GETARC_UNEXPEOF);
  if (Truncated)
  {
    if (!ReadFile(ArcHandle,&ZipHd1,sizeof(ZipHd1),&ReadSize,NULL))
      return(GETARC_READERROR);
    memset(&ZipHeader,0,sizeof(ZipHeader));
    ZipHeader.UnpVer=ZipHd1.UnpVer;
    ZipHeader.UnpOS=ZipHd1.UnpOS;
    ZipHeader.Flags=ZipHd1.Flags;
    ZipHeader.ftime=ZipHd1.ftime;
    ZipHeader.PackSize=ZipHd1.PackSize;
    ZipHeader.UnpSize=ZipHd1.UnpSize;
    ZipHeader.NameLen=ZipHd1.NameLen;
    ZipHeader.AddLen=ZipHd1.AddLen;
  }
  else
  {
    if (!ReadFile(ArcHandle,&ZipHeader,sizeof(ZipHeader),&ReadSize,NULL))
      return(GETARC_READERROR);
    if (ZipHeader.Mark!=0x02014b50 && ZipHeader.Mark!=0x06054b50)
      if (FirstRecord)
      {
        if (SFXSize>0)
        {
          NextPosition+=SFXSize;
          SetFilePointer(ArcHandle,NextPosition,NULL,FILE_BEGIN);
          if (!ReadFile(ArcHandle,&ZipHeader,sizeof(ZipHeader),&ReadSize,NULL))
            return(GETARC_READERROR);
        }
        if (ZipHeader.Mark!=0x02014b50 && ZipHeader.Mark!=0x06054b50)
        {
          Truncated=TRUE;
          NextPosition=SFXSize;
          return(GetArcItem(Item,Info));
        }
      }
      else
        return(GETARC_UNEXPEOF);
  }

  FirstRecord=FALSE;

  if (ReadSize==0 || ZipHeader.Mark==0x06054b50 ||
      Truncated && ZipHeader.Mark==0x02014b50)
  {
    if (!Truncated && *(WORD *)((char *)&ZipHeader+20)!=0)
      ArcComment=TRUE;
    return(GETARC_EOF);
  }
  DWORD SizeToRead=(ZipHeader.NameLen<NM-1) ? ZipHeader.NameLen : NM-1;
  if (!ReadFile(ArcHandle,Item->FindData.cFileName,SizeToRead,&ReadSize,NULL) ||
      ReadSize!=SizeToRead)
    return(GETARC_READERROR);
  Item->FindData.cFileName[NM-1]=0;

  char *EndPos = Item->FindData.cFileName;
  while( *EndPos )
  {
    if( *EndPos == '/' )
      *EndPos = '\\';
    EndPos++;
  }

/*// Commented out as since NTFS file times support scans the extra field
  // and the file comment is also readed.
  // ZipHeader.PackSize is skipped later (see below)

  long SeekLen=ZipHeader.AddLen+ZipHeader.CommLen;
  if (Truncated)
    SeekLen+=ZipHeader.PackSize;
  NextPosition=SetFilePointer(ArcHandle,SeekLen,NULL,FILE_CURRENT);
*/

  Item->FindData.dwFileAttributes=ZipHeader.Attr & 0x3f;
  Item->FindData.nFileSizeLow=ZipHeader.UnpSize;
  Item->PackSize=ZipHeader.PackSize;
  Item->CRC32=ZipHeader.CRC;
  FILETIME lft;
  DosDateTimeToFileTime(HIWORD(ZipHeader.ftime),LOWORD(ZipHeader.ftime),&lft);
  LocalFileTimeToFileTime(&lft,&Item->FindData.ftLastWriteTime);
  if (ZipHeader.Flags & 1)
    Info->Encrypted=TRUE;
  if (ZipHeader.CommLen > 0)
    Info->Comment=TRUE;
  static char *ZipOS[]={"DOS","Amiga","VAX/VMS","Unix","VM/CMS","Atari ST",
                        "OS/2","Mac-OS","Z-System","CP/M","TOPS-20",
                        "Win32","SMS/QDOS","Acorn RISC OS","Win32 VFAT","MVS",
                        "BeOS","Tandem"};
  if (ZipHeader.PackOS<sizeof(ZipOS)/sizeof(ZipOS[0]))
    strcpy(Info->HostOS,ZipOS[ZipHeader.PackOS]);

  if (ZipHeader.PackOS==11 && ZipHeader.PackVer>20 && ZipHeader.PackVer<25)
    CharToOem(Item->FindData.cFileName,Item->FindData.cFileName);
  Info->UnpVer=(ZipHeader.UnpVer/10)*256+(ZipHeader.UnpVer%10);
  Info->DictSize=32;


//NTFS file times support

  // Search for NTFS extra block
  for(  DWORD dwExtraFieldEnd = SetFilePointer(ArcHandle, 0, NULL, FILE_CURRENT)
          +ZipHeader.AddLen;
        dwExtraFieldEnd > SetFilePointer(ArcHandle, 0, NULL, FILE_CURRENT);
  )
  {
    struct ExtraBlockHeader
    {
      WORD Type;
      WORD Length;
    }
    BlockHead;

    if (!ReadFile(ArcHandle, &BlockHead, sizeof(BlockHead), &ReadSize,NULL)
            || ReadSize!=sizeof(BlockHead) )
      return(GETARC_READERROR);

    if (0xA!=BlockHead.Type) //NTFS Header ID
      // Move to extra block end
      SetFilePointer(ArcHandle, BlockHead.Length, NULL, FILE_CURRENT);
    else
    {
      SetFilePointer(ArcHandle, 4, NULL, FILE_CURRENT); // Skip the reserved 4 bytes

      // Search for file times attribute
      for(  DWORD dwNTFSExtraBlockEnd = SetFilePointer(ArcHandle, 0, NULL, FILE_CURRENT)
              -4+BlockHead.Length;
            dwNTFSExtraBlockEnd > SetFilePointer(ArcHandle, 0, NULL, FILE_CURRENT);
      )
      {
        struct NTFSAttributeHeader
        {
          WORD Tag;
          WORD Length;
        }
        AttrHead;

        if (!ReadFile(ArcHandle, &AttrHead, sizeof(AttrHead), &ReadSize,NULL)
                || ReadSize!=sizeof(AttrHead) )
          return(GETARC_READERROR);

        if (1!=AttrHead.Tag) // File times attribute tag
          // Move to attribute end
          SetFilePointer(ArcHandle, AttrHead.Length, NULL, FILE_CURRENT);
        else
        { // Read file times
          struct TimesAttribute
          {
            FILETIME Modification;
            FILETIME Access;
            FILETIME Creation;
          }
          Times;

          if (!ReadFile(ArcHandle, &Times, sizeof(Times), &ReadSize,NULL)
                  || ReadSize!=sizeof(Times) )
            return(GETARC_READERROR);

          Item->FindData.ftLastWriteTime = Times.Modification;
          Item->FindData.ftLastAccessTime = Times.Access;
          Item->FindData.ftCreationTime = Times.Creation;

          // Interrupt search
          SetFilePointer(ArcHandle, dwExtraFieldEnd, NULL, FILE_BEGIN);
        }
      }
    }
  }
// End of NTFS file times support

// Read the in-archive file comment if any
  if (ZipHeader.CommLen>0)
  {
    DWORD SizeToRead= (ZipHeader.CommLen>255) ? 255 : ZipHeader.CommLen;

    if (!ReadFile(ArcHandle, Info->Description, SizeToRead, &ReadSize, NULL)
            || ReadSize!=SizeToRead )
      return(GETARC_READERROR);

    // Skip comment tail
    SetFilePointer(ArcHandle, ZipHeader.CommLen-ReadSize,NULL,FILE_CURRENT);
  }
//


  long SeekLen=0;
  if (Truncated)
    SeekLen+=ZipHeader.PackSize;
  NextPosition=SetFilePointer(ArcHandle,SeekLen,NULL,FILE_CURRENT);

  return(GETARC_SUCCESS);
}


BOOL WINAPI _export CloseArchive(struct ArcInfo *Info)
{
  if(Info)
  {
    Info->SFXSize=SFXSize;
    Info->Comment=ArcComment;
  }
  return(CloseHandle(ArcHandle));
}

DWORD WINAPI _export GetSFXPos(void)
{
  return SFXSize;
}

BOOL WINAPI _export GetFormatName(int Type,char *FormatName,char *DefaultExt)
{
  if (Type==0)
  {
    strcpy(FormatName,"ZIP");
    strcpy(DefaultExt,"zip");
    return(TRUE);
  }
  return(FALSE);
}


BOOL WINAPI _export GetDefaultCommands(int Type,int Command,char *Dest)
{
  if (Type==0)
  {
    // Correct commands for PKZIP 2.04g!
    static char *Commands[]={
    /*Extract               */"pkunzip -d -o {-s%%P} {%%S} %%a @%%lNM",
    /*Extract without paths */"pkunzip -o {-s%%P} {%%S} %%a @%%lNM",
    /*Test                  */"pkunzip -t {-s%%P} {%%S} %%a @%%lNM",
    /*Delete                */"pkzip -d {-b%%W} {%%S} %%a @%%lNM",
    /*Comment archive       */"pkzip -z {-b%%W} {%%S} %%a",
    /*Comment files         */"pkzip -c {-b%%W} {%%S} %%a @%%lNM",
    /*Convert to SFX        */"zip2exe {%%S} %%a",
    /*Lock archive          */"",
    /*Protect archive       */"",
    /*Recover archive       */"pkzipfix {%%S} %%a",
    /*Add files             */"pkzip -a -whs {-s%%P} {-b%%W} {%%S} %%a @%%lNM",
    /*Move files            */"pkzip -m -whs {-s%%P} {-b%%W} {%%S} %%a @%%lNM",
    /*Add files and folders */"pkzip -a -whs -r -P {-s%%P} {-b%%W} {%%S} %%a @%%lNM",
    /*Move files and folders*/"pkzip -m -whs -r -P {-s%%P} {-b%%W} {%%S} %%a @%%lNM",
    /*"All files" mask      */"*.*"
    };
    if (Command<sizeof(Commands)/sizeof(Commands[0]))
    {
      strcpy(Dest,Commands[Command]);
      return(TRUE);
    }
  }
  return(FALSE);
}
