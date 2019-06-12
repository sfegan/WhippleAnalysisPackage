#include<iostream>
#include<iomanip>
#include<sstream>
#include<vector>
#include<algorithm>

#include"Gains.h"
#include"GainsCalc.h"

using std::vector;
using std::sort;

///////////////////////////////////////////////////////////////////////////////

void
NS_Analysis::StandardGESelector::
setPeds(const Pedestals* peds)
{
  m_peds=peds;
  
  const int nchannels=m_cam->nchannels();
  for(int i=0;i<nchannels;i++)
    {
      m_masked[i] = 0;

      // count the number of "active" (ie. non masked) channels
      if((!peds->mask(i).isMasked()) && (!m_cam->channel(i).isMasked()))
	m_lowSigRejectNumber++;
      else m_masked[i]=1;
    }
  m_lowSigRejectNumber=
    int(floor(double(m_lowSigRejectNumber)*m_lowSigRejectFraction));
}

bool
NS_Analysis::StandardGESelector::
takeThisEvent(int evno, const RedEvent* re)
{
  if(re->code()!=8)return false;

  int nLowSignal=0;
  const int nchannels=m_cam->nchannels();
  for(int i=0;i<nchannels;i++)
    {
      const int adc=re->getADC(i);
      if((m_masked[i]==0) && (adc >= m_saturationThresh))return false;

      const double signal=double(adc)-m_peds->val(i);
      if((m_masked[i]==0) && (signal < m_lowSigThresh))
	{
	  nLowSignal++;
	  if(nLowSignal > m_lowSigRejectNumber)return false;
	}
    }
  
  return true;
}

///////////////////////////////////////////////////////////////////////////////

NS_Analysis::GainsTransferPolicy::
~GainsTransferPolicy()
{
}

NS_Analysis::StandardGainsTransfer::
~StandardGainsTransfer()
{
}

void
NS_Analysis::StandardGainsTransfer::
transfer(Gains* gains, const vector<double>& means, const vector<double>& devs)
{
  int nchannels=gains->nchannels();
  for(int i=0; i<nchannels; i++)
    {
      gains->val(i)=means[i];
      gains->dev(i)=devs[i];
      if(means[i]>m_hiGainsFactor)gains->mask(i).mask("Gain Hi");
      if(means[i]<m_loGainsFactor)gains->mask(i).mask("Gain Lo");
    }
}

////////////////////////////// PedestalsCalculator ////////////////////////////


NS_Analysis::N2GainsCalculator::
~N2GainsCalculator()
{ 
}

void
NS_Analysis::N2GainsCalculator::
process(RedFile* rf, const Pedestals* peds, ProgressBar* pb)
{
  m_selector->setPeds(peds);
  m_peds=peds;
  RedEventSelectedOperator op(this,m_selector.get());
  RedFileEventProcessor proc(&op,pb);
  proc.run(rf);
}

void 
NS_Analysis::N2GainsCalculator::
operateOnEvent(int evno, const RedEvent* re)
{  
  const int nchannels=m_cam->nchannels();

  // In this next section we calculate the mean signal in the camera.. really
  // this should be done seperately for each population in the camera like in
  // PedestalsCalc but I'm too lazy. When we figure out how to handle the
  // outer tubes I'll come back and fix it (which will probably never happen)

  double eventSignalSum    = 0;
  double eventSignalSumSq  = 0;
  int    eventSignalNTubes = 0;
  for(int i=0; i<nchannels; i++)
    {
      const int adc=re->getADC(i);
      const double signal=double(adc)-m_peds->val(i);

      if((m_cam->channel(i).isMasked())||   // Don't count masked tubes
//	 (m_peds->mask(i).isMasked())||     // or tubes with signal below
	 (signal < m_lowSigThresh))         // threshold in the mean
	continue;

      eventSignalNTubes++;
      eventSignalSum+=signal;
      eventSignalSumSq+=signal*signal;
    }
  const double eventSignalMean = eventSignalSum/eventSignalNTubes;

  m_eventsSelected++;
  m_meanSum+=eventSignalMean;
  m_meanSumSquared+=eventSignalMean*eventSignalMean;
  
  for(int i=0; i<nchannels; i++)
    {
      const int adc=re->getADC(i);
      const double signal=double(adc)-m_peds->val(i);
      if(signal < m_lowSigThresh)continue;
      const double gain=eventSignalMean/signal;
      m_nAccumulated[i]++;
      m_gainsSum[i]+=gain;
      m_gainsSumSquared[i]+=gain*gain;
    }
}

NS_Analysis::Gains*
NS_Analysis::N2GainsCalculator::
generateGains() const
{
  int nchannels=m_cam->nchannels();
  Gains* gains=new Gains(nchannels);
 
  vector<double> mean(nchannels);
  vector<double> dev(nchannels);
  
  for(int i=0; i<nchannels; i++)
    {
      if(m_nAccumulated[i])
	{
	  mean[i] = m_gainsSum[i] / m_nAccumulated[i];
	  dev[i] = sqrt(m_gainsSumSquared[i] / m_nAccumulated[i] -
			mean[i]*mean[i]);
	}
      else
	{
	  mean[i] = 0;
	  dev[i] = 1;
	}
    }

  m_trpolicy->transfer(gains,mean,dev);

  gains->setNEvents(m_eventsSelected);
  gains->setCamera(m_cam->description());
  if(m_eventsSelected)
    {
      double msm=m_meanSum/m_eventsSelected;
      gains->setMeanSignalMean(msm);
      gains->setMeanSignalDev(sqrt(m_meanSumSquared/m_eventsSelected-msm*msm));
    }
  
  return gains;
}
