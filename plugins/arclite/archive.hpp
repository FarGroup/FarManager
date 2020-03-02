#pragma once

#include "comutils.hpp"

extern const ArcType c_7z;
extern const ArcType c_zip;
extern const ArcType c_bzip2;
extern const ArcType c_gzip;
extern const ArcType c_xz;
extern const ArcType c_iso;
extern const ArcType c_udf;
extern const ArcType c_rar;
extern const ArcType c_split;
extern const ArcType c_wim;
extern const ArcType c_tar;
extern const ArcType c_SWFc;
extern const ArcType c_dmg;
extern const ArcType c_hfs;

extern const wchar_t* c_method_copy;   // pseudo method

extern const wchar_t* c_method_lzma;   // standard 7z methods
extern const wchar_t* c_method_lzma2;  //
extern const wchar_t* c_method_ppmd;   //

extern const UInt64 c_min_volume_size;

extern const wchar_t* c_sfx_ext;
extern const wchar_t* c_volume_ext;

struct ICompressCodecsInfo;

struct ArcLib {
  HMODULE h_module;
  UInt64 version;
  std::wstring module_path;
  Func_CreateObject CreateObject;
  Func_GetNumberOfMethods GetNumberOfMethods;
  Func_GetMethodProperty GetMethodProperty;
  Func_GetNumberOfFormats GetNumberOfFormats;
  Func_GetHandlerProperty GetHandlerProperty;
  Func_GetHandlerProperty2 GetHandlerProperty2;
  Func_SetCodecs SetCodecs;
  Func_CreateDecoder CreateDecoder;
  Func_CreateEncoder CreateEncoder;
  Func_GetIsArc GetIsArc;
  ComObject<IHashers> ComHashers;

  HRESULT get_prop(UInt32 index, PROPID prop_id, PROPVARIANT* prop) const;
  HRESULT get_bool_prop(UInt32 index, PROPID prop_id, bool& value) const;
  HRESULT get_uint_prop(UInt32 index, PROPID prop_id, UInt32& value) const;
  HRESULT get_string_prop(UInt32 index, PROPID prop_id, std::wstring& value) const;
  HRESULT get_bytes_prop(UInt32 index, PROPID prop_id, ByteVector& value) const;
};

struct ArcFormat {
  std::wstring name;
  bool updatable;
  std::list<std::wstring> extension_list;
  std::map<std::wstring, std::wstring> nested_ext_mapping;
  std::wstring default_extension() const;

  UInt32 Flags;
  bool NewInterface;

  UInt32 SignatureOffset;
  std::vector<ByteVector> Signatures;

  int lib_index;
  UInt32 FormatIndex;
  ByteVector ClassID;

  Func_IsArc IsArc;

  bool Flags_KeepName() const { return (Flags & NArcInfoFlags::kKeepName) != 0; }
  bool Flags_FindSignature() const { return (Flags & NArcInfoFlags::kFindSignature) != 0; }

  bool Flags_AltStreams() const { return (Flags & NArcInfoFlags::kAltStreams) != 0; }
  bool Flags_NtSecure() const { return (Flags & NArcInfoFlags::kNtSecure) != 0; }
  bool Flags_SymLinks() const { return (Flags & NArcInfoFlags::kSymLinks) != 0; }
  bool Flags_HardLinks() const { return (Flags & NArcInfoFlags::kHardLinks) != 0; }

  bool Flags_UseGlobalOffset() const { return (Flags & NArcInfoFlags::kUseGlobalOffset) != 0; }
  bool Flags_StartOpen() const { return (Flags & NArcInfoFlags::kStartOpen) != 0; }
  bool Flags_BackwardOpen() const { return (Flags & NArcInfoFlags::kBackwardOpen) != 0; }
  bool Flags_PreArc() const { return (Flags & NArcInfoFlags::kPreArc) != 0; }
  bool Flags_PureStartOpen() const { return (Flags & NArcInfoFlags::kPureStartOpen) != 0; }

