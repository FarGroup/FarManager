#ifndef __SHELLCOPY_HPP__
#define __SHELLCOPY_HPP__
/*
copy.hpp

class ShellCopy - Копирование файлов

*/

#include "dizlist.hpp"
#include "udlist.hpp"

class Panel;

enum COPY_CODES {
  COPY_CANCEL,
  COPY_NEXT,
  COPY_FAILURE,
  COPY_FAILUREREAD,
  COPY_SUCCESS,
  COPY_SUCCESS_MOVE
};

enum COPY_FLAGS {
  FCOPY_COPYTONUL               = 0x00000001, // Признак копирования в NUL
  FCOPY_CURRENTONLY             = 0x00000002, // Только текщий?
  FCOPY_ONLYNEWERFILES          = 0x00000004, // Copy only newer files
  FCOPY_CREATESYMLINK           = 0x00000004, // создание симлинка
  FCOPY_OVERWRITENEXT           = 0x00000008, // Overwrite all
  FCOPY_LINK                    = 0x00000010, // создание связи
  FCOPY_MOVE                    = 0x00000040, // перенос/переименование
  FCOPY_DIZREAD                 = 0x00000080, //
  FCOPY_COPYSECURITY            = 0x00000100, // [x] Copy access rights
  FCOPY_NOSHOWMSGLINK           = 0x00000200, // не показывать месаги при ликовании
  FCOPY_VOLMOUNT                = 0x00000400, // операция монтированния тома
  FCOPY_STREAMSKIP              = 0x00000800, // потоки
  FCOPY_STREAMALL               = 0x00001000, // потоки
  FCOPY_SKIPSETATTRFLD          = 0x00002000, // больше не пытаться ставить атрибуты для каталогов - когда нажали Skip All
  FCOPY_COPYSYMLINKCONTENTS     = 0x00004000, // Копировать содержимое симолических связей?
  FCOPY_COPYPARENTSECURITY      = 0x00008000, // Накладывать родительские права, в случае если мы не копируем права доступа
  FCOPY_LEAVESECURITY           = 0x00010000, // Move: [?] Ничего не делать с правами доступа
  FCOPY_DECRYPTED_DESTINATION   = 0x00020000, // для криптованных файлов - расшифровывать...
  FCOPY_USESYSTEMCOPY           = 0x00040000, // использовать системную функцию копирования
  FCOPY_COPYLASTTIME            = 0x10000000, // При копировании в несколько каталогов устанавливается для последнего.
  FCOPY_UPDATEPPANEL            = 0x80000000, // необходимо обновить пассивную панель
};

class ConsoleTitle;

class ShellCopy
{
  private:
    DWORD Flags;

    Panel *SrcPanel;
    int    SrcPanelMode;
    int    SrcDriveType;

    string strSrcDriveRoot;
    string strSrcFSName;
    DWORD  SrcFSFlags;

    Panel *DestPanel;
    int    DestPanelMode;
    int    DestDriveType;

    string strDestDriveRoot;
    string strDestFSName;
    DWORD  DestFSFlags;

    char   *sddata; // Security

    DizList DestDiz;

    string strDestDizPath;

    char *CopyBuffer;
    int CopyBufSize;
    int CopyBufferSize;
    clock_t StartTime;
    clock_t StopTime;

    string strCopiedName;
    string strRenamedName;

    /* $ 02.03.2002 KM
      + Новое свойство SkipMode - для пропуска при копировании
        залоченных файлов.
    */
    int OvrMode;
    int ReadOnlyOvrMode;
    int ReadOnlyDelMode;
    int SkipMode;
    int SkipEncMode;
    /* KM $ */

    long TotalFiles;
    int SelectedFolderNameLength;

    UserDefinedListW DestListW;
    ConsoleTitle *CopyTitle;   // заголовок

  private:
    COPY_CODES CopyFileTreeW(const wchar_t *Dest);

    COPY_CODES ShellCopyOneFileW(const wchar_t *Src,
                                const FAR_FIND_DATA_EX &SrcData,
                                const wchar_t *Dest,
                                int KeepPathPos, int Rename);


    COPY_CODES CheckStreamsW(const wchar_t *Src,const wchar_t *DestPath);


    int  ShellCopyFileW(const wchar_t *SrcName,const FAR_FIND_DATA_EX &SrcData,
                       const wchar_t *DestName, DWORD DestAttr,int Append);


    int  ShellSystemCopyW(const wchar_t *SrcName,const wchar_t *DestName,const FAR_FIND_DATA_EX &SrcData);

    int  DeleteAfterMoveW(const wchar_t *Name,int Attr);

    void SetDestDizPathW(const wchar_t *DestPath);

    int  AskOverwriteW(const FAR_FIND_DATA_EX &SrcData,const wchar_t *DestName,
                      DWORD DestAttr,int SameName,int Rename,int AskAppend,
                      int &Append,int &RetCode);


    int  GetSecurityW(const wchar_t *FileName,SECURITY_ATTRIBUTES &sa);
    int  SetSecurityW(const wchar_t *FileName,const SECURITY_ATTRIBUTES &sa);
    int  SetRecursiveSecurityW(const wchar_t *FileName,const SECURITY_ATTRIBUTES &sa);

    int  IsSameDiskW(const wchar_t *SrcPath,const wchar_t *DestPath);

    bool CalcTotalSize();

    string& GetParentFolderW(const wchar_t *Src, string &strDest);

    int  CmpFullNamesW(const wchar_t *Src,const wchar_t *Dest);

    int  CmpFullPathW(const wchar_t *Src,const wchar_t *Dest);

    BOOL LinkRulesW(DWORD *Flags7,DWORD* Flags5,int* Selected5,const wchar_t *SrcDir,const wchar_t *DstDir,struct CopyDlgParam *CDP);

    int  ShellSetAttrW(const wchar_t *Dest,DWORD Attr);

    BOOL CheckNulOrConW(const wchar_t *Src);

    void CheckUpdatePanel(); // выставляет флаг FCOPY_UPDATEPPANEL

    void ShellCopyMsgW(const wchar_t *Src,const wchar_t *Dest,int Flags);

  public:
    ShellCopy(Panel *SrcPanel,int Move,int Link,int CurrentOnly,int Ask,
              int &ToPlugin, const wchar_t *PluginDestPath, bool bUnicode /*=FAKE!! for overload*/);

    ~ShellCopy();

  public:
    static int  ShowBar(unsigned __int64 WrittenSize,unsigned __int64 TotalSize,bool TotalBar);
    static void ShowTitle(int FirstTime);
    static LONG_PTR WINAPI CopyDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2);
//    static long WINAPI CopyDlgProcW(HANDLE hDlg,int Msg,int Param1,long Param2);
    static int  MkSymLinkW(const wchar_t *SelName,const wchar_t *Dest,DWORD Flags);
    static void PR_ShellCopyMsgW(void);
};


#endif  // __SHELLCOPY_HPP__
