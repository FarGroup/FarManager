#pragma once

#include "comutils.hpp"

extern const ArcType c_7z;
extern const ArcType c_zip;
extern const ArcType c_iso;
extern const ArcType c_udf;
extern const ArcType c_rar;
extern const ArcType c_split;

extern const wchar_t* c_method_copy;
extern const wchar_t* c_method_lzma;
extern const wchar_t* c_method_lzma2;
extern const wchar_t* c_method_ppmd;

extern const unsigned __int64 c_min_volume_size;

extern const wchar_t* c_sfx_ext;
extern const wchar_t* c_volume_ext;

struct ArcLib {
  HMODULE h_module;
  unsigned __int64 version;
  wstring module_path;
  typedef UInt32 (WINAPI *FCreateObject)(const GUID *clsID, const GUID *interfaceID, void **outObject);
  typedef UInt32 (WINAPI *FGetNumberOfMethods)(UInt32 *numMethods);
  typedef UInt32 (WINAPI *FGetMethodProperty)(UInt32 index, PROPID propID, PROPVARIANT *value);
  typedef UInt32 (WINAPI *FGetNumberOfFormats)(UInt32 *numFormats);
  typedef UInt32 (WINAPI *FGetHandlerProperty)(PROPID propID, PROPVARIANT *value);
  typedef UInt32 (WINAPI *FGetHandlerProperty2)(UInt32 index, PROPID propID, PROPVARIANT *value);
  typedef UInt32 (WINAPI *FSetLargePageMode)();
  FCreateObject CreateObject;
  FGetNumberOfMethods GetNumberOfMethods;
  FGetMethodProperty GetMethodProperty;
  FGetNumberOfFormats GetNumberOfFormats;
  FGetHandlerProperty GetHandlerProperty;
  FGetHandlerProperty2 GetHandlerProperty2;

  HRESULT get_prop(UInt32 index, PROPID prop_id, PROPVARIANT* prop) const;
  HRESULT get_bool_prop(UInt32 index, PROPID prop_id, bool& value) const;
  HRESULT get_string_prop(UInt32 index, PROPID prop_id, wstring& value) const;
  HRESULT get_bytes_prop(UInt32 index, PROPID prop_id, ByteVector& value) const;
};

struct ArcFormat {
  unsigned lib_index;
  wstring name;
  bool updatable;
  ByteVector start_signature;
  wstring extension_list;
  wstring default_extension() const;
};

typedef vector<ArcLib> ArcLibs;

class ArcFormats: public map<ArcType, ArcFormat> {
public:
  ArcTypes get_arc_types() const;
  ArcTypes find_by_name(const wstring& name) const;
  ArcTypes find_by_ext(const wstring& ext) const;
};

struct SfxModule {
  wstring path;
  wstring description() const;
  bool all_codecs() const;
  bool install_config() const;
};

class SfxModules: public vector<SfxModule> {
public:
  unsigned find_by_name(const wstring& name) const;
};

class ArcAPI {
private:
  ArcLibs arc_libs;
  ArcFormats arc_formats;
  SfxModules sfx_modules;
  static ArcAPI* arc_api;
  ArcAPI() {}
  ~ArcAPI();
  void load_libs(const wstring& path);
  void find_sfx_modules(const wstring& path);
  void load();
  static ArcAPI* get();
public:
  static const ArcLibs& libs() {
    return get()->arc_libs;
  }
  static const ArcFormats& formats() {
    return get()->arc_formats;
  }
  static const SfxModules& sfx() {
    return get()->sfx_modules;
  }
  static void create_in_archive(const ArcType& arc_type, IInArchive** in_arc);
  static void create_out_archive(const ArcType& format, IOutArchive** out_arc);
  static void free();
};

struct ArcFileInfo {
  UInt32 parent;
  wstring name;
  bool is_dir;
  bool operator<(const ArcFileInfo& file_info) const;
};
typedef vector<ArcFileInfo> FileList;

const UInt32 c_root_index = -1;
const UInt32 c_dup_index = -2;

