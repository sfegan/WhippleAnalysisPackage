//-*-mode:c++; mode:font-lock;-*-

#ifndef TYPES_H
#define TYPES_H

#define __STL_USE_NAMESPACES

#include<hdf5/hdf5.h>

namespace NS_Analysis {

  // VSFA
  typedef hsize_t fasize_type;

  typedef unsigned int channelnum_type;

  template<class T> class MPtr
  {
  public:
    MPtr() throw(): m_managed(false), m_ptr(0) {}
    MPtr(const MPtr& o): m_managed(false), m_ptr(o.m_ptr) { }
    MPtr(MPtr& o): m_managed(o.m_managed), m_ptr(o.m_ptr)
    { o.m_managed=false; }
    MPtr(T* Tp, bool managed=false) throw(): m_managed(managed), m_ptr(Tp) 
    { if(m_ptr==0)m_managed=false; }
    ~MPtr() throw() { if((m_managed)&&(m_ptr))delete m_ptr; }
    
    T& operator* () const throw() { return *m_ptr; }
    T* operator-> () const throw() { return m_ptr; }

    MPtr& operator= (MPtr& o) 
    { adopt(o.m_ptr,o.m_managed); o.m_ptr=0; o.m_managed=false; return *this; }

    T* get() const { return m_ptr; }
    T* release() throw() { T* Tp=m_ptr; m_managed=0; m_ptr=0; return Tp; }

    void reset() throw() 
    { if((m_managed)&&(m_ptr))delete m_ptr; m_ptr=0; m_managed=false; }

    void manage(T* Tp) throw() 
    { 
      reset();
      m_managed=true;
      m_ptr=Tp;
    }

    void adopt(T* Tp, bool manage) throw() 
    { 
      reset();
      m_managed=manage;
      m_ptr=Tp;
    }

  private:

    bool   m_managed;
    T*     m_ptr;
  };
  
} // namespace NS_Analysis

#endif // defined TYPES_H
