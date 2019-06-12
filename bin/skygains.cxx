//-*-mode:c++; mode:font-lock;-*-
int errno;

#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

#include <iostream>
#include <iomanip>

#include <cmath>

#include <assert.h>
#include <unistd.h>
#include <getopt.h>

#include "fz.h"

float gammln(float xx);

using namespace std;

#define MAX_ADC        2048
#define MAX_LINEAR_ADC 1024

#define MIN_CR_SIGMA   7.0

#define NLNBINS        10

// ============================================================================
// GDFPedDevCalculator - calculate pedestal val / dev directly from .fz file
// ============================================================================

class GDFPedDevCalculator: public GDFRecordHandler
{
public:
  GDFPedDevCalculator(unsigned int nchannel, short cr = 75);

  unsigned int nevents() const { return m_nevents; }
  double ped(unsigned int ch) const 
  { assert((ch>=0)&&(ch<m_nchannel)); return m_mean[ch]; }
  double dev(unsigned int ch) const
  { assert((ch>=0)&&(ch<m_nchannel)); return m_dev[ch]; }

  virtual void ev10(GDFRecordDispatcher &dispatcher, 
		    const struct gdf_ev10_s& record);

  virtual void starting_file(GDFRecordDispatcher &dispatcher,
			     const std::string& filename);

  virtual void finished_file(GDFRecordDispatcher &dispatcher,
			     const std::string& filename);

private:
  short          m_cr_threshold;

  unsigned int   m_nchannel;

  unsigned int   m_nevents;
  unsigned int   m_nallevents;

  vector<double> m_sum;
  vector<double> m_sumsq;

  vector<double> m_mean;
  vector<double> m_dev;
};

GDFPedDevCalculator::GDFPedDevCalculator(unsigned int nchannel, short cr)
  :  m_cr_threshold(cr),
     m_nchannel(nchannel), m_nevents(0), m_nallevents(0), 
     m_sum(nchannel), m_sumsq(nchannel), m_mean(nchannel), m_dev(nchannel)
{
  // nothing to see here
}

void GDFPedDevCalculator::starting_file(GDFRecordDispatcher &dispatcher,
					const std::string& filename)
{
  unsigned int nchannel=m_sum.size();
  m_nevents=0;
  m_nallevents=0;
  m_sum.clear();
  m_sum.resize(nchannel);
  m_sumsq.clear();
  m_sumsq.resize(nchannel);
  cerr << filename << ": calculating pedestals ";
}

void GDFPedDevCalculator::finished_file(GDFRecordDispatcher &dispatcher,
					const std::string& filename)
{
  assert(m_nevents > 0);

  for(unsigned int i=0; i<m_nchannel; i++)
    {
      m_mean[i] = m_sum[i]/double(m_nevents);
      m_dev[i]  = sqrt(m_sumsq[i]/double(m_nevents) - m_mean[i]*m_mean[i]);
    }

  cerr << endl
       << "Found " << m_nevents << " pedestal events (from " << m_nallevents 
       << " total)" << endl;
}

void 
GDFPedDevCalculator::ev10(GDFRecordDispatcher &dispatcher, 
			  const struct gdf_ev10_s& record)
{
  if((++m_nallevents % 2000) == 0)cerr << '=';

  int trigger = record.trigger;
  if((trigger&0x1)==0)return; // pedestal events have bit 1 set

  // the above condition usually would be trigger!=0x01 but we want to
  // allow for the case when the PST is misbehaving so allow other
  // bits to be set also.

  unsigned int nchannel=m_nchannel;
  if(nchannel > record.nadc)nchannel=record.nadc;

  if(m_cr_threshold>=0)
    for(unsigned int i=0; i<nchannel; i++)
      if(record.adc[i] > m_cr_threshold)return;

  for(unsigned int i=0; i<nchannel; i++)
    {
      double adc=double(record.adc[i]);
      m_sum[i]   += adc;
      m_sumsq[i] += adc*adc;
    }

  m_nevents++;
}

// ============================================================================
// GDFSkyGainCalculator
// ============================================================================

class GDFSkyGainCalculator: public GDFRecordHandler
{
public:
  GDFSkyGainCalculator(const GDFPedDevCalculator* peddev, 
		       unsigned int nchannels, 
		       double cr_min=60, double cr_max=300);
  GDFSkyGainCalculator(const GDFPedDevCalculator* peddev,
		       const vector<unsigned int>& nchannels, 
		       double cr_min=60, double cr_max=300);
  virtual ~GDFSkyGainCalculator();

  virtual void ev10(GDFRecordDispatcher &dispatcher, 
		    const struct gdf_ev10_s& record);

  virtual void starting_file(GDFRecordDispatcher &dispatcher,
			     const std::string& filename);

  virtual void finished_file(GDFRecordDispatcher &dispatcher,
			     const std::string& filename);

  void printResults(ostream& str);

private:
  typedef vector<unsigned int>                     spectrum_t;
  typedef vector<spectrum_t>                       spectra_t;

  unsigned int                                     m_nevents;

  const GDFPedDevCalculator*                       m_peddev;

