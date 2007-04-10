#ifndef __LANGUAGE_HPP__
#define __LANGUAGE_HPP__
/*
language.hpp

Работа с LNG-файлами

*/

#include "farconst.hpp"
#include "unicodestring.hpp"


class VMenu;

class Language
{
  private:
    wchar_t **MsgAddr;
    wchar_t *MsgList;

    char **MsgAddrA; //фантастика, да
    char *MsgListA;

    long MsgSize;

    int MsgCount;

    string strMessageFile;

  private:
    void ConvertString(const wchar_t *Src,string &strDest);
    BOOL CheckMsgId(int MsgId);
    void Free();

  public:
    Language();
    ~Language();

  public:
    int Init(const wchar_t *Path,int CountNeed=-1);
    void Close();

    const wchar_t* GetMsg (int nID);
    const char* GetMsgA (int nID);

    static FILE* OpenLangFile(const wchar_t *Path,const wchar_t *Mask,const wchar_t *Language,string &strFileName, int &nCodePage, BOOL StrongLang=FALSE, string *pLangName=NULL);
    static int GetLangParam(FILE *SrcFile,const wchar_t *ParamName,string *strParam1, string *strParam2, int nCodePage);
    /* $ 01.09.2000 SVS
      + Новый метод, для получения параметров для .Options
        .Options <KeyName>=<Value>
    */
    static int GetOptionsParam(FILE *SrcFile,const wchar_t *KeyName,string &strValue, int nCodePage);
    /* SVS $ */
    static int Select(int HelpLanguage,VMenu **MenuPtr);
};

extern Language Lang;

#endif  // __LANGUAGE_HPP__
