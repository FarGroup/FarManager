#include "utils.hpp"

std::wstring long_path(const std::wstring& path) {
  if (substr_match(path, 0, L"\\\\")) {
    if (substr_match(path, 2, L"?\\") || substr_match(path, 2, L".\\")) {
      return path;
    }
    else {
      return std::wstring(path).replace(0, 1, L"\\\\?\\UNC");
    }
  }
  else {
    return std::wstring(path).insert(0, L"\\\\?\\");
  }
}

std::wstring long_path_norm(const std::wstring& path) {
  auto nt_path = long_path(path);
  auto p = nt_path.find(L'/');
  while (p != std::wstring::npos) {
    nt_path[p] = L'\\'; p = nt_path.find(L'/', p+1);
  }
  return nt_path;
}

std::wstring add_trailing_slash(const std::wstring& path) {
  if ((path.size() == 0) || (path[path.size() - 1] == L'\\')) {
    return path;
  }
  else {
    return path + L'\\';
  }
}

std::wstring del_trailing_slash(const std::wstring& path) {
  if ((path.size() < 2) || (path[path.size() - 1] != L'\\')) {
    return path;
  }
  else {
    return path.substr(0, path.size() - 1);
  }
}

void locate_path_root(const std::wstring& path, size_t& path_root_len, bool& is_unc_path) {
  unsigned prefix_len = 0;
  is_unc_path = false;
  if (substr_match(path, 0, L"\\\\")) {
    if (substr_match(path, 2, L"?\\UNC\\")) {
      prefix_len = 8;
      is_unc_path = true;
    }
    else if (substr_match(path, 2, L"?\\") || substr_match(path, 2, L".\\")) {
      prefix_len = 4;
    }
    else {
      prefix_len = 2;
      is_unc_path = true;
    }
  }
  if ((prefix_len == 0) && !substr_match(path, 1, L":\\")) {
    path_root_len = 0;
  }
  else {
    std::wstring::size_type p = path.find(L'\\', prefix_len);
    if (p == std::wstring::npos) {
      p = path.size();
    }
    if (is_unc_path) {
      p = path.find(L'\\', p + 1);
      if (p == std::wstring::npos) {
        p = path.size();
      }
    }
    path_root_len = p;
  }
}

std::wstring extract_path_root(const std::wstring& path) {
  size_t path_root_len;
  bool is_unc_path;
  locate_path_root(path, path_root_len, is_unc_path);
  if (path_root_len)
    return path.substr(0, path_root_len).append(1, L'\\');
  else
    return std::wstring();
}

std::wstring extract_file_name(const std::wstring& path) {
  size_t pos = path.rfind(L'\\');
  if (pos == std::wstring::npos) {
    pos = 0;
  }
  else {
    pos++;
  }
  size_t path_root_len;
  bool is_unc_path;
  locate_path_root(path, path_root_len, is_unc_path);
  if ((pos <= path_root_len) && (path_root_len != 0))
    return std::wstring();
  else
    return path.substr(pos);
}

std::wstring extract_file_path(const std::wstring& path) {
  size_t pos = path.rfind(L'\\');
  if (pos == std::wstring::npos) {
    pos = 0;
  }
  size_t path_root_len;
  bool is_unc_path;
  locate_path_root(path, path_root_len, is_unc_path);
  if ((pos <= path_root_len) && (path_root_len != 0))
    return path.substr(0, path_root_len).append(1, L'\\');
  else
    return path.substr(0, pos);
}

std::wstring extract_file_ext(const std::wstring& path) {
  size_t ext_pos = path.rfind(L'.');
  if (ext_pos == std::wstring::npos) {
    return std::wstring();
  }
  size_t name_pos = path.rfind(L'\\');
  if (name_pos == std::wstring::npos) {
    name_pos = 0;
  }
  else {
    name_pos++;
  }
  if (ext_pos <= name_pos)
    return std::wstring();
  size_t path_root_len;
  bool is_unc_path;
  locate_path_root(path, path_root_len, is_unc_path);
  if ((ext_pos <= path_root_len) && (path_root_len != 0))
    return std::wstring();
  else
    return path.substr(ext_pos);
}

bool is_root_path(const std::wstring& path) {
  size_t path_root_len;
  bool is_unc_path;
  locate_path_root(path, path_root_len, is_unc_path);
  return (path.size() == path_root_len) || ((path.size() == path_root_len + 1) && (path[path.size() - 1] == L'\\'));
}

bool is_unc_path(const std::wstring& path) {
  size_t path_root_len;
  bool is_unc_path;
  locate_path_root(path, path_root_len, is_unc_path);
  return is_unc_path;
}

