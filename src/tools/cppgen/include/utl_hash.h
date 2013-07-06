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
#ifndef _UTL_UTLHASH_H_
#define _UTL_UTLHASH_H_

/******************************************************************
 
   UtlMap<KEY,VAL> and UtlSet<KEY> are two, templatized collections
   that use hashed access for storage. They follow the proposed STL
   hashtable API in spirit, but factor routines into a non-template
 base class to prevent code bloat. The user must provide a hashing 
 function to convert from the specified KEY type to an unsigned 
 long hash value. Hash values will be modulo mapped into the
 available bucket space. 
 
   The implementation uses hashing buckets, and chains collisions.   
 KEYs are assumed unique (i.e. not a multimap or multiset).
   No STL files need be included to support functionality in 
   this module. The operator[](int) and constructor functions do
   not follow STL API but are very convenient (see example below). 
 Hash iterators add the non-standard function valid(), which 
 avoids overhead of creating a second iterator for comparison.
 Also, iterators are customized for set and map to access key()
 and value() information.
 
   Tables will automatically resize to accomodate additional members.
 Hashtables do not currently shrink when members are removed.
 
 If value is a pointer type, erasing an element from the map does not
 delete or DDS_release the pointer.   If value is an object it's dtor
 will be called when the item is erased.
 
   Here is an example hashing function for strings and simple one for chars...
 
      static unsigned long
      hash_str ( const UtlStd::String &str )
      {
       const long  p = 1073741827L; // prime
       int    n = str.length();
       const char * d = (const char*)str;
       long    h = 0;
       unsigned   retval = 0;
 
       for( int i = 0; i < n; ++i, ++d)
        h = (h << 2) + *d;
       retval =  ((h >= 0) ? (h % p) : (-h % p));
       return retval;
      }
 
  static unsigned long
  hash_char ( const char & theChar )
  {
   return theChar;
  }
 
   And how they might be used ...
 
  typedef  UtlMap<UtlStd::String, int>   WordCounter;
  typedef  UtlMap<char, int>       CharCounter;
  typedef  UtlSet<char>            CharSet;
 
  WordCounter             words ( hash_str );
  CharCounter             letters ( hash_char );
  CharSet                 letter_set( hash_char );
  char                    letter;
 
  UtlStd::String str = "";
  char     cstr[1024];
  ifstream in("main.C");
 
  while ( in >> cstr )
  {
   str = cstr;
   ++words[str];
   for( int i = 0; i < str.length(); ++i)
   {
    letterSet.insert(str[i]);
   ++letters[str[i]];
   }
  }
 
  WordCounter::iterator wi;
  for( wi = words.begin(); wi != words.end(); wi++ )
   cout << *wi << "\t" << wi.key() << endl;
 
  for( letter = 'a'; letter <= 'z'; letter++ )
   if(letterSet[letter])
    cout << letter;
 
 NOTES:
 
  Known problems/caveats...
 
  The public constructor format of these classes means they
  storage must be created in the *.C file vs. interface.
  The ramification is that a hashtable must be a pointer member
  in a class definition.
 
  There are some typedef situations that don't do well with
  the nested iterator class. Specifically typedef'ing a
  template definition iterator for use within a class definition.
  Scott believes this is a Sun compiler problem.
 
  The templates cannot be created with a pointer KEY type.
  Even past these issues, the STL hashtable API's are not very 
  accomodating to managing KEY pointer deletes.
 
******************************************************************/

#define UtlHashBase_GROWTH 2
#define UtlHashBase_SIZE  31
#define UtlHashBase_DENSITY 0.80

// ------------------------------------------------------------
//  UtlHashLink
// ------------------------------------------------------------


typedef unsigned long UtlHashValue;

class UtlHashLink
{

public:
   UtlHashLink();
   UtlHashLink( UtlHashValue hv );

   inline UtlHashValue hash();

   ~UtlHashLink();

   UtlHashLink * m_next;
   UtlHashValue m_hash;
};

// ------------------------------------------------------------
//  UtlHashBaseIterator
// ------------------------------------------------------------

class UtlHashBaseIterator
{

   friend class UtlHashBase;

public:
   UtlHashBaseIterator();
   UtlHashBaseIterator( const UtlHashBaseIterator& other );
   UtlHashBaseIterator( UtlHashLink ** pcurr,
                        UtlHashLink ** pend,
                        UtlHashLink * node );

