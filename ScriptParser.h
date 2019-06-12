//-*-mode:c++; mode:font-lock;-*-

#ifndef SCRIPTPARSER_H
#define SCRIPTPARSER_H

#include<string>
#include<vector>

#include"Types.h"
#include"Exceptions.h"

namespace NS_Analysis
{
  using std::vector;

  class ScriptItemHandler
  {
  public:
    virtual void n2(const string& n2_name, int n2_date);
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
    virtual ~ScriptItemHandler();
  };

  class ScriptParser
  {
  public:
    virtual ~ScriptParser();
    virtual void parse(const string& filename) = 0;
  };


  class QLScriptParser
  {
  public:
    QLScriptParser(MPtr<ScriptItemHandler> h): m_handler(h) {}
    QLScriptParser(ScriptItemHandler* h): m_handler(h,false) {}

    virtual ~QLScriptParser();
    virtual void parse(const string& filename);

  private:
    MPtr<ScriptItemHandler> m_handler;
  };
  

} // namespace Analysis

#endif // defined SCRIPTPARSER_H
