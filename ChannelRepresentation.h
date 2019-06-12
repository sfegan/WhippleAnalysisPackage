//-*-mode:c++; mode:font-lock;-*-

#ifndef CHANNELREPRESENTATION_H
#define CHANNELREPRESENTATION_H

#define __STL_USE_NAMESPACES

#include<iostream>
#include<vector>

#include"Types.h"

#include"CameraConfiguration.h"
#include"ChannelData.h"
#include"Cleaning.h"
#include"Pedestals.h"
#include"Gains.h"
#include"RedEvent.h"
#include"Random.h"

namespace NS_Analysis {
  
  using std::vector;


  class EventChannelReps
  {
  private:
    channelnum_type m_nchannels;
    ChannelData<double> m_rawsignal;     // Raw ADC values before subtraction
    ChannelData<double> m_signal;        // Ped subtracted ADC values
    ChannelData<double> m_signaltonoise; // Above divided by Ped Deviations
    ChannelData<double> m_light;         // Gain adjusted, subtraced ADCs
    ChannelData<CleanedState> m_clean;   // Cleaning results 
        
  public:
    EventChannelReps(channelnum_type n=0):
      m_nchannels(n), m_rawsignal(n), m_signal(n), m_signaltonoise(n), 
      m_light(n), m_clean(n) {}

    EventChannelReps(const EventChannelReps &o):
      m_nchannels(o.m_nchannels), m_rawsignal(o.m_rawsignal), 
      m_signal(o.m_signal), m_signaltonoise(o.m_signaltonoise), 
      m_light(o.m_light), m_clean(o.m_clean) {}
      
    EventChannelReps& operator=(const EventChannelReps &o)
    {
      m_nchannels=o.m_nchannels; 
      m_rawsignal=o.m_rawsignal; 
      m_signal=o.m_signal; 
      m_signaltonoise=o.m_signaltonoise;
      m_light=o.m_light; 
      m_clean=o.m_clean;
      return *this;
    }

    channelnum_type nchannels() const { return m_nchannels; }
    bool isCompatible(channelnum_type n) const { return n==nchannels(); }
    void requireCompatible(channelnum_type n) const
    { 
      if(!isCompatible(n))
	{
	  ChannelDataIncompatible err("EventChannelReps::requireCompatible",
				      nchannels(),n);
	  err.stream() << "Meshing of object with " << n 
		       << " channels is incompatible with this " 
		       << nchannels() << endl;
	  throw(err);
	}
    }
    
    // RAWSIGNAL
    ChannelData<double>& rawsignal() { return m_rawsignal; }
    const ChannelData<double>& rawsignal() const { return m_rawsignal; }
    double& rawsignal(channelnum_type c) { return m_rawsignal(c); }
    const double& rawsignal(channelnum_type c) const { return m_rawsignal(c); }

    // SIGNAL
    ChannelData<double>& signal() { return m_signal; }
    const ChannelData<double>& signal() const { return m_signal; }
    double& signal(channelnum_type c) { return m_signal(c); }
    const double& signal(channelnum_type c) const { return m_signal(c); }

    // SIGNALOVERNOISE
    ChannelData<double>& signaltonoise() { return m_signaltonoise; }
    const ChannelData<double>& signaltonoise() const { return m_signaltonoise; }
    double& signaltonoise(channelnum_type c) { return m_signaltonoise(c); }
    const double& signaltonoise(channelnum_type c) const { return m_signaltonoise(c); }

    // LIGHT
    ChannelData<double>& light() { return m_light; }
    const ChannelData<double>& light() const { return m_light; }
    double& light(channelnum_type c) { return m_light(c); }
    const double& light(channelnum_type c) const { return m_light(c); }
    
    // CLEAN
    ChannelData<CleanedState>& clean() { return m_clean; }
    const ChannelData<CleanedState>& clean() const { return m_clean; }
    CleanedState& clean(channelnum_type c) { return m_clean(c); }
    const CleanedState& clean(channelnum_type c) const { return m_clean(c); }
    
    // DUMP
    void dump(ostream& stream);
  }; // EventChannelReps

  class ECRGenerator
  {
  private:
    
  public:
    virtual ~ECRGenerator();
    virtual EventChannelReps* generate(const RedEvent* raw) const=0;
    virtual void dump(ostream& stream) = 0;
  }; // ECRGenerator

  class Standard_ECRGenerator: public ECRGenerator
  {
  private:
    const CameraConfiguration* m_camera;
    const Cleaner* m_cleaner;

  protected:
    ChannelData<ChannelMask> m_mask; 
    
    ChannelData<double> m_gains;

