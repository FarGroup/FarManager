#include "msg.hpp"
#include "error.hpp"
#include "utils.hpp"
#include "farutils.hpp"
#include "sysutils.hpp"
#include "common.hpp"
#include "comutils.hpp"
#include "ui.hpp"
#include "archive.hpp"
#include "options.hpp"

static std::wstring uint_to_hex_str(UInt64 val, unsigned num_digits = 0) {
  wchar_t str[16];
  unsigned pos = 16;
  do {
    unsigned d = static_cast<unsigned>(val % 16);
    pos--;
    str[pos] = d < 10 ? d + L'0' : d - 10 + L'A';
    val /= 16;
  }
  while (val);
  if (num_digits) {
    while (pos + num_digits > 16) {
      pos--;
      str[pos] = L'0';
    }
  }
  return std::wstring(str + pos, 16 - pos);
}

static std::wstring format_str_prop(const PropVariant& prop) {
  std::wstring str = prop.get_str();
  for (unsigned i = 0; i < str.size(); i++)
    if (str[i] == L'\r' || str[i] == L'\n')
      str[i] = L' ';
  return str;
}

static std::wstring format_int_prop(const PropVariant& prop) {
  wchar_t buf[32];
  return std::wstring(_i64tow(prop.get_int(), buf, 10));
}

static std::wstring format_uint_prop(const PropVariant& prop) {
  wchar_t buf[32];
  return std::wstring(_ui64tow(prop.get_uint(), buf, 10));
}

static std::wstring format_size_prop(const PropVariant& prop) {
  if (!prop.is_uint())
    return std::wstring();
  std::wstring short_size = format_data_size(prop.get_uint(), get_size_suffixes());
  std::wstring long_size = format_uint_prop(prop);
  if (short_size == long_size)
    return short_size;
  else
    return short_size + L" = " + long_size;
}

static std::wstring format_filetime_prop(const PropVariant& prop) {
  if (!prop.is_filetime())
    return std::wstring();
  FILETIME prop_file_time = prop.get_filetime();
  FILETIME local_file_time;
  if (!FileTimeToLocalFileTime(&prop_file_time, &local_file_time))
    return std::wstring();
  SYSTEMTIME sys_time;
  if (!FileTimeToSystemTime(&local_file_time, &sys_time))
    return std::wstring();
  wchar_t buf[64];
  if (GetDateFormatW(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &sys_time, nullptr, buf, ARRAYSIZE(buf)) == 0)
    return std::wstring();
  std::wstring date_time(buf);
  if (GetTimeFormatW(LOCALE_USER_DEFAULT, 0, &sys_time, nullptr, buf, ARRAYSIZE(buf)) == 0)
    return std::wstring();
  date_time = date_time + L" " + buf;
  return date_time;
}

static std::wstring format_crc_prop(const PropVariant& prop) {
  if (!prop.is_uint())
    return std::wstring();
  return uint_to_hex_str(prop.get_uint(), prop.get_int_size() * 2);
}

static const wchar_t kPosixTypes[16 + 1] = L"0pc3d5b7-9lBsDEF";
#define ATTR_CHAR(a, n, c) (((a) & (1 << (n))) ? c : L'-')

static std::wstring format_posix_attrib_prop(const PropVariant& prop)
{
  if (!prop.is_uint())
    return std::wstring();

  unsigned val = static_cast<unsigned>(prop.get_uint());
  wchar_t attr[10];

  attr[0] = kPosixTypes[(val >> 12) & 0xF];
  for (int i = 6; i >= 0; i -= 3)
  {
    attr[7 - i] = ATTR_CHAR(val, i + 2, L'r');
    attr[8 - i] = ATTR_CHAR(val, i + 1, L'w');
    attr[9 - i] = ATTR_CHAR(val, i + 0, L'x');
  }
  if ((val & 0x800) != 0) attr[3] = ((val & (1 << 6)) ? L's' : L'S');
  if ((val & 0x400) != 0) attr[6] = ((val & (1 << 3)) ? L's' : L'S');
  if ((val & 0x200) != 0) attr[9] = ((val & (1 << 0)) ? L't' : L'T');

  val &= ~(unsigned)0xFFFF;
  return val ? std::wstring(attr, 10) + L' ' + uint_to_hex_str(val, 8) : std::wstring(attr, 10);
}

