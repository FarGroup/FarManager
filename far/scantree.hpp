#ifndef __SCANTREE_HPP__
#define __SCANTREE_HPP__
/*
scantree.hpp

Сканирование текущего каталога и, опционально, подкаталогов на
предмет имен файлов

*/

/* Revision: 1.02 25.05.2000 $ */

/*
Modify:
  25.06.2001 IS
    ! Внедрение const
  06.05.2001 DJ
    ! перетрях #include
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "farconst.hpp"

class ScanTree
{
  private:
    void Init();

    HANDLE FindHandle[NM/2];
    int SecondPass[NM/2];
    int FindHandleCount;
    int RetUpDir;
    int Recurse;
    int SecondDirName;
    char FindPath[2*NM];
    char FindMask[NM];
  public:
    ScanTree(int RetUpDir,int Recurse=1);
    ~ScanTree();
    void SetFindPath(const char *Path,const char *Mask);
    int GetNextName(WIN32_FIND_DATA *fdata,char *FullName);
    void SkipDir();
    int IsDirSearchDone() {return(SecondDirName);};
};


#endif	// __SCANTREE_HPP__
