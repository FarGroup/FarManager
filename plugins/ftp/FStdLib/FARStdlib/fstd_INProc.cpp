#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

/** @class FARINProc

    Used to mark nested, named code block.
    Designed for use as procedure Enter|Exit marker.

    Write to log name of procedure and given parameters at start and increment log indent level.
    Do not use directly.
    Use PROC macro instead, which defined differentry for debug and release.
*/

/** @def Log(v)
    Log(( <log data> ));

    Writes data to current log file indenting output by current indent level.
*/

/** @def PROC(v)
    PROC(( <procedure name>, <procedure definition> ))

    Define procedure and write procedure header into current log file.
    Increase current log indent level.
    Indent level will be decreased after code block where PROC placed ends.
*/

/** @var FARINProc::Counter
    @brief Current global log file indent level.
*/
int FARINProc::Counter = 0;

/** @brief Writes named data to log;

    Writes named data to log file and increment log indent level.

    @param Name     name of indent block. Usually function name.
    @param Format   printf-like format string for formatted output following parameters

    @return <nothing>
*/
FARINProc::FARINProc(LPCSTR Name,LPCSTR Format,...)
	: Name(Name)
{
	va_list  ap;
	char     str[500];
	sprintf(str, "%*c%s(", Counter*2,' ',Name);

	if(Format)
	{
		va_start(ap,Format);
		StrCat(str,MessageV(Format,ap),sizeof(str));
		va_end(ap);
	}

	StrCat(str,") {",sizeof(str));
	FP_FILELog("%s",str);
	Counter++;
}

FARINProc::~FARINProc()
{
	Counter--;
	FP_FILELog("%*c}<%s>", Counter*2,' ',Name);
}

void __cdecl FARINProc::Say(LPCSTR s,...)
{
	va_list ap;
	va_start(ap,s);
	FP_FILELog("%*c%s", Counter*2,' ', MessageV(s,ap));
	va_end(ap);
}
