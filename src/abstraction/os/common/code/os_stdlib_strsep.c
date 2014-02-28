#include "os_stdlib.h"

char *os_strsep (char **str, const char *sep)
{
  char *ret;
  if (**str == '\0')
    return 0;
  ret = *str;
  while (**str && strchr (sep, **str) == 0)
    (*str)++;
  if (**str != '\0')
  {
    **str = '\0';
    (*str)++;
  }
  return ret;
}
