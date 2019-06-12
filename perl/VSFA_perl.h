//-*-mode:c++; mode:font-lock;-*-

#ifndef VSFA_PERL_H
#define VSFA_PERL_H

#define __STL_USE_NAMESPACES

#include<string>

#include<EXTERN.h>
#include<perl.h>
#include<XSUB.h>

#include"../GH5/VSFA.h"

using namespace NS_VSFA;

template<class T, class FA>
XS(XS_VSFA_new)
{
  dXSARGS;
  if (items != 4)
    croak("Usage: ${class_name}::new(CLASS,version,options,create)");

  // Parameter 0: the class
  char* CLASS=(char *)SvPV(ST(0),na);

  // Parameter 1: version to pass to the FA constructor
  unsigned int version=(unsigned int)SvIV(ST(1));
  
  // Parameter 2: pointer to options structure
  const typename FA::options* options;
  options=
    reinterpret_cast<const typename FA::options*>(SvIV((SV*)SvRV(ST(2))));
  
  // Parameter 3
  bool mustcreate=(bool)SvIV(ST(3));

#ifdef DEBUG_PERL_INTERFACE
  cerr << "VSFA::new(" << CLASS << ',' << version << ',' 
       << "opt:" << reinterpret_cast<int>(options) << ','
       << (mustcreate?"true":"false") << '\n';
#endif // DEBUG_PERL_INTERFACE

  FA* fa;
  try
    {
      fa = new FA(version,*options,mustcreate);
    }
  catch(VSFA_RangeError x)
    {
      cerr << x << '\n';
      croak("Could not create OBJECT");
    }
  catch(VSFA_Error x)
    {
      cerr << x << '\n';
      croak("Could not create OBJECT");
    }
  
  ST(0)=sv_newmortal();
  sv_setref_iv(ST(0),CLASS,reinterpret_cast<int>(fa));

#ifdef DEBUG_PERL_INTERFACE
  cerr << "VSFA::new: created VSFA:" << reinterpret_cast<int>(fa) << '\n';
#endif // DEBUG_PERL_INTERFACE

  XSRETURN(1);
}

template<class T, class FA>
XS(XS_VSFA_DESTROY)
{
  dXSARGS;
  if (items != 1)
    croak("Usage: ${class_name}::DESTROY(THIS)");
  
  FA* fa;
  fa = reinterpret_cast<FA*>(SvIV((SV*)SvRV(ST(0))));

#ifdef DEBUG_PERL_INTERFACE
  cerr << "VSFA::delete(VSFA:" << reinterpret_cast<int>(fa) << ")\n";
#endif // DEBUG_PERL_INTERFACE

  delete(fa);
  
  XSRETURN_YES;
}

template<class T, class FA>
XS(XS_VSFA_id)
{
  dXSARGS;
  
  if (items != 1)
    croak("Usage: ${class_name}::DESTROY(THIS)");
	
  FA* fa;
  fa = reinterpret_cast<FA*>(SvIV((SV*)SvRV(ST(0))));

#ifdef DEBUG_PERL_INTERFACE
  cerr << "VSFA::id(VSFA:" << reinterpret_cast<int>(fa) << ")\n";
#endif // DEBUG_PERL_INTERFACE
  
  string id;

  try
    {
      id=fa->id();
    }
  catch(VSFA_RangeError x)
    {
      cerr << x << '\n';
      croak("Could not create OBJECT");
    }
  catch(VSFA_Error x)
    {
      cerr << x << '\n';
      croak("Could not create OBJECT");
    }
  
  ST(0)=sv_newmortal();
  sv_setpv(ST(0), id.c_str());
  
  XSRETURN(1);
}

template<class T, class FA>
XS(XS_VSFA_size)
{
  dXSARGS;
  
  if (items != 1)
    croak("Usage: ${class_name}::DESTROY(THIS)");
	
  FA* fa;
  fa = reinterpret_cast<FA*>(SvIV((SV*)SvRV(ST(0))));

#ifdef DEBUG_PERL_INTERFACE
  cerr << "VSFA::size(VSFA:" << reinterpret_cast<int>(fa) << ")\n";
#endif // DEBUG_PERL_INTERFACE
  
  fasize_t size;

  try
    {
      size=fa->size();
    }
  catch(VSFA_RangeError x)
    {
      cerr << x << '\n';
      croak("Could not create OBJECT");
    }
  catch(VSFA_Error x)
    {
      cerr << x << '\n';
      croak("Could not create OBJECT");
    }
  
  ST(0)=sv_newmortal();
  sv_setiv(ST(0), size);
  
  XSRETURN(1);
}

