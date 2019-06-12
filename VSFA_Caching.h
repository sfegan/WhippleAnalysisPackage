//-*-mode:c++; mode:font-lock;-*-

#ifndef VECTORSTREAMFA_CACHING_H
#define VECTORSTREAMFA_CACHING_H

#define __STL_USE_NAMESPACES

#include <string>
#include <iostream>


#include "VSFA.h"

namespace NS_Analysis
{
  using std::string;
  using std::ostream;

  /////////////////////////////////////////////////////////////////////////////
  ////////////////// VSFA_Caching - Caching File Association //////////////////
  /////////////////////////////////////////////////////////////////////////////

  template<class T> class SimpleCache;

  template<class T>
  class VSFA_Caching: public VSFA<T>
  {
  private:
    VSFA_Caching();
    VSFA_Caching(const VSFA_Caching&);
    VSFA_Caching& operator=(const VSFA_Caching &);
    
  private:
    // The file accessor
    MPtr<VSFA<T> >               m_fa;

    // The read cache
    SimpleCache<T>*              m_rcache;
    
    // The write cache
    SimpleCache<T>*              m_wcache;
  
    // Useful things
    const string errormsg(const string& err) const 
    { return string("VSFA_Caching: ")+id()+": "+err; }
    void throwerror(const string& func, const string& errm) const
    { 
      VSFA_Error err(func);
      err.stream() << "VSFA_Caching(" << id() << "): " << errm << '\n';
      throw(err); 
    }
    
  public:
    VSFA_Caching(VSFA<T>* fa, 
		 fasize_type wcache_length, fasize_type rcache_length,
		 bool manage_fa) 
      throw(VSFA_Error);
    ~VSFA_Caching();
    
    const string& id() const;
    fasize_type size() const;
    bool unlimited() const;
    fasize_type maxsize() const;
    fasize_type read(fasize_type start, T* p, fasize_type count);
    fasize_type write(fasize_type start, const T* p, fasize_type count);
    void sync();
  };

///////////////////////////////////////////////////////////////////////////////
////// SimpleCache: provides some simple routines for read & write cache //////
///////////////////////////////////////////////////////////////////////////////

  template <class T> class SimpleCache
  {
  private:
    T* m_cache;
    fasize_type m_length;
    fasize_type m_start;
    fasize_type m_occupancy;
    bool m_valid;
    
  public:
    SimpleCache(fasize_type l);
    ~SimpleCache() { if(m_length)delete[] m_cache; }
    void zero() { m_occupancy = m_start = 0; m_valid = false; }
    void fill(fasize_type st, fasize_type occ) 
    { m_occupancy=occ; m_start=st; m_valid=true; }
    
    bool valid() const { return m_valid; }
    fasize_type length() const { return m_length; }
    fasize_type start() const { return valid() ? m_start : 0; }
    fasize_type occupancy() const { return valid() ? m_occupancy : 0; }
    fasize_type end() const { return start()+occupancy()-1; }
    
    // Test whether any of the users requested elements are in the cache
    bool have_overlap(fasize_type first, fasize_type count) const;
    fasize_type overlapping_first(fasize_type first, fasize_type count) const;
    fasize_type overlapping_last(fasize_type first, fasize_type count) const;
    fasize_type overlapping_count(fasize_type first, fasize_type count) const;
    
    // Test whether we can store a range of elements in the cache
    bool can_take_all_these(fasize_type first, fasize_type count) const;
    bool take_these(fasize_type first, fasize_type count, const T* p);
    
    const T* element(fasize_type first) const { return cache()+(first-start()); }
    T* element(fasize_type first) { return cache()+(first-start()); }
    
    const T* cache() const { return m_cache; }
    T* cache() { return m_cache; }
    
    // copy potentaially useful elements from another cache
    void update(const SimpleCache& other);
  };
  
  template <class T> 
  ostream& 
  operator<<(ostream& stream, const SimpleCache<T>& cache)
  {
    stream << "cache(";
    if(!cache.valid()) stream << "INVALID,";
    stream << cache.start() << ',' 
	   << cache.occupancy() << '/'
	   << cache.length() << ')';
    return stream;
  }
  
} // namespace NS_Analysis

template <class T> 
NS_Analysis::SimpleCache<T>::
SimpleCache(fasize_type s)
{
  m_length=s;
  zero();
  if(m_length)m_cache=new T[m_length];
  else m_cache=0;
}

template <class T> 
inline bool
NS_Analysis::SimpleCache<T>::
have_overlap(fasize_type first, fasize_type count) const
{
  if(!valid())return false;
  fasize_type last=first+count-1;
  if((first < start()) && (last<start()))return false;
  if(first > end())return false;
  return true;
}

