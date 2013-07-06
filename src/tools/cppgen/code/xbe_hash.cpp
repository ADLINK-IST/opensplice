/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include "xbe_hash.h"

// -----------------------------------------
// hash_str is used by the generated
//  dispatcher
// -----------------------------------------
/*
DDS::ULong
DDSStd::hash_str(const char* str)
{
 const long     p = 1073741827L;  // prime
 int            n = strlen(str);
 long           h = 0;
 long           retval = 0;
 
   for (int i = 0; i < n; ++i, ++str) 
 {
  h = (h << 2) + *str;
 }
 
   retval = ((h >= 0) ? (h % p) : (-h % p));
 
   return retval;
}
 
*/

#define each_xps_hash_base_element(x, array, size) \
   x = &(array)[size]; \
   x-- > (array);


// ------------------------------------------------------------
//  DDSHashBaseIterator
// ------------------------------------------------------------

DDSHashBaseIterator::DDSHashBaseIterator()
      :
      m_pbuckets(0),
      m_pend(0),
      m_pnode(0)
{}

DDSHashBaseIterator::DDSHashBaseIterator(const DDSHashBaseIterator& other)
      :
      m_pbuckets(other.m_pbuckets),
      m_pend(other.m_pend),
      m_pnode(other.m_pnode)
{}

DDSHashBaseIterator::DDSHashBaseIterator( DDSHashLink ** ptable, DDSHashLink ** pend, DDSHashLink * node )
      :
      m_pbuckets(ptable),
      m_pend(pend),
      m_pnode(node)
{}


DDSHashBaseIterator &
DDSHashBaseIterator::operator = ( const DDSHashBaseIterator & other )
{
   m_pbuckets = other.m_pbuckets;
   m_pend = other.m_pend;
   m_pnode = other.m_pnode;
   return *this;
}

short
DDSHashBaseIterator::operator == ( const DDSHashBaseIterator & other ) const
{
   return ( (m_pbuckets == other.m_pbuckets) &&
            (m_pend == other.m_pend) &&
            (m_pnode == other.m_pnode));
}

short
DDSHashBaseIterator::operator != ( const DDSHashBaseIterator & other ) const
{
   return ( (m_pbuckets != other.m_pbuckets) ||
            (m_pend != other.m_pend) ||
            (m_pnode != other.m_pnode));
}

void
DDSHashBaseIterator::incr()
{
   if ( m_pnode )
      m_pnode = m_pnode->m_next;

   if ( !m_pnode && m_pbuckets != m_pend )
   {
      do
      {
         m_pbuckets++;
      }
      while ( m_pbuckets != m_pend && !(*m_pbuckets) );

      m_pnode = (m_pbuckets == m_pend ? ((DDSHashLink*) NULL) : *m_pbuckets);
   }
}




// ------------------------------------------------------------
//  DDSHashBase
// ------------------------------------------------------------

DDSHashBase::DDSHashBase
(
   DDS::ULong buckets,
   DDSHashEqualsProc eProc,
   DDSHashDeleteProc dProc,
   DDS::ULong growthFactor,
   float densityLimit
)
:
   m_EqualEntries (eProc),
   m_DeleteEntry (dProc),
   m_tablesize (buckets),
   m_count (0),
   m_growthFactor ((2 > growthFactor ? 2 : growthFactor)),
   m_triggerLimit (DDS::ULong (buckets * densityLimit)),
   m_densityLimit (densityLimit)
{
   m_buckets = new DDSHashLink * [m_tablesize];
   DDSHashLink ** ptr = m_buckets;

   for ( each_xps_hash_base_element( ptr, m_buckets, m_tablesize))
      *ptr = NULL;
}

DDSHashBase::~DDSHashBase()
{
   DDSHashLink ** list;

   for (each_xps_hash_base_element(list, m_buckets, m_tablesize))
      while ( *list )
      {
         DDSHashLink *item = *list;
         *list = item->m_next;
         m_DeleteEntry(item);
      }

   delete [] m_buckets;
}

