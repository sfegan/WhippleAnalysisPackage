#include"StringTokenizer.h"

using std::string;
using std::endl;

NS_Analysis::STState::
~STState()
{

}

///////////////////////////////////////////////////////////////////////////////

NS_Analysis::StringTokenizer::
StringTokenizer(): 
  m_state(InitialSTState::Instance()), m_escaped_state(0),
  m_line(), m_here(m_line.end()), m_token(0), m_tokens()
{
  
}

NS_Analysis::StringTokenizer::
~StringTokenizer()
{
  reset();
}

void
NS_Analysis::StringTokenizer::
reset()
{
  for(vector<string*>::iterator i=m_tokens.begin(); i!=m_tokens.end(); i++)
    delete *i;
  m_tokens.clear();
  m_token=0;
  m_state=InitialSTState::Instance();
  m_line.erase();
  m_here=m_line.end();
}

void
NS_Analysis::StringTokenizer::
tokenize(const string& str)
{
  reset();
  m_line=str;
  m_here=m_line.begin();
  while(m_state->operate(*this));
}

///////////////////////////////////////////////////////////////////////////////

NS_Analysis::InitialSTState* NS_Analysis::InitialSTState::m_instance;

NS_Analysis::InitialSTState::
~InitialSTState()
{
}

bool
NS_Analysis::InitialSTState::
operate(StringTokenizer& machine)
{
  setState(machine,FindNextTokenSTState::Instance());
  return true;
}

///////////////////////////////////////////////////////////////////////////////

NS_Analysis::FindNextTokenSTState* 
NS_Analysis::FindNextTokenSTState::m_instance;

NS_Analysis::FindNextTokenSTState::
~FindNextTokenSTState()
{
}

bool
NS_Analysis::FindNextTokenSTState::
operate(StringTokenizer& machine)
{
  if(!haveChar(machine))return false;
  char c=getChar(machine);

  switch(c)
    {
    case ' ':
    case '\t':
      nextChar(machine);
      break;

    case '#':
    case '!':
      setState(machine,SkipCommentSTState::Instance());
      break;

    default:
      nextToken(machine);
      setState(machine,ReadTokenSTState::Instance());
      break;
    }
  
  return true;
}

///////////////////////////////////////////////////////////////////////////////

NS_Analysis::ReadTokenSTState* NS_Analysis::ReadTokenSTState::m_instance;

NS_Analysis::ReadTokenSTState::
~ReadTokenSTState()
{
}

bool
NS_Analysis::ReadTokenSTState::
operate(StringTokenizer& machine)
{
  if(!haveChar(machine))return false;
  char c=getChar(machine);
  nextChar(machine);

  switch(c)
    {
    case ' ':
    case '\t':
      setState(machine,FindNextTokenSTState::Instance());
      break;
      
    case '#':
    case '!':
      setState(machine,SkipCommentSTState::Instance());
      break;

    case '"':
      setState(machine,DoubleQuoteSTState::Instance());
      break;

    case '\'':
      setState(machine,SingleQuoteSTState::Instance());
      break;

    case '\\':
      escapeState(machine);
      setState(machine,EscapedSTState::Instance());
      break;

    default:
      getToken(machine) += c;
      break;
    }
  
  return true;
}

///////////////////////////////////////////////////////////////////////////////

NS_Analysis::SkipCommentSTState* NS_Analysis::SkipCommentSTState::m_instance;

NS_Analysis::SkipCommentSTState::
~SkipCommentSTState()
{
}

bool
NS_Analysis::SkipCommentSTState::
operate(StringTokenizer& machine)
{
  if(!haveChar(machine))return false;
  nextChar(machine);
  return true;
}

///////////////////////////////////////////////////////////////////////////////

NS_Analysis::DoubleQuoteSTState* NS_Analysis::DoubleQuoteSTState::m_instance;

NS_Analysis::DoubleQuoteSTState::
~DoubleQuoteSTState()
{
}

bool
NS_Analysis::DoubleQuoteSTState::
operate(StringTokenizer& machine)
{
  if(!haveChar(machine))
    {
      Error err("DoubleQuoteSTState::operate");
      err.stream() << "End of line reached before closing \" was found" 
		   << endl;
      throw(err);
    }

  char c=getChar(machine);
  nextChar(machine);

  switch(c)
    {
    case '"':
      setState(machine,ReadTokenSTState::Instance());
      break;

    case '\\':
      escapeState(machine);
      setState(machine,EscapedSTState::Instance());
      break;
      
    default:
      getToken(machine) += c;
      break;
    }
  
  return true;
}

///////////////////////////////////////////////////////////////////////////////

NS_Analysis::SingleQuoteSTState* NS_Analysis::SingleQuoteSTState::m_instance;

NS_Analysis::SingleQuoteSTState::
~SingleQuoteSTState()
{
}

bool
NS_Analysis::SingleQuoteSTState::
operate(StringTokenizer& machine)
{
  if(!haveChar(machine))
    {
      Error err("SingleQuoteSTState::operate");
      err.stream() << "End of line reached before closing ' was found" 
		   << endl;
      throw(err);
    }

  char c=getChar(machine);
  nextChar(machine);

  switch(c)
    {
    case '\'':
      setState(machine,ReadTokenSTState::Instance());
      break;

    default:
      getToken(machine) += c;
      break;
    }
  
  return true;
}

///////////////////////////////////////////////////////////////////////////////

NS_Analysis::EscapedSTState* NS_Analysis::EscapedSTState::m_instance;

NS_Analysis::EscapedSTState::
~EscapedSTState()
{
}

bool
NS_Analysis::EscapedSTState::
operate(StringTokenizer& machine)
{
  if(!haveChar(machine))
    {
      Error err("EscapedSTState::operate");
      err.stream() << "End of line reached at \\ character" 
		   << endl;
      throw(err);
    }
  
  char c=getChar(machine);
  nextChar(machine);

  switch(c)
    {
    case '\'':
    case '\"':
    case '\\':
    case ' ':
    case '\t':
      getToken(machine) += c;
      break;

    case 'n':
      getToken(machine) += '\n';
      break;

    default:
      {
	Error err("EscapedSTState::operate");
	err.stream() << "Unknown escape sequence \\" << c << endl;
	throw err;
      }
      break;
    }
  
  restoreState(machine);
  return true;
}

///////////////////////////////////////////////////////////////////////////////