  ArcFormat() : updatable(false), Flags(0), NewInterface(false), lib_index(-1), FormatIndex(0), IsArc(nullptr) {}
};

typedef std::vector<ArcLib> ArcLibs;

struct CDllCodecInfo {
  unsigned LibIndex;
  UInt32 CodecIndex;
  UInt32 CodecId;
  bool EncoderIsAssigned;
  bool DecoderIsAssigned;
  CLSID Encoder;
  CLSID Decoder;
  std::wstring Name;
};
typedef std::vector<CDllCodecInfo> ArcCodecs;

struct CDllHasherInfo
{
  unsigned LibIndex;
  UInt32 HasherIndex;
};
typedef std::vector<CDllHasherInfo> ArcHashers;

class ArcFormats: public std::map<ArcType, ArcFormat> {
public:
  ArcTypes get_arc_types() const;
  ArcTypes find_by_name(const std::wstring& name) const;
  ArcTypes find_by_ext(const std::wstring& ext) const;
};

struct SfxModule {
  std::wstring path;
  std::wstring description() const;
  bool all_codecs() const;
  bool install_config() const;
};

class SfxModules: public std::vector<SfxModule> {
public:
  uintptr_t find_by_name(const std::wstring& name) const;
};

class MyCompressCodecsInfo;

class ArcAPI {
private:
  ArcLibs arc_libs;
  size_t n_base_format_libs;
  size_t n_format_libs;
  ArcCodecs arc_codecs;
  size_t n_7z_codecs;
  ArcHashers arc_hashers;
  ArcFormats arc_formats;
  SfxModules sfx_modules;
  static ArcAPI* arc_api;
  ArcAPI() { n_base_format_libs = n_format_libs = n_7z_codecs = 0; }
  ~ArcAPI();
  void load_libs(const std::wstring& path);
  void load_codecs(const std::wstring& path);
  void find_sfx_modules(const std::wstring& path);
  void load();
  static ArcAPI* get();
public:
  static const ArcLibs& libs() {
    return get()->arc_libs;
  }
  static const ArcFormats& formats() {
    return get()->arc_formats;
  }
  static const ArcCodecs& codecs() {
    return get()->arc_codecs;
  }
  static size_t Count7zCodecs() {
    return get()->n_7z_codecs;
  }
  static const ArcHashers& hashers() {
    return get()->arc_hashers;
  }

  static const SfxModules& sfx() {
    return get()->sfx_modules;
  }
  static void create_in_archive(const ArcType& arc_type, IInArchive** in_arc);
  static void create_out_archive(const ArcType& format, IOutArchive** out_arc);
  static void free();

  static bool is_single_file_format(const ArcType& arc_ty) {
    return arc_ty == c_bzip2 || arc_ty == c_gzip || arc_ty == c_xz || arc_ty == c_SWFc;
  }
};

struct ArcFileInfo {
  UInt32 parent;
  std::wstring name;
  bool is_dir;
  bool is_altstream;
  bool operator<(const ArcFileInfo& file_info) const;
};
typedef std::vector<ArcFileInfo> FileList;

const UInt32 c_root_index = -1;
const UInt32 c_dup_index = -2;

typedef std::vector<UInt32> FileIndex;
typedef std::pair<FileIndex::const_iterator, FileIndex::const_iterator> FileIndexRange;

struct ArcEntry {
  ArcType type;
  size_t sig_pos;
  ArcEntry(const ArcType& type, size_t sig_pos): type(type), sig_pos(sig_pos) {}
};

typedef std::list<ArcEntry> ArcEntries;

class ArcChain: public std::list<ArcEntry> {
public:
  std::wstring to_string() const;
};

class Archive;
typedef std::vector<std::shared_ptr<Archive>> Archives;