template <class T> 
inline NS_Analysis::fasize_type 
NS_Analysis::SimpleCache<T>::
overlapping_first(fasize_type first, fasize_type count) const
{
  if(first < start())return start();
  else return first;
}

template <class T> 
inline NS_Analysis::fasize_type 
NS_Analysis::SimpleCache<T>::
overlapping_last(fasize_type first, fasize_type count) const
{
  fasize_type last=first+count-1;
  if(last > end())return end();
  else return last;
}

template <class T> 
inline NS_Analysis::fasize_type 
NS_Analysis::SimpleCache<T>::
overlapping_count(fasize_type first, fasize_type count) const
{
  return overlapping_last(first,count)-overlapping_first(first,count)+1;
}

template <class T>
inline bool
NS_Analysis::SimpleCache<T>::
can_take_all_these(fasize_type first, fasize_type count) const
{
  if(!valid())return(count<length()); // if we're empty the decision is easy
  if(first < start())return false;
  if(first > start()+occupancy())return false; // check for possible holes
  return(first+count-start() <= length()); // have we the space
}

template <class T>
inline bool
NS_Analysis::SimpleCache<T>::
take_these(fasize_type first, fasize_type count, const T* p)
{
#ifdef FA_DEBUG
  cerr << "SimpleCache::take_these(" << first << ',' << count << "); ";
  cerr << *this << '\n';
#endif 
  if(can_take_all_these(first,count) == false)return false;
  if(!valid())fill(first,0);
  memcpy(element(first),p,sizeof(T)*count);
  fill(start(),first-start()+count);
  return true;
}

template <class T>
inline void 
NS_Analysis::SimpleCache<T>::
update(const SimpleCache& other)
{
  if(!valid())
    {
      memcpy(cache(),other.cache(),sizeof(T)*other.occupancy());
      fill(other.start(),other.occupancy());
      return;
    }
  
  // test whether any of their's overlaps our cache's available space
  if(other.have_overlap(start(),length()) == false)return;
  
  // check to see if there would be a hole in our cache if we copied
  if(other.start() > start()+occupancy())return;
  
  // ok copy the other cache
  fasize_type first=other.overlapping_first(start(),length());
  fasize_type count=other.overlapping_count(start(),length());
  
  // memcpy.... in a c++ program.... its a travesty. well who cares ?
  // other modules in this library (the H5 one for instance) use
  // "objects" as mrere pieces of memory so whi can't we
  memcpy(element(first),other.element(first),sizeof(T)*count);
  
  fill(start(),first-start()+count);
}

///////////////////////////////////////////////////////////////////////////////
///////////////// VSFA_Caching - Member function definitions //////////////////
///////////////////////////////////////////////////////////////////////////////

template <class T>
NS_Analysis::VSFA_Caching<T>::
VSFA_Caching(VSFA<T>* fa, 
	     fasize_type wcache_length, fasize_type rcache_length,
	     bool manage_fa) 
  throw(VSFA_Error): VSFA<T>(), m_fa(fa,manage_fa)
{ 
  // Set up the write cache
  m_wcache=new SimpleCache<T>(wcache_length);

  // Set up the read cache
  m_rcache=new SimpleCache<T>(rcache_length);

  setTVersion(m_fa->tVersion());
}


template <class T>
NS_Analysis::VSFA_Caching<T>::
~VSFA_Caching()
{ 
  sync();
  m_fa.reset();
  delete m_wcache;
  delete m_rcache;
}

template <class T>
inline const std::string&
NS_Analysis::VSFA_Caching<T>::
id() const
{
  return m_fa->id();
}

template <class T>
inline bool
NS_Analysis::VSFA_Caching<T>::
unlimited() const
{
  return m_fa->unlimited();
}

template <class T>
inline NS_Analysis::fasize_type 
NS_Analysis::VSFA_Caching<T>::
maxsize() const
{
  return m_fa->maxsize();
}

template <class T>
NS_Analysis::fasize_type 
NS_Analysis::VSFA_Caching<T>::
size() const
{
#ifdef FA_DEBUG
  cerr << "VSFA_Caching::size(); w" << *m_wcache << '\n';
#endif 

  fasize_type fasize=m_fa->size(); 
  fasize_type wcsize=m_wcache->start()+m_wcache->occupancy(); // == 0 if not valid
  return fasize > wcsize ? fasize : wcsize;
}

