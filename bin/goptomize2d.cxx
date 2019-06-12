#include <unistd.h>
#include <getopt.h>

#include <vector>
#include <iostream>
#include <iomanip>

#include <Exceptions.h>
#include <EventInfo.h>
#include <HillasParam.h>
#include <ParamFile.h>
#include <Binner.h>
#include <ScriptParser.h>
#include <ProgressBar.h>

using NS_Analysis::Error;
using NS_Analysis::EventInfo;
using NS_Analysis::HillasParam;
using NS_Analysis::ParamFile;
using NS_Analysis::ParamEventSelector;
using NS_Analysis::ParamEventOperator;
using NS_Analysis::ParamFileEventProcessor;

using NS_Analysis::Binner;
using NS_Analysis::Summer;

using NS_Analysis::ScriptItemHandler;
using NS_Analysis::QLScriptParser;

using NS_Analysis::TextProgressBar;

using namespace std;

class DistanceCalculator
{
public:
  virtual double dist(double parameter, const HillasParam* hp) = 0;
};

class GOptomize2DEngine: public ScriptItemHandler, private ParamEventOperator
{
public:
  virtual void pair(const string& on_name, int on_date,
		    const string& n2_name, int n2_date,
		    const string& pad_name, int pad_date);

  virtual ~GOptomize2DEngine();

  GOptomize2DEngine(DistanceCalculator* rdc,
		    double firstparamval, double paramvalinc, int nvals,
		    const string& ext="_cut.ph5"):
    m_proc(this,&m_pb), m_distcalc(rdc), 
    m_first_param_value(firstparamval), m_param_value_interval(paramvalinc),
    m_n_param_values(nvals), m_on(false), m_filename_extension(ext),
    m_on_plots(nvals,Summer(0.05,Binner::BIN_START)),
    m_off_plots(nvals,Summer(0.05,Binner::BIN_START))
  {}

private:
  void operateOnEvent(int evno, 
		      const EventInfo* ei, const HillasParam* hp);

  TextProgressBar           m_pb;
  ParamFileEventProcessor   m_proc;

  DistanceCalculator*       m_distcalc;

  double                    m_first_param_value;
  double                    m_param_value_interval;
  int                       m_n_param_values;

  bool                      m_on;

  const string              m_filename_extension;

  vector<Summer>            m_on_plots;
  vector<Summer>            m_off_plots;

  void processFile(const string& filename);
};

static inline int max(int a, int b) { return (a>b)?a:b; }

GOptomize2DEngine::~GOptomize2DEngine()
{
  for(int i=0;i<m_n_param_values;i++)
    {
      double param = m_first_param_value + m_param_value_interval*double(i);

      cout << "ON   " 
	   << resetiosflags(ios::floatfield)
	   << setiosflags(ios::fixed) << setw(8) << setprecision(4) << param;
      
      int onMinBin=m_on_plots[i].valToBin(0.0);
      int onMaxBin=m_on_plots[i].valToBin(1.5);
      for(int bin=onMinBin; bin<=onMaxBin; bin++)
	cout << setw(6) << setprecision(0) << m_on_plots[i].binCount(bin);
      cout << endl;

      /////////////////////////////////////////////////////////////////////////

      cout << "OFF  " 
	   << resetiosflags(ios::floatfield)
	   << setiosflags(ios::fixed) << setw(8) << setprecision(4) << param;
      
      int offMinBin=m_off_plots[i].valToBin(0.0);
      int offMaxBin=m_off_plots[i].valToBin(1.5);
      for(int bin=offMinBin; bin<=offMaxBin; bin++)
	cout << setw(6) << setprecision(0) << m_off_plots[i].binCount(bin);
      cout << endl;

      /////////////////////////////////////////////////////////////////////////

      cout << "DIFF " 
	   << resetiosflags(ios::floatfield)
	   << setiosflags(ios::fixed) << setw(8) << setprecision(4) << param;
      
      int nbins=max(offMaxBin-offMinBin, onMaxBin-onMinBin);

      for(int bin=0; bin<=nbins; bin++)
	cout << setw(6) << setprecision(0) 
	     << m_on_plots[i].binCount(onMinBin+bin) - 
	        m_off_plots[i].binCount(offMinBin+bin);
      cout << endl;


    }
}

