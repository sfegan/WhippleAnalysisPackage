/*

  Program: timesfft

  Purpose: Takes a parameterized or raw file, bins the events in time and
           produces an FFT of the event times. The size of the time bins is
	   set on the command line as is the number of points in the spectrum.
	   
  Usage:   timesfft [options] filename time_bin_size number_of_spectrum_points

  Params:  filename       File to read in

           time_bin_size  Size of time bins in seconds

           number_of_spectrum_points 
                          Number of points in the spectrum. Actually it is the
                          number of points in the FFT, there is one more point
                          in the spectrum, corresponding to the Nyquest freq

  Options: -raw      Specify that "filename" corresponds to a raw file rather
                     than a parameterized one

           -code8s   Take only the real trigger events from the file. Default
                     is to take all events regardless.

	   -peds     Select only pedestals

  Output:   The power spectrum is output to standard output. The first column
            is frequency (in Hz), the second is power.

  Examples: timesfft /data/mrk421/gt012345.ph5 0.1 2048 > spect.dat

                     See the pedestal events and other features on the 
                     "second" scale. Pipe output to spect.dat

            timesfft -raw /data/nitrogen/gt012345.h5 0.0001 2048 > spect.dat

                     See the frequency of the N2 flasher (750Hz in May 2001)

*/

/* 

   TO DO:   Have PowerSpectrum take Window function as arguement. Provide a
            few options.

            Move PowerSpectrum over to be its own .h, .cxx files. 

            Have its so that huge vector<double> is not created, rather have
	    FFT routine generate binning directly from raw file using a clever
	    DataReader

*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <getopt.h>
#include <cstdlib>
#include <cmath>

#include <Exceptions.h>

#include <HillasParam.h>
#include <ParamFile.h>
#include <AlphaPlotGenerator.h>

#include <RedEvent.h>
#include <RedFile.h>

#include <ScriptParser.h>
#include <ProgressBar.h>

#include <Binner.h>

#include <Types.h>

using NS_Analysis::Error;

using NS_Analysis::HillasParam;
using NS_Analysis::EventInfo;
using NS_Analysis::ParamFile;
using NS_Analysis::ParamFileEventProcessor;
using NS_Analysis::ParamEventOperator;
using NS_Analysis::ParamEventSelector;
using NS_Analysis::ParamEventSelectedOperator;
using NS_Analysis::ByCodePESelector;
using NS_Analysis::AllEventsPESelector;

using NS_Analysis::RedEvent;
using NS_Analysis::RedFile;
using NS_Analysis::RedFileEventProcessor;
using NS_Analysis::RedEventOperator;
using NS_Analysis::RedEventSelector;
using NS_Analysis::RedEventSelectedOperator;
using NS_Analysis::ByCodeRESelector;
using NS_Analysis::AllEventsRESelector;

using NS_Analysis::Summer;
using NS_Analysis::TwoDimBinner;

using NS_Analysis::QLScriptParser;
using NS_Analysis::ScriptItemHandler;

using NS_Analysis::TextProgressBar;

using NS_Analysis::MPtr;

using namespace std;

template<class T> class DataReader
{
public:
  virtual ~DataReader();
  virtual bool haveMoreData() = 0;
  virtual void getData(T& data) = 0;
  T getData() { T data; getData(data); return data; }
};

template<class T> DataReader<T>::~DataReader()
{
}

typedef DataReader<double> DoubleReader;

///////////////////////////////////////////////////////////////////////////////

#include <sys/types.h>
#include <sys/stat.h>
#include <cstdio>

#include <fftw.h>
#include <rfftw.h>

inline static double sqr(double a) { return a*a; }

class PowerSpectrum
{
public:
  PowerSpectrum(double deltaT, int nPoints, DoubleReader* reader);
  virtual ~PowerSpectrum();
 
  void resetReader(DoubleReader* reader) { m_reader=reader; m_data_used=0; }
  int dataUsed() const { return m_data_used*m_spectrum_points; }

  void execute();

  double power(int i);
  double frequency(int i) { return double(i)*m_frequency_unit; }

private:
  int                     m_data_used;
  DoubleReader*           m_reader;
 
  double                  m_frequency_unit;

  int                     m_spectrum_points;

  fftw_real*              m_window;
  double                  m_window_norm;

  fftw_real*              m_overlap;
  fftw_real*              m_data;

  fftw_real*              m_transform;

  double*                 m_spectrum;
  int                     m_spectrum_norm;

  rfftw_plan              m_plan;

  bool readData();
};

PowerSpectrum::PowerSpectrum(double deltaT, int nPoints, DoubleReader* reader):
  m_data_used(0), m_reader(reader), 
  m_window_norm(0),
  m_spectrum_points(nPoints)
{
  m_frequency_unit=1/(double(2*m_spectrum_points)*deltaT);
  
  /// INITIALIZATION OF WINDOW ///

  m_window = new fftw_real[2*m_spectrum_points];
  for(int j=0; j<2*m_spectrum_points; j++) 
    {
      m_window[j]=1.0; // (1.0 - fabs((j - (m_spectrum_points - 0.5)) / 
                       // (m_spectrum_points + 0.5)));
      m_window_norm += sqr(m_window[j]);
    }
  
  /// INITIALIZATION OF DATA STORAGE AREA ///

  m_overlap     = new fftw_real[m_spectrum_points];
  m_data        = new fftw_real[2*m_spectrum_points];
  m_transform   = new fftw_real[2*m_spectrum_points];

  m_spectrum    = new double[m_spectrum_points+1];
  for(int j=0; j<m_spectrum_points; j++) m_spectrum[j] = 0;
  m_spectrum_norm = 0;

  /// INITIALIZATION OF FFTW SYSTEM ///    

  FILE* wisdomFile = fopen("/tmp/fftw_wisdom","r");
  if(wisdomFile)
    {
      if (FFTW_FAILURE == fftw_import_wisdom_from_file(wisdomFile))
	printf("Error reading wisdom!\n");
      fclose(wisdomFile); /* be sure to close the file! */
    }

  /* create a plan for N=64, possibly creating and/or using wisdom */
  m_plan = rfftw_create_plan(2*m_spectrum_points,FFTW_REAL_TO_COMPLEX,
				   FFTW_MEASURE | FFTW_USE_WISDOM);
  
}

