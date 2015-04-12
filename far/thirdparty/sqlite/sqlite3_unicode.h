#ifndef _SQLITE3_UNICODE_H
#define _SQLITE3_UNICODE_H

#ifdef __cplusplus
extern "C" {
#endif

/*
** Add the ability to override 'extern'
*/
/*
** <sqlite3_unicode>
** The define of SQLITE_EXPORT is necessary to add the ability of exporting
** functions for both Microsoft Windows and Linux systems without the need
** of a .def file containing the names of the functions being exported.
*/
#ifndef SQLITE_EXPORT
# if ((defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)) && (!defined(SQLITE_CORE)))
#  define SQLITE_EXPORT __declspec(dllexport)
# else
#  define SQLITE_EXPORT SQLITE_EXTERN
# endif
#endif

#ifndef SQLITE_PRIVATE
# define SQLITE_PRIVATE static
#endif
#ifndef SQLITE_API
# define SQLITE_API
#endif

/*
** Integers of known sizes.  These typedefs might change for architectures
** where the sizes very.  Preprocessor macros are available so that the
** types can be conveniently redefined at compile-type.  Like this:
**
**         cc '-DUINTPTR_TYPE=long long int' ...
*/
#ifndef UINT32_TYPE
# ifdef HAVE_UINT32_T
#  define UINT32_TYPE uint32_t
# else
#  define UINT32_TYPE unsigned int
# endif
#endif
#ifndef UINT16_TYPE
# ifdef HAVE_UINT16_T
#  define UINT16_TYPE uint16_t
# else
#  define UINT16_TYPE unsigned short int
# endif
#endif
#ifndef INT16_TYPE
# ifdef HAVE_INT16_T
#  define INT16_TYPE int16_t
# else
#  define INT16_TYPE short int
# endif
#endif
#ifndef UINT8_TYPE
# ifdef HAVE_UINT8_T
#  define UINT8_TYPE uint8_t
# else
#  define UINT8_TYPE unsigned char
# endif
#endif
#ifndef INT8_TYPE
# ifdef HAVE_INT8_T
#  define INT8_TYPE int8_t
# else
#  define INT8_TYPE signed char
# endif
#endif
#ifndef LONGDOUBLE_TYPE
# define LONGDOUBLE_TYPE long double
#endif
typedef sqlite_int64 i64;          /* 8-byte signed integer */
typedef sqlite_uint64 u64;         /* 8-byte unsigned integer */
typedef UINT32_TYPE u32;           /* 4-byte unsigned integer */
typedef UINT16_TYPE u16;           /* 2-byte unsigned integer */
typedef INT16_TYPE i16;            /* 2-byte signed integer */
typedef UINT8_TYPE u8;             /* 1-byte unsigned integer */
typedef INT8_TYPE i8;              /* 1-byte signed integer */

/*
** <sqlite3_unicode>
** These functions are intended for case conversion of single characters
** and return a single character containing the case converted character
** based on the unicode mapping tables.
*/
SQLITE_EXPORT u16 sqlite3_unicode_fold(u16 c);
SQLITE_EXPORT u16 sqlite3_unicode_lower(u16 c);
SQLITE_EXPORT u16 sqlite3_unicode_upper(u16 c);
SQLITE_EXPORT u16 sqlite3_unicode_title(u16 c);

/*
** <sqlite3_unicode>
** This function is intended for decomposing of single characters
** and return a pointer of characters (u16 **)p containing the decomposed
** character or string of characters. (int *)l will contain the length
** of characters contained in (u16 **)p based on the unicode mapping tables.
*/
SQLITE_EXPORT u16 sqlite3_unicode_unacc(u16 c, u16 **p, int *l);

/*
** Another built-in collating sequence: NOCASE. 
**
** This collating sequence is intended to be used for "case independant
** comparison". SQLite's knowledge of upper and lower case equivalents
** extends only to the 26 characters used in the English language.
**
** At the moment there is only a UTF-8 implementation.
*/
/*
** <sqlite3_unicode>
** The built-in collating sequence: NOCASE is extended to accomodate the
** unicode case folding mapping tables to normalize characters to their
** fold equivalents and test them for equality.
**
** Both UTF-8 and UTF-16 implementations are supported.
** 
** (void *)encoding takes the following values
**   * SQLITE_UTF8  for UTF-8  encoded string comparison
**   * SQLITE_UFT16 for UTF-16 encoded string comparison
*/
SQLITE_EXPORT int sqlite3_unicode_collate(
  void *encoding,
  int nKey1, const void *pKey1,
  int nKey2, const void *pKey2
  );

/*
** <sqlite3_unicode>
** Register the UNICODE extension functions with database db.
*/
SQLITE_EXPORT int sqlite3_unicode_init(sqlite3 *db);

/*
** <sqlite3_unicode>
** The following function needs to be called at application startup to load the extension.
*/
SQLITE_EXPORT int sqlite3_unicode_load();

/*
** <sqlite3_unicode>
** The following function needs to be called before application exit to unload the extension.
*/
SQLITE_EXPORT void sqlite3_unicode_free();

#ifdef __cplusplus
}
#endif

#endif /* _SQLITE3_UNICODE_H */