#ifndef __FILEPOSITIONCACHE_HPP__
#define __FILEPOSITIONCACHE_HPP__
/*
poscache.hpp

Кэш позиций в файлах для viewer/editor

*/

/* Revision: 1.05 15.12.2005 $ */

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
    wchar_t *Names;
    int SizeValue;
    int CurPos;

    BYTE *Param;
    BYTE *Position;
//    static char SubKeyItem[16] ,*PtrSubKeyItem;
//    static char SubKeyShort[16],*PtrSubKeyShort;

  private:
    int FindPosition(const wchar_t *FullName);

  public:
    FilePositionCache(int TypeCache);
   ~FilePositionCache();

  public:
    void AddPosition(const wchar_t *Name,void *PosCache);
    BOOL GetPosition(const wchar_t *Name,void *PosCache);

    BOOL Read(const wchar_t *Key);
    BOOL Save(const wchar_t *Key);
};


#endif  // __FILEPOSITIONCACHE_HPP__