   inline UtlHashLink * node() const
   {
      return m_pnode;
   }

   inline short valid() const
   {
      return m_pnode != NULL;
   }

   UtlHashBaseIterator & operator = ( const UtlHashBaseIterator & other );
   short operator == ( const UtlHashBaseIterator & other ) const;
   short operator != ( const UtlHashBaseIterator & other ) const;
   void incr();

   ~UtlHashBaseIterator();

private:
   UtlHashLink ** m_pbuckets;
   UtlHashLink ** m_pend;
   UtlHashLink * m_pnode;

   UtlHashLink ** pbucket() const
   {
      return m_pbuckets;
   }

   UtlHashLink ** pend() const
   {
      return m_pend;
   }
};



// ------------------------------------------------------------
//  UtlHashBase
// ------------------------------------------------------------

class UtlHashBase
{

   friend class UtlHashBaseIterator;

public:
   inline unsigned long size();

private:
   UtlHashBase( const UtlHashBase & other );
   UtlHashBase & operator = ( const UtlHashBase & other );


protected:
   typedef void (*UtlHashDeleteProc) ( UtlHashLink * entry );
   typedef int (*UtlHashEqualsProc) ( UtlHashLink * a, UtlHashLink * b );

   UtlHashEqualsProc m_EqualEntries;
   UtlHashDeleteProc m_DeleteEntry;

   UtlHashLink ** m_buckets;
   unsigned long m_tablesize;
   unsigned long m_count;
   unsigned long m_growthFactor;
   unsigned long m_triggerLimit;
   float m_densityLimit;

   UtlHashBase( unsigned long numBuckets,
                UtlHashEqualsProc eFunc,
                UtlHashDeleteProc dFunc,
                unsigned long growthFactor = UtlHashBase_GROWTH,
                float densityLimit = UtlHashBase_DENSITY );

   UtlHashBaseIterator find( UtlHashLink * entry );
   UtlHashBaseIterator insert( UtlHashLink * entry );

   void remove
      ( const UtlHashBaseIterator & iter );

   void remove
      ( const UtlHashBaseIterator & begin,
            const UtlHashBaseIterator & end );

   void resize( unsigned long newSize );

   void remove_all();

   inline UtlHashLink ** find_bucket( UtlHashValue hash );

   inline UtlHashLink * next_hit( UtlHashLink * node );

   UtlHashLink ** begin_bucket();

   inline UtlHashLink ** end_bucket();

   ~UtlHashBase();
};


// ------------------------------------------------------------
//  UtlMap template
// ------------------------------------------------------------

