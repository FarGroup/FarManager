#include "headers.hpp"
#pragma hdrstop

#if defined(__GNUC__) && __GNUC__ < 4
 #warning "Consider using newer GCC version for this hook to work"
#else

#if ((!defined(_MSC_VER) || _MSC_VER < 1300) && !defined(__GNUC__)) || defined(_WIN64)
#error
#endif
/*
 * If compiled with VC7 and linking with ulink, please specify
 * -DLINK_WITH_ULINK
*/

#if defined(_MSC_VER)
#pragma optimize("gty", on)
#endif

//-----------------------------------------------------------------------------
#include <windows.h>

static
#if defined(__GNUC__)
__thread
#endif
PVOID
#if !defined(__GNUC__)
__declspec(thread)
#endif
saveval;

typedef struct {
  BOOL (WINAPI *disable)(PVOID*);
  BOOL (WINAPI *revert)(PVOID);
}WOW;

static BOOL WINAPI e_disable(PVOID* p) { (void)p; return FALSE; }
static BOOL WINAPI e_revert(PVOID p) { (void)p; return FALSE; }

volatile const WOW wow = { e_disable, e_revert };

//-----------------------------------------------------------------------------
void wow_restore(void)
{
    wow.revert(saveval);
}

PVOID wow_disable(void)
{
    PVOID p;
    wow.disable(&p);
    return p;
}

static void wow_disable_and_save(void)
{
    saveval = wow_disable();
}

//-----------------------------------------------------------------------------
static void init_hook(void);
static void WINAPI HookProc(PVOID h, DWORD dwReason, PVOID u)
{
    (void)h;
    (void)u;
    switch(dwReason) {
      case DLL_PROCESS_ATTACH:
        init_hook();
      case DLL_THREAD_ATTACH:
        wow_disable_and_save();
      default:
        break;
    }
}

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
 #if _MSC_VER < 1400 && !defined(LINK_WITH_ULINK)
 #pragma message("VC8 or higher is strongly recommended")
 #pragma data_seg(".CRT$XLY")
 #else
 #pragma const_seg(".CRT$XLY")
 const
 #endif
      PIMAGE_TLS_CALLBACK hook_wow64_tlscb = HookProc;
 #pragma const_seg()
#else
 ULONG __tls_index__ = 0;
 char __tls_end__ __attribute__((section(".tls$zzz"))) = 0;
 char __tls_start__ __attribute__((section(".tls"))) = 0;

 PIMAGE_TLS_CALLBACK __crt_xl_start__ __attribute__ ((section(".CRT$XLA"))) = 0;
 PIMAGE_TLS_CALLBACK hook_wow64_tlscb __attribute__ ((section(".CRT$XLY"))) = HookProc;
 PIMAGE_TLS_CALLBACK __crt_xl_end__ __attribute__ ((section(".CRT$XLZ"))) = 0;

 const IMAGE_TLS_DIRECTORY32 _tls_used __attribute__ ((section(".rdata$T"))) =
 {
   (DWORD) &__tls_start__,
   (DWORD) &__tls_end__,
   (DWORD) &__tls_index__,
   (DWORD) (&__crt_xl_start__+1),
   (DWORD) 0,
   (DWORD) 0
 };
#endif

#ifdef __cplusplus
}
#endif
// for ulink
#pragma comment(linker, "/include:_hook_wow64_tlscb")

//-----------------------------------------------------------------------------
static void
#if defined(__GNUC__)
// Unfortunatly GCC doesn't support this attribute on x86
//__attribute__((naked))
#else
__declspec(naked)
#endif
hook_ldr(void)
{
#if !defined(__GNUC__)
  __asm {
        call    wow_restore
        pop     edx             // real call
        pop     eax             // real return
        pop     ecx             // arg1
        xchg    eax, [esp+8]    // real return <=> arg4
        xchg    eax, [esp+4]    // arg4 <=> arg3
        xchg    eax, [esp]      // arg3 <=> arg2
        push    eax             // arg2
        push    ecx             // arg1
        call    _l1
        push    eax     // answer
        call    wow_disable
        pop     eax
        retn
//-----
_l1:    push    240h
        jmp     edx
  }
#else
  __asm__ ("call    _wow_restore \n\t"
           "popl    %edx         \n\t" // real call
           "popl    %eax         \n\t" // real return
           "popl    %ecx         \n\t" // arg1
           "xchg    8(%esp),%eax \n\t" // real return <=> arg4
           "xchg    4(%esp),%eax \n\t" // arg4 <=> arg3
           "xchg    (%esp),%eax  \n\t" // arg3 <=> arg2
           "pushl   %eax         \n\t" // arg2
           "pushl   %ecx         \n\t" // arg1
           "call    _l1          \n\t"
           "pushl   %eax         \n\t" // answer
           "call    _wow_disable \n\t"
           "popl    %eax         \n\t"
           "ret                  \n\t"
       "_l1:                     \n\t"
           "pushl   $0x240       \n\t"
           "jmp     *%edx");
#endif
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void init_hook(void)
{
   DWORD p;
   static const wchar_t k32_w[] = L"kernel32", ntd_w[] = L"ntdll";
   static const char dis_c[] = "Wow64DisableWow64FsRedirection",
                     rev_c[] = "Wow64RevertWow64FsRedirection",
                     ldr_c[] = "LdrLoadDll";

    WOW rwow;
#pragma pack(1)
    struct {
      BYTE  cod;
      DWORD off;
    }data = { 0xE8, (DWORD)(SIZE_T)((LPCH)hook_ldr - sizeof(data)) };
#pragma pack()

    register union {
      HMODULE h;
      FARPROC f;
      LPVOID  p;
      DWORD   d;
    }ur;

    if(   (ur.h = GetModuleHandleW(k32_w)) == NULL
       || (*(FARPROC*)&rwow.disable = GetProcAddress(ur.h, dis_c)) == NULL
       || (*(FARPROC*)&rwow.revert = GetProcAddress(ur.h, rev_c)) == NULL
       || (ur.h = GetModuleHandleW(ntd_w)) == NULL
       || (ur.f = GetProcAddress(ur.h, ldr_c)) == NULL
       || *(LPDWORD)ur.p != 0x24068 // push 240h
       || ((LPBYTE)ur.p)[sizeof(DWORD)]) return;

    data.off -= ur.d;
    if(   !WriteProcessMemory(GetCurrentProcess(), ur.p, &data, sizeof(data), &data.off)
       || data.off != sizeof(data)) return;

    // don't use WriteProcessMemory here - BUG in 2003x64 32bit kernel32.dll :(
    if(!VirtualProtect((void*)&wow, sizeof(wow), PAGE_READWRITE, &data.off))
      return;
    *(WOW*)&wow = rwow;
    VirtualProtect((void*)&wow, sizeof(wow), data.off, &p);
}

#endif
