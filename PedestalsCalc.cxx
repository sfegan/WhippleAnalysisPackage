#include<iostream>
#include<iomanip>
#include<sstream>
#include<vector>
#include<algorithm>

#include"Pedestals.h"
#include"PedestalsCalc.h"

using std::vector;
using std::sort;

///////////////////////////////////////////////////////////////////////////////

bool
NS_Analysis::StdPedRESelector::
takeThisEvent(int evno, const RedEvent* re)
{
  if(re->code()==1)
    {
      const int nADC=re->sizeADC();
      int maximumADC=0;
      for(int i=0;i<nADC;i++)
	{
	  if(m_cam->channel(i).isMasked())continue;
	  const int adc=re->getADC(i);
	  if(adc > maximumADC)maximumADC=adc;
	}
      if(maximumADC < m_cosmicRayThreshold)return true;
    }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

NS_Analysis::PedestalsTransferPolicy::
~PedestalsTransferPolicy()
{
}

void
NS_Analysis::StandardPedsTransfer::
transfer(Pedestals* peds, 
	 const vector<double>& means, const vector<double>& devs)
{
  channelnum_type nchannels=means.size();
  
  for(unsigned int i=0;i<nchannels;i++)
    {
      peds->setVal(i,means[i]);
      peds->setDev(i,devs[i]);
    }

  vector<vector<double>*> medianfinder;
  
  for(unsigned int i=0; i<nchannels; i++)
    {
      if(!m_cam->channel(i).isRealChannel())continue;
      if(m_cam->channel(i).isMasked())continue;
      
      unsigned int pop=m_cam->channel(i).population();
      if(medianfinder.size() <= pop)medianfinder.resize(pop+1);
      if(medianfinder[pop] == 0)medianfinder[pop]=new vector<double>;

      medianfinder[pop]->push_back(peds->dev(i));
    }

  vector<double> medians(medianfinder.size());
  for(unsigned int i=0; i<medianfinder.size(); i++)
    if(medianfinder[i] != 0)
      {
	vector<double>* popdevs = medianfinder[i];
	int size=popdevs->size();
	sort(popdevs->begin(),popdevs->end());
	if(size % 2 == 0) // even number so median is...
	  medians[i] = ((*popdevs)[size/2]+(*popdevs)[size/2+1])/2.0;
	else
	  medians[i] = (*popdevs)[(size+1)/2];

	ostringstream ost;
	ost << "MedDev(Pop " << i << ")=" 
	    << std::setprecision(3)<< medians[i] << " ";
	peds->addToComment(ost.str());
	
	delete popdevs;
	medianfinder[i] = 0;
      }

  for(unsigned int i=0; i<nchannels; i++)
    {
      if(!m_cam->channel(i).isRealChannel())continue;
      if(m_cam->channel(i).isMasked())continue;
      
      unsigned int pop=m_cam->channel(i).population();

      double lo = m_loThresholdFactor * medians[pop];
      double hi = m_hiThresholdFactor * medians[pop];

      if(peds->dev(i) < lo)peds->mask(i).mask("Ped Dev Lo");
      if(peds->dev(i) > hi)peds->mask(i).mask("Ped Dev Hi");
    }
}

////////////////////////////// PedestalsCalculator ////////////////////////////

void
NS_Analysis::PedestalsCalculator::
process(RedFile* rf, ProgressBar* pb)
{
  ADCSpectrumBuilder specbuilder(m_cam,&m_pedspec);
  specbuilder.insert(rf,m_selector,pb);
}

NS_Analysis::Pedestals*
NS_Analysis::PedestalsCalculator::
generatePedestals() const
{
  channelnum_type nchannels=m_pedspec.nchannels();
  Pedestals* p=new Pedestals(nchannels);

  vector<double> means(nchannels);
  vector<double> devs(nchannels);

  for(unsigned int i=0;i<nchannels;i++)
    {
      double totalCount = 0;
      double mean       = 0;
      double variance   = 0;

      spectrum(i).getStats(totalCount,mean,variance);

      means[i] = mean;
      devs[i] = sqrt(variance);
    }

  m_trpolicy->transfer(p,means,devs);

  p->setNEvents(m_pedspec.nEvents());
  p->setCamera(m_cam->description());

  return p;
}
