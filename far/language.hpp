#ifndef __LANGUAGE_HPP__
#define __LANGUAGE_HPP__
/*
language.hpp

Работа с LNG-файлами

*/

/* Revision: 1.01 01.09.2000 $ */

/*
Modify:
  01.09.2000 SVS
    + Новый метод, для получения параметров для .Options
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#ifndef __STDIO_H
#include <stdio.h>
#endif

class Language
{
  private:
    void ConvertString(char *Src,char *Dest);
    char **MsgAddr;
    char *MsgList;
    long MsgSize;
    int MsgCount;
    char MessageFile[NM];
  public:
    Language();
    int Init(char *Path);
    void Close();
    char* GetMsg(int MsgId);
    static FILE* OpenLangFile(char *Path,char *Mask,char *Language,char *FileName);
    static int GetLangParam(FILE *SrcFile,char *ParamName,char *Param1,char *Param2);
    /* $ 01.09.2000 SVS
      + Новый метод, для получения параметров для .Options
        .Options <KeyName>=<Value>
    */
    static int GetOptionsParam(FILE *SrcFile,char *KeyName,char *Value);
    /* SVS $ */
    static int Select(int HelpLanguage,VMenu **MenuPtr);
};

#endif	// __LANGUAGE_HPP__
