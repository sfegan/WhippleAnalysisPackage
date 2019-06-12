#include<hdf5/hdf5.h>

#include"VSFA_cH5.h"
#include"VSFA_Caching.h"

#include"ParamFile.h"

void NS_Analysis::ParamFile::
create(const string& filename, unsigned int version)
{
  m_file.manage(new cH5FileNewTruncate(filename));
  
  m_header.manage(new VSFA_cH5<RedHeader>(version, *m_file, "header",
					  H5P_DEFAULT, 1));
  
  hid_t compress;
  compress=H5Pcreate(H5P_DATASET_CREATE);
  //  H5Pset_deflate(compress, 4); 

  params_list_t* pfa;
  pfa=new VSFA_cH5<HillasParam>(version, *m_file, "param", compress);
  m_params.manage(new VSFA_Caching<HillasParam>(pfa, m_buffersize, 
						m_buffersize, true));
  
  evinfo_list_t* efa;
  efa=new VSFA_cH5<EventInfo>(version, *m_file, "evinfo", compress);
  m_evinfo.manage(new VSFA_Caching<EventInfo>(efa, m_buffersize, 
					      m_buffersize, true));

  H5Pclose(compress);
  
  m_fileisopen=true;
}

void NS_Analysis::ParamFile::
openorcreate(const string& filename, unsigned int version)
{
  if(H5Fis_hdf5(filename.c_str())!=1)
    {
      create(filename,version);
      return;
    }
  
  m_file.manage(new cH5FileReadWrite(filename));
  
  if(m_file->isDataset("header"))
    m_header.manage(new VSFA_cH5<RedHeader>(*m_file, "header"));
  else 
    m_header.manage(new VSFA_cH5<RedHeader>(version, *m_file, "header",
					    H5P_DEFAULT, 1));

  hid_t compress;
  compress=H5Pcreate(H5P_DATASET_CREATE);
  //  H5Pset_deflate(compress, 4); 
  
  params_list_t* pfa;
  if(m_file->isDataset("param"))
    pfa=new VSFA_cH5<HillasParam>(*m_file, "param");
  else
    pfa=new VSFA_cH5<HillasParam>(version, *m_file, "param", compress);
  m_params.manage(new VSFA_Caching<HillasParam>(pfa, m_buffersize, 
						m_buffersize, true));
  
  evinfo_list_t* efa;
  if(m_file->isDataset("einfo"))
    efa=new VSFA_cH5<EventInfo>(*m_file, "evinfo");
  else
    efa=new VSFA_cH5<EventInfo>(version, *m_file, "evinfo", compress);
  m_evinfo.manage(new VSFA_Caching<EventInfo>(efa, m_buffersize, 
					      m_buffersize, true));
  
  H5Pclose(compress);

  m_fileisopen=true;
}

void NS_Analysis::ParamFile::
open(const string& filename)
{
  m_file.manage(new cH5FileReadOnly(filename));

  m_header.manage(new VSFA_cH5<RedHeader>(*m_file, "header"));

  params_list_t* pfa = new VSFA_cH5<HillasParam>(*m_file, "param");
  m_params.manage(new VSFA_Caching<HillasParam>(pfa, m_buffersize, 
						m_buffersize, true));
  
  evinfo_list_t* efa = new VSFA_cH5<EventInfo>(*m_file, "evinfo");
  m_evinfo.manage(new VSFA_Caching<EventInfo>(efa, m_buffersize, 
					      m_buffersize, true));

  m_fileisopen=true;
}

void 
NS_Analysis::ParamFile::
close()
{
  testopen();
  m_header.reset();
  m_params.reset();
  m_evinfo.reset();
  m_file.reset();
  m_fileisopen=false;
}

NS_Analysis::ParamFile::
~ParamFile()
{
  if(fileisopen())close();
}


NS_Analysis::ParamEventOperator::
~ParamEventOperator()
{
}

NS_Analysis::ParamEventSelector::
~ParamEventSelector()
{
}

NS_Analysis::ParamEventSelectedOperator::
~ParamEventSelectedOperator()
{
}

void 
NS_Analysis::ParamEventSelectedOperator::
operateOnEvent(int evno, 
	       const EventInfo* ei, const HillasParam* hp)
{
  if(m_selector->takeThisEvent(evno,ei,hp))
    m_operator->operateOnEvent(evno,ei,hp);
}

NS_Analysis::AllEventsPESelector::
~AllEventsPESelector()
{

}

bool 
NS_Analysis::AllEventsPESelector::
takeThisEvent(int evno,
	       const EventInfo* ei, const HillasParam* hp)
{
  return true;
}

NS_Analysis::Code8PESelector::
~Code8PESelector()
{

}

bool 
NS_Analysis::Code8PESelector::
takeThisEvent(int evno, const EventInfo* ei, const HillasParam* hp)
{
  if(ei->code() == 8)return true;
  return false;
}

NS_Analysis::ByCodePESelector::
~ByCodePESelector()
{
}

bool 
NS_Analysis::ByCodePESelector::
takeThisEvent(int evno, const EventInfo* ei, const HillasParam* hp)
{
  return(ei->code()==m_code);
}

void
NS_Analysis::ParamFileEventProcessor::
run(ParamFile* pf)
{
  int nevents=pf->params()->size();
  if(m_pb)m_pb->reset(nevents);
  for(int i=0; i<nevents; i++)
    {
      if(m_pb)m_pb->tick(i);
 
      HillasParam hp;
      pf->params()->read(i,&hp,1);

      EventInfo ei;
      pf->evinfo()->read(i,&ei,1);

      m_peo->operateOnEvent(i,&ei,&hp);
    }
  if(m_pb)m_pb->tick(nevents);
}