typedef vector<UInt32> FileIndex;
typedef pair<FileIndex::const_iterator, FileIndex::const_iterator> FileIndexRange;

struct ArcEntry {
  ArcType type;
  size_t sig_pos;
  ArcEntry(const ArcType& type, size_t sig_pos): type(type), sig_pos(sig_pos) {
  }
};

typedef list<ArcEntry> ArcEntries;

class ArcChain: public list<ArcEntry> {
public:
  wstring to_string() const;
};

class Archive;
typedef vector<ComObject<Archive>> Archives;

class Archive: public ComBase {
public:
  UNKNOWN_IMPL

  // open
private:
  ComObject<IInArchive> in_arc;
  bool open(IInStream* in_stream, const ArcType& type);
  static ArcEntries detect(IInStream* stream, const wstring& file_ext, const ArcTypes& arc_types);
  static void open(const OpenOptions& options, Archives& archives);
public:
  static unsigned max_check_size;
  wstring arc_path;
  FindData arc_info;
  set<wstring> volume_names;
  ArcChain arc_chain;
  wstring arc_dir() const {
    return extract_file_path(arc_path);
  }
  wstring arc_name() const {
    wstring name = extract_file_name(arc_path);
    return name.empty() ? arc_path : name;
  }
  static Archives open(const OpenOptions& options);
  void close();
  void reopen();
  bool is_open() const {
    return in_arc;
  }
  bool updatable() const {
    return arc_chain.size() == 1 && ArcAPI::formats().at(arc_chain.back().type).updatable;
  }
  bool is_pure_7z() const {
    return arc_chain.size() == 1 && arc_chain.back().type == c_7z && arc_chain.back().sig_pos == 0;
  }

  // archive contents
public:
  UInt32 num_indices;
  FileList file_list;
  FileIndex file_list_index;
  void make_index();
  UInt32 find_dir(const wstring& dir);
  FileIndexRange get_dir_list(UInt32 dir_index);
  bool get_stream(UInt32 index, IInStream** stream);
  wstring get_path(UInt32 index);
  FindData get_file_info(UInt32 index);
  bool get_main_file(UInt32& index) const;
  DWORD get_attr(UInt32 index) const;
  unsigned __int64 get_size(UInt32 index) const;
  unsigned __int64 get_psize(UInt32 index) const;
  FILETIME get_ctime(UInt32 index) const;
  FILETIME get_mtime(UInt32 index) const;
  FILETIME get_atime(UInt32 index) const;
  unsigned get_crc(UInt32 index) const;

  // extract
private:
  wstring get_default_name() const;
  void prepare_dst_dir(const wstring& path);
  void prepare_test(UInt32 file_index, list<UInt32>& indices);
public:
  void extract(UInt32 src_dir_index, const vector<UInt32>& src_indices, const ExtractOptions& options, shared_ptr<ErrorLog> error_log);
  void test(UInt32 src_dir_index, const vector<UInt32>& src_indices);
  void delete_archive();

  // create & update archive
private:
  wstring get_temp_file_name() const;
  void set_properties(IOutArchive* out_arc, const UpdateOptions& options);
  void load_sfx_module(Buffer<char>& buffer, const UpdateOptions& options);
public:
  unsigned level;
  wstring method;
  bool solid;
  bool encrypted;
  wstring password;
  bool update_props_defined;
  bool has_crc;
  void load_update_props();
public:
  void create(const wstring& src_dir, const vector<wstring>& file_names, const UpdateOptions& options, shared_ptr<ErrorLog> error_log);
  void update(const wstring& src_dir, const vector<wstring>& file_names, const wstring& dst_dir, const UpdateOptions& options, shared_ptr<ErrorLog> error_log);
  void create_dir(const wstring& dir_name, const wstring& dst_dir);

  // delete files in archive
private:
  void enum_deleted_indices(UInt32 file_index, vector<UInt32>& indices);
public:
  void delete_files(const vector<UInt32>& src_indices);

  // attributes
private:
  void load_arc_attr();
public:
  AttrList arc_attr;
  AttrList get_attr_list(UInt32 item_index);

public:
  Archive(): update_props_defined(false) {
  }
};