bool is_absolute_path(const std::wstring& path) {
  size_t path_root_len;
  bool is_unc_path;
  locate_path_root(path, path_root_len, is_unc_path);
  if (path_root_len == 0)
    return false;
  std::wstring::size_type p1 = path_root_len;
  while (p1 != path.size()) {
    p1 += 1;
    std::wstring::size_type p2 = path.find(L'\\', p1);
    if (p2 == std::wstring::npos)
      p2 = path.size();
    std::wstring::size_type sz = p2 - p1;
    if (sz == 1 && path[p1] == L'.')
      return false;
    if (sz == 2 && path[p1] == L'.' && path[p1 + 1] == L'.')
      return false;
    p1 = p2;
  }
  return true;
}

std::wstring remove_path_root(const std::wstring& path) {
  size_t path_root_len;
  bool is_unc_path;
  locate_path_root(path, path_root_len, is_unc_path);
  if ((path_root_len < path.size()) && (path[path_root_len] == L'\\'))
    path_root_len++;
  return path.substr(path_root_len);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#ifndef TOOLS_TOOL

static const wchar_t simple_replace_char  = L'_';
static const wchar_t simple_replace_str[] = L"_";

// '<'
static const wchar_t LeftPointing_Double_Angle_Quotation_Mark  = L'\u00AB'; // '«'
//static const wchar_t Single_LeftPointing_Angle_Quotation_Mark  = L'\u2039'; // '‹'

// '>'
static const wchar_t RightPointing_Double_Angle_Quotation_Mark = L'\u00BB'; // '»'
//static const wchar_t Single_RightPointing_Angle_Quotation_Mark = L'\u203A'; // '›'

static wchar_t quotes2[2] = { LeftPointing_Double_Angle_Quotation_Mark, RightPointing_Double_Angle_Quotation_Mark };
//static wchar_t quotes1[2] = { Single_LeftPointing_Angle_Quotation_Mark, Single_RightPointing_Angle_Quotation_Mark };

// ':'
//static const wchar_t Division_Sign                             = L'\u00F7'; // '÷'
//static const wchar_t Identical_To                              = L'\u2216'; // '≡'
static const wchar_t Horizontal_Ellipsis                       = L'\u2026'; // '…'
static const wchar_t American_Full_Stop                        = L'\u0589'; // '։' --missed in Lucida Console font
//static const wchar_t Raised Colon                              = L'\u02F8'; // '˸' --missed in Lucida Console font
static wchar_t colons[2] = {Horizontal_Ellipsis, American_Full_Stop };

// '*'
static const wchar_t Currency_Sign                             = L'\u00A4'; // '¤'
static const wchar_t Six_Pointed_Black_Star                    = L'\u0589'; // '✶' --missed in Lucida Console font
static wchar_t asterisks[2] = { Currency_Sign, Six_Pointed_Black_Star };

// '?'
static const wchar_t Inverted_Question_Mark                    = L'\u00BF'; // '¿'
//static const wchar_t Interrobang                               = L'\u203D'; // '‽' --missed in Lucida Console font

// '|'
static const wchar_t Broken_Bar                                = L'\u00A6'; // '¦'
//static const wchar_t Dental_Click                              = L'\u01C0'; // 'ǀ' --missed in Lucida Console font

// '"'
static const wchar_t Left_Double_Quotation_Mark                = L'\u201C'; // '“'
static const wchar_t Right_Double_Quotation_Mark               = L'\u201D'; // '”'
static wchar_t quotes3[2] = { Left_Double_Quotation_Mark, Right_Double_Quotation_Mark };

// '/'
static const wchar_t Fraction_Slash                            = L'\u2044'; // '⁄'

// '\'
static const wchar_t Not_Sign                                  = L'\u00AC'; // '¬'

//static const wchar_t Middle_Dot                              = L'\u00B7'; // '·' 

static const wchar_t control_chars[32 - 1] =
{ L'\u263A' // \x01  '☺'  (White Smiling Face)
, L'\u263B' // \x02  '☻'  (Black Smiling Face)
, L'\u2665' // \x03  '♥'  (Black Heart Suit)
, L'\u2666' // \x04  '♦'  (Black Diamond Suit)
, L'\u2663' // \x05  '♣'  (Black Club Suit)
, L'\u2660' // \x06  '♠'  (Black Spade Suit)
, L'\u2022' // \x07  '•'  (Bullet)
, L'\u25D8' // \x08  '◘'  (Inverse Bullet)
, L'\u25CB' // \x09  '○'  (White Circle)
, L'\u25D9' // \x0A  '◙'  (Inverse White Circle)
, L'\u2642' // \x0B  '♂'  (Male Sign)
, L'\u2640' // \x0C  '♀'  (Female Sign)
, L'\u266A' // \x0D  '♪'  (Eight Note)
, L'\u266B' // \x0E  '♫'  (Beamed Eight Note)
, L'\u263C' // \x0F  '☼'  (White Sun With Rays)
, L'\u25BA' // \x10  '►'  (Black Right-Pointing Pointer)
, L'\u25C4' // \x11  '◄'  (Black Left-Pointing Pointer)
, L'\u2195' // \x12  '↕'  (Up Down Arrow)
, L'\u203C' // \x13  '‼'  (Double Exclamation Mark)
, L'\u00B6' // \x14  '¶'  (Pilcrow Sign)
, L'\u00A7' // \x15  '§'  (Section Sign)
, L'\u25AC' // \x16  '▬'  (Black Rectangle)
, L'\u21A8' // \x17  '↨'  (Up Down Arrow With Base)
, L'\u2191' // \x18  '↑'  (Upwards Arrow)
, L'\u2193' // \x19  '↓'  (Downwards Arrow)
, L'\u2192' // \x1A  '→'  (Rightwards Arrow)
, L'\u2190' // \x1B  '←'  (Leftwards Arrow)
, L'\u221F' // \x1C  '∟'  (Right Angle)
, L'\u2194' // \x1D  '↔'  (Left Right Arrow)
, L'\u25B2' // \x1E  '▲'  (Black Up-Pointing Triangle)
, L'\u25BC' // \x1F  '▼'  (Black Down-Pointing Triangle)
};

//-----------------------------------------------------------------------------

static const wchar_t* reserved_names[] =
{ L"CON", L"PRN", L"AUX", L"NUL", L"COM9", L"LPT9" };

static bool is_matched(const std::wstring& name, const wchar_t *res)
{
  size_t i = 0, len = name.size();
  while (i < len)
  {
    auto c1 = res[i];
    auto c2 = name[i];
    if (!c1) {
      break;
    }
    else if (c1 == L'9') {
      if (c2 < '0' || c2 > L'9')
        return false;
    }
    else {
      if (c1 != (wchar_t)(size_t)CharUpperW((LPWSTR)(size_t)c2))
        return false;
    }
    ++i;
  }
  if (res[i])
    return false;

  while (i < len)
  {
    if (name[i] == L'.')
      return true;
    else if (name[i] != L' ')
      return false;
    ++i;
  }
  return true;
}

static bool is_reserved_name(const std::wstring& name)
{
  for (const auto& res : reserved_names)
  {
    if (is_matched(name, res))
      return true;
  }
  return false;
}

//-----------------------------------------------------------------------------

std::wstring correct_filename(const std::wstring& orig_name, int mode, bool alt_stream)
{
  bool correct_empty = (mode & 0x10) != 0;
  bool remove_final_dotsp = (mode & 0x20) != 0;
  bool correct_reserved = (mode & 0x40) != 0;
  int m = std::min(mode & 0x0f, 3);
    
  auto name(orig_name);
  if (m > 0)
  {
    int i = 0, q = 0;
    for (const auto c : name)
    {
      switch (c) {
      case L'<':  name[i] = m > 1 ? quotes2[0] : simple_replace_char;
        break;
      case L'>':  name[i] = m > 1 ? quotes2[1] : simple_replace_char;
        break;
      case L':':  name[i] = alt_stream ? c : (m > 1 ? colons[m-2] : simple_replace_char);
        break;
      case L'*':  name[i] = m > 1 ? asterisks[m-2] : simple_replace_char;
        break;
      case L'?':  name[i] = m > 1 ? Inverted_Question_Mark : simple_replace_char;
        break;
      case L'|':  name[i] = m > 1 ? Broken_Bar : simple_replace_char;
        break;
      case L'"':  name[i] = m > 1 ? quotes3[q] : simple_replace_char; q = 1 - q;
        break;
      case L'/':  name[i] = m > 1 ? Fraction_Slash : simple_replace_char;
        break;
      case L'\\': name[i] = m > 1 ? Not_Sign : simple_replace_char;
        break;
      default: if (c < L' ' && c > L'\0') name[i] = m > 1 ? control_chars[c-L'\x01'] : simple_replace_char;
        break;
      }
      ++i;
    }
  }
    
  if (correct_reserved)
  {
    if (is_reserved_name(name))
      name = simple_replace_str + name;
  }
    
  if (remove_final_dotsp)
  {
    while (!name.empty() && (name.back() == L'.' || name.back() == L' '))
      name.pop_back();
  }
    
  if (name.empty() && correct_empty)
  {
    name = simple_replace_str;
  }
    
  return name;
}
#endif
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
