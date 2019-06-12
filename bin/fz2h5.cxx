//-*-mode:c++; mode:font-lock;-*-
#define BOGUS

#define _ISOC99_SOURCE // long long functions (llabs etc..)

#include <unistd.h>
#include <getopt.h>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cstdlib>

#ifndef __USE_BSD
#define __USE_BSD
#endif

#include <cmath>

#include <Types.h>
#include <RedHeader.h>
#include <RedEvent.h>
#include <VSFA.h>
#include <RedFile.h>
#include <Exceptions.h>
#include <ScriptParser.h>
#include <UtilityFunctions.h>

#include "fz.h"

using NS_Analysis::MPtr;
using NS_Analysis::RedHeader;
using NS_Analysis::RedEvent;
using NS_Analysis::Error;
using NS_Analysis::FileError;
using NS_Analysis::RedFile;

using NS_Analysis::QLScriptParser;
using NS_Analysis::ScriptItemHandler;

using NS_Analysis::getFilenameBase;
using NS_Analysis::fixDate2000;

using namespace std;

#ifndef BOGUS

class GDFtoH5_Pass1: public GDFRecordHandler
{
public:
  GDFtoH5_Pass1(): GDFRecordHandler(), m_eventtime(), m_ncode1(0), m_ncode8(0) 
  {}
  ~GDFtoH5_Pass1();

  virtual void ev10(GDFRecordDispatcher &dispatcher, 
		    const struct gdf_ev10_s& record);
  
  virtual void starting_file(GDFRecordDispatcher &dispatcher,
			     const string& filename);

  virtual void finished_file(GDFRecordDispatcher &dispatcher,
			     const string& filename);

private:
  typedef struct { unsigned int msb; unsigned int lsb; } TimeStruct;
  typedef struct 
  {
    TimeStruct etstime;
    TimeStruct ltstime;
    TimeStruct grstime;
    int code;
  } EventTime;

  vector<EventTime> m_eventtime;

  unsigned int      m_ncode1;
  unsigned int      m_ncode8;
};

GDFtoH5_Pass1::~GDFtoH5_Pass1()
{

}

void GDFtoH5_Pass1::starting_file(GDFRecordDispatcher &dispatcher,
				  const string& filename)
{
  cout << "Pass 1: Reading in event times from file " << filename << endl;
}

static inline long long make_time(int msb, int lsb)
{
  return msb*10000000LL + lsb;
}

static inline long long make_time(int msb, int lsb, double rate)
{
  lsb = int(floor(double(lsb)/rate + 0.5));
  return msb*10000000LL + lsb;
}

