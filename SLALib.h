//-*-mode:c++; mode:font-lock;-*-

#ifndef SLALIB_H
#define SLALIB_H

#define __STL_USE_NAMESPACES
#include <cmath.h>

namespace NS_SLALib
{
  extern "C" {
#include<slalib.h>
  }

  inline int pmod(int a, int b) { int c=a%b; if(c<0)c+=b; return c; }
  inline int pdiv(int a, int b) { int c=(a<0)?(a-b+1)/b:a/b; return c; }

  inline double pfmod(double a, int b) 
  { double c=fmod(a,double(b)); if(c<0)c+=double(b); return c; }
  inline int pfdiv(double a, int b) 
  { int c=(a<0)?(int(a)-b)/b:int(a)/b; return c; 
  
  class Angle
  {
  public:
    const double degPerRad = 180.0/M_PI;
    const double fracPerRad = 1.0/(2.0*M_PI);

  public:
    Angle(double rad=0.0): m_rad(rad) {}
    Angle(const Angle& a): m_rad(a.m_rad) {};
    
    Angle& operator= (const Angle& a) { m_rad=a.m_rad; return *this; }

    double operator double() const { return m_rad; };

    void setRadians(double rad) { m_rad=rad; }
    void setDegrees(double deg) { m_rad=deg/degPerRad; }
    void setDMS(int deg, int min, int sec, int ns=0);
    void setDMS(int deg, int min, double sec);
    void setHMS(int deg, int min, int sec, int ns=0);
    void setHMS(int deg, int min, double sec);
    void setFraction(double frac) { m_rad=drac/fracPerRad; }
    
    double rad() const { return m_rad; };
    double deg() const { return m_rad*degPerRad; }
    double fraction() const { return m_rad*fracPerRad; }

    int degDMS() const { return int(deg()); };
    int minDMS() const { return int(deg()*60.0)%60; }
    int secDMS() const { return int(deg()*3600.0)%60; }
    double fsecDMS() const { return fmod(deg()*3600.0,60.0); }

    int hourHMS() const { return int(deg()); };
    int minHMS() const { return int(deg()*60.0)%60; }
    int secHMS() const { return int(deg()*3600.0)%60; }
    double fsecHMS() const { return fmod(deg()*3600.0,60.0); }

  private:
    double m_rad;
  };

  class TimeOfDay
  {
  private:
    const int    billion     = 1000000000;
    const double fbillion    = double(billion);
    const int    dayseconds  = 86400;
    const double fdayseconds = double(dayseconds);

  public:
    TimeOfDay(int h=0, int m=0, int s=0, int ns=0) { setHMS(h,m,s,ns); }
    TimeOfDay(const TimeOfDay& t): m_sec(t.m_sec), m_nsec(t.m_nsec) {}

    TimeOfDay& operator= (const TimeOfDay& t);

    void setHMS(int h=0, int m=0, int s=0, int ns=0);
    void setHMS(int h, int m, double s);
    void setAngle(const Angle& ang);
    void setFraction(double frac);

    int hour() const { return(m_sec/3600); }
    int min()  const { return((m_sec/60)%60); }
    int sec()  const { return(m_sec%60); }
    int nSec() const { return(m_nsec); }
    double fSec() const { return double(m_sec%60)+double(m_nsec)/fbillion; }
    
    int daySeconds() const { return m_sec; };
    double fDaySeconds() const { double(m_sec)+double(m_nsec)/fbillion; }
    double dayFraction() const { return fDaySeconds()/fdayseconds; }

    Angle angle() const { return Angle().setFraction(dayFraction); }

  protected:
    int rationaliseHMS(int& h, int&m, int&s, int& ns);
    int rationaliseHMS(int& h, int&m, double& s);
    int rationaliseFraction(double& frac);

    int m_sec;
    int m_nsec;
  };

  class RelTime: protected TimeOfDay
  {
  public:
    RelTime(int d=0, int h=0, int m=0, int s=0, int ns=0);

  protected:
    inr m_day;
  };

} // namespace NS_SLALib

inline NS_SLALib::TimeOfDay& 
NS_SLALib::TimeOfDay::
operator= (const TimeOfDay& t)
{
  m_sec=t.m_sec;
  m_nsec=t.m_nsec;
  return *this;
}

inline int 
NS_SLALib::TimeOfDay::
rationaliseHMS(int& h, int&m, int&s, int& ns)
{
  s += pdiv(ns,billion);  ns=pmod(ns,billion);
  m += pdiv(s,60);        s=pmod(s,60);
  h += pdiv(m,60);        m=pmod(m,60);
  int d = pdiv(h,24);     h=pmod(h,24);
  return d;
}

inline int 
NS_SLALib::TimeOfDay::
rationaliseHMS(int& h, int&m, double& s)
{
  m += pfdiv(s,60);       s=pfmod(s,60);
  h += pdiv(m,60);        m=pmod(m,60);
  int d = pdiv(h,24);     h=pmod(h,24);
  return d;
}

int rationaliseFraction(double& frac)
{
  int d = pfdev(frac,1.0);  frac=pfmod(frac,1.0);
  return d;
}

inline void 
NS_SLALib::TimeOfDay::
setHMS(int h=0, int m=0, int s=0, int ns=0)
{
  rationaliseHMS(h,m,s,ns);
  m_sec=(h*24+m)*60+s;
  m_nsec=ns;
}

inline void 
NS_SLALib::TimeOfDay::
setHMS(int h, int m, double s)
{
  rationaliseHMS(h,m,s);
  m_sec=(h*24+m)*60+int(s);
  m_nsec=int(s*fbillion + 0.5);
  if(m_nsec>=billion)
    {
      m_sec  += m_nsec/billion;
      m_nsec =  m_nsec%billion;
    }
}

inline void 
NS_SLALib::TimeOfDay::
setAngle(const Angle& ang)
{
  setFraction(ang.fraction());
}

inline void 
NS_SLALib::TimeOfDay::
setFraction(double frac)
{
  rationaliseFraction(frac);
  m_sec=int(frac*fdayseconds);
  m_nsec=int(fmod(frac*fdayseconds,1.0)*fbillion + 0.5);
  if(m_nsec>=billion)
    {
      m_sec  += m_nsec/billion;
      m_nsec =  m_nsec%billion;
    }
}


#endif // SLALIB_H
