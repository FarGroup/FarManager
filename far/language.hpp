#ifndef __LANGUAGE_HPP__
#define __LANGUAGE_HPP__
/*
language.hpp

Работа с LNG-файлами

*/

/* Revision: 1.07 29.04.2003 $ */

/*
Modify:
  29.04.2003 SVS
    ! из GetMsg вынесем код проверки в отдельную функцию CheckMsgId
  14.07.2002 IS
    ! внедрение const
  24.12.2001 SVS
    + Доп.параметр у OpenLangFile() - StrongLang: "только заданный язык и не более"
      По умолчанию StrongLang=FALSE (как и раньше)
  06.05.2001 DJ
    ! перетрях #include
  27.02.2001 SVS
    ! Нафига сюда впиндювивали stdio.h - ума не дам...
  19.01.2001 SVS
    + дополнительный параметр для Init - количество нужных строк
  01.09.2000 SVS
    + Новый метод, для получения параметров для .Options
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "farconst.hpp"

class VMenu;

class Language
{
  private:
    char **MsgAddr;
    char *MsgList;
    long MsgSize;
    int MsgCount;
    char MessageFile[NM];

  private:
    void ConvertString(char *Src,char *Dest);
    BOOL CheckMsgId(int MsgId);

  public:
    Language();

  public:
    int Init(char *Path,int CountNeed=-1);
    void Close();
    char* GetMsg(int MsgId);

    static FILE* OpenLangFile(const char *Path,const char *Mask,const char *Language,char *FileName,BOOL StrongLang=FALSE);
    static int GetLangParam(FILE *SrcFile,char *ParamName,char *Param1,char *Param2);
    /* $ 01.09.2000 SVS
      + Новый метод, для получения параметров для .Options
        .Options <KeyName>=<Value>
    */
    static int GetOptionsParam(FILE *SrcFile,char *KeyName,char *Value);
    /* SVS $ */
    static int Select(int HelpLanguage,VMenu **MenuPtr);
};

extern Language Lang;

#endif  // __LANGUAGE_HPP__