void GDFtoH5_Pass1::finished_file(GDFRecordDispatcher &dispatcher,
				  const string& filename)
{

  cout << "Pass 1: Found " << m_eventtime.size() << " event" 
       << (m_eventtime.size() == 1 ? "":"s") << endl;

  // --------------------------------------------------------------------------
  // STEP 1 - CALIBRATE THE OSCILLATOR FROM THE PEDESTAL EVENTS
  // --------------------------------------------------------------------------

  unsigned int       n_peds      = 0;
  long double        delta_sum   = 0;
  long double        delta_sumsq = 0;
  
  long long last_ped = 0;
  cout << "Pass 1: Calibrating oscillator using Pedestal events" << endl;
  for(vector<EventTime>::iterator i = m_eventtime.begin();
      i != m_eventtime.end(); i++)
    if(i->code == 1)
      {
	long long this_ped = 
	  make_time(i->etstime.msb, i->etstime.lsb);

	if(last_ped != 0)
	  {
	    long long delta_t = this_ped - last_ped;
	    if((delta_t > 0)&&(delta_t < 15000000LL))
	      {
		delta_sum   += delta_t;
		delta_sumsq += delta_t*delta_t;
		n_peds++;
	      }
	  }
	last_ped = this_ped;
      }

  double delta_bar = double(delta_sum / n_peds);
  double delta_var = double(delta_sumsq / n_peds) - delta_bar*delta_bar;
  double delta_dev = sqrt(delta_var);
  
  double ped_oscfreq = delta_bar;

  cout << "Pass 1: Oscillator freq " << setprecision(10) << delta_bar << " Hz"
       << " (jitter +/- " << setprecision(5) << delta_dev << " Hz)" << endl;

  // --------------------------------------------------------------------------
  // STEP 2 - CALIBRATE THE OSCILLATOR PURELY FROM THE GPS CLOCK DIFFERENCES
  // --------------------------------------------------------------------------

  // The GRS clock and the elapsed time scaler both count the same
  // 10MHz oscillator. The scaler just counts it blindly from start to
  // finish. The GRS clock uses the GPS 1 pulse-per-second output to
  // clear the 10MHz register and advance the "seconds" register. It
  // is in this time, the beginning of a new second, that the GRS and
  // elapsed time scaler diverge... the GRS gets advanced/retarded the
  // el. scaler does not. So we look for the clicking over of a new
  // second and accumulate the time difference between the two clocks
  // from the event BEFORE the new sec to the SECOND event AFTER the
  // new sec. This is made more difficult by two things... one that
  // the GRS clock is sometimes read out after the 10MHz register is
  // cleared but before the second register is advanced (so the clock
  // seems to move back ~1 sec) - this is why we choose the SECOND
  // event AFTER the new sec. Also the elapsed time scaler is prone to
  // having some bits stick so we ignore time differences > 100 ticks
  // (thus effectively requiring the oscillator to be in the range
  // 9,999,900 to 10,000,100 Hz which it is (at least between 1998 and
  // 2002).
  
  // All things being equal we could have just measured the time of
  // the run with the GRS and elapsed time scalar then just divided
  // them, but weird things can happen in runs, like the scalars
  // getting reset etc.. this is a little bit more robust (I think).

  long long          elap_sum = 0;
  long long           grs_sum = 0;

  long long          last_ets     = 0;
  long long          last_grs     = 0;
  unsigned int       last_grs_sec = 0;
  bool               found_sec    = false;
  int                n_sec_found  = 0;

  cout << "Pass 1: Calibrating oscillator using GRS clock differences" << endl;
  for(vector<EventTime>::iterator i = m_eventtime.begin();
      i != m_eventtime.end(); i++)
    {
      long long this_ets = 
	make_time(i->etstime.msb, i->etstime.lsb);
      long long this_grs = 
	make_time(i->grstime.msb, i->grstime.lsb);

      if(found_sec)
	{
	  long long delta_ets = this_ets  - last_ets;
	  long long delta_grs = this_grs  - last_grs;

	  if(llabs(delta_ets - delta_grs)<24LL)
	    {
	      elap_sum += delta_ets;
	      grs_sum  += delta_grs;
	      n_sec_found++;
	    }
	  //	  else cout << delta_ets << ' ' << delta_grs << ' ' << delta_ets-delta_grs << endl;
	  
	  found_sec = false;
	  last_ets  = this_ets;
	  last_grs  = this_grs;
	  last_grs_sec = i->grstime.msb;
	}
      else if((i->grstime.msb == last_grs_sec+1) || 
	      ((i->grstime.lsb == 0)&&(i->grstime.msb == last_grs_sec)))
	{
	  // GRS clicked over to new second
	  found_sec = true;
	}
      else
	{
	  last_ets  = this_ets;
	  last_grs  = this_grs;
	  last_grs_sec = i->grstime.msb;
	}
    }
  
  double grs_oscfreq = 10000000.0 - double(grs_sum - elap_sum)/n_sec_found;
  
  cout << "Pass 1: Oscillator freq " << setprecision(10) << grs_oscfreq 
       << " Hz (" << n_sec_found << " samples)" << endl;


  // --------------------------------------------------------------------------
  // STEP 3 - CHECK FOR STUCK BITS
  // --------------------------------------------------------------------------

  cout << "Pass 1: Checking for stuck bits in GRS, elapsed and live scalars"
       << endl;

  unsigned int gr_msb_bitsc[11];
  unsigned int gr_lsb_bitsc[24];
  unsigned int el_msb_bitsc[11];
  unsigned int el_lsb_bitsc[24];
  unsigned int li_msb_bitsc[11];
  unsigned int li_lsb_bitsc[24];
  unsigned int no_lsb_bitsc[24];
  
  for(int i=0; i<11; i++)gr_msb_bitsc[i]=el_msb_bitsc[i]=li_msb_bitsc[i]=0;
  for(int i=0; i<24; i++)gr_lsb_bitsc[i]=el_lsb_bitsc[i]=li_lsb_bitsc[i]=no_lsb_bitsc[i]=0;

  for(vector<EventTime>::iterator i = m_eventtime.begin();
      i != m_eventtime.end(); i++)
    {
      if(i->code != 8)continue;

      unsigned int dongle = 0x00000001 << 10;
      for(int j=0; j<11; j++)
	{
	  if(i->grstime.msb & dongle)gr_msb_bitsc[j]++;
	  if(i->etstime.msb & dongle)el_msb_bitsc[j]++;
	  if(i->ltstime.msb & dongle)li_msb_bitsc[j]++;
	  dongle >>= 1;
	}

      dongle = 0x00000001 << 23;
      for(int j=0; j<24; j++)
	{
	  if(i->grstime.lsb & dongle)gr_lsb_bitsc[j]++;
	  if(i->etstime.lsb & dongle)el_lsb_bitsc[j]++;
	  if(i->ltstime.lsb & dongle)li_lsb_bitsc[j]++;
	  dongle >>= 1;
	}
    }

  for(int j=0; j<24; j++)
    {
      // This basically works out the number of cases (between 0 and
      // 9999999) that each bit is set, so we can work out whether
      // something screwy is happening with any of the bits.

      unsigned int zongle = 0xFFFFFFFF << 24-j;
      unsigned int dongle = 0x00000001 << 23-j;
      unsigned int tenmillion = 10000000;
      
      no_lsb_bitsc[j] = ((tenmillion&zongle)>>1) + 
	((tenmillion&dongle)?(tenmillion&(dongle-1)):0);
    }

#if 0
  cout << "Pass 1: GRS MSB ";
  for(int i=0; i<11; i++)cout << setw(2) << (gr_msb_bitsc[i]*100+m_ncode8/2)/m_ncode8 << ' ';
  cout << endl;

  cout << "Pass 1: ETS MSB ";
  for(int i=0; i<11; i++)cout << setw(2) << (el_msb_bitsc[i]*100+m_ncode8/2)/m_ncode8 << ' ';
  cout << endl;

  cout << "Pass 1: LTS MSB ";
  for(int i=0; i<11; i++)cout << setw(2) << (li_msb_bitsc[i]*100+m_ncode8/2)/m_ncode8 << ' ';
  cout << endl;
#endif

  cout << "Pass 1: BIT NUM 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0" << endl;
  cout << "Pass 1: GRS LSB ";
  for(int i=0; i<24; i++)cout << setw(2) << (gr_lsb_bitsc[i]*100+m_ncode8/2)/m_ncode8 << ' ';
  cout << endl;

  cout << "Pass 1: ETS LSB ";
  for(int i=0; i<24; i++)cout << setw(2) << (el_lsb_bitsc[i]*100+m_ncode8/2)/m_ncode8 << ' ';
  cout << endl;

  cout << "Pass 1: LTS LSB ";
  for(int i=0; i<24; i++)cout << setw(2) << (li_lsb_bitsc[i]*100+m_ncode8/2)/m_ncode8 << ' ';
  cout << endl;

  cout << "Pass 1: NOMINAL ";
  for(int i=0; i<24; i++)cout << setw(2) << (no_lsb_bitsc[i]+50000)/100000 << ' ';
  cout << endl;

  cout << "Pass 1: Likely stuck bits (>3 sigma deviation for expected counts):"
       << endl;

  for(int i=0; i<24; i++)
    {
      unsigned counts  = gr_lsb_bitsc[23-i];
      unsigned nominal = (m_ncode8*(no_lsb_bitsc[23-i]/1000))/10000;
      unsigned sigma   = int(floor(sqrt(nominal)));
      unsigned level   = 3*sigma;
      
      if((counts < nominal-level) || (counts > nominal+level))
	{
	  cout << "Pass 1:    GRS bit " << setw(2) << i
	       << " was on " << setw(5) << counts 
	       << " times ("
	       << (gr_lsb_bitsc[i]*100+m_ncode8/2)/m_ncode8 
	       << "%), expected " << setw(5) << nominal
	       << " -- " << setiosflags(ios::fixed|ios::showpos) 
	       << setprecision(1) 
	       << (double(counts)-double(nominal))/double(sigma) 
	       << resetiosflags(ios::floatfield|ios::showpos)
	       << " sigma" << endl;
	}
    }

  for(int i=0; i<24; i++)
    {
      unsigned counts  = el_lsb_bitsc[23-i];
      unsigned nominal = (m_ncode8*(no_lsb_bitsc[23-i]/1000))/10000;
      unsigned sigma   = int(floor(sqrt(nominal)));
      unsigned level   = 3*sigma;
      
      if((counts < nominal-level) || (counts > nominal+level))
	{
	  cout << "Pass 1:    ETS bit " << setw(2) << i
	       << " was on " << setw(5) << counts 
	       << " times ("
	       << (el_lsb_bitsc[i]*100+m_ncode8/2)/m_ncode8 
	       << "%), expected " << setw(5) << nominal
	       << " -- " << setiosflags(ios::fixed|ios::showpos)
	       << setprecision(1) 
	       << (double(counts)-double(nominal))/double(sigma) 
	       << resetiosflags(ios::floatfield|ios::showpos)
	       << " sigma" << endl;
	}
    }

  for(int i=0; i<24; i++)
    {
      unsigned counts  = li_lsb_bitsc[23-i];
      unsigned nominal = (m_ncode8*(no_lsb_bitsc[23-i]/1000))/10000;
      unsigned sigma   = int(floor(sqrt(nominal)));
      unsigned level   = 3*sigma;
      
      if((counts < nominal-level) || (counts > nominal+level))
	{
	  cout << "Pass 1:    LTS bit " << setw(2) << i
	       << " was on " << setw(5) << counts 
	       << " times ("
	       << (li_lsb_bitsc[i]*100+m_ncode8/2)/m_ncode8 
	       << "%), expected " << setw(5) << nominal
	       << " -- " << setiosflags(ios::fixed|ios::showpos) 
	       << setprecision(1) 
	       << (double(counts)-double(nominal))/double(sigma) 
	       << resetiosflags(ios::floatfield|ios::showpos)
	       << " sigma" << endl;
	}
    }

  // --------------------------------------------------------------------------
  // STEP 5 - FIND MEAN DEADTIME PER EVENT
  // --------------------------------------------------------------------------

  long long dead_time;

  {
  unsigned int       dead_cnt     = 0;
  long long          dead_sum     = 0;
  long long          dead_sumsq   = 0;
  long long          last_grs     = 0;
  long long          last_ets     = 0;
  long long          last_lts     = 0;
  unsigned int       last_grs_sec = 0;
  bool               skip_one     = true;

  vector<long long>  dead_times;

  for(vector<EventTime>::iterator i = m_eventtime.begin();
      i != m_eventtime.end(); i++)
    {
      long long this_grs = 
	make_time(i->grstime.msb, i->grstime.lsb);
      long long this_ets = 
	make_time(i->etstime.msb, i->etstime.lsb);
      long long this_lts = 
	make_time(i->ltstime.msb, i->ltstime.lsb);


      if(skip_one)
	{
	  skip_one = false;
	}
      else if((i->code != 8) ||
	      (i->grstime.msb != last_grs_sec)||(i->grstime.lsb == 0))
	{
	  skip_one = true;
	}
      else
	{
	  long long delta_grs = this_grs - last_grs;
	  long long delta_ets = this_ets - last_ets;
	  long long delta_lts = this_lts - last_lts;
	  
	  if(llabs(delta_ets - delta_grs)<30LL)
	    {
	      long long dead = delta_grs - delta_lts;
	      dead_cnt++;
	      dead_sum   += dead;
	      dead_sumsq += dead*dead;
	      dead_times.push_back(dead);
	    }
	}
      
      last_grs_sec = i->grstime.msb;
      last_grs = this_grs;
      last_ets = this_ets;
      last_lts = this_lts;
    }

  long long dead_bar = (dead_sum + dead_cnt/2) / dead_cnt;
  long long dead_var = (dead_sumsq - dead_bar*dead_sum + dead_cnt/2)/dead_cnt;
  long long dead_dev = (long long)(sqrt(dead_var));

  sort(dead_times.begin(), dead_times.end());

  long long dead_median;

  if(dead_cnt%2 == 0)
    dead_median = (dead_times[dead_cnt/2]+dead_times[dead_cnt/2 + 1])/2;
  else dead_median = dead_times[dead_cnt/2 + 1];

  dead_time = dead_median;
  
  cout << "Pass 1: Mean dead time / event " << dead_bar << " 10MHz ticks"
       << " (jitter +/- " << dead_dev << " ticks)" << endl;
  cout << "Pass 1: Median dead time / event " << dead_median << " 10MHz ticks"
       << endl;
  }

  // --------------------------------------------------------------------------
  // STEP 6 - CLOCK BIT DIFFERENCES
  // --------------------------------------------------------------------------
  
  {
  cout << "Pass 1: Clock bit differences" << endl;

  unsigned int       grs_ets[24];
  unsigned int       grs_lts[24];
  unsigned int       ets_lts[24];
  long long          last_grs     = 0;
  long long          last_ets     = 0;
  long long          last_lts     = 0;
  unsigned int       last_grs_sec = 0;
  bool               skip_one     = true;

  for(int i=0;i<24;i++)grs_ets[i]=grs_lts[i]=ets_lts[i]=0;

  for(vector<EventTime>::iterator i = m_eventtime.begin();
      i != m_eventtime.end(); i++)
    {
      long long this_grs = 
	make_time(i->grstime.msb, i->grstime.lsb);
      long long this_ets = 
	make_time(i->etstime.msb, i->etstime.lsb);
      long long this_lts = 
	make_time(i->ltstime.msb, i->ltstime.lsb);


      if(skip_one)
	{
	  skip_one = false;
	}
      else if((i->code != 8) ||
	      (i->grstime.msb != last_grs_sec)||(i->grstime.lsb == 0))
	{
	  skip_one = true;
	}
      else
	{
	  long long delta_grs = this_grs - last_grs;
	  long long delta_ets = this_ets - last_ets;
	  long long delta_lts = this_lts - last_lts;
	  
	  bool done;

	  done = false;
	  for(int j=0;j<24 && !done;j++)
	    {
	      unsigned int dongle = 0xC0000000 >> (31-j);
	      if(llabs(delta_grs-delta_ets) <= dongle)
		{
		  grs_ets[j]++;
		  done=true;
		}
	    }

	  done = false;
	  for(int j=0;j<24 && !done;j++)
	    {
	      unsigned int dongle = 0xC0000000 >> (31-j);
	      if(llabs(delta_grs-delta_lts-dead_time) <= dongle)
		{
		  grs_lts[j]++;
		  done=true;
		}
	    }

	  done = false;
	  for(int j=0;j<24 && !done;j++)
	    {
	      unsigned int dongle = 0xC0000000 >> (31-j);
	      if(llabs(delta_ets-delta_lts-dead_time) <= dongle)
		{
		  ets_lts[j]++;
		  done=true;
		}
	    }
	}
      
      last_grs_sec = i->grstime.msb;
      last_grs = this_grs;
      last_ets = this_ets;
      last_lts = this_lts;
    }

  int widths[24];
  for(int i=0;i<24;i++)
    {
      const int c[] = {0, 10, 100, 1000, 10000, 100000, 1000000};
      widths[i]=1;
      for(int j=0;j<7;j++)
	if((grs_ets[i]>=c[j])||(grs_lts[i]>=c[j])||(ets_lts[i]>=c[j]))
	  widths[i]=j+1;
    }

  cout << "Pass 1: GRS-ETS ";
  for(int i=0;i<20;i++)
    {
      if(i != 0)cout << ' ';
      cout << setw(widths[i]) << grs_ets[i];
    }
  cout << endl;

  cout << "Pass 1: GRS-LTS ";
  for(int i=0;i<20;i++)
    {
      if(i != 0)cout << ' ';
      cout << setw(widths[i]) << grs_lts[i];
    }
  cout << endl;

  cout << "Pass 1: ETS-LTS ";
  for(int i=0;i<20;i++)
    {
      if(i != 0)cout << ' ';
      cout << setw(widths[i]) << ets_lts[i];
    }
  cout << endl;

  }
  
  // --------------------------------------------------------------------------
  // STEP 7 - FIX THE CLOCKS
  // --------------------------------------------------------------------------

  cout << "Pass 1: Fixing known clock errors" << endl;

  // GRS NEW SECOND GLITCH
  {
  unsigned fixes                  = 0;
  long long          last_grs     = 0;
  unsigned int       last_grs_sec = 0;

  for(vector<EventTime>::iterator i = m_eventtime.begin();
      i != m_eventtime.end(); i++)
    {
      long long this_grs = 
	make_time(i->grstime.msb, i->grstime.lsb);

      if((this_grs < last_grs) && 
	 (i->grstime.msb == last_grs_sec)&&(i->grstime.lsb == 0))
	i->grstime.msb++, fixes++;
      last_grs_sec = i->grstime.msb;
      last_grs = this_grs;
    }
  
  cout << "Pass 1:    GRS new second glitch - fixed " << fixes 
       << " occurances" << endl;
  }

  // ETS bit problem
  {
  const unsigned int ignore_bits     = 6;
  const long long    threshold       = ~((~0LL) << (ignore_bits));
  const long long    rounder         = threshold >> 1;
  const long long    mask            = ~threshold;
  const long long    limiter         = 0x0000000000000001LL << (ignore_bits);

  const unsigned int max_flipbits    = 2;
  const unsigned int max_scanevents  = 5;

  //  cout << threshold << ' ' << mask << ' ' << rounder << endl;

  unsigned fixes           = 0;
  unsigned nofix_nscan     = 0;
  unsigned nofix_nbits     = 0;

  long long last_grs       = 0;
  long long last_ets       = 0;

  unsigned  nevent         = 0;

  for(vector<EventTime>::iterator i = m_eventtime.begin();
      i != m_eventtime.end(); i++)
    {
      long long this_grs = make_time(i->grstime.msb, i->grstime.lsb);
      long long this_ets = make_time(i->etstime.msb, i->etstime.lsb);
      nevent++;

      if(last_grs == 0)
	{
	  last_grs = this_grs;
	  last_ets = this_ets;
	  continue;
	}

      long long delta_grs = this_grs - last_grs;
      long long delta_ets = this_ets - last_ets;

      long long delta_delta = delta_ets - delta_grs;
      long long delta_delta_round = 
	(delta_delta>=0)?
	((delta_delta+rounder)&mask):((delta_delta+threshold-rounder)&mask);
      
      cout << nevent << '\t' 
	   << delta_delta << '\t' << delta_delta_round << '\t'
	   << endl;	      

      if(delta_delta_round != 0)
	//      if(llabs(delta_delta) >= limiter)
	{

	  int nbits = 0;
	  long long tocount = 
	    (delta_delta_round>=0)?(delta_delta_round):(~delta_delta_round)+1;

	  long long dongle = 0x0000000000000001LL;
	  for(int j=0; j<64; j++)
	    {
	      if(tocount & dongle)nbits++;
	      dongle <<= 1;
	    }

	  if(nbits > max_flipbits)
	    {
	      cout << "Pass 1:    ETS sticky bit glitch: " 
		   << "could not fix event " << nevent << " ("
		   << delta_delta << '/' << delta_delta_round << ") NBITS"
		   << endl;
	      nofix_nbits++;
	      last_grs = this_grs;
	      last_ets = this_ets;
	      continue;
	    }

	  int nscan = 0;
	  for(vector<EventTime>::iterator j = i+1;
	      j != m_eventtime.end(); j++)
	    {
	      if(nscan >= max_scanevents)break;

	      long long last_ets = this_ets - delta_delta_round;
	      long long last_grs = this_grs;

	      long long this_grs = make_time(j->grstime.msb, j->grstime.lsb);
	      long long this_ets = make_time(j->etstime.msb, j->etstime.lsb);

	      long long delta_grs = this_grs - last_grs;
	      long long delta_ets = this_ets - last_ets;
	      
	      long long delta_delta = delta_ets - delta_grs;
      long long delta_delta_round = 
	(delta_delta>=0)?
	((delta_delta+rounder)&mask):((delta_delta+threshold-rounder)&mask);
      //	      long long delta_delta_round = (delta_delta+rounder)&mask;
      cout << "  + " << nscan+1 << ' ' << delta_delta << ' ' << delta_delta_round << endl;	      
	      if(llabs(delta_delta) < limiter/2)break;
	      nscan++;
	    }

	  if(nscan >= max_scanevents)
	    {
	      cout << "Pass 1:    ETS sticky bit glitch: " 
		   << "could not fix event " << nevent << " ("
		   << delta_delta << '/' << delta_delta_round << ") NSCAN"
		   << endl;
	      nofix_nscan++;
	      last_grs = this_grs;
	      last_ets = this_ets;
	      continue;
	    }

	  //	  cout << nevent << " fixed" << endl;
	  this_ets -= delta_delta_round;
	  
	  i->etstime.msb = this_ets/10000000;
	  i->etstime.lsb = this_ets%10000000;
	  fixes++;
	}
      
      last_grs = this_grs;
      last_ets = this_ets;
    }

  cout << "Pass 1:    ETS sticky bit glitch - fixed " << fixes
       << " occurances. Could not fix " 
       << nofix_nscan << " (NSCAN) "
       << nofix_nbits << " (NBITS) " << endl;
  }

  // --------------------------------------------------------------------------
  // STEP 8 - CLOCK BIT DIFFERENCES AGAIN
  // --------------------------------------------------------------------------
  
  {
  cout << "Pass 1: Clock bit differences" << endl;

  unsigned int       grs_ets[24];
  unsigned int       grs_lts[24];
  unsigned int       ets_lts[24];
  long long          last_grs     = 0;
  long long          last_ets     = 0;
  long long          last_lts     = 0;
  unsigned int       last_grs_sec = 0;
  bool               skip_one     = true;

  for(int i=0;i<24;i++)grs_ets[i]=grs_lts[i]=ets_lts[i]=0;

  for(vector<EventTime>::iterator i = m_eventtime.begin();
      i != m_eventtime.end(); i++)
    {
      long long this_grs = 
	make_time(i->grstime.msb, i->grstime.lsb);
      long long this_ets = 
	make_time(i->etstime.msb, i->etstime.lsb);
      long long this_lts = 
	make_time(i->ltstime.msb, i->ltstime.lsb);


      if(skip_one)
	{
	  skip_one = false;
	}
      else if((i->code != 8) ||
	      (i->grstime.msb != last_grs_sec)||(i->grstime.lsb == 0))
	{
	  skip_one = true;
	}
      else
	{
	  long long delta_grs = this_grs - last_grs;
	  long long delta_ets = this_ets - last_ets;
	  long long delta_lts = this_lts - last_lts;
	  
	  bool done;

	  done = false;
	  for(int j=0;j<24 && !done;j++)
	    {
	      unsigned int dongle = 0xC0000000 >> (31-j);
	      if(llabs(delta_grs-delta_ets) <= dongle)
		{
		  grs_ets[j]++;
		  done=true;
		}
	    }

	  done = false;
	  for(int j=0;j<24 && !done;j++)
	    {
	      unsigned int dongle = 0xC0000000 >> (31-j);
	      if(llabs(delta_grs-delta_lts-dead_time) <= dongle)
		{
		  grs_lts[j]++;
		  done=true;
		}
	    }

	  done = false;
	  for(int j=0;j<24 && !done;j++)
	    {
	      unsigned int dongle = 0xC0000000 >> (31-j);
	      if(llabs(delta_ets-delta_lts-dead_time) <= dongle)
		{
		  ets_lts[j]++;
		  done=true;
		}
	    }
	}
      
      last_grs_sec = i->grstime.msb;
      last_grs = this_grs;
      last_ets = this_ets;
      last_lts = this_lts;
    }

  int widths[24];
  for(int i=0;i<24;i++)
    {
      const int c[] = {0, 10, 100, 1000, 10000, 100000, 1000000};
      widths[i]=1;
      for(int j=0;j<7;j++)
	if((grs_ets[i]>=c[j])||(grs_lts[i]>=c[j])||(ets_lts[i]>=c[j]))
	  widths[i]=j+1;
    }

  cout << "Pass 1: GRS-ETS ";
  for(int i=0;i<20;i++)
    {
      if(i != 0)cout << ' ';
      cout << setw(widths[i]) << grs_ets[i];
    }
  cout << endl;

  cout << "Pass 1: GRS-LTS ";
  for(int i=0;i<20;i++)
    {
      if(i != 0)cout << ' ';
      cout << setw(widths[i]) << grs_lts[i];
    }
  cout << endl;

  cout << "Pass 1: ETS-LTS ";
  for(int i=0;i<20;i++)
    {
      if(i != 0)cout << ' ';
      cout << setw(widths[i]) << ets_lts[i];
    }
  cout << endl;

  }
  
}