template <class KEY, class VAL> class UtlMap : private UtlHashBase
{
public:

   class KeyValuePair : public UtlHashLink
   {

   public:

      KeyValuePair () {}

      KeyValuePair (KEY key, UtlHashValue hv)
         : UtlHashLink (hv), m_key (key)
      {}

      VAL operator * () const
      {
         return m_value;
      }

      KEY m_key;
      VAL m_value;
   };

   class iterator : public UtlHashBaseIterator
   {
      friend class KeyValuePair;

   public:

      iterator() : UtlHashBaseIterator()
      {}

      iterator( const UtlHashBaseIterator & baseIter )
            : UtlHashBaseIterator(baseIter)
      {}

      iterator(UtlHashLink ** pcur, UtlHashLink ** pend, UtlHashLink * pnode )
            : UtlHashBaseIterator(pcur, pend, pnode)
      {}

      ~iterator() {}

      iterator & operator = (const iterator & other)
      {
         UtlHashBaseIterator::operator = (other);
         return *this;
      }

      iterator & operator ++ ()
      {
         incr();
         return *this;
      }

      iterator operator ++ (int)
      {
         iterator tmp(*this);
         incr();
         return tmp;
      }

      inline VAL & operator * () const
      {
         return ((KeyValuePair*)node())->m_value;
      }

      inline KEY & key ()
      {
         return ((KeyValuePair*)node())->m_key;
      }

      inline VAL & value ()
      {
         return ((KeyValuePair*)node())->m_value;
      }
   };

   typedef UtlHashValue (*HashFunction) (const KEY &);

   UtlMap( HashFunction hashFunc,
           unsigned long tablesize = UtlHashBase_SIZE,
           unsigned long growthFactor = UtlHashBase_GROWTH,
           float densityLimit = UtlHashBase_DENSITY );

   inline VAL & operator [] ( const KEY & key ); // non-stl (Expersoft only)

   iterator begin()
   {
      UtlHashLink * nodeBegin;
      UtlHashLink ** bucketBegin = begin_bucket();

      if ( bucketBegin == end_bucket() )
         nodeBegin = NULL;
      else
         nodeBegin = *bucketBegin;

      return UtlMap<KEY, VAL>::iterator( bucketBegin, end_bucket(), nodeBegin);
   }

   iterator end()
   {
      return TYPENAME UtlMap<KEY, VAL>::iterator
         (end_bucket(), end_bucket(), NULL);
   }

   inline UtlHashValue hash (const KEY & key);   // non-stl (Expersoft only)

   iterator find (const KEY & key)
   {
      UtlHashLink ** fbuck = find_bucket (hash (key));
      UtlHashLink * node = *fbuck;

      while (node)
      {
         if (key == ((KeyValuePair*)node)->m_key)
         {
            TYPENAME UtlMap<KEY, VAL>::iterator tmpIter
               (fbuck, end_bucket(), node);
            return tmpIter;
         }

         node = next_hit (node);
      }

      return end();
   }

   iterator find_all( UtlHashValue hashValue)
   {
      UtlHashLink ** fbuck = find_bucket(hashValue);
      UtlHashLink * node = *fbuck;

      TYPENAME UtlMap<KEY, VAL>::iterator tmpIter
         (fbuck, end_bucket(), node);

      return tmpIter;
   }


   iterator insert (const KEY & key)
   {
      TYPENAME UtlMap<KEY, VAL>::iterator position = find(key);

      if (!position.valid())
      {
         KeyValuePair * newEntry = new KeyValuePair(key, hash(key));
         position = UtlHashBase::insert(newEntry);
      }

      return position;
   }

   void erase(iterator position)
   {

      UtlHashBase::remove
         (position);
   }

   void erase(iterator begin, iterator end)
   {

      UtlHashBase::remove
         (begin, end);
   }

   void erase();         // non-std STL (a la ObjectSpace)
   unsigned long erase( const KEY & key );
   inline unsigned long size();

private:

   UtlMap();             // interface only
   UtlMap( const UtlMap<KEY, VAL> & other );   // interface only
   UtlMap<KEY, VAL> & operator = ( const UtlMap<KEY, VAL> & other ); // interface only

   HashFunction m_Hasher;

   static int EqualHashEntries(KeyValuePair * nodeA, KeyValuePair * nodeB)
   {
      return (nodeA && nodeB ? (nodeA->m_key == nodeB->m_key) : 0);
   }

   static void DeleteHashEntry( KeyValuePair * node )
   {
      delete node;
   }
};



// ------------------------------------------------------------
//  UtlSet template
// ------------------------------------------------------------

template <class KEY>

