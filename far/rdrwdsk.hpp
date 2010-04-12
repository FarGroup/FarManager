#ifndef __REDRAWDESKTOP_HPP__
#define __REDRAWDESKTOP_HPP__
/*
rdrwdsk.hpp

class RedrawDesktop

*/

class RedrawDesktop
{
	private:
		int LeftVisible;
		int RightVisible;
		int KeyBarVisible;
		int TopMenuBarVisible;

	public:
		RedrawDesktop(BOOL IsHidden=FALSE);
		~RedrawDesktop();
};


#endif  // __REDRAWDESKTOP_HPP__
