///////////////////////////////////////////////////////////////////////////////

#include<Types.h>
#include<CameraConfiguration.h>
#include<ChannelData.h>
#include<RedEvent.h>
#include<RedFile.h>
#include<Exceptions.h>
#include<Cleaning.h>

using namespace std;

namespace NS_Analysis {

  class FinderError: public Error
  {
  public:
    FinderError(const string& err): Error(err) {}
  };

  class CameraFinder
  {
  public:
    CameraConfiguration* findByNChannels(channelnum_type nchannels) const;
    CameraConfiguration* findByDate() const;
    CameraConfiguration* findForRedFile(RedFile* rf) const;
  };
  
}

inline NS_Analysis::CameraConfiguration* 
NS_Analysis::CameraFinder::
findForRedFile(RedFile* rf) const
{
  int rfEventVersion=rf->events()->tVersion();
  int nchannels=RedEvent::sizeADC(rfEventVersion);
  return findByNChannels(nchannels);
}

inline NS_Analysis::CameraConfiguration* 
NS_Analysis::CameraFinder::
findByNChannels(channelnum_type nchannels) const
{
  string filename;
  if((nchannels == 492) || (nchannels == 490))filename="WC490.h5";
  else if((nchannels == 336) || (nchannels == 331))filename="WC331.h5";
  else if((nchannels == 156) || (nchannels == 151))filename="WC151.h5";
  else if((nchannels == 120) || (nchannels == 109))filename="WC109.h5";
  else throw FinderError("No camera found");

  filename="/home/sfegan/Projects/analysis/package/bin/"+filename;

  return new CameraConfiguration(filename);
}

///////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>

#include <unistd.h>
#include <getopt.h>
#include <time.h>

#include <Types.h>
#include <Exceptions.h>

#include <VSFA.h>

#include <RedHeader.h>
#include <RedFile.h>

#include <ParamFile.h>
#include <ParamFileGenerator.h>

#include <Pedestals.h>
#include <Gains.h>
#include <PedsAndGainsFactory.h>
#include <ODBCPedsAndGainsFactory.h>

#include <Random.h>
#include <ChannelRepresentation.h>
#include <Cleaning.h>
#include <HillasParameterization.h>
#include <CameraConfiguration.h>

#include <ProgressBar.h>
#include <ScriptParser.h>

#include <UtilityFunctions.h>

using NS_Analysis::MPtr;
using NS_Analysis::Error;

using NS_Analysis::CameraConfiguration;

using NS_Analysis::RedHeader;
using NS_Analysis::RedFile;

using NS_Analysis::ParamFile;
using NS_Analysis::ParamFileGenerator;

using NS_Analysis::Gains;
using NS_Analysis::Pedestals;
using NS_Analysis::PedsAndGainsFactory;
using NS_Analysis::ODBCPedsAndGainsFactory;

using NS_Analysis::LinearRNG;
using NS_Analysis::NRRand2;
using NS_Analysis::GaussianRNG;
using NS_Analysis::BMLinearRNGToGaussianRNGAdaptor;

using NS_Analysis::Cleaner;
using NS_Analysis::CleanerPicBnd;
using NS_Analysis::CleanerRegional;
using NS_Analysis::EventChannelReps;
using NS_Analysis::ECRGenerator;
using NS_Analysis::Standard_ECRGenerator;
using NS_Analysis::Padding_ECRGenerator;
using NS_Analysis::HillasParameterization;

using NS_Analysis::TextProgressBar;
using NS_Analysis::QLScriptParser;
using NS_Analysis::ScriptItemHandler;

using NS_Analysis::getFilenameRoot;
using NS_Analysis::getFilenameBase;

using NS_Analysis::CameraFinder;