void GOptomize2DEngine::operateOnEvent(int evno, const EventInfo* ei, 
				       const HillasParam* hp)
{
  const double cospsi       = hp->cos_psi();
  const double sinpsi       = hp->sin_psi();
  const double xc           = hp->xc();
  const double yc           = hp->yc();

  for(int i=0;i<m_n_param_values;i++)
    {
      double param = m_first_param_value + m_param_value_interval*double(i);

      const double edist    = m_distcalc->dist(param, hp);

      const double X1       = xc - edist*cospsi;
      const double Y1       = yc - edist*sinpsi;

      const double X2       = xc + edist*cospsi;
      const double Y2       = yc + edist*sinpsi;

      const double radial1  = sqrt(X1*X1 + Y1*Y1);
      const double radial2  = sqrt(X2*X2 + Y2*Y2);

      if((finite(radial1))&&(radial1 < 2.0))
	{
	  if(m_on)m_on_plots[i].insert(radial1);
	  else m_off_plots[i].insert(radial1);
	}

      if((finite(radial2))&&(radial2 < 2.0))
	{
	  if(m_on)m_on_plots[i].insert(radial2);
	  else m_off_plots[i].insert(radial2);
	}
    }
}

void GOptomize2DEngine::pair(const string& on_name, int on_date,
			     const string& n2_name, int n2_date,
			     const string& pad_name, int pad_date)
{
  m_on=true;
  processFile(on_name);
  m_on=false;
  processFile(pad_name);
}

void GOptomize2DEngine::processFile(const string& filename)
{
  string ifilename=filename+m_filename_extension;
  cerr << "Opening: " << ifilename;
  ParamFile pf(ifilename);
  cerr << " ... OK, now processing" << endl;
  m_proc.run(&pf);
}

///////////////////////////////////////////////////////////////////////////////

class SimpleEllipticityDistanceCalculator:
  public DistanceCalculator
{
public:
  virtual double dist(double parameter, const HillasParam* hp);
};

double
SimpleEllipticityDistanceCalculator::
dist(double parameter, const HillasParam* hp)
{
  const double edist=parameter*(1.0-hp->width()/hp->length());
  return edist;
}

///////////////////////////////////////////////////////////////////////////////

class SizeDependentEllipticityDistanceCalculator:
  public DistanceCalculator
{
public:
  virtual double dist(double parameter, const HillasParam* hp);
};

double
SizeDependentEllipticityDistanceCalculator::
dist(double parameter, const HillasParam* hp)
{
  const double logsize=log10(hp->size());
  const double edist=parameter*logsize*(1.0-hp->width()/hp->length());
  return edist;
}

///////////////////////////////////////////////////////////////////////////////

int main( int argc, char **argv )
{
  try
    {
      string scriptname;
      string extension="_cut.ph5";
      
      double param_start     = 0.5;
      double param_interval  = 0.05;
      int    param_steps     = 55;

      while (1)
	{
	  int this_option_optind = optind ? optind : 1;
	  int option_index = 0;
	  
	  static struct option long_options[] =
	    {
	      {"extension", 1, 0, 'e'},
	      {"script", 1, 0, 's'},
	      {0, 0, 0, 0}
	    };
	  
	  int c=getopt_long_only(argc, argv, "se", long_options, 
				 &option_index);
	  
	  if(c==-1)break;
	  
	  switch(c)
	    {
	    case 's':
	      scriptname=optarg;
	      break;

	    case 'e':
	      extension=optarg;
	      break;

	    case '?':
	    default:
	      exit(EXIT_FAILURE);
	    }
	}

      argc-=optind;
      argv+=optind;

      SimpleEllipticityDistanceCalculator ecalc;
      GOptomize2DEngine go2de(&ecalc,param_start,param_interval,param_steps,
			      extension);
      
      if(scriptname != "")
	{
	  QLScriptParser parser(&go2de);
	  parser.parse(scriptname);
	}
    }
  catch (const Error& x)
    {
      cerr << x;
    }
}
