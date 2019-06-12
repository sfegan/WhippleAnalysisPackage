#include"Gains.h"
#include"ChannelData.h"

NS_Analysis::Gains::
~Gains()
{
}

std::ostream& 
NS_Analysis::
operator<<(ostream& stream, const Gains& g)
{
  stream << "Camera: " << g.camera() << endl;
  stream << "Number of events selected: " << g.nevents() << endl;
  stream << "Comment: " << g.comment() << endl;
  stream << "Mean signal mean: " << g.meanSignalMean() << endl;
  stream << "Mean signal deviation: " << g.meanSignalDev() << endl;
  stream << NS_Analysis::ChannelValDevAndMask(g);
  
  return stream;
}
