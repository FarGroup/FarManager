#include "msg.hpp"
#include "utils.hpp"
#include "sysutils.hpp"
#include "farutils.hpp"
#include "common.hpp"
#include "ui.hpp"
#include "msearch.hpp"
#include "archive.hpp"

OpenOptions::OpenOptions() : detect(false), open_password_len(nullptr), recursive_panel(false), delete_on_close('\0')
{}

class ArchiveSubStream : public IInStream, private ComBase {
private:
  ComObject<IInStream> base_stream;
  size_t start_offset;

public:
  ArchiveSubStream(IInStream* stream, size_t offset) : base_stream(stream), start_offset(offset) {
    base_stream->Seek(static_cast<Int64>(start_offset), STREAM_SEEK_SET, nullptr);
  }

  UNKNOWN_IMPL_BEGIN
  UNKNOWN_IMPL_ITF(ISequentialInStream)
  UNKNOWN_IMPL_ITF(IInStream)
  UNKNOWN_IMPL_END

  STDMETHODIMP Read(void *data, UInt32 size, UInt32 *processedSize) noexcept override {
    return base_stream->Read(data, size, processedSize);
  }

  STDMETHODIMP Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) noexcept override {
    if (seekOrigin == STREAM_SEEK_SET)
      offset += start_offset;
    UInt64 newPos = 0;
    auto res = base_stream->Seek(offset, seekOrigin, &newPos);
    if (res == S_OK && newPosition)
      *newPosition = newPos - start_offset;
    return res;
  }
};

class ArchiveOpenStream: public IInStream, private ComBase, private File {
private:
  bool device_file;
  UInt64 device_pos;
  UInt64 device_size;
  unsigned device_sector_size;

  Byte *cached_header;
  UInt32 cached_size;

