#define GETARC_EOF               0
#define GETARC_SUCCESS           1
#define GETARC_BROKEN            2
#define GETARC_UNEXPEOF          3
#define GETARC_READERROR         4

extern "C" {
DWORD WINAPI _export LoadFormatModule(char *ModuleName);
BOOL WINAPI _export IsArchive(char *Name,const unsigned char *Data,int DataSize);
BOOL WINAPI _export OpenArchive(char *Name,int *Type);
int WINAPI _export GetArcItem(struct PluginPanelItem *Item,struct ArcItemInfo *Info);
BOOL WINAPI _export CloseArchive(struct ArcInfo *Info);
BOOL WINAPI _export GetFormatName(int Type,char *FormatName,char *DefaultExt);
BOOL WINAPI _export GetDefaultCommands(int Type,int Command,char *Dest);
}


struct ArcItemInfo
{
  char HostOS[32];
  char Description[256];
  int Solid;
  int Comment;
  int Encrypted;
  int DictSize;
  int UnpVer;
};


enum ARCINFO_FLAGS
{
  AF_AVPRESENT=1,
  AF_IGNOREERRORS=2
};

struct ArcInfo
{
  int SFXSize;
  int Volume;
  int Comment;
  int Recovery;
  int Lock;
  DWORD Flags;
};
