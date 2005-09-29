#ifndef __SCANTREE_HPP__
#define __SCANTREE_HPP__
/*
scantree.hpp

Сканирование текущего каталога и, опционально, подкаталогов на
предмет имен файлов

*/

/* Revision: 1.08 29.09.2005 $ */

/*
Modify:
  29.09.2005 SVS
    + FSCANTREE_USEDALTFOLDERNAME
    + доп.параметр у конструктора ScanTree()
  14.06.2003 SVS
    ! Внедрение новых флагов
    ! Вместо SecondPass[] и FindHandle[] вводим структуру ScanTreeData
    + InsideJunction() - при очередном проходе скажет нам - "мы в симлинке?"
    ! FRS_SCANJUNCTION -> FRS_SCANSYMLINK
  01.06.2003 SVS
    ! переходим на BitFlags
  27.12.2002 VVM
    + Новый параметр ScanFlags. Разные флаги. Пока что только один SF_FILES_FIRST.
  23.06.2002 SVS
    ! Немного красоты ;-)
  26.03.2002 DJ
    ! GetNextName() принимает размер буфера для имени файла
  25.06.2001 IS
    ! Внедрение const
  06.05.2001 DJ
    ! перетрях #include
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "farconst.hpp"
#include "bitflags.hpp"

enum{
  // эту фигню может ставить плагин (младшие 8 бит)
  FSCANTREE_RETUPDIR         = 0x00000001, // = FRS_RETUPDIR
  FSCANTREE_RECUR            = 0x00000002, // = FRS_RECUR
  FSCANTREE_SCANSYMLINK      = 0x00000004, // = FRS_SCANSYMLINK

  // в младшем слове старшие 8 бита служебные!
  FSCANTREE_USEDALTFOLDERNAME= 0x00001000, //
  FSCANTREE_SECONDPASS       = 0x00002000, // то, что раньше было было SecondPass[]
  FSCANTREE_SECONDDIRNAME    = 0x00004000,
  FSCANTREE_INSIDEJUNCTION   = 0x00008000, // - мы внутри симлинка?

  // здесь те флаги, которые могут выставляться в 3-м параметре SetFindPath()
  FSCANTREE_FILESFIRST       = 0x00010000, // Сканирование каталга за два прохода. Сначала файлы, затем каталоги
};

struct ScanTreeData{
  BitFlags Flags;
  HANDLE FindHandle;
};

class ScanTree
{
  private:
    BitFlags Flags;
    struct ScanTreeData Data[NM/2];
    int FindHandleCount;
    char FindPath[4*NM];
    char FindMask[NM];

  private:
    void Init();

  public:
    ScanTree(int RetUpDir,int Recurse=1,int ScanJunction=-1,int UsedAltFolderName=0);
    ~ScanTree();

  public:
    // 3-й параметр - флаги из старшего слова
    void SetFindPath(const char *Path,const char *Mask, const DWORD NewScanFlags = FSCANTREE_FILESFIRST);
    int GetNextName(WIN32_FIND_DATA *fdata,char *FullName, size_t BufSize);
    void SkipDir();
    int IsDirSearchDone() {return Flags.Check(FSCANTREE_SECONDDIRNAME);};
    int InsideJunction()   {return Flags.Check(FSCANTREE_INSIDEJUNCTION);};
};

#endif  // __SCANTREE_HPP__