  vector<unsigned int>                             m_nchannels;

  double                                           m_cr_min;
  double                                           m_cr_max;

  spectra_t                                        m_spectra;
};

GDFSkyGainCalculator::GDFSkyGainCalculator(const GDFPedDevCalculator* peddev,
					   unsigned int nchannels, 
					   double cr_min, double cr_max)
  : GDFRecordHandler(), 
    m_peddev(peddev), m_nchannels(1,nchannels),
    m_cr_min(cr_min), m_cr_max(cr_max), m_spectra()
{
  m_spectra.resize(nchannels,spectrum_t(MAX_ADC));
} 

GDFSkyGainCalculator::GDFSkyGainCalculator(const GDFPedDevCalculator* peddev,
				        const vector<unsigned int>& nchannels, 
					   double cr_min, double cr_max)
  : GDFRecordHandler(),
    m_peddev(peddev), m_nchannels(nchannels),
    m_cr_min(cr_min), m_cr_max(cr_max), m_spectra()
{
  sort(m_nchannels.begin(), m_nchannels.end());
  cerr << "NChannels: " << m_nchannels[m_nchannels.size()-1];
  m_spectra.resize(nchannels[nchannels.size()-1],spectrum_t(MAX_ADC));
} 

GDFSkyGainCalculator::~GDFSkyGainCalculator()
{

} 

void 
GDFSkyGainCalculator::ev10(GDFRecordDispatcher &dispatcher, 
			   const struct gdf_ev10_s& record)
{
  if((++m_nevents % 2000)==0)cerr << '=';

  int trigger = record.trigger;
  if((trigger&0x1)==1)return; // pedestal events have bit 1 set, skip them

  for(unsigned int channel = 0; 
      channel<m_nchannels[m_nchannels.size()-1]; channel++)
    {
      assert(record.adc[channel] < MAX_ADC);

      double ped = m_peddev->ped(channel);
      double dev = m_peddev->dev(channel);

      unsigned int dc = (unsigned int)(floor(double(record.adc[channel]) - ped));

      if((dc>=0)&&(dc<MAX_ADC))
	m_spectra[channel][dc]++;
    }
}

void 
GDFSkyGainCalculator::starting_file(GDFRecordDispatcher &dispatcher,
				    const std::string& filename)
{
  m_nevents=0;

  unsigned int channel = 0;

  // Main loop over gradiations in camera
  for(vector<unsigned int>::const_iterator nchannels = m_nchannels.begin();
      nchannels != m_nchannels.end(); nchannels++)
    {
      // Second loop over channels in gradiation
      while(channel < *nchannels)
	{
	  double ped = m_peddev->ped(channel);
	  double dev = m_peddev->dev(channel);
	  
	  if(m_cr_min<(dev*MIN_CR_SIGMA))
	    cerr << "WARNING: channel " << channel+1 
		 << " minimum value of " << m_cr_min
		 << " is less than " << MIN_CR_SIGMA 
		 << " sigma above pedestal " << ped << " +/- " << dev << endl;
	  
	  if((m_cr_max+ped)>(MAX_LINEAR_ADC))
	    cerr << "WARNING: channel " << channel+1
		 << " maximum value of " << m_cr_max+ped
		 << " is greater than " << MAX_LINEAR_ADC
		 << " counts" <<  endl;
	  
	  channel++;
	}
    }

  cerr << filename << ": processing events ";
}

void 
GDFSkyGainCalculator::finished_file(GDFRecordDispatcher &dispatcher,
			      const std::string& filename)
{
  cerr << endl;
}