void 
GDFtoH5_Pass1::ev10(GDFRecordDispatcher &dispatcher, 
		       const struct gdf_ev10_s& record)
{
  EventTime etime;

  etime.ltstime.msb  = record.live_sec;
  etime.ltstime.lsb  = (record.live_ns+50)/100;    // GRANITE screws this up
  etime.etstime.msb  = record.elapsed_sec;
  etime.etstime.lsb  = (record.elapsed_ns+50)/100; // GRANITE screws this up

  int grs_day         = record.grs_clock[2];
  int grs_time        = record.grs_clock[1];
  int grs_sec         = (((grs_time&0x00F00000) >> 20) * 60*60*10 + // HRS
			 ((grs_time&0x000F0000) >> 16) * 60*60    + // HRS
			 ((grs_time&0x0000F000) >> 12) * 60*10    + // MINS
			 ((grs_time&0x00000F00) >> 8)  * 60       + // MINS
			 ((grs_time&0x000000F0) >> 4)  * 10       + // SECS
			 ((grs_time&0x0000000F) >> 0))            ; // SECS

  etime.grstime.msb   = grs_sec;
  etime.grstime.lsb   = record.grs_clock[0];

  int    trigger       = record.trigger;
  int     code         = 0;
  if     (trigger == 0x1)code=1; // Pedestal ONLY if bit 1 (and only bit 1) set
  else if(trigger == 0x6)code=8; // Event if bits 2&3 set (ONLY)
  else if(trigger == 0x4)code=8; // Event if bit 3 set (ONLY)
  else if(trigger == 0x2)code=8; // Event if bit 2 set (ONLY)
  else if(trigger &  0x8)code=2; // CFD test event
  else                   code=0; // UNKNOWN

  if(code == 8)m_ncode8++;
  else if(code == 1)m_ncode1++;

  etime.code           = code;

  m_eventtime.push_back(etime);
}

