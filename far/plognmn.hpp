#ifndef __PRESERVELONGNAME_HPP__
#define __PRESERVELONGNAME_HPP__
/*
plognmn.hpp

class PreserveLongName

*/

#include "farconst.hpp"
#include "unicodestring.hpp"


class PreserveLongName
{
	private:
		string strSaveLongName;
		string strSaveShortName;
		int Preserve;
	public:
		PreserveLongName(const wchar_t *ShortName,int Preserve);
		~PreserveLongName();
};



#endif  // __PRESERVELONGNAME_HPP__
