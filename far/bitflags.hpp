#ifndef __BITFLAGS_HPP__
#define __BITFLAGS_HPP__
/*
bitflags.hpp

Флаги

*/

/* Revision: 1.00 08.11.2001 $ */

/*
Modify:
  08.11.2001 SVS
    + Успешно создан (осталось везде выставить то, что надо)
*/

class BitFlags{
  public:
    DWORD Flags;

  public:
    BitFlags(){Flags=0;}
    ~BitFlags(){}

  public:
    // установить набор флагов
    DWORD Set(DWORD NewFlags){ Flags|=NewFlags;return Flags;}
    // сбросить набор флагов
    DWORD Skip(DWORD NewFlags){ Flags&=~NewFlags;return Flags; }
    // проверить набор флагов
    BOOL  Check(DWORD NewFlags){ return Flags&NewFlags; }
    // изменить состояние набора флагов
    DWORD Change(DWORD NewFlags,BOOL Status){ if(Status) Flags|=NewFlags; else Flags&=~NewFlags; return Flags;}
};

#endif // __BITFLAGS_HPP__