static const unsigned kNumWinAtrribFlags = 21;
static const wchar_t g_WinAttribChars[kNumWinAtrribFlags + 1] = L"RHS8DAdNTsLCOIEV.X.PU";

/* FILE_ATTRIBUTE_
 0 READONLY
 1 HIDDEN
 2 SYSTEM
 3 (Volume label - obsolete)
 4 DIRECTORY
 5 ARCHIVE
 6 DEVICE
 7 NORMAL
 8 TEMPORARY
 9 SPARSE_FILE
10 REPARSE_POINT
11 COMPRESSED
12 OFFLINE
13 NOT_CONTENT_INDEXED (I - Win10 attrib/Explorer)
14 ENCRYPTED
15 INTEGRITY_STREAM (V - ReFS Win8/Win2012)
16 VIRTUAL (reserved)
17 NO_SCRUB_DATA (X - ReFS Win8/Win2012 attrib)
18 RECALL_ON_OPEN or EA
19 PINNED
20 UNPINNED
21 STRICTLY_SEQUENTIAL
22 RECALL_ON_DATA_ACCESS
*/

static std::wstring format_attrib_prop(const PropVariant& prop)
{
  if (!prop.is_uint())
    return std::wstring();

  auto [posix, val] = get_posix_and_nt_attributes(static_cast<DWORD>(prop.get_uint()));

  wchar_t attr[kNumWinAtrribFlags];
  size_t na = 0;
  for (unsigned i = 0; i < kNumWinAtrribFlags; i++) {
    unsigned flag = (1U << i);
    if ((val & flag) != 0) {
      auto c = g_WinAttribChars[i];
      if (c != L'.') {
        val &= ~flag;
        // if (i != 7) // we can disable N (NORMAL) printing
        { attr[na++] = c; }
      }
    }
  }
  auto res = std::wstring(attr, na);

  if (val != 0) {
    if (na)
      res += L' ';
    res += uint_to_hex_str(val, 8);
  }

  if (posix) {
    if (!res.empty())
      res += L' ';
    PropVariant p((UInt32)posix);
    res += format_posix_attrib_prop(p);
  }

  return res;
}

typedef std::wstring (*PropToString)(const PropVariant& var);

struct PropInfo {
  PROPID prop_id;
  unsigned name_id;
  PropToString prop_to_string;
};

