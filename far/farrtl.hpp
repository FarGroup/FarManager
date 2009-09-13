#pragma once

/*
farrtl.cpp

Переопределение различных CRT функций
*/

#ifdef __cplusplus
extern "C" {
#endif

int _cdecl getdisk();
void __cdecl qsortex(char *base, size_t nel, size_t width,
            int (__cdecl *comp_fp)(const void *, const void *,void*), void *user);

char * __cdecl xstrncat (char * dest,const char * src,size_t maxlen);
wchar_t * __cdecl xwcsncat (wchar_t * dest,const wchar_t * src,size_t maxlen);
char * __cdecl xstrncpy (char * dest,const char * src,size_t maxlen);
wchar_t * __cdecl xwcsncpy (wchar_t * dest,const wchar_t * src,size_t maxlen);
char * __cdecl xf_strdup (const char * string);
wchar_t * __cdecl xf_wcsdup (const wchar_t * string);
void __cdecl far_qsort (
    void *base,
    size_t num,
    size_t width,
    int (__cdecl *comp)(const void *, const void *)
    );

void  __cdecl xf_free(void *__block);
void *__cdecl xf_malloc(size_t __size);
void *__cdecl xf_realloc_nomove(void *__block, size_t __size);
void *__cdecl xf_realloc(void *__block, size_t __size);

#ifdef __cplusplus
}
#endif

long filelen(FILE *FPtr);
__int64 filelen64(FILE *FPtr);
__int64 ftell64(FILE *fp);
int fseek64 (FILE *fp, __int64 offset, int whence);
