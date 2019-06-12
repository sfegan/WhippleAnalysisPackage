#include"../GH5/VSFA_H5.h"
#include"VSFA_perl.h"

#include"../RedEvent.h"

using namespace NS_VSFA_H5;

using namespace NS_RedEvent;

extern "C" {

XS(boot_VSFA_H5)
{
    dXSARGS;
    char* file = __FILE__;

    XS_VERSION_BOOTCHECK ;

    XSRETURN_YES;
}

} // extern "C"
