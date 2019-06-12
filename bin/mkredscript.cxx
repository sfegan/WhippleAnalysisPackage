#include <math.h>
#include <unistd.h>
#include <getopt.h>

#include <iostream>
#include <sstream>
#include <iomanip>

#include <ScriptParser.h>
#include <Exceptions.h>

#include "f77redfile.h"

using NS_Analysis::Error;
using NS_Analysis::ScriptItemHandler;
using NS_Analysis::QLScriptParser;

using namespace std;

class RedScriptMaker: public ScriptItemHandler
{
public:
  virtual void n2(const string& n2_name, int n2_date);
  virtual void trk(const string& tr_name, int tr_date, 
		   const string& n2_name, int n2_date);
  virtual void pair(const string& on_name, int on_date,
		    const string& n2_name, int n2_date,
		    const string& pad_name, int pad_date);
  virtual ~RedScriptMaker();

  RedScriptMaker(): ScriptItemHandler() {}

private:
  void writeEntry(const string& name, int date);
};

RedScriptMaker::~RedScriptMaker()
{

}

void RedScriptMaker::n2(const string& n2_name, int n2_date)
{
  writeEntry(n2_name, n2_date);
}

void RedScriptMaker::trk(const string& tr_name, int tr_date, 
		       const string& n2_name, int n2_date)
{
  writeEntry(tr_name, tr_date);
}

void RedScriptMaker::pair(const string& on_name, int on_date,
			const string& n2_name, int n2_date,
			const string& pad_name, int pad_date)
{
  writeEntry(on_name, on_date);
  writeEntry(pad_name, pad_date);
}

void RedScriptMaker::writeEntry(const string& name, int date)
{
  ostringstream shortdatestream;
  shortdatestream << setfill('0') <<  setw(6) << setprecision(6) 
		  << date%1000000;
  string shortdate = shortdatestream.str();

  cout << "echo Now processing " << name << " from date " << date << endl;

  cout << "cp /data/raw10/d" << shortdate
       << "/" << name << ".fz.bz2 ." << endl;

  cout << "bunzip2 " << name << ".fz.bz2" << endl;
  
  cout << "fz2red " << name << ".fz " << shortdate << endl;

  cout << "rm -f " << name << ".fz" << endl;

  cout << "bzip2 " << name.substr(0,2) << name.substr(name.length()-4,4)
       << endl;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

main(int argc, char **argv)
{
  char *progname;
  string scriptname;

  try
    {
      progname=*argv;
      
      argc-=optind;
      argv+=optind;
      
      if(argc<1)
	{
	  fprintf(stderr,"Usage: %s script [script ....]\n",
	      progname);
	  exit(EXIT_FAILURE);
	}
      
      RedScriptMaker rsm;
      
      while(argc)
	{
	  QLScriptParser parser(&rsm);
	  parser.parse(*argv);
	  argc--;
	  argv++;
	}
    }
  catch(const Error& x)
    {
      cerr << x;
    }
}
