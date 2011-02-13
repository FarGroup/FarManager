#include "utils.hpp"
#include "iniparse.hpp"

namespace Ini {

wstring File::get(const wstring& section_name, const wstring& key_name) {
  const_iterator section_pos = find(section_name);
  CHECK(section_pos != end());
  const Section& section = section_pos->second;
  Section::const_iterator key_pos = section.find(key_name);
  CHECK(key_pos != section.end());
  return key_pos->second;
}

void File::parse(const wstring& text) {
  clear();
  wstring section_name;
  Section section;
  size_t begin_pos = 0;
  while (begin_pos < text.size()) {
    size_t end_pos = text.find(L'\n', begin_pos);
    if (end_pos == wstring::npos)
      end_pos = text.size();
    else
      end_pos++;
    wstring line = strip(text.substr(begin_pos, end_pos - begin_pos));
    if ((line.size() > 2) && (line[0] == L'[') && (line[line.size() - 1] == L']')) {
      // section header
      if (!section.empty()) {
        (*this)[section_name] = section;
        section.clear();
      }
      section_name = strip(line.substr(1, line.size() - 2));
    }
    if ((line.size() > 0) && (line[0] == L';')) {
      // comment
    }
    else {
      size_t delim_pos = line.find(L'=');
      if (delim_pos != wstring::npos) {
        // name = value pair
        wstring name = strip(line.substr(0, delim_pos));
        wstring value = strip(line.substr(delim_pos + 1, line.size() - delim_pos - 1));
        // remove quotes if needed
        if ((value.size() >= 2) && (value[0] == L'"') && (value[value.size() - 1] == L'"'))
          value = value.substr(1, value.size() - 2);
        if (!name.empty() && !value.empty())
          section[name] = value;
      }
    }
    begin_pos = end_pos;
  }
  if (!section.empty()) {
    (*this)[section_name] = section;
  }
}

}