  void check_device_file() {
    device_pos = 0;
    device_file = false;
    if (size_nt(device_size))
      return;

    PARTITION_INFORMATION part_info;
    if (io_control_out_nt(IOCTL_DISK_GET_PARTITION_INFO, part_info)) {
      device_size = part_info.PartitionLength.QuadPart;
      DWORD sectors_per_cluster, bytes_per_sector, number_of_free_clusters, total_number_of_clusters;
      if (GetDiskFreeSpaceW(add_trailing_slash(path()).c_str(), &sectors_per_cluster, &bytes_per_sector, &number_of_free_clusters, &total_number_of_clusters))
        device_sector_size = bytes_per_sector;
      else
        device_sector_size = 4096;
      device_file = true;
      return;
    }

    DISK_GEOMETRY disk_geometry;
    if (io_control_out_nt(IOCTL_DISK_GET_DRIVE_GEOMETRY, disk_geometry)) {
      device_size = disk_geometry.Cylinders.QuadPart * disk_geometry.TracksPerCylinder * disk_geometry.SectorsPerTrack * disk_geometry.BytesPerSector;
      device_sector_size = disk_geometry.BytesPerSector;
      device_file = true;
      return;
    }
  }
public:
  ArchiveOpenStream(const std::wstring& file_path) {
    open(file_path, FILE_READ_DATA | FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
    check_device_file();
    cached_header = nullptr;
    cached_size = 0;
  }

  void CacheHeader(Byte *buffer, UInt32 size) {
    if (!device_file) {
      cached_header = buffer;
      cached_size = size;
    }
  }

  UNKNOWN_IMPL_BEGIN
  UNKNOWN_IMPL_ITF(ISequentialInStream)
  UNKNOWN_IMPL_ITF(IInStream)
  UNKNOWN_IMPL_END

  STDMETHODIMP Read(void *data, UInt32 size, UInt32 *processedSize) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    if (processedSize)
      *processedSize = 0;
    unsigned size_read;
    if (device_file) {
      UInt64 aligned_pos = device_pos / device_sector_size * device_sector_size;
      unsigned aligned_offset = static_cast<unsigned>(device_pos - aligned_pos);
      unsigned aligned_size = aligned_offset + size;
      aligned_size = (aligned_size / device_sector_size + (aligned_size % device_sector_size ? 1 : 0)) * device_sector_size;
      Buffer<unsigned char> buffer(aligned_size + device_sector_size);
      const auto buffer_addr = reinterpret_cast<ptrdiff_t>(buffer.data());
      unsigned char* aligned_buffer = reinterpret_cast<unsigned char*>(buffer_addr % device_sector_size ? (buffer_addr / device_sector_size + 1) * device_sector_size : buffer_addr);
      set_pos(aligned_pos, FILE_BEGIN);
      size_read = static_cast<unsigned>(read(aligned_buffer, aligned_size));
      if (size_read < aligned_offset)
        size_read = 0;
      else
        size_read -= aligned_offset;
      if (size_read > size)
        size_read = size;
      device_pos += size_read;
      std::memcpy(data, aligned_buffer + aligned_offset, size_read);
    }
    else {
      size_read = 0;
      if (cached_size) {
        auto cur_pos = set_pos(0, FILE_CURRENT);
        if (cur_pos < cached_size) {
          UInt32 off = static_cast<UInt32>(cur_pos);
          UInt32 siz = std::min(size, cached_size - off);
          std::memcpy(data, cached_header + off, siz);
          size -= (size_read = siz);
          data = static_cast<void *>(static_cast<Byte *>(data)+siz);
          set_pos(static_cast<Int64>(siz), FILE_CURRENT);
        }
      }
      if (size > 0)
        size_read += static_cast<unsigned>(read(data, size));
    }
    if (processedSize)
      *processedSize = size_read;
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    if (newPosition)
      *newPosition = 0;
    UInt64 new_position;
    if (device_file) {
      switch (seekOrigin) {
      case STREAM_SEEK_SET:
        device_pos = offset; break;
      case STREAM_SEEK_CUR:
        device_pos += offset; break;
      case STREAM_SEEK_END:
        device_pos = device_size + offset; break;
      default:
        FAIL(E_INVALIDARG);
      }
      new_position = device_pos;
    }
    else {
      new_position = set_pos(offset, translate_seek_method(seekOrigin));
    }
    if (newPosition)
      *newPosition = new_position;
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  FindData get_info() {
    FindData file_info{};
    std::wstring file_name = extract_file_name(path());
    if (file_name.empty())
      file_name = path();
    std::wcscpy(file_info.cFileName, file_name.c_str());
    BY_HANDLE_FILE_INFORMATION fi;
    if (get_info_nt(fi)) {
      file_info.dwFileAttributes = fi.dwFileAttributes;
      file_info.ftCreationTime = fi.ftCreationTime;
      file_info.ftLastAccessTime = fi.ftLastAccessTime;
      file_info.ftLastWriteTime = fi.ftLastWriteTime;
      file_info.nFileSizeLow = fi.nFileSizeLow;
      file_info.nFileSizeHigh = fi.nFileSizeHigh;
    }
    return file_info;
  }
};


class ArchiveOpener: public IArchiveOpenCallback, public IArchiveOpenVolumeCallback, public ICryptoGetTextPassword, public ComBase, public ProgressMonitor {
private:
  std::shared_ptr<Archive> archive;
  FindData volume_file_info;

  UInt64 total_files;
  UInt64 total_bytes;
  UInt64 completed_files;
  UInt64 completed_bytes;

  void do_update_ui() override {
    const unsigned c_width = 60;
    std::wostringstream st;
    st << fit_str(volume_file_info.cFileName, c_width) << L'\n';
    st << completed_files << L" / " << total_files << L'\n';
    st << Far::get_progress_bar_str(c_width, completed_files, total_files) << L'\n';
    st << L"\x01\n";
    st << format_data_size(completed_bytes, get_size_suffixes()) << L" / " << format_data_size(total_bytes, get_size_suffixes()) << L'\n';
    st << Far::get_progress_bar_str(c_width, completed_bytes, total_bytes) << L'\n';
    progress_text = st.str();

    if (total_files)
      percent_done = calc_percent(completed_files, total_files);
    else
      percent_done = calc_percent(completed_bytes, total_bytes);
  }

public:
  ArchiveOpener(std::shared_ptr<Archive> archive): ProgressMonitor(Far::get_msg(MSG_PROGRESS_OPEN)), archive(archive), volume_file_info(archive->arc_info), total_files(0), total_bytes(0), completed_files(0), completed_bytes(0) {
  }

  UNKNOWN_IMPL_BEGIN
  UNKNOWN_IMPL_ITF(IArchiveOpenCallback)
  UNKNOWN_IMPL_ITF(IArchiveOpenVolumeCallback)
  UNKNOWN_IMPL_ITF(ICryptoGetTextPassword)
  UNKNOWN_IMPL_END

