//-*-mode:c++; mode:font-lock;-*-

#ifndef REDFILE_H
#define REDFILE_H

#define __STL_USE_NAMESPACES

#include<string>

#include<hdf5/hdf5.h>

#include"cH5Utils.h"
#include"VSFA_cH5.h"
#include"VSFA_Caching.h"

#include"Types.h"
#include"ProgressBar.h"
#include"Exceptions.h"
#include"RedHeader.h"
#include"RedEvent.h"

namespace NS_Analysis {
  
  using std::string;

  class RedFile
  {
  public:
    typedef VSFA<RedHeader>        header_list_t;
    typedef VSFA<RedEvent>         events_list_t;
    
  private:
    static const int               BUFFER = 5000;
    unsigned int                   m_buffersize;

    bool                           m_fileisopen;
    MPtr<cH5File>                  m_file;
    
    MPtr<header_list_t>            m_header;
    MPtr<events_list_t>            m_events;
    
    RedFile(const RedFile&);
    RedFile& operator= (const RedFile&);
    
    void testopen() const 
    {
      if(!m_fileisopen)
	{
	  Error err("RedFile::testopen");
	  err.stream() << "Attempt to access unopened RedFile" << endl;
	  throw(err);
	}
    }
    
  public:
    cH5File* h5file() { return m_file.get(); }
    const cH5File* h5file() const { return m_file.get(); }

    void create(const string& filename, unsigned int version);
    void openorcreate(const string& filename, unsigned int version);
    void open(const string& filename);
    void close();

    RedFile(int buf=BUFFER): m_buffersize(buf), m_fileisopen(false)
    { }
    RedFile(const string& filename, unsigned int version) 
      : m_buffersize(BUFFER)
    { create(filename,version); }
    RedFile(const string& filename) 
      : m_buffersize(BUFFER)
    { open(filename); }

    virtual ~RedFile();

    const header_list_t* header() const { testopen(); return m_header.get(); }
    header_list_t* header() { testopen(); return m_header.get(); }

    const events_list_t* events() const { testopen(); return m_events.get(); }
    events_list_t* events() { testopen(); return m_events.get(); }
  };

  /////////////////////////////////////////////////////////////////////////////
  //
  // The RedFileEventProcessor class runs through all events and does a class
  // defined operation on them. The abstract class RedEventOperator should be
  // implemented as a drived class to do the operation required.
  //

  class RedEventOperator
  {
  public:
    virtual ~RedEventOperator();
    virtual void operateOnEvent(int evno, const RedEvent* re) = 0;
  };

  class RedEventSelector
  {
  public:
    virtual ~RedEventSelector();
    virtual bool takeThisEvent(int evno, const RedEvent* re) = 0;
  };

  class RedEventSelectedOperator: public RedEventOperator
  {
  public:
    virtual ~RedEventSelectedOperator();
    virtual void operateOnEvent(int evno, const RedEvent* re);

    RedEventSelectedOperator(RedEventOperator* op, RedEventSelector* sel):
      m_operator(op), m_selector(sel) {}
  private:
    RedEventOperator* m_operator;
    RedEventSelector* m_selector;
  };
  
  class RedFileEventProcessor
  {
  public:
    RedFileEventProcessor(RedEventOperator* reo, 
			  ProgressBar* pb=0): 
      m_reo(reo), m_pb(pb) {}
    void setREO(RedEventOperator* reo) { m_reo=reo; }

    void run(RedFile* rf);

  private:
    RedEventOperator* m_reo;
    ProgressBar* m_pb;
  };

  /////////////////////////////////////////////////////////////////////////////
  //
  // class AllEventsRESelector: a simple example of RedEventSelector which
  // just accepts all events, no questions asked
  //
  /////////////////////////////////////////////////////////////////////////////

  class AllEventsRESelector: public RedEventSelector
  {
  public:
    virtual bool takeThisEvent(int evno, const RedEvent* re);
    AllEventsRESelector() { }

  private:
    AllEventsRESelector(const AllEventsRESelector&);
    AllEventsRESelector& operator= (const AllEventsRESelector&);
  };

  class Code8RESelector: public RedEventSelector
  {
  public:
    virtual bool takeThisEvent(int evno, const RedEvent* re);
    Code8RESelector() { }
  };

  class ByCodeRESelector: public RedEventSelector
  {
  public:
    ByCodeRESelector(int code): m_code(code) {}
    virtual ~ByCodeRESelector();
    virtual bool takeThisEvent(int evno, const RedEvent* re);
  private:
    int m_code;
  };

}; // namespace NS_Analysis

inline void
NS_Analysis::RedFileEventProcessor::
run(RedFile* rf)
{
  int nevents=rf->events()->size();
  if(m_pb)m_pb->reset(nevents);
  for(int i=0; i<nevents; i++)
    {
      if(m_pb)m_pb->tick(i);
      RedEvent re;
      rf->events()->read(i,&re,1);
      m_reo->operateOnEvent(i,&re);
    }
  if(m_pb)m_pb->tick(nevents);
}

#endif // REDFILE_H
