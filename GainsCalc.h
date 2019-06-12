//-*-mode:c++; mode:font-lock;-*-

#ifndef GAINSCALC_H
#define GAINSCALC_H

#define __STL_USE_NAMESPACES

#include<iostream>
#include<string>
#include<vector>
#include<algorithm>
#include<cmath>

#include"Types.h"
#include"ChannelData.h"
#include"CameraConfiguration.h"
#include"ProgressBar.h"
#include"Binner.h"
#include"ADCSpectrum.h"
#include"RedFile.h"
#include"RedEvent.h"
#include"Gains.h"
#include"Pedestals.h"

namespace NS_Analysis {
  using std::vector;

  /////////////////////////////////////////////////////////////////////////////

  class GainsRedEventSelector: public RedEventSelector
  {
  public:
    virtual void setPeds(const Pedestals* peds) = 0;
  };
  
  class StandardGESelector: public GainsRedEventSelector
  {
  public:
    virtual bool takeThisEvent(int evno, const RedEvent* re);
    
    StandardGESelector(const CameraConfiguration* cam, 
		       int saturationThresh, 
		       double lowSigThresh, double lowSigRejectFraction):
      m_cam(cam), m_peds(0), m_masked(cam->nchannels()), 
      m_saturationThresh(saturationThresh),
      m_lowSigThresh(lowSigThresh), 
      m_lowSigRejectFraction(lowSigRejectFraction), m_lowSigRejectNumber(0)
    { }

    virtual void setPeds(const Pedestals* peds);

  private:
    StandardGESelector(const StandardGESelector&);
    StandardGESelector& operator=(const StandardGESelector&);

    const CameraConfiguration*       m_cam;

    const Pedestals*                 m_peds;

    ChannelData<int>                 m_masked;
    
    int                              m_saturationThresh;
    double                           m_lowSigThresh;
    double                           m_lowSigRejectFraction;
    int                              m_lowSigRejectNumber;
  };

  /////////////////////////////////////////////////////////////////////////////

  class GainsTransferPolicy
  {
  public:
    virtual ~GainsTransferPolicy();
    virtual void transfer(Gains* gainss, const vector<double>& means, 
			  const vector<double>& devs) = 0;
  };

  class StandardGainsTransfer: public GainsTransferPolicy
  {
  public:
    StandardGainsTransfer(const CameraConfiguration* cam, 
			  double lo, double hi): 
      m_cam(cam), m_loGainsFactor(lo), m_hiGainsFactor(hi) {}

    virtual ~StandardGainsTransfer();

    virtual void transfer(Gains* gains, const vector<double>& means, 
			  const vector<double>& devs);
    
  private:
    const CameraConfiguration* m_cam;
    double m_loGainsFactor;
    double m_hiGainsFactor;
  };

  /////////////////////////////////////////////////////////////////////////////

  class N2GainsCalculator: private RedEventOperator
  {
  public:
    N2GainsCalculator(const CameraConfiguration* cam, 
		      int saturationThresh, 
		      double lowSigThresh, double lowSigRejectFraction,
		      double lofac, double hifac):
      m_cam(cam), m_peds(0), m_lowSigThresh(lowSigThresh),
      m_eventsSelected(0), m_meanSum(0), m_meanSumSquared(0),
      m_nAccumulated(cam->nchannels()),
      m_gainsSum(cam->nchannels()), m_gainsSumSquared(cam->nchannels()),
      m_selector(new StandardGESelector(cam,saturationThresh,
					lowSigThresh,lowSigRejectFraction),
		 true),
      m_trpolicy(new StandardGainsTransfer(cam,lofac,hifac), true)
    {}
    
    N2GainsCalculator(const CameraConfiguration* cam, 
		      double lowSigThresh, 
		      GainsRedEventSelector* selector, 
		      double lofac, double hifac):
      m_cam(cam), m_peds(0), m_lowSigThresh(lowSigThresh),
      m_eventsSelected(0), m_meanSum(0), m_meanSumSquared(0),
      m_nAccumulated(cam->nchannels()),
      m_gainsSum(cam->nchannels()), m_gainsSumSquared(cam->nchannels()),
      m_selector(selector,false),
      m_trpolicy(new StandardGainsTransfer(cam,lofac,hifac), true)
    {}
    
    N2GainsCalculator(const CameraConfiguration* cam, 
		      double lowSigThresh, 
		      GainsRedEventSelector* selector, 
		      GainsTransferPolicy* trpolicy):
      m_cam(cam), m_peds(0), m_lowSigThresh(lowSigThresh),
      m_eventsSelected(0), m_meanSum(0), m_meanSumSquared(0),
      m_nAccumulated(cam->nchannels()),
      m_gainsSum(cam->nchannels()), m_gainsSumSquared(cam->nchannels()),
      m_selector(selector,false),
      m_trpolicy(trpolicy,false)
    {}
    
    ~N2GainsCalculator();

    void process(RedFile* rf, const Pedestals* peds, ProgressBar* pb=0);

    Gains* generateGains() const;

  private:
    virtual void operateOnEvent(int evno, const RedEvent* re);

    const CameraConfiguration*  m_cam;
    const Pedestals*            m_peds;

    double                      m_lowSigThresh;

    int                         m_eventsSelected;
    double                      m_meanSum;
    double                      m_meanSumSquared;

    vector<int>                 m_nAccumulated;
    vector<double>              m_gainsSum;
    vector<double>              m_gainsSumSquared;

    MPtr<GainsRedEventSelector> m_selector;
    MPtr<GainsTransferPolicy>   m_trpolicy;
  };
  
} // namespace NS_Analysis

#endif // GAINSCALC_H
