#include<iostream>
#include<iomanip>
#include<sstream>

#include"ProgressBar.h"

using std::cerr;
using std::setw;
using std::setprecision;
using std::ostringstream;

NS_Analysis::ProgressBar::
~ProgressBar()
{
  // nothing to see here
}

void 
NS_Analysis::ProgressBar::
reset(int n)
{ 
  m_i=0; 
  m_n=n; 
  draw(); 
}

NS_Analysis::TextProgressBar::
~TextProgressBar()
{ 
  if(m_i < m_n)m_stream << '\n'; 
}

void 
NS_Analysis::TextProgressBar::
reset(int n)
{ 
  time(&m_start_time);
  ProgressBar::reset(n);
}

bool NS_Analysis::TextProgressBar::must_redraw(int i) 
{ 
  int oc=int(floor(double(m_i)/double(m_n)*double(m_len)));
  int nc=int(floor(double(i)/double(m_n)*double(m_len)));
  if(nc>oc)return true;
  time_t now=time(0);
  if(now > m_update_time+m_max_update_interval)return true;
  return false;
}

void NS_Analysis::TextProgressBar::draw()
{
  double frac_done=double(m_i)/double(m_n);

  time(&m_update_time);
  int elapsed = m_update_time-m_start_time;

  string clock;
  if(elapsed > 30)
    {
      bool rbi = true;
      ostringstream ost;
      int remaining = int(elapsed/frac_done - elapsed);
      if(remaining>3600)
	{
	  ost << int(remaining/3600) << "h ";
	  rbi=false;
	}
      
      remaining = remaining % 3600;
      if((rbi==false)||(remaining>60))
	{
	  ost << setw(2) << setprecision(2) << int(remaining/60) << "m ";
	  rbi=false;
	}
      
      remaining = remaining % 60;
      if((rbi==false)||(remaining))
	ost << setw(2) << setprecision(2) << int(remaining) << "s ";
      
      clock=ost.str();
    }

  unsigned int nc=int(floor(frac_done*double(m_len)));
  if(m_i >= m_n)m_stream << '\n';
  else if(clock.length())m_stream << string(1,'\r') << clock 
				  << string(((nc>clock.length())?nc-clock.length():0),'=');
  else m_stream << string(1,'\r') << string(nc,'=');
}
