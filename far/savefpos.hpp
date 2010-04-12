#ifndef __SAVEFILEPOS_HPP__
#define __SAVEFILEPOS_HPP__
/*
savefpos.hpp

class SaveFilePos

*/

class SaveFilePos
{
	private:
		FILE *SaveFile;
		__int64 SavePos;
	public:
		SaveFilePos(FILE *SaveFile);
		~SaveFilePos();
};


#endif	// __SAVEFILEPOS_HPP__