template <class T>
NS_Analysis::fasize_type
NS_Analysis::VSFA_Caching<T>::
read(fasize_type start, T* p, fasize_type count)
{
#ifdef FA_DEBUG
  cerr << "VSFA_Caching::read(" << start << ',' << count << "); ";
  cerr << 'r' << *m_rcache << "; w" << *m_wcache << '\n';
#endif 

  if(!VSFA<T>::read_ok(start,count))
    {
      VSFA_RangeError err("VSFA_Caching<>::read",start,count,size(),maxsize());
      err.stream()<< "VSFA_Caching(" <<id() 
		  << "): read: parameters out of range";
      throw(err);
    }
  if(count==0)return 0;
  
  // if we are requested to read more than half the cache size in one
  // go then just do it directly (after flushing the write cache)
  if(count > m_rcache->length()/2)
    {
      sync();
      return m_fa->read(start,p,count);
    }

  // OK so we have to negotiate with the read and write caches along with 
  // the FA to find the requested entries.

  fasize_type find_start=start;
  fasize_type find_count=count;

  fasize_type found_count=0;

  // Firstly see if we have some useful entries in the read cache
  if(m_rcache->have_overlap(start,count))
    {
      fasize_type ov_first=m_rcache->overlapping_first(start,count);
      fasize_type ov_count=m_rcache->overlapping_count(start,count);

      find_count -= ov_count;
      found_count += ov_count;

      memcpy(p+(ov_first-start), m_rcache->element(ov_first),
	     ov_count*sizeof(T));
      
      if(ov_first == start)find_start = start + ov_count;
      else find_start=start;
    }
  
  // Now refill the read cache if needs be
  if((find_count > 0) && (find_start < m_fa->size()))
    {
      fasize_type new_cache_occupancy;
      new_cache_occupancy=m_fa->read(find_start,m_rcache->cache(),
				   m_rcache->length());
      m_rcache->fill(find_start,new_cache_occupancy);
#ifdef FA_DEBUG
      cerr << "VSFA_Caching::read(); r" << *m_rcache << '\n';
#endif 

      fasize_type found_more_count=find_count;
      if(new_cache_occupancy < find_count)found_more_count=new_cache_occupancy;

      found_count+=found_more_count;
      memcpy(p+find_start-start, m_rcache->element(find_start),
	     found_more_count*sizeof(T));

      find_start += found_more_count;
      find_count -= found_more_count;
    }
  
  // Finally see if we have some entries in the write cache
  if(m_wcache->have_overlap(start,count))
    {
      fasize_type ov_first=m_wcache->overlapping_first(start,count);
      fasize_type ov_count=m_wcache->overlapping_count(start,count);
      
      if((ov_first <= find_start)&&(ov_first+ov_count>=find_start+find_count))
	find_count=0;

      memcpy(p+(ov_first-start), m_wcache->element(ov_first), 
	     ov_count*sizeof(T));
    }
  
  if(find_count != 0)
    {
      throwerror("VSFA_Caching<>::read","internal error, missing elements!");
    }     

  return count;
}

template <class T>
NS_Analysis::fasize_type
NS_Analysis::VSFA_Caching<T>::
write(fasize_type start, const T* p, fasize_type count)
{
#ifdef FA_DEBUG
  cerr << "VSFA_Caching::write(" << start << ',' << count << "); ";
  cerr << 'w' << *m_wcache << '\n';
#endif 

  if(!VSFA<T>::write_ok(start,count))
    {
      VSFA_RangeError err("VSFA_Caching<>::write",
			  start,count,size(),maxsize());
      err.stream() << "VSFA_Caching(" << id() 
		   << "): write: request out of range";
      throw(err);
    }

  if(count==0)return true;

  if(count > m_wcache->length()/2)
    {
      // if the request was to write more than half the number of
      // elements in the cache then just write them straight
      sync();
      return m_fa->write(start,p,count);
    }
  else if(m_wcache->take_these(start,count,p))
    {
      // they all fitted into the cache, nothing more to do
      return count;
    }
  else
    {
      sync();
      if(!m_wcache->take_these(start,count,p))
	throwerror("VSFA_Caching<>::write","cache error");
      return count;
    }
}

template <class T>
void
NS_Analysis::VSFA_Caching<T>::sync()
{
#ifdef FA_DEBUG
  cerr << "VSFA_Caching::sync(); write " << *m_wcache << '\n';
#endif 

  if(m_wcache->occupancy() > 0)
    {
      m_fa->write(m_wcache->start(),m_wcache->cache(), m_wcache->occupancy());
      m_rcache->update(*m_wcache);
    }
  m_wcache->zero();
  m_fa->sync();
}

#endif // VECTORSTREAMFA_CACHING_H
