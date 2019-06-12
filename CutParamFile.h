//-*-mode:c++; mode:font-lock;-*-

#ifndef CUTPARAMFILE_H
#define CUTPARAMFILE_H

#include "Types.h"

#include "cH5Utils.h"
#include "VSFA.h"
#include "VSFA_cH5.h"
#include "VSFA_Caching.h"

#include "RedHeader.h"
#include "HillasParam.h"
#include "EventInfo.h"
#include "ParamFile.h"

#include "BaseCutsHillasParam.h"
#include "CutsHillasParam.h"

#include "ProgressBar.h"

namespace NS_Analysis {







  class HPCutsPESelector: public ParamEventSelector
  {
  public:
    HPCutsPESelector(const CutsHillasParam* cuts): m_cuts(cuts) {}
    virtual ~HPCutsPESelector();
    virtual bool takeThisEvent(int evno,
			       const EventInfo* ei, const HillasParam* hp);
    
  private:
    const CutsHillasParam* m_cuts;
  };

  class CutParamFile: public ParamFile
  {
  public:
    typedef VSFA<BaseCutsHillasParam>                    cuts_list_t;
    
  private:
    MPtr<cuts_list_t>                                    m_cuts;

  public:
    virtual void create(const string& filename, unsigned int version);
    virtual void openorcreate(const string& filename, unsigned int version);
    virtual void open(const string& filename);
    virtual void close();

    CutParamFile(): ParamFile(), m_cuts() {}
    CutParamFile(const string& filename, unsigned int version):
      ParamFile(filename, version), m_cuts() {}
    CutParamFile(const string& filename): 
      ParamFile(filename), m_cuts() {}
    virtual ~CutParamFile();
  };

  class CutParamFileGenerator: private ParamEventOperator
  {
  public:
    CutParamFileGenerator(const CutsHillasParam* cuts, ProgressBar* pb=0)
      : m_cuts(cuts), m_pb(pb) {}

    virtual ~CutParamFileGenerator();

    virtual void operateOnEvent(int evno, 
				const EventInfo* ei, const HillasParam* hp);
    
    void cut(ParamFile* in, ParamFile* out);
    
  private:
    const CutsHillasParam* m_cuts;
    ProgressBar* m_pb;

    ParamFile* m_out;
  };

} // namespace CutParamFile

#endif // defined CUTPARAMFILE_H
