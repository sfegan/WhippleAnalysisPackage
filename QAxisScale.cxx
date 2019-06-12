#include<cmath>

#include"QAxisScale.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////////////// QLinearAxisTics ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include<iostream>
NS_Analysis::QLinearAxisTics::
QLinearAxisTics(const QAxisScale* a, unsigned int nb, unsigned int ns):
  QAxisTics(), first_tic(0), delta_tic(0)
{
  double min=a->worldMin();
  double max=a->worldMax();

  double scale=abs(max-min)/double(nb);
  scale=pow(10.0,ceil(log10(scale)));

  // Check whether we could accomodate more big tics
  unsigned int san=int(ceil(abs(max-min)/scale));

  nsub=10;
  if(san >= nb*5)scale*=5.0,nsub=5;
  else if(san >= nb*2)scale*=2.0,nsub=2;
  else if(nb >= san*5)scale/=5.0,nsub=2;
  else if(nb >= san*2)scale/=2.0,nsub=5;

  // Round the min and max towards each other to next scale unit
  int roundedmin=int((max>min)?ceil(min/scale):floor(min/scale));
  int roundedmax=int((max<min)?ceil(max/scale):floor(max/scale));

  nbig=int(roundedmax-roundedmin)+1;
  first_tic=double(roundedmin)*scale;
  delta_tic=scale;
}

double
NS_Analysis::QLinearAxisTics::
tic(unsigned int tn, unsigned int sn) const
{
  return first_tic+(double(tn)+double(sn)/double(nsub))*delta_tic;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////////////// QLinearAxisScale ///////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

double 
NS_Analysis::QLinearAxisScale::
worldFTransform(double w) const
{
  return w;
}

double 
NS_Analysis::QLinearAxisScale::
worldRTransform(double w) const
{
  return w;
}

double 
NS_Analysis::QLogAxisScale::
worldFTransform(double w) const
{
  return log(w);
}

double 
NS_Analysis::QLogAxisScale::
worldRTransform(double w) const
{
  return exp(w);
}
