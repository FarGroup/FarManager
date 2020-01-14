#ifdef _MSC_VER
#define __midl  // do not include inline implementation
#endif

#include "crt.hpp"

//#if !(defined(_MSC_VER) && _MSC_VER >= 1900 && defined(UNICODE))

extern "C"
{
	TCHAR* __cdecl
#ifndef UNICODE
		strtok
#else
		wcstok
#endif
		(TCHAR* string, const TCHAR* control)
	{
#ifndef UNICODE
		typedef unsigned char token_t;
#else
		typedef wchar_t       token_t;
#endif
		token_t* str;
		const token_t* ctrl = (const token_t*)control;

		static token_t* nextoken;

#ifndef UNICODE
		unsigned char map[32];
		int count;

		for (count = 0; count < 32; count++)
			map[count] = 0;

		do
		{
			map[*ctrl >> 3] |= (1 << (*ctrl & 7));
		} while (*ctrl++);
#endif

		if (string)
			str = (token_t*)string;
		else
			str = nextoken;

#ifndef UNICODE
		while ((map[*str >> 3] & (1 << (*str & 7))) && *str)
			str++;
#else
		while (*str) {
			for (ctrl = control; *ctrl && *ctrl != *str; ctrl++) /* empty loop */;
			if (!*ctrl) break;
			str++;
		}
#endif

		string = (TCHAR*)str;

		for (; *str; str++) {
#ifndef UNICODE
			if (map[*str >> 3] & (1 << (*str & 7)))
			{
				*str++ = '\0';
				break;
			}
#else
			for (ctrl = control; *ctrl && *ctrl != *str; ctrl++)  /* empty loop */;
			if (*ctrl) {
				*str++ = '\0';
				break;
			}
#endif
		}

		nextoken = str;

		if (string == (TCHAR*)str)
			return NULL;
		else
			return string;
	}
}
//#endif