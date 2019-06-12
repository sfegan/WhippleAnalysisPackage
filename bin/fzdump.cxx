//-*-mode:c++; mode:font-lock;-*-
#include <iostream>
#include <iomanip>

#include <unistd.h>
#include <getopt.h>

#include "fz.h"

using namespace std;

class GDFRecordPrinter: public GDFRecordHandler
{
public:
  GDFRecordPrinter(bool run=true, bool ev10=false, bool track=false,
		   bool hv=false, bool fr10=false):
    m_show_run(run), m_show_ev10(ev10), m_show_track(track), m_show_hv(hv),
    m_show_fr10(fr10) {}

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

public:
  bool m_show_run;
  bool m_show_ev10;
  bool m_show_track;
  bool m_show_hv;
  bool m_show_fr10;
};

void 
GDFRecordPrinter::run(GDFRecordDispatcher &dispatcher, 
		      const struct gdf_run_s& record)
{
  if(!m_show_run)return;
  cout << "GDF RUN RECORD" << endl;
  cout << "  Version    " << record.version << endl
       << "  UTC        " << record.utc << endl
       << "  Status     " << record.status[0] << endl
       << "  Date       " << record.idate << endl
       << "  Time       " << record.itime << endl
       << "  Year       " << record.year << endl
       << "  Run        " << record.run << endl
       << "  Type       " << record.type << endl
       << "  Sky        " << record.sky_quality << endl
       << "  TrigMode   " << record.trig_mode[0] << endl
       << "  TrigNHigh  " << record.trig_nhig[0] << endl
       << "  TrigNLow   " << record.trig_nlow[0] << endl
       << "  SID Len    " << record.sid_length[0] << endl
       << "  SID Cycle  " << record.sid_cycle << endl
       << "  SID Actual " << record.sid_actual << endl
       << "  Trig LoTh  " << record.trig_thrlow[0] << endl
       << "  Trig HiTh  " << record.trig_thrhig[0] << endl
       << "  UTC Start  " << record.utc_start << endl
       << "  UTC End    " << record.utc_end << endl
       << "  File       " << record.file << endl
       << "  Observer   " << record.observer << endl
       << "  Comment    " << record.comment << endl
    ;

  cout << endl;
}

void 
GDFRecordPrinter::ev10(GDFRecordDispatcher &dispatcher, 
		       const struct gdf_ev10_s& record)
{
  if(!m_show_ev10)return;
  cout << "GDF EVENT RECORD" << endl;
  cout << "  Version    " << record.version << endl
       << "  UTC        " << record.utc << endl
       << "  NADC       " << record.nadc << endl 
       << "  Run        " << record.run << endl
       << "  Event Num  " << record.event << endl
       << "  Live Time  " << record.live_sec << " s " 
                          << record.live_ns << " ns" << endl
       << "  Elap Time  " << record.elapsed_sec << " s " 
                          << record.elapsed_ns << " ns" << endl
       << "  N Patterns " << record.ntrg << endl

       << "  Trigger    " << record.trigger << ' '
                          << (record.trigger&0x10 ? '1':'0')
                          << (record.trigger&0x08 ? '1':'0')
			  << (record.trigger&0x04 ? '1':'0')
			  << (record.trigger&0x02 ? '1':'0')
                          << (record.trigger&0x01 ? '1':'0') << endl

       << "  GRS        " << ((record.grs_clock[2]&0x00000F00) >> 8)
                          << ((record.grs_clock[2]&0x000000F0) >> 4) 
                          << ((record.grs_clock[2]&0x0000000F) >> 0) << ' '

                          << ((record.grs_clock[1]&0x00F00000) >> 20)
                          << ((record.grs_clock[1]&0x000F0000) >> 16) << ':'
                          << ((record.grs_clock[1]&0x0000F000) >> 12) 
                          << ((record.grs_clock[1]&0x00000F00) >> 8)  << ':'
                          << ((record.grs_clock[1]&0x000000F0) >> 4) 
                          << ((record.grs_clock[1]&0x0000000F) >> 0) << ' '
    
                          << record.grs_clock[0] << " - TQ/S: "

                          << (record.grs_clock[2]&0x00800000 ? '1':'0')
                          << (record.grs_clock[2]&0x00400000 ? '1':'0')
			  << (record.grs_clock[2]&0x00200000 ? '1':'0')
                          << (record.grs_clock[2]&0x00100000 ? '1':'0') 
    
                          << "  Y(1) " 
                          << ((record.grs_clock[2]&0x0000F000) >> 12) 
                          << "  Y(0) "
                          << ((record.grs_clock[2]&0x000F0000) >> 16)
                          << endl

       << "  Status     " << record.status << endl

       << "  Mark GPS   " << record.mark_gps << endl
       << "  Mark Open  " << record.mark_open << endl
       << "  Mark Close " << record.mark_close << endl

       << "  Gate Open  " << record.gate_open << endl
       << "  Gate Close " << record.gate_close << endl

       << "  GPS        " << record.gps_mjd << " MJD "
                          << record.gps_sec << " sec "
                          << record.gps_ns << " ns" << endl
       << "  GPS Bits   " << record.gps_clock[0] << ' '
                          << record.gps_clock[1] << ' '
                          << record.gps_clock[2] << endl
       << "  GPS Status " << record.gps_status[0] << ' '
                          << record.gps_status[0] << endl

       << "  Encoders   " << record.track[0] << ' ' 
                          << record.track[1] << endl
    ;

  cout << endl;
}
  
