/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
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