PowerSpectrum::~PowerSpectrum()
{
  delete[] m_window;

  delete[] m_spectrum;

  delete[] m_transform;
  delete[] m_data;
  delete[] m_overlap;

  /* always destroy plans when you are done */
  rfftw_destroy_plan(m_plan);

  FILE* wisdomFile = fopen("/tmp/fftw_wisdom","w");
  if(wisdomFile)
    {
      fftw_export_wisdom_to_file(wisdomFile);
      fclose(wisdomFile);
    }
  chmod("/tmp/fftw_wisdom",00666);
}

inline bool PowerSpectrum::readData()
{
  for(int i=0; i<m_spectrum_points; i++)
    {
      if(m_reader->haveMoreData())
	{
	  double data;
	  m_reader->getData(data);
	  m_overlap[i] = data;
	}
      else return false;
    }
  return true;
}

void PowerSpectrum::execute()
{
  cerr << "FFT: ";

  if(!readData())
    {
      cerr << endl;
      return;
    }
  
  while(1)
    {
      for(int i=0; i<m_spectrum_points; i++)m_data[i]=m_overlap[i];
      if(!readData())
	{
	  cerr << endl;
	  return;
	}

      if(m_data_used == 0)m_data_used++;
      m_data_used++;

      for(int i=0; i<m_spectrum_points; i++)
	m_data[m_spectrum_points+i]=m_overlap[i];
      
      for(int i=0;i<m_spectrum_points*2;i++)m_data[i] *= m_window[i];

      //for(int i=0;i<m_spectrum_points*2;i++)cerr << m_window[i] << ' ';
      //cerr << endl;

      rfftw_one(m_plan, m_data, m_transform);

      m_spectrum[0] += sqr(m_transform[0]);
      for(int i=1; i<m_spectrum_points; i++)
	{
	  m_spectrum[i] += 
	  sqr(m_transform[i]) + sqr(m_transform[2*m_spectrum_points - i]);
	}
      m_spectrum[m_spectrum_points] = sqr(m_transform[m_spectrum_points]);
      cerr << '=';
      m_spectrum_norm ++;
    }
  cerr << endl;
}

double PowerSpectrum::power(int i) 
{ 
  double norm = 
    double(m_spectrum_norm) * m_window_norm * double(2 * m_spectrum_points);
  return m_spectrum[i]/norm;
}

///////////////////////////////////////////////////////////////////////////////

template<class T> class VectorDataReader: public DataReader<T>
{
public:
  VectorDataReader(const vector<T>& v): here(v.begin()), last(v.end()) {}
  virtual ~VectorDataReader();
  virtual bool haveMoreData();
  virtual void getData(T& data);

private:
  typename vector<T>::const_iterator here;
  typename vector<T>::const_iterator last;
};

template<class T> VectorDataReader<T>::~VectorDataReader()
{
}

template<class T> bool VectorDataReader<T>::haveMoreData()
{
  return here != last;
}

template<class T> void VectorDataReader<T>::getData(T& data)
{
  data=*here;
  here++;
}

///////////////////////////////////////////////////////////////////////////////

