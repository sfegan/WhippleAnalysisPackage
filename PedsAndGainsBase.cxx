#include"PedsAndGainsBase.h"

NS_Analysis::PedsAndGainsBase::
~PedsAndGainsBase()
{

}


std::ostream& 
NS_Analysis::
operator<<(ostream& stream, const PedsAndGainsBase& p)
{
  stream << "Camera: " << p.camera() << endl;
  stream << "Number of events selected: " << p.nevents() << endl;
  stream << "Comment: " << p.comment() << endl;
  stream << ChannelValDevAndMask(p);
  
  return stream;
}

