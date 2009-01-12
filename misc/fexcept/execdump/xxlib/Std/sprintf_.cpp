#include <all_lib.h>
#pragma hdrstop

#include <limits.h>

#define ZEROPAD 1       /* pad with zero */
#define SIGN    2       /* unsigned/signed long */
#define PLUS    4       /* show plus */
#define SPACE   8       /* space if plus */
#define LEFT    16      /* left justified */
#define SPECIAL 32      /* 0x */
#define LARGE   64      /* use 'ABCDEF' instead of 'abcdef' */

#if defined(__HDOS__) || defined(__QNX__)
  #define MAXSIZE_INT long
#else
#if defined(__BORLAND) || defined(__MSOFT) || defined(__INTEL)
  #define MAXSIZE_INT __int64
#else
#if defined(__SYMANTEC) || defined(__DMC)
  #define MAXSIZE_INT long long
#else
#if defined(__GNUC__)
  #define MAXSIZE_INT long long
#else
  #error "Define maximum integer value"
#endif  //dos, qnx
#endif  //bcc, msc
#endif  //sc
#endif  //gcc

#if defined(__WINUNICODE__)
  #define HAS_UNICODE 1
  typedef wchar_t *PUNICODE_STRING;
  typedef char *PANSI_STRING;
#else
  typedef char *PANSI_STRING;
#endif

#define STR( v ) do{                                  \
                   if ( cnt ) {                       \
                     buf[nstr++] = v;                 \
                     if ( nstr >= cnt ) goto Done;    \
                   } else                             \
                     nstr++;                          \
                 }while(0)

static int skip_atoi(const char **s)
{
    int i=0;

    while (isdigit(**s))
        i = i*10 + *((*s)++) - '0';
    return i;
}


static size_t number( char *buf, size_t nstr, size_t cnt, MAXSIZE_INT num, int base, int size, int precision, int type)
{
    char c,sign,tmp[66];
    const char *digits="0123456789abcdefghijklmnopqrstuvwxyz";
    int i;

    if (type & LARGE)
        digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (type & LEFT)
        type &= ~ZEROPAD;
    if (base < 2 || base > 36)
        return 0;

    c = (type & ZEROPAD) ? '0' : ' ';
    sign = 0;

    if (type & SIGN) {
        if (num < 0) {
            sign = '-';
            num = -num;
            size--;
        } else if (type & PLUS) {
            sign = '+';
            size--;
        } else if (type & SPACE) {
            sign = ' ';
            size--;
        }
    }

    if (type & SPECIAL) {
        if (base == 16)
            size -= 2;
        else if (base == 8)
            size--;
    }

    i = 0;
    if (num == 0)
      tmp[i++]='0';
    else
      while (num != 0) {
        unsigned __res = (unsigned) (((unsigned MAXSIZE_INT) num) % ((unsigned) base));
        num = (unsigned) (((unsigned MAXSIZE_INT) num) / ((unsigned) base));
        tmp[i++] = digits[ __res ];
      }

    if (i > precision)
        precision = i;
    size -= precision;
    if (!(type&(ZEROPAD+LEFT)))
        while(size-->0)
            STR( ' ' );
    if (sign)
        STR( sign );
    if (type & SPECIAL) {
        if (base==8) {
            STR( '0' );
        } else if (base==16) {
            STR( '0' );
            STR( digits[33] );
        }
    }
    if (!(type & LEFT))
      while (size-- > 0)
        STR( c );

    while (i < precision--)
        STR( '0' );

    while (i-- > 0)
        STR( tmp[i] );

    while (size-- > 0)
        STR( ' ' );
 Done:
 return nstr;
}

