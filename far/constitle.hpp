#ifndef __CONSTITLE_HPP__
#define __CONSTITLE_HPP__
/*
constitle.hpp

Заголовок консоли

*/


class ConsoleTitle
{
	private:
		char OldTitle[512];

	public:
		ConsoleTitle(char *title=NULL);
		~ConsoleTitle();

	public:
		void Set(char *fmt,...);

};

#endif
