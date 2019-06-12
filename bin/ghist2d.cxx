#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <getopt.h>
#include <cstdlib>
#include <cmath>

#include <Exceptions.h>

#include <RedHeader.h>
#include <HillasParam.h>
#include <ParamFile.h>

#include <ScriptParser.h>
#include <ProgressBar.h>

#include <TwoDimensionalParameterization.h>
#include <TwoDimensionalHistGenerator.h>

#include <Binner.h>

using NS_Analysis::Error;

using NS_Analysis::RedHeader;
using NS_Analysis::HillasParam;
using NS_Analysis::ParamFile;

using NS_Analysis::Summer;
using NS_Analysis::TwoDimBinner;

using NS_Analysis::QLScriptParser;
using NS_Analysis::ScriptItemHandler;

using NS_Analysis::TwoDimensionalHistGenerator;
using NS_Analysis::TwoDimensionalParameterization;
using NS_Analysis::DoublePoint2DParametrization;
using NS_Analysis::Asymmetry2DParametrization;
using NS_Analysis::TwoDimensionalDistCalculator;

using NS_Analysis::TextProgressBar;

using namespace std;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class SimpleEllipticity2DDistCalc: public TwoDimensionalDistCalculator
{
public:
  SimpleEllipticity2DDistCalc(double fac): m_factor(fac) {}

  virtual double edist(const HillasParam* hp, 
		       double& edisterror, double& alphaerror) const ;

private:
  double m_factor;
};

