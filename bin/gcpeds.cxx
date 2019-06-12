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
#include<Pedestals.h>
#include<ProgressBar.h>
#include<PedestalsCalc.h>
#include<ODBCPedsAndGainsFactory.h>
#include<CameraConfiguration.h>
#include<Exceptions.h>
#include<ScriptParser.h>
#include<UtilityFunctions.h>

using NS_Analysis::CameraConfiguration;
using NS_Analysis::TextProgressBar;
using NS_Analysis::RedFile;
using NS_Analysis::RedHeader;
using NS_Analysis::Pedestals;
using NS_Analysis::PedestalsCalculator;
using NS_Analysis::PedsAndGainsFactory;
using NS_Analysis::ODBCPedsAndGainsFactory;
using NS_Analysis::CameraFinder;
using NS_Analysis::Error;
using NS_Analysis::QLScriptParser;
using NS_Analysis::ScriptItemHandler;
using NS_Analysis::getFilenameBase;

class GCPedsEngine: public ScriptItemHandler
{
public:
  virtual void n2(const string& n2_name, int n2_date);
  virtual void trk(const string& tr_name, int tr_date, 
		   const string& n2_name, int n2_date);
  virtual void pair(const string& on_name, int on_date,
		    const string& n2_name, int n2_date,
		    const string& pad_name, int pad_date);
  virtual ~GCPedsEngine();

  GCPedsEngine(PedsAndGainsFactory* pf, bool del=false): 
    ScriptItemHandler(), m_pedsfact(pf), m_deletepeds(del) {}
  
private:
  PedsAndGainsFactory*  m_pedsfact;
  bool                  m_deletepeds;
  TextProgressBar       m_pb;

  void processFile(const string& filename, int date);
};

void GCPedsEngine::n2(const string& n2_name, int n2_date)
{
  processFile(n2_name,n2_date);
}

void GCPedsEngine::trk(const string& tr_name, int tr_date, 
		       const string& n2_name, int n2_date)
{
  processFile(tr_name,tr_date);
}

void GCPedsEngine::pair(const string& on_name, int on_date,
			const string& n2_name, int n2_date,
			const string& pad_name, int pad_date)
{
  processFile(on_name,on_date);
  processFile(pad_name,pad_date);
}

GCPedsEngine::~GCPedsEngine()
{
}

void GCPedsEngine::processFile(const string& filename, int date)
{
  int pcutoff=75;
  double plofact=0.6;
  double phifact=1.5;
  
  H5garbage_collect();
  RedFile rf(filename+".h5");

  RedHeader rh;
  rf.header()->read(0,&rh,1);
  if(date==-1)date=rh.date();

  string filenamebase=getFilenameBase(filename);
	  
  cerr << "File: " << filenamebase;

  Pedestals* peds=0;
	  
  peds=m_pedsfact->loadPeds(filenamebase,date);
  if((peds == 0) || (m_deletepeds))
    {
      if((m_deletepeds)&&(peds!=0))
	{
	  cerr << " .. peds found in DB, deleting and recalculating now" 
	       << endl;
	  m_pedsfact->deletePeds(filenamebase,date);
	  delete peds;
	}
      else cerr << " .. peds not found in DB, calculating now" << endl;
      
      CameraFinder camfinder;
      auto_ptr<CameraConfiguration> cam(camfinder.findForRedFile(&rf));
      
      PedestalsCalculator pedcalc(cam.get(), 
				  pcutoff, plofact, phifact);
      pedcalc.process(&rf,&m_pb);
      peds = pedcalc.generatePedestals();
      
      m_pedsfact->savePeds(peds,filenamebase,date);
    }
  else
    {
      cerr << " .. peds found in DB, skipping" << endl;
    }
  
  delete peds;
}

int main(int argc, char** argv)
{
  try
    {
      ODBCPedsAndGainsFactory pedsfact("DSN=AnalysisDB");
      
      string scriptname;
      bool deletepeds = false;

      while (1)
	{
	  int this_option_optind = optind ? optind : 1;
	  int option_index = 0;
	  
	  static struct option long_options[] =
	    {
	      {"delete", 0, 0, 'd'},
	      {"script", 1, 0, 's'},
	      {0, 0, 0, 0}
	    };
	  
	  int c=getopt_long_only(argc, argv, "ds:", long_options, &option_index);
	  
	  if(c==-1)break;
	  
	  switch(c)
	    {
	    case 'd':
	      deletepeds=true;
	      break;

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
	  cerr << "Usage: " << endl 
	       << "   " << progname << " [options] --script scriptfile" << endl
	       << "or " << progname << " [options] filename ...." << endl
	       << endl 
	       << "Options: " << endl
	       << "         --delete : delete peds if they are in DB already"
	       << endl;
	  exit(EXIT_FAILURE);
	}
  
      TextProgressBar pb;
      GCPedsEngine gcp(&pedsfact,deletepeds);

      if(scriptname != "")
	{
	  QLScriptParser parser(&gcp);
	  parser.parse(scriptname);
	}
      else
	{
	  for(int i=0; i<argc; i++)
	    {
	      cerr << *argv << endl;
	      gcp.trk(*argv,-1,"",-1);
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
