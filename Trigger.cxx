#include <vector>

#include "Trigger.h"
#include "Exceptions.h"

static inline void swap(double& a, double& b)
{
  double s=b;
  b=a;
  a=s;
}

///////////////////////////////////////////////////////////////////////////////
////////////////////// Trigger MUL - Multiplicity Trigger /////////////////////
///////////////////////////////////////////////////////////////////////////////

bool NS_Analysis::MUL::trigger(const vector<double>& signal) const
{
  if(signal.size()<camera()->ntrigger())
    throw Error("MUL::trigger: Not enough signal tubes");
  
  unsigned int N=0;
  for(unsigned int c=0;c<camera()->ntrigger();c++)
    if((signal[c]>=level())&&(++N==multiplicity()))return true;
  return false;
}

double NS_Analysis::MUL::maxThreshold(const vector<double>& signal) const
{
  if(signal.size()<camera()->ntrigger())
    throw Error("MUL::maxThreshold: Not enough signal tubes");
  
  if(multiplicity()==1)
    {
      double max1=0;
      for(unsigned int c=0;c<camera()->ntrigger();c++)
	if(signal[c]>max1)max1=signal[c];
      return max1;
    }
  else if(multiplicity()==2)
    {
      double max1=0;
      double max2=0;
      for(unsigned int c=0;c<camera()->ntrigger();c++)
	if(signal[c]>max2)
	  {
	    max2=signal[c];
	    if(max2>max1)swap(max1,max2);
	  }
      return max2;
    }
  else if(multiplicity()==3)
    {
      double max1=0;
      double max2=0;
      double max3=0;
      for(unsigned int c=0;c<camera()->ntrigger();c++)
	if(signal[c]>max3)
	  {
	    max3=signal[c];
	    if(max3>max2)
	      {
		swap(max3,max2);
		if(max2>max1)swap(max2,max1);
	      }
	  }
      return max3;
    }
  else
    {
      throw Error("MUL::maxThreshold: not implemented for multiplicity>3");
    }
}


///////////////////////////////////////////////////////////////////////////////
//////////////////// Trigger PST - Pattern Selection Trigger //////////////////
///////////////////////////////////////////////////////////////////////////////

bool NS_Analysis::PST::trigger(const vector<double>& signal) const
{
  if(signal.size()<camera()->ntrigger())
    throw Error("PST::trigger: Not enough signal tubes");
  
  if(multiplicity()==1)
    {
      for(unsigned int c=0;c<camera()->ntrigger();c++)
	if(signal[c]>=level())return true;
      return false;
    }
  else if(multiplicity()==2)
    {
      for(unsigned int c=0;c<camera()->ntrigger();c++)if(signal[c]>=level())
	{
	  const ChannelDescription& channel=camera()->channel(c);
	  for(unsigned int j=0;j<channel.numneighbors();j++)
	    {
	      unsigned int nc=channel.neighbor(j);
	      if((nc<c)&&(signal[nc]>=level()))return true;
	    }
	}
      return false;
    }
  else if(multiplicity()==3)
    {
      for(unsigned int c=0;c<camera()->ntrigger();c++)if(signal[c]>=level())
	{
	  const ChannelDescription& channel=camera()->channel(c);
	  unsigned int N=1;
	  for(unsigned int j=0;j<channel.numneighbors();j++)
	    {
	      unsigned int nc=channel.neighbor(j);
	      if((nc<camera()->ntrigger())&&(signal[nc]>=level()))if(++N==3)
		return true;
	    }
	}
      return false;
    }
  else
    {
      throw Error("PST::trigger: not implemented for multiplicity>3");
    }  
}

double NS_Analysis::PST::maxThreshold(const vector<double>& signal) const
{
  if(signal.size()<camera()->ntrigger())
    throw Error("PST::maxThreshold: Not enough signal tubes");

  if(multiplicity()==1)
    {
      double max=0;
      for(unsigned int c=0;c<camera()->ntrigger();c++)
	if(signal[c]>max)max=signal[c];
      return max;
    }
  else if(multiplicity()==2)
    {
      double max2=0;
      for(unsigned int c=0;c<camera()->ntrigger();c++)if(signal[c]>max2)
	{
	  const ChannelDescription& channel=camera()->channel(c);
	  double lmax1=0;
	  for(unsigned int j=0;j<channel.numneighbors();j++)
	    {
	      unsigned int nc=channel.neighbor(j);
	      if((nc<camera()->ntrigger())&&(signal[nc]>lmax1))lmax1=signal[nc];
	    }
	  if((lmax1>signal[c])&&(signal[c]>max2))max2=signal[c];
	  else if((lmax1<=signal[c])&&(lmax1>max2))max2=lmax1;
	}
      return max2;
    }
  else if(multiplicity()==3)
    {
      double max3=0;
      for(unsigned int c=0;c<camera()->ntrigger();c++)if(signal[c]>max3)
	{
	  const ChannelDescription& channel=camera()->channel(c);
	  double lmax1=0;
	  double lmax2=0;
	  for(unsigned int j=0;j<channel.numneighbors();j++)
	    {
	      unsigned int nc=channel.neighbor(j);
	      if((nc<camera()->ntrigger())&&(signal[nc]>lmax2))
		{
		  lmax2=signal[nc];
		  if(lmax2>lmax1)swap(lmax1,lmax2);
		}
	    }
	  if((lmax2>signal[c])&&(signal[c]>max3))max3=signal[c];
	  else if((lmax2<=signal[c])&&(lmax2>max3))max3=lmax2;
	}
      return max3;
    }
  else
    {
      throw Error("PST::maxThreshold: not implemented for multiplicity>3");
    }
}