double
SimpleEllipticity2DDistCalc::
edist(const HillasParam* hp, double& edisterror, double& alphaerror) const 
{
  const double dist=m_factor*(1.0-hp->width()/hp->length());
  edisterror=alphaerror=0;
  return dist;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static double deMungeRA(double ra)
{
  double hours = floor(ra/10000);
  double mins  = floor(fmod(ra/100.0,100.0));
  double secs  = fmod(ra,100.0);
  return((hours+(mins+secs/60.0)/60.0)/12.0*M_PI);
}

static double deMungeDEC(double dec)
{
  bool positive = dec>=0.0;
  dec=fabs(dec);

  double deg   = floor(dec/10000);
  double mins  = floor(fmod(dec/100.0,100.0));
  double secs  = fmod(dec,100.0);
  if(positive)return((deg+(mins+secs/60.0)/60.0)/180.0*M_PI);
  else return(-(deg+(mins+secs/60.0)/60.0)/180.0*M_PI);
}

class TrackingInfoError: public Error
{
public:
  TrackingInfoError(const string& func): Error(func) {}
  
  virtual string exceptionType() const;
};

string TrackingInfoError::exceptionType() const
{
  return "TrackingInfoError";
}


class GHist2DEngine: public ScriptItemHandler
{
public:
  GHist2DEngine(double bs, TwoDimensionalParameterization* par,
		const string& ext): 
    m_pb(), m_on(bs,par,&m_pb), m_off(bs,par,&m_pb), 
    m_have_ra(false), m_have_dec(false), m_have_mjd(false),
    m_filename_extension(ext) {}
  
  virtual void pair(const string& on_name, int on_date,
		    const string& n2_name, int n2_date,
		    const string& pad_name, int pad_date);

  virtual void option(const string& name, const vector<string>& values);
  
  virtual ~GHist2DEngine();
  
private:
  TextProgressBar                 m_pb;

  TwoDimensionalHistGenerator     m_on;
  TwoDimensionalHistGenerator     m_off;

  string                          m_filename_extension;

  bool                            m_have_ra;
  bool                            m_have_dec;

  bool                            m_have_mjd;

  double                          m_ra;
  double                          m_dec;
  double                          m_mjd;

  void processFile(const string& filename, TwoDimensionalHistGenerator& gen);
  void print(ostream& stream, const string& prefix, 
	     const TwoDimBinner<Summer>& binner) const;
};

GHist2DEngine::~GHist2DEngine()
{
  ofstream on("on2d.dat");
  print(on,"", m_on.totalResults().twoDim());
  ofstream off("off2d.dat");
  print(off,"",m_off.totalResults().twoDim());
}

void GHist2DEngine::print(ostream& stream, const string& prefix, 
			  const TwoDimBinner<Summer>& binner) const
{
  int miny=binner.valToYBin(-2.0);
  int maxy=binner.valToYBin(2.0);

  for(int ybin=miny; ybin<=maxy; ybin++)
    {
      stream << setiosflags(ios::fixed);
      
      if(prefix != "")
	stream << setiosflags(ios::left) << setw(4) << prefix.c_str()
	       << resetiosflags(ios::left);
      
      int minx=binner.valToXBin(-2.0,ybin);
      int maxx=binner.valToXBin( 2.0,ybin);

      //      stream << setw(3) << minx << ' ' << maxx ;

      for(int xbin=minx; xbin<=maxx; xbin++)
	stream << ' ' << setw(5) << setprecision(0) 
	     << binner.binCount(xbin,ybin);

      stream << endl;
    }
}


void GHist2DEngine::pair(const string& on_name, int on_date,
			 const string& n2_name, int n2_date,
			 const string& pad_name, int pad_date)
{
  double stored_ra;

  processFile(on_name,  m_on);

  if(m_have_ra)stored_ra=m_ra, m_ra += 0.5/12.0*M_PI;
  if(m_have_mjd)m_mjd += 0.5 / 24;  // half an hour

  processFile(pad_name, m_off);
  if(m_have_ra)m_ra=stored_ra;

  m_have_mjd=false;
  m_mjd=0;
}

void GHist2DEngine::option(const string& name, const vector<string>& values)
{
  if(name == "ra")
    {
      int hrs;
      int mins;
      double secs;
      char c;

      const char* value = values[0].c_str();
      istringstream(value) >> hrs >> c >> mins >> c >> secs;
      m_ra = (double(hrs)+(double(mins) + secs/60.0)/60.0)*M_PI/12.0;
      m_have_ra=true;
    }

  else if(name == "dec")
    {
      int deg;
      int mins;
      double secs;
      char c;

      const char* value = values[0].c_str();
      istringstream(value) >> deg >> c >> mins >> c >> secs;
      m_dec = (fabs(double(deg))+(double(mins) + secs/60.0)/60.0)*M_PI/180.0;
      if(values[0][0] == '-')m_dec *= -1.0;
      m_have_dec=true;
    }

  else if(name == "mjd")
    {
      const char* value = values[0].c_str();
      istringstream(value) >> m_mjd;
      m_have_mjd=true;
    }
}

void GHist2DEngine::processFile(const string& filename,
				TwoDimensionalHistGenerator& gen)
{
  string ifilename=filename+m_filename_extension;
  cerr << "Processing: " << ifilename;

  ParamFile pf(ifilename);
  cerr << "...." << endl;

  RedHeader rh;
  pf.header()->read(0,&rh,1);

  if((rh.elevation()==0)&&(rh.azimuth()==0)&&(rh.ra()==0)&&(rh.dec()==0)&&
     (!m_have_ra)&&(!m_have_dec))
    throw TrackingInfoError("GHist2DEngine::processFile");
  
  double ra;
  if(m_have_ra)ra=m_ra;
  else ra=deMungeRA(rh.ra());
		 
  double dec;
  if(m_have_dec)dec=m_dec;
  else dec=deMungeDEC(rh.dec());

  double mjd;
  if(m_have_mjd)mjd=m_mjd;
  else mjd=rh.mjd();

  gen.generate(&pf,ra,dec,mjd);
}


class GHist2DOptionReader: public ScriptItemHandler
{
public:
  GHist2DOptionReader(): ScriptItemHandler(),
			 m_fixbinwidth(false), m_binwidth(0.05),
			 m_fixeta(false), m_eta(1.5) { }
  virtual void option(const string& name, const vector<string>& values);
  
  virtual ~GHist2DOptionReader();

  void setBinWidth(double x) { m_binwidth = x; m_fixbinwidth=true; }
  void setEta(double x) { m_eta = x; m_fixeta=true; }

  double binWidth() const { return m_binwidth; }
  double eta() const { return m_eta; }

private:
  bool m_fixbinwidth;
  double m_binwidth;
  bool m_fixeta;
  double m_eta;
};

GHist2DOptionReader::~GHist2DOptionReader()
{
  // nothing to see here
}

void 
GHist2DOptionReader::option(const string& name, const vector<string>& values)
{
  if(name == "eta")
    {
      const char* value = values[0].c_str();
      if(!m_fixeta)istringstream(value) >> m_eta;
    }
  if(name == "width")
    {
      const char* value = values[0].c_str();
      if(!m_fixbinwidth)istringstream(value) >> m_binwidth;
    }
}

int main(int argc, char** argv)
{
  try
    {
      string scriptname;
      string extension="_cut.ph5";
      bool use_asymmetry = false;

      GHist2DOptionReader optread;

      while (1)
	{
	  int this_option_optind = optind ? optind : 1;
	  int option_index = 0;
	  
	  static struct option long_options[] =
	    {
	      {"eta", 1, 0, 'E'},
	      {"width", 1, 0, 'W'},
	      {"extension", 1, 0, 'e'},
	      {"asymmetry", 1, 0, 'A'},
	      {"script", 1, 0, 's'},
	      {0, 0, 0, 0}
	    };
	  
	  int c=getopt_long_only(argc, argv, "seEWA", long_options, 
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

	    case 'E':
	      optread.setEta(atof(optarg));
	      break;
	      
	    case 'A':
	      use_asymmetry = true;
	      break;

	    case 'W':
	      optread.setBinWidth(atof(optarg));
	      break;

	    case '?':
	    default:
	      exit(EXIT_FAILURE);
	    }
	}

      argc-=optind;
      argv+=optind;

      if(scriptname != "")
	{
	  QLScriptParser parser(&optread);
	  parser.parse(scriptname);
	}

      cerr << "Using eta=" << optread.eta() << endl;
      SimpleEllipticity2DDistCalc ecalc(optread.eta());
      TwoDimensionalParameterization* param;
      
      if(use_asymmetry)
	param = new Asymmetry2DParametrization(&ecalc);
      else
	param = new DoublePoint2DParametrization(&ecalc);

      GHist2DEngine gh2de(optread.binWidth(),param,extension);
      
      if(scriptname != "")
	{
	  QLScriptParser parser(&gh2de);
	  parser.parse(scriptname);
	}

      delete param;
    }
  catch (const Error& x)
    {
      cerr << x;
    }
}
