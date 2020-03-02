#include "Rtl.Base.h"


inline void SetOptions (DWORD &fOptions, DWORD fFlags, bool bOn)
{
	if ( bOn )
		fOptions = fOptions | fFlags;
	else
		fOptions &=~ fFlags;
}


inline bool OptionIsOn (DWORD fOptions, DWORD fFlags)
{
	return (fOptions & fFlags) == fFlags;
}


inline void SetOptions (int &fOptions, DWORD fFlags, bool bOn)
{
	if ( bOn )
		fOptions = fOptions | fFlags;
	else
		fOptions &=~ fFlags;
}
