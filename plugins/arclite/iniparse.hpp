#pragma once

namespace Ini {

typedef map<wstring, wstring> Section;
class File: public map<wstring, Section> {
public:
  wstring get(const wstring& section_name, const wstring& key_name);
  void parse(const wstring& text);
};

}
