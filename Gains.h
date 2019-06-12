//-*-mode:c++; mode:font-lock;-*-

#ifndef GAINS_Analysis
#define GAINS_Analysis

#define __STL_USE_NAMESPACES

#include"Types.h"
#include"PedsAndGainsBase.h"

namespace NS_Analysis {

  
  class Gains: public PedsAndGainsBase
  {
  public:
    Gains(channelnum_type nchannels): PedsAndGainsBase(nchannels) {}
    virtual ~Gains();

    double meanSignalMean() const { return m_meanSignalMean; }
    double meanSignalDev() const  { return m_meanSignalDev; }

    void setMeanSignalMean(double msm) { m_meanSignalMean=msm; }
    void setMeanSignalDev(double msd)  { m_meanSignalDev=msd; }

  private:
    double  m_meanSignalMean;
    double  m_meanSignalDev;
  };

  ostream& operator<<(ostream& stream, const Gains& p);
  
} // namespace NS_Analysis

#endif // GAINS_Analysis