class PFTimeExtractor: public ParamEventOperator
{
public:
  PFTimeExtractor(vector<double>& c, double binsize): 
    m_count(c), m_binsize(binsize) {}
  void operateOnEvent(int evno, 
		      const EventInfo* ei, const HillasParam* hp);

private:
  double          m_binsize;
  vector<double>& m_count;
};

void PFTimeExtractor::operateOnEvent(int evno, 
				   const EventInfo* ei, const HillasParam* hp)
{
  int bin=int(floor(ei->time() / m_binsize));
  //if(evno%100 != 0)return;
  //int bin=int(floor(evno/100 / m_binsize));
  if(m_count.size() <= bin)m_count.resize(bin+1);
  m_count[bin]++;
}

class RFTimeExtractor: public RedEventOperator
{
public:
  RFTimeExtractor(vector<double>& c, double binsize): 
    m_count(c), m_binsize(binsize) {}
  void operateOnEvent(int evno, const RedEvent* re);

private:
  double          m_binsize;
  vector<double>& m_count;
};

void RFTimeExtractor::operateOnEvent(int evno, const RedEvent* re)
{
  int bin=int(floor(re->time() / m_binsize));
  if(m_count.size() <= bin)m_count.resize(bin+1);
  m_count[bin]++;
}

#include<cmath>


int main(int argc, char** argv)
{
  try
    {
      bool rawfile = false;
      int  selection = 0; // 0=all, 1=peds, 8=triggers

      while (1)
	{

	  int this_option_optind = optind ? optind : 1;
	  int option_index = 0;
	  
	  static struct option long_options[] =
	  {
	    {"raw", 0, 0, 1},
	    {"code8s", 0, 0, 2},
	    {"peds", 0, 0, 3},
	    {0, 0, 0, 0}
	  };
	  
	  int c=getopt_long_only(argc, argv, "gc", long_options, 
				 &option_index);
	  
	  if(c==-1)break;
	  
	  switch(c)
	    {
	    case 1:
	      rawfile=true;
	      break;
	      
	    case 2:
	      selection=8;
	      break;
	    
	    case 3:
	      selection=1;
	      break;
	    
	    case '?':
	    default:
	      exit(EXIT_FAILURE);
	    }
	}
      
      string progname(*argv);
      argc-=optind;
      argv+=optind;

      if(argc!=3)
	{
	  cerr << "Usage: " 
	       << progname << " <paramfile> <timebinsize> <nbins>" 
	       << endl;
	  return 0;
	}

      string filename(*argv);
      argv++,argc--;

      double timebinsize;
      istringstream(*argv) >> timebinsize;
      argv++; argc--;

      int nbins;
      istringstream(*argv) >> nbins;
      argv++; argc--;

      cerr << "Input file:              " << filename << endl;
      cerr << "Time bin size:           " << timebinsize << " sec" << endl;
      cerr << "Maximum Frequency:       " << 1.0/(2.0*timebinsize) << " Hz" 
	   << endl;
      cerr << "Number of spectrum bins: " << nbins << endl;

      TextProgressBar pb;

      vector<double> times;

      if(!rawfile)
	{
	  ParamFile pf;
	  pf.open(filename);

	  MPtr<ParamEventSelector> selector;

	  if(selection == 0)
	    {
	      selector.manage(new AllEventsPESelector);
	    }
	  else
	    {
	      selector.manage(new ByCodePESelector(selection));
	    }

	  PFTimeExtractor te(times,timebinsize);
	  ParamEventSelectedOperator op(&te,selector.get());
	  ParamFileEventProcessor proc(&op,&pb);
	  proc.run(&pf);
	}
      else
	{
	  RedFile rf;
	  rf.open(filename);

	  MPtr<RedEventSelector> selector;

	  if(selection == 0)
	    {
	      selector.manage(new AllEventsRESelector);
	    }
	  else
	    {
	      selector.manage(new ByCodeRESelector(selection));
	    }

	  RFTimeExtractor te(times,timebinsize);
	  RedEventSelectedOperator op(&te,selector.get());
	  RedFileEventProcessor proc(&op,&pb);
	  proc.run(&rf);
	}

      cerr << "Number of time bins:     " << times.size() << endl;

      VectorDataReader<double> reader(times);
      PowerSpectrum spec(timebinsize,nbins,&reader);
      spec.execute();

      cerr << "Time bins used in FFT:   " << spec.dataUsed() 
	   << " (" << floor(10000.0*spec.dataUsed()/times.size()+0.5)/100 << "%)" 
	   << endl;

      for(int i=0; i<nbins+1; i++)
	cout << spec.frequency(i) << ' ' << spec.power(i) << endl;
    }
  catch (const Error& x)
    {
      cerr << x;
    }
}
