#include "rsrc.hpp"
#include "utils.hpp"
#include "farutils.hpp"
#include "common.hpp"
#include "utils.hpp"
#include "sysutils.hpp"

class RsrcId {
private:
  std::wstring name;
  WORD id;
  void set(LPCTSTR name_id) {
    if (IS_INTRESOURCE(name_id)) {
      id = static_cast<WORD>(reinterpret_cast<size_t>(name_id));
      name.clear();
    }
    else {
      id = 0;
      name = name_id;
    }
  }
public:
  RsrcId() {
    set({});
  }
  RsrcId(LPCTSTR name_id) {
    set(name_id);
  }
  void operator=(LPCTSTR name_id) {
    set(name_id);
  }
  operator LPCTSTR() const {
    return name.empty() ? MAKEINTRESOURCE(id) : name.c_str();
  }
  bool is_int() const {
    return name.empty();
  }
};

struct IconImage {
  BYTE width{};
  BYTE height{};
  BYTE color_cnt{};
  WORD plane_cnt{};
  WORD bit_cnt{};
  ByteVector bitmap;
};

typedef std::list<IconImage> IconFile;

#pragma pack(push)
#pragma pack(1)
struct IconFileHeader {
  WORD reserved; // Reserved (must be 0)
  WORD type;     // Resource Type (1 for icons)
  WORD count;    // How many images?
};

struct IconFileEntry {
  BYTE width;         // Width, in pixels, of the image
  BYTE height;        // Height, in pixels, of the image
  BYTE color_count;   // Number of colors in image (0 if >=8bpp)
  BYTE reserved;      // Reserved ( must be 0)
  WORD planes;        // Color Planes
  WORD bit_count;     // Bits per pixel
  DWORD bytes_in_res; // How many bytes in this resource?
  DWORD image_offset; // Where in the file is this image?
};
#pragma pack(pop)