#endif

class GDFtoH5_Pass2: public GDFRecordHandler
{
public:
  GDFtoH5_Pass2(RedFile *rf, int date): m_rf(rf), m_date(date),
					m_got_one_event(false) { WARN(); }
  ~GDFtoH5_Pass2();
  
  virtual void ev10(GDFRecordDispatcher &dispatcher, 
		    const struct gdf_ev10_s& record);
  
  virtual void run(GDFRecordDispatcher &dispatcher, 
		   const struct gdf_run_s& record);

  virtual void track(GDFRecordDispatcher &dispatcher, 
		     const struct gdf_track_s& record);

private:
  RedHeader m_header;
  RedFile*  m_rf;
  int       m_date;

  double    m_live_start;
  double    m_live_end;

  bool      m_got_one_event;

  vector<struct gdf_track_s> m_tr_records;

  void WARN();
};

GDFtoH5_Pass2::~GDFtoH5_Pass2()
{
  m_header.setNEvents(m_rf->events()->size());
  m_header.setLiveTime(m_live_end-m_live_start);

  if(m_tr_records.size() == 0)
    {
      cerr << "Found NO tracking records in file, "
	"header will not be filled out correctly" << endl;
    }
  else
    {
      unsigned n = m_tr_records.size();
      cerr << "Found " << n << " tracking records" << endl;

      unsigned median_n = n/2;
      struct gdf_track_s record = m_tr_records[median_n];

      m_header.setMode("???");
      m_header.setSource(record.source);

      const double ra = record.rasc_today / (2*M_PI) * 24; // in hours
      const int ra_h = int(floor(ra));
      const int ra_m = int(floor((ra-ra_h)*60.0));
      const int ra_s = int(floor(((ra-ra_h)*60.0-ra_m)*60.0));
      m_header.setRA(ra_h*10000+ra_m*100+ra_s);

      const double dec = record.decl_today / (2*M_PI) * 360;
      const int dec_d = int(floor(dec));
      const int dec_m = int(floor((dec-dec_d)*60.0));
      const int dec_s = int(floor(((dec-dec_d)*60.0-dec_m)*60.0));
      m_header.setDec(dec_d*10000+dec_m*100+dec_s);

      const double ut = fmod(record.utc,1.0)*24.0;
      const int ut_h = int(floor(ut));
      const int ut_m = int(floor((ut-ut_h)*60.0));
      m_header.setUT(ut_h*100+ut_m);

      const double st = record.stl / (2*M_PI) *24.0;
      const int st_h = int(floor(st));
      const int st_m = int(floor((st-st_h)*60.0));
      m_header.setST(st_h*100+st_m);

      m_header.setAzimuth(record.azimuth);
      m_header.setElevation(record.elevation);
    }      

  m_rf->header()->write(0,&m_header,1);
}

