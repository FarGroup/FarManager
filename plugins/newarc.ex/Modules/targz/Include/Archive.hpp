#ifndef __ARCHIVE_HPP__
#define __ARCHIVE_HPP__

#include <plugin.hpp>

class ArchiveBase
{
  public:
    virtual ~ArchiveBase() {}
    virtual int Next(PluginPanelItem *data)=0;
    virtual bool Extract(PluginPanelItem *pItem,char *destination)=0;
};

class ArchiveOne: public ArchiveBase
{
  private:
    char ZipName[NM];
    bool Processed;
  protected:
    HANDLE Handle;
  public:
    ArchiveOne(const char *ArcName);
    ~ArchiveOne();
    int Next(PluginPanelItem *data);
    bool Extract(PluginPanelItem *pItem,char *destination);
};

class ArchiveGzip: public ArchiveOne
{
  public:
    ArchiveGzip(const char *ArcName): ArchiveOne(ArcName) {}
    int Next(PluginPanelItem *data);
};

class ArchiveDetect
{
  public:
    virtual ~ArchiveDetect() {}
    bool Read(unsigned char *Data,DWORD Size);
    virtual bool read(void *buffer,DWORD size,DWORD *readed)=0;
};

class TarBase: public ArchiveBase
{
  private:
    bool firsttime;
    bool valid;
    DWORD NextPosition;
    int Exist,Retry;
    PluginPanelItem Item;
  protected:
    virtual bool read(void *buffer,DWORD size,DWORD *readed)=0;
    virtual bool error(void)=0;
    virtual int seek(int position)=0;
  public:
    TarBase();
    int Next(PluginPanelItem *data);
    bool Extract(PluginPanelItem *pItem,char *destination);
};

class CpioBase: public ArchiveBase
{
  private:
    bool firsttime;
    bool valid;
    DWORD NextPosition;
    int Exist,Retry;
    PluginPanelItem Item;
  protected:
    virtual bool read(void *buffer,DWORD size,DWORD *readed)=0;
    virtual bool error(void)=0;
    virtual int seek(int position)=0;
  public:
    CpioBase();
    int Next(PluginPanelItem *data);
    bool Extract(PluginPanelItem *pItem,char *destination);
};

extern PluginStartupInfo Info; //FIXME

#endif
