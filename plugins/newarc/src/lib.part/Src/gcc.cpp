/*
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is a part of the mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within the package.
 *
 * Some changes by FarTeam
 *
 */

#include <Rtl.Base.h>
#include <errno.h>

typedef void (*func_ptr) (void);
extern func_ptr __CTOR_LIST__[];
extern func_ptr __DTOR_LIST__[];

static void _pei386_runtime_relocator (void);

typedef void (* p_atexit_fn )(void);
static p_atexit_fn* first_atexit;
static p_atexit_fn* next_atexit;

#ifdef __cplusplus
extern "C"{
#endif

  /* This  is based on the function in the Wine project's exit.c */
  p_atexit_fn __dllonexit (p_atexit_fn, p_atexit_fn**, p_atexit_fn**);

  BOOL WINAPI DllMainCRTStartup (HANDLE hDll, DWORD dwReason, LPVOID lpReserved);

#ifdef __cplusplus
};
#endif

/*
 * The atexit exported from msvcrt.dll causes problems in DLLs.
 * Here, we override the exported version of atexit with one that passes the
 * private table initialised in DllMainCRTStartup to __dllonexit.
 * That means we have to hide the mscvrt.dll atexit because the
 * atexit defined here gets __dllonexit from the same lib.
 */

int
atexit (p_atexit_fn pfn )
{
  return (__dllonexit (pfn,  &first_atexit, &next_atexit)
    == NULL ? -1  : 0 );
}

void
__do_global_dtors (void)
{
  static func_ptr *p = __DTOR_LIST__ + 1;

  /*
   * Call each destructor in the destructor list until a null pointer
   * is encountered.
   */
  while (*p)
    {
      (*(p)) ();
      p++;
    }
}

void
__do_global_ctors (void)
{
  unsigned long nptrs = (unsigned long) __CTOR_LIST__[0];
  unsigned i;

  /*
   * If the first entry in the constructor list is -1 then the list
   * is terminated with a null entry. Otherwise the first entry was
   * the number of pointers in the list.
   */
  if (nptrs == (unsigned long)-1)
    {
      for (nptrs = 0; __CTOR_LIST__[nptrs + 1] != 0; nptrs++)
        ;
    }

  /*
   * Go through the list backwards calling constructors.
   */
  for (i = nptrs; i >= 1; i--)
    {
      __CTOR_LIST__[i] ();
    }

  /*
   * Register the destructors for processing on exit.
   */
  atexit (__do_global_dtors);
}

static int initialized = 0;

static void
__main (void)
{
  if (!initialized)
    {
      initialized = 1;
      __do_global_ctors ();
    }
}

static
void
__dll_exit(void)
/* Run LIFO terminators registered in private atexit table */
{
  if ( first_atexit )
    {
      p_atexit_fn* __last = next_atexit - 1;
      while ( __last >= first_atexit )
        {
          if ( *__last != NULL )
            {
              (**__last) ();
          }
          __last--;
      }
      free ( first_atexit ) ;
      first_atexit = NULL ;
    }
    /*
       Make sure output buffers opened by DllMain or
       atexit-registered functions are flushed before detaching,
       otherwise we can have problems with redirected output.
     */
    //fflush (NULL);
}

BOOL WINAPI
DllMainCRTStartup (HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
  if (dwReason == DLL_PROCESS_ATTACH)
    {

      /* Initialize private atexit table for this dll.
   32 is min size required by ANSI */

      first_atexit = (p_atexit_fn*) malloc (32 * sizeof (p_atexit_fn));
      if (first_atexit == NULL ) /* can't allocate memory */
        {
          errno=ENOMEM;
          return FALSE;
      }
      *first_atexit =  NULL;
      next_atexit = first_atexit;

      /* Adust references to dllimported data (from other DLL's)
   that have non-zero offsets.  */
      _pei386_runtime_relocator ();

      /* From libgcc.a, __main calls global class constructors,
   __do_global_ctors, which registers __do_global_dtors
   as the first entry of the private atexit table we
   have just initialised  */
      __main ();

   }

  /*
   * Call the user-supplied DllMain subroutine.
   * This has to come after initialization of atexit table and
   * registration of global constructors.
   * NOTE: DllMain is optional, so libmingw32.a includes a stub
   *       which will be used if the user does not supply one.
   */

  if (dwReason == DLL_PROCESS_DETACH)
    {
      /* If not attached, return FALSE. Cleanup already done above
         if failed attachment attempt. */
      if  (! first_atexit )
        return FALSE;
      else
  /*
   * We used to call __do_global_dtors () here. This is
   * no longer necessary since  __do_global_dtors is now
   * registered at start (last out) of private atexit table.
   */
        __dll_exit ();
   }
  return TRUE;
}


/* pseudo-reloc.c

   Written by Egor Duda <deo@logos-m.ru>
   THIS SOFTWARE IS NOT COPYRIGHTED

   This source code is offered for use in the public domain. You may
   use, modify or distribute it freely.

   This code is distributed in the hope that it will be useful but
   WITHOUT ANY WARRANTY. ALL WARRENTIES, EXPRESS OR IMPLIED ARE HEREBY
   DISCLAMED. This includes but is not limited to warrenties of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

extern char __RUNTIME_PSEUDO_RELOC_LIST__;
extern char __RUNTIME_PSEUDO_RELOC_LIST_END__;
extern char _image_base__;

typedef struct
  {
    DWORD addend;
    DWORD target;
  }
runtime_pseudo_reloc;

static void
do_pseudo_reloc (void* start, void* end, void* base)
{
  DWORD reloc_target;
  runtime_pseudo_reloc* r;
  for (r = (runtime_pseudo_reloc*) start; r < (runtime_pseudo_reloc*) end; r++)
    {
      reloc_target = (DWORD) base + r->target;
      *((DWORD*) reloc_target) += r->addend;
    }
}

static void
_pei386_runtime_relocator ()
{
  do_pseudo_reloc (&__RUNTIME_PSEUDO_RELOC_LIST__,
       &__RUNTIME_PSEUDO_RELOC_LIST_END__,
       &_image_base__);
}