  STDMETHODIMP SetTotal(const UInt64 *files, const UInt64 *bytes) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    if (files) total_files = *files;
    if (bytes) total_bytes = *bytes;
    update_ui();
    return S_OK;
    COM_ERROR_HANDLER_END
  }
  STDMETHODIMP SetCompleted(const UInt64 *files, const UInt64 *bytes) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    if (files) completed_files = *files;
    if (bytes) completed_bytes = *bytes;
    update_ui();
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP GetProperty(PROPID propID, PROPVARIANT *value) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    PropVariant prop;
    switch (propID) {
    case kpidName:
      prop = volume_file_info.cFileName; break;
    case kpidIsDir:
      prop = volume_file_info.is_dir(); break;
    case kpidSize:
      prop = volume_file_info.size(); break;
    case kpidAttrib:
      prop = static_cast<UInt32>(volume_file_info.dwFileAttributes); break;
    case kpidCTime:
      prop = volume_file_info.ftCreationTime; break;
    case kpidATime:
      prop = volume_file_info.ftLastAccessTime; break;
    case kpidMTime:
      prop = volume_file_info.ftLastWriteTime; break;
    }
    prop.detach(value);
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP GetStream(const wchar_t *name, IInStream **inStream) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    std::wstring file_path = add_trailing_slash(archive->arc_dir()) + name;
    FindData find_data;
    if (!File::get_find_data_nt(file_path, find_data))
      return S_FALSE;
    if (find_data.is_dir())
      return S_FALSE;
    archive->volume_names.insert(name);
    volume_file_info = find_data;
    ComObject<IInStream> file_stream(new ArchiveOpenStream(file_path));
    file_stream.detach(inStream);
    update_ui();
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP CryptoGetTextPassword(BSTR *password) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    if (archive->m_password.empty()) {
      if (archive->m_open_password == -'A') // open from AnalyzeW
        FAIL(E_PENDING);
      ProgressSuspend ps(*this);
      if (!password_dialog(archive->m_password, archive->arc_path)) {
        archive->m_open_password = -3;
        FAIL(E_ABORT);
      }
      else {
        archive->m_open_password = static_cast<int>(archive->m_password.size());
      }
    }
    BStr(archive->m_password).detach(password);
    return S_OK;
    COM_ERROR_HANDLER_END
  }
};


bool Archive::get_stream(UInt32 index, IInStream** stream) {
  UInt32 num_indices = 0;
  if (in_arc->GetNumberOfItems(&num_indices) != S_OK)
    return false;
  if (index >= num_indices)
    return false;

  ComObject<IInArchiveGetStream> get_stream;
  if (in_arc->QueryInterface(IID_IInArchiveGetStream, reinterpret_cast<void**>(&get_stream)) != S_OK || !get_stream)
    return false;

  ComObject<ISequentialInStream> seq_stream;
  if (get_stream->GetStream(index, seq_stream.ref()) != S_OK || !seq_stream)
    return false;

  if (seq_stream->QueryInterface(IID_IInStream, reinterpret_cast<void**>(stream)) != S_OK || !stream)
    return false;

  return true;
}

HRESULT Archive::copy_prologue(IOutStream *out_stream)
{
  auto prologue_size = arc_chain.back().sig_pos;
  if (prologue_size <= 0)
    return S_OK;

  if (!base_stream)
    return E_FAIL;

  auto res = base_stream->Seek(0, STREAM_SEEK_SET, nullptr);
  if (res != S_OK)
    return res;

  while (prologue_size > 0) {
    char buf[16 * 1024];
    UInt32 nr = 0, nw = 0, nb = static_cast<UInt32>(sizeof(buf));
    if (prologue_size < nb) nb = static_cast<UInt32>(prologue_size);
      res = base_stream->Read(buf, nb, &nr);
    if (res != S_OK || nr == 0)
      break;
    res = out_stream->Write(buf, nr, &nw);
    if (res != S_OK || nr != nw)
      break;
    prologue_size -= nr;
  }

  if (res == S_OK && prologue_size > 0)
    res = E_FAIL;
  return res;
}

