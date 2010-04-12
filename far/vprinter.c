/*-----------------------------------------------------------------------*
 * filename - vprinter.c
 *
 * function(s)
 *        Hex8       - converts int to 8 hex digits
 *        __vprinter - sends formatted output
 *-----------------------------------------------------------------------*/

/*
 *      C/C++ Run Time Library - Version 11.0
 *
 *      Copyright (c) 1987, 2002 by Borland Software Corporation
 *      All Rights Reserved.
 *
 */

#include <stdio.h>
//#include <_printf.h>
#include <string.h>
#include <stdlib.h>
#include <values.h>
#include <tchar.h>
#include <limits.h>

#if !defined(__STDARG_H)
#include <stdarg.h>
#endif

#if !defined(__STDDEF_H)
#include <stddef.h>
#endif


#define __XCVTDIG__  40   /* xcvt() maximum digits supported    */
#define __XCVTMAX__  47   /* bytes for longest xcvt() string    */
#define __LTOAMAX__  32   /* bytes for radix 2 longtoa() string */
#define __CVTMAX__   48   /* MAX of the above, + 1 for \0       */

/*
    Defining the symbol FLOATS_32_BIT enables recognition of
    32 bit floats on the stack by using the 'h' size modifier.
    YOU, must FORCE the float to be pushed as a 32 bit number
    though!  The compiler normally widens floats to 64 bit doubles.

    example:
            union {
                float f;
                long l
                } u;
            printf("%hg\n", u.l);

    This feature can be DISABLED by default for strict ANSI compatability.
*/
#define FLOATS_32_BIT       1

/*
    Floating point parameter types to formatting functions.
    The numbers are the offset of the fraction in the data type.
*/
#define F_4byteFloat    2
#define F_8byteFloat    6
#define F_10byteFloat   8

/* These must NEVER be the same as an unnormalized 87' exponent! */

#define NAN_number  32766   /* xcvt() -> as NAN exponent    */
#define INF_number  32767   /* xcvt() -> as INF exponent    */



/****************************************************************************/
/* Set up some UNICODE defines */
#define DEFWIDTH  (sizeof (_TCHAR) == sizeof (wchar_t))
static  char    NullStringA[] =  "(null)";
static  wchar_t NullStringW[] = L"(null)";
#ifdef _UNICODE
#  define DefNullString (NullStringW)
#else
#  define DefNullString (NullStringA)
#endif /* _UNICODE */

#define MAX_BUF_LEN      512
#if (MAX_BUF_LEN < __CVTMAX__)
#  error MAX_BUF_LEN Must be >= __CVTMAX__ !
#endif
typedef unsigned char    bits08;
/****************************************************************************/
typedef unsigned putnF(const _TCHAR *ptr, unsigned n, void *fp, int eos_flag);

#define Ssize 80
typedef struct
{
	_TCHAR  S[Ssize];
	int     Scount;
	putnF   *putter;
	void    *outP;
	int     totalSent;
	int     eof_error;
	size_t *pnsize;
} PutRec;


/*-----------------------------------------------------------------------*

Name            __int64toa - converts a __int64 to a character string
                __int64tow - converts a __int64 to a wide-character string

Usage           char *    __int64toa (__int64 value, char *strP, int radix,
                                      char maybeSigned, char hexStyle);
                wchar_t * __int64tow (__int64 value, wchar_t *strP, int radix,
                                      char maybeSigned, wchar_t hexStyle);

Prototype in    _printf.h

Description     This function converts a long value to a  null-terminated string
                and  stores the result in  string strP.

                radix specifies the base to be used in converting value. it
                must be between  2 and 36 (inclusive).

                maybeSigned is treated as a boolean. If false then value is
                treated  as unsigned  long and  no sign  will be  placed in
                *strP.

                hexStyle  may take  the values  'a' or  'A' and  determines
                whether lower or  upper case alphabetics are used  when the
                radix is 11 or greater.

                Note: The space  allocated for string must be  large enough
                to hold the returned  string including the terminating null
                character (\0).  The maximum is the length the number
                of bits in the integral type plus one for the terminator.

Return value    pointer to the string

*------------------------------------------------------------------------*/

