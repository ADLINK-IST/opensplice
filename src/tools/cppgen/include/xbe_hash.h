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
#ifndef _DDS_HASH_H_
#define _DDS_HASH_H_

/******************************************************************
 
   DDSMap<KEY,VAL> and DDSSet<KEY> are two, templatized collections
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
 delete or DDS::release the pointer.   If value is an object it's dtor
 will be called when the item is erased.
 
   Here is an example hashing function for strings and simple one for chars...
 
      static unsigned long
      hash_str ( const DDSStd::String &str )
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
 
  typedef  DDSMap<DDSStd::String, int>   WordCounter;
  typedef  DDSMap<char, int>       CharCounter;
  typedef  DDSSet<char>            CharSet;
 
  WordCounter             words ( hash_str );
  CharCounter             letters ( hash_char );
  CharSet                 letter_set( hash_char );
  char                    letter;
 
  DDSStd::String str = "";
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


#include "idl.h"

#define DDSHashBase_GROWTH 2
#define DDSHashBase_SIZE  31
#define DDSHashBase_DENSITY 0.80

// ------------------------------------------------------------
//  DDSHashLink
// ------------------------------------------------------------


typedef unsigned long DDSHashValue;

class DDSHashLink
{
public:

   DDSHashLink () : m_next (0), m_hash (0) {}

   DDSHashLink (DDSHashValue hv) : m_next (0), m_hash (hv) {}

   ~DDSHashLink () {}

   inline DDSHashValue hash ()
   {
      return m_hash;
   }

   DDSHashLink * m_next;
   DDSHashValue m_hash;
};

// ------------------------------------------------------------
//  DDSHashBaseIterator
// ------------------------------------------------------------

class DDSHashBaseIterator
{
   friend class DDSHashBase;

public:

   DDSHashBaseIterator ();
   DDSHashBaseIterator (const DDSHashBaseIterator & other);
   DDSHashBaseIterator
   (
       DDSHashLink ** pcurr,
       DDSHashLink ** pend,
       DDSHashLink * node
   );
   virtual ~DDSHashBaseIterator () {};

   inline DDSHashLink * node () const
   {
      return m_pnode;
   }

   inline short valid () const
   {
      return m_pnode != NULL;
   }

   DDSHashBaseIterator & operator = (const DDSHashBaseIterator & other);
   short operator == (const DDSHashBaseIterator & other) const;
   short operator != (const DDSHashBaseIterator & other) const;
   void incr ();

private:

   DDSHashLink ** m_pbuckets;
   DDSHashLink ** m_pend;
   DDSHashLink *  m_pnode;

   DDSHashLink ** pbucket () const
   {
      return m_pbuckets;
   }

   DDSHashLink ** pend () const
   {
      return m_pend;
   }
};


// ------------------------------------------------------------
//  DDSHashBase
// ------------------------------------------------------------

class DDSHashBase
{
   friend class DDSHashBaseIterator;

public:

   inline unsigned long size ();

private:

   DDSHashBase (const DDSHashBase & other);
   DDSHashBase & operator = (const DDSHashBase & other);

protected:

   typedef void (*DDSHashDeleteProc) (DDSHashLink * entry);
   typedef int (*DDSHashEqualsProc) (DDSHashLink * a, DDSHashLink * b);

   DDSHashEqualsProc m_EqualEntries;
   DDSHashDeleteProc m_DeleteEntry;

   DDSHashLink ** m_buckets;
   DDS::ULong m_tablesize;
   unsigned long m_count;
   DDS::ULong m_growthFactor;
   DDS::ULong m_triggerLimit;
   float m_densityLimit;

   DDSHashBase
   (
      DDS::ULong numBuckets,
      DDSHashEqualsProc eFunc,
      DDSHashDeleteProc dFunc,
      DDS::ULong growthFactor = DDSHashBase_GROWTH,
      float densityLimit = DDSHashBase_DENSITY
   );

   DDSHashBaseIterator find (DDSHashLink * entry);
   DDSHashBaseIterator insert (DDSHashLink * entry);

   void remove (const DDSHashBaseIterator & iter);

   void remove
   (
      const DDSHashBaseIterator & begin,
      const DDSHashBaseIterator & end 
   );

   void resize (DDS::ULong newSize);

   void remove_all ();

   inline DDSHashLink ** find_bucket (DDSHashValue hash);

   inline DDSHashLink * next_hit (DDSHashLink * node);

   DDSHashLink ** begin_bucket ();

   inline DDSHashLink ** end_bucket ();

   virtual ~DDSHashBase ();
};

template <class KEY, class VAL> class KeyValuePair
   : public DDSHashLink
{
public:

   KeyValuePair () {}

   KeyValuePair (KEY key, DDSHashValue hv)
      : DDSHashLink (hv), m_key (key)
   {}

   VAL operator * () const
   {
      return m_value;
   }

   KEY m_key;
   VAL m_value;
};

template <class KEY, class VAL> class DDSMapIterator
   : public DDSHashBaseIterator
{
public:

   DDSMapIterator () : DDSHashBaseIterator () {}

   DDSMapIterator (const DDSHashBaseIterator & baseIter)
      : DDSHashBaseIterator (baseIter)
   {}

   DDSMapIterator (DDSHashLink ** pcur, DDSHashLink ** pend, DDSHashLink * pnode)
      : DDSHashBaseIterator (pcur, pend, pnode)
   {}

   ~DDSMapIterator () {}

   DDSMapIterator & operator = (const DDSMapIterator & other)
   {
      DDSHashBaseIterator::operator = (other);
      return *this;
   }

   DDSMapIterator & operator ++ ()
   {
      incr ();
      return *this;
   }

   DDSMapIterator operator ++ (int)
   {
      DDSMapIterator<KEY, VAL> tmp (*this);
      incr ();
      return tmp;
   }

   inline VAL & operator * () const
   {
      return ((KeyValuePair<KEY, VAL> *) node())->m_value;
   }

   inline KEY & key ()
   {
      return ((KeyValuePair<KEY, VAL> *) node())->m_key;
   }

   inline VAL & value ()
   {
      return ((KeyValuePair<KEY, VAL> *) node())->m_value;
   }
};


// ------------------------------------------------------------
//  DDSMap template
// ------------------------------------------------------------

template <class KEY, class VAL> class DDSMap
   : private DDSHashBase
{
public:

   typedef DDSMapIterator<KEY, VAL> iterator;

   typedef DDSHashValue (*HashFunction)(const KEY &);


   DDSMap
   (
      HashFunction hashFunc,
      DDS::ULong tablesize = DDSHashBase_SIZE,
      DDS::ULong growthFactor = DDSHashBase_GROWTH,
      float densityLimit = DDSHashBase_DENSITY
   );

   inline VAL & operator [] ( const KEY & key ); // non-stl (Expersoft only)

   iterator begin()
   {
      DDSHashLink * nodeBegin;
      DDSHashLink ** bucketBegin = begin_bucket();

      if ( bucketBegin == end_bucket() )
         nodeBegin = NULL;
      else
         nodeBegin = *bucketBegin;

      return TYPENAME DDSMap<KEY, VAL>::iterator( bucketBegin, end_bucket(), nodeBegin);
   }

   iterator end()
   {
      return TYPENAME DDSMap<KEY, VAL>::iterator( end_bucket(), end_bucket(), NULL);
   }

   inline DDSHashValue hash( const KEY & key );   // non-stl

   iterator find( const KEY & key )
   {
      DDSHashLink ** fbuck = find_bucket(hash(key));
      DDSHashLink * node = *fbuck;

      while (node)
      {
         if (key == ((KeyValuePair<KEY, VAL>*)node)->m_key)
         {
            TYPENAME DDSMap<KEY, VAL>::iterator tmpIter(fbuck, end_bucket(), node);
            return tmpIter;
         }

         node = next_hit(node);
      }

      return end();
   }

   iterator find_all( DDSHashValue hashValue)
   {
      DDSHashLink ** fbuck = find_bucket(hashValue);
      DDSHashLink * node = *fbuck;

      TYPENAME DDSMap<KEY, VAL>::iterator tmpIter(fbuck, end_bucket(), node);

      return tmpIter;
   }


   iterator insert (const KEY & key)
   {
      TYPENAME DDSMap<KEY, VAL>::iterator position = find(key);

      if (!position.valid())
      {
         KeyValuePair<KEY, VAL> * newEntry = new KeyValuePair<KEY, VAL> (key, hash(key));
         position = DDSHashBase::insert(newEntry);
      }

      return position;
   }

   void erase (iterator position)
   {
      DDSHashBase::remove (position);
   }

   void erase (iterator begin, iterator end)
   {
      DDSHashBase::remove (begin, end);
   }

   void erase();         // non-std STL (a la ObjectSpace)
   unsigned long erase( const KEY & key );
   inline unsigned long size();

   typedef DDSMap<KEY, VAL> tempParamType;

private:

   DDSMap();             // interface only
   DDSMap( const DDSMap<KEY, VAL> & other );   // interface only
   DDSMap<KEY, VAL> & operator = ( const DDSMap<KEY, VAL> & other ); // interface only

   HashFunction m_Hasher;

   static int EqualHashEntries
   (
      KeyValuePair<KEY, VAL> * nodeA,
      KeyValuePair<KEY, VAL> * nodeB
   )
   {
      return (nodeA && nodeB ? (nodeA->m_key == nodeB->m_key) : 0);
   }

   static void DeleteHashEntry (KeyValuePair<KEY, VAL> * node )
   {
      if (node)
      {
         delete node;
      }
   }
};



// ------------------------------------------------------------
//  DDSSet template
// ------------------------------------------------------------

template <class KEY> class KeyEntry 
   : public DDSHashLink
{
public:

   KeyEntry () {}

   KeyEntry (KEY key, DDSHashValue hv)
      : DDSHashLink (hv), m_key (key)
   {}

   KEY operator * () const
   {
      return m_key;
   }

   KEY m_key;
};

template <class KEY> class DDSSetIterator
   : public DDSHashBaseIterator
{
public:

   DDSSetIterator () : DDSHashBaseIterator ()
   {}

   DDSSetIterator (const DDSHashBaseIterator & baseIter)
      : DDSHashBaseIterator (baseIter)
   {}

   DDSSetIterator (DDSHashLink ** pcur, DDSHashLink ** pend, DDSHashLink * pnode)
      : DDSHashBaseIterator (pcur, pend, pnode)
   {}

   ~DDSSetIterator () {}

   DDSSetIterator & operator = (const DDSSetIterator & other )
   {
      DDSHashBaseIterator::operator = (other);
      return *this;
   }

   DDSSetIterator & operator ++ ()
   {
      incr ();
      return *this;
   }

   DDSSetIterator operator ++ (int)
   {
      DDSSetIterator<KEY> tmp (*this);
      incr ();
      return tmp;
   }

   inline KEY & operator * () const
   {
      return ((KeyEntry<KEY> *)node())->m_key;
   }

   inline KEY & key()
   {
      return ((KeyEntry<KEY> *)node())->m_key;
   }
};

template <class KEY> class DDSSet
   : private DDSHashBase
{
public:

   typedef DDSSetIterator<KEY> iterator;

   typedef DDSHashValue (*HashFunction)(const KEY &);

   DDSSet
   (
      HashFunction hashFunc,
      DDS::ULong tablesize = DDSHashBase_SIZE,
      DDS::ULong growthFactor = DDSHashBase_GROWTH,
      float densityLimit = DDSHashBase_DENSITY
   );

   inline short operator [] ( const KEY & key ); // non-stl (Expersoft only)

   iterator begin()
   {
      DDSHashLink * nodeBegin;
      DDSHashLink ** bucketBegin = begin_bucket();

      if ( bucketBegin == end_bucket() )
         nodeBegin = NULL;
      else
         nodeBegin = *bucketBegin;

      return TYPENAME DDSSet<KEY>::iterator( bucketBegin, end_bucket(), nodeBegin);
   }

   iterator end()
   {
      return TYPENAME DDSSet<KEY>::iterator( end_bucket(), end_bucket(), NULL);
   }

   inline DDSHashValue hash( const KEY & key );   // non-stl (Expersoft only)

   iterator find (const KEY & key)
   {
      DDSHashLink ** fbuck = find_bucket (hash(key));
      DDSHashLink * node = *fbuck;

      while (node)
      {
         if (key == ((KeyEntry<KEY> *)node)->m_key)
         {
            TYPENAME DDSSet<KEY>::iterator iter (fbuck, end_bucket (), node);
            return iter;
         }

         node = next_hit(node);
      }

      return end();
   }

   iterator insert (const KEY & key)
   {
      TYPENAME DDSSet<KEY>::iterator position = find (key);

      if (!position.valid())
      {
         KeyEntry<KEY> * newEntry = new KeyEntry<KEY> (key, hash (key));
         position = DDSHashBase::insert (newEntry);
      }

      return position;
   }

   void erase (iterator position)
   {
      DDSHashBase::remove (position);
   }

   void erase (iterator begin, iterator end)
   {
      DDSHashBase::remove (begin, end);
   }

   void erase();         // non-std STL (a la ObjectSpace)
   unsigned long erase( const KEY & key );
   inline unsigned long size();

   ~DDSSet()
{}

private:

   DDSSet ();
   DDSSet (const DDSSet<KEY> & other);
   DDSSet<KEY> & operator = (const DDSSet<KEY> & other );

   HashFunction m_Hasher;

   static int EqualHashEntries (KeyEntry<KEY> * nodeA, KeyEntry<KEY> * nodeB)
   {
      return (nodeA && nodeB ? (nodeA->m_key == nodeB->m_key) : 0);
   }

   static void DeleteHashEntry (KeyEntry<KEY> * node)
   {
      if (node)
      {
         node->_release ();
      }
   }
};


// ------------------------------------------------------------
//  DDSHashBase inlines
// ------------------------------------------------------------

inline unsigned long DDSHashBase::size ()
{
   return m_count;
}

inline DDSHashLink ** DDSHashBase::find_bucket (DDSHashValue hash)
{
   return &m_buckets[hash % m_tablesize];
}

inline DDSHashLink * DDSHashBase::next_hit (DDSHashLink * node)
{
   return node->m_next;
}

inline DDSHashLink ** DDSHashBase::end_bucket ()
{
   return &m_buckets[m_tablesize];
}

// ------------------------------------------------------------
//  DDSMap<KEY> template definitions
// ------------------------------------------------------------


template <class KEY, class VAL> inline unsigned long DDSMap<KEY, VAL>::size ()
{
   return DDSHashBase::size ();
}

template <class KEY, class VAL>
inline VAL &
DDSMap<KEY, VAL>::operator [] (const KEY & key)
{
   TYPENAME DDSMap<KEY, VAL>::iterator position = find(key);

   if (!position.valid())
   {
      KeyValuePair<KEY, VAL> * newEntry = new KeyValuePair<KEY, VAL> (key, hash(key));
      position = DDSHashBase::insert(newEntry);
   }

   return *position;
}


template <class KEY, class VAL>
DDSHashValue
DDSMap<KEY, VAL>::hash( const KEY & key )
{
   return m_Hasher(key);
}

template <class KEY, class VAL>
void
DDSMap<KEY, VAL>::erase()
{
   remove_all();
}

template <class KEY, class VAL>
unsigned long
DDSMap<KEY, VAL>::erase( const KEY & key )
{
   TYPENAME DDSMap<KEY, VAL>::iterator position = find(key);

   DDSHashBase::remove
      (position);

   return size();
}

template <class KEY, class VAL>
DDSMap<KEY, VAL>::DDSMap( DDSHashValue (*hashFunc)(const KEY &),
                          DDS::ULong tablesize,
                          DDS::ULong growthFactor,
                          float densityLimit)
      :
      DDSHashBase(tablesize,
                  DDSHashEqualsProc(DDSMap<KEY, VAL>::EqualHashEntries),
                  DDSHashDeleteProc(DDSMap<KEY, VAL>::DeleteHashEntry),
                  growthFactor,
                  densityLimit
                 ),
      m_Hasher(hashFunc)
{}

// ------------------------------------------------------------
//  DDSSet<KEY> template definitions
// ------------------------------------------------------------


template <class KEY> unsigned long DDSSet<KEY>::size ()
{
   return DDSHashBase::size ();
}

template <class KEY> inline short DDSSet<KEY>::operator [] (const KEY & key)
{
   TYPENAME DDSSet<KEY>::iterator findIter = find(key);
   return findIter.valid();
}

template <class KEY> DDSHashValue DDSSet<KEY>::hash (const KEY & key)
{
   return m_Hasher(key);
}

template <class KEY> void DDSSet<KEY>::erase ()
{
   remove_all ();
}

template <class KEY> unsigned long DDSSet<KEY>::erase (const KEY & key)
{
   TYPENAME DDSSet<KEY>::iterator position = find (key);

   DDSHashBase::remove (position);

   return size ();
}

template <class KEY> DDSSet<KEY>::DDSSet
(
   DDSHashValue (*hashFunc)(const KEY &),
   DDS::ULong tablesize,
   DDS::ULong growthFactor,
   float densityLimit
)
:
   DDSHashBase
   (
      tablesize,
      DDSHashEqualsProc(DDSSet<KEY>::EqualHashEntries),
      DDSHashDeleteProc(DDSSet<KEY>::DeleteHashEntry),
      growthFactor,
      densityLimit
   ),
   m_Hasher (hashFunc)
{}

#define each_xps_hash_base_element(x, array, size) \
   x = &(array)[size]; \
   x-- > (array);

#endif
