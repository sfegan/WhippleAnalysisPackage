#include<string>

#include<hdf5.h>

#include<EXTERN.h>
#include<perl.h>
#include<XSUB.h>

#include"../RedFile.h"
#include"VSFA_perl.h"

using namespace NS_RedFile;

using namespace NS_VSFA_H5;
using namespace NS_VSFA_Caching;

using namespace NS_RedEvent;

using std::string;

extern "C" {
  
  XS(XS_RedFile_new)
  {
    dXSARGS;
    if (items != 3)
      croak("Usage: RedFile::new(CLASS,filename,create)");
    
    // Parameter 0: the class
    char* CLASS=(char *)SvPV(ST(0),na);
    
    // Parameter 1: version to pass to the FA constructor
    STRLEN len;
    const char *fn=(const char *)SvPV(ST(1),len);
    const string filename=string(fn,len);
    
    // Parameter 2
    bool create=(bool)SvIV(ST(2));
    
    RedFile* rf;
    try 
      {
	rf = new RedFile(filename.c_str(),create);
      }
    catch(VSFA_RangeError x)
      {
	cerr << x;
	croak("Could not create OBJECT");
      }
    catch(VSFA_Error x)
      {
	cerr << x;
	croak("Could not create OBJECT");
      }
    
    ST(0)=sv_newmortal();
    sv_setref_iv(ST(0),CLASS,reinterpret_cast<int>(rf));

#ifdef DEBUG_PERL_INTERFACE
    cerr << "RedFile::new created at " << reinterpret_cast<int>(rf) << '\n';
#endif // DEBUG_PERL_INTERFACE
    
    XSRETURN(1);
  }
  
  XS(XS_RedFile_DESTROY)
  {
    dXSARGS;
    if (items != 1)
      croak("Usage: RedFile::DESTROY(THIS)");
    
    RedFile* rf;
    rf = reinterpret_cast<RedFile*>(SvIV((SV*)SvRV(ST(0))));

#ifdef DEBUG_PERL_INTERFACE
    cerr << "RedFile::deleted " << reinterpret_cast<int>(rf) << '\n';
#endif // DEBUG_PERL_INTERFACE

    delete(rf);
    
    XSRETURN_YES;
  }

  XS(XS_RedFile_header)
  {
    dXSARGS;
    if (items != 1)
      croak("Usage: RedFile::header(THIS)");
    
    RedFile* rf;
    rf = reinterpret_cast<RedFile*>(SvIV((SV*)SvRV(ST(0))));
    
#ifdef DEBUG_PERL_INTERFACE
    cerr << "RedFile::header " << reinterpret_cast<int>(rf) << '\n';
#endif // DEBUG_PERL_INTERFACE

    RedFile::header_list_t* header=rf->header();
    
    ST(0)=sv_newmortal();
    sv_setref_iv(ST(0),"VSFA_H5_RedHeader",
		 reinterpret_cast<int>(header));
    
    XSRETURN(1);
  }

  XS(XS_RedFile_events)
  {
    dXSARGS;
    if (items != 1)
      croak("Usage: RedFile::events(THIS)");
    
    RedFile* rf;
    rf = reinterpret_cast<RedFile*>(SvIV((SV*)SvRV(ST(0))));
    
#ifdef DEBUG_PERL_INTERFACE
    cerr << "RedFile::events " << reinterpret_cast<int>(rf) << '\n';
#endif // DEBUG_PERL_INTERFACE

    RedFile::events_list_t* events=rf->events();
    
    ST(0)=sv_newmortal();
    sv_setref_iv(ST(0),"VSFA_Caching_H5_RedEvent",
		 reinterpret_cast<int>(events));
    
    XSRETURN(1);
  }
    
  XS(boot_RedFile)
  {
    dXSARGS;
    char* file = __FILE__;

    XS_VERSION_BOOTCHECK ;

    newXS("RedFile::new",     XS_RedFile_new,     file);
    newXS("RedFile::DESTROY", XS_RedFile_DESTROY, file);
    newXS("RedFile::header",  XS_RedFile_header,  file);
    newXS("RedFile::events",  XS_RedFile_events,  file);

    VSFA_initialise_functions<RedEvent,
      VSFA_Caching<RedEvent,VSFA_H5<RedEvent> > >
      ("VSFA_Caching_H5_RedEvent");

    VSFA_initialise_functions<RedHeader,VSFA_H5<RedHeader> >
      ("VSFA_H5_RedHeader");
    
    XSRETURN_YES;
  }


} // extern "C"
