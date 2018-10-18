/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

/* Implementation of bsearch not available on Windows CE */

void *
os_bsearch(const void *key, const void *base, size_t nmemb, size_t size,
    int (*compar) (const void *, const void *))
{   
   /* The comparison function "compar" is expected to return :
    * < 0 : if the item is less than the array member
    *   0 : equal to the array member
    * > 0 : greater than the array member
    */

   int lowestIndex = 0;
   int highestIndex = nmemb - 1;
   int currentIndex = 0;
   int comparisonResult = 0;

   void * result = NULL;
   /*
    * Cannot do pointer arithmetic on void*, so use a char*
    */
   char *currentElement;

   while (lowestIndex <= highestIndex)
   {
      currentIndex = lowestIndex + ((highestIndex - lowestIndex) / 2);
      currentElement = (char*)base + (size*currentIndex);
 
      comparisonResult = (compar) (key, currentElement);

      if (comparisonResult < 0) /* key is less than the array item so look at lower end of array */
      {
         highestIndex = currentIndex - 1;
      }
      else if (comparisonResult == 0) /* found the item */
      {
         result = (void*)currentElement;
         break;
      }
      else /* key is greater than the array item so look at higher end of array */
      {
         lowestIndex = currentIndex + 1;
      }
   }

   return result;
}
