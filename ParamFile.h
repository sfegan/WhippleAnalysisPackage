//-*-mode:c++; mode:font-lock;-*-

#ifndef PARAMFILE_H
#define PARAMFILE_H

#define __STL_USE_NAMESPACES

#include<string>

#include<hdf5/hdf5.h>

#include"Types.h"
  
#include"cH5Utils.h"
#include"VSFA.h"
#include"VSFA_cH5.h"
#include"VSFA_Caching.h"

#include"Exceptions.h"
#include"RedHeader.h"
#include"HillasParam.h"
#include"EventInfo.h"
#include"ProgressBar.h"

namespace NS_Analysis {
  
  using std::string;

  class ParamFile
  {
  public:
    typedef VSFA<RedHeader>                               header_list_t;
    typedef VSFA<EventInfo>                               evinfo_list_t;
    typedef VSFA<HillasParam>                             params_list_t;
    
  private:
    static const int                                      BUFFER = 5000;

    unsigned int                                          m_buffersize;

    bool                                                  m_fileisopen;

    MPtr<cH5File>                                         m_file;

    MPtr<header_list_t>                                   m_header;
    MPtr<evinfo_list_t>                                   m_evinfo;
    MPtr<params_list_t>                                   m_params;
    
    ParamFile(const ParamFile&);
    ParamFile& operator=(const ParamFile&);
    
    void testopen() const 
    { 
      if(!fileisopen())
	{
	  Error err("ParamFile::testopen");
	  err.stream() << "Attempt to access unopened ParamFile" << endl;
	  throw(err);
	}
    }
    
  public:
    cH5File* h5file() { return m_file.get(); }
    const cH5File* h5file() const { return m_file.get(); }

    bool fileisopen() const { return m_fileisopen; }

    virtual void create(const string& filename, unsigned int version);
    virtual void openorcreate(const string& filename, unsigned int version);
    virtual void open(const string& filename);
    virtual void close();

    ParamFile(int buf=BUFFER): m_buffersize(buf), m_fileisopen(false) {}
    ParamFile(const string& filename, unsigned int version);
    ParamFile(const string& filename);
    virtual ~ParamFile();
    
    const header_list_t* header() const { testopen(); return m_header.get(); }
    header_list_t* header() { testopen(); return m_header.get(); }

    const evinfo_list_t* evinfo() const { testopen(); return m_evinfo.get(); }
    evinfo_list_t* evinfo() { testopen(); return m_evinfo.get(); }

    const params_list_t* params() const { testopen(); return m_params.get(); }
    params_list_t* params() { testopen(); return m_params.get(); }
  };

  /////////////////////////////////////////////////////////////////////////////
  //
  // The ParamFileEventProcessor class runs through all events and does a class
  // defined operation on them. The abstract class ParamEventOperator should be
  // implemented as a drived class to do the operation required.
  //

  class ParamEventOperator
  {
  public:
    virtual ~ParamEventOperator();
    virtual void operateOnEvent(int evno, 
				const EventInfo* ei, const HillasParam* hp)=0;
  };

  class ParamEventSelector
  {
  public:
    virtual ~ParamEventSelector();
    virtual bool takeThisEvent(int evno,
			       const EventInfo* ei, const HillasParam* hp)=0;
  };

  class ParamEventSelectedOperator: public ParamEventOperator
  {
  public:
    virtual ~ParamEventSelectedOperator();
    virtual void operateOnEvent(int evno,
				const EventInfo* ei, const HillasParam* hp);
    
    ParamEventSelectedOperator(ParamEventOperator* op,
			       ParamEventSelector* sel):
      m_operator(op), m_selector(sel) {}
  private:
    ParamEventOperator* m_operator;
    ParamEventSelector* m_selector;
  };
  
  class ParamFileEventProcessor
  {
  public:
    ParamFileEventProcessor(ParamEventOperator* peo, 
			    ProgressBar* pb=0): 
      m_peo(peo), m_pb(pb) {}
    void setREO(ParamEventOperator* peo) { m_peo=peo; }
    
    void run(ParamFile* pf);
    
  private:
    ParamEventOperator* m_peo;
    ProgressBar* m_pb;
  };
  
  /////////////////////////////////////////////////////////////////////////////
  //
  // class AllEventsRESelector: a simple example of ParamEventSelector which
  // just accepts all events, no questions asked
  //
  /////////////////////////////////////////////////////////////////////////////

  class AllEventsPESelector: public ParamEventSelector
  {
  public:
    virtual ~AllEventsPESelector();
    virtual bool takeThisEvent(int evno, 
			       const EventInfo* ei, const HillasParam* hp);
  };

  class Code8PESelector: public ParamEventSelector
  {
  public:
    virtual ~Code8PESelector();
    virtual bool takeThisEvent(int evno,
			       const EventInfo* ei, const HillasParam* hp);
  };
  
  class ByCodePESelector: public ParamEventSelector
  {
  public:
    ByCodePESelector(int code): m_code(code) {}
    virtual ~ByCodePESelector();
    virtual bool takeThisEvent(int evno,
			       const EventInfo* ei, const HillasParam* hp);
  private:
    int m_code;
  };
  
}; // namespace NS_Analysis

inline NS_Analysis::ParamFile::
ParamFile(const string& filename, unsigned int version):
  m_buffersize(BUFFER)
{
  create(filename,version);
}

inline NS_Analysis::ParamFile::
ParamFile(const string& filename):
  m_buffersize(BUFFER)
{
  open(filename);
}

#endif // PARAMFILE_H
