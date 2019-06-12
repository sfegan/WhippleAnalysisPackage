#include <cmath>
#include <iomanip>
#include <iostream>
#include <fstream>

#include <unistd.h>
#include <getopt.h>
#include <hdf5/hdf5.h>

#include <Types.h>
#include <Exceptions.h>

#include <ParamFile.h>
#include <CutsHillasParam.h>

#include <CutParamFile.h>

#include <UtilityFunctions.h>
#include <ProgressBar.h>

#include <ScriptParser.h>

using NS_Analysis::MPtr;
using namespace NS_Analysis;
using NS_Analysis::ParamFile;
using NS_Analysis::CutsHillasParam;
using NS_Analysis::CutParamFileGenerator;
using NS_Analysis::TextProgressBar;
using NS_Analysis::getFilenameRoot;
using NS_Analysis::QLScriptParser;
using NS_Analysis::ScriptItemHandler;

using namespace std;

class GDistillEngine: public ScriptItemHandler
{
public:
  virtual void trk(const string& tr_name, int tr_date, 
		   const string& n2_name, int n2_date);
  virtual void pair(const string& on_name, int on_date,
		    const string& n2_name, int n2_date,
		    const string& pad_name, int pad_date);
  virtual ~GDistillEngine();

  GDistillEngine(CutsHillasParam* cuts):
    m_pb(), m_gen(cuts,&m_pb), m_cuts(cuts) {}

private:
  TextProgressBar         m_pb;
  CutParamFileGenerator   m_gen;
  CutsHillasParam*        m_cuts;

  void processFile(const string& filename);
};

GDistillEngine::~GDistillEngine()
{
}

void GDistillEngine::trk(const string& tr_name, int tr_date, 
			 const string& n2_name, int n2_date)
{
  processFile(tr_name);
}

void GDistillEngine::pair(const string& on_name, int on_date,
			  const string& n2_name, int n2_date,
			  const string& pad_name, int pad_date)
{
  processFile(on_name);
  processFile(pad_name);
}

void GDistillEngine::processFile(const string& filename)
{
  string ifilename=filename+".ph5";
  ParamFile ipf(filename+".ph5");

  string ofilename=filename+"_cut.ph5";
  ParamFile opf(ofilename,ipf.header()->tVersion());

  cerr << "Distilling " << ifilename << " --> " << ofilename << endl;
  
  m_gen.cut(&ipf,&opf);
}

int main( int argc, char **argv )
{
  try
    {
      CutsHillasParam cuts;
      cuts.zero();
      cuts.setCutsFromCommandLine(argc,argv);
      cuts.streamDumpCuts(cerr);

      GDistillEngine gde(&cuts);
      
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
	  
	  int c=getopt_long_only(argc, argv, "htl", long_options, 
				 &option_index);
	  
	  if(c==-1)break;
	  
	  switch(c)
	    {
	    case 's':
	      scriptname=optarg;
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
	  QLScriptParser parser(&gde);
	  parser.parse(scriptname);
	}
      else
	{ 
	  while(argc)
	    {
	      gde.trk(*argv,-1,"",-1);
	      argv++, argc--;
	    }     
	} 
    }
  catch (const Error& x)
    {
      cerr << x;
    }
}
