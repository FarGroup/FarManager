#include "strncat.hpp"

char *strncat (char *first, const char *last, size_t count)
{
  char *start = first;

  while (*first++)
    ;
  first--;

  while (count--)
    if (!(*first++ = *last++))
      return(start);

  *first = '\0';
  return(start);
}