DDSHashBaseIterator
DDSHashBase::find( DDSHashLink * entry )
{
   DDSHashLink * item;
   DDSHashLink ** list;

   if (!entry)
      return DDSHashBaseIterator(end_bucket(), end_bucket(), NULL);

   list = find_bucket(entry->m_hash);

   for ( item = *list; item != NULL; item = next_hit(item) )
      if (m_EqualEntries(item, entry))
         return DDSHashBaseIterator(list, end_bucket(), item);

   return DDSHashBaseIterator(end_bucket(), end_bucket(), NULL);
}

DDSHashBaseIterator
DDSHashBase::insert( DDSHashLink * entry )
{
   if ( m_count >= m_triggerLimit )
   {
      resize( m_tablesize * m_growthFactor + 1 );
      m_triggerLimit = DDS::ULong(m_tablesize * m_densityLimit);
   }

   DDSHashLink ** list = find_bucket(entry->hash());
   entry->m_next = *list;
   *list = entry;

   m_count++;
   return DDSHashBaseIterator(list, end_bucket(), entry);
}


void

DDSHashBase::remove
   ( const DDSHashBaseIterator & iter )
{
   if ( iter.pbucket() != iter.pend() )
   {
      DDSHashLink * item;
      DDSHashLink * priorItem = NULL;

      for ( item = *iter.pbucket(); item != NULL; item = next_hit(item) )
      {
         if ( item == iter.node() )
         {
            if ( priorItem )
               priorItem->m_next = item->m_next;
            else
               *iter.pbucket() = item->m_next;

            m_DeleteEntry(item);

            m_count--;

            return ;
         }

         priorItem = item;
      }
   }
}


void

DDSHashBase::remove
   ( const DDSHashBaseIterator & beginIter,
         const DDSHashBaseIterator & endIter )
{
   if ( beginIter.valid() )
   {
      DDSHashLink ** bucketOne = begin_bucket();

      if ( beginIter.pbucket() == bucketOne &&
            beginIter.node() == *bucketOne &&
            endIter.pbucket() == end_bucket() )
      {
         remove_all();  // faster if we know to remove all items
      }
      else
      {
         DDSHashBaseIterator lastIter;
         DDSHashBaseIterator iter = beginIter;

         if ( iter != endIter )
            do
            {
               lastIter = iter;
               iter.incr();

               remove
                  (lastIter);
            }
            while ( iter != endIter );
      }
   }
}


void
DDSHashBase::resize( DDS::ULong newSize )
{
   DDSHashLink ** newTable;
   DDSHashLink ** oldTable;
   DDSHashLink ** bucket;
   DDS::ULong oldSize;

   newTable = new DDSHashLink * [newSize];

   for (each_xps_hash_base_element( bucket, newTable, newSize))
      *bucket = NULL;

   oldTable = m_buckets;

   oldSize = m_tablesize;

   m_buckets = newTable;  // find method uses member vars

   m_tablesize = newSize;

   DDSHashLink ** list;

   DDSHashLink * item;

   DDSHashLink * nextItem;

   for (each_xps_hash_base_element(list, oldTable, oldSize))
      for ( item = *list; item != NULL; item = nextItem )
      {
         nextItem = item->m_next;
         bucket = find_bucket(item->hash());
         item->m_next = *bucket;
         *bucket = item;
      }

   //delete [] oldTable;
   delete [] oldTable;
}

void
DDSHashBase::remove_all()
{
   DDSHashLink ** list;
   DDSHashLink * item;
   DDSHashLink * nextItem;

   for (each_xps_hash_base_element(list, m_buckets, m_tablesize))
   {
      for ( item = *list; item != NULL; item = nextItem )
      {
         nextItem = item->m_next;
         m_DeleteEntry(item);
      }

      *list = NULL;
   }

   m_count = 0;
}


DDSHashLink **
DDSHashBase::begin_bucket()
{
   DDSHashLink ** bucket;
   DDSHashLink ** limit = end_bucket();

   for (bucket = m_buckets; bucket < limit; bucket++ )
      if ( *bucket )
         return bucket;

   return limit;
}
