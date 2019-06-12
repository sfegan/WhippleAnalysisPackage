#include<iostream>
#include<iomanip>

#include"ChannelRepresentation.h"

using std::setw;
using std::setprecision;

NS_Analysis::ECRGenerator::
~ECRGenerator()
{

}

NS_Analysis::Standard_ECRGenerator::
~Standard_ECRGenerator()
{

}

NS_Analysis::Standard_ECRGenerator::
Standard_ECRGenerator(const CameraConfiguration* cam, const Cleaner* cl,
		      const Gains* gains, 
		      const Pedestals* pedestals):
  m_camera(cam), m_cleaner(cl), m_mask(cam->nchannels()), 
  m_gains(cam->nchannels()), 
  m_peds(cam->nchannels()), m_pdev(cam->nchannels()) 
{
  // Copy the camera mask to our local mask
  for(unsigned int i=0;i<camera()->nchannels();i++)
    if(camera()->channel(i).isMasked())
      mask(i).addMask(camera()->channel(i));
  
  // Copy the gains and add to the mask
  gains->requireCompatible(camera()->nchannels());
  m_gains = gains->vals();
  gains->addYourMask(m_mask);

  m_peds=pedestals->vals();
  m_pdev=pedestals->devs();
  pedestals->addYourMask(m_mask);
}

void 
NS_Analysis::Standard_ECRGenerator::
dump(std::ostream& stream)
{
  stream << setiosflags(std::ios::fixed) 
	 << setw(8) << "PMT" << "  "
	 << setw(6) << "Gain"  << "  "
	 << setw(6) << "Ped"  << "  "
	 << setw(7) << "Ped Dev" << "  "
	 << "Masked Reason"
	 << endl;

  for(unsigned int i=0; i<m_peds.nchannels(); i++)
    {
      stream << setiosflags(std::ios::fixed) 
	     << setw(8) << camera()->channel(i).name().c_str() << "  "
	     << setw(6) << setprecision(3) << gains(i)  << "  "
	     << setw(6) << setprecision(3) << peds(i)  << "  "
	     << setw(7) << setprecision(3) << pdev(i) << "   "
	     << mask(i).whyMasked()
	     << endl;
    }
}

NS_Analysis::Padding_ECRGenerator::
~Padding_ECRGenerator()
{

}

template<class T>
static inline bool PADME(T mydev, T otherdev, T devlatitude)
{ return((mydev+devlatitude)<otherdev); }

template<class T>
static inline T PADAMOUNT(T mydev, T otherdev) 
{ return sqrt(otherdev*otherdev - mydev*mydev); }

NS_Analysis::Padding_ECRGenerator::
Padding_ECRGenerator(const CameraConfiguration* cam, const Cleaner* cl,
		     MPtr<GaussianRNG>& rng,
		     const Gains* gains, 
		     const Pedestals* pedestals,
		     const Pedestals* pairpedestals):
  Standard_ECRGenerator(cam,cl,gains,pedestals), m_rng(rng),
  m_padme(cam->nchannels()), m_padamount(cam->nchannels())
{
  ChannelData<double> pairpdev(camera()->nchannels());
  pairpdev=pairpedestals->devs();
  pairpedestals->addYourMask(m_mask);

  for(unsigned int i=0; i<camera()->nchannels(); i++)
    {
      if(PADME(m_pdev(i), pairpdev(i), 0.0))
	{
	  m_padme(i)=true;
	  m_padamount(i)=PADAMOUNT(m_pdev(i), pairpdev(i));
	  m_pdev(i)=pairpdev(i);
	}
      else
	{
	  m_padme(i)=false;
	  m_padamount(i)=0.0;
	}
    }
}

NS_Analysis::EventChannelReps*
NS_Analysis::Standard_ECRGenerator::
generate(const RedEvent* raw) const
{
  EventChannelReps* ecr = new EventChannelReps(camera()->nchannels());
  ecr->requireCompatible(raw->sizeADC());
    
  setRawSignal(raw,ecr);
  removePedestals(ecr);
  calculateSignalToNoise(ecr);
  clean(ecr);
  calculateLight(ecr);

  return ecr;
}

NS_Analysis::EventChannelReps*
NS_Analysis::Padding_ECRGenerator::
generate(const RedEvent* raw) const
{
  EventChannelReps* ecr = new EventChannelReps(camera()->nchannels());
  ecr->requireCompatible(raw->sizeADC());
    
  setRawSignal(raw,ecr);
  removePedestals(ecr);
  addPaddingNoise(ecr);
  calculateSignalToNoise(ecr);
  clean(ecr);
  calculateLight(ecr);

  return ecr;
}

void 
NS_Analysis::Padding_ECRGenerator::
dump(ostream& stream)
{
  stream << setiosflags(std::ios::fixed) 
	 << setw(8) << "PMT" << "  "
	 << setw(6) << "Gain" << "  "
	 << setw(6) << "Ped" << "  "
	 << setw(7) << "Ped Dev" << "  "
	 << setw(3) << "Pad" << "  "
	 << setw(10) << "Pad Amount" << "  "
	 << "Masked Reason"
	 << endl;

  for(unsigned int i=0; i<m_peds.nchannels(); i++)
    {
      stream << setiosflags(std::ios::fixed) 
	     << setw(8) << camera()->channel(i).name().c_str() << "  "
	     << setw(6) << setprecision(3) << gains(i) << "  "
	     << setw(6) << setprecision(3) << peds(i) << "  "
	     << setw(7) << setprecision(3) << pdev(i) << "  "
	     << setw(3) << (padme(i)?"yes":"no") << "  "
	     << setw(10) << setprecision(3) << padamount(i) << "  "
	     << mask(i).whyMasked()
	     << endl;
    }
}

void 
NS_Analysis::EventChannelReps::
dump(ostream& stream)
{
  for(unsigned int c=0;c<nchannels();c++)
    {
      stream 
	<< setw(3) << c+1 << "  "
	<< resetiosflags(std::ios::floatfield)
	<< setiosflags(std::ios::fixed)
	<< setw(4) << setprecision(0) << rawsignal(c) << "  "
	<< setw(7) << setprecision(2) << signal(c) << "  "
	<< setw(7) << setprecision(2) << signaltonoise(c) << "  "
	<< setw(7) << setprecision(2) << light(c) << "  "
	<< ((clean(c).image())?'*':' ') << '\n';
    }
}
