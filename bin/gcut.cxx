#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <fstream>

#include <Types.h>
#include <Exceptions.h>

#include <ParamFile.h>
#include <CutsHillasParam.h>

#include <Binner.h>
#include <AlphaPlotGenerator.h>

#include <ProgressBar.h>
#include <ScriptParser.h>

#include <UtilityFunctions.h>

#include <unistd.h>
#include <getopt.h>

#include <hdf5/hdf5.h>

using NS_Analysis::MPtr;
using NS_Analysis::ParamFile;
using NS_Analysis::CutsHillasParam;
using NS_Analysis::Summer;
using NS_Analysis::AlphaPlotGenerator;
using NS_Analysis::TextProgressBar;
using NS_Analysis::QLScriptParser;
using NS_Analysis::ScriptItemHandler;
using NS_Analysis::Error;

using NS_Analysis::rate;
using NS_Analysis::significance;

using namespace std;

class GCutEngine: public ScriptItemHandler
{
public:
  virtual void trk(const string& tr_name, int tr_date, 
		   const string& n2_name, int n2_date);
  virtual void pair(const string& on_name, int on_date,
		    const string& n2_name, int n2_date,
		    const string& pad_name, int pad_date);
  virtual ~GCutEngine();

  GCutEngine(CutsHillasParam* cuts, 
	     double onla, double onua, double offla, double offua, double tr,
	     const string& ext=".ph5"):
    m_tracking_ratio(tr),
    m_on_res(cuts, onla, onua, offla, offua, 5.0, &m_pb),
    m_off_res(cuts, onla, onua, offla, offua, 5.0, &m_pb),
    m_trk_res(cuts, onla, onua, offla, offua, 5.0, &m_pb),
    m_filename_extension(ext)
  {
  }

  const AlphaPlotGenerator& onRes() { return m_on_res; }
  const AlphaPlotGenerator& offRes() { return m_off_res; }
  const AlphaPlotGenerator& trkRes() { return m_trk_res; }

private:
  TextProgressBar    m_pb;

  double             m_tracking_ratio;

  AlphaPlotGenerator m_on_res;
  AlphaPlotGenerator m_off_res;
  AlphaPlotGenerator m_trk_res;

  string             m_filename_extension;

  void processFile(const string& prefix, 
		   const string& filename, AlphaPlotGenerator& al);

  void printFileResults(ostream& str, const string& prefix, 
			const string& filename, 
			const AlphaPlotGenerator::Results& r);

  void printExcess(ostream& str, const string& prefix, const string& filename,
		   const AlphaPlotGenerator::Results& on, 
		   const AlphaPlotGenerator::Results& off);
};

void GCutEngine::trk(const string& tr_name, int tr_date, 
		     const string& n2_name, int n2_date)
{
  processFile("TRK",tr_name, m_trk_res);
}

void GCutEngine::pair(const string& on_name, int on_date,
		      const string& n2_name, int n2_date,
		      const string& pad_name, int pad_date)
{
  processFile("ON ",on_name, m_on_res);
  processFile("OFF",pad_name, m_off_res);
  printExcess(cout,"PR",on_name,
	      onRes().fileResults(),offRes().fileResults());
}

GCutEngine::~GCutEngine()
{

  if(trkRes().totalResults().nFiles())
    {
      cout << endl;
      printFileResults(cout,"TRK","total",trkRes().totalResults());
    }

  if(onRes().totalResults().nFiles())
    {
      cout << endl;
      printFileResults(cout,"ON","total",onRes().totalResults());
      printFileResults(cout,"OFF","total",offRes().totalResults());
      printExcess(cout,"PR","excess",
		  onRes().totalResults(),offRes().totalResults());
    }
}

void GCutEngine::printFileResults(ostream& str, const string& prefix, 
				  const string& filename, 
				  const AlphaPlotGenerator::Results& r)
{
  double nOn   = r.nPassedOn();
  double nOff  = r.nPassedOff();
  double ltime = r.liveTime();
  
  double trRate=rate(nOn,nOff,ltime,ltime,m_tracking_ratio);
  double trSigma=significance(nOn,nOff,ltime,ltime,m_tracking_ratio);
  
  str << setiosflags(ios::left) << setw(3) << prefix.c_str() << ' ' 
      << setw(8) << filename.c_str() << resetiosflags(ios::left) << ' ' 
      << setiosflags(ios::fixed)
      << setw(6) << setprecision(2) << trSigma << ' ' 
      << setw(6) << setprecision(2) << trRate << ' ' 
      << setw(6) << setprecision(2) << trRate/trSigma << ' '
      << resetiosflags(ios::floatfield)
      << r << endl;
}

