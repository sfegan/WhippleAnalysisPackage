#include<iostream>
#include<fstream>
#include<sstream>
#include<string>

#include"ScriptParser.h"
#include"StringTokenizer.h"
#include"UtilityFunctions.h"

using std::istringstream;
using std::vector;
using std::string;
using std::endl;

NS_Analysis::ScriptItemHandler::
~ScriptItemHandler()
{

}

void
NS_Analysis::ScriptItemHandler::
n2(const string& n2_name, int n2_date)
{
  // DEFAULT: Do nothing!
}

void 
NS_Analysis::ScriptItemHandler::
trk(const string& tr_name, int tr_date, 
    const string& n2_name, int n2_date)
{
  // DEFAULT: Do nothing!
}

void 
NS_Analysis::ScriptItemHandler::
pair(const string& on_name, int on_date,
     const string& n2_name, int n2_date,
     const string& pad_name, int pad_date)
{
  // DEFAULT: Do nothing!
}

void 
NS_Analysis::ScriptItemHandler::
matched_pair(const string& on_name, int on_date, 
	     const string& on_n2_name, int on_n2_date,
	     const string& pad_name, int pad_date,
	     const string& pad_n2_name, int pad_n2_date)
{
  // DEFAULT: Do nothing!
}

void 
NS_Analysis::ScriptItemHandler::
option(const string& name, const vector<string>& values)
{
  // DEFAULT: Do nothing!
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

NS_Analysis::ScriptParser::
~ScriptParser()
{

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

NS_Analysis::QLScriptParser::
~QLScriptParser()
{

}

void 
NS_Analysis::QLScriptParser::
parse(const string& filename)
{
  StringTokenizer tokenizer;

  std::ifstream ist(filename.c_str());
  if(!ist)
    {
      FileError err("QLScriptParser::parse");
      err.stream() << "Could not open file " << filename << endl;
      throw err;
    }

  int lineno;
  string line;
  while(getline(ist,line))
    {
      lineno++;

      tokenizer.tokenize(line);
      if(tokenizer.nTokens() == 0)continue;

      const string& command=tokenizer.token(0);

      if(command == "option")
	{
	  if(tokenizer.nTokens() < 2)
	    {
	      Error err("QLScriptParser::parse");
	      err.stream() << "Error in script " << filename 
			   << " on line " << lineno << endl
			   << "Line: " << line << endl ;
	      throw(err);
	    }

	  vector<string> data;
	  for(int i=2; i<tokenizer.nTokens(); i++)
	    data.push_back(tokenizer.token(i));
	  m_handler->option(tokenizer.token(1),data);
	}
      
      else if(command == "n2")
	{
	  if(tokenizer.nTokens() != 3)
	    {
	      Error err("QLScriptParser::parse");
	      err.stream() << "Error in script " << filename 
			   << " on line " << lineno << endl
			   << "Line: " << line << endl ;
	      throw(err);
	    }

	  int date = -1;
	  istringstream(tokenizer.token(2)) >> date;
	  fixDate2000(date);

	  m_handler->n2(tokenizer.token(1),date);
	}
      
      else if(command == "tr")
	{
	  if(tokenizer.nTokens() != 4)
	    {
	      Error err("QLScriptParser::parse");
	      err.stream() << "Error in script " << filename 
			   << " on line " << lineno << endl
			   << "Line: " << line << endl ;
	      throw(err);
	    }

	  int date = -1;
	  istringstream(tokenizer.token(3)) >> date;
	  fixDate2000(date);

	  m_handler->trk(tokenizer.token(1),date,
			 tokenizer.token(2),date);
	}

      else if(command == "pr")
	{
	  if(tokenizer.nTokens() != 5)
	    {
	      Error err("QLScriptParser::parse");
	      err.stream() << "Error in script " << filename 
			   << " on line " << lineno << endl
			   << "Line: " << line << endl ;
	      throw(err);
	    }

	  int date = -1;
	  istringstream(tokenizer.token(4)) >> date;
	  fixDate2000(date);

	  m_handler->pair(tokenizer.token(1),date,
			  tokenizer.token(3),date,
			  tokenizer.token(2),date);
	}
      else if(command == "mp")
	{
	  if(tokenizer.nTokens() != 7)
	    {
	      Error err("QLScriptParser::parse");
	      err.stream() << "Error in script " << filename 
			   << " on line " << lineno << endl
			   << "Line: " << line << endl ;
	      throw(err);
	    }

	  int on_date = -1;
	  istringstream(tokenizer.token(5)) >> on_date;
	  fixDate2000(on_date);

	  int off_date = -1;
	  istringstream(tokenizer.token(6)) >> off_date;
	  fixDate2000(off_date);

	  m_handler->matched_pair(tokenizer.token(1),on_date,
				  tokenizer.token(3),on_date,
				  tokenizer.token(2),off_date,
				  tokenizer.token(4),off_date);
	}      
      else
	{
	  Error err("QLScriptParser::parse");
	  err.stream() << "Unknown directive " << command 
		       << " in script " << filename 
		       << " on line " << lineno << endl
		       << "Line: " << line << endl ;
	  throw(err);
	}
    }
}
