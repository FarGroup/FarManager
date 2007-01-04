#ifndef __PRESERVELONGNAME_HPP__
#define __PRESERVELONGNAME_HPP__
/*
plognmn.hpp

class PreserveLongName

*/

#include "farconst.hpp"
#include "unicodestring.hpp"


class PreserveLongNameW
{
  private:
    string strSaveLongName;
    string strSaveShortName;
    int Preserve;
  public:
    PreserveLongNameW(const wchar_t *ShortName,int Preserve);
    ~PreserveLongNameW();
};



#endif  // __PRESERVELONGNAME_HPP__