void GDFtoH5_Pass2::WARN()
{
  cerr 
    << " ***********************************************************" << endl
    << " *                                                         *" << endl
    << " * fz2h5 --- this version is TEMPORARY and UNFINISHED. No  *" << endl
    << " *           timing or sanity checks have been implemented *" << endl
    << " *                                                         *" << endl
    << " *           BOGUS TIME INFORMATION COULD BE IN THE OUTPUT *" << endl
    << " *                                                         *" << endl
    << " ***********************************************************" << endl;
}

void 
GDFtoH5_Pass2::run(GDFRecordDispatcher &dispatcher, 
		      const struct gdf_run_s& record)
{
  ostringstream ostr;
  ostr << "gt" << record.run;
  m_header.setRunID(ostr.str());

  m_header.setNEvents(0);  // unknown for now
  m_header.setLiveTime(0); // unknown for now

  m_header.setSTDur(int(record.sid_length[0]));

  m_header.setMode("");
  m_header.setSource("");  // unknown for now

  int date = m_date; //record.idate;
  fixDate2000(date);
  m_header.setDate(date);

  m_header.setMJD(record.utc_start);

  m_header.setFRJD(0);     // who know what this is ?
  
  m_header.setRA(0);       // unknown for now
  m_header.setDec(0);      // unknown for now

  m_header.setUT(0);       // ??
  m_header.setST(0);       // ??

  m_header.setAzimuth(0);  // unknown for now
  m_header.setElevation(0);// unknown for now

  m_header.setSkyQ("N/A"); // phase this out
  m_header.setComms(record.comment);

  for(int i=0; i<7; i++)
    m_header.setGPSBeg(0,i); 
}