static _TCHAR * __int64tot(__int64 value, _TCHAR *strP, int radix,
                           _TCHAR maybeSigned, _TCHAR hexStyle)
{
	_TCHAR    buf [65];
	_TCHAR    c, *p, *bufp;
	p = strP;

	/* If the requested radix is invalid, generate an empty result.
	 */
	if (radix >= 2 && radix <= 36)
	{
		/* If the value is signed and less than zero, generate a minus sign.
		 */
		if (value < 0 && maybeSigned != 0)
		{
			*p++ = _TEXT('-');
			value = -value;
		}

		/* Now loop, taking each digit as modulo radix, and reducing the value
		 * by dividing by radix, until the value is zeroed.  Note that
		 * at least one loop occurs even if the value begins as 0,
		 * since we want "0" to be generated rather than "".
		 */
		bufp = buf;

		for (;;)
		{
			*bufp++ = (char)((unsigned __int64)value % radix);

			if ((value = (unsigned __int64)value / radix) == 0)
				break;
		}

		/* The digits in the buffer must now be copied in reverse order into
		 * the target string, translating to ASCII as they are moved.
		 */
		while (bufp != buf)
		{
			if ((c = *--bufp) < 10)
				*p++ = (_TCHAR)(c + _TEXT('0'));
			else
				*p++ = (_TCHAR)((c - 10) + hexStyle);
		}
	}

	/* terminate the output string with a zero.
	 */
	*p = _TEXT('\0');
	return (strP);          /* return a pointer to the string */
}

/*-----------------------------------------------------------------------*

Name            PutFlush - flush PutRec buffer to putter function

Usage           int PutFlush( PutRec *p )

Description     Flushes the PutRec buffer to the putter function.
                If an error occurs, p->eof_error is set to 1.

*------------------------------------------------------------------------*/

static void PutFlush(PutRec *p)
{
	int eos_flag = 1;

	if (p->Scount != 0)
	{
		if (p->pnsize && ((size_t)p->totalSent) >= (*p->pnsize))
			eos_flag = 0;

		if (!(*(p->putter))(p->S, p->Scount, p->outP, eos_flag))
			p->eof_error = 1;

		p->Scount = 0;
	}
}

/*-----------------------------------------------------------------------*

Name            PutToS - output a character to putter function

Usage           static void PutToS( _TCHAR c, PutRec *p )

Description     Output one character to the _putrec buffer, and flush
                the buffer when full to the putter function.
                If an error occurs, p->eof_error is set to 1.

*------------------------------------------------------------------------*/

static void PutToS(_TCHAR c, PutRec *p)
{
	if (p->Scount >= Ssize)
		PutFlush(p);

	/* Only add to the buffer if we haven't gone over the snprintf limit or
	   if none is specified (regular printf stuff)
	 */
	if ((!p->pnsize) || ((size_t)p->totalSent < *p->pnsize))
	{
		p->S[p->Scount] = c;
		p->Scount++;
	}

	p->totalSent++;
}

/*-----------------------------------------------------------------------*

Name            Hex8 - converts int to 8 hex digits

Usage           static void Hex8( unsigned n, _TCHAR *buf )

Description     Convert 32 bit parameter (n) to 8 hex digits at buf.

*------------------------------------------------------------------------*/

static void Hex8(unsigned long n, _TCHAR *buf)
{
	int i, c;

	for (i = 7; i >= 0; i--)
	{
		if ((c = (int)n & 0x0f) < 10)
			buf[i] = c + _TEXT('0');
		else
			buf[i] = c - 10 + _TEXT('A');

		n >>= 4;
	}
}

/*-----------------------------------------------------------------------*

__vprinter is a table-driven design, for speed and flexibility. There are
two tables.  The first table classifies all 7-bit ASCII chars and then the
second table is the switch table which points to the function blocks which
handle the various classes of characters.

All characters with the 8th bit set are currently classed as don't cares,
which is the class of character also used for normal alphabetics.  All
characters less than ' ' (0x20) are also classed as don't cares.

*------------------------------------------------------------------------*/

typedef
enum
{
	_si,    /* sign fill +/-        */
	_af,    /* alternate form       */
	_ar,    /* format (width or precision) by argument */
	_lj,    /* left justify         */

	_pr,    /* precision            */
	_nu,    /* numeral              */
	_lo,    /* long                 */
	_ld,    /* long double, __int64 */
	_sh,    /* short, narrow        */
	_fz,    /* fill zeros           */

	_de,    /* decimal              */
	_oc,    /* octal                */
	_un,    /* unsigned decimal     */
	_he,    /* hexadecimal          */

	_pt,    /* pointer              */
	_fl,    /* float                */
	_ch,    /* char                 */
	_st,    /* string               */
	_Ch,    /* wide char            */
	_St,    /* wide string          */

	_ns,    /* number sent          */
	_zz,    /* terminator           */
	_dc,    /* don't care           */
	_pc,    /* percent              */

	_ne,    /* near pointer         */
	_fa,    /* far pointer          */
	_wi,    /* wide (for I64)       */
} characterClass;

