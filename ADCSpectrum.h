//-*-mode:c++; mode:font-lock;-*-

#ifndef ADCSPECTRUM_H
#define ADCSPECTRUM_H

#define __STL_USE_NAMESPACES

#include<iostream>
#include<string>

#include"Exceptions.h"
#include"ChannelData.h"
#include"CameraConfiguration.h"
#include"RedEvent.h"
#include"Binner.h"
#include"ProgressBar.h"
#include"RedFile.h"

namespace NS_Analysis {
  
  //-*- hi z-sir -*-
  class ADCSpectrum
  {
  public:
    ADCSpectrum(channelnum_type n)
      : m_nchannels(n), m_spectrum(n,Summer(1.0)), m_nevents(0) { }

    virtual ~ADCSpectrum();

    channelnum_type nchannels() const { return m_nchannels; }

    void insert(const RedEvent* re);
    void insert(const ChannelData<double>* data);

    const Summer& spectrum(int n) const { return m_spectrum(n); }
    int nEvents() const { return m_nevents; }

    double maxVal() const;
    double minVal() const;
    
  private:
    channelnum_type m_nchannels;
    ChannelData<Summer> m_spectrum;
    int m_nevents;

    void validate(channelnum_type n) const
    { 
      if(n>=m_nchannels)
	{
	  ChannelOutOfRange err("ADCSpectrum::validate",n);
	  err.stream() << "Request for channel number " << n 
		       << " is out of range. Max is:" << nchannels() << endl;
	  throw(err); 
	}
    }
  };

  class ADCSpectrumBuilder: private RedEventOperator
  {
  public:
    ADCSpectrumBuilder(const CameraConfiguration* cam, 
		       ADCSpectrum* spectrum)
      : m_spectrum(spectrum), m_cam(cam) {}
    virtual ~ADCSpectrumBuilder();

    void insert(RedFile* rf,
		RedEventSelector* sel,
		ProgressBar* pb=0);

  private:
    ADCSpectrum* m_spectrum;
    const CameraConfiguration* m_cam;

    void operateOnEvent(int evno, const RedEvent* re);
  };

} // namespace NS_Analysis

inline void NS_Analysis::ADCSpectrum::insert(const RedEvent* re)
{
  if(m_nchannels != re->sizeADC())
    {
      ChannelDataIncompatible err("ADCSpectrum::insert",
				  m_nchannels,re->sizeADC());
      err.stream() << "Number of ADC channels in RedEvent (" << re->sizeADC()
		   << ") != number of ADCSpectrum channels (" << m_nchannels
		   << ")" << endl;
      throw(err);
    }
  
  m_nevents++;
  for(unsigned int i=0;i<nchannels();i++)
    m_spectrum(i).insert(re->getADC(i));
}

inline void NS_Analysis::ADCSpectrum::insert(const ChannelData<double>* data)
{
  if(m_nchannels != data->nchannels())
    {
      ChannelDataIncompatible err("ADCSpectrum::insert",
				  m_nchannels,data->nchannels());
      err.stream() << "Number of ADC channels in ChannelData (" 
		   << data->nchannels()
		   << ") != number of ADCSpectrum channels (" << m_nchannels
		   << ")" << endl;
      throw(err);
    }
  
  m_nevents++;
  for(unsigned int i=0;i<nchannels();i++)
    m_spectrum(i).insert((*data)(i));
}

inline double NS_Analysis::ADCSpectrum::maxVal() const 
{
  bool active=false;
  double max;
  for(unsigned int i=0;i<nchannels();i++)
    if((!active)||(max < m_spectrum(i).maxVal()))
      {
	max=m_spectrum(i).maxVal();
	active=true;
      }
  return max;
}

inline double NS_Analysis::ADCSpectrum::minVal() const 
{
  bool active=false;
  double min;
  for(unsigned int i=0;i<nchannels();i++)
    if((!active)||(min > m_spectrum(i).minVal()))
      {
	min=m_spectrum(i).minVal();
	active=true;
      }
  return min;
}

#endif // defined ADCSPECTRUM_H