class UtlSet
         :
         private UtlHashBase
{

public:

   class KeyEntry
            :
            public UtlHashLink
   {

   public:
      KeyEntry()
      {}

      KeyEntry( KEY key, UtlHashValue hv )
            : UtlHashLink(hv), m_key(key)
      {}

      KEY operator * () const
      {
         return m_key;
      }

      KEY m_key;
   };


   class iterator
            :
            public UtlHashBaseIterator
   {

   public:
      iterator() : UtlHashBaseIterator()
      {}

      iterator( const UtlHashBaseIterator & baseIter )
            : UtlHashBaseIterator(baseIter)
      {}

      iterator(UtlHashLink ** pcur, UtlHashLink ** pend, UtlHashLink * pnode )
            : UtlHashBaseIterator(pcur, pend, pnode)
      {}

      ~iterator()
      {}

      iterator & operator = ( const iterator & other )
      {
         UtlHashBaseIterator::operator=(other);
         return *this;
      }

      iterator & operator ++ ()
      {
         incr();
         return *this;
      }

      iterator operator ++ (int)
      {
         iterator tmp(*this);
         incr();
         return tmp;
      }

      inline KEY & operator * () const
      {
         return ((KeyEntry*)node())->m_key;
      }

      inline KEY & key()
      {
         return ((KeyEntry*)node())->m_key;
      }
   };

   typedef UtlHashValue (*HashFunction)(const KEY &);

   UtlSet( HashFunction hashFunc,
           unsigned long tablesize = UtlHashBase_SIZE,
           unsigned long growthFactor = UtlHashBase_GROWTH,
           float densityLimit = UtlHashBase_DENSITY );

   inline short operator [] ( const KEY & key ); // non-stl (Expersoft only)

   iterator begin()
   {
      UtlHashLink * nodeBegin;
      UtlHashLink ** bucketBegin = begin_bucket();

      if ( bucketBegin == end_bucket() )
         nodeBegin = NULL;
      else
         nodeBegin = *bucketBegin;

      return UtlSet<KEY>::iterator( bucketBegin, end_bucket(), nodeBegin);
   }

   iterator end()
   {
      return UtlSet<KEY>::iterator( end_bucket(), end_bucket(), NULL);
   }

   inline UtlHashValue hash( const KEY & key );   // non-stl (Expersoft only)

   iterator find( const KEY & key )
   {
      UtlHashLink ** fbuck = find_bucket(hash(key));
      UtlHashLink * node = *fbuck;

      while (node)
      {
         if (key == ((KeyEntry*)node)->m_key)
         {
            TYPENAME UtlSet<KEY>::iterator iter
               (fbuck, end_bucket(), node);
            return iter;
         }

         node = next_hit(node);
      }

      return end();
   }

   iterator insert( const KEY & key )
   {
      TYPENAME UtlSet<KEY>::iterator position = find(key);

      if (!position.valid())
      {
         KeyEntry * newEntry = new KeyEntry(key, hash(key));
         position = UtlHashBase::insert(newEntry);
      }

      return position;
   }

   void erase( iterator position )
   {

      UtlHashBase::remove
         (position);
   }

   void erase( iterator begin, iterator end )
   {

      UtlHashBase::remove
         (begin, end);
   }

   void erase();         // non-std STL (a la ObjectSpace)
   unsigned long erase( const KEY & key );
   inline unsigned long size();

   ~UtlSet()
{}

private:
   UtlSet();           // interface only
   UtlSet( const UtlSet<KEY> & other );  // interface only
   UtlSet<KEY> & operator = ( const UtlSet<KEY> & other ); // interface only

   HashFunction m_Hasher;

   static int EqualHashEntries( KeyEntry * nodeA, KeyEntry * nodeB )
   {
      return (nodeA && nodeB ? (nodeA->m_key == nodeB->m_key) : 0);
   }

   static void DeleteHashEntry( KeyEntry * node )
   {
      delete node;
   }
};



// ------------------------------------------------------------
//  UtlHashLink inlines
// ------------------------------------------------------------

inline UtlHashValue UtlHashLink::hash ()
{
   return m_hash;
}

// ------------------------------------------------------------
//  UtlHashBase inlines
// ------------------------------------------------------------

inline unsigned long
UtlHashBase::size()
{
   return m_count;
}

inline UtlHashLink **
UtlHashBase::find_bucket( UtlHashValue hash )
{
   return &m_buckets[hash % m_tablesize];
}

inline UtlHashLink *
UtlHashBase::next_hit( UtlHashLink * node )
{
   return node->m_next;
}



inline UtlHashLink **
UtlHashBase::end_bucket()
{
   return &m_buckets[m_tablesize];
}

// ------------------------------------------------------------
//  UtlMap<KEY> template definitions
// ------------------------------------------------------------



template <class KEY, class VAL>
inline unsigned long
UtlMap<KEY, VAL>::size()
{
   return UtlHashBase::size();
}

template <class KEY, class VAL>
inline VAL &
UtlMap<KEY, VAL>::operator [] (const KEY & key)
{
   TYPENAME UtlMap<KEY, VAL>::iterator position = find(key);

   if (!position.valid())
   {
      KeyValuePair * newEntry = new KeyValuePair(key, hash(key));
      position = UtlHashBase::insert(newEntry);
   }

   return *position;
}


template <class KEY, class VAL>
UtlHashValue
UtlMap<KEY, VAL>::hash( const KEY & key )
{
   return m_Hasher(key);
}

template <class KEY, class VAL>
void
UtlMap<KEY, VAL>::erase()
{
   remove_all();
}

template <class KEY, class VAL>
unsigned long
UtlMap<KEY, VAL>::erase( const KEY & key )
{
   TYPENAME UtlMap<KEY, VAL>::iterator position = find(key);

   UtlHashBase::remove
      (position);

   return size();
}