static PropInfo c_prop_info[] =
{
  { kpidPath, MSG_KPID_PATH, nullptr },
  { kpidName, MSG_KPID_NAME, nullptr },
  { kpidExtension, MSG_KPID_EXTENSION, nullptr },
  { kpidIsDir, MSG_KPID_ISDIR, nullptr },
  { kpidSize, MSG_KPID_SIZE, format_size_prop },
  { kpidPackSize, MSG_KPID_PACKSIZE, format_size_prop },
  { kpidAttrib, MSG_KPID_ATTRIB, format_attrib_prop },
  { kpidCTime, MSG_KPID_CTIME, format_filetime_prop },
  { kpidATime, MSG_KPID_ATIME, format_filetime_prop },
  { kpidMTime, MSG_KPID_MTIME, format_filetime_prop },
  { kpidSolid, MSG_KPID_SOLID, nullptr },
  { kpidCommented, MSG_KPID_COMMENTED, nullptr },
  { kpidEncrypted, MSG_KPID_ENCRYPTED, nullptr },
  { kpidSplitBefore, MSG_KPID_SPLITBEFORE, nullptr },
  { kpidSplitAfter, MSG_KPID_SPLITAFTER, nullptr },
  { kpidDictionarySize, MSG_KPID_DICTIONARYSIZE, format_size_prop },
  { kpidCRC, MSG_KPID_CRC, format_crc_prop },
  { kpidType, MSG_KPID_TYPE, nullptr },
  { kpidIsAnti, MSG_KPID_ISANTI, nullptr },
  { kpidMethod, MSG_KPID_METHOD, nullptr },
  { kpidHostOS, MSG_KPID_HOSTOS, nullptr },
  { kpidFileSystem, MSG_KPID_FILESYSTEM, nullptr },
  { kpidUser, MSG_KPID_USER, nullptr },
  { kpidGroup, MSG_KPID_GROUP, nullptr },
  { kpidBlock, MSG_KPID_BLOCK, nullptr },
  { kpidComment, MSG_KPID_COMMENT, nullptr },
  { kpidPosition, MSG_KPID_POSITION, nullptr },
  { kpidPrefix, MSG_KPID_PREFIX, nullptr },
  { kpidNumSubDirs, MSG_KPID_NUMSUBDIRS, nullptr },
  { kpidNumSubFiles, MSG_KPID_NUMSUBFILES, nullptr },
  { kpidUnpackVer, MSG_KPID_UNPACKVER, nullptr },
  { kpidVolume, MSG_KPID_VOLUME, nullptr },
  { kpidIsVolume, MSG_KPID_ISVOLUME, nullptr },
  { kpidOffset, MSG_KPID_OFFSET, nullptr },
  { kpidLinks, MSG_KPID_LINKS, nullptr },
  { kpidNumBlocks, MSG_KPID_NUMBLOCKS, nullptr },
  { kpidNumVolumes, MSG_KPID_NUMVOLUMES, nullptr },
  { kpidTimeType, MSG_KPID_TIMETYPE, nullptr },
  { kpidBit64, MSG_KPID_BIT64, nullptr },
  { kpidBigEndian, MSG_KPID_BIGENDIAN, nullptr },
  { kpidCpu, MSG_KPID_CPU, nullptr },
  { kpidPhySize, MSG_KPID_PHYSIZE, format_size_prop },
  { kpidHeadersSize, MSG_KPID_HEADERSSIZE, format_size_prop },
  { kpidChecksum, MSG_KPID_CHECKSUM, nullptr },
  { kpidCharacts, MSG_KPID_CHARACTS, nullptr },
  { kpidVa, MSG_KPID_VA, nullptr },
  { kpidId, MSG_KPID_ID, nullptr },
  { kpidShortName, MSG_KPID_SHORTNAME, nullptr },
  { kpidCreatorApp, MSG_KPID_CREATORAPP, nullptr },
  { kpidSectorSize, MSG_KPID_SECTORSIZE, format_size_prop },
  { kpidPosixAttrib, MSG_KPID_POSIXATTRIB, format_posix_attrib_prop },
  { kpidSymLink, MSG_KPID_LINK, nullptr },
  { kpidError, MSG_KPID_ERROR, nullptr },
  { kpidTotalSize, MSG_KPID_TOTALSIZE, format_size_prop },
  { kpidFreeSpace, MSG_KPID_FREESPACE, format_size_prop },
  { kpidClusterSize, MSG_KPID_CLUSTERSIZE, format_size_prop },
  { kpidVolumeName, MSG_KPID_VOLUMENAME, nullptr },
  { kpidLocalName, MSG_KPID_LOCALNAME, nullptr },
  { kpidProvider, MSG_KPID_PROVIDER, nullptr },
  { kpidNtSecure, MSG_KPID_NTSECURE, nullptr },
  { kpidIsAltStream, MSG_KPID_ISALTSTREAM, nullptr },
  { kpidIsAux, MSG_KPID_ISAUX, nullptr },
  { kpidIsDeleted, MSG_KPID_ISDELETED, nullptr },
  { kpidIsTree, MSG_KPID_ISTREE, nullptr },
  { kpidSha1, MSG_KPID_SHA1, nullptr },
  { kpidSha256, MSG_KPID_SHA256, nullptr },
  { kpidErrorType, MSG_KPID_ERRORTYPE, nullptr },
  { kpidNumErrors, MSG_KPID_NUMERRORS, nullptr },
  { kpidErrorFlags, MSG_KPID_ERRORFLAGS, nullptr },
  { kpidWarningFlags, MSG_KPID_WARNINGFLAGS, nullptr },
  { kpidWarning, MSG_KPID_WARNING, nullptr },
  { kpidNumStreams, MSG_KPID_NUMSTREAMS, nullptr },
  { kpidNumAltStreams, MSG_KPID_NUMALTSTREAMS, nullptr },
  { kpidAltStreamsSize, MSG_KPID_ALTSTREAMSSIZE, format_size_prop },
  { kpidVirtualSize, MSG_KPID_VIRTUALSIZE, format_size_prop },
  { kpidUnpackSize, MSG_KPID_UNPACKSIZE, format_size_prop },
  { kpidTotalPhySize, MSG_KPID_TOTALPHYSIZE, format_size_prop },
  { kpidVolumeIndex, MSG_KPID_VOLUMEINDEX, nullptr },
  { kpidSubType, MSG_KPID_SUBTYPE, nullptr },
  { kpidShortComment, MSG_KPID_SHORTCOMMENT, nullptr },
  { kpidCodePage, MSG_KPID_CODEPAGE, nullptr },
  { kpidIsNotArcType, MSG_KPID_ISNOTARCTYPE, nullptr },
  { kpidPhySizeCantBeDetected, MSG_KPID_PHYSIZECANTBEDETECTED, nullptr },
  { kpidZerosTailIsAllowed, MSG_KPID_ZEROSTAILISALLOWED, nullptr },
  { kpidTailSize, MSG_KPID_TAILSIZE, format_size_prop },
  { kpidEmbeddedStubSize, MSG_KPID_EMBEDDEDSTUBSIZE, format_size_prop },
  { kpidNtReparse, MSG_KPID_NTREPARSE, nullptr },
  { kpidHardLink, MSG_KPID_HARDLINK, nullptr },
  { kpidINode, MSG_KPID_INODE, nullptr },
  { kpidStreamId, MSG_KPID_STREAMID, nullptr },
  { kpidReadOnly, MSG_KPID_READONLY, nullptr },
  { kpidOutName, MSG_KPID_OUTNAME, nullptr },
  { kpidCopyLink, MSG_KPID_COPYLINK, nullptr },
  { kpidArcFileName, MSG_KPID_ARCFILENAME, nullptr },
  { kpidIsHash, MSG_KPID_ISHASH, nullptr },
  { kpidChangeTime, MSG_KPID_METADATA_CHANGED, nullptr },
  { kpidUserId, MSG_KPID_USER_ID, nullptr },
  { kpidGroupId, MSG_KPID_GROUP_ID, nullptr },
  { kpidDeviceMajor, MSG_KPID_DEVICE_MAJOR, nullptr },
  { kpidDeviceMinor, MSG_KPID_DEVICE_MINOR, nullptr },
  { kpidDevMajor, MSG_KPID_DEV_MAJOR, nullptr },
  { kpidDevMinor, MSG_KPID_DEV_MINOR, nullptr }
};