int MYRTLEXP _VSNprintf( char *buf, size_t cnt, const char *fmt, va_list args )
  { int len;
    unsigned MAXSIZE_INT num;
    int i, base;
    size_t nstr;
    const char *s;
#if defined(HAS_UNICODE)
    const wchar_t *sw;
#endif

    int flags;      /* flags to number() */

    int field_width;    /* width of output field */
    int precision;      /* min. # of digits for integers; max
                   number of chars for from string */
    int qualifier;      /* 'h', 'l', 'L', 'I' or 'w' for integer fields */

    if ( !buf && cnt )
      return 0;

    if ( buf && cnt == 1 ) {
      buf[0] = 0;
      return 1;
    }

    for (nstr=0; *fmt ; ++fmt) {
        if (*fmt != '%') {
          STR( *fmt );
            continue;
        }

        /* process flags */
        flags = 0;
        repeat:
            ++fmt;      /* this also skips first '%' */
            switch (*fmt) {
                case '-': flags |= LEFT; goto repeat;
                case '+': flags |= PLUS; goto repeat;
                case ' ': flags |= SPACE; goto repeat;
                case '#': flags |= SPECIAL; goto repeat;
                case '0': flags |= ZEROPAD; goto repeat;
            }

        /* get field width */
        field_width = -1;
        if (isdigit(*fmt))
            field_width = skip_atoi(&fmt);
        else if (*fmt == '*') {
            ++fmt;
            /* it's the next argument */
            field_width = va_arg(args, int);
            if (field_width < 0) {
                field_width = -field_width;
                flags |= LEFT;
            }
        }

        /* get the precision */
        precision = -1;
        if (*fmt == '.') {
            ++fmt;
            if (isdigit(*fmt))
                precision = skip_atoi(&fmt);
            else if (*fmt == '*') {
                ++fmt;
                /* it's the next argument */
                precision = va_arg(args, int);
            }
            if (precision < 0)
                precision = 0;
        }

        /* get the conversion qualifier */
        qualifier = -1;
        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt == 'w') {
            qualifier = *fmt;
            ++fmt;
        } else if (*fmt == 'I' && *(fmt+1) == '6' && *(fmt+2) == '4') {
            qualifier = *fmt;
            fmt += 3;
        }

        /* default base */
        base = 10;

        switch (*fmt) {
        case 'c': /* finished */
            if (!(flags & LEFT))
                while (--field_width > 0)
                    STR( ' ' );
#if defined(HAS_UNICODE)
            if (qualifier == 'l' || qualifier == 'w')
              STR( (unsigned char)(wchar_t) va_arg(args, int) );
             else
#endif
              STR( (unsigned char) va_arg(args, int) );
            while (--field_width > 0)
              STR( ' ' );
            continue;

        case 'C': /* finished */
            if (!(flags & LEFT))
                while (--field_width > 0)
                    STR( ' ' );
#if defined(HAS_UNICODE)
            if (qualifier != 'h')
              STR( (unsigned char)(wchar_t) va_arg(args, int) );
            else
#endif
              STR( (unsigned char) va_arg(args, int) );
            while (--field_width > 0)
                STR( ' ' );
            continue;

        case 's': /* finished */
#if defined(HAS_UNICODE)
            if (qualifier == 'l' || qualifier == 'w') {
                /* print unicode string */
                sw = va_arg(args, wchar_t *);
                if (sw == NULL)
                    sw = L"<NULL>";

                for (len = 0; (unsigned int)len < (unsigned int)precision && sw[len]; len++);

                if (!(flags & LEFT))
                    while (len < field_width--)
                        STR( ' ' );
                for (i = 0; i < len; ++i)
                    STR( (unsigned char)(*sw++) );
                while (len < field_width--)
                    STR( ' ' );
            } else
#endif
            {
                /* print ascii string */
                s = va_arg(args, char *);
                if (s == NULL)
                    s = "<NULL>";

                for (len = 0; (unsigned int)len < (unsigned int)precision && s[len]; len++);

                if (!(flags & LEFT))
                    while (len < field_width--)
                        STR( ' ' );
                for (i = 0; i < len; ++i)
                    STR( *s++ );
                while (len < field_width--)
                    STR( ' ' );
            }
            continue;

        case 'S':
#if defined(HAS_UNICODE)
            if (qualifier != 'h') {
                /* print unicode string */
                sw = va_arg(args, wchar_t *);
                if (sw == NULL)
                    sw = L"<NULL>";

                for (len = 0; (unsigned int)len < (unsigned int)precision && sw[len]; len++);

                if (!(flags & LEFT))
                    while (len < field_width--)
                        STR( ' ' );
                for (i = 0; i < len; ++i)
                    STR( (unsigned char)(*sw++) );
                while (len < field_width--)
                    STR( ' ' );
            } else
#endif
            {
                /* print ascii string */
                s = va_arg(args, char *);
                if (s == NULL)
                    s = "<NULL>";

                for (len = 0; (unsigned int)len < (unsigned int)precision && s[len]; len++);

                if (!(flags & LEFT))
                    while (len < field_width--)
                        STR( ' ' );
                for (i = 0; i < len; ++i)
                    STR( *s++ );
                while (len < field_width--)
                    STR( ' ' );
            }
            continue;

        case 'p':
            if (field_width == -1) {
                field_width = 2 * sizeof(void *);
                flags |= ZEROPAD;
            }
            nstr = number( buf, nstr, cnt, (unsigned long)va_arg(args, void *), 16, field_width, precision, flags );
            continue;

        case 'n':
            if (qualifier == 'l') {
                long * ip = va_arg(args, long *);
                *ip = (long)nstr;
            } else {
                int * ip = va_arg(args, int *);
                *ip = (int)nstr;
            }
            continue;

        /* integer number formats - set up the flags and "break" */
        case 'o':
            base = 8;
            break;

        case 'b':
            base = 2;
            break;

        case 'X':
            flags |= LARGE;
        case 'x':
            base = 16;
            break;

        case 'd':
        case 'i':
            flags |= SIGN;
        case 'u':
            break;

        default:
            if (*fmt != '%')
                STR( '%' );
            if (*fmt)
                STR( *fmt );
            else
                --fmt;
            continue;
        }

        if (qualifier == 'I') num = va_arg(args, unsigned MAXSIZE_INT);  else
        if (qualifier == 'l') num = va_arg(args, unsigned long);       else
        if (qualifier == 'h') {
          if (flags & SIGN)
            num = va_arg(args, int);
          else
            num = va_arg(args, unsigned int);
        } else {
          if (flags & SIGN)
            num = va_arg(args, int);
          else
            num = va_arg(args, unsigned int);
        }
        nstr = number( buf, nstr, cnt, num, base, field_width, precision, flags);
    }

 Done:
    if ( buf )
      buf[nstr] = '\0';

 return (int)nstr;
}

#if defined(__UNICODE__)
int MYRTLEXP _VSNwprintf( wchar_t *buf, size_t cnt, const wchar_t *fmt, va_list args )
  {
 return 0;
}
#endif

#if 0
static int _SNprintf(char * buf, size_t cnt, const char *fmt, ...)
  { va_list args;
    int i;

    va_start(args, fmt);
    i=_VSNprintf(buf,cnt,fmt,args);
    va_end(args);
    return i;
}

void main( void )
  {
    printf( "%d,%d,%d\n",sizeof(float), sizeof(double), sizeof(long double) );

    printf( "%d\n", _SNprintf( NULL,0,"%s%d", "text", 101 ) );

    char str[ 100 ];
    printf( "%d: ", _SNprintf( str,10,"%s%d", "text", 101 ) ); printf( "%s\n", str );
    printf( "%d: ", _SNprintf( str,6,"%s%d", "text", 101 ) ); printf( "%s\n", str );
    printf( "%d: ", _SNprintf( str,1,"%s%d", "text", 101 ) ); printf( "%s\n", str );
    printf( "%d: ", _SNprintf( str,0,"%s%d", "text", 101 ) ); printf( "%s\n", str );
}
#endif
