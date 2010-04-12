#ifndef __LASTERROR_HPP__
#define __LASTERROR_HPP__
/*
lasterror.hpp

Сохрание/восстановление LastError

*/

#include "headers.hpp"
#pragma hdrstop

class GuardLastError
{
	private:
		DWORD LastError;
	public:
		GuardLastError() {LastError=GetLastError();}
		~GuardLastError() {SetLastError(LastError);}
		DWORD Get() {return LastError;}
};

#endif  // __LASTERROR_HPP__
