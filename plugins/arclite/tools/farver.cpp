std::wstring extract_version_number(const std::wstring& text, const std::wstring& ver_name) {
  size_t pos = text.find(ver_name);
  CHECK(pos != std::wstring::npos);
  pos += ver_name.size();
  while (pos < text.size() && (text[pos] == L' ' || text[pos] == L'\t'))
    pos++;
  size_t start_pos = pos;
  while (pos < text.size() && text[pos] >= L'0' && text[pos] <= L'9')
    pos++;
  size_t end_pos = pos;
  return text.substr(start_pos, end_pos - start_pos);
}

void farver(const std::deque<std::wstring>& params) {
  if (params.size() != 2)
    FAIL_MSG(L"Usage: farver <plugin.hpp> <farver.ini>");
  std::wstring text = load_file(params[0]);
  std::wstring result;
  result += L"FAR_VER_MAJOR = " + extract_version_number(text, L"FARMANAGERVERSION_MAJOR") + L"\n";
  result += L"FAR_VER_MINOR = " + extract_version_number(text, L"FARMANAGERVERSION_MINOR") + L"\n";
  result += L"FAR_VER_BUILD = " + extract_version_number(text, L"FARMANAGERVERSION_BUILD") + L"\n";
  save_file(params[1], result, CP_UTF8);
}
