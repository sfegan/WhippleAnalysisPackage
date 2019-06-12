//-*-mode:c++; mode:font-lock;-*-

#ifndef PARAMFILEGENERATOR_H
#define PARAMFILEGENERATOR_H

#define __STL_USE_NAMESPACES

#include"ChannelRepresentation.h"
#include"RedHeader.h"
#include"RedEvent.h"
#include"RedFile.h"
#include"HillasParam.h"
#include"HillasParameterization.h"
#include"EventInfo.h"
#include"ParamFile.h"
#include"ProgressBar.h"

namespace NS_Analysis {

  class PFGenREOperator: public RedEventOperator
  {
  public:
    PFGenREOperator(const ECRGenerator* ecrgen, 
		    const HillasParameterization* par,
		    ParamFile* pf)
      : RedEventOperator(), m_pf(pf), m_ecrgen(ecrgen), m_parameterize(par) {}

    virtual ~PFGenREOperator();
    
    virtual void operateOnEvent(int evno, const RedEvent* re);
    
  private:
    ParamFile* m_pf;
    const ECRGenerator* m_ecrgen;
    const HillasParameterization* m_parameterize;
  };

  class ParamFileGenerator
  {
  public:
    ParamFileGenerator(const ECRGenerator* ecrgen,
		       const HillasParameterization* par,
		       ProgressBar* pb)
      : m_ecrgen(ecrgen), m_parameterize(par), m_pb(pb) {}

    void generate(RedFile* rf, ParamFile* pf);
		  
  private:
    const ECRGenerator* m_ecrgen;
    const HillasParameterization* m_parameterize;
    ProgressBar* m_pb;
  };

}

#endif // PARAMFILEGENERATOR_H
