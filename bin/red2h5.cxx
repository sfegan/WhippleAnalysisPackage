#include <math.h>
#include <unistd.h>
#include <getopt.h>

#include <iostream>
#include <sstream>

#include <RedFile.h>
#include <RedEvent.h>
#include <RedHeader.h>
#include <ProgressBar.h>
#include <ScriptParser.h>
#include <UtilityFunctions.h>
#include <Exceptions.h>

#include "f77redfile.h"

using NS_Analysis::Error;
using NS_Analysis::RedFile;
using NS_Analysis::RedEvent;
using NS_Analysis::RedHeader;
using NS_Analysis::ProgressBar;
using NS_Analysis::TextProgressBar;
using NS_Analysis::ScriptItemHandler;
using NS_Analysis::QLScriptParser;
using NS_Analysis::getFilenameName;
using NS_Analysis::getFilenamePath;

using namespace std;

class Red2H5Engine: public ScriptItemHandler
{
public:
  virtual void n2(const string& n2_name, int n2_date);
  virtual void trk(const string& tr_name, int tr_date, 
		   const string& n2_name, int n2_date);
  virtual void pair(const string& on_name, int on_date,
		    const string& n2_name, int n2_date,
		    const string& pad_name, int pad_date);
  virtual ~Red2H5Engine();

  Red2H5Engine(int nchan): 
    ScriptItemHandler(), m_nchannels(nchan) {}
  
private:
  TextProgressBar m_pb;
  int             m_nchannels;

  string mungedFilename(const string &filename);
  void convert(const string& filename);
  void convert(const string& ifilename, const string& ofilename);

};

Red2H5Engine::~Red2H5Engine()
{
}

void Red2H5Engine::n2(const string& n2_name, int n2_date)
{
  convert(n2_name);
}

void Red2H5Engine::trk(const string& tr_name, int tr_date, 
		       const string& n2_name, int n2_date)
{
  convert(tr_name);
}

void Red2H5Engine::pair(const string& on_name, int on_date,
			const string& n2_name, int n2_date,
			const string& pad_name, int pad_date)
{
  convert(on_name);
  convert(pad_name);
}

string Red2H5Engine::mungedFilename(const string& filename)
{
  string filenamename=getFilenameName(filename);
  if((filenamename.length() == 8) && (filenamename.substr(0,2) == "gt"))
    return getFilenamePath(filename)+"gt"+filenamename.substr(4,4);
  else return filename;
}

void Red2H5Engine::convert(const string& filename)
{
  string ifilename=mungedFilename(filename);
  string ofilename=filename+".h5";
  convert(ifilename,ofilename);
}

void Red2H5Engine::convert(const string& ifilename, const string& ofilename)
{
  int nevents;
  
  F77RedFile f77file(ifilename, m_nchannels);
  H5garbage_collect();

  RedFile redfile(ofilename,m_nchannels);
  
  RedHeader rh;
  rh.setVersion(m_nchannels);
  f77file.getHeader(&rh);
  redfile.header()->write(0,&rh,1);
  
  cout << "File: " << ifilename 
       << ", nevents=" << rh.nevents()
       << ", live time=" << rh.live_time()
       << ", stdur=" << rh.stdur()
       << ", mjd=" << rh.mjd() << endl;
  
  
  RedEvent re;
  re.setVersion(m_nchannels);
  
  nevents=rh.nevents();
  m_pb.reset(nevents);
  for(int i=0;i<nevents;i++)
    {
      f77file.getNextEvent(&re);
      redfile.events()->appendOne(&re);
      m_pb.tick(i);
    }
  m_pb.tick(nevents);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

main(int argc, char **argv)
{
  char *progname;

  int nChannels=492;
  string scriptname;

  try
    {
      while (1)
	{
	  int this_option_optind = optind ? optind : 1;
	  int option_index = 0;
	  
	  static struct option long_options[] =
	    {
	      {"channel", 1, 0, 'c'},
	      {"channels", 1, 0, 'c'},
	      {"script", 1, 0, 's'},
	      {0, 0, 0, 0}
	    };
	  
	  int c=getopt_long_only(argc, argv, "c", long_options, &option_index);
	  
	  if(c==-1)break;
	  
	  switch(c)
	    {
	    case 'c':
	      istringstream(optarg) >> nChannels;
	      break;
	      
	    case 's':
	      scriptname=optarg;
	      break;
	    }
	}
      
      progname=*argv;
      
      argc-=optind;
      argv+=optind;
      
      if((argc<1) && (scriptname==""))
	{
	  fprintf(stderr,"Usage: %s [-s script] reducedfilename ....\n",
	      progname);
	  exit(EXIT_FAILURE);
	}
      
      cout << "Using " << nChannels << " channels\n";
      
      Red2H5Engine r2h5(nChannels);
      
      if(scriptname != "")
	{
	  QLScriptParser parser(&r2h5);
	  parser.parse(scriptname);
	}
      else
	{
	  while(*argv)
	    {
	      r2h5.trk(*argv,-1,"",-1);
	      argv++, argc--;
	    }
	}
    }
  catch(const Error& x)
    {
      cerr << x;
    }
}
