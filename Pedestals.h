//-*-mode:c++; mode:font-lock;-*-

#ifndef PEDESTALS_H
#define PEDESTALS_H

#define __STL_USE_NAMESPACES

#include"Types.h"
#include"PedsAndGainsBase.h"

namespace NS_Analysis {

  
  class Pedestals: public PedsAndGainsBase
  {
  public:
    Pedestals(channelnum_type nchannels): PedsAndGainsBase(nchannels) {}
    virtual ~Pedestals();

  private:
  };

  // ostream& operator<<(ostream& stream, const Pedestals& p);
  
} // namespace NS_Analysis

#endif // PEDESTALS_H
