//-*-mode:c++; mode:font-lock;-*-

#ifndef PEDESTALSCALC_H
#define PEDESTALSCALC_H

#define __STL_USE_NAMESPACES

#include<iostream>
#include<string>
#include<vector>
#include<algorithm>
#include<cmath>

#include"ChannelData.h"
#include"CameraConfiguration.h"
#include"ProgressBar.h"
#include"Binner.h"
#include"ADCSpectrum.h"
#include"RedFile.h"
#include"RedEvent.h"
#include"Pedestals.h"

namespace NS_Analysis {
  
  /////////////////////////////////////////////////////////////////////////////

  class StdPedRESelector: public RedEventSelector
  {
  public:
    virtual bool takeThisEvent(int evno, const RedEvent* re);

    StdPedRESelector(const CameraConfiguration* cam, int cRT): 
      m_cam(cam), m_cosmicRayThreshold(cRT) { }
    
  private:
    StdPedRESelector(const StdPedRESelector&);
    StdPedRESelector& operator=(const StdPedRESelector&);
    
    const CameraConfiguration* m_cam;
    int m_cosmicRayThreshold;
  };

  /////////////////////////////////////////////////////////////////////////////

  class PedestalsTransferPolicy
  {
  public:
    virtual ~PedestalsTransferPolicy();
    virtual void transfer(Pedestals* peds, const vector<double>& means, 
			  const vector<double>& devs) = 0;
  };

  class StandardPedsTransfer: public PedestalsTransferPolicy
  {
  public:
    StandardPedsTransfer(const CameraConfiguration* cam, 
			 double lo, double hi): 
      m_cam(cam), m_loThresholdFactor(lo), m_hiThresholdFactor(hi) {}
    
    virtual void transfer(Pedestals* peds, const vector<double>& means, 
			  const vector<double>& devs);
    
  private:
    const CameraConfiguration* m_cam;
    double m_loThresholdFactor;
    double m_hiThresholdFactor;
  };

  ///////////////////////////// PedestalaClculator ////////////////////////////

  class PedestalsCalculator
  {
  public:
    PedestalsCalculator(const CameraConfiguration* cam, 
		       int CRThreshold,
		       double lofac, double hifac):
      m_pedspec(cam->nchannels()), m_cam(cam),
      m_mySelector(true), m_selector(new StdPedRESelector(cam,CRThreshold)),
      m_myTrPolicy(true), m_trpolicy(new StandardPedsTransfer(cam,lofac,hifac))
    {}

    PedestalsCalculator(const CameraConfiguration* cam, 
		       RedEventSelector* selector, 
		       double lofac, double hifac):
      m_pedspec(cam->nchannels()), m_cam(cam),
      m_mySelector(false), m_selector(selector),
      m_myTrPolicy(true), m_trpolicy(new StandardPedsTransfer(cam,lofac,hifac))
    {}
    
    PedestalsCalculator(const CameraConfiguration* cam, 
		       RedEventSelector* selector, 
		       PedestalsTransferPolicy* trpolicy):
      m_pedspec(cam->nchannels()), m_cam(cam),
      m_mySelector(false), m_selector(selector),
      m_myTrPolicy(false), m_trpolicy(trpolicy)
    {}
    
    ~PedestalsCalculator() 
    { if(m_mySelector)delete m_selector; if(m_myTrPolicy)delete m_trpolicy; }

    void process(RedFile* rf, ProgressBar* pb=0);

    const ADCSpectrum& spectra() const { return m_pedspec; }
    const Summer& spectrum(int i) const { return m_pedspec.spectrum(i); }

    Pedestals* generatePedestals() const;

  private:
    ADCSpectrum m_pedspec;

    const CameraConfiguration* m_cam;

    bool m_mySelector;
    RedEventSelector* m_selector;

    bool m_myTrPolicy;
    PedestalsTransferPolicy* m_trpolicy;
  };
  
} // namespace NS_Analysis

#endif // PEDESTALSCALC_H
