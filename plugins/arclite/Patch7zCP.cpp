#include "utils.hpp"
#include "sysutils.hpp"

//#############################################################################
//#############################################################################

static UINT orig_oemCP = 0, orig_ansiCP = 0;
static UINT over_oemCP = 0, over_ansiCP = 0;
static bool patched_7z_dll = false;

//#############################################################################

struct patched
{
  static UINT WINAPI GetOEMCP(void)
  {
    return over_oemCP;
  }

  static UINT WINAPI GetACP(void)
  {
    return over_ansiCP;
  }

  static int WINAPI MultiByteToWideChar(
    UINT cp,
    DWORD flags,
    LPCCH lpMultiByte,
    int cbMultiByte,
    LPWSTR lpWide,
    int cchWide
  ){
    if (cp == CP_OEMCP)
      cp = over_oemCP;
    else if (cp == CP_ACP || cp == CP_THREAD_ACP)
      cp = over_ansiCP;

    return ::MultiByteToWideChar(
      cp, flags, lpMultiByte,cbMultiByte, lpWide,cchWide);
  }

  static int WINAPI WideCharToMultiByte(
    UINT cp,
    DWORD flags,
    LPCWCH lpWide,
    int cchWide,
    LPSTR lpMultiByte,
    int cbMultiByte,
    LPCCH lpDef,
    LPBOOL lpUsedDef
  ){
    if (cp == CP_OEMCP)
      cp = over_oemCP;
    else if (cp == CP_ACP || cp == CP_THREAD_ACP)
      cp = over_ansiCP;

    return ::WideCharToMultiByte(
      cp, flags, lpWide,cchWide, lpMultiByte,cbMultiByte, lpDef,lpUsedDef);
  }
};

//#############################################################################
//#############################################################################

typedef const IMAGE_THUNK_DATA * PCImgThunkData;
typedef DWORD RVA;

static __inline PIMAGE_NT_HEADERS WINAPI PinhFromImageBase(HMODULE hmod)
{
  return reinterpret_cast<PIMAGE_NT_HEADERS>(PBYTE(hmod) + PIMAGE_DOS_HEADER(hmod)->e_lfanew);
}

template<class X> X PFromRva(HMODULE m, RVA rva) { return reinterpret_cast<X>(PBYTE(m) + rva); }

static __inline unsigned CountOfImports(PCImgThunkData pitdBase)
{
  unsigned cRet = 0;
  PCImgThunkData  pitd = pitdBase;
  while (pitd->u1.Function) { pitd++; cRet++; }
  return cRet;
}

//#############################################################################

static bool patch_IAT(void **ppIAT, const void *pf)
{
  MEMORY_BASIC_INFORMATION mbi;
  ::VirtualQuery((LPCVOID)ppIAT, &mbi, sizeof(mbi));
  DWORD prot;

  switch (mbi.Protect) {
  case PAGE_READWRITE:
  case PAGE_EXECUTE_READWRITE:
    *ppIAT = const_cast<void*>(pf);
    return true;
  case PAGE_READONLY:
    prot = PAGE_READWRITE;
    break;
  default: // PAGE_EXECUTE, PAGE_EXECUTE_READ
    prot = PAGE_EXECUTE_READWRITE;
    break;
  }

  if (!::VirtualProtect(mbi.BaseAddress, mbi.RegionSize, prot, &prot))
    return false;

  *ppIAT = const_cast<void*>(pf);

  ::VirtualProtect(mbi.BaseAddress, mbi.RegionSize, mbi.Protect, &prot);
  return true;
}

//#############################################################################

static bool patch_7z_dll()
{
  static const char dll_7z[] = "7z.dll";
  static const char dll_kernel[] = "kernel32.dll";
  static const char f_GetOEMCP[] = "GetOEMCP";
  static const char f_GetACP[] = "GetACP";
  static const char f_MultiByteToWideChar[] = "MultiByteToWideChar";
  static const char f_WideCharToMultiByte[] = "WideCharToMultiByte";

  HMODULE hm = ::GetModuleHandleA(dll_7z);
  if (!hm)
    return false;

  auto pinh = PinhFromImageBase(hm);
  if (pinh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size <= 0)
    return false;

  int n_patched = 0;
  auto pid = PFromRva<PIMAGE_IMPORT_DESCRIPTOR>(
    hm, pinh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

  while (pid->OriginalFirstThunk)
  {
    auto szDllCur = PFromRva<LPCSTR>(hm, pid->Name);
    if (0 == _memicmp(szDllCur, dll_kernel, sizeof(dll_kernel)))
    {
      auto pINT = PFromRva<PCImgThunkData>(hm, pid->OriginalFirstThunk);
      auto ppfnIATEntry = PFromRva<FARPROC*>(hm, pid->FirstThunk);
      size_t cpfnIATEntries = CountOfImports(PCImgThunkData(ppfnIATEntry));
      FARPROC *ppfnIATEntryMax = ppfnIATEntry + cpfnIATEntries;

      for (unsigned iIAT = 0; ppfnIATEntry < ppfnIATEntryMax; ppfnIATEntry++, iIAT++)
      {
        PCImgThunkData pitd = &(pINT[iIAT]);
        if (IMAGE_SNAP_BY_ORDINAL(pitd->u1.Ordinal))
          continue;

        auto func = LPCSTR(PFromRva<PIMAGE_IMPORT_BY_NAME>(hm, RVA(UINT_PTR(pitd->u1.AddressOfData)))->Name);
        FARPROC pf = nullptr;
        if (0 == memcmp(func, f_GetOEMCP, sizeof(f_GetOEMCP)))
          pf = reinterpret_cast<FARPROC>(reinterpret_cast<void*>(patched::GetOEMCP));
        else if (0 == memcmp(func, f_GetACP, sizeof(f_GetACP)))
          pf = reinterpret_cast<FARPROC>(reinterpret_cast<void*>(patched::GetACP));
        else if (0 == memcmp(func, f_MultiByteToWideChar, sizeof(f_MultiByteToWideChar)))
          pf = reinterpret_cast<FARPROC>(reinterpret_cast<void*>(patched::MultiByteToWideChar));
        else if (0 == memcmp(func, f_WideCharToMultiByte, sizeof(f_WideCharToMultiByte)))
          pf = reinterpret_cast<FARPROC>(reinterpret_cast<void*>(patched::WideCharToMultiByte));

        if (pf != nullptr && patch_IAT(reinterpret_cast<void**>(ppfnIATEntry), reinterpret_cast<void*>(pf)))
          ++n_patched;
      }
    }
    ++pid;
  }

  return n_patched > 0;
}

//#############################################################################
//#############################################################################

static bool is_valid_cp(UINT cp)
{
  CPINFO cpinfo;
  return cp > CP_THREAD_ACP && ::GetCPInfo(cp, &cpinfo) != FALSE;
}

//#############################################################################

void Patch7zCP::SetCP(UINT oemCP, UINT ansiCP)
{
  if (orig_oemCP == 0)
    orig_oemCP = over_oemCP = ::GetOEMCP();
  if (orig_ansiCP == 0)
    orig_ansiCP = over_ansiCP = ::GetACP();

  if (!is_valid_cp(oemCP))
    oemCP = orig_oemCP;
  if (!is_valid_cp(ansiCP))
    ansiCP = orig_ansiCP;

  over_oemCP = oemCP;
  over_ansiCP = ansiCP;

  if (!patched_7z_dll && (orig_oemCP != oemCP || orig_ansiCP != ansiCP))
  {
    patch_7z_dll();
    patched_7z_dll = true;
  }
}

//#############################################################################
//#############################################################################
