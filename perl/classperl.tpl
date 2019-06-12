@perl use lib ".";
@perl use Utility;
@//
@// Generate the class perl interface
@//
@foreach class_list          

@perl $myfilename=${class_name}."_perl.cxx";
@perl print "Generating $myfilename\n";
@openfile $myfilename
@include ../noedit.tpl
#include<EXTERN.h>
#include<perl.h>
#include<XSUB.h>

#undef assert // Stupid.. its defined by PERL and by string

#include"../${class_name}.h"

using NS_${class_name}::${class_name};

extern "C" {

XS(XS_${class_name}_new)
{
  dXSARGS;
  if (items != 1)
     croak("Usage: ${class_name}::new(CLASS)");

  char* CLASS=(char *)SvPV(ST(0),na);
  ${class_name}* THIS;
  THIS = new ${class_name}();

  ST(0)=sv_newmortal();
  sv_setref_iv(ST(0),CLASS,(int)static_cast<void*>(THIS));

  XSRETURN(1);
}

XS(XS_${class_name}_DESTROY)
{
  dXSARGS;
  if (items != 1)
     croak("Usage: ${class_name}::DESTROY(THIS)");
	
  ${class_name}* THIS;
  THIS = static_cast<${class_name}*>((void *)SvIV((SV*)SvRV(ST(0))));
  delete(THIS);

  XSRETURN_YES;
}

XS(XS_${class_name}_DUMP)
{
  dXSARGS;
  if (items != 1)
     croak("Usage: ${class_name}::DESTROY(THIS)");
	
  ${class_name}* THIS;
  THIS = static_cast<${class_name}*>((void *)SvIV((SV*)SvRV(ST(0))));

  cerr << *THIS;

  XSRETURN_YES;
}

  @foreach attr_list 
XS(XS_${class_name}_${attr_name})
{
  dXSARGS;
  
  items--;

  ${class_name}* THIS;
  THIS = static_cast<${class_name}*>((void *)SvIV((SV*)SvRV(ST(0))));

    @perl $mtype=Utility::MType($attr_type);
    @perl $mytype=Utility::MakeType($attr_type,$dim);
    @perl $plfunc=Utility::MakePerlFunc($attr_type,$dim);
    @perl $plnfunc=Utility::MakePerlNewFunc($attr_type,$dim);
    @//
    @//***********************************************************************
    @//
    @if ( defined($dim) ) and ( $attr_type ne "string" )
    @//
    @//***********************************************************************
    @// Its an array so lets put it in an AV
  const $mytype x;
  if((items == 1) && (SvROK(ST(1))) && (SvTYPE(SvRV(ST(1))) == SVt_PVAV))
  {
    AV* yarray=reinterpret_cast<AV*>(SvRV(ST(1)));
    if(av_len(yarray) != ($dim-1))croak("Refernced array not of size $dim");
    $mtype y[$dim];
    int i;
    for(i=0;i<$dim;i++)
      y[i]=static_cast<$mtype>(${plfunc}(*av_fetch(yarray,i,0)));
    x=THIS->${attr_name}(y);
  }
  else if(items == $dim)
  {
    // Set the member array
    $mtype y[$dim];
    int i;
    for(i=0;i<$dim;i++)y[i]=static_cast<$mtype>(${plfunc}(ST(i+1)));
    x=THIS->${attr_name}(y);
  }
  else if(items == 0)
  {
    // Access the member array
    x=THIS->${attr_name}();
  }
  else
  {
    croak("Usage: ${class_name}::${attr_name}([value x $dim])");
  }

  AV* xarray=newAV();
  av_extend(xarray, $dim-1);
  int i;
  for(i=0;i<$dim;i++)
  {
    av_store(xarray, i, ${plnfunc}(x[i]));
  }  

  ST(0)=sv_2mortal(newRV_noinc(reinterpret_cast<SV*>(xarray)));
  XSRETURN(1);
  PUTBACK;
  return;
    @//
    @//***********************************************************************
    @//
    @elsif ( $attr_type eq "string" )
    @//
    @//***********************************************************************
    @// A string
  int xlen;
  string x;
  if(items == 1)
  {
    // Set the member
    STRLEN ylen;
    char *y;
    y=SvPV(ST(1),ylen);
    x=THIS->${attr_name}(string(y,ylen));
  }
  else if(items == 0)
  {
    // Access the member
    x=THIS->${attr_name}();
  }
  else
  {
    croak("Usage: ${class_name}::${attr_name}([value])");
  }
  EXTEND(SP,2);
  xlen=strlen(x.c_str());
  ST(0)=sv_2mortal(newSVpv(const_cast<char*>(x.c_str()),xlen));
  XSRETURN(1);
  PUTBACK;
  return;
    @//
    @//***********************************************************************
    @//
    @else
    @//
    @//***********************************************************************
    @//
  $mytype x;
  if(items == 1)
  {
    // Set the member
    $mytype y;
    y=static_cast<$mytype>(${plfunc}(ST(1)));
    x=THIS->${attr_name}(y);
  }
  else if(items == 0)
  {
    // Access the member
    x=THIS->${attr_name}();
  }
  else
  {
    croak("Usage: ${class_name}::${attr_name}([value])");
  }
  EXTEND(SP,2);
  ST(0)=sv_2mortal(${plnfunc}(x));
  XSRETURN(1);    
  PUTBACK;
  return;
    @endif
}

  @end
XS(boot_${class_name})
{
    dXSARGS;
    char* file = __FILE__;

    XS_VERSION_BOOTCHECK ;

    newXS("${class_name}::new", XS_${class_name}_new, file);
    newXS("${class_name}::DESTROY", XS_${class_name}_DESTROY, file);
    newXS("${class_name}::DUMP", XS_${class_name}_DUMP, file);
  @foreach attr_list 
    newXS("${class_name}::${attr_name}", XS_${class_name}_${attr_name}, file);
  @end

    XSRETURN_YES;
}

} // extern "C"
@//
@// Now for the perl module to load this baby
@//
@perl $myfilename=${class_name}.".pm";
@perl print "Generating $myfilename\n";
@openfile $myfilename
package ${class_name};

require Exporter;
require DynaLoader;

\@ISA = qw(Exporter DynaLoader);
# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXDORT_OK instead.
# Do not simply export all your public functions/methods/constants.
\$EXPORT = qw(
           );

\$VERSION = '1.0';

bootstrap $class_name \$VERSION;
@end
