//-*-mode:c++; mode:font-lock;-*-

#ifndef FZ_H
#define FZ_H

#include <Exceptions.h>

#include <gdf.h>

using NS_Analysis::Error;
using NS_Analysis::FileError;;

class GDFError: public Error
{
public:
  GDFError(const std::string& func, int err): Error(func), m_errno(err) {}
  
  virtual std::string exceptionType() const;
  int gdfErrorNumber() const { return m_errno; }
private:
  int m_errno;
};

class GDFSystem
{
public:
  GDFSystem();
  ~GDFSystem();
  
  static void demandInitialized();

private:
  static int m_instances;
};

class GDFRecordDispatcher;

class GDFRecordHandler
{
public:
  virtual ~GDFRecordHandler();

  virtual void run(GDFRecordDispatcher &dispatcher, 
		   const struct gdf_run_s& record);

  virtual void ev10(GDFRecordDispatcher &dispatcher, 
		    const struct gdf_ev10_s& record);
  
  virtual void track(GDFRecordDispatcher &dispatcher, 
		     const struct gdf_track_s& record);
  
  virtual void hv(GDFRecordDispatcher &dispatcher, 
		  const struct gdf_hv_s& record);
  
  virtual void fr10(GDFRecordDispatcher &dispatcher, 
		    const struct gdf_fr10_s& record);

  virtual void starting_file(GDFRecordDispatcher &dispatcher,
			     const std::string& filename);

  virtual void finished_file(GDFRecordDispatcher &dispatcher,
			     const std::string& filename);
};

class GDFRecordDispatcher
{
public:
  GDFRecordDispatcher(GDFRecordHandler* handler, bool igre=false)
    : m_handler(handler), 
      m_stop_requested(false), m_ignore_gdf_read_errors(igre) {}
  GDFRecordHandler* resetHandler(GDFRecordHandler* handler);
  void process(const std::string& filename);
  void stopProcessing();

private:
  static bool m_dispatcher_active;

  GDFRecordHandler* m_handler;
  bool m_stop_requested;

  bool m_ignore_gdf_read_errors;
};

inline 
GDFRecordHandler* GDFRecordDispatcher::resetHandler(GDFRecordHandler* handler)
{
  GDFRecordHandler* old_handler = m_handler;
  m_handler = handler;
  return old_handler;
}

#endif // defined FZ_H