bool Archive::open(IInStream* stream, const ArcType& type, const bool allow_tail) {
  ArcAPI::create_in_archive(type, in_arc.ref());
  ComObject<IArchiveOpenCallback> opener(new ArchiveOpener(shared_from_this()));
  if (allow_tail && ArcAPI::formats().at(type).Flags_PreArc())  {
    ComObject<IArchiveAllowTail> allowTail;
    in_arc->QueryInterface(IID_IArchiveAllowTail, (void**)&allowTail);
    if (allowTail)
      allowTail->AllowTail(TRUE);
  }
  const UInt64 max_check_start_position = 0;
  auto res = in_arc->Open(stream, &max_check_start_position, opener);
  if (res == HRESULT_FROM_WIN32(ERROR_INVALID_DATA)) // unfriendly eDecode
	  res = S_FALSE;
  COM_ERROR_CHECK(res);
  return res == S_OK;
}

static void prioritize(std::list<ArcEntry>& arc_entries, const ArcType& first, const ArcType& second) {
  std::list<ArcEntry>::iterator iter = arc_entries.end();
  for (std::list<ArcEntry>::iterator arc_entry = arc_entries.begin(); arc_entry != arc_entries.end(); ++arc_entry) {
    if (arc_entry->type == second) {
      iter = arc_entry;
    }
    else if (arc_entry->type == first) {
      if (iter != arc_entries.end()) {
        arc_entries.insert(iter, *arc_entry);
        arc_entries.erase(arc_entry);
      }
      break;
    }
  }
}

//??? filter multi-volume .zip archives to avoid wrong opening.
//??? if someone can find better solution - welcome...
//
static Byte zip_LOCAL_sig[] = { 0x50, 0x4B, 0x03, 0x04 };
static const std::string_view zip_EOCD_sig = "\x50\x4B\x05\x06"sv;
static const UInt32 check_size = 16 * 1024, min_LOCAL = 30, min_EOCD = 22;
//
static bool accepted_signature(
  size_t pos, const SigData& sig, const Byte *buffer, size_t size, IInStream* stream, int eof_i)
{
  if (!pos
   || sig.signature.size() != sizeof(zip_LOCAL_sig)
   || !std::equal(zip_LOCAL_sig, zip_LOCAL_sig+ sizeof(zip_LOCAL_sig), buffer+pos)
  ) return true;
  if (eof_i < 0)
    return false;

  std::unique_ptr<Byte[]> buf;
  std::string_view tail;
  if (eof_i) {
    pos += min_LOCAL;
    size -= (min_EOCD - zip_EOCD_sig.size());
    if (pos >= size)
      return true;
    if (pos + check_size < size)
      pos = size - check_size;
    tail = { (const char*)buffer + pos, size - pos };
  }
  else {
    buf = std::make_unique<Byte[]>(check_size);
    pos = 0;
    buffer = buf.get();
    UInt64 cur_pos;
    if (S_OK != stream->Seek(0, STREAM_SEEK_CUR, &cur_pos))
      return false;
    if (S_OK == stream->Seek(-(Int64)check_size, STREAM_SEEK_END, nullptr)) {
      UInt32 nr;
      if (S_OK == stream->Read(buf.get(), check_size, &nr) && nr == check_size)
        tail = { (const char*)buffer, check_size - min_EOCD + zip_EOCD_sig.size() };
    }
    stream->Seek((Int64)cur_pos, STREAM_SEEK_SET, nullptr);
  }
  if (!tail.empty()) {
    auto eocd = tail.rfind(zip_EOCD_sig);
    if (eocd != std::string_view::npos) {
      pos += eocd + zip_EOCD_sig.size();
      if (buffer[pos] != 0 || buffer[pos + 1] != 0) // This disk (aka Volume) number
        return false;
    }
  }
  return true;
}