class Archive: public std::enable_shared_from_this<Archive> {
  // open
private:
  ComObject<IInArchive> in_arc;
  UInt32 error_flags, warning_flags;
  std::wstring error_text, warning_text;
  IInStream *base_stream;
  bool open(IInStream* in_stream, const ArcType& type);
  UInt64 get_physize();
  UInt64 archive_filesize();
  UInt64 get_skip_header(IInStream *stream, const ArcType& type);
  static ArcEntries detect(Byte *buffer, UInt32 size, bool eof, const std::wstring& file_ext, const ArcTypes& arc_types, IInStream *stream);
  static void open(const OpenOptions& options, Archives& archives);
public:
  std::shared_ptr<Archive> parent;
  static unsigned max_check_size;
  std::wstring arc_path;
  FindData arc_info;
  std::set<std::wstring> volume_names;
  ArcChain arc_chain;
  std::wstring arc_dir() const {
    return extract_file_path(arc_path);
  }
  std::wstring arc_name() const {
    std::wstring name = extract_file_name(arc_path);
    return name.empty() ? arc_path : name;
  }
  static std::unique_ptr<Archives> open(const OpenOptions& options);
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

  HRESULT copy_prologue(IOutStream *out_stream);

  // archive contents
public:
  UInt32 num_indices;
  FileList file_list;
  FileIndex file_list_index;
  void make_index();
  UInt32 find_dir(const std::wstring& dir);
  FileIndexRange get_dir_list(UInt32 dir_index);
  bool get_stream(UInt32 index, IInStream** stream);
  std::wstring get_path(UInt32 index);
  FindData get_file_info(UInt32 index);
  bool get_main_file(UInt32& index) const;
  DWORD get_attr(UInt32 index) const;
  UInt64 get_size(UInt32 index) const;
  UInt64 get_psize(UInt32 index) const;
  FILETIME get_ctime(UInt32 index) const;
  FILETIME get_mtime(UInt32 index) const;
  FILETIME get_atime(UInt32 index) const;
  unsigned get_crc(UInt32 index) const;
  bool get_anti(UInt32 index) const;
  bool get_isaltstream(UInt32 index) const;

  void read_open_results();
  std::list<std::wstring> get_open_errors() const;
  std::list<std::wstring> get_open_warnings() const;

  // extract
private:
  std::wstring get_default_name() const;
  void prepare_dst_dir(const std::wstring& path);
  void prepare_test(UInt32 file_index, std::list<UInt32>& indices);
public:
  void extract(UInt32 src_dir_index, const std::vector<UInt32>& src_indices, const ExtractOptions& options, std::shared_ptr<ErrorLog> error_log, std::vector<UInt32>* extracted_indices = nullptr);
  void test(UInt32 src_dir_index, const std::vector<UInt32>& src_indices);
  void delete_archive();

  // create & update archive
private:
  std::wstring get_temp_file_name() const;
  void set_properties(IOutArchive* out_arc, const UpdateOptions& options);
public:
  unsigned level;
  std::wstring method;
  bool solid;
  bool encrypted;
  std::wstring password;
  int open_password;
  bool update_props_defined;
  bool has_crc;
  void load_update_props();
public:
  void create(const std::wstring& src_dir, const std::vector<std::wstring>& file_names, const UpdateOptions& options, std::shared_ptr<ErrorLog> error_log);
  void update(const std::wstring& src_dir, const std::vector<std::wstring>& file_names, const std::wstring& dst_dir, const UpdateOptions& options, std::shared_ptr<ErrorLog> error_log);
  void create_dir(const std::wstring& dir_name, const std::wstring& dst_dir);

  // delete files in archive
private:
  void enum_deleted_indices(UInt32 file_index, std::vector<UInt32>& indices);
public:
  void delete_files(const std::vector<UInt32>& src_indices);

  // attributes
private:
  void load_arc_attr();
public:
  AttrList arc_attr;
  AttrList get_attr_list(UInt32 item_index);

public:
  Archive()
   : base_stream(nullptr), num_indices(0)
   , level(0), solid(false), encrypted(false), open_password(0), update_props_defined(false), has_crc(false)
  { error_flags = warning_flags = 0; }
};