void 
GDFtoH5_Pass2::ev10(GDFRecordDispatcher &dispatcher, 
		    const struct gdf_ev10_s& record)
{
  int    nadc          = record.nadc;
  int    evnum         = record.event;
  int    ltstime_sec  = record.live_sec;
  int    ltstime_ns   = record.live_ns;
  int    elapsed_sec   = record.live_sec;
  int    elapsed_ns    = record.live_ns;
  int    npatterns     = record.ntrg;
  int    trigger       = record.trigger;
  int    grs_day       = record.grs_clock[2];
  int    grs_time      = record.grs_clock[1];
  int    grs_time_10ns = record.grs_clock[0];
  int    status        = record.status; // what is this ?

  double elapsed  = float(elapsed_sec)  + float(elapsed_ns)/1000000000.0;
  double ltstime = float(ltstime_sec) + float(ltstime_ns)/1000000000.0;
  double utc      = 
    float(((grs_time&0x00F00000) >> 20)*60*60*10 +
	  ((grs_time&0x000F0000) >> 16)*60*60 +
	  ((grs_time&0x0000F000) >> 12)*60*10 +
	  ((grs_time&0x00000F00) >> 8)*60 +
	  ((grs_time&0x000000F0) >> 4)*10 +
	  ((grs_time&0x0000000F) >> 0)) +
    float(grs_time_10ns)/100000000.0;
  
  // record.mark_gps
  // record.mark_open
  // record.mark_close
  // record.gate_open
  // record.gate_close

  // record.gps_mjd
  // record.gps_sec
  // record.gps_ns

  // record.gps_clock[0]
  // record.gps_clock[1]
  // record.gps_clock[2]
  
  // record.gps_status[0]
  // record.gps_status[0]

  // record.track[0]
  // record.track[1]

  RedEvent re;
  re.setVersion(nadc);

  int code=0;
  if     (trigger == 0x1)code=1; // Pedestal ONLY if bit 1 (and only bit 1) set
  else if(trigger == 0x6)code=8; // Event if bits 2&3 set (ONLY)
  else if(trigger == 0x4)code=8; // Event if bit 3 set (ONLY)
  else if(trigger == 0x2)code=8; // Event if bit 2 set (ONLY)
  else if(trigger &  0x8)code=2; // CFD test event
  else                   code=0; // UNKNOWN

  re.setCode(code);

  re.setTime(elapsed);
  re.setGPSUTC(utc);
  re.setLiveTime(ltstime);

  if((!m_got_one_event)||(ltstime < m_live_start))m_live_start=ltstime;
  if((!m_got_one_event)||(ltstime > m_live_end))m_live_end=ltstime;
  m_got_one_event = true;

  for(int i=0;i<nadc;i++)re.setADC(record.adc[i],i);

  m_rf->events()->appendOne(&re);
}