static const PropInfo* find_prop_info(PROPID prop_id) {
  static_assert(_countof(c_prop_info) == kpid_NUM_DEFINED-kpidPath, "Missed items in c_prop_info");
  if (prop_id < kpidPath || prop_id >= kpid_NUM_DEFINED)
    return nullptr;
  else
    return c_prop_info + (prop_id - kpidPath);
}

AttrList Archive::get_attr_list(UInt32 item_index) {
  AttrList attr_list;
  if (item_index >= m_num_indices) // fake index
    return attr_list;
  UInt32 num_props;
  CHECK_COM(in_arc->GetNumberOfProperties(&num_props));
  for (unsigned i = 0; i < num_props; i++) {
    Attr attr;
    BStr name;
    PROPID prop_id;
    VARTYPE vt;
    CHECK_COM(in_arc->GetPropertyInfo(i, name.ref(), &prop_id, &vt));
    const PropInfo* prop_info = find_prop_info(prop_id);
    if (prop_info)
      attr.name = Far::get_msg(prop_info->name_id);
    else if (name)
      attr.name.assign(name, name.size());
    else
      attr.name = int_to_str(prop_id);

    PropVariant prop;
    CHECK_COM(in_arc->GetProperty(item_index, prop_id, prop.ref()));

    if (prop_info != nullptr && prop_info->prop_to_string) {
      attr.value = prop_info->prop_to_string(prop);
    }
    else {
      if (prop.is_str())
        attr.value = format_str_prop(prop);
      else if (prop.is_bool())
        attr.value = Far::get_msg(prop.get_bool() ? MSG_PROPERTY_TRUE : MSG_PROPERTY_FALSE);
      else if (prop.is_uint())
        attr.value = format_uint_prop(prop);
      else if (prop.is_int())
        attr.value = format_int_prop(prop);
      else if (prop.is_filetime())
        attr.value = format_filetime_prop(prop);
    }

    if (!attr.value.empty())
      attr_list.push_back(attr);
  }

  return attr_list;
}

