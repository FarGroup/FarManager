#ifndef __BLOCKEXTKEY_HPP__
#define __BLOCKEXTKEY_HPP__
/*
BlockExtKey.hpp

ƒобавка к исключателю - блокировка активных действий

Ётот класс в основном используетс€ только в исключателе и при
загрузке плагинов при старте ‘ј–а. ÷ель - заблокировать
некоторые интересные клавиши (типа Alt-F9 и еже с ним), т.к.
в это врем€ возможен полнейший бардак с прорисовкой, зацикливаением
и прочими прелест€ми...

*/

/* Revision: 1.00 21.02.2002 $ */

/*
Modify:
  21.02.2002 SVS
    ! Created
*/
#include "global.hpp"

class BlockExtKey{
   int OldIsProcessAssignMacroKey, OldIsProcessVE_FindFile;
 public:
   BlockExtKey()
   {
     OldIsProcessAssignMacroKey=IsProcessAssignMacroKey;
     IsProcessAssignMacroKey=1;
     OldIsProcessVE_FindFile=IsProcessVE_FindFile;
     IsProcessVE_FindFile=0;
   }
  ~BlockExtKey(){IsProcessAssignMacroKey=OldIsProcessAssignMacroKey; IsProcessVE_FindFile=OldIsProcessVE_FindFile;}
};

#endif // __BLOCKEXTKEY_HPP__
