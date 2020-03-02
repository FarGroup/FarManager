#include <all_far.h>
#pragma hdrstop

#include "Int.h"

#define LLog(v) if ( FP_Info ) Log( v )

#ifdef _DEBUG

#if defined(__NOMEM__)
void ShowMemInfo(void)
{}

#else

long MemCount = 0;
long MemUsage = 0;

void ShowMemInfo(void)
{
	static BOOL into = FALSE;

	if(into) return;

	into = TRUE;
	CONSOLE_SCREEN_BUFFER_INFO ci;
	char                       str[100];
	HANDLE                     h = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD                      dw;
	sprintf(str,"%6ld in %-6ld ",MemUsage,MemCount);
	ci.dwSize.X = FP_ConWidth() - strlen(str);
	ci.dwSize.Y = 0;
	WriteConsoleOutputCharacter(h,str,strlen(str),ci.dwSize,&dw);
	into = FALSE;
}

#ifdef __cplusplus
namespace std
{
#endif

	void * _RTLENTRY _EXPFUNC calloc(size_t nitems, size_t size)
	{
		return _Alloc(nitems*size);
	}

	void * _RTLENTRY _EXPFUNC _Alloc(size_t size)
	{
		if(size == 0) size = 1;

		void *ptr = (void*)GlobalAlloc(GPTR,(DWORD)size);

		if(!ptr)
		{
			LLog(("!_Alloc %d",size));
			return NULL;
		}
		else
			LLog(("_Alloc(%d) = %p",size,ptr));

		MemCount++;
		MemUsage += size;
		ShowMemInfo();
		return ptr;
	}

	void _RTLENTRY _EXPFUNC free(void *ptr)
	{
		size_t sz;

		if(!ptr || (sz=GlobalSize((HGLOBAL)ptr)) == 0)
		{
			if(ptr)
				LLog(("!allocated block %p",ptr));

			return;
		}

		if(GlobalFree((HGLOBAL)ptr) == NULL)
		{
			LLog(("free(%p)",ptr));
			MemCount--;
			MemUsage -= sz;
			ShowMemInfo();
		}
		else
			LLog(("!free block %p[%d]",ptr,sz));
	}

	void *_RTLENTRY _EXPFUNC realloc(void *ptr,size_t size)
	{
		size_t sz;
		void *ptrnew;

		if(!ptr)
			sz = 0;
		else if((sz=GlobalSize((HGLOBAL)ptr)) == 0)
		{
			if(ptr)
				LLog(("!allocated block %p -> %d",ptr,size));

			return NULL;
		}

		if((ptrnew=(void*)GlobalAlloc(GPTR,size)) != NULL)
		{
			LLog(("realloc(%p,%d) = %p",ptr,size,ptrnew));
			MemUsage = MemUsage - sz + size;

			if(!sz)
				MemCount++;

			if(ptr)
			{
				memmove(ptrnew,ptr,sz);
				GlobalFree(ptr);
			}

			ShowMemInfo();
		}
		else
			LLog(("!realloc %p[%d] -> %p[%d] [%s]",ptr,sz,ptrnew,size,__WINError()));

		return ptrnew;
	}
#ifdef __cplusplus
} //std
#endif

#endif //BC
#endif
