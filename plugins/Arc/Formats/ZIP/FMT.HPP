#define NM 260

#define GETARC_EOF               0
#define GETARC_SUCCESS           1
#define GETARC_BROKEN            2
#define GETARC_UNEXPEOF          3
#define GETARC_READERROR         4

struct PluginPanelItem
{
  WIN32_FIND_DATA FindData;
  DWORD PackSize;
  char *DizText;
  char Owner[40];
};