void GDFtoH5_Pass2::track(GDFRecordDispatcher &dispatcher, 
			  const struct gdf_track_s& record)
{
  m_tr_records.push_back(record);
}

//
//
//

class Fz2H5Engine: public ScriptItemHandler
{
public:
  virtual void n2(const string& n2_name, int n2_date);
  virtual void trk(const string& tr_name, int tr_date, 
		   const string& n2_name, int n2_date);
  virtual void pair(const string& on_name, int on_date,
		    const string& n2_name, int n2_date,
		    const string& pad_name, int pad_date);
  virtual ~Fz2H5Engine();

  Fz2H5Engine() {}
  
private:
  void processFile(const string& filename, int date);
};

void Fz2H5Engine::n2(const string& n2_name, int n2_date)
{
  processFile(n2_name,n2_date);
}

void Fz2H5Engine::trk(const string& tr_name, int tr_date, 
		       const string& n2_name, int n2_date)
{
  processFile(tr_name,tr_date);
}

void Fz2H5Engine::pair(const string& on_name, int on_date,
			const string& n2_name, int n2_date,
			const string& pad_name, int pad_date)
{
  processFile(on_name,on_date);
  processFile(pad_name,pad_date);
}

Fz2H5Engine::~Fz2H5Engine()
{
}

