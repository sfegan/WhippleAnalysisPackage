//-*-mode:c++; mode:font-lock;-*-

#ifndef STRINGTOKENIZER_H
#define STRINGTOKENIZER_H

#include<vector>
#include<string>

#include"Exceptions.h"

namespace NS_Analysis
{
  using std::vector;
  using std::string;

  class StringTokenizer;

  class STState
  {
  public:
    virtual bool operate(StringTokenizer& machine) = 0;
    virtual ~STState();

  protected:
    bool haveChar(class StringTokenizer& machine);
    char getChar(class StringTokenizer& machine);
    bool nextChar(class StringTokenizer& machine);
    void nextToken(class StringTokenizer& machine);
    string& getToken(class StringTokenizer& machine);
    void setState(class StringTokenizer& machine, STState* state);
    void escapeState(class StringTokenizer& machine);
    void restoreState(class StringTokenizer& machine);
  };

  /////////////////////////////////////////////////////////////////////////////

  class StringTokenizer
  {
  public:
    typedef vector<string*>::const_iterator iterator;

    StringTokenizer();
    ~StringTokenizer();

    void tokenize(const string& str);

    void reset();
    
    int nTokens() const { return m_tokens.size(); }
    const string& token(int i) const { return *m_tokens[i]; }

    iterator begin() const { return m_tokens.begin(); }
    iterator end() const { return m_tokens.end(); }

  protected:
    bool haveChar() { return(m_here != m_line.end()); }
    char getChar() const { return *m_here; }
    bool nextChar() { if(haveChar())m_here++; return haveChar(); }
    
    void nextToken() { m_token=new string; m_tokens.push_back(m_token); }
    string& getToken() { return *m_token; }
    
    void setState(STState* state) { m_state=state; }
    void escapeState() { m_escaped_state=m_state; setState(0); }
    void restoreState() { setState(m_escaped_state); m_escaped_state=0; }
    
    friend class STState;

  private:
    STState* m_state;
    STState* m_escaped_state;

    string m_line;
    string::iterator m_here;

    string* m_token;
    vector<string*> m_tokens;
  };

  /////////////////////////////////////////////////////////////////////////////
  
  inline bool STState::haveChar(class StringTokenizer& machine)
  {
    return machine.haveChar();
  }

  inline char STState::getChar(class StringTokenizer& machine)
  {
    return machine.getChar();
  }

  inline bool STState::nextChar(class StringTokenizer& machine)
  {
    return machine.nextChar();
  }

  inline void STState::nextToken(class StringTokenizer& machine)
  {
    machine.nextToken();
  }

  inline string& STState::getToken(class StringTokenizer& machine)
  {
    return machine.getToken();
  }

  inline void STState::setState(class StringTokenizer& machine, 
				STState* state)
  {
    machine.setState(state);
  }

  inline void STState::escapeState(class StringTokenizer& machine)
  {
    machine.escapeState();
  }

  inline void STState::restoreState(class StringTokenizer& machine)
  {
    machine.restoreState();
  }

  /////////////////////////////////////////////////////////////////////////////

  class InitialSTState: public STState
  {
  public:
    virtual bool operate(StringTokenizer& machine);
    virtual ~InitialSTState();
    static InitialSTState* Instance();
  protected:
    InitialSTState() {}
    static InitialSTState* m_instance;
  };

  inline InitialSTState* InitialSTState::Instance()
  {
    if(m_instance == 0)m_instance=new InitialSTState();
    return m_instance;
  }

  /////////////////////////////////////////////////////////////////////////////

  class FindNextTokenSTState: public STState
  {
  public:
    virtual bool operate(StringTokenizer& machine);
    virtual ~FindNextTokenSTState();
    static FindNextTokenSTState* Instance();
  protected:
    FindNextTokenSTState() {}
    static FindNextTokenSTState* m_instance;
  };

  inline FindNextTokenSTState* FindNextTokenSTState::Instance()
  {
    if(m_instance == 0)m_instance=new FindNextTokenSTState();
    return m_instance;
  }

  /////////////////////////////////////////////////////////////////////////////

  class ReadTokenSTState: public STState
  {
  public:
    virtual bool operate(StringTokenizer& machine);
    virtual ~ReadTokenSTState();
    static ReadTokenSTState* Instance();
  protected:
    ReadTokenSTState() {}
    static ReadTokenSTState* m_instance;
  };

  inline ReadTokenSTState* ReadTokenSTState::Instance()
  {
    if(m_instance == 0)m_instance=new ReadTokenSTState();
    return m_instance;
  }

  /////////////////////////////////////////////////////////////////////////////

  class SkipCommentSTState: public STState
  {
  public:
    virtual bool operate(StringTokenizer& machine);
    virtual ~SkipCommentSTState();
    static SkipCommentSTState* Instance();
  protected:
    SkipCommentSTState() {}
    static SkipCommentSTState* m_instance;
  };

  inline SkipCommentSTState* SkipCommentSTState::Instance()
  {
    if(m_instance == 0)m_instance=new SkipCommentSTState();
    return m_instance;
  }

  /////////////////////////////////////////////////////////////////////////////

  class DoubleQuoteSTState: public STState
  {
  public:
    virtual bool operate(StringTokenizer& machine);
    virtual ~DoubleQuoteSTState();
    static DoubleQuoteSTState* Instance();
  protected:
    DoubleQuoteSTState() {}
    static DoubleQuoteSTState* m_instance;
  };

  inline DoubleQuoteSTState* DoubleQuoteSTState::Instance()
  {
    if(m_instance == 0)m_instance=new DoubleQuoteSTState();
    return m_instance;
  }

  /////////////////////////////////////////////////////////////////////////////

  class SingleQuoteSTState: public STState
  {
  public:
    virtual bool operate(StringTokenizer& machine);
    virtual ~SingleQuoteSTState();
    static SingleQuoteSTState* Instance();
  protected:
    SingleQuoteSTState() {}
    static SingleQuoteSTState* m_instance;
  };

  inline SingleQuoteSTState* SingleQuoteSTState::Instance()
  {
    if(m_instance == 0)m_instance=new SingleQuoteSTState();
    return m_instance;
  }

  /////////////////////////////////////////////////////////////////////////////

  class EscapedSTState: public STState
  {
  public:
    virtual bool operate(StringTokenizer& machine);
    virtual ~EscapedSTState();
    static EscapedSTState* Instance();
  protected:
    EscapedSTState() {}
    static EscapedSTState* m_instance;
  };

  inline EscapedSTState* EscapedSTState::Instance()
  {
    if(m_instance == 0)m_instance=new EscapedSTState();
    return m_instance;
  }

  
} // namespace NS_Analysis

#endif // defined STRINGTOKENIZER_H
