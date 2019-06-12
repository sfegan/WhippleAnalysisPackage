#include<iostream>
#include<iomanip>

#include"ChannelData.h"

using std::setw;
using std::setprecision;
using std::setiosflags;
using std::ostream;
using std::ios;

ostream& 
NS_Analysis::
operator<<(ostream& stream, const ChannelValDevAndMask& cvdm)
{
  for(unsigned int i=0; i<cvdm.nchannels(); i++)
    stream << setw(3) << i << '\t'
	   << setw(6) << setprecision(3) << setiosflags(ios::fixed)
	   << cvdm.val(i) << '\t'
	   << cvdm.dev(i) << '\t'
	   << cvdm.mask(i).whyMasked() << endl;
  
  return stream;
}