void GDFSkyGainCalculator::printResults(ostream& stream)
{
  unsigned int channel = 0;

  // Main loop over gradiations in camera
  for(vector<unsigned int>::const_iterator nchannels = m_nchannels.begin();
      nchannels != m_nchannels.end(); nchannels++)
    {
      vector<double> gains;
      
      // Second loop over channels in gradiation
      while(channel < *nchannels)
	{
	  unsigned int min_dc = (unsigned int)(ceil(m_cr_min));
	  unsigned int max_dc = (unsigned int)(floor(m_cr_max));

	  vector<unsigned int>* spectrum = &(m_spectra[channel]);

	  unsigned int allcounts = 0;
	  for(unsigned int dc = min_dc; dc < max_dc; dc++)
	    allcounts += (*spectrum)[dc];

	  double maxG = 0;
	  double maxA = 0;
	  double maxLnP = 0;
	  unsigned int maxM = 0;

	  double Gstart = 0.0;
	  double Gstop  = 2.0;
	  double Ginc   = 0.2;

	  double Mstart = 0.8;
	  double Mstop  = 1/Mstart;
	  double Msteps = 20;

	  for(unsigned int iteration=0; iteration<2; iteration++)
	    {
#if 0
	      cout << Gstart << ' ' << Gstop << ' ' << Ginc << '\t'
		   << Mstart << ' ' << Mstop << ' ' << endl;
#endif

	      for(double G=Gstart; G<Gstop; G+=Ginc)
		{
		  double 
		    Mbase = double(allcounts)*G/(1-pow(m_cr_max/m_cr_min,-G));

		  for(unsigned int M=0; M<=Msteps; M++)
		    {
		      double A = Mbase*pow(Mstop/Mstart,
					   0.5+(2.0*double(M)/Msteps-1.0));
	  
		      double lnP = 0;

		      double logallexpect=
			log(A*(1-pow(m_cr_max/m_cr_min,-G))/G);

		      for(unsigned int dc = min_dc; dc < max_dc; dc++)
			{
			  double signal = double(dc);
			  double expect = A*(pow(signal/m_cr_min,-G)-
					     pow((signal+1)/m_cr_min,-G))/G;
			  double counts = (*spectrum)[dc];
			  
			  lnP += counts*log(expect);
			  //lnP += counts*log(expect)-gammln(counts+1)-expect;
			}
		      lnP -= allcounts*logallexpect;
#if 0
		      cout << G << '\t' << A << '\t' << lnP << endl;
#endif

		      if((maxG==0)||(lnP>maxLnP))
			maxG=G, maxA=A, maxLnP=lnP, maxM=M;
		    }
		}

#if 0
	      cout << channel+1 << '\t' << allcounts << '\t'
		   << Gstart << ' ' << Gstop << ' ' << Ginc << '\t'
		   << Mstart << ' ' << Mstop << '\t'
		   << maxG << '\t' << maxA << '\t' << maxLnP << endl;
#endif

	      Gstart = maxG-Ginc/2;
	      Gstop  = maxG+Ginc/2;
	      Ginc   = Ginc/10;

	      allcounts = 
		(unsigned int)(double(floor(maxA/maxG*(1-pow(m_cr_max/m_cr_min,
							   -maxG)))));

	      Mstart = pow(Mstart, 2.0/Msteps);
	      Mstop  = pow(Mstop, 2.0/Msteps);
	    }
	  
	  if((maxM==0)||(maxM==Msteps))
	    cerr << "WARNING: Channel " << channel+1 
		 << " - maximum occurred at edge of M range" << endl;

	  cerr << channel+1 << '\t' << allcounts << '\t' 
	       << maxG << '\t' << maxA << '\t' << maxLnP << endl;
	  
	  gains.push_back(pow(maxA, 1.0/maxG));

	  channel++;
	}

      double sum = 0;
      for(vector<double>::const_iterator i = gains.begin();
	  i != gains.end(); i++)sum += *i;

      double mean = sum/(*nchannels);
      for(unsigned int i = 0; i<*nchannels; i++)
	stream << mean/gains[i] << endl;
    }
}

int main(int argc,char **argv)
{
  try
    {
      while (1)
	{
	  int this_option_optind = optind ? optind : 1;
	  int option_index = 0;
	  
	  static struct option long_options[] =
	    {
	      {"help",  0, 0, 'h'},
	      {0, 0, 0, 0}
	    };
	  
	  int c=getopt_long_only(argc, argv, "h", long_options, 
				 &option_index);
	  
	  if(c==-1)break;
	  
	  switch(c)
	    {
	    case 'h':
	      cout << "Usage: " << *argv << " [options] filename" << endl
		   << endl 
		   << "Options: " << endl
		;
	      exit(EXIT_SUCCESS);
	      
	    case '?':
	    default:
	      cerr << "Unknown option dumm dumm" << endl;
	      exit(EXIT_FAILURE);
	    }
	}

      char* progname=*argv;
      argc-=optind;
      argv+=optind;

      if(argc < 1)
	{
	  cerr << progname << ": no files specified , nothing to do" << endl;
	  exit(EXIT_SUCCESS);
	}

      GDFSystem gdf;
      GDFRecordDispatcher dispatcher(0,true);

      GDFPedDevCalculator pedcalc(379);
      GDFSkyGainCalculator skygain(&pedcalc, 379);

      while(argc)
	{
	  try
	    {
	      // First pass, calculate the pedestals and variances
	      dispatcher.resetHandler(&pedcalc);
	      dispatcher.process(*argv);
	      for(unsigned int i=0;i<379;i++)
		cerr << i+1 << '\t' 
		     << pedcalc.ped(i) << '\t' << pedcalc.dev(i) << endl;
	      
	      // Second pass, analyze the PST data
	      dispatcher.resetHandler(&skygain);
	      dispatcher.process(*argv);
	    }
	  catch(const GDFError& x)
	    {
	      cerr << x;
	    }

	  argc--;
	  argv++;
	}

      skygain.printResults(cout);
    }
  catch(const Error& x)
    {
      cerr << x;
    }
}

float gammln(float xx)
{
  double x,y,tmp,ser;
  static double cof[6]={76.18009172947146,-86.50532032941677,
			24.01409824083091,-1.231739572450155,
			0.1208650973866179e-2,-0.5395239384953e-5};
  int j;
  
  y=x=xx;
  tmp=x+5.5;
  tmp -= (x+0.5)*log(tmp);
  ser=1.000000000190015;
  for (j=0;j<=5;j++) ser += cof[j]/++y;
  return -tmp+log(2.5066282746310005*ser/x);
}

