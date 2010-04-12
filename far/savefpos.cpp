/*
savefpos.cpp

class SaveFilePos

*/

#include "headers.hpp"
#pragma hdrstop

#include "savefpos.hpp"
#include "fn.hpp"

SaveFilePos::SaveFilePos(FILE *SaveFile)
{
	SaveFilePos::SaveFile=SaveFile;

	if (SaveFile)
		SavePos=ftell64(SaveFile);
}


SaveFilePos::~SaveFilePos()
{
	if (SaveFile)
		fseek64(SaveFile,SavePos,SEEK_SET);
}
