#ifndef __FILEPOSITIONCACHE_HPP__
#define __FILEPOSITIONCACHE_HPP__
/*
poscache.hpp

Кэш позиций в файлах для viewer/editor

*/

/* Revision: 1.04 17.12.2002 $ */

/*
Modify:
  17.12.2002 SVS
    ! класс FilePositionCache подвергся значительным переделкам,
     т.к. позиции вьювере измеряются в терминах __int64, а редактора - DWORD
     Т.е. класс теперь понимает 2 типа кэша - FPOSCACHE_32 и FPOSCACHE_64.
  17.06.2001 IS
    + include "udlist.hpp"
  02.04.2001 VVM
    + int FindPosition()
  24.09.2000 SVS
    + Работа по сохранению/восстановлению позиций в файле по RCtrl+<N>
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#define MAX_POSITIONS 64

enum {
  FPOSCACHE_32,
  FPOSCACHE_64,
};

struct TPosCache32{
  DWORD Param[5];
  DWORD *Position[4];
};

struct TPosCache64{
  __int64 Param[5];
  __int64 *Position[4];
};

class FilePositionCache
{
  private:
    int IsMemory;
    char *Names;
    int SizeValue;
    int CurPos;

    BYTE *Param;
    BYTE *Position;
    static char SubKeyItem[16] ,*PtrSubKeyItem;
    static char SubKeyShort[16],*PtrSubKeyShort;

  private:
    int FindPosition(const char *FullName);

  public:
    FilePositionCache(int TypeCache);
   ~FilePositionCache();

  public:
    void AddPosition(const char *Name,void *PosCache);
    BOOL GetPosition(const char *Name,void *PosCache);

    BOOL Read(const char *Key);
    BOOL Save(const char *Key);
};


#endif  // __FILEPOSITIONCACHE_HPP__