void 
GDFRecordPrinter::track(GDFRecordDispatcher &dispatcher, 
			const struct gdf_track_s& record)
{
  if(!m_show_track)return;
  cout << "GDF TRACKING RECORD" << endl;
  cout << "  Version    " << record.version << endl
       << "  UTC        " << record.utc << endl
       << "  Telescope  " << record.telescope << endl
       << "  Mode       " << record.mode << endl
       << "  Cycle Num  " << record.cycle << endl
       << "  Status     " << record.status << endl
       << "  RA  2000   " << record.rasc_2000 << endl
       << "  DEC 2000   " << record.decl_2000 << endl
       << "  RA  Today  " << record.rasc_today << endl
       << "  DEC Today  " << record.decl_today << endl
       << "  RA  Scope  " << record.rasc_tele << endl
       << "  DEC Scope  " << record.decl_tele << endl
       << "  Azimuth    " << record.azimuth << endl
       << "  Elevation  " << record.elevation << endl
       << "  Deviation  " << record.deviation << endl
       << "  Offset RA  " << record.rasc_offset << endl
       << "  Offset DEC " << record.decl_offset << endl
       << "  LST        " << record.stl << endl
       << "  Height     " << record.height << endl
       << "  AZ Change  " << record.azi_incl << endl
       << "  EL Change  " << record.ele_incl << endl
       << "  Source     " << record.source << endl
    ;

  cout << endl;
}
  
void 
GDFRecordPrinter::hv(GDFRecordDispatcher &dispatcher, 
		     const struct gdf_hv_s& record)
{
  if(!m_show_hv)return;
  cout << "GDF HV RECORD" << endl;
  cout << "  Version    " << record.version << endl
       << "  UTC        " << record.utc << endl
       << "  Telescope  " << record.telescope << endl
       << "  Mode       " << record.mode << endl
       << "  N Channels " << record.nch << endl
       << "  Cycle Num  " << record.cycle << endl
       << "  Data...... " << endl
    ;
  cout << endl;
}
  
void 
GDFRecordPrinter::fr10(GDFRecordDispatcher &dispatcher, 
		       const struct gdf_fr10_s& record)
{
  if(!m_show_fr10)return;
  cout << "GDF FRAME RECORD" << endl;
  cout << endl;
}

class GDFTimesDumper: public GDFRecordHandler
{
public:
  GDFTimesDumper(): haveoneevent(false) {} 

  virtual void ev10(GDFRecordDispatcher &dispatcher, 
		    const struct gdf_ev10_s& record);

private:
  bool haveoneevent;
  int firstgpssec;
};

void 
GDFTimesDumper::ev10(GDFRecordDispatcher &dispatcher, 
		     const struct gdf_ev10_s& record)
{
  int trigger       = record.trigger;
  int code=0;
  if     (trigger == 0x1)code=1; // Pedestal ONLY if bit 1 (and only bit 1) set
  else if(trigger == 0x6)code=8; // Event if bits 2&3 set (ONLY)
  else if(trigger == 0x4)code=8; // Event if bit 3 set (ONLY)
  else if(trigger == 0x2)code=8; // Event if bit 2 set (ONLY)
  else if(trigger &  0x8)code=2; // CFD test event
  else                   code=0; // UNKNOWN

  long long livetime = 
    (long long)(record.live_sec)*10000000LL +
    (long long)(record.live_ns+50)/100;

  double elaptime = 
    (long long)(record.elapsed_sec)*10000000LL +
    (long long)(record.elapsed_ns+50)/100;

  int gpssec = 
    ((record.grs_clock[1]&0x00F00000) >> 20) * 60*60*10 + // HRS
    ((record.grs_clock[1]&0x000F0000) >> 16) * 60*60    + // HRS
    ((record.grs_clock[1]&0x0000F000) >> 12) * 60*10    + // MINS
    ((record.grs_clock[1]&0x00000F00) >> 8)  * 60       + // MINS
    ((record.grs_clock[1]&0x000000F0) >> 4)  * 10       + // SECS
    ((record.grs_clock[1]&0x0000000F) >> 0)             ; // SECS
  
  if(!haveoneevent)
    {
      haveoneevent=true;
      firstgpssec=gpssec;
    }

  long long gpstime = 
    (long long)(gpssec-firstgpssec)*10000000LL + 
    (long long)(record.grs_clock[0]);

  cout << setw(20) << setprecision(20) << elaptime << ' ' 
       << setw(20) << setprecision(20) << gpstime << ' ' 
       << setw(20) << setprecision(20) << livetime << ' '
       << setw(9) << setprecision(9) << record.elapsed_sec << ' ' 
       << setw(9) << setprecision(9) << record.elapsed_ns << ' ' 
       << setw(9) << setprecision(9) << gpssec << ' ' 
       << setw(9) << setprecision(9) << record.grs_clock[0] << ' '  
       << setw(20) << setprecision(20) << elaptime-gpstime << ' '
       << setw(20) << setprecision(20) << elaptime-livetime << ' '
       << code << '\n';
}