template<class T, class FA>
XS(XS_VSFA_maxsize)
{
  dXSARGS;
  
  if (items != 1)
    croak("Usage: ${class_name}::DESTROY(THIS)");
	
  FA* fa;
  fa = reinterpret_cast<FA*>(SvIV((SV*)SvRV(ST(0))));
  
#ifdef DEBUG_PERL_INTERFACE
  cerr << "VSFA::maxsize(VSFA:" << reinterpret_cast<int>(fa) << ")\n";
#endif // DEBUG_PERL_INTERFACE

  fasize_t size;

  try
    {
      size=fa->maxsize();
    }
  catch(VSFA_RangeError x)
    {
      cerr << x << '\n';
      croak("Could not create OBJECT");
    }
  catch(VSFA_Error x)
    {
      cerr << x << '\n';
      croak("Could not create OBJECT");
    }

  ST(0)=sv_newmortal();
  sv_setiv(ST(0), size);
  
  XSRETURN(1);
}

template<class T, class FA>
XS(XS_VSFA_unlimited)
{
  dXSARGS;
  
  if (items != 1)
    croak("Usage: ${class_name}::DESTROY(THIS)");
	
  FA* fa;
  fa = reinterpret_cast<FA*>(SvIV((SV*)SvRV(ST(0))));

#ifdef DEBUG_PERL_INTERFACE
  cerr << "VSFA::unlimited(VSFA:" << reinterpret_cast<int>(fa) << ")\n";
#endif // DEBUG_PERL_INTERFACE
  
  bool unlimited;

  try
    {
      unlimited=fa->unlimited();
    }
  catch(VSFA_RangeError x)
    {
      cerr << x << '\n';
      croak("Could not create OBJECT");
    }
  catch(VSFA_Error x)
    {
      cerr << x << '\n';
      croak("Could not create OBJECT");
    }
  
  ST(0)=sv_newmortal();
  sv_setiv(ST(0), unlimited);
  
  XSRETURN(1);
}

template<class T, class FA>
XS(XS_VSFA_sync)
{
  dXSARGS;
  
  if (items != 1)
    croak("Usage: ${class_name}::(THIS)");
	
  FA* fa;
  fa = reinterpret_cast<FA*>(SvIV((SV*)SvRV(ST(0))));

#ifdef DEBUG_PERL_INTERFACE
  cerr << "VSFA::sync(VSFA:" << reinterpret_cast<int>(fa) << ")\n";
#endif // DEBUG_PERL_INTERFACE

  try
    {
      fa->sync();
    }
  catch(VSFA_RangeError x)
    {
      cerr << x << '\n';
      croak("Could not create OBJECT");
    }
  catch(VSFA_Error x)
    {
      cerr << x << '\n';
      croak("Could not create OBJECT");
    }
  
  XSRETURN_YES;
}

template<class T, class FA>
XS(XS_VSFA_read)
{
  dXSARGS;
  
  if (items < 3)
    croak("Usage: ${class_name}::read(THIS,start,element...)");
	
  FA* fa;
  fa = reinterpret_cast<FA*>(SvIV((SV*)SvRV(ST(0))));
  
  // Parameter 1: Start
  fasize_t start=SvIV(ST(1));

  // Implicit parameter: Count
  fasize_t count=items-2;

#ifdef DEBUG_PERL_INTERFACE
  cerr << "VSFA::read(VSFA:" << reinterpret_cast<int>(fa) << ','
       << start << ',' << count << ")\n";
#endif // DEBUG_PERL_INTERFACE

  if(count == 1)
    {
      T* readbuffer;
      readbuffer=reinterpret_cast<T*>(SvIV((SV*)SvRV(ST(2))));
      try
	{
	  count=fa->read(start,readbuffer,count);
	}
      catch(VSFA_RangeError x)
	{
	  cerr << x << '\n';
	  croak("Could not create OBJECT");
	}
      catch(VSFA_Error x)
	{
	  cerr << x << '\n';
	  croak("Could not create OBJECT");
	}
    }
  else
    {
      int i;
      T* readbuffer;
      readbuffer=new T[count];
      try
	{
	  count=fa->read(start,readbuffer,count);
	}
      catch(VSFA_RangeError x)
	{
	  cerr << x << '\n';
	  croak("Could not create OBJECT");
	}
      catch(VSFA_Error x)
	{
	  cerr << x << '\n';
	  croak("Could not create OBJECT");
	}
      
      for(i=0;i<count;i++)
	{
	  T* ptr=reinterpret_cast<T*>(SvIV((SV*)SvRV(ST(2+i))));
	  *ptr=readbuffer[i];
	}
      delete[] readbuffer;
    }

  ST(0)=sv_newmortal();
  sv_setiv(ST(0), count);
  XSRETURN(1);
}