    ChannelData<double> m_peds;
    ChannelData<double> m_pdev;

    void setRawSignal(const RedEvent* raw, EventChannelReps* ecr) const;
    void removePedestals(EventChannelReps* ecr) const;
    void calculateSignalToNoise(EventChannelReps* ecr) const;
    void clean(EventChannelReps* ecr) const;
    void calculateLight(EventChannelReps* ecr) const;

    ChannelMask& mask(channelnum_type c) { return m_mask(c); }
    double& gains(channelnum_type c) { return m_gains(c); }
    double& peds(channelnum_type c) { return m_peds(c); }
    double& pdev(channelnum_type c) { return m_pdev(c); }

  public:
    Standard_ECRGenerator(const CameraConfiguration* cam, const Cleaner* cl,
			  const Gains* gains, 
			  const Pedestals* pedestals);

    virtual ~Standard_ECRGenerator();

    virtual void dump(ostream& stream);

    const CameraConfiguration* camera() const { return m_camera; }
    const Cleaner* cleaner() const { return m_cleaner; }

    // Mask
    const ChannelMask& mask(channelnum_type c) const { return m_mask(c); }

    // Gains
    const double& gains(channelnum_type c) const { return m_gains(c); }
    
    // Peds
    const double& peds(channelnum_type c) const { return m_peds(c); }

    // Ped Devs
    const double& pdev(channelnum_type c) const { return m_pdev(c); }
    
    virtual EventChannelReps* generate(const RedEvent* raw) const;
  };

  class Padding_ECRGenerator: public Standard_ECRGenerator
  {
  private:
    MPtr<GaussianRNG> m_rng;
    ChannelData<int> m_padme;
    ChannelData<double> m_padamount;

  protected:
    void addPaddingNoise(EventChannelReps* ecr) const;

    int& padme(channelnum_type c) { return m_padme(c); }
    double& padamount(channelnum_type c) { return m_padamount(c); }

  public:
    Padding_ECRGenerator(const CameraConfiguration* cam, const Cleaner* cl,
			 MPtr<GaussianRNG>& rng,
			 const Gains* gains, 
			 const Pedestals* pedestals,
			 const Pedestals* pairpedestals);
    virtual ~Padding_ECRGenerator();

    virtual void dump(ostream& stream);

    GaussianRNG* randomNumberGenerator() const { return m_rng.get(); };

    // PadMe
    int padme(channelnum_type c) const { return m_padme(c); }

    // PadAmount
    const double& padamount(channelnum_type c) const { return m_padamount(c); }

    virtual EventChannelReps* generate(const RedEvent* raw) const;
  };

} // namespace NS_Analysis

inline void
NS_Analysis::Standard_ECRGenerator::
setRawSignal(const RedEvent* raw, EventChannelReps* ecr) const
{
  for(unsigned int i=0;i<camera()->nchannels();i++)
    ecr->rawsignal(i)=raw->getADC(i);
}

inline void
NS_Analysis::Standard_ECRGenerator::
removePedestals(EventChannelReps* ecr) const
{
  for(unsigned int i=0;i<camera()->nchannels();i++)
    ecr->signal(i)=ecr->rawsignal(i)-peds(i);
}

inline void
NS_Analysis::Standard_ECRGenerator::
calculateSignalToNoise(EventChannelReps* ecr) const
{
  for(unsigned int i=0;i<camera()->nchannels();i++)
    ecr->signaltonoise(i)=ecr->signal(i)/pdev(i);
}

inline void
NS_Analysis::Standard_ECRGenerator::
calculateLight(EventChannelReps* ecr) const
{
  for(unsigned int i=0;i<camera()->nchannels();i++)
    ecr->light(i)=ecr->signal(i)*gains(i);
}

inline void
NS_Analysis::Standard_ECRGenerator::
clean(EventChannelReps* ecr) const
{
  for(unsigned int i=0;i<camera()->nchannels();i++)
    {
      if(mask(i).isMasked())ecr->clean(i)=CleanedState::CL_DISABLED;
      else ecr->clean(i)=CleanedState::CL_UNKNOWN;
    }
  cleaner()->clean(ecr->clean(),ecr->signaltonoise());  
}

inline void
NS_Analysis::Padding_ECRGenerator::
addPaddingNoise(EventChannelReps* ecr) const
{
  for(unsigned int i=0;i<camera()->nchannels();i++)
    {
      if(padme(i))ecr->signal(i) = ecr->signal(i) + 
		    randomNumberGenerator()->rand()*padamount(i);
    }
}

#endif // CHANNELREPRESENTATION_H