class GDFTrackingDumper: public GDFRecordHandler
{
public:
  GDFTrackingDumper(const string& file): 
    m_filename(file), m_have_header(false) {} 

  virtual void run(GDFRecordDispatcher &dispatcher, 
		   const struct gdf_run_s& record);

  virtual void track(GDFRecordDispatcher &dispatcher, 
		     const struct gdf_track_s& record);

private:
  bool   m_have_header;
  double m_utc_start;
  string m_filename;
};

void 
GDFTrackingDumper::run(GDFRecordDispatcher &dispatcher, 
		       const struct gdf_run_s& record)
{
  if(record.sid_length[0] <= 2)dispatcher.stopProcessing();
  m_utc_start = record.utc_start;
  m_have_header = true;
}

void 
GDFTrackingDumper::track(GDFRecordDispatcher &dispatcher, 
			 const struct gdf_track_s& record)
{
  if(m_have_header)
    cout 
      << m_filename << ' '
      <<setiosflags(ios::fixed)
      << setw(11) << setprecision(5) << m_utc_start << ' '
      << setw(14) << setprecision(8) << record.utc       << ' '
      << setw(6) << setprecision(4) << record.azimuth   << ' '
      << setw(6) << setprecision(4) << record.elevation << ' '
      << setw(6) << setprecision(4) << record.stl << endl;
}

int main(int argc,char **argv)
{
  try
    {
      bool s_run    = true;
      bool s_ev10   = false;
      bool s_track  = false;
      bool s_hv     = false;

      bool dump_times    = false;
      bool dump_tracking = false;
      
      while (1)
	{
	  int this_option_optind = optind ? optind : 1;
	  int option_index = 0;
	  
	  static struct option long_options[] =
	    {
	      {"run",   0, 0, 1},
	      {"event",  0, 0, 2},
	      {"track", 0, 0, 3},
	      {"hv",    0, 0, 4},
	      {"tracking",    0, 0, 6},
	      {"times",    0, 0, 5},
	      {"help",  0, 0, 'h'},
	      {0, 0, 0, 0}
	    };
	  
	  int c=getopt_long_only(argc, argv, "h", long_options, 
				 &option_index);
	  
	  if(c==-1)break;
	  
	  switch(c)
	    {
	    case 1:
	      s_run = (s_run==false)?true:false;
	      break;
	      
	    case 2:
	      s_ev10 = (s_ev10==false)?true:false;
	      break;
	      
	    case 3:
	      s_track = (s_track==false)?true:false;
	      break;
	      
	    case 4:
	      s_hv = (s_hv==false)?true:false;
	      break;

	    case 5:
	      dump_times = (dump_times==false)?true:false;
	      break;

	    case 6:
	      dump_tracking = (dump_tracking==false)?true:false;
	      break;
	      
	    case 'h':
	      cout << "Usage: " << *argv << " [options] filename" << endl
		   << endl 
		   << "Options: " << endl
		   << "    -run    Toggle printing of run info       [default=on]" << endl
		   << "    -event  Toggle printing of event info     [default=off]" << endl
		   << "    -track  Toggle printing of tracking info  [default=off]" << endl
		   << "    -hv     Toggle printing of HV info        [default=off]" << endl
		;
	      exit(EXIT_SUCCESS);
	      
	    case '?':
	    default:
	      exit(EXIT_FAILURE);
	    }
	}
       
      char* progname=*argv;
      argc-=optind;
      argv+=optind;

      GDFSystem gdf;

      GDFRecordHandler* handler;

      if(dump_tracking)
	{
	  handler = new GDFTrackingDumper(*argv);
	}
      else if(dump_times)
	{
	  handler = new GDFTimesDumper();
	}
      else
	{
	  handler = new GDFRecordPrinter(s_run,s_ev10,s_track,s_hv);
	}

      GDFRecordDispatcher dispatcher(handler);
      
      if(argc!=0)
	dispatcher.process(*argv);
    }
  catch(const Error& x)
    {
      cerr << x;
    }
}