ArcEntries Archive::detect(
  Byte *buffer, UInt32 size, bool eof, const std::wstring& file_ext, const ArcTypes& arc_types, IInStream *stream)
{
  ArcEntries arc_entries;
  std::set<ArcType> found_types;

  // 1. find formats by signature
  //
  std::vector<SigData> signatures;
  signatures.reserve(2 * arc_types.size());
  std::for_each(arc_types.begin(), arc_types.end(), [&] (const ArcType& arc_type) {
    const auto& format = ArcAPI::formats().at(arc_type);
    if (format.ClassID != c_dmg) {
      std::for_each(format.Signatures.begin(), format.Signatures.end(), [&](const ByteVector& signature) {
        signatures.emplace_back(SigData(signature, format));
      });
    }
  });
  std::vector<StrPos> sig_positions = msearch(buffer, size, signatures, eof);

  int eof_i = eof ? 1 : 0;
  std::for_each(sig_positions.begin(), sig_positions.end(), [&] (const StrPos& sig_pos) {
    const auto& signature = signatures[sig_pos.idx];
    const auto& format = signature.format;
    if (accepted_signature(sig_pos.pos, signature, buffer, size, stream, eof_i)) {
      found_types.insert(format.ClassID);
      arc_entries.emplace_back(format.ClassID, sig_pos.pos - format.SignatureOffset);
    }
    else {
      eof_i = -1;
    }
  });

  // 2. find formats by file extension
  //
  ArcTypes types_by_ext = ArcAPI::formats().find_by_ext(file_ext);
  std::for_each(types_by_ext.begin(), types_by_ext.end(), [&] (const ArcType& arc_type) {
    if (found_types.count(arc_type) == 0 && std::find(arc_types.begin(), arc_types.end(), arc_type) != arc_types.end()) {
      found_types.insert(arc_type);
      arc_entries.emplace_front(arc_type, 0);
    }
  });

  // 3. all other formats
  //
  std::for_each(arc_types.begin(), arc_types.end(), [&] (const ArcType& arc_type) {
    if (found_types.count(arc_type) == 0) {
      const auto& format = ArcAPI::formats().at(arc_type);
      if (!format.Flags_ByExtOnlyOpen())
        arc_entries.emplace_back(arc_type, 0);
    }
  });

  // special case: UDF must go before ISO
  prioritize(arc_entries, c_udf, c_iso);
  // special case: Rar must go before Split
  prioritize(arc_entries, c_rar, c_split);
  // special case: Dmg must go before HFS
  prioritize(arc_entries, c_dmg, c_hfs);

  return arc_entries;
}

UInt64 Archive::get_physize()
{
  UInt64 physize = 0;
  PropVariant prop;
  auto res = in_arc->GetArchiveProperty(kpidPhySize, prop.ref());
  if (res == S_OK && prop.is_uint())
    physize = prop.get_uint();
  return physize;
}

UInt64 Archive::archive_filesize()
{
  auto arc_size = arc_info.size();
#if 0
  for (const auto& volume_name : volume_names) {
    auto volume_path = add_trailing_slash(arc_dir()) + volume_name;
    FindData find_data;
    if (File::get_find_data_nt(volume_path, find_data))
      arc_size += find_data.size();
  }
#endif
  return arc_size;
}

UInt64 Archive::get_skip_header(IInStream *stream, const ArcType& type)
{
  if (ArcAPI::formats().at(type).Flags_PreArc()) {
    ComObject<IArchiveAllowTail> allowTail;
    in_arc->QueryInterface(IID_IArchiveAllowTail, (void **)&allowTail);
    if (allowTail)
      allowTail->AllowTail(TRUE);
  }

  auto res = stream->Seek(0, STREAM_SEEK_SET, nullptr);
  if (S_OK == res) {
    const UInt64 max_check_start_position = max_check_size;
    res = in_arc->Open(stream, &max_check_start_position, nullptr);
    if (res == S_OK) {
      auto physize = get_physize();
      if (physize < arc_info.size())
        return physize;
    }
  }
  return 0;
}