template <class KEY, class VAL>
UtlMap<KEY, VAL>::UtlMap( UtlHashValue (*hashFunc)(const KEY &),
                          unsigned long tablesize,
                          unsigned long growthFactor,
                          float densityLimit)
      :
      UtlHashBase(tablesize,
                  UtlHashEqualsProc(UtlMap<KEY, VAL>::EqualHashEntries),
                  UtlHashDeleteProc(UtlMap<KEY, VAL>::DeleteHashEntry),
                  growthFactor,
                  densityLimit
                 ),
      m_Hasher(hashFunc)
{}

// ------------------------------------------------------------
//  UtlSet<KEY> template definitions
// ------------------------------------------------------------


template <class KEY>
unsigned long
UtlSet<KEY>::size()
{
   return UtlHashBase::size();
}

template <class KEY>
inline short
UtlSet<KEY>::operator [] (const KEY & key)
{
   TYPENAME UtlSet<KEY>::iterator findIter = find(key);
   return findIter.valid();
}

template <class KEY>
UtlHashValue
UtlSet<KEY>::hash( const KEY & key )
{
   return m_Hasher(key);
}

template <class KEY>
void
UtlSet<KEY>::erase()
{
   remove_all();
}

template <class KEY>
unsigned long
UtlSet<KEY>::erase( const KEY & key )
{
   TYPENAME UtlSet<KEY>::iterator position = find(key);

   UtlHashBase::remove
      (position);

   return size();
}


template <class KEY>
UtlSet<KEY>::UtlSet( UtlHashValue (*hashFunc)(const KEY &),
                     unsigned long tablesize,
                     unsigned long growthFactor,
                     float densityLimit)
      :
      UtlHashBase(tablesize,
                  UtlHashEqualsProc(UtlSet<KEY>::EqualHashEntries),
                  UtlHashDeleteProc(UtlSet<KEY>::DeleteHashEntry),
                  growthFactor,
                  densityLimit
                 ),
      m_Hasher(hashFunc)
{}

#define each_xps_hash_base_element(x, array, size) \
   x = &(array)[size]; \
   x-- > (array);

// ------------------------------------------------------------
//  UtlHashLink
// ------------------------------------------------------------

inline
UtlHashLink::UtlHashLink()
      :
      m_next(0),
      m_hash(0)
{}

inline

UtlHashLink::UtlHashLink( UtlHashValue hv )
      :
      m_next(0),
      m_hash(hv)
{}

inline
UtlHashLink::~UtlHashLink()
{}


// ------------------------------------------------------------
//  UtlHashBaseIterator
// ------------------------------------------------------------
inline
UtlHashBaseIterator::UtlHashBaseIterator()
      :
      m_pbuckets(0),
      m_pend(0),
      m_pnode(0)
{}

inline
UtlHashBaseIterator::UtlHashBaseIterator(const UtlHashBaseIterator& other)
      :
      m_pbuckets(other.m_pbuckets),
      m_pend(other.m_pend),
      m_pnode(other.m_pnode)
{}

inline
UtlHashBaseIterator::UtlHashBaseIterator( UtlHashLink ** ptable, UtlHashLink ** pend, UtlHashLink * node )
      :
      m_pbuckets(ptable),
      m_pend(pend),
      m_pnode(node)
{}

inline
UtlHashBaseIterator::~UtlHashBaseIterator()
{}


inline UtlHashBaseIterator &
UtlHashBaseIterator::operator = ( const UtlHashBaseIterator & other )
{
   m_pbuckets = other.m_pbuckets;
   m_pend = other.m_pend;
   m_pnode = other.m_pnode;
   return *this;
}


inline short
UtlHashBaseIterator::operator == ( const UtlHashBaseIterator & other ) const
{
   return ( (m_pbuckets == other.m_pbuckets) &&
            (m_pend == other.m_pend) &&
            (m_pnode == other.m_pnode));
}


inline short
UtlHashBaseIterator::operator != ( const UtlHashBaseIterator & other ) const
{
   return ( (m_pbuckets != other.m_pbuckets) ||
            (m_pend != other.m_pend) ||
            (m_pnode != other.m_pnode));
}


inline void
UtlHashBaseIterator::incr()
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

      m_pnode = (m_pbuckets == m_pend ? ((UtlHashLink*) NULL) : *m_pbuckets);
   }
}




