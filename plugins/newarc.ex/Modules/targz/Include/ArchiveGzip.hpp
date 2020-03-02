#ifndef __ARCHIVEGZIP_HPP__
#define __ARCHIVEGZIP_HPP__

#include <plugin.hpp>
#include "Archive.hpp"
#include "FarZlib.hpp"
#include "FarBlib.hpp"

template <class T> class TarPlain: public T
{
  private:
    HANDLE in;
  protected:
    bool read(void *buffer,DWORD size,DWORD *readed) { return ReadFile(in,buffer,size,readed,NULL); }
    bool error(void) { return (in==INVALID_HANDLE_VALUE); }
    int seek(int position) { return SetFilePointer(in,position,NULL,FILE_BEGIN); }
  public:
    TarPlain(const char *filename) : T() { in=CreateFile(filename,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL); }
    ~TarPlain() { if(in!=INVALID_HANDLE_VALUE) CloseHandle(in); }
};

template <class T> class TarGzip: public T
{
  private:
    gzFile in;
  protected:
    bool read(void *buffer,DWORD size,DWORD *readed) { *readed=pgzread(in,buffer,size); return (*readed)>=0; }
    bool error(void)
    {
      return false;//(in!=NULL);
    }
    int seek(int position) { return pgzseek(in,position,SEEK_SET); }
  public:
    TarGzip(const char *filename) : T() { in=pgzopen(filename,"rb"); }
    ~TarGzip() { if(in!=NULL) pgzclose(in); }
};

template <class T> class TarBzip: public T
{
  private:
    BZFILE *in;
    int curpos;
    char FileName[260];
  protected:
    bool read(void *buffer,DWORD size,DWORD *readed) { *readed=pbzread(in,buffer,size); if((*readed)>=0) curpos+=*readed; return (*readed)>=0; }
    bool error(void)
    {
      return false;//(in!=NULL);
    }
    int seek(int position)
    {
      if(curpos>=position)
      {
        if(in!=NULL) pbzclose(in);
        curpos=0; in=pbzopen(FileName,"rb");
      }
      if(curpos>=position) return curpos;
      int remain=position-curpos,bytes,readed;
      char buffer[32*1024];
      while(remain)
      {
        bytes=(remain>(int)sizeof(buffer))?sizeof(buffer):remain;
        readed=pbzread(in,buffer,bytes);
        curpos+=readed;
        if(readed<bytes) break;
        remain-=bytes;
      }
      return curpos;
    }
  public:
    TarBzip(const char *filename) : T() { curpos=0; in=pbzopen(filename,"rb"); strcpy(FileName,filename);}
    ~TarBzip() { if(in!=NULL) pbzclose(in); }
};

#endif
