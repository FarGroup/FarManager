#ifndef __MENUBAR_HPP__
#define __MENUBAR_HPP__
/*
menubar.hpp

Показ горизонтального меню при включенном "Always show menu bar"

*/

#include "scrobj.hpp"

class MenuBar:public ScreenObject
{
	private:
		virtual void DisplayObject();

	public:
		MenuBar() {};
		virtual ~MenuBar() {};
};

#endif  // __MENUBAR_HPP__
