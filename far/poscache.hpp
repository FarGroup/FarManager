#ifndef __FILEPOSITIONCACHE_HPP__
#define __FILEPOSITIONCACHE_HPP__
/*
poscache.hpp

Кэш позиций в файлах для viewer/editor

*/

/* Revision: 1.02 02.04.2001 $ */

/*
Modify:
  02.04.2001 VVM
    + int FindPosition()
  24.09.2000 SVS
    + Работа по сохранению/восстановлению позиций в файле по RCtrl+<N>
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#define MAX_POSITIONS 64

class FilePositionCache
{
  private:
    int IsMemory;
    char *Names;
    unsigned int *Positions;
    long *ShortPos;
    int CurPos;

    int FindPosition(char *Name);
  public:
    FilePositionCache();
   ~FilePositionCache();

  public:
    void AddPosition(char *Name,unsigned int Position1,unsigned int Position2,
                     unsigned int Position3,unsigned int Position4,
                     unsigned int Position5,
                     long *PosLine,long *PosCursor,long *PosScreenLine,
                     long *PosLeftPos);
    void GetPosition(char *Name,unsigned int &Position1,unsigned int &Position2,
                     unsigned int &Position3,unsigned int &Position4,
                     unsigned int &Position5,
                     long *PosLine,long *PosCursor,long *PosScreenLine,
                     long *PosLeftPos);
    void Read(char *Key);
    void Save(char *Key);
};


#endif	// __FILEPOSITIONCACHE_HPP__