template<class T, class FA>
XS(XS_VSFA_write)
{
  dXSARGS;
  
  if (items < 3)
    croak("Usage: ${class_name}::write(THIS,start,element...)");
	
  FA* fa;
  fa = reinterpret_cast<FA*>(SvIV((SV*)SvRV(ST(0))));
  
  // Parameter 1: Start
  fasize_t start=SvIV(ST(1));

  // Implicit parameter: Count
  fasize_t count=items-2;

#ifdef DEBUG_PERL_INTERFACE
  cerr << "VSFA::write(VSFA:" << reinterpret_cast<int>(fa) << ','
       << start << ',' << count << ")\n";
#endif // DEBUG_PERL_INTERFACE

  if(count == 1)
    {
      T* writebuffer;
      writebuffer=reinterpret_cast<T*>(SvIV((SV*)SvRV(ST(2))));
      try
	{
	  count=fa->write(start,writebuffer,count);
	}
      catch(VSFA_RangeError x)
	{
	  cerr << x << '\n';
	  croak("Could not create OBJECT");
	}
      catch(VSFA_Error x)
	{
	  cerr << x << '\n';
	  croak("Could not create OBJECT");
	}
    }
  else
    {
      int i;
      T* writebuffer;
      writebuffer=new T[count];
      for(i=0;i<count;i++)
	{
	  T* ptr=reinterpret_cast<T*>(SvIV((SV*)SvRV(ST(2+i))));
	  writebuffer[i]=*ptr;
	}
      try
	{
	  count=fa->write(start,writebuffer,count);
	}
      catch(VSFA_RangeError x)
	{
	  cerr << x << '\n';
	  croak("Could not create OBJECT");
	}
      catch(VSFA_Error x)
	{
	  cerr << x << '\n';
	  croak("Could not create OBJECT");
	}

      delete[] writebuffer;
    }

  ST(0)=sv_newmortal();
  sv_setiv(ST(0), count);
  XSRETURN(1);
}

template<class T, class FA>
void VSFA_initialise_functions(const string& classname)
{
  char* file = __FILE__;

  // I've taken "new" and "DESTROY" away from PERL as it seems to cause more
  // trouble than its worth. The suggested way of using these VSFA things is
  // through a C++ interface... ie. make a higher level object that creates
  // the VSFA's and cleans up later. Look at the RedFile example.

  //  newXS(const_cast<char*>((classname+"::new").c_str()),
  //	XS_VSFA_new<T,FA>,       file);
  //  newXS(const_cast<char*>((classname+"::DESTROY").c_str()),
  //	XS_VSFA_DESTROY<T,FA>,   file);

  newXS(const_cast<char*>((classname+"::id").c_str()),
	XS_VSFA_id<T,FA>,        file);
  newXS(const_cast<char*>((classname+"::size").c_str()),
	XS_VSFA_size<T,FA>,      file);
  newXS(const_cast<char*>((classname+"::unlimited").c_str()),
	XS_VSFA_unlimited<T,FA>, file);
  newXS(const_cast<char*>((classname+"::maxsize").c_str()),
	XS_VSFA_maxsize<T,FA>,   file);
  newXS(const_cast<char*>((classname+"::read").c_str()),
	XS_VSFA_read<T,FA>,      file);
  newXS(const_cast<char*>((classname+"::write").c_str()),
	XS_VSFA_write<T,FA>,     file);
  newXS(const_cast<char*>((classname+"::sync").c_str()),
	XS_VSFA_sync<T,FA>,      file);
}

#endif // VSFA_PERL_H