// ------------------------------------------------------------
//  UtlHashBase
// ------------------------------------------------------------
inline
UtlHashBase::UtlHashBase(unsigned long buckets,
                         UtlHashEqualsProc eProc,
                         UtlHashDeleteProc dProc,
                         unsigned long growthFactor,
                         float densityLimit)
      :
      m_EqualEntries(eProc),
      m_DeleteEntry(dProc),
      m_tablesize(buckets),
      m_count(0),
      m_growthFactor((2 > growthFactor ? 2 : growthFactor)),
      m_triggerLimit((unsigned long)(buckets * densityLimit) ),
      m_densityLimit(densityLimit)
{
   m_buckets = (UtlHashLink **)new UtlHashLink * [m_tablesize];
   UtlHashLink ** ptr = m_buckets;

   for ( each_xps_hash_base_element( ptr, m_buckets, m_tablesize))
      *ptr = NULL;
}

inline
UtlHashBase::~UtlHashBase()
{
   UtlHashLink ** list;

   for (each_xps_hash_base_element(list, m_buckets, m_tablesize))
      while ( *list )
      {
         UtlHashLink *item = *list;
         *list = item->m_next;
         m_DeleteEntry(item);
      }

   delete []m_buckets;
}


inline UtlHashBaseIterator
UtlHashBase::find( UtlHashLink * entry )
{
   UtlHashLink * item;
   UtlHashLink ** list;

   if (!entry)
      return UtlHashBaseIterator(end_bucket(), end_bucket(), NULL);

   list = find_bucket(entry->m_hash);

   for ( item = *list; item != NULL; item = next_hit(item) )
      if (m_EqualEntries(item, entry))
         return UtlHashBaseIterator(list, end_bucket(), item);

   return UtlHashBaseIterator(end_bucket(), end_bucket(), NULL);
}


inline UtlHashBaseIterator
UtlHashBase::insert( UtlHashLink * entry )
{
   if ( m_count >= m_triggerLimit )
   {
      resize( m_tablesize * m_growthFactor + 1 );
      m_triggerLimit = (unsigned long)(m_tablesize * m_densityLimit);
   }

   UtlHashLink ** list = find_bucket(entry->hash());
   entry->m_next = *list;
   *list = entry;

   m_count++;
   return UtlHashBaseIterator(list, end_bucket(), entry);
}



inline void

UtlHashBase::remove
   ( const UtlHashBaseIterator & iter )
{
   if ( iter.pbucket() != iter.pend() )
   {
      UtlHashLink * item;
      UtlHashLink * priorItem = NULL;

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



inline void

UtlHashBase::remove
   ( const UtlHashBaseIterator & beginIter,
         const UtlHashBaseIterator & endIter )
{
   if ( beginIter.valid() )
   {
      UtlHashLink ** bucketOne = begin_bucket();

      if ( beginIter.pbucket() == bucketOne &&
            beginIter.node() == *bucketOne &&
            endIter.pbucket() == end_bucket() )
      {
         remove_all();  // faster if we know to remove all items
      }
      else
      {
         UtlHashBaseIterator lastIter;
         UtlHashBaseIterator iter = beginIter;

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



inline void
UtlHashBase::resize( unsigned long newSize )
{
   UtlHashLink ** newTable;
   UtlHashLink ** oldTable;
   UtlHashLink ** bucket;
   unsigned long oldSize;

   newTable = (UtlHashLink **)new UtlHashLink * [newSize];

   for (each_xps_hash_base_element( bucket, newTable, newSize))
      *bucket = NULL;

   oldTable = m_buckets;

   oldSize = m_tablesize;

   m_buckets = newTable;  // find method uses member vars

   m_tablesize = newSize;

   UtlHashLink ** list;

   UtlHashLink * item;

   UtlHashLink * nextItem;

   for (each_xps_hash_base_element(list, oldTable, oldSize))
      for ( item = *list; item != NULL; item = nextItem )
      {
         nextItem = item->m_next;
         bucket = find_bucket(item->hash());
         item->m_next = *bucket;
         *bucket = item;
      }

   delete [] oldTable;
}


inline void
UtlHashBase::remove_all()
{
   UtlHashLink ** list;
   UtlHashLink * item;
   UtlHashLink * nextItem;

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




inline UtlHashLink **
UtlHashBase::begin_bucket()
{
   UtlHashLink ** bucket;
   UtlHashLink ** limit = end_bucket();

   for (bucket = m_buckets; bucket < limit; bucket++ )
      if ( *bucket )
         return bucket;

   return limit;
}


#endif
