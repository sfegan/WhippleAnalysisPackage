#include"ADCSpectrum.h"

NS_Analysis::ADCSpectrum::
~ADCSpectrum()
{
}

void 
NS_Analysis::ADCSpectrumBuilder::
insert(RedFile* rf,
       RedEventSelector* sel,
       ProgressBar* pb)
{
  RedEventSelectedOperator reso(this,sel);
  RedFileEventProcessor processor(&reso,pb);
  processor.run(rf);
}

NS_Analysis::ADCSpectrumBuilder::
~ADCSpectrumBuilder()
{
}

void 
NS_Analysis::ADCSpectrumBuilder::
operateOnEvent(int evno, const RedEvent* re)
{
  m_spectrum->insert(re);
}
