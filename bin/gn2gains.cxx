///////////////////////////////////////////////////////////////////////////////

#include<Types.h>
#include<CameraConfiguration.h>
#include<ChannelData.h>
#include<RedEvent.h>
#include<RedFile.h>
#include<Exceptions.h>

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

#include<math.h>
#include<unistd.h>
#include<getopt.h>

#include<iostream>
#include<iomanip>
#include<memory>

#include<hdf5/hdf5.h>

#include<RedFile.h>
#include<RedHeader.h>
#include<Gains.h>
#include<Pedestals.h>
#include<ProgressBar.h>
#include<GainsCalc.h>
#include<ODBCPedsAndGainsFactory.h>
#include<CameraConfiguration.h>
#include<Exceptions.h>
#include<ScriptParser.h>
#include<UtilityFunctions.h>

using NS_Analysis::MPtr;
using NS_Analysis::CameraConfiguration;
using NS_Analysis::TextProgressBar;
using NS_Analysis::RedFile;
using NS_Analysis::RedHeader;
using NS_Analysis::Gains;
using NS_Analysis::Pedestals;
using NS_Analysis::N2GainsCalculator;
using NS_Analysis::PedsAndGainsFactory;
using NS_Analysis::ODBCPedsAndGainsFactory;
using NS_Analysis::CameraFinder;
using NS_Analysis::Error;
using NS_Analysis::FileError;
using NS_Analysis::QLScriptParser;
using NS_Analysis::ScriptItemHandler;
using NS_Analysis::getFilenameBase;

class GN2GainsEngine: public ScriptItemHandler
{
public:
  virtual void n2(const string& n2_name, int n2_date);
  virtual ~GN2GainsEngine();

  GN2GainsEngine(PedsAndGainsFactory* pf, bool del=false): 
    ScriptItemHandler(), m_gainsfact(pf), m_deletegains(del) {}
  
private:
  PedsAndGainsFactory*  m_gainsfact;
  bool                  m_deletegains;
  TextProgressBar       m_pb;

  vector<string>        m_failedfiles;

  void processFile(const string& filename, int date);
};

void GN2GainsEngine::n2(const string& n2_name, int n2_date)
{
  processFile(n2_name,n2_date);
}

GN2GainsEngine::~GN2GainsEngine()
{
  if(m_failedfiles.size() != 0)
    {
      cerr << endl
	   << "Failed files list" << endl;
      for(vector<string>::iterator x=m_failedfiles.begin(); 
	  x != m_failedfiles.end(); x++)
	cerr << *x << endl;
    }
}

void GN2GainsEngine::processFile(const string& filename, int date)
{
  int gainsADCsaturation=1023;   // Any ADC > this level => reject event
  double gainsLowSignal=50;      // ADC < this => probably saw no light
  
  double gainsLowSignalRejectionFrac=0.25; // Max fraction of all tubes
                                           // that we allow to have low
                                           // signal before rejecting event
  
  double gainLo = 0.1;  // if tubes gain is less than this or more
  double gainHi = 5.0;  // than this then assume its gone mad and mask it

  int gainMinEvents = 50;

  string filenamebase=getFilenameBase(filename);
  string filenamefull=filename;
  if(filenamefull == filenamebase)filenamefull+=".h5";

  MPtr<RedFile> rf;

  try
    {
      rf.manage(new RedFile(filenamefull));
    }
  catch(const FileError& x)
    {
      cerr << "Caught error trying to open " << filenamefull << ".. skipping"
	   << endl;
      cerr << "Error is: " << x;
      m_failedfiles.push_back(filename);
      return;
    }

  RedHeader rh;
  rf->header()->read(0,&rh,1);
  if(date==-1)date=rh.date();

  cerr << "File: " << filenamebase;

  MPtr<Gains> gains;
  gains.manage(m_gainsfact->loadGains(filenamebase,date));
  if((gains.get() != 0) && (m_deletegains))
    {
      m_gainsfact->deleteGains(filenamebase,date);
      gains.reset();
    }
  else if(gains.get() != 0)
    {
      cerr << "  Gains already in DB.. skipping this file" << endl;
      return;
    }

  MPtr<Pedestals> peds;
  peds.manage(m_gainsfact->loadPeds(filenamebase,date));

  if(peds.get() == 0)
    {
      Error err("GN2GainsEngine::processFile");
      cerr << "  Pedestals not found, skipping this file" << endl;
      err.stream() << "Pedestals not found, skipping this file" << endl;
      m_failedfiles.push_back(filename);
      return;
    }
  
  cerr << "  Calculating Gains..." << endl;
  
  CameraFinder camfinder;
  auto_ptr<CameraConfiguration> cam(camfinder.findForRedFile(rf.get()));
  
  N2GainsCalculator gcalc(cam.get(),gainsADCsaturation, 
			  gainsLowSignal, gainsLowSignalRejectionFrac,
			  gainLo,gainHi);
  gcalc.process(rf.get(),peds.get(),&m_pb);
  gains.manage(gcalc.generateGains());
  
  if(gains->nevents() < gainMinEvents)
    {
      cerr << filenamefull << ": Only " << gains->nevents() 
	   << " good events found (less than " << gainMinEvents << ")"
	   << ", skipping this file!" << endl;
      m_failedfiles.push_back(filename);
      return;
    }      
  
  m_gainsfact->saveGains(gains.get(),filenamebase,date);
}

int main(int argc, char** argv)
{
  try
    {
      ODBCPedsAndGainsFactory gainsfact("DSN=AnalysisDB");
      
      string scriptname;
      
      while (1)
	{
	  int this_option_optind = optind ? optind : 1;
	  int option_index = 0;
	  
	  static struct option long_options[] =
	    {
	      {"script", 1, 0, 's'},
	      {0, 0, 0, 0}
	    };
	  
	  int c=getopt_long_only(argc, argv, "s:", long_options, &option_index);
	  
	  if(c==-1)break;
	  
	  switch(c)
	    {
	    case 's':
	      scriptname=optarg;
	      break;
	    }
	}
      
      char *progname=*argv;
      
      argc-=optind;
      argv+=optind;
      
      if((argc<1) && (scriptname==""))
	{
	  cerr << "Usage: " << progname << " [-s script] filename... " << endl;
	  exit(EXIT_FAILURE);
	}
  
      TextProgressBar pb;
      GN2GainsEngine gn2p(&gainsfact);

      if(scriptname != "")
	{
	  QLScriptParser parser(&gn2p);
	  parser.parse(scriptname);
	}
      else
	{
	  for(int i=0; i<argc; i++)
	    {
	      gn2p.n2(*argv,-1);
	      argv++;
	    }
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
}
