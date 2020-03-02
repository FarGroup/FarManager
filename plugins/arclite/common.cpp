#include "utils.hpp"
#include "sysutils.hpp"
#include "farutils.hpp"
#include "error.hpp"
#include "common.hpp"
#include "farutils.hpp"
#include "guids.hpp"

Error g_com_error;

int al_round(double d) {
  double a = fabs(d);
  int res = static_cast<int>(a);
  double frac = a - res;
  if (frac >= 0.5)
    res++;
  if (d >= 0)
    return res;
  else
    return -res;
}

unsigned calc_percent(UInt64 completed, UInt64 total) {
  unsigned percent;
  if (total == 0)
    percent = 0;
  else
    percent = al_round(static_cast<double>(completed) / total * 100);
  if (percent > 100)
    percent = 100;
  return percent;
}

UInt64 get_module_version(const std::wstring& file_path) {
  UInt64 version = 0;
  DWORD dw_handle;
  DWORD ver_size = GetFileVersionInfoSizeW(file_path.c_str(), &dw_handle);
  if (ver_size) {
    Buffer<unsigned char> ver_block(ver_size);
    if (GetFileVersionInfoW(file_path.c_str(), dw_handle, ver_size, ver_block.data())) {
      VS_FIXEDFILEINFO* fixed_file_info;
      UINT len;
      if (VerQueryValueW(ver_block.data(), L"\\", reinterpret_cast<LPVOID*>(&fixed_file_info), &len)) {
        return (static_cast<UInt64>(fixed_file_info->dwFileVersionMS) << 32) + fixed_file_info->dwFileVersionLS;
      }
    }
  }
  return version;
}

UInt64 parse_size_string(const std::wstring& str) {
  UInt64 size = 0;
  unsigned i = 0;
  while (i < str.size() && str[i] >= L'0' && str[i] <= L'9') {
    size = size * 10 + (str[i] - L'0');
    i++;
  }
  while (i < str.size() && str[i] == L' ') i++;
  if (i < str.size()) {
    wchar_t mod_ch = str[i];
    UInt64 mod_mul;
    if (mod_ch == L'K' || mod_ch == L'k') mod_mul = 1024;
    else if (mod_ch == L'M' || mod_ch == L'm') mod_mul = 1024 * 1024;
    else if (mod_ch == L'G' || mod_ch == L'g') mod_mul = 1024 * 1024 * 1024;
    else mod_mul = 1;
    size *= mod_mul;
  }
  return size;
}

// expand macros enclosed in question marks
std::wstring expand_macros(const std::wstring& text) {
  const wchar_t c_macro_sep = L'?';
  std::wstring result;
  size_t b_pos, e_pos;
  size_t pos = 0;
  while (true) {
    b_pos = text.find(c_macro_sep, pos);
    if (b_pos == std::wstring::npos)
      break;
    e_pos = text.find(c_macro_sep, b_pos + 1);
    if (e_pos == std::wstring::npos)
      break;

    std::wstring macro = L"print(" + text.substr(b_pos + 1, e_pos - b_pos - 1) + L") Keys('Enter')";
    std::wstring mresult;
    if (Far::post_macro(macro))
      Far::input_dlg(c_generic_guid, std::wstring(), std::wstring(), mresult);
    else
      FAIL(E_ABORT);

    result.append(text.data() + pos, b_pos - pos);
    result.append(mresult);
    pos = e_pos + 1;
  }
  result.append(text.data() + pos, text.size() - pos);
  return result;
}

#define CP_UTF16 1200
std::wstring load_file(const std::wstring& file_path, unsigned* code_page) {
  File file(file_path, FILE_READ_DATA, FILE_SHARE_READ, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
  Buffer<char> buffer(static_cast<size_t>(file.size()));
  unsigned size = static_cast<unsigned>(file.read(buffer.data(), buffer.size()));
  if (size >= 2 && buffer.data()[0] == '\xFF' && buffer.data()[1] == '\xFE') {
    if (code_page)
      *code_page = CP_UTF16;
    return std::wstring(reinterpret_cast<wchar_t*>(buffer.data() + 2), (buffer.size() - 2) / 2);
  }
  else if (size >= 3 && buffer.data()[0] == '\xEF' && buffer.data()[1] == '\xBB' && buffer.data()[2] == '\xBF') {
    if (code_page)
      *code_page = CP_UTF8;
    return ansi_to_unicode(std::string(buffer.data() + 3, size - 3), CP_UTF8);
  }
  else {
    if (code_page)
      *code_page = CP_ACP;
    return ansi_to_unicode(std::string(buffer.data(), size), CP_ACP);
  }
}

std::wstring auto_rename(const std::wstring& file_path) {
  if (File::exists(file_path)) {
    unsigned n = 0;
    while (File::exists(file_path + L"." + int_to_str(n))) n++;
    return file_path + L"." + int_to_str(n);
  }
  else
    return file_path;
}
