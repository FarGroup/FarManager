#ifndef __GETFILESTRING_HPP__
#define __GETFILESTRING_HPP__
/*
filestr.hpp

Класс GetFileString

*/

class GetFileString
{
	private:
		char ReadBuf[8192];
		int ReadPos,ReadSize;
		char *Str;
		int StrLength;
		FILE *SrcFile;
	public:
		GetFileString(FILE *SrcFile);
		~GetFileString();
		int GetString(char **DestStr,int &Length);
};

#endif	// __GETFILESTRING_HPP__