void Fz2H5Engine::processFile(const string& filename, int date)
{
  H5garbage_collect();

#ifdef BOGUS
  string ofilename = getFilenameBase(filename)+string(".h5");
  RedFile rf(ofilename,492);
  GDFtoH5_Pass2 converter(&rf,date);
#else
  GDFtoH5_Pass1 converter;
#endif
  GDFRecordDispatcher dispatcher(&converter);
  dispatcher.process(filename);
}

int main(int argc,char **argv)
{
  try
    {
      string scriptname;

      while (1)
	{
	  int this_option_optind = optind ? optind : 1;
	  int option_index = 0;
	  
	  static struct option long_options[] =
	    {
	      {"script", 1, 0, 's'},
	      {0, 0, 0, 0}
	    };
	  
	  int c=getopt_long_only(argc, argv, "s:", long_options, &option_index);
	  
	  if(c==-1)break;
	  
	  switch(c)
	    {
	    case 's':
	      scriptname=optarg;
	      break;
	    }
	}
      
      char* progname=*argv;
      argv++;
      argc--;
      
      if((argc!=2) && (scriptname==""))
      {
	cerr << "Usage: " << endl 
#ifndef BOGUS
	     << "   " << progname << " [options] --script scriptfile" << endl
	     << "or " << progname << " [options] filename utdate" << endl
#else
	     << "   " << progname << " [options] filename utdate" << endl
#endif
	     << endl;
	exit(EXIT_FAILURE);
      }
	 
      GDFSystem gdf;
      Fz2H5Engine fz2h5engine;
      
      if(scriptname != "")
	{
	  QLScriptParser parser(&fz2h5engine);
	  parser.parse(scriptname);
	}
      else
	{
	  int date;
          istringstream(*(argv+1)) >> date;
	  fz2h5engine.trk(*argv,date,"",-1);
	}
    }
  catch(const Error& x)
    {
      cerr << x;
    }
}
