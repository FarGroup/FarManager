#include "common.hpp"

wstring extract_version_number(const wstring& text, const wstring& ver_name) {
  size_t pos = text.find(ver_name);
  CHECK(pos != wstring::npos);
  pos += ver_name.size();
  while (pos < text.size() && (text[pos] == L' ' || text[pos] == L'\t'))
    pos++;
  size_t start_pos = pos;
  while (pos < text.size() && text[pos] >= L'0' && text[pos] <= L'9')
    pos++;
  size_t end_pos = pos;
  return text.substr(start_pos, end_pos - start_pos);
}

int wmain(int argc, wchar_t* argv[]) {
  BEGIN_ERROR_HANDLER;
  if (argc != 3)
    FAIL_MSG(L"Usage: farver <plugin.hpp> <farver.ini>");
  wstring text = load_file(argv[1]);
  wstring result;
  result += L"FAR_VER_MAJOR = " + extract_version_number(text, L"FARMANAGERVERSION_MAJOR") + L"\n";
  result += L"FAR_VER_MINOR = " + extract_version_number(text, L"FARMANAGERVERSION_MINOR") + L"\n";
  result += L"FAR_VER_BUILD = " + extract_version_number(text, L"FARMANAGERVERSION_BUILD") + L"\n";
  save_file(argv[2], result, CP_UTF8);
  return 0;
  END_ERROR_HANDLER;
  return 1;
}
