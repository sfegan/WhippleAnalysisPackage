#include<hdf5/hdf5.h>

#include"VSFA_cH5.h"
#include"VSFA_Caching.h"

#include"RedHeader.h"
#include"RedEvent.h"
#include"RedFile.h"

void NS_Analysis::RedFile::
create(const string& filename, unsigned int version)
{
  m_file.manage(new cH5FileNewTruncate(filename));
  
  m_header.manage(new VSFA_cH5<RedHeader>(version, *m_file, "header",
					  H5P_DEFAULT, 1));
  
  hid_t compress;
  compress=H5Pcreate(H5P_DATASET_CREATE);
  //  H5Pset_deflate(compress, 4); 

  VSFA<RedEvent>* fa;
  fa = new VSFA_cH5<RedEvent>(version, *m_file, "events", compress);
  m_events.manage(new VSFA_Caching<RedEvent>(fa, m_buffersize, m_buffersize,
					     true));
		 
  H5Pclose(compress);

  m_fileisopen=true;
}

void NS_Analysis::RedFile::
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
  
  VSFA<RedEvent>* fa;
  if(m_file->isDataset("events"))
    fa = new VSFA_cH5<RedEvent>(*m_file, "events");
  else
    {
      hid_t compress;
      compress=H5Pcreate(H5P_DATASET_CREATE);
      //  H5Pset_deflate(compress, 4); 

      fa = new VSFA_cH5<RedEvent>(version, *m_file, "events", compress);
      H5Pclose(compress);
    }
  
  m_events.manage(new VSFA_Caching<RedEvent>(fa, m_buffersize, m_buffersize,
					     true));
  
  m_fileisopen=true;
}

void NS_Analysis::RedFile::
open(const string& filename)
{
  m_file.manage(new cH5FileReadOnly(filename));
  
  m_header.manage(new VSFA_cH5<RedHeader>(*m_file, "header"));

  VSFA<RedEvent>* fa =  new VSFA_cH5<RedEvent>(*m_file, "events");
  m_events.manage(new VSFA_Caching<RedEvent>(fa, m_buffersize, m_buffersize,
					     true));
  
  m_fileisopen=true;
}

void NS_Analysis::RedFile::
close()
{
  testopen();
  m_header.reset();
  m_events.reset();
  m_file.reset();
  m_fileisopen=false;
}

NS_Analysis::RedFile::
~RedFile()
{ 
  if(m_fileisopen)close(); 
}

NS_Analysis::RedEventOperator::
~RedEventOperator()
{
}

NS_Analysis::RedEventSelector::
~RedEventSelector()
{
}

NS_Analysis::RedEventSelectedOperator::
~RedEventSelectedOperator()
{
}

void 
NS_Analysis::RedEventSelectedOperator::
operateOnEvent(int evno, const RedEvent* re)
{
  if(m_selector->takeThisEvent(evno,re))
    m_operator->operateOnEvent(evno,re);
}

bool 
NS_Analysis::AllEventsRESelector::
takeThisEvent(int evno, const RedEvent* re)
{
  return true;
}

bool 
NS_Analysis::Code8RESelector::
takeThisEvent(int evno, const RedEvent* re)
{
  return(re->code()==8);
}

NS_Analysis::ByCodeRESelector::
~ByCodeRESelector()
{
}

bool 
NS_Analysis::ByCodeRESelector::
takeThisEvent(int evno, const RedEvent* re)
{
  return(re->code()==m_code);
}
