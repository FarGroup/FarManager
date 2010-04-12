#ifndef __PRESERVELONGNAME_HPP__
#define __PRESERVELONGNAME_HPP__
/*
plognmn.hpp

class PreserveLongName

*/

#include "farconst.hpp"

class PreserveLongName
{
	private:
		char SaveLongName[NM],SaveShortName[NM];
		int Preserve;
	public:
		PreserveLongName(char *ShortName,int Preserve);
		~PreserveLongName();
};



#endif  // __PRESERVELONGNAME_HPP__
