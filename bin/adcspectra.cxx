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
  //filename="/home/observer/sfegan/package/bin/"+filename;

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
#include <PedestalsCalc.h>
#include <AlphaPlotGenerator.h>

#include <ADCSpectrum.h>

#include <Pedestals.h>
#include <Gains.h>
#include <PedsAndGainsFactory.h>
#include <ODBCPedsAndGainsFactory.h>

#include <Random.h>
#include <ChannelData.h>
#include <ChannelRepresentation.h>
#include <Cleaning.h>
#include <CameraConfiguration.h>

#include <ProgressBar.h>
#include <ScriptParser.h>

#include <UtilityFunctions.h>

using NS_Analysis::MPtr;
using NS_Analysis::Error;

using NS_Analysis::CameraConfiguration;

using NS_Analysis::RedHeader;
using NS_Analysis::RedFile;
using NS_Analysis::RedEventSelector;
using NS_Analysis::StdPedRESelector;
using NS_Analysis::Code8RESelector;

using NS_Analysis::Gains;
using NS_Analysis::Pedestals;
using NS_Analysis::PedsAndGainsFactory;
using NS_Analysis::ODBCPedsAndGainsFactory;

using NS_Analysis::ADCSpectrum;

using NS_Analysis::LinearRNG;
using NS_Analysis::NRRand2;
using NS_Analysis::GaussianRNG;
using NS_Analysis::BMLinearRNGToGaussianRNGAdaptor;

using NS_Analysis::Cleaner;
using NS_Analysis::CleanedState;
using NS_Analysis::ChannelData;
using NS_Analysis::EventChannelReps;
using NS_Analysis::ECRGenerator;
using NS_Analysis::Standard_ECRGenerator;
using NS_Analysis::Padding_ECRGenerator;

using NS_Analysis::TextProgressBar;
using NS_Analysis::QLScriptParser;
using NS_Analysis::ScriptItemHandler;

using NS_Analysis::getFilenameRoot;
using NS_Analysis::getFilenameBase;

using NS_Analysis::CameraFinder;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <RedEvent.h>
using NS_Analysis::RedEvent;
using NS_Analysis::RedFileEventProcessor;
using NS_Analysis::RedEventOperator;
using NS_Analysis::RedEventSelectedOperator;
using NS_Analysis::ProgressBar;

class ADCSpectrumEventInserter
{
public:
  virtual ~ADCSpectrumEventInserter();
  virtual void insertEvent(ADCSpectrum* spectrum, const RedEvent* re) = 0;
};

ADCSpectrumEventInserter::~ADCSpectrumEventInserter()
{
}

class ADCSpectrumBuilder: private RedEventOperator
{
public:
  ADCSpectrumBuilder(ADCSpectrumEventInserter* insert, 
		     RedEventSelector* sel=0, ProgressBar* pb=0);

  void insertFile(ADCSpectrum* spectrum, RedFile* rf);

private:
  void operateOnEvent(int evno, const RedEvent* re);

  ADCSpectrumEventInserter*   m_insert;
  MPtr<RedEventOperator>      m_operator;
  MPtr<RedFileEventProcessor> m_processor;

  ADCSpectrum*                m_spec;
};

ADCSpectrumBuilder::ADCSpectrumBuilder(ADCSpectrumEventInserter* insert,
				       RedEventSelector* sel,
				       ProgressBar* pb):
  RedEventOperator(), m_insert(insert), m_operator(), m_processor()
{
  if(sel)m_operator.manage(new RedEventSelectedOperator(this,sel));
  else m_operator.adopt(this,false);

  m_processor.manage(new RedFileEventProcessor(m_operator.get(),pb));
}

void ADCSpectrumBuilder::insertFile(ADCSpectrum* spectrum, 
				    RedFile* rf)
{
  m_spec=spectrum;
  m_processor->run(rf);
  m_spec=0;
}