void GCutEngine::printExcess(ostream& str, 
			     const string& prefix, const string& filename,
			     const AlphaPlotGenerator::Results& on, 
			     const AlphaPlotGenerator::Results& off)
{
  const int    onNFiles  = on.nFiles();
  const double onNOn     = on.nPassedOn();
  const double onNOff    = on.nPassedOff();
  const double onNRaw    = on.nPassedRaw();
  const double onNImg    = on.nPassedImage3();
  const double onNSha    = on.nPassedShape();
  const double onNOrt    = on.nPassedOrient();
  const double onLTime   = on.liveTime();
	  	  
  const int    offNFiles = off.nFiles();
  const double offNOn    = off.nPassedOn();
  const double offNOff   = off.nPassedOff();
  const double offNRaw   = off.nPassedRaw();
  const double offNImg   = off.nPassedImage3();
  const double offNSha   = off.nPassedShape();
  const double offNOrt   = off.nPassedOrient();
  const double offLTime  = off.liveTime();
  
  const double excessOn  = significance(onNOn,offNOn,onLTime,offLTime);
  const double excessOff = significance(onNOff,offNOff,onLTime,offLTime);
  const double excessRaw = significance(onNRaw,offNRaw,onLTime,offLTime);
  const double excessImg = significance(onNImg,offNImg,onLTime,offLTime);
  const double excessSha = significance(onNSha,offNSha,onLTime,offLTime);
  const double excessOrt = significance(onNOrt,offNOrt,onLTime,offLTime);
  
  const double pairRate  = rate(onNOn,offNOn,onLTime,offLTime);

  str << resetiosflags(ios::floatfield)
      << setiosflags(ios::fixed)
      << setiosflags(ios::left) << setw(3) << prefix.c_str() << ' ' 
      << setw(8) << filename.c_str() << resetiosflags(ios::left) << ' ' 
      << setw(6) << setprecision(2) << excessOn << ' ' 
      << setw(6) << setprecision(2) << pairRate << ' ' 
      << setw(6) << setprecision(2) << pairRate/excessOn << ' '
      << setw(3) << setprecision(3) << (offNFiles-onNFiles) << ' '
      << setw(9) << setprecision(1) << (onLTime-offLTime) << ' '
      << setw(6) << setprecision(2) << excessOn << ' ' 
      << setw(6) << setprecision(2) << excessOff << ' '
      << setw(8) << setprecision(2) << excessRaw << ' ' 
      << setw(8) << setprecision(2) << excessImg << ' ' 
      << setw(7) << setprecision(2) << excessSha << ' ' 
      << setw(7) << setprecision(2) << excessOrt;
  
  int onZeroBin=on.alphaPlot().valToBin(0);
  int offZeroBin=off.alphaPlot().valToBin(0);
  double cOn=0;
  double cOff=0;
  for(int i=0;i<18;i++)
    {
      cOn  += on.alphaPlot().binCount(onZeroBin+i);
      cOff += off.alphaPlot().binCount(offZeroBin+i);
      
      str << ' ' << setw(5) << setprecision(1) 
	  << significance(cOn,cOff,onLTime,offLTime);
    }
	  
  str << resetiosflags(ios::floatfield) << endl;
}

void GCutEngine::processFile(const string& prefix,
			     const string& filename, AlphaPlotGenerator& al)
{
  ParamFile pf(filename+m_filename_extension);
  al.cut(&pf);
  printFileResults(cout, prefix, filename, al.fileResults());
}

int main( int argc, char **argv )
{
  try
    {
      CutsHillasParam cuts;
      cuts.zero();
      cuts.setCutsFromCommandLine(argc,argv);
      
      double ONlalpha=0;
      double ONualpha=15;
      
      double OFFlalpha=20;
      double OFFualpha=65;
      
      double trackingratio=3.10;
      
      string scriptname;
      string extension=".ph5";

      while (1)
	{
	  int this_option_optind = optind ? optind : 1;
	  int option_index = 0;
	  
	  static struct option long_options[] =
	    {
	      {"alpha", 1, 0, 100},
	      {"ualpha", 1, 0, 100},
	      {"onualpha", 1, 0, 100},
	      {"lalpha", 1, 0, 101},
	      {"onlalpha", 1, 0, 101},
	      
	      {"offualpha", 1, 0, 102},
	      {"offlalpha", 1, 0, 103},
	      
	      {"trackingratio", 1, 0, 104},
	      {"offonratio", 1, 0, 104},
	      
	      {"script", 1, 0, 's'},
	      {"extension", 1, 0, 'E'},

	      {0, 0, 0, 0}
	    };
	  
	  int c=getopt_long_only(argc, argv, "sE", long_options, 
				 &option_index);
	  
	  if(c==-1)break;
	  
	  switch(c)
	    {
	    case 100:
	      istringstream(optarg) >> ONualpha;
	      break;
	      
	    case 101:
	      istringstream(optarg) >> ONlalpha;
	      break;
	      
	    case 102:
	      istringstream(optarg) >> OFFualpha;
	      break;
	      
	    case 103:
	      istringstream(optarg) >> OFFlalpha;
	      break;
	      
	    case 104:
	      istringstream(optarg) >> trackingratio;
	      break;
	      
	    case 's':
	      scriptname=optarg;
	      break;

	    case 'E':
	      extension=optarg;
	      break;

	    case '?':
	    default:
	      exit(EXIT_FAILURE);
	    }
	}
      
      cuts.streamDumpCuts(cerr);
      
      cerr << "\nON region defined by:  "
	   << ONlalpha << " < alpha < " << ONualpha << '\n';
      cerr << "OFF region defined by:  "
	   << OFFlalpha << " < alpha < " << OFFualpha << '\n';
      cerr << "Tracking ratio: " << trackingratio << '\n';
      
      argc-=optind;
      argv+=optind;
      
      GCutEngine gce(&cuts, 
		     ONlalpha, ONualpha, OFFlalpha, OFFualpha, trackingratio,
		     extension);

      if(scriptname != "")
	{
	  QLScriptParser parser(&gce);
	  parser.parse(scriptname);
	}
      else
	{
	  while(argc)
	    {
	      gce.trk(*argv,-1,"",-1);
	      argv++, argc--;
	    }      
	}
    }
  catch(const Error& x)
    {
      cerr << x;
    }
}