void Archive::open(const OpenOptions& options, Archives& archives) {
  size_t parent_idx = -1;
  if (!archives.empty())
    parent_idx = archives.size() - 1;

  ArchiveOpenStream* stream_impl = nullptr;
  ComObject<IInStream> stream;
  FindData arc_info{};
  if (parent_idx == (size_t)-1) {
    stream_impl = new ArchiveOpenStream(options.arc_path);
    stream = stream_impl;
    arc_info = stream_impl->get_info();
  }
  else {
    UInt32 main_file;
    if (!archives[parent_idx]->get_main_file(main_file))
      return;
    if (!archives[parent_idx]->get_stream(main_file, stream.ref()))
      return;
    arc_info = archives[parent_idx]->get_file_info(main_file);
  }

  Buffer<unsigned char> buffer(max_check_size);
  UInt32 size;
  CHECK_COM(stream->Read(buffer.data(), static_cast<UInt32>(buffer.size()), &size));
  if (stream_impl)
    stream_impl->CacheHeader(buffer.data(), size);

  UInt64 skip_header = 0;
  bool first_open = true;
  ArcEntries arc_entries = detect(buffer.data(), size, size < max_check_size, extract_file_ext(arc_info.cFileName), options.arc_types, stream);

  for (ArcEntries::const_iterator arc_entry = arc_entries.cbegin(); arc_entry != arc_entries.cend(); ++arc_entry) {
    const auto archive = std::make_shared<Archive>();
    if (options.open_password_len && *options.open_password_len == -'A')
      archive->m_open_password = *options.open_password_len;
    archive->arc_path = options.arc_path;
    archive->arc_info = arc_info;
    archive->m_password = options.password;
    if (parent_idx != (size_t)-1)
      archive->volume_names = archives[parent_idx]->volume_names;

    const auto& format = ArcAPI::formats().at(arc_entry->type);
    bool opened = false, have_tail = false;
    CHECK_COM(stream->Seek(arc_entry->sig_pos, STREAM_SEEK_SET, nullptr));
    if (!arc_entry->sig_pos) {
      opened = archive->open(stream, arc_entry->type);
      if (archive->m_open_password && options.open_password_len)
        *options.open_password_len = archive->m_open_password;
      if (!opened && first_open) {
        if (format.Flags_PreArc()) {
           stream->Seek(0, STREAM_SEEK_SET, nullptr);
           opened = have_tail = archive->open(stream, arc_entry->type, true);
        }
        if (!opened) {
          auto next_entry = arc_entry;
          ++next_entry;
          if (next_entry != arc_entries.cend() && next_entry->sig_pos > 0) {
            skip_header = archive->get_skip_header(stream, arc_entry->type);
          }
		  }
      }
    }
    else if (arc_entry->sig_pos >= skip_header) {
      archive->arc_info.set_size(arc_info.size() - arc_entry->sig_pos);
      ComObject<IInStream> substream(new ArchiveSubStream(stream, arc_entry->sig_pos));
      opened = archive->open(substream, arc_entry->type);
      if (archive->m_open_password && options.open_password_len)
        *options.open_password_len = archive->m_open_password;
      if (opened)
        archive->base_stream = stream;
    }
    if (opened) {
      if (parent_idx != (size_t)-1) {
        archive->parent = archives[parent_idx];
        archive->arc_chain.assign(archives[parent_idx]->arc_chain.begin(), archives[parent_idx]->arc_chain.end());
      }
      archive->arc_chain.push_back(*arc_entry);

      archives.push_back(archive);
      open(options, archives);

      if (!options.detect && !have_tail)
        break;
      skip_header = arc_entry->sig_pos + std::min(archive->arc_info.size(), archive->get_physize());
    }
    first_open = false;
  }

  if (stream_impl)
    stream_impl->CacheHeader(nullptr, 0);
}

std::unique_ptr<Archives> Archive::open(const OpenOptions& options) {
  auto archives = std::make_unique<Archives>();
  open(options, *archives);
  if (!options.detect && !archives->empty())
    archives->erase(archives->begin(), archives->end() - 1);
  return archives;
}

void Archive::reopen() {
  assert(!in_arc);
  volume_names.clear();
  ArchiveOpenStream* stream_impl = new ArchiveOpenStream(arc_path);
  ComObject<IInStream> stream(stream_impl);
  arc_info = stream_impl->get_info();
  ArcChain::const_iterator arc_entry = arc_chain.begin();
  if (arc_entry->sig_pos > 0) {
    auto opened = open(new ArchiveSubStream(stream, arc_entry->sig_pos), arc_entry->type);
    if (opened)
      base_stream = stream;
    else
      FAIL(E_FAIL);
  }
  else {
    if (!open(stream, arc_entry->type))
      FAIL(E_FAIL);
  }
  ++arc_entry;
  while (arc_entry != arc_chain.end()) {
    UInt32 main_file;
    CHECK(get_main_file(main_file));
    ComObject<IInStream> sub_stream;
    CHECK(get_stream(main_file, sub_stream.ref()));
    arc_info = get_file_info(main_file);
    sub_stream->Seek(arc_entry->sig_pos, STREAM_SEEK_SET, nullptr);
    CHECK(open(sub_stream, arc_entry->type));
    ++arc_entry;
  }
}

void Archive::close() {
  base_stream = nullptr;
  if (in_arc) {
    in_arc->Close();
    in_arc.Release();
  }
  file_list.clear();
  file_list_index.clear();
}