void Archive::load_arc_attr() {
  arc_attr.clear();

  Attr attr;
  attr.name = Far::get_msg(MSG_KPID_PATH);
  attr.value = arc_path;
  arc_attr.push_back(attr);

  UInt32 num_props;
  CHECK_COM(in_arc->GetNumberOfArchiveProperties(&num_props));
  for (unsigned i = 0; i < num_props; i++) {
    BStr name;
    PROPID prop_id;
    VARTYPE vt;
    CHECK_COM(in_arc->GetArchivePropertyInfo(i, name.ref(), &prop_id, &vt));
    const PropInfo* prop_info = find_prop_info(prop_id);
    if (prop_info)
      attr.name = Far::get_msg(prop_info->name_id);
    else if (name)
      attr.name.assign(name, name.size());
    else
      attr.name = int_to_str(prop_id);

    PropVariant prop;
    CHECK_COM(in_arc->GetArchiveProperty(prop_id, prop.ref()));

    attr.value.clear();
    if (prop_info != nullptr && prop_info->prop_to_string) {
      attr.value = prop_info->prop_to_string(prop);
    }
    else {
      if (prop.is_str())
        attr.value = format_str_prop(prop);
      else if (prop.is_bool())
        attr.value = Far::get_msg(prop.get_bool() ? MSG_PROPERTY_TRUE : MSG_PROPERTY_FALSE);
      else if (prop.is_uint())
        attr.value = format_uint_prop(prop);
      else if (prop.is_int())
        attr.value = format_int_prop(prop);
      else if (prop.is_filetime())
        attr.value = format_filetime_prop(prop);
    }

    if (!attr.value.empty())
      arc_attr.push_back(attr);
  }

  // compression ratio
  bool total_size_defined = true;
  UInt64 total_size = 0;
  bool total_packed_size_defined = true;
  UInt64 total_packed_size = 0;
  unsigned file_count = 0;
  PropVariant prop;
  for (UInt32 file_id = 0; file_id < m_num_indices && total_size_defined; file_id++) {
    if (!file_list[file_id].is_dir) {
      if (in_arc->GetProperty(file_id, kpidSize, prop.ref()) == S_OK && prop.is_uint())
        total_size += prop.get_uint();
      else
        total_size_defined = false;
      if (in_arc->GetProperty(file_id, kpidPackSize, prop.ref()) == S_OK && prop.is_uint())
        total_packed_size += prop.get_uint();
      else
        total_packed_size_defined = false;
      bool is_dir = in_arc->GetProperty(file_id, kpidIsDir, prop.ref()) == S_OK && prop.is_bool() && prop.get_bool();
      if (!is_dir)
        ++file_count;
    }
  }
  if (total_size_defined) {
    attr.name = Far::get_msg(MSG_PROPERTY_COMPRESSION_RATIO);
    auto arc_size = archive_filesize();
    unsigned ratio = total_size ? al_round(static_cast<double>(arc_size) / static_cast<double>(total_size) * 100.0) : 100;
    if (ratio > 100)
      ratio = 100;
    attr.value = int_to_str(ratio) + L'%';
    arc_attr.push_back(attr);
    attr.name = Far::get_msg(MSG_PROPERTY_TOTAL_SIZE);
    attr.value = format_size_prop(total_size);
    arc_attr.push_back(attr);
  }
  if (total_packed_size_defined) {
    attr.name = Far::get_msg(MSG_PROPERTY_TOTAL_PACKED_SIZE);
    attr.value = format_size_prop(total_packed_size);
    arc_attr.push_back(attr);
  }
  attr.name = Far::get_msg(MSG_PROPERTY_FILE_COUNT);
  attr.value = int_to_str(file_count);
  arc_attr.push_back(attr);
  attr.name = Far::get_msg(MSG_PROPERTY_DIR_COUNT);
  attr.value = int_to_str(m_num_indices - file_count);
  arc_attr.push_back(attr);

  // archive files have CRC?
  m_has_crc = true;
  for (UInt32 file_id = 0; file_id < m_num_indices && m_has_crc; file_id++) {
    if (!file_list[file_id].is_dir) {
      if (in_arc->GetProperty(file_id, kpidCRC, prop.ref()) != S_OK || !prop.is_uint())
        m_has_crc = false;
    }
  }
}