void ADCSpectrumBuilder::operateOnEvent(int evno, const RedEvent* re)
{
  m_insert->insertEvent(m_spec,re);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class SignalADCSpectrumInserter: public ADCSpectrumEventInserter
{
public:
  SignalADCSpectrumInserter(ECRGenerator* ecrgen): m_ecrgen(ecrgen) {}
  virtual void insertEvent(ADCSpectrum* spectrum, const RedEvent* re);
  
private:
  ECRGenerator* m_ecrgen;
};

void SignalADCSpectrumInserter::insertEvent(ADCSpectrum* spectrum, 
					    const RedEvent* re)
{
  EventChannelReps* ecr=m_ecrgen->generate(re);
  spectrum->insert(&ecr->light());
  delete ecr;
}

///////////////////////////////////////////////////////////////////////////////

class LogSignalADCSpectrumInserter: public ADCSpectrumEventInserter
{
public:
  LogSignalADCSpectrumInserter(ECRGenerator* ecrgen): m_ecrgen(ecrgen) {}
  virtual void insertEvent(ADCSpectrum* spectrum, const RedEvent* re);
  
private:
  ECRGenerator* m_ecrgen;
};

void LogSignalADCSpectrumInserter::insertEvent(ADCSpectrum* spectrum, 
					    const RedEvent* re)
{
  EventChannelReps* ecr=m_ecrgen->generate(re);
  int nchannels=ecr->light().nchannels();
  ChannelData<double> loglight(nchannels);
  for(int i=0;i<nchannels;i++)
    {
      double light=ecr->light(i);
      if(light >= 1.0)loglight[i]=log10(light)*50;
      else loglight[i]=-1;
    }
  spectrum->insert(&loglight);
  delete ecr;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class BogusCleaner: public Cleaner
{
public:
  BogusCleaner(): Cleaner(0) {}
  virtual void clean(ChannelData<CleanedState>& clean,
		     const ChannelData<double>& signaltonoise) const;
};

void BogusCleaner::clean(ChannelData<CleanedState>& clean,
			 const ChannelData<double>& signaltonoise) const
{
}

int main( int argc, char **argv )
{
  bool   usepadding   = false;
  string pad_name;

  bool   usegains     = false;
  string gains_name;

  bool   useloginserter = false;

  bool   selectCode8s = false;
  bool   subtractPeds = false;

  int seed=0;

  try
    {
      while (1)
	{
	  int this_option_optind = optind ? optind : 1;
	  int option_index = 0;
	  
	  static struct option long_options[] =
	  {
	    {"pad", 1, 0, 1},
	    {"gains", 1, 0, 5},
	    {"log", 0, 0, 6},
	    {"code8", 0, 0, 2},
	    {"seed", 1, 0, 3},
	    {"subtractpeds", 0, 0, 4},
	    {0, 0, 0, 0}
	  };
	  
	  int c=getopt_long_only(argc, argv, "gc", long_options, 
				 &option_index);
	  
	  if(c==-1)break;
	  
	  switch(c)
	    {
	    case 1:
	      usepadding=true;
	      pad_name=optarg;
	      break;
	      
	    case 2:
	      selectCode8s=true;
	      break;
	    
	    case 3:
	      istringstream(optarg) >> seed;
	      break;
	    
	    case 4:
	      subtractPeds=true;
	      break;

	    case 5:
	      usegains=true;
	      gains_name=optarg;
	      break;
	    
	    case 6:
	      useloginserter=true;
	      break;
	    
	    case '?':
	    default:
	      exit(EXIT_FAILURE);
	    }
	}
      
      char* progname=*argv;
      argc-=optind;
      argv+=optind;

      if(argc==0)
	{
	  cerr << "Must supply filename" << endl;
	  exit(EXIT_FAILURE);
	}

      string ifilename = *argv;
      argv++,argc--;

      ///////////////////////////////////////////////////////////////////////

      RedFile rf;
      
      cout << "---------------------------------- Input file " 
	   << "----------------------------------" << endl;
      
      string on_name=getFilenameBase(ifilename);
      
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

      /////////////////////////////////////////////////////////////////////////

      cout << endl;
      cout << "------------------------------------ Camera " 
	   << "------------------------------------" << endl;

      CameraFinder camfinder;
      auto_ptr<CameraConfiguration> cam(camfinder.findForRedFile(&rf));

      cout << "Using camera:     " << cam->description() << endl;
      cout << "Channels:         " << cam->nchannels() << endl;

      /////////////////////////////////////////////////////////////////////////

      cout << "----------------------------------- Selector " 
	   << "-----------------------------------" << endl;
      
      MPtr<RedEventSelector> selector;
      if(selectCode8s)
	{
	  selector.manage(new Code8RESelector());
	  cout << "Event selector: All Code8 Events" << endl;
	}
      else
	{
	  const int pcutoff=75;
	  selector.manage(new StdPedRESelector(cam.get(),pcutoff));
	  cout << "Event selector: Pedestal Events" << endl;
	  cout << "CR Threshold:   " << pcutoff << " DC" << endl;
	}
      
      /////////////////////////////////////////////////////////////////////////

      ODBCPedsAndGainsFactory pgfact("DSN=AnalysisDB");
      cout << endl;
      cout << "----------------------------------- Pedestals " 
	   << "----------------------------------" << endl;
      
      string onfile_tag = on_name;
  
      int on_date=rh.date();
      cout << "Searching for:    " << onfile_tag << " on date " 
	   << on_date << endl;
  
      auto_ptr<Pedestals> peds(pgfact.loadPeds(onfile_tag,on_date));
      if(peds.get() == 0)
	{
	  cout << endl
	       << "Pedestals could not be found!" << endl;
	  exit(EXIT_FAILURE);
	}
      
      cout << "Nevents:          " << peds->nevents() << endl;
      cout << "Camera:           " << peds->camera() << endl;
      cout << "Comment:          " << peds->comment() << endl;

      // Set the ped value to 0 -- we only need the ped deviation
      if(subtractPeds == false)
	for(int c=0;c<cam->nchannels();c++)peds->val(c)=0.0;
      
      /////////////////////////////////////////////////////////////////////////

      MPtr<Gains> gains;

      if(usegains == true)
	{
	  cout << endl;
	  cout << "------------------------------------- Gains " 
	       << "------------------------------------" << endl;
	  
	  int n2_date=rh.date();
	  cout << "Searching for:    " << gains_name 
	       << " on date " << n2_date << endl;

	  gains.manage(pgfact.loadGains(gains_name,n2_date));
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
      else
	{
	  gains.manage(new Gains(cam->nchannels()));
	  for(int c=0;c<cam->nchannels();c++)gains->val(c)=1.0;
	}

      BogusCleaner cleaner;

      auto_ptr<ECRGenerator> ecrgen;
      if(usepadding)
	{
	  cout << endl;
	  cout << "---------------------------- Padding File Pedestals " 
	       << "----------------------------" << endl;
	  
	  
	  int pad_date=rh.date();
	  cout << "Searching for:    " << pad_name << " on date " 
	       << pad_date << endl;
	  
	  auto_ptr<Pedestals> 
	    padpeds(pgfact.loadPeds(pad_name,pad_date));
	  
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
	  if(seed==0)seed=time(0);
	  cout << "RNG Seed:         " << seed << endl;
	  
	  MPtr<LinearRNG> 
	    lrng(new NRRand2(seed), true);
	  MPtr<GaussianRNG> 
	    rng(new BMLinearRNGToGaussianRNGAdaptor(lrng),true);
	  seed=0;
	  
	  ecrgen.reset(new Padding_ECRGenerator(cam.get(),&cleaner,
						rng,gains.get(),
						peds.get(),padpeds.get()));
	}
      else
	{
	  ecrgen.reset(new Standard_ECRGenerator(cam.get(),&cleaner,
						 gains.get(),peds.get()));
	}
  
      ecrgen->dump(cout);

      ADCSpectrum spectrum(cam->nchannels());

      TextProgressBar pb;

      MPtr<ADCSpectrumEventInserter> inserter;
      if(useloginserter)
	{
	  inserter.manage(new LogSignalADCSpectrumInserter(ecrgen.get()));
	}
      else
	{
	  inserter.manage(new SignalADCSpectrumInserter(ecrgen.get()));
	}
      
      ADCSpectrumBuilder builder(inserter.get(), selector.get(), &pb);
      builder.insertFile(&spectrum, &rf);

      /////////////////////////////////////////////////////////////////////////

      string ofilename=getFilenameRoot(ifilename)+".spectrum";

      cout << endl;
      cout << "---------------------------------- Output File " 
	   << "---------------------------------" << endl;
      cout << "File:                 " << ofilename << endl;

      ofstream out(ofilename.c_str());

      double min = spectrum.minVal();
      double max = spectrum.maxVal();

      for(double v=min;v<=max;v+=1.0)
	{
	  out << v;
	  for(int c=0; c<cam->nchannels(); c++)
	    {
	      out << '\t' << spectrum.spectrum(c).valCount(v);
	    }
	  out << '\n';
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
