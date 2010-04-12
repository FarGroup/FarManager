/*
lockscrn.cpp

class LockScreen

*/

#include "headers.hpp"
#pragma hdrstop

#include "lockscrn.hpp"
#include "scrbuf.hpp"

LockScreen::LockScreen()
{
	ScrBuf.Lock();
}


LockScreen::~LockScreen()
{
	ScrBuf.Unlock();
	ScrBuf.Flush();
}
