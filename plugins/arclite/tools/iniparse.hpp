#pragma once

namespace Ini {

typedef std::map<std::wstring, std::wstring> Section;
class File: public std::map<std::wstring, Section> {
public:
  std::wstring get(const std::wstring& section_name, const std::wstring& key_name);
  void parse(const std::wstring& text);
};

}