class GParamdatEngine: public ScriptItemHandler
{
public:
  virtual void trk(const string& tr_name, int tr_date, 
		   const string& n2_name, int n2_date);
  virtual void pair(const string& on_name, int on_date,
		    const string& n2_name, int n2_date,
		    const string& pad_name, int pad_date);
  virtual void matched_pair(const string& on_name, int on_date, 
			    const string& on_n2_name, int on_n2_date,
			    const string& pad_name, int pad_date,
			    const string& pad_n2_name, int pad_n2_date);
  virtual void option(const string& name, const vector<string>& values);
  virtual ~GParamdatEngine();
  
  GParamdatEngine(PedsAndGainsFactory* pgfact, double pic, double bnd);

private:
  PedsAndGainsFactory*  m_pgfact;

  vector<string>        m_cleaner;

  int                   m_seed;

  void processFile(const string& on_name, int on_date,
		   const string& n2_name, int n2_date,
		   bool pad=false,
		   const string& pad_name="", int pad_date=0);
};

GParamdatEngine::GParamdatEngine(PedsAndGainsFactory* pgfact, 
				 double pic, double bnd):
  m_pgfact(pgfact), m_seed(0)
{
  m_cleaner.push_back("picbnd");

  ostringstream picstr;
  picstr << pic;
  m_cleaner.push_back(picstr.str());

  ostringstream bndstr;
  bndstr << bnd;
  m_cleaner.push_back(bndstr.str());
}

GParamdatEngine::~GParamdatEngine()
{
}

void GParamdatEngine::option(const string& name, const vector<string>& values)
{
  if(name == "cleaner")
    {
      if(values.size() == 0)
	{
	  Error err("GParamdatEngine::option");
	  err.stream() << "Insufficient inputs to \"cleaner\" option";
	  throw(err);
	}

      m_cleaner = values;
    }
}

void GParamdatEngine::trk(const string& tr_name, int tr_date, 
			  const string& n2_name, int n2_date)
{
  processFile(tr_name, tr_date, n2_name, n2_date);
}

void GParamdatEngine::pair(const string& on_name, int on_date,
			   const string& n2_name, int n2_date,
			   const string& pad_name, int pad_date)
{
  processFile(on_name, on_date, n2_name, n2_date, true, pad_name, pad_date);
  processFile(pad_name, pad_date, n2_name, n2_date, true, on_name, on_date);
}

void 
GParamdatEngine::matched_pair(const string& on_name, int on_date, 
			      const string& on_n2_name, int on_n2_date,
			      const string& pad_name, int pad_date,
			      const string& pad_n2_name, int pad_n2_date)
{
  processFile(on_name, on_date, on_n2_name, on_n2_date, 
	      true, pad_name, pad_date);
  processFile(pad_name, pad_date, pad_n2_name, pad_n2_date, 
	      true, on_name, on_date);
}

