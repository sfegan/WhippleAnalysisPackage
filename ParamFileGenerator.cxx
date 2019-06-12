#include<memory>

#include"ParamFileGenerator.h"

NS_Analysis::PFGenREOperator::
~PFGenREOperator()
{

}

void
NS_Analysis::PFGenREOperator::
operateOnEvent(int evno, const RedEvent* re)
{
  std::auto_ptr<EventChannelReps> ecr(m_ecrgen->generate(re));
  
  HillasParam hp;
  hp.setVersion(m_pf->params()->tVersion());
  m_parameterize->parameterize(&hp,ecr.get(),0,0);

  EventInfo evi;
  evi.setVersion(m_pf->evinfo()->tVersion());
  
  evi.setEventNumber(evno);
  evi.setCode(re->code());
  evi.setTime(re->time());
  evi.setGPSUTC(re->gpsutc());
  evi.setLiveTime(re->live_time());
  
  m_pf->params()->appendOne(&hp);
  m_pf->evinfo()->appendOne(&evi);
}
 
void
NS_Analysis::ParamFileGenerator::
generate(RedFile* rf, ParamFile* pf)
{
  RedHeader rh;
  rf->header()->read(0,&rh,1);
  pf->header()->write(0,&rh,1);

  PFGenREOperator pfgenreop(m_ecrgen, m_parameterize, pf);
  RedFileEventProcessor proc(&pfgenreop,m_pb);
  proc.run(rf);
}
