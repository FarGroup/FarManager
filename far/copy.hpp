#ifndef __SHELLCOPY_HPP__
#define __SHELLCOPY_HPP__
/*
copy.hpp

class ShellCopy - Копирование файлов

*/

/* Revision: 1.09 19.06.2001 $ */

/*
Modify:
  19.06.2001 SVS
    + ShellSetAttr - оболочка вокруг SetFileAttributes
  02.06.2001 IS
    ! #define FCOPY* -> enum
      ну, не люблю дефайны (начитался Скотта Мейерса)
    + #include "udlist" & DestList
  30.05.2001 SVS
    ! ShellCopy::CreatePath выведена из класса в отдельню функцию
    + CopyDlgProc()
    + MkSymLink() - как отдельная функция
    ! Немного уменьшим размер объекта за счет замены некоторых переменных
      на флаги.
    + LinkRules() - соблюдение рулесов при линковке
  06.05.2001 DJ
    ! перетрях #include
  01.01.2001 VVM
    + Переменная CopyBufferSize -  размер буфера для копирования
  14.12.2000 SVS
    + CopyToNUL - Признак копирования в NUL
  23.10.2000 VVM
    + Динамический буфер копирования - рабочие переменные
  21.10.2000 SVS
    + Переменная Copy_Buffer_Size -  размер буфера для копирования
  04.08.2000 SVS
    + Опция "Only newer file(s)"
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "dizlist.hpp"
#include "int64.hpp"
#include "udlist.hpp"

class Panel;

enum COPY_CODES {
  COPY_CANCEL,
  COPY_NEXT,
  COPY_FAILURE,
  COPY_SUCCESS,
  COPY_SUCCESS_MOVE
};

enum COPY_FLAGS {
  FCOPY_COPYTONUL       = 0x00000001, // Признак копирования в NUL
  FCOPY_CURRENTONLY     = 0x00000002, //
  FCOPY_ONLYNEWERFILES  = 0x00000004, // Copy only newer files
  FCOPY_CREATESYMLINK   = 0x00000004,
  FCOPY_OVERWRITENEXT   = 0x00000008,
  FCOPY_LINK            = 0x00000010,
  FCOPY_MOVE            = 0x00000040,
  FCOPY_DIZREAD         = 0x00000080,
  FCOPY_COPYSECURITY    = 0x00000100,
  FCOPY_NOSHOWMSGLINK   = 0x00000200,
  FCOPY_VOLMOUNT        = 0x00000400,
};

class ShellCopy
{
  private:
    DWORD Flags;

    Panel *SrcPanel,*AnotherPanel;
    int PanelMode,SrcPanelMode;

    char sddata[16000]; // Security

    DizList DestDiz;
    char DestDizPath[2*NM];

    /* $ 23.10.2000 VVM
       + Динамический буфер копирования - рабочие переменные */
    char *CopyBuffer;
    int CopyBufSize;
    int CopyBufferSize;
    clock_t StartTime;
    clock_t StopTime;
    /* VVM $ */

    char RenamedName[NM],CopiedName[NM];
    int OvrMode,ReadOnlyOvrMode,ReadOnlyDelMode;
    long TotalFiles;
    int SrcDriveType;
    char SrcDriveRoot[NM];
    int SelectedFolderNameLength;
    #ifndef COPY_NOMULTICOPY
    UserDefinedList DestList; // хранение списка целей
    #endif

  private:
    COPY_CODES CopyFileTree(char *Dest);
    COPY_CODES ShellCopyOneFile(char *Src,WIN32_FIND_DATA *SrcData,char *Dest,
                                int KeepPathPos,int Rename);
    int ShellCopyFile(char *SrcName,WIN32_FIND_DATA *SrcData,char *DestName,
                      DWORD DestAttr,int Append);
    int ShellSystemCopy(char *SrcName,char *DestName,WIN32_FIND_DATA *SrcData);
    void ShellCopyMsg(char *Src,char *Dest,int Flags);
    int ShellCopyConvertWildcards(char *Src,char *Dest);
    int DeleteAfterMove(char *Name,int Attr);
    void SetDestDizPath(char *DestPath);
    int AskOverwrite(WIN32_FIND_DATA *SrcData,char *DestName,
        DWORD DestAttr,int SameName,int Rename,int AskAppend,
        int &Append,int &RetCode);
    int GetSecurity(char *FileName,SECURITY_ATTRIBUTES *sa);
    int SetSecurity(char *FileName,SECURITY_ATTRIBUTES *sa);
    int IsSameDisk(char *SrcPath,char *DestPath);
    bool CalcTotalSize();
    int CmpFullNames(char *Src,char *Dest);
    BOOL LinkRules(DWORD *Flags7,DWORD* Flags5,int* Selected5,char *SrcDir,char *DstDir,struct CopyDlgParam *CDP);
    int ShellSetAttr(char *Dest,DWORD Attr);

  public:
    ShellCopy(Panel *SrcPanel,int Move,int Link,int CurrentOnly,int Ask,
              int &ToPlugin,char *PluginDestPath);
    ~ShellCopy();

  public:
    static void ShowBar(int64 WrittenSize,int64 TotalSize,bool TotalBar);
    static void ShowTitle(int FirstTime);
    static long WINAPI CopyDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2);
    static int MkSymLink(char *SelName,char *Dest,DWORD Flags);
};


#endif	// __SHELLCOPY_HPP__