void GParamdatEngine::processFile(const string& on_name, int on_date,
				  const string& n2_name, int n2_date,
				  bool pad,
				  const string& pad_name, int pad_date)
{
  RedFile rf;

  cout << "---------------------------------- Input file " 
       << "----------------------------------" << endl;
  
  string ifilename=on_name+".h5";
  
  rf.open(ifilename);
  cout << "File:             " << ifilename << endl;
  
  RedHeader rh;
  rf.header()->read(0,&rh,1);
  
  cout << "Raw events:       " << rh.nevents() << endl;
  cout << "Live time:        " << rh.live_time() << endl;
  cout << "Source:           " << rh.source() << endl;
  cout << "  RA:             " << rh.ra() << endl;
  cout << "  Dec:            " << rh.dec() << endl;
  cout << "Date:             " << rh.date() << endl;
  cout << "Start Time UT:    " << rh.ut() << endl;
  cout << "  Siderial:       " << rh.st() << endl;
  cout << "  MJD:            " << rh.mjd() << endl;
  cout << "Telescope Az:     " << rh.azimuth() << endl;
  cout << "          El:     " << rh.elevation() << endl;
  cout << "Comments:         " << rh.comms() << endl;

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  cout << endl;
  cout << "------------------------------------ Camera " 
       << "------------------------------------" << endl;
  
  CameraFinder camfinder;
  auto_ptr<CameraConfiguration> cam(camfinder.findForRedFile(&rf));
  
  cout << "Using camera:     " << cam->description() << endl;
  cout << "Channels:         " << cam->nchannels() << endl;
  
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
      
  cout << endl;
  cout << "------------------------------------ Cleaner " 
       << "-----------------------------------" << endl;
  
  auto_ptr<Cleaner> cleaner;

  string cleanername = m_cleaner[0];
  if(cleanername == "picbnd")
    {
      double pic = 5.00;
      double bnd = 2.25;
      if(m_cleaner.size() == 3)
	{
	  const char *input;
	  
	  input = m_cleaner[1].c_str();
	  istringstream( input ) >> pic;
	  
	  input = m_cleaner[2].c_str();
	  istringstream( input ) >> bnd;
	}
      
      cleaner.reset(new CleanerPicBnd(cam.get(), pic, bnd));

      cout << "Cleaner:          " << "Picture/Boundary" << endl;
      cout << "Picture Level:    " << pic << " sigma" << endl;
      cout << "Boundary Level:   " << bnd << " sigma" << endl;
    }
  else if(cleanername == "regional")
    {
      double ilevel = 5.50;
      double rlevel = 2.00;
      int    mult   = 3;
      if(m_cleaner.size() == 4)
	{
	  const char *input;
	  
	  input = m_cleaner[1].c_str();
	  istringstream( input ) >> ilevel;
	  
	  input = m_cleaner[2].c_str();
	  istringstream( input ) >> rlevel;
	  
	  input = m_cleaner[3].c_str();
	  istringstream( input ) >> mult;
	}
      cleaner.reset(new CleanerRegional(cam.get(), ilevel, rlevel, mult));

      cout << "Cleaner:          " << "Regional" << endl;
      cout << "Immediate Level:  " << ilevel << " sigma" << endl;
      cout << "Regional Level:   " << rlevel << " sigma" << endl;
      cout << "Multiplicity:     " << mult << " channel" << endl;
    }
  
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  cout << endl;
  cout << "------------------------------------- Gains " 
       << "------------------------------------" << endl;

  auto_ptr<Gains> gains;
  
  if(n2_name == "none")
    {
      gains.reset(new Gains(cam->nchannels()));
      for(int c=0;c<cam->nchannels();c++)gains->val(c)=1.0;
    }
  else
    {
      if(n2_date==-1)n2_date=rh.date();
      cout << "Searching for:    " << n2_name 
	   << " on date " << n2_date << endl;
      
      gains.reset(m_pgfact->loadGains(n2_name,n2_date));
      if(gains.get() == 0)
	{
	  cout << endl
	       << "Gains could not be found!" << endl;
	  exit(EXIT_FAILURE);
	}
      
      cout << "Nevents:          " << gains->nevents() << endl;
      cout << "Mean signal:      " << gains->meanSignalMean() 
	   << " +/- " << gains->meanSignalDev() << " DC" << endl;
      cout << "Camera:           " << gains->camera() << endl;
      cout << "Comment:          " << gains->comment() << endl;
    }  

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  
  cout << endl;
  cout << "----------------------------------- Pedestals " 
       << "----------------------------------" << endl;
  
  string onfile_tag = getFilenameBase(ifilename);
  
  if(on_date==-1)on_date=rh.date();
  cout << "Searching for:    " << on_name << " on date " << on_date << endl;
  
  auto_ptr<Pedestals> peds(m_pgfact->loadPeds(on_name,on_date));
  if(peds.get() == 0)
    {
      cout << endl
	   << "Pedestals could not be found!" << endl;
      exit(EXIT_FAILURE);
    }
  
  cout << "Nevents:          " << peds->nevents() << endl;
  cout << "Camera:           " << peds->camera() << endl;
  cout << "Comment:          " << peds->comment() << endl;
  
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  auto_ptr<ECRGenerator> ecrgen;
  if(pad)
    {
      cout << endl;
      cout << "---------------------------- Padding File Pedestals " 
	   << "----------------------------" << endl;
      
      
      if(pad_date==-1)pad_date=rh.date();
      cout << "Searching for:    " << pad_name << " on date " 
	   << pad_date << endl;
      
      auto_ptr<Pedestals> 
	padpeds(m_pgfact->loadPeds(pad_name,pad_date));
      
      if(padpeds.get() == 0)
	{
	  cout << endl
	       << "Pedestals could not be found!" << endl;
	  exit(EXIT_FAILURE);
	}
      
      cout << "Nevents:          " << padpeds->nevents() << endl;
      cout << "Camera:           " << padpeds->camera() << endl;
      cout << "Comment:          " << padpeds->comment() << endl;
      
      cout << endl;
      cout << "------------------------------------ Padding " 
	   << "-----------------------------------" << endl;
      if(m_seed==0)m_seed=time(0);
      cout << "RNG Seed:         " << m_seed << endl;
      
      MPtr<LinearRNG> 
	lrng(new NRRand2(m_seed), true);
      MPtr<GaussianRNG> 
	rng(new BMLinearRNGToGaussianRNGAdaptor(lrng),true);
      m_seed=0;

      ecrgen.reset(new Padding_ECRGenerator(cam.get(),cleaner.get(),
					    rng,gains.get(),
					    peds.get(),padpeds.get()));
    }
  else
    {
      ecrgen.reset(new Standard_ECRGenerator(cam.get(),cleaner.get(),
					     gains.get(),peds.get()));
    }
  
  ecrgen->dump(cout);
  
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
    
  string ofilename=on_name+".ph5";
  cout << endl;
  cout << "---------------------------------- Output File " 
       << "---------------------------------" << endl;
  cout << "File:                 " << ofilename << endl;
      
  ParamFile pf(ofilename,1);
  
  cout << endl 
       << "Parameterizing events..." << endl;
  
  HillasParameterization hp(cam.get());
  TextProgressBar pb;
  
  ParamFileGenerator pfgen(ecrgen.get(),&hp,&pb);
  pfgen.generate(&rf,&pf);
}

