#include <string.h>
#include <memory.h>

// dest и src могут пересекаться
char * __cdecl strncpy (char * dest,const char * src,size_t maxlen)
{
  size_t len;

  len = strlen(src);
  len = (len > maxlen)?maxlen:len;
  memmove(dest,src,len);
  // здесь бы try/except поставить...
  dest[len]=0;
  return (dest);
}
