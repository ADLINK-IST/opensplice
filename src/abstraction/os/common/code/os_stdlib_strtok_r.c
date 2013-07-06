char *
os_index(
    const char *s,
    int c)
{
    char *first = NULL;
    while (*s) {
        if (*s == c) {
            first = (char *)s;
            break;
        }
        s++;
    }
    return first;
}

char *os_strtok_r(char *str, const char *delim, char **saveptr)
{
#if OS_HAS_STRTOK_R == 1
   return( strtok_r( str, delim, saveptr ) );
#else
   char *ret;
   int found = 0;

   if ( str == NULL )
   {
      str = *saveptr;
   }
   ret = str;

   if ( str != NULL )
   {
      /* Ignore delimiters at start */
      while ( *str != '\0' && os_index( delim, *str ) != NULL  )
      {
         str++;
         ret++;
      };

      while ( *str != '\0' )
      {
         if ( os_index( delim, *str ) != NULL )
         {
            *str='\0';
            *saveptr=(str+1);
            found=1;
            break;
         }
         str++;
      }

      if ( !found )
      {
         *saveptr=NULL;
      }
   }
   return ( ret != NULL && *ret == '\0' ? NULL : ret );
#endif
}
