#ifndef __BITFLAGS_HPP__
#define __BITFLAGS_HPP__
/*
bitflags.hpp

Флаги

*/

/* Revision: 1.03 25.06.2002 $ */

/*
Modify:
  25.06.2002 SVS
    ! Косметика:  BitFlags::Skip -> BitFlags::Clear
  14.01.2002 SVS
    + Новый метод - Swap()
  10.01.2002 SVS
    + Новый конструктор
  08.11.2001 SVS
    + Успешно создан (осталось везде выставить то, что надо)
*/

class BitFlags{
  public:
    DWORD Flags;

  public:
    BitFlags(){Flags=0;}
    BitFlags(DWORD Fl){Flags=Fl;}

    ~BitFlags(){}

  public:
    // установить набор флагов
    DWORD Set(DWORD NewFlags){ Flags|=NewFlags;return Flags;}
    // сбросить набор флагов
    DWORD Clear(DWORD NewFlags){ Flags&=~NewFlags;return Flags; }
    // проверить набор флагов
    BOOL  Check(DWORD NewFlags){ return Flags&NewFlags?TRUE:FALSE; }
    // изменить состояние набора флагов в заивисмости от Status
    DWORD Change(DWORD NewFlags,BOOL Status){ if(Status) Flags|=NewFlags; else Flags&=~NewFlags; return Flags;}
    // инвертировать состояние флагов
    DWORD Swap(DWORD SwapedFlags){ if(Flags&SwapedFlags) Flags&=~SwapedFlags; else Flags|=SwapedFlags; return Flags;}
};

#endif // __BITFLAGS_HPP__
