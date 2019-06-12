#include "CutParamFile.h"

NS_Analysis::HPCutsPESelector::
~HPCutsPESelector()
{

}

bool 
NS_Analysis::HPCutsPESelector::
takeThisEvent(int evno, const EventInfo* ei, const HillasParam* hp)
{
  if((ei->code() == 8)&&(m_cuts->test(*hp)))return true;
  return false;
}

///////////////////////////////////////////////////////////////////////////////

NS_Analysis::CutParamFile::
~CutParamFile()
{
  
}

void 
NS_Analysis::CutParamFile::
create(const string& filename, unsigned int version)
{
  ParamFile::create(filename,version);
  m_cuts.manage(new VSFA_cH5<BaseCutsHillasParam>(version, *h5file(), "cuts",
						  H5P_DEFAULT, 1));
}

void 
NS_Analysis::CutParamFile::
openorcreate(const string& filename, unsigned int version)
{
  ParamFile::openorcreate(filename,version);

  if(h5file()->isDataset("cuts"))
    m_cuts.manage(new VSFA_cH5<BaseCutsHillasParam>(*h5file(), "cuts"));
  else 
    m_cuts.manage(new VSFA_cH5<BaseCutsHillasParam>(version, *h5file(), "cuts",
						      H5P_DEFAULT, 1));
}

void 
NS_Analysis::CutParamFile::
open(const string& filename)
{
  ParamFile::open(filename);
  m_cuts.manage(new VSFA_cH5<BaseCutsHillasParam>(*h5file(), "cuts"));
}

void 
NS_Analysis::CutParamFile::
close()
{
  m_cuts.reset();
  ParamFile::close();
}

///////////////////////////////////////////////////////////////////////////////

NS_Analysis::CutParamFileGenerator::
~CutParamFileGenerator()
{
}

void
NS_Analysis::CutParamFileGenerator::
operateOnEvent(int evno, const EventInfo* ei, const HillasParam* hp)
{
  m_out->evinfo()->appendOne(ei);
  m_out->params()->appendOne(hp);
}
    
void 
NS_Analysis::CutParamFileGenerator::
cut(ParamFile* in, ParamFile* out)
{
  m_out=out;

  RedHeader rh;
  in->header()->read(0,&rh,1);

  HPCutsPESelector hpc_sel(m_cuts);
  ParamEventSelectedOperator peso(this,&hpc_sel);
  ParamFileEventProcessor pfproc(&peso,m_pb);
  pfproc.run(in);

  rh.setNEvents(in->evinfo()->size());
  out->header()->write(0,&rh,1);
}
