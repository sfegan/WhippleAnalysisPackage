//-*-mode:c++; mode:font-lock;-*-

#define NOEXTERNALIZE
#include <gdf.h>
#undef  NOEXTERNALIZE

#include "fz.h"

using namespace std;

string GDFError::exceptionType() const
{
  return "GDFError";
}

int GDFSystem::m_instances = 0;

GDFSystem::GDFSystem()
{
  if(m_instances == 0)
    {
      unsigned uval;
      int ierr=0;
      uval=1; gdf_option_("CIO", &uval, &ierr, 3 /* strlen("CIO") */);
      uval=1; gdf_option_("RESET", &uval, &ierr, 3 /* strlen("RESET") */);
      gdf_init_("Z",&ierr,1);
      if(ierr)
	{
	  GDFError err("GDFSystem::GDFSystem",ierr);
	  err.stream() << "GDF system could not be initialized" << endl;
	  throw err;
	}
    }
  m_instances++;
}

GDFSystem::~GDFSystem()
{
  m_instances--;
  if(m_instances == 0)
    {
      int ierr=0;
      gdf_exit_(&ierr);
    }
}

void GDFSystem::demandInitialized()
{
  if(m_instances > 0)return;
  Error err("GDFSystem::demandInitialized");
  err.stream() << "GDF system not initialized" << endl;
  throw err;
}

GDFRecordHandler::~GDFRecordHandler()
{
}

void 
GDFRecordHandler::run(GDFRecordDispatcher &dispatcher, 
		      const struct gdf_run_s& record)
{
  // DEFAULT BEHAVIOUR : Do nothing at all
}

void 
GDFRecordHandler::ev10(GDFRecordDispatcher &dispatcher, 
		       const struct gdf_ev10_s& record)
{
  // DEFAULT BEHAVIOUR : Do nothing at all
}
  
void 
GDFRecordHandler::track(GDFRecordDispatcher &dispatcher, 
			const struct gdf_track_s& record)
{
  // DEFAULT BEHAVIOUR : Do nothing at all
}
  
void 
GDFRecordHandler::hv(GDFRecordDispatcher &dispatcher, 
		     const struct gdf_hv_s& record)
{
  // DEFAULT BEHAVIOUR : Do nothing at all
}
  
void 
GDFRecordHandler::fr10(GDFRecordDispatcher &dispatcher, 
		       const struct gdf_fr10_s& record)
{
  // DEFAULT BEHAVIOUR : Do nothing at all
}

void 
GDFRecordHandler::starting_file(GDFRecordDispatcher &dispatcher,
				const string& filename)
{
  // DEFAULT BEHAVIOUR : Do nothing at all
}

void 
GDFRecordHandler::finished_file(GDFRecordDispatcher &dispatcher,
				const string& filename)
{
  // DEFAULT BEHAVIOUR : Do nothing at all
}

bool GDFRecordDispatcher::m_dispatcher_active = false;

void
GDFRecordDispatcher::
process(const string& filename)
{
  GDFSystem::demandInitialized();

  if(m_dispatcher_active)
    {
      Error err("GDFRecordDispatcher::process");
      err.stream() 
	<< "Programming error: The GDF Dispatcher is already active !" << endl;
      throw err;
    }
  m_dispatcher_active = true;

  m_stop_requested = false;

  int lunit = 10;
  int ierr = 0;

  // Lets go.. open the file then read all the records

  gdf_open_(&lunit,filename.c_str(),"R",&ierr,filename.length(),1);
  if(ierr)
    {
      GDFError err("GDFRecordDispatcher::process",ierr);
      err.stream() << "Error opening GDF file: " << filename << endl;
      throw(err);
    }

  m_handler->starting_file(*this, filename);

  while(!m_stop_requested)
    {
      ierr = 0;
      gdf_read_(&lunit," ",&ierr,1);
      if((ierr == 1)||((ierr>0)&&(m_ignore_gdf_read_errors)))break;
      else if(ierr != 0)
	{
	  GDFError err("GDFRecordDispatcher::process",ierr);
	  err.stream() << "Error reading GDF file: " << filename << endl;
	  
	  gdf_close_(&lunit,&ierr);  // WHAT SHOULD WE DO IN CASE OF ERROR ??
	  m_dispatcher_active=false; // LETS CLOSE THE FILE AND THROW AN ERROR

	  throw(err);
	}

      if(gdf_data_.gdf_run.record_new)
	m_handler->run(*this, gdf_data_.gdf_run);
      if(m_stop_requested)break;

      if(gdf_data_.gdf_ev10.record_new)
	m_handler->ev10(*this, gdf_data_.gdf_ev10);
      if(m_stop_requested)break;
      
      if(gdf_data_.gdf_track[0].record_new)
	m_handler->track(*this, gdf_data_.gdf_track[0]);
      if(m_stop_requested)break;

      if(gdf_data_.gdf_hv[0].record_new)
	m_handler->hv(*this, gdf_data_.gdf_hv[0]);
      if(m_stop_requested)break;

      if(gdf_data_.gdf_fr10.record_new)
	m_handler->fr10(*this, gdf_data_.gdf_fr10);
      if(m_stop_requested)break;
    }

  // All done, close the file and leave

  m_handler->finished_file(*this, filename);

  ierr = 0;
  gdf_close_(&lunit,&ierr);
  if(ierr)
    {
      GDFError err("GDFRecordDispatcher::process",ierr);
      err.stream() << "Error closing GDF file: " << filename << endl;
      throw(err);
    }

  m_dispatcher_active = false;
}

void 
GDFRecordDispatcher::
stopProcessing()
{
  m_stop_requested=true;
}
