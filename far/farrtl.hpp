
#pragma once

/*
farrtl.cpp

Переопределение различных CRT функций
*/

#ifdef _DEBUG
#define MEMCHECK
#endif

#ifdef MEMCHECK
wchar_t* DuplicateString(const wchar_t* str, const char* Function, const char* File, int Line);

void* operator new(size_t size, const char* Function, const char* File, int Line);
void* operator new(size_t size, const std::nothrow_t& nothrow_value, const char* Function, const char* File, int Line) noexcept;
void* operator new[](size_t size, const char* Function, const char* File, int Line);
void* operator new[](size_t size, const std::nothrow_t& nothrow_value, const char* Function, const char* File, int Line) noexcept;
void operator delete(void* block, const char* Function, const char* File, int Line);
void operator delete[](void* block, const char* Function, const char* File, int Line);

#define DuplicateString(str) DuplicateString(str, __FUNCTION__, __FILE__, __LINE__)
#define new new(__FUNCTION__, __FILE__, __LINE__)
#else
wchar_t* DuplicateString(const wchar_t* string);
#endif

void PrintMemory();

char* xstrncpy(char* dest, const char* src, size_t DestSize);
wchar_t* xwcsncpy(wchar_t* dest, const wchar_t* src, size_t DestSize);

#define ALIGNAS(value, alignment) ((value+(alignment-1))&~(alignment-1))
#define ALIGN(value) ALIGNAS(value, sizeof(void*))
