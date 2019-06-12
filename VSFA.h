//-*-mode:c++; mode:font-lock;-*-

#ifndef VSFA_H
#define VSFA_H

#define __STL_USE_NAMESPACES

#include <iostream>
#include <string>

#include "Types.h"
#include "Exceptions.h"

namespace NS_Analysis
{
  using std::string;
  using std::ostream;


  template<class T> 
  class VSFA
  {
    unsigned int m_tversion;
    
  protected:
    void setTVersion(unsigned int tv) { m_tversion=tv; }
    
  public:
    virtual ~VSFA() {}

    virtual const string& id() const=0;
    virtual fasize_type size() const=0;
    virtual bool unlimited() const=0;
    virtual fasize_type maxsize() const=0;
    virtual fasize_type read(fasize_type start, T* p, 
			     fasize_type count)=0;

    virtual fasize_type write(fasize_type start, const T* p, 
			      fasize_type count)=0;
    virtual void sync()=0;

    unsigned int tVersion() const { return m_tversion; }

    fasize_type append(const T* p, fasize_type count) 
    { return write(size(),p,count); }
    fasize_type appendOne(const T* p) { return append(p,1); }
    
    // Will a read be safe from a VSFA_RangeError
    bool read_ok(fasize_type start, fasize_type count) 
    { return(start < size()); }

    // Will a write be safe from a VSFA_RangeError
    bool write_ok(fasize_type start, fasize_type count)
    { return((start <= size())&&((unlimited())||(start+count<=maxsize()))); }
  };
} // namespace NS_Analysis

#endif // VSFA_H