/*  Here is the table of classes, indexed by character. */

static bits08 printCtype [96] =
{
	/*       SP   !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /  */
	_si,_dc,_dc,_af,_dc,_pc,_dc,_dc,_dc,_dc,_ar,_si,_dc,_lj,_pr,_dc,

	/*        0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?  */
	_fz,_nu,_nu,_nu,_nu,_nu,_nu,_nu,_nu,_nu,_dc,_dc,_dc,_dc,_dc,_dc,

	/*        _   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O  */
	_dc,_dc,_dc,_Ch,_dc,_fl,_fa,_fl,_sh,_wi,_dc,_dc,_ld,_dc,_ne,_dc,

	/*        P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _  */
	_dc,_dc,_dc,_St,_dc,_dc,_dc,_dc,_he,_dc,_dc,_dc,_dc,_dc,_dc,_dc,

	/*        `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o  */
	_dc,_dc,_dc,_ch,_de,_fl,_fl,_fl,_sh,_de,_dc,_dc,_lo,_dc,_ns,_oc,

	/*        p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~ DEL  */
	_pt,_dc,_dc,_st,_dc,_un,_dc,_dc,_he,_dc,_dc,_dc,_dc,_dc,_dc,_dc,
};


/*---------------------------------------------------------------------*

Name            __vprinter - sends formatted output

Usage           int   __vprinter (putnF  *putter,
                                         void   *outP,
                                         const _TCHAR *formP,
                                         int __use_nsize,
                                         size_t __nsize,
                                         va_list *argP)

Prototype in    _printf.h

Description     The list of arguments *argP is combined with literal text in
                the format string *formP according to format specifications
                inside the format string.

                The supplied procedure *putter is used to generate the output.
                It is required to take the string S, which has been
                constructed by __vprinter, and copy it to the destination
                outP.  The destination may be a string, a FILE, or whatever
                other class of construct the caller requires.  It is possible
                for several calls to be made to *putter since the buffer S
                is of limited size.

                *putter is required to preserve  SI, DI.

                The only purpose of the outP argument is to be passed through
                to putter.

                The object at *argP is a record of unknown structure, which
                structure is interpreted with the aid of the format string.
                Each field of the structure may be an integer, long, double,
                or string (char *).  Chars may appear but occupy integer-sized
                cells.  Floats, character arrays, and structures may not
                appear.

                __use_nsize is normally 0.  The (v)snprintf series of
                functions, will set __use_nsize to 1 and will also pass a
                valid maximum size value in the __nsize parameter.  This
                __nsize value represents the maximum length of the output
                buffer. The entire format string is processed and then the
                number of characters writen (via *outP) is returned except:

                  1. When the __nsize value is less than the number of chars
                     needed to be written.  In this case the extra
                     characters are counted and ignored (not written to the
                     output via *outP) and the number of characters that
                     COULD HAVE BEEN WRITTEN if the __nsize value had been
                     sufficiently large.

                  2. When __nsize is 0, then no characters are written, in
                     fact, outP can even be NULL.  The format string is
                     processed as normal and the number of characters needed
                     to store this string is calculated and returned.


Return value    The result of the function is a count of the characters sent to
                *outP (subject to the previously mentions restrictions and
                enhancements).

                There is no error indication.  When an incorrect conversion
                spec is encountered __vprinter copies the format as a literal
                (since it is assumed that alignment with the argument list has
                been lost), beginning with the '%' which introduced the bad
                format.

                If the destination outP is of limited size, for example a
                string or a full disk, __vprinter does not know.  Overflowing
                the destination causes undefined results.  In some cases
                *putter is able to handle overflows safely, but that is not
                the concern of __vprinter.

                The syntax of the format string is:

                format ::=      ([literal] ['%' conversion ])* ;

                conversion ::=  '%' | [flag]* [width] ['.' precision]
                                      ['l'] type ;

                flag ::=        '-' | '+' | ' ' | '#' | '0' ;

                width ::=       '*' | number ;
                precision ::=   '.' ('*' | number) ;

                type ::=        'd'|'i'|'o'|'u'|'x'|'n'|'X'|'f'|'e'|'E'|
                                'g'|'G'|'c'|'s'|'p'|'N'|'F'

*---------------------------------------------------------------------*/
int __vprintert(putnF        *putter,
                void         *outP,
                const _TCHAR *formP,
                int          __use_nsize,
                size_t       __nsize,
                va_list      argP)
{
	typedef
	enum
	{
		flagStage, fillzStage, wideStage, dotStage, precStage,
		ellStage, typeStage,
	} syntaxStages;
	typedef
	enum
	{
		altFormatBit = 0x0001, /* the '#' flag                 */
		leftJustBit  = 0x0002, /* the '-' flag                 */
		notZeroBit   = 0x0004, /* 0 (octal) or 0x (hex) prefix */
		fillZerosBit = 0x0008, /* zero fill width              */
		isLongBit    = 0x0010, /* long-type argument           */
		farPtrBit    = 0x0020, /* far pointers                 */
		alt0xBit     = 0x0040, /* '#' confirmed for %x format  */
		floatBit     = 0x0080, /* float arg 4 bytes not 8!     */
		LongDoubleBit= 0x0100, /* signal a long double argument*/
		isShortBit   = 0x0200, /* short-type argument          */
	} flagBits;
	flagBits flagSet;
	_TCHAR   fc;                    /* format char, from fmt string */
	_TCHAR   isSigned;              /* chooses signed/unsigned ints */
	int      width;
	int      precision;
	_TCHAR   plusSign;
	int      leadZ;
	_TCHAR   *abandonP;             /* posn of bad syntax in fmt str*/
	char     *PtrA;                 /* Other extra pointers */
	wchar_t  *PtrW;
	union
	{
		char     *PtrA;
		wchar_t  *PtrW;
	} DualPtr;
	union
	{
		char      StrA [__CVTMAX__];
		wchar_t   StrW [__CVTMAX__];
	} DualStr;
	union
	{
		char      StrA [MAX_BUF_LEN];
		wchar_t   StrW [MAX_BUF_LEN];
	} Buffer;
#define tempStrA  (DualStr.StrA)
#define tempStrW  (DualStr.StrW)
#define cPA       (DualPtr.PtrA)
#define cPW       (DualPtr.PtrW)
#ifdef _UNICODE
#  define tempStr tempStrW
#  define cP      (DualPtr.PtrW)
#else
#  define tempStr tempStrA
#  define cP      (DualPtr.PtrA)
#endif
	int      isWideBuffer;          /* use to denote width of 'DualStr' buffer */
	PutRec   put;                   /* characters sent to putter    */
	_TCHAR   hexCase;               /* upper/lower Hex alphabet     */
	unsigned __int64  temp64;       /* used to be long, now it's int64 */
	unsigned long     tempL;
	unsigned          tempI;
	unsigned short    tempS;
	syntaxStages      stage;        /* was CH */
	_TCHAR   c;
	int      radix;
	int      ndigits;
	int      len;
	/*
	General outline of the method:

	First the string is scanned, and conversion specifications detected.

	The preliminary fields of a conversion (flags, width, precision, long)
	are detected and noted.

	The argument is fetched and converted, under the optional guidance of
	the values of the preliminary fields.  With the sole exception of the
	's' conversion, the converted strings are first placed in the tempStr
	buffer.

	The contents of the tempStr (or the argument string for 's') are copied
	to the output, following the guidance of the preliminary fields in
	matters such as zero fill, field width, and justification.
	*/
	put.Scount = put.totalSent = put.eof_error = 0;
	put.putter = putter;
	put.outP   = outP;
	/* Set the pnsize ptr only if we've been called by snprintf */
	put.pnsize = (__use_nsize ? (&__nsize) : NULL);

	for (;;)
	{
NEXT:

		for (;;)
		{
			/* This code is arranged to give in-line flow to the most frequent
			 * case, literal transcription from *formP to *outP.
			 */
			if ((fc = *formP++) == _TEXT('\0'))
				goto respond;             /* end of format string */

			if (fc == _TEXT('%'))         /* '%' char begins a conversion */
			{
				if ((fc = *formP) == _TEXT('%'))
					formP++;              /* but "%%" is just a literal '%'. */
				else
					break;
			}

#if defined(_MBCS)

			if (_istleadbyte(fc) && *formP)
			{
				PutToS(fc,&put);          /* copy literal character */
				fc = *formP++;
			}

#endif
			PutToS(fc,&put);              /* copy literal character */
		}

		/* If arrived here then a conversion specification has been
		 * encountered.
		 */
		abandonP = (_TCHAR *)formP - 1;   /* abandon will print from '%' */
		stage = flagStage;
		leadZ = 0;
		plusSign = _TEXT('\0');
		flagSet = farPtrBit;
		width = precision = -1;
		isWideBuffer = DEFWIDTH;
		/*==================================*/
		/* loop to here when scanning flags */
		/*==================================*/

		for (;;)
		{
			fc = *formP++;              /* get next format character */

			if (fc < _TEXT(' ') ||      /* filter out controls */
			        (int)fc > (int)_TEXT('\x7F'))     /* or highs */
				goto abandon;

			/**************************************************************************
			 *                 Main character classification switch                   *
			 **************************************************************************/

			switch (printCtype[(bits08)(fc - _TEXT(' '))])
			{
				case(_af):                  /* when '#' was seen            */

					if (stage > flagStage)
						goto abandon;

					flagSet |= altFormatBit;
					continue;
				case(_lj):                  /* when '-' was seen            */

					if (stage > flagStage)
						goto abandon;

					flagSet |= leftJustBit;
					continue;
				case(_si):                  /* when ' ' or '+' was seen     */

					if (stage > flagStage)
						goto abandon;

					if (plusSign != _TEXT('+'))
						plusSign = fc;      /* ' ' ignored if '+' already   */

					continue;
				case(_ne):                  /* near pointer                 */
					flagSet &= ~farPtrBit;
					stage = ellStage;
					continue;
				case(_fa):                  /* far pointer                  */
					flagSet |= farPtrBit;
					stage = ellStage;
					continue;
				case(_fz):                  /* leading width '0' is a flag  */

					if (stage > flagStage)
						goto case_nu;       /*   else it is just a digit    */

					if ((flagSet & leftJustBit) == 0)
					{
						flagSet |= fillZerosBit;
						stage = fillzStage; /*   but it must be part of width */
					}

					continue;
				case(_ar):
					/* When '*' is seen it causes the next argument to be
					 * taken, depending on the stage, as the width or
					 * precision.
					 */
					tempI = va_arg(argP,int);

					if (stage < wideStage)
					{
						if ((int)tempI < 0)      /* is the width negative?       */
						{
							width = -(int)tempI;
							flagSet |= leftJustBit;
						}
						else
							width = tempI;

						stage = wideStage + 1;
					}
					else
					{
						if (stage != precStage)
							goto abandon;

						precision = tempI;
						stage++;
					}

					continue;
				case(_pr):                  /* when '.' is seen, precision  */

					if (stage >= precStage) /* should follow                */
						goto abandon;

					stage = precStage;
					precision++;            /* if no digits, ANSI says zero */
					continue;
					/*
					        When a numeral is seen, it may be either part of a width, or
					        part of the precision, depending on the stage.
					*/
				case(_nu):                      /* when 0..9 seen               */
case_nu:
					fc -= _TEXT('0');          /* turn '0'-'9' to 0-9          */

					if (stage <= wideStage)     /* is it part of a width spec?  */
					{
						stage = wideStage;

						if (width == -1)        /* first width digit ?          */
							width = (int)fc;    /*   default width was -1       */
						else
							width = (width * 10) + (int)fc;
					}
					else                        /* no, see if it's a precision  */
					{
						if (stage != precStage) /* is it part of precision spec */
							goto abandon;       /* no, it's just a literal      */

						/* At this point we know that the precision specifier
						 * '.' has been seen, so we know that the precision
						 * is zero (set at '.') or greater.
						 */
						precision = (precision * 10) + (int)fc;
					}

					continue;
				case(_lo):                      /* 'l' was seen (long)          */
					flagSet |= isLongBit;
					stage = ellStage;
					continue;
				case(_ld):                      /* 'L' was seen (long double or */
					/*  __int64)                    */
					flagSet = (flagSet | LongDoubleBit) & ~isLongBit;
					stage = ellStage;
					continue;
				case(_sh):                      /* 'h' or 'H' was seen (short   */
					/*  or narrow)                  */
					flagSet = (flagSet | isShortBit) & ~(isLongBit);
					stage = ellStage;
					continue;
				case(_wi):                      /* 'I' was seen (wide)           */

					if (formP[0] == _TEXT('6') && formP[1] == _TEXT('4'))
						/* handles the Microsoft I64 format */
					{
						formP += 2;
						flagSet = (flagSet | LongDoubleBit) & ~(isLongBit | isShortBit);
						stage = ellStage;
					}
					else if (formP[0] == _TEXT('3') && formP[1] == _TEXT('2'))
						/* handles the Microsoft I32 format */
					{
						formP += 2;
						flagSet = (flagSet | isLongBit) & ~(LongDoubleBit | isShortBit);
						stage = ellStage;
					}
					else if (formP[0] == _TEXT('1') && formP[1] == _TEXT('6'))
						/* handles the Microsoft I16 format */
					{
						formP += 2;
						flagSet = (flagSet | isShortBit) & ~(LongDoubleBit | isLongBit);
						stage = ellStage;
					}
					else if (formP[0] == _TEXT('8'))
						/* handles the Microsoft I8 format */
					{
						formP += 1;
						/* make it a regular int since that's how a char is passed */
						flagSet &= ~(LongDoubleBit | isLongBit | isShortBit);
						stage = ellStage;
					}

					continue;
					/*--------------------------------------------------------------------------
					The previous cases covered all the possible flags.  Now the following
					cases deal with the different argument types.

					Remember fc contains a copy of the original character.
					--------------------------------------------------------------------------*/
					/*==========================================================*/
					/* The first group of cases is for the integer conversions. */
					/*==========================================================*/
				case(_oc):                  /* octal                        */
					radix = 8;
					goto NoSign;
				case(_un):                  /* unsigned                     */
					radix = 10;
					goto NoSign;
				case(_he):                  /* hex                          */
					radix = 16;
					/* Adjust for aAbBcC etc later  */
					hexCase = fc - _TEXT('X') + _TEXT('A');
NoSign:
					plusSign = 0;           /* It's an unsigned operand     */
					isSigned = 0;
					goto toAscii;
				case(_de):                  /* decimal                      */
					radix = 10;
					isSigned = 1;
toAscii:

					if (flagSet & LongDoubleBit) /* context here means __int64 */
						temp64 = va_arg(argP,unsigned __int64);
					else if (flagSet & isLongBit)
					{
						tempL = va_arg(argP,unsigned long);

						if (isSigned)       /* check for sign extension     */
							temp64 = (unsigned __int64)(long)tempL;
						else
							temp64 = (unsigned __int64)tempL;
					}
					else if (flagSet & isShortBit)
					{
						tempS = va_arg(argP,short);

						if (isSigned)       /* check for sign extension     */
							temp64 = (unsigned __int64)(short)tempS;
						else
							temp64 = (unsigned __int64)tempS;
					}
					else
					{
						tempI = va_arg(argP,int);

						if (isSigned)       /* check for sign extension     */
							temp64 = (unsigned __int64)(int)tempI;
						else
							temp64 = (unsigned __int64)tempI;
					}

					cP = &tempStr[1];

					if (temp64 == 0i64)
					{
						/* ANSI special case where the value is zero and
						 * the precision is zero: don't print anything.
						 */
						if (precision == 0)
						{
							*cP = _TEXT('\0');
							goto converted;
						}
					}
					else
						flagSet |= notZeroBit;  /* flag non-zeroness           */

					/*-------------------------------------------------------------------------
					        "Normal" integer output cases wind up down here somewhere.
					-------------------------------------------------------------------------*/
					__int64tot(temp64, cP, radix, isSigned, hexCase);
converted:

					if (precision >= 0)
					{
						len = ndigits = _tcslen(cP);

						if (*cP == _TEXT('-'))  /* Is the number negative? */
							ndigits--;          /* decrement no. of digits */
						else if (plusSign != _TEXT('\0'))
						{                       /* It's positive and needs sign */
							len++;              /* Increase length of string */
							*--cP = plusSign;   /* Insert a '+' */
						}

						/* Calculate no. of leading zeros based on precision
						 * and the number of digits, NOT on the field width
						 * and the length of the converted number.
						 */
						if (precision > ndigits)
							leadZ = precision - ndigits;

						goto CopyLen;
					}
					else
						goto testFillZeros;

				case(_pt):                  /* pointer      */
					cP = va_arg(argP,_TCHAR *);
					Hex8((unsigned long)cP,tempStr);
					tempStr[8] = _TEXT('\0');
					/*                isSigned = 0;  */   /* removed because of warning */
					flagSet &= ~notZeroBit;
					cP = tempStr;
					goto testFillZeros;
				case(_Ch):                  /* char, opposite */

					if (!(flagSet & (isShortBit | isLongBit)))
						flagSet |=
#ifdef _UNICODE
						    isShortBit;

#else
						    isLongBit;
#endif
					/* fall through to _ch */
				case(_ch):                  /* char, normal  */
					/* The 'c' conversion takes a character as parameter.
					 * Note, however, that the character occupies an
					 * (int) sized cell in the argument list.
					 *
					 * Note: We must handle both narrow and wide versions
					 * depending on the flags specified and the version called:
					 *
					 * Format           printf          wprintf
					 * ----------------------------------------
					 * %c               narrow          wide
					 * %C               wide            narrow
					 * %hc              narrow          narrow
					 * %hC              narrow          narrow
					 * %lc              wide            wide
					 * %lC              wide            wide
					 *
					 *
					 */
#if defined (_UNICODE)

					if ((flagSet & isShortBit) != 0)
					{
						tempStrA[0] = (char)(va_arg(argP,int) & 0x00FF) ;
						tempStrA[1] = '\0';
						isWideBuffer = 0;
						cPA = tempStrA;
						len = 1;
					}

#else

					if ((flagSet & isLongBit) != 0)
					{
						tempStrW[0] = (wchar_t) va_arg(argP,int);
						tempStrW[1] = L'\0';
						isWideBuffer = 1;
						cPW = tempStrW;
						len = 1;
					}

#endif /* _UNICODE */
					else
					{
						/* Use default char size for normal operation */
						tempStr[0] = (_TCHAR)va_arg(argP,int);
						tempStr[1] = _TEXT('\0');
						cP = tempStr;
						isWideBuffer = DEFWIDTH;
						len = 1;
					}

					goto CopyLen;
				case(_St):                  /* string, opposite */

					if (!(flagSet & (isShortBit | isLongBit)))
						flagSet |=
#ifdef _UNICODE
						    isShortBit;

#else
						    isLongBit;
#endif
					/* fall through to _st */
				case(_st):                  /* string, normal   */
					/* The 's' conversion takes a string (char *) as
					 * argument and copies the string to the output
					 * buffer.
					 *
					 * Note: We must handle both narrow and wide versions
					 * depending on the flags specified and the version called:
					 *
					 * Format           printf          wprintf
					 * ----------------------------------------
					 * %s               narrow          wide
					 * %S               wide            narrow
					 * %hs              narrow          narrow
					 * %hS              narrow          narrow
					 * %ls              wide            wide
					 * %lS              wide            wide
					 *
					 */
#ifdef _UNICODE

					if ((flagSet & isShortBit) != 0)
					{
						cPA = va_arg(argP,char *);
						isWideBuffer = 0;

						if (cPA == NULL)
							cPA = NullStringA;
					}

#else

					if ((flagSet & isLongBit) != 0)
					{
						cPW = va_arg(argP,wchar_t *);
						isWideBuffer = 1;

						if (cPW == NULL)
							cPW = NullStringW;
					}

#endif /* _UNICODE */
					else
					{
						cP = va_arg(argP,_TCHAR *);
						isWideBuffer = DEFWIDTH;

						if (cP == NULL)
							cP = (_TCHAR *) DefNullString;
					}

					if (isWideBuffer)
					{
						int l;  /* temp var for holding the max len or prec. */

						if (precision >= 0)
							l = precision;
						else
							l = MAXINT;

						PtrW = cPW;
						len = 0;

						while (l && (*PtrW != L'\0'))
							l--, len++, PtrW++;
					}
					else
					{
						int l;  /* temp var for holding the max len or prec. */

						if (precision >= 0)
							l = precision;
						else
							l = MAXINT;

						PtrA = cPA;
						len = 0;

						while (l && (*PtrA != '\0'))
							l--, len++, PtrA++;
					}

					goto CopyLen;
				case(_fl):                  /* float        */
					/* All real-number conversions are done by __realcvt.
					 */
					__realcvt(
					    argP,
					    precision < 0 ? 6 : precision,  /* default prec. is 6 */
					    cP = &tempStr[1],
					    fc,
					    flagSet & altFormatBit,
					    flagSet & LongDoubleBit ? F_10byteFloat : F_8byteFloat);
#if 1
					/* Avoid generating reference to __fltused or __turboFloat
					 * by using external function to advance arg pointer.
					 */
					argP = __nextreal(argP,flagSet & LongDoubleBit);
#else

					if (flagSet & LongDoubleBit)
						va_arg(argP,long double);
					else
						va_arg(argP,double);

#endif
testFillZeros:

					if ((flagSet & fillZerosBit) && width > 0)
					{
						len = _tcslen(cP);

						if (*cP == _TEXT('-'))
							len--;          /* Length too long because '-'  */

						if (width > len)    /* leadZ defaulted to 0 before  */
							leadZ = width - len;
					}

					/*
					If arrived here, then tempStr contains the result of a numeric
					conversion.  It may be necessary to prefix the number with
					a mandatory sign or space.
					*/

					/* If we need a sign and it's not there already,
					 * back up 1 in the string and insert the sign.
					 * Adjust number of leading zeros down by one
					 * if precision not specified.
					 */
					if (*cP == _TEXT('-') || plusSign != _TEXT('\0'))
					{
						if (*cP != _TEXT('-'))
							*--cP = plusSign;

						if (leadZ > 0)
							leadZ--;
					}

					/*
					If arrived here then cP points to the converted string,
					which must now be padded, aligned, and copied to the output.
					*/
					len = _tcslen(cP);
CopyLen:                                /* comes from %c or %s section  */
#if 0           /* What the heck is this nonsence for? -- jjp 8/19/98 */

					if ((flagSet & (notZeroBit | altFormatBit))
					        == (notZeroBit | altFormatBit))
#else
					if (flagSet & altFormatBit)
#endif
					{

						if (fc == _TEXT('o')) /* Is it alternate octal form?  */
						{
							if (leadZ <= 0) /* Yes, alternate mode w/octal  */
								leadZ = 1;  /*     one leading zero.        */
						}
						else if (fc == _TEXT('x') || fc == _TEXT('X'))
						{
							/* Alternate hex form: send 0x or 0X prefix.
							 */
							flagSet |= alt0xBit;
							width -= 2;

							if ((leadZ -= 2) < 0)   /* Still leading 0's?   */
								leadZ = 0;          /* No more leading 0's  */
						}
					}
					len += leadZ;

					/* If result is NOT left justified, insert leading spaces.
					 */
					if ((flagSet & leftJustBit) == 0)
					{                               /* (! leftJust) == leftFill */
						while (width > len)
						{
							PutToS(_TEXT(' '),&put);
							width--;
						}
					}

					/* Need alternate hex form?
					 */
					if (flagSet & alt0xBit)
					{                       /* Yes, Send "0x" or "0X"       */
						PutToS(_TEXT('0'),&put);
						PutToS(fc,&put);    /* fc is 'x' or 'X'             */
					}

					/* Leading zero fill required?
					 */
					if (leadZ > 0)
					{
						len -= leadZ;
						width -= leadZ;

						/* Any leading sign must be copied before
						 * the leading zeroes.
						 */
						if (*cP == _TEXT('-') || *cP == _TEXT(' ') ||
						        *cP == _TEXT('+'))
						{
							PutToS(*cP++,&put);
							len--;
							width--;
						}

						while (leadZ--)
							PutToS(_TEXT('0'),&put);
					}

					/* Convert the string to the proper type if needed */
#ifdef _UNICODE

					if (!isWideBuffer)
					{
						/* Convert single chars into wide chars */
						char    *p = cPA;
						wchar_t wch;
						int widx = 0;
						int ret, count;
						count = len;

						while (count-- > 0)
						{
							ret = mbtowc(&wch, p, MB_LEN_MAX);

							if (ret <= 0)
								break;

							Buffer.StrW[widx++] = wch;
							p += ret;
						}

						cPW = Buffer.StrW;
						len = widx;
					}

#else

					if (isWideBuffer)
					{
						/* Convert wide chars into MBCS */
						wchar_t *p  = cPW;
						char     ch;
						int idx = 0;
						int ret, count, j;
						char mbBuf[MB_LEN_MAX];  /* maximum length for a MB char? */
						count = len;

						while (count-- > 0)
						{
							ret = wctomb(mbBuf, *p++);

							if (ret <= 0)
								break;

							for (j=0; j<ret; j++)
								Buffer.StrA[idx++] = mbBuf[j];
						}

						cPA = Buffer.StrA;
						len = idx;
					}

#endif

					/* Now we copy the actual converted string from tempStr
					 * to output.
					 */
					if (len != 0)
					{
						width -= len;

						while (len--)
							PutToS(*cP++,&put);
					}

					/* Is the field to be right-filled?
					 */
					while (width-- > 0)         /* while any remaining width */
						PutToS(_TEXT(' '),&put);

					/* If arrive here, the conversion has been done and copied
					 * to output.
					 */
					goto NEXT;
				case(_ns) :                  /* number sent */
					cP = va_arg(argP,_TCHAR *);

					if (flagSet & isLongBit)
						*((long *)cP) = put.totalSent;
					else if (flagSet & isShortBit)
						*((short *)cP) = put.totalSent;
					else
						*((int *)cP) = put.totalSent;

					goto NEXT;
				case(_zz):
				case(_dc):
				case(_pc):
					/* \0 characters, unexpected end of format string,
					 * ordinary "don't care" chars in the wrong position,
					 * '%' percent characters in the wrong position
					 */
					goto abandon;
			}               /* end switch */
		}
	}

	/* If the format goes badly wrong, then copy it literally to the output
	 * and abandon the conversion.
	 */
abandon:

	while ((c = *abandonP++) != _TEXT('\0'))
		PutToS(c,&put);

	/* If arrived here then the function has finished
	 * (either correctly or not).
	 */
respond:
	PutFlush(&put);

	if (put.eof_error)
		return (EOF);
	else
		return (put.totalSent);
}
