#include<cmath>
#include<memory>

extern "C" {
#include<slalib.h>
}

#include"TwoDimensionalHistGenerator.h"

using std::cerr;
using std::endl;

void 
NS_Analysis::TwoDimensionalHistGenerator::Results::
clear()
{
  m_2d.clear();
  m_radial.clear();
}

void 
NS_Analysis::TwoDimensionalHistGenerator::Results::
insert(const Results& r)
{
  m_2d.insert(r.twoDim());
  m_radial.insert(r.radial());
}

void
NS_Analysis::TwoDimensionalHistGenerator::
generate(ParamFile* pf, double ra, double dec, double mjd)
{
  m_file.clear();
  
  RedHeader rh;
  pf->header()->read(0,&rh,1);

  m_ra  = ra;
  m_dec = dec;
  m_mjd_start = mjd;

  ParamFileEventProcessor proc(this,m_pb);
  proc.run(pf);

  m_total.insert(m_file);
}

void
NS_Analysis::TwoDimensionalHistGenerator::
operateOnEvent(int evno, const EventInfo* ei, const HillasParam* hp)
{
  //////////////////////////////// CHANGE ME //////////////////////////////////

  const double longitude = 110.879/180.0*M_PI;
  const double latitude  = 31.6805/180.0*M_PI;

  double UTC             = m_mjd_start + ei->time()/86400.0;
  double siderealTime    = slaDranrm(slaGmst(UTC) - longitude);
  double hourAngle       = slaDranrm(siderealTime - m_ra);
  double derotationAngle = -slaPa(hourAngle,m_dec,latitude);
  
  /////////////////////////////////////////////////////////////////////////////

  m_parameterize->parameterize(derotationAngle, hp,
			       &m_file.twoDim(), &m_file.radial());

  if(evno == 0)
    {
      double stHrs=siderealTime/M_PI*12.0;
      cerr << "EVENT 1 INFO";
      cerr << " DEC:   " <<  m_dec/M_PI*180.0 << endl;
      cerr << " RA:    " << m_ra/M_PI*180.0 << endl;
      cerr << " MJD:   " << UTC << endl;
      cerr << " HA:    " << hourAngle/M_PI*180.0 << endl;
      cerr << " LST:   " << int(stHrs) << ':' << int(stHrs*60.0)%60 << endl;
      cerr << " Derot: " << derotationAngle/M_PI*180.0 << endl;
    }
  
  return;
}