void Archive::load_update_props() {
  if (m_update_props_defined) return;

  m_encrypted = false;
  PropVariant prop;
  for (UInt32 i = 0; i < m_num_indices; i++) {
    if (in_arc->GetProperty(i, kpidEncrypted, prop.ref()) == S_OK && prop.is_bool() && prop.get_bool()) {
      m_encrypted = true;
      break;
    }
  }

  m_solid = in_arc->GetArchiveProperty(kpidSolid, prop.ref()) == S_OK && prop.is_bool() && prop.get_bool();

  m_level = (unsigned)-1;
  m_method.clear();
  if ((in_arc->GetArchiveProperty(kpidMethod, prop.ref()) == S_OK && prop.is_str()) || (in_arc->GetProperty(0, kpidMethod, prop.ref()) == S_OK && prop.is_str())) {
    std::list<std::wstring> m_list = split(prop.get_str(), L' ');

    static const wchar_t *known_methods[] = { c_method_lzma, c_method_lzma2, c_method_ppmd, c_method_deflate, c_method_deflate64 };

    for (const auto& m_full_str: m_list) {
      const auto m_str = m_full_str.substr(0, m_full_str.find(L':'));
      if (_wcsicmp(m_str.c_str(), c_method_copy) == 0) {
        m_level = 0;
        m_method = c_method_lzma;
        break;
      }
      for (const auto known : known_methods) {
        if (_wcsicmp(m_str.c_str(), known) == 0)
        { m_method = known; break; }
      }
      for (const auto& known : g_options.codecs) {
        if (_wcsicmp(m_str.c_str(), known.name.c_str()) == 0)
        { m_method = known.name; break; }
      }
      if (!m_method.empty())
        break;
    }
  }

  if (m_level == (unsigned)-1)
    m_level = 7; // maximum
  if (m_method.empty())
    m_method = c_method_lzma;

  m_update_props_defined = true;
}
