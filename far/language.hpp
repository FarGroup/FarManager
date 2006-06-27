#ifndef __LANGUAGE_HPP__
#define __LANGUAGE_HPP__
/*
language.hpp

������ � LNG-�������

*/

/* Revision: 1.12 05.04.2006 $ */

#include "farconst.hpp"
#include "unicodestring.hpp"

#define TYPE_ANSI       0
#define TYPE_UNICODE    1
#define TYPE_REVERSEBOM 2
#define TYPE_UTF8       3

class VMenu;

class Language
{
  private:
    char **MsgAddr;
    char *MsgList;

    wchar_t **MsgAddrW;
    wchar_t *MsgListW;

    long MsgSize;
    long MsgSizeW;

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

    char* GetMsg(int MsgId);
    wchar_t* GetMsgW (int nID);

    static FILE* OpenLangFile(const wchar_t *Path,const wchar_t *Mask,const wchar_t *Language,string &strFileName, int &nType, BOOL StrongLang=FALSE);
    static int GetLangParam(FILE *SrcFile,const wchar_t *ParamName,string *strParam1, string *strParam2, int nType);
    /* $ 01.09.2000 SVS
      + ����� �����, ��� ��������� ���������� ��� .Options
        .Options <KeyName>=<Value>
    */
    static int GetOptionsParam(FILE *SrcFile,const wchar_t *KeyName,string &strValue, int nType);
    /* SVS $ */
    static int Select(int HelpLanguage,VMenu **MenuPtr);
};

extern Language Lang;

#endif  // __LANGUAGE_HPP__
