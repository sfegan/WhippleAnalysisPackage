#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <fstream>

#include <Types.h>
#include <Exceptions.h>

#include <HillasParam.h>
#include <EventInfo.h>
#include <ParamFile.h>

#include <ProgressBar.h>
#include <ScriptParser.h>

#include <UtilityFunctions.h>

#include <unistd.h>
#include <getopt.h>

#include <hdf5/hdf5.h>

using namespace NS_Analysis;

using namespace std;

class DumpParamsEngine: public ScriptItemHandler, private ParamEventOperator
{
public:
  virtual void trk(const string& tr_name, int tr_date, 
		   const string& n2_name, int n2_date);
  virtual void pair(const string& on_name, int on_date,
		    const string& n2_name, int n2_date,
		    const string& pad_name, int pad_date);
  virtual ~DumpParamsEngine();
  
  DumpParamsEngine(const string& iext=".ph5",const string& oext=".pdat"):
    m_ifilename_extension(iext), m_ofilename_extension(oext)
  {
  }
  
  virtual void operateOnEvent(int evno, 
			      const EventInfo* ei, const HillasParam* hp);

private:
  TextProgressBar    m_pb;

  string             m_ifilename_extension;
  string             m_ofilename_extension;

  ostream*           m_stream;

  void processFile(const string& filename);
};

void DumpParamsEngine::trk(const string& tr_name, int tr_date, 
		     const string& n2_name, int n2_date)
{
  processFile(tr_name);
}

void DumpParamsEngine::pair(const string& on_name, int on_date,
		      const string& n2_name, int n2_date,
		      const string& pad_name, int pad_date)
{
  processFile(on_name);
  processFile(pad_name);
}

DumpParamsEngine::~DumpParamsEngine()
{

}

void
DumpParamsEngine::operateOnEvent(int evno, 
				 const EventInfo* ei, const HillasParam* hp)
{
  *m_stream << resetiosflags(ios::floatfield)
	    << setiosflags(ios::fixed)
	    << setiosflags(ios::left) << setw(6) << evno
	    << setw(1) << setprecision(1) << ei->getCode() << ' ' 
	    << setw(12) << setprecision(7) << ei->getTime() << ' ' 
	    << setw(12) << setprecision(7) << ei->getLiveTime() << ' ' 
	    << setw(5) << setprecision(3) << hp->getLength() << ' '
	    << setw(5) << setprecision(3) << hp->getWidth() << ' '
	    << setw(5) << setprecision(3) << hp->getDist() << ' '
	    << setw(3) << setprecision(3) << hp->getNImage() << ' '
	    << setw(7) << setprecision(1) << hp->getSize() << ' '
	    << setw(5) << setprecision(1) << hp->getAlpha() << endl;
}

void DumpParamsEngine::processFile(const string& filename)
{
  ParamFile pf(filename+m_ifilename_extension);
  ofstream of((filename+m_ofilename_extension).c_str());
  m_stream = &of;
  
  ParamFileEventProcessor pfproc(this,&m_pb);
  pfproc.run(&pf);
}

int main( int argc, char **argv )
{
  try
    {
      string scriptname;
      string iextension=".ph5";
      string oextension=".pdat";

      while (1)
	{
	  int this_option_optind = optind ? optind : 1;
	  int option_index = 0;
	  
	  static struct option long_options[] =
	    {
	      {"script"    , 1, 0, 's'},
	      {"oextension", 1, 0, 'e'},
	      {"iextension", 1, 0, 'E'},

	      {0, 0, 0, 0}
	    };
	  
	  int c=getopt_long_only(argc, argv, "s:e:E:", long_options, 
				 &option_index);
	  
	  if(c==-1)break;
	  
	  switch(c)
	    {
	    case 's':
	      scriptname=optarg;
	      break;

	    case 'e':
	      oextension=optarg;
	      break;

	    case 'E':
	      iextension=optarg;
	      break;

	    case '?':
	    default:
	      exit(EXIT_FAILURE);
	    }
	}
      
      argc-=optind;
      argv+=optind;
      
      DumpParamsEngine gce(iextension,oextension);

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
