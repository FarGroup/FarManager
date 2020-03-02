#include <Rtl.Base.h>
#include <FarPluginBase.hpp>
#include "unixutils.hpp"

int IsTarHeader(const unsigned char *Data,int DataSize)
{
  int I;
  const struct posix_header *Header;
  if((unsigned int)DataSize<sizeof(posix_header))
    return FALSE;
  Header=(const struct posix_header *)Data;
  if(!strncmp(Header->magic,TMAGIC,TMAGLEN-1))
  {
    /* If we have what looks like a real ustar archive, we must
    check the version number.  We only understand version 00.  */
    if(Header->magic[TMAGLEN-1]=='\0'&&(Header->version[0]!='0'||Header->version[1]!='0'))
      return FALSE;
  }
  for (I=0;Header->name[I];I++)
    if (I==sizeof(Header->name) || Header->name[I]<' ')
      return(FALSE);
  for (I=0;I<&Header->typeflag-Header->mode;I++)
    if (Header->mode[I]>'7' || (Header->mode[I]<'0' && Header->mode[I]!=0 &&
        Header->mode[I]!=' '))
      return(FALSE);
  if(strcmp(Header->name,"././@LongLink"))
  {
    DWORD Seconds=GetOctal(Header->mtime);
    if (Seconds<300000000 || Seconds>1500000000)
      return(FALSE);
  }
  return(TRUE);
}

int IsCpioHeader(const unsigned char *Data,int DataSize) //FIXME
{
  if(DataSize>5)
  {
    if(!strncmp((const char *)Data,"070701",6)) return TRUE;
  }
  return FALSE;
}
