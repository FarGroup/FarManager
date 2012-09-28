#include "msg.h"
#include "utils.hpp"
#include "sysutils.hpp"
#include "farutils.hpp"
#include "common.hpp"
#include "ui.hpp"
#include "msearch.hpp"
#include "archive.hpp"

OpenOptions::OpenOptions(): detect(false) {
}

class ArchiveOpenStream: public IInStream, private ComBase, private File {
private:
  bool device_file;
  unsigned __int64 device_pos;
  unsigned __int64 device_size;
  unsigned device_sector_size;

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
  ArchiveOpenStream(const wstring& file_path) {
    open(file_path, FILE_READ_DATA | FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
    check_device_file();
  }

  UNKNOWN_IMPL_BEGIN
  UNKNOWN_IMPL_ITF(ISequentialInStream)
  UNKNOWN_IMPL_ITF(IInStream)
  UNKNOWN_IMPL_END

  STDMETHODIMP Read(void *data, UInt32 size, UInt32 *processedSize) {
    COM_ERROR_HANDLER_BEGIN
    if (processedSize)
      *processedSize = 0;
    unsigned size_read;
    if (device_file) {
      unsigned __int64 aligned_pos = device_pos / device_sector_size * device_sector_size;
      unsigned aligned_offset = static_cast<unsigned>(device_pos - aligned_pos);
      unsigned aligned_size = aligned_offset + size;
      aligned_size = (aligned_size / device_sector_size + (aligned_size % device_sector_size ? 1 : 0)) * device_sector_size;
      Buffer<unsigned char> buffer(aligned_size + device_sector_size);
      ptrdiff_t buffer_addr = buffer.data() - static_cast<unsigned char*>(0);
      unsigned char* alligned_buffer = reinterpret_cast<unsigned char*>(buffer_addr % device_sector_size ? (buffer_addr / device_sector_size + 1) * device_sector_size : buffer_addr);
      set_pos(aligned_pos, FILE_BEGIN);
      size_read = static_cast<unsigned>(read(alligned_buffer, aligned_size));
      if (size_read < aligned_offset)
        size_read = 0;
      else
        size_read -= aligned_offset;
      if (size_read > size)
        size_read = size;
      device_pos += size_read;
      memcpy(data, alligned_buffer + aligned_offset, size_read);
    }
    else {
      size_read = static_cast<unsigned>(read(data, size));
    }
    if (processedSize)
      *processedSize = size_read;
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) {
    COM_ERROR_HANDLER_BEGIN
    if (newPosition)
      *newPosition = 0;
    unsigned __int64 new_position;
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
    FindData file_info;
    memzero(file_info);
    wstring file_name = extract_file_name(path());
    if (file_name.empty())
      file_name = path();
    wcscpy(file_info.cFileName, file_name.c_str());
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
  shared_ptr<Archive> archive;
  FindData volume_file_info;

  UInt64 total_files;
  UInt64 total_bytes;
  UInt64 completed_files;
  UInt64 completed_bytes;

  virtual void do_update_ui() {
    const unsigned c_width = 60;
    wostringstream st;
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
  ArchiveOpener(shared_ptr<Archive> archive): ProgressMonitor(Far::get_msg(MSG_PROGRESS_OPEN)), archive(archive), volume_file_info(archive->arc_info), total_files(0), total_bytes(0), completed_files(0), completed_bytes(0) {
  }

  UNKNOWN_IMPL_BEGIN
  UNKNOWN_IMPL_ITF(IArchiveOpenCallback)
  UNKNOWN_IMPL_ITF(IArchiveOpenVolumeCallback)
  UNKNOWN_IMPL_ITF(ICryptoGetTextPassword)
  UNKNOWN_IMPL_END

  STDMETHODIMP SetTotal(const UInt64 *files, const UInt64 *bytes) {
    COM_ERROR_HANDLER_BEGIN
    if (files) total_files = *files;
    if (bytes) total_bytes = *bytes;
    update_ui();
    return S_OK;
    COM_ERROR_HANDLER_END
  }
  STDMETHODIMP SetCompleted(const UInt64 *files, const UInt64 *bytes) {
    COM_ERROR_HANDLER_BEGIN
    if (files) completed_files = *files;
    if (bytes) completed_bytes = *bytes;
    update_ui();
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP GetProperty(PROPID propID, PROPVARIANT *value) {
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

  STDMETHODIMP GetStream(const wchar_t *name, IInStream **inStream) {
    COM_ERROR_HANDLER_BEGIN
    wstring file_path = add_trailing_slash(archive->arc_dir()) + name;
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

  STDMETHODIMP CryptoGetTextPassword(BSTR *password) {
    COM_ERROR_HANDLER_BEGIN
    if (archive->password.empty()) {
      ProgressSuspend ps(*this);
      if (!password_dialog(archive->password, archive->arc_path))
        FAIL(E_ABORT);
    }
    BStr(archive->password).detach(password);
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

bool Archive::open(IInStream* stream, const ArcType& type) {
  ArcAPI::create_in_archive(type, in_arc.ref());
  CHECK_COM(stream->Seek(0, STREAM_SEEK_SET, nullptr));
  ComObject<IArchiveOpenCallback> opener(new ArchiveOpener(shared_from_this()));
  const UInt64 max_check_start_position = max_check_size;
  HRESULT res;
  COM_ERROR_CHECK(res = in_arc->Open(stream, &max_check_start_position, opener));
  return res == S_OK;
}

void prioritize(list<ArcEntry>& arc_entries, const ArcType& first, const ArcType& second) {
  list<ArcEntry>::iterator iter = arc_entries.end();
  for (list<ArcEntry>::iterator arc_entry = arc_entries.begin(); arc_entry != arc_entries.end(); arc_entry++) {
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

ArcEntries Archive::detect(IInStream* stream, const wstring& file_ext, const ArcTypes& arc_types) {
  ArcEntries arc_entries;
  set<ArcType> found_types;

  // 1. find formats by signature
  vector<ByteVector> signatures;
  signatures.reserve(arc_types.size());
  vector<ArcType> sig_types;
  sig_types.reserve(arc_types.size());
  for_each(arc_types.begin(), arc_types.end(), [&] (const ArcType& arc_type) {
    if (!ArcAPI::formats().at(arc_type).start_signature.empty()) {
      sig_types.push_back(arc_type);
      signatures.push_back(ArcAPI::formats().at(arc_type).start_signature);
    }
  });

  Buffer<unsigned char> buffer(max_check_size);
  UInt32 size;
  CHECK_COM(stream->Read(buffer.data(), static_cast<UInt32>(buffer.size()), &size));

  vector<StrPos> sig_positions = msearch(buffer.data(), size, signatures);

  for_each(sig_positions.begin(), sig_positions.end(), [&] (const StrPos& sig_pos) {
    found_types.insert(sig_types[sig_pos.idx]);
    arc_entries.push_back(ArcEntry(sig_types[sig_pos.idx], sig_pos.pos));
  });

  // 2. find formats by file extension
  ArcTypes types_by_ext = ArcAPI::formats().find_by_ext(file_ext);
  for_each(types_by_ext.begin(), types_by_ext.end(), [&] (const ArcType& arc_type) {
    if (found_types.count(arc_type) == 0 && find(arc_types.begin(), arc_types.end(), arc_type) != arc_types.end()) {
      found_types.insert(arc_type);
      arc_entries.push_front(ArcEntry(arc_type, 0));
    }
  });

  // 3. all other formats
  for_each(arc_types.begin(), arc_types.end(), [&] (const ArcType& arc_type) {
    if (found_types.count(arc_type) == 0) {
      arc_entries.push_back(ArcEntry(arc_type, 0));
    }
  });

  // special case: UDF must go before ISO
  prioritize(arc_entries, c_udf, c_iso);
  // special case: Rar must go before Split
  prioritize(arc_entries, c_rar, c_split);

  return arc_entries;
}

void Archive::open(const OpenOptions& options, Archives& archives) {
  size_t parent_idx = -1;
  if (!archives.empty())
    parent_idx = archives.size() - 1;

  ComObject<IInStream> stream;
  FindData arc_info;
  memzero(arc_info);
  if (parent_idx == -1) {
    ArchiveOpenStream* stream_impl = new ArchiveOpenStream(options.arc_path);
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

  ArcEntries arc_entries = detect(stream, extract_file_ext(arc_info.cFileName), options.arc_types);
  for (ArcEntries::const_iterator arc_entry = arc_entries.begin(); arc_entry != arc_entries.end(); arc_entry++) {
    shared_ptr<Archive> archive(new Archive());
    archive->arc_path = options.arc_path;
    archive->arc_info = arc_info;
    archive->password = options.password;
    if (parent_idx != -1)
      archive->volume_names = archives[parent_idx]->volume_names;
    if (archive->open(stream, arc_entry->type)) {
      if (parent_idx != -1)
        archive->arc_chain.assign(archives[parent_idx]->arc_chain.begin(), archives[parent_idx]->arc_chain.end());
      archive->arc_chain.push_back(*arc_entry);
      archives.push_back(archive);
      open(options, archives);
      if (!options.detect) break;
    }
  }
}

unique_ptr<Archives> Archive::open(const OpenOptions& options) {
  unique_ptr<Archives> archives(new Archives());
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
  if (!open(stream, arc_entry->type))
    FAIL(E_FAIL);
  arc_entry++;
  while (arc_entry != arc_chain.end()) {
    UInt32 main_file;
    CHECK(get_main_file(main_file));
    ComObject<IInStream> sub_stream;
    CHECK(get_stream(main_file, sub_stream.ref()))
    arc_info = get_file_info(main_file);
    CHECK(open(sub_stream, arc_entry->type));
    arc_entry++;
  }
}

void Archive::close() {
  if (in_arc) {
    in_arc->Close();
    in_arc.Release();
  }
  file_list.clear();
  file_list_index.clear();
}