int main( int argc, char **argv )
{
  double pic_level=4.25;
  double bnd_level=2.25;
  
  string scriptname;

  try
    {
      while (1)
	{
	  int this_option_optind = optind ? optind : 1;
	  int option_index = 0;
	  
	  static struct option long_options[] =
	  {
	    {"pic", 1, 0, 1},
	    {"bnd", 1, 0, 2},
	    {"script", 1, 0, 's'},
	    {0, 0, 0, 0}
	  };
	  
	  int c=getopt_long_only(argc, argv, "gc", long_options, 
				 &option_index);
	  
	  if(c==-1)break;
	  
	  switch(c)
	    {
	    case 1:
	      istringstream(optarg) >> pic_level;
	      break;
	      
	    case 2:
	      istringstream(optarg) >>  bnd_level;
	      break;
	    
	    case 's':
	      scriptname=optarg;
	      break;
  
	    case '?':
	    default:
	      exit(EXIT_FAILURE);
	    }
	}
      
      char* progname=*argv;
      argc-=optind;
      argv+=optind;
      
      ODBCPedsAndGainsFactory pgfact("DSN=AnalysisDB");
      GParamdatEngine gpe(&pgfact,pic_level,bnd_level);

      if(scriptname != "")
	{
	  QLScriptParser parser(&gpe);
	  parser.parse(scriptname);
	}
      else
	{
	  if(argc < 2)
	    {
	      cerr << "Usage: " << progname << " [-script name]||[<filename> "
		   << "<gains>]" << endl;
	      exit(EXIT_FAILURE);
	    }

	  gpe.trk(argv[0],-1,argv[1],-1);
	}
    }
  catch(const Error& x)
    {
      cerr << x;
    }
  catch(const odbc::SQLException& x)
    {
      cerr << "ODBC++ Exception: " << x.what() << endl;
    }

  for(unsigned i=0;i<NS_Analysis::Cleaner::s_count.size(); i++)
    std::cout << i << ' ' << NS_Analysis::Cleaner::s_count[i] << std::endl;
}
