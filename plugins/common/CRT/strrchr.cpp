#include "strrchr.hpp"

char *strrchr(const char *string, int ch)
{
  const char *start = string;

  while (*string++)
    ;

  while (--string != start && *string != (char)ch)
    ;

  if (*string == (char)ch)
    return (char *)string;

  return NULL;
}
