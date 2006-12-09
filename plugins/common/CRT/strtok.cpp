#include "crt.hpp"

char * __cdecl strtok(char *string, const char *control)
{
  unsigned char *str;
  const unsigned char *ctrl = (const unsigned char *) control;

  unsigned char map[32];
  int count;

  static unsigned char *nextoken;

  for (count = 0; count < 32; count++)
    map[count] = 0;

  do
  {
    map[*ctrl >> 3] |= (1 << (*ctrl & 7));
  } while (*ctrl++);

  if (string)
    str = (unsigned char *) string;
  else
    str = nextoken;

  while ((map[*str >> 3] & (1 << (*str & 7))) && *str)
          str++;

  string = (char *) str;

  for (; *str; str++)
    if (map[*str >> 3] & (1 << (*str & 7)))
    {
      *str++ = '\0';
      break;
    }

  nextoken = str;

  if (string == (char *)str)
    return NULL;
  else
    return string;
}
