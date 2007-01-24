#ifndef __FILEPOSITIONCACHE_HPP__
#define __FILEPOSITIONCACHE_HPP__
/*
poscache.hpp

Кэш позиций в файлах для viewer/editor

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
