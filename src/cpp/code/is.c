#include "is.h"

extern int ishspace (char c)
{
   return((c == ' ') || (c == '\t'));
}

extern int isbsymchar (char c)
{
   return(isalpha((int) c) || (c == '_'));
}

extern int issymchar (char c)
{
   return(isalnum((int) c) || (c == '_') || (c == '$'));
}
