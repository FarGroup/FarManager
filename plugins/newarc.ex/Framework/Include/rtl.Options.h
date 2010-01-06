#include "Rtl.Base.h"


inline void SetOptions (dword &fOptions, dword fFlags, bool bOn)
{
	if ( bOn )
		fOptions = fOptions | fFlags;
	else
		fOptions &=~ fFlags;
}


inline bool OptionIsOn (dword fOptions, dword fFlags)
{
	return (fOptions & fFlags) == fFlags;
}


inline void SetOptions (int &fOptions, dword fFlags, bool bOn)
{
	if ( bOn )
		fOptions = fOptions | fFlags;
	else
		fOptions &=~ fFlags;
}
