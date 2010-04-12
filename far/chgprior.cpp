/*
chgprior.cpp

class ChangePriority

*/

#include "headers.hpp"
#pragma hdrstop

#ifndef __CHANGEPRIORITY_HPP__
#include "chgprior.hpp"
#endif


ChangePriority::ChangePriority(int NewPriority)
{
	SavePriority=GetThreadPriority(GetCurrentThread());
	SetThreadPriority(GetCurrentThread(),NewPriority);
}


ChangePriority::~ChangePriority()
{
	SetThreadPriority(GetCurrentThread(),SavePriority);
}
