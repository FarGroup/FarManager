#include <Rtl.Base.h>

void CutTo (TCHAR *s, TCHAR symbol, bool bInclude)
{
	for (int i = StrLength(s)-1; i >= 0; i--)
		if ( s[i] == symbol )
		{
			bInclude?s[i] = 0:s[i+1] = 0;
			break;
		}

}

void CutToSlash (TCHAR *s)
{
	CutTo (s, _T('\\'), false);
}