static IconFile load_icon_file(const std::wstring& file_path) {
  IconFile icon;
  File file(file_path, FILE_READ_DATA, FILE_SHARE_READ, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
  IconFileHeader header;
  UInt64 file_size = file.size();
  file.read(&header, sizeof(header));
  for (unsigned i = 0; i < header.count; i++) {
    IconImage image;
    IconFileEntry entry;
    file.read(&entry, sizeof(entry));
    image.width = entry.width;
    image.height = entry.height;
    image.color_cnt = entry.color_count;
    image.plane_cnt = entry.planes;
    image.bit_cnt = entry.bit_count;
    CHECK(entry.image_offset + entry.bytes_in_res <= file_size);
    UInt64 cur_pos = file.set_pos(0, FILE_CURRENT);
    file.set_pos(entry.image_offset);
    Buffer<unsigned char> buf(entry.bytes_in_res);
    file.read(buf.data(), buf.size());
    image.bitmap.assign(buf.data(), buf.data() + buf.size());
    file.set_pos(cur_pos);
    icon.push_back(image);
  }
  return icon;
}


struct IconImageRsrc {
  WORD id{};
  WORD lang_id{};
  IconImage image;
};

struct IconRsrc {
  RsrcId id;
  WORD lang_id;
  std::list<IconImageRsrc> images;
};

#pragma pack(push)
#pragma pack(2)
struct IconGroupHeader {
  WORD reserved; // Reserved (must be 0)
  WORD type;     // Resource type (1 for icons)
  WORD count;    // How many images?
};

struct IconGroupEntry {
  BYTE width;     // Width, in pixels, of the image
  BYTE height;    // Height, in pixels, of the image
  BYTE color_cnt; // Number of colors in image (0 if >=8bpp)
  BYTE reserved;  // Reserved
  WORD plane_cnt; // Color Planes
  WORD bit_cnt;   // Bits per pixel
  DWORD size;     // how many bytes in this resource?
  WORD id;        // the ID
};
#pragma pack(pop)

static IconRsrc load_icon_rsrc(HMODULE h_module, LPCTSTR name, WORD lang_id) {
  IconRsrc icon_rsrc;
  icon_rsrc.id = name;
  icon_rsrc.lang_id = lang_id;
  HRSRC h_rsrc_group = FindResourceEx(h_module, RT_GROUP_ICON, name, lang_id);
  CHECK_SYS(h_rsrc_group);
  HGLOBAL h_global_group = LoadResource(h_module, h_rsrc_group);
  CHECK_SYS(h_global_group);
  unsigned char* res_data = static_cast<unsigned char*>(LockResource(h_global_group));
  CHECK_SYS(res_data);
  const IconGroupHeader* header = reinterpret_cast<const IconGroupHeader*>(res_data);
  for (unsigned i = 0; i < header->count; i++) {
    IconImageRsrc image_rsrc;
    const IconGroupEntry* entry = reinterpret_cast<const IconGroupEntry*>(res_data + sizeof(IconGroupHeader) + i * sizeof(IconGroupEntry));
    image_rsrc.id = entry->id;
    image_rsrc.lang_id = lang_id;
    image_rsrc.image.width = entry->width;
    image_rsrc.image.height = entry->height;
    image_rsrc.image.color_cnt = entry->color_cnt;
    image_rsrc.image.plane_cnt = entry->plane_cnt;
    image_rsrc.image.bit_cnt = entry->bit_cnt;
    HRSRC h_rsrc = FindResourceEx(h_module, RT_ICON, MAKEINTRESOURCE(entry->id), lang_id);
    CHECK_SYS(h_rsrc);
    HGLOBAL h_global = LoadResource(h_module, h_rsrc);
    CHECK_SYS(h_global);
    unsigned char* icon_data = static_cast<unsigned char*>(LockResource(h_global));
    CHECK_SYS(icon_data);
    image_rsrc.image.bitmap.assign(icon_data, icon_data + entry->size);
    icon_rsrc.images.push_back(image_rsrc);
  }
  return icon_rsrc;
}

static BOOL CALLBACK enum_names_proc(HMODULE h_module, LPCTSTR type, LPTSTR name, LONG_PTR param) {
  try {
    std::list<RsrcId>* result = reinterpret_cast<std::list<RsrcId>*>(param);
    result->push_back(name);
    return TRUE;
  }
  catch (...) {
    return FALSE;
  }
}

static std::list<RsrcId> enum_rsrc_names(HMODULE h_module, LPCTSTR type) {
  std::list<RsrcId> result;
  EnumResourceNames(h_module, type, enum_names_proc, reinterpret_cast<LONG_PTR>(&result));
  return result;
}

static BOOL CALLBACK enum_langs_proc(HMODULE h_module, LPCTSTR type, LPCTSTR name, WORD language, LONG_PTR param) {
  try {
    std::list<WORD>* result = reinterpret_cast<std::list<WORD>*>(param);
    result->push_back(language);
    return TRUE;
  }
  catch (...) {
    return FALSE;
  }
}

static std::list<WORD> enum_rsrc_langs(HMODULE h_module, LPCTSTR type, LPCTSTR name) {
  std::list<WORD> result;
  EnumResourceLanguages(h_module, type, name, enum_langs_proc, reinterpret_cast<LONG_PTR>(&result));
  return result;
}

class RsrcModule {
private:
  HMODULE h_module;
public:
  RsrcModule(const std::wstring& file_path) {
    h_module = LoadLibraryEx(file_path.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE);
    CHECK_SYS(h_module);
  }
  ~RsrcModule() {
    close();
  }
  HMODULE handle() const {
    return h_module;
  }
  void close() {
    if (h_module) {
      FreeLibrary(h_module);
      h_module = nullptr;
    }
  }
};

class ResourceUpdate {
private:
  HANDLE handle;
public:
  ResourceUpdate(const std::wstring& file_path) {
    handle = BeginUpdateResource(file_path.c_str(), FALSE);
    CHECK_SYS(handle);
  }
  ~ResourceUpdate() {
    if (handle)
      EndUpdateResource(handle, TRUE);
  }
  void update(const wchar_t* type, const wchar_t* name, WORD lang_id, const unsigned char* data, DWORD size) {
    assert(handle);
    CHECK_SYS(UpdateResource(handle, type, name, lang_id, reinterpret_cast<LPVOID>(const_cast<unsigned char*>(data)), size));
  }
  void finalize() {
    CHECK_SYS(EndUpdateResource(handle, FALSE));
    handle = nullptr;
  }
};

void replace_icon(const std::wstring& pe_path, const std::wstring& ico_path) {
  std::list<IconRsrc> icons;
  RsrcModule module(pe_path);
  std::list<RsrcId> group_ids = enum_rsrc_names(module.handle(), RT_GROUP_ICON);
  std::for_each(group_ids.cbegin(), group_ids.cend(), [&] (const RsrcId& id) {
    std::list<WORD> lang_ids = enum_rsrc_langs(module.handle(), RT_GROUP_ICON, id);
    std::for_each(lang_ids.cbegin(), lang_ids.cend(), [&] (WORD lang_id) {
      icons.emplace_back(load_icon_rsrc(module.handle(), id, lang_id));
    });
  });
  module.close();

  ResourceUpdate rupdate(pe_path);
  // delete existing icons
  std::for_each(icons.cbegin(), icons.cend(), [&] (const IconRsrc& icon) {
    for_each (icon.images.cbegin(), icon.images.cend(), [&] (const IconImageRsrc& image) {
      rupdate.update(RT_ICON, MAKEINTRESOURCE(image.id), image.lang_id, nullptr, 0);
    });
    rupdate.update(RT_GROUP_ICON, icon.id, icon.lang_id, nullptr, 0);
  });

  WORD lang_id;
  if (!icons.empty())
    lang_id = icons.front().lang_id;
  else
    lang_id = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
  IconFile icon_file = load_icon_file(ico_path);
  IconRsrc icon_rsrc;
  icon_rsrc.lang_id = lang_id;
  std::for_each(icon_file.cbegin(), icon_file.cend(), [&] (const IconImage& image) {
    IconImageRsrc image_rsrc;
    image_rsrc.lang_id = lang_id;
    image_rsrc.image = image;
    icon_rsrc.images.push_back(image_rsrc);
  });

  // drop first icon (all languages)
  if (!icons.empty()) {
    RsrcId id = icons.front().id;
    while (!icons.empty() && icons.front().id == id)
      icons.pop_front();
  }

  icons.push_front(icon_rsrc);

  // renumber resource ids
  WORD icon_id = 1;
  WORD image_id = 1;
  std::for_each(icons.begin(), icons.end(), [&] (IconRsrc& icon) {
    if (icon.id.is_int()) {
      icon.id = MAKEINTRESOURCE(icon_id);
      icon_id++;
    }
    std::for_each(icon.images.begin(), icon.images.end(), [&] (IconImageRsrc& image) {
      image.id = image_id;
      image_id++;
    });
  });

  // write new icons
  std::for_each(icons.cbegin(), icons.cend(), [&] (const IconRsrc& icon) {
    Buffer<unsigned char> buf(sizeof(IconGroupHeader) + icon.images.size() * sizeof(IconGroupEntry));
    IconGroupHeader* header = reinterpret_cast<IconGroupHeader*>(buf.data());
    header->reserved = 0;
    header->type = 1;
    header->count = static_cast<WORD>(icon.images.size());
    unsigned offset = sizeof(IconGroupHeader);
    for_each (icon.images.cbegin(), icon.images.cend(), [&] (const IconImageRsrc& image) {
      IconGroupEntry* entry = reinterpret_cast<IconGroupEntry*>(buf.data() + offset);
      entry->width = image.image.width;
      entry->height = image.image.height;
      entry->color_cnt = image.image.color_cnt;
      entry->reserved = 0;
      entry->plane_cnt = image.image.plane_cnt;
      entry->bit_cnt = image.image.bit_cnt;
      entry->size = static_cast<DWORD>(image.image.bitmap.size());
      entry->id = image.id;
      rupdate.update(RT_ICON, MAKEINTRESOURCE(image.id), image.lang_id, image.image.bitmap.data(), static_cast<DWORD>(image.image.bitmap.size()));
      offset += sizeof(IconGroupEntry);
    });
    rupdate.update(RT_GROUP_ICON, icon.id, icon.lang_id, buf.data(), static_cast<DWORD>(buf.size()));
  });
  rupdate.finalize();
}


struct VersionEncoder: public ByteVector {
private:
  template<typename T> void encode(const T& d) {
    const unsigned char* b = reinterpret_cast<const unsigned char*>(&d);
    insert(end(), b, b + sizeof(T));
  }
  void encode_string(const wchar_t* d, size_t l) {
    const unsigned char* b = reinterpret_cast<const unsigned char*>(d);
    insert(end(), b, b + (l + 1) * sizeof(wchar_t));
  }
public:
  void encode_WORD(WORD d) {
    encode(d);
  }
  void encode_DWORD(DWORD d) {
    encode(d);
  }
  void encode_string(const wchar_t* d) {
    encode_string(d, std::wcslen(d));
  }
  void encode_string(const std::wstring& d) {
    encode_string(d.c_str(), d.size());
  }
  void pad() {
    unsigned l = size() % 4;
    if (l)
      insert(end(), 4 - l, 0);
  }
  void update_length(size_t p) {
    assert(p + sizeof(WORD) <= size());
    *reinterpret_cast<WORD*>(data() + p) = static_cast<WORD>(size() - p);
  }
};

struct IdLang {
  RsrcId id;
  WORD lang_id;
};
void replace_ver_info(const std::wstring& pe_path, const SfxVersionInfo& ver_info) {
  // numeric version
  std::list<std::wstring> ver_parts = split(ver_info.version, L'.');
  DWORD ver_hi = 0, ver_lo = 0;
  auto ver_part = ver_parts.cbegin();
  if (ver_part != ver_parts.end()) {
    ver_hi |= (str_to_int(*ver_part) & 0xFFFF) << 16;
    ++ver_part;
  }
  if (ver_part != ver_parts.end()) {
    ver_hi |= str_to_int(*ver_part) & 0xFFFF;
    ++ver_part;
  }
  if (ver_part != ver_parts.end()) {
    ver_lo |= (str_to_int(*ver_part) & 0xFFFF) << 16;
    ++ver_part;
  }
  if (ver_part != ver_parts.end()) {
    ver_lo |= str_to_int(*ver_part) & 0xFFFF;
    ++ver_part;
  }

  // existing version info list
  std::list<IdLang> vi_list;
  RsrcModule module(pe_path);
  std::list<RsrcId> ids = enum_rsrc_names(module.handle(), RT_VERSION);
  std::for_each(ids.begin(), ids.end(), [&] (const RsrcId& id) {
    std::list<WORD> lang_ids = enum_rsrc_langs(module.handle(), RT_VERSION, id);
    std::for_each(lang_ids.begin(), lang_ids.end(), [&] (WORD lang_id) {
      IdLang id_lang;
      id_lang.id = id;
      id_lang.lang_id = lang_id;
      vi_list.push_back(id_lang);
    });
  });
  module.close();

  WORD lang_id;
  RsrcId ver_id;
  if (vi_list.empty()) {
    ver_id = MAKEINTRESOURCE(1);
    lang_id = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
  }
  else {
    ver_id = vi_list.front().id;
    lang_id = vi_list.front().lang_id;
  }

  // encode version info
  VersionEncoder e;
  e.reserve(4096);

  size_t vs_versioninfo_pos = e.size();
  e.encode_WORD(0); // VS_VERSIONINFO
  e.encode_WORD(sizeof(VS_FIXEDFILEINFO));
  e.encode_WORD(0);
  e.encode_string(L"VS_VERSION_INFO");
  e.pad();

  e.encode_DWORD(0xFEEF04BD); // VS_FIXEDFILEINFO
  e.encode_DWORD(0x00010000);
  e.encode_DWORD(ver_hi);
  e.encode_DWORD(ver_lo);
  e.encode_DWORD(ver_hi);
  e.encode_DWORD(ver_lo);
  e.encode_DWORD(0x3F);
  e.encode_DWORD(0);
  e.encode_DWORD(VOS_NT_WINDOWS32);
  e.encode_DWORD(VFT_APP);
  e.encode_DWORD(0);
  e.encode_DWORD(0);
  e.encode_DWORD(0);
  e.pad();

  size_t string_file_info_pos = e.size();
  e.encode_WORD(0); // StringFileInfo
  e.encode_WORD(0);
  e.encode_WORD(1);
  e.encode_string(L"StringFileInfo");
  e.pad();

  size_t string_table_pos = e.size();
  e.encode_WORD(0); // StringTable
  e.encode_WORD(0);
  e.encode_WORD(1);
  std::wostringstream st;
  st << std::hex << std::setw(4) << std::setfill(L'0') << lang_id << L"04b0";
  e.encode_string(st.str());
  e.pad();

  std::map<std::wstring, std::wstring> strings;
  strings[L"Comments"] = ver_info.comments;
  strings[L"CompanyName"] = ver_info.company_name;
  strings[L"FileDescription"] = ver_info.file_description;
  strings[L"FileVersion"] = ver_info.version;
  strings[L"LegalCopyright"] = ver_info.legal_copyright;
  strings[L"ProductName"] = ver_info.product_name;
  strings[L"ProductVersion"] = ver_info.version;

  std::for_each(strings.cbegin(), strings.cend(), [&] (const std::pair<std::wstring, std::wstring>& str) {
    size_t string_pos = e.size();
    e.encode_WORD(0); // String
    e.encode_WORD(static_cast<WORD>(str.second.size() + 1));
    e.encode_WORD(1);
    e.encode_string(str.first);
    e.pad();
    e.encode_string(str.second);
    e.pad();
    e.update_length(string_pos);
  });

  e.update_length(string_table_pos);

  e.update_length(string_file_info_pos);

  size_t var_file_info_pos = e.size();
  e.encode_WORD(0); // VarFileInfo
  e.encode_WORD(0);
  e.encode_WORD(1);
  e.encode_string(L"VarFileInfo");
  e.pad();

  size_t var_pos = e.size();
  e.encode_WORD(0); // Var
  e.encode_WORD(4);
  e.encode_WORD(0);
  e.encode_string(L"Translation");
  e.pad();
  e.encode_WORD(lang_id);
  e.encode_WORD(0x04B0);
  e.update_length(var_pos);

  e.update_length(var_file_info_pos);

  e.update_length(vs_versioninfo_pos);

  // wrire resource
  ResourceUpdate rupdate(pe_path);
  std::for_each(vi_list.cbegin(), vi_list.cend(), [&] (const IdLang& id_lang) {
    rupdate.update(RT_VERSION, id_lang.id, id_lang.lang_id, nullptr, 0);
  });
  rupdate.update(RT_VERSION, ver_id, lang_id, e.data(), static_cast<DWORD>(e.size()));
  rupdate.finalize();
}
