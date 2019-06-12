#include <math.h>

#include "HillasParam.h"
#include "HillasParameterization.h"

NS_Analysis::HillasParameterization::
~HillasParameterization()
{

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/////////////////////////// Hillas Parameterization ///////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void 
NS_Analysis::HillasParameterization::
parameterize(HillasParam* param, const EventChannelReps* ecr,
	     double origin_x, double origin_y) const
{
  bool origin_shifted=false;
  if((origin_x != 0.0)||(origin_y != 0.0))origin_shifted=true;
  
  param->zero();
  
  const int nchannel=m_cam->nchannels();

  // Index of channels that are part of the image
  vector<channelnum_type> imagechannelindex;
  imagechannelindex.reserve(ecr->light().nchannels());
  
  ////////////////////////////////////////////////////////////////////////////
  ////////////////////// nimage - number of image tubes //////////////////////
  ////////////////////////////////////////////////////////////////////////////

  unsigned int nimage=0;
  for(int i=0;i<nchannel;i++)
    {
      if(ecr->clean(i).image())
	{
	  imagechannelindex.push_back(i);
	  nimage++;
	}
    }

  param->setNImage(nimage);
  if(nimage==0)return;

  ////////////////////////////////////////////////////////////////////////////
  /////// max 1,2,3 AND loc 1,2,3 - amount of light in brightest tubes ///////
  ////////////////////////////////////////////////////////////////////////////

  channelnum_type loc1=0;
  channelnum_type loc2=0;
  channelnum_type loc3=0;
  double max1=0;
  double max2=0;
  double max3=0;

  for(unsigned int i=0;i<nimage;i++)
    {
      channelnum_type ch=imagechannelindex[i];
      double si=ecr->light(ch);
      if(si>max1)
	{
	  max3=max2,  loc3=loc2;
	  max2=max1,  loc2=loc1;
	  max1=si,    loc1=ch;
	}
      else if(si>max2)
	{
	  max3=max2,  loc3=loc2;
	  max2=si,    loc2=ch;
	}
      else if(si>max3)
	{
	  max3=si,    loc3=ch;
	}
    }
  
  param->setMax1(max1);
  param->setMax2(max2);
  param->setMax3(max3);
  param->setLoc1(loc1);
  param->setLoc2(loc2);
  param->setLoc3(loc3);

  ////////////////////////////////////////////////////////////////////////////
  //////////////// first, second and third moments of the image //////////////
  ////////////////////////////////////////////////////////////////////////////

  double sumsig=0;
  double sumxsig=0;
  double sumysig=0;
  double sumx2sig=0;
  double sumy2sig=0;
  double sumxysig=0;
  double sumx3sig=0;
  double sumy3sig=0;
  double sumx2ysig=0;
  double sumxy2sig=0;

  for(unsigned int j=0;j<nimage;j++)
    {
      const channelnum_type i=imagechannelindex[j];
      double xi=m_cam->channels()[i].x();
      double yi=m_cam->channels()[i].y();
      const double si=ecr->light(i);
      
      if(origin_shifted)
	{
	  xi -= origin_x;
	  yi -= origin_y;
	}

      sumsig += si;

      const double sixi=si*xi;
      const double siyi=si*yi;

      sumxsig += sixi;
      sumysig += siyi;

      const double sixi2=sixi*xi;
      const double siyi2=siyi*yi;
      const double sixiyi=sixi*yi;

      sumx2sig += sixi2;
      sumy2sig += siyi2;
      sumxysig += sixiyi;
      
      sumx3sig += sixi2*xi;
      sumy3sig += siyi2*yi;
      sumx2ysig += sixi2*yi;
      sumxy2sig += siyi2*xi;
    }

  const double xmean = sumxsig / sumsig;
  const double ymean = sumysig / sumsig;
  
  const double x2mean = sumx2sig / sumsig;
  const double y2mean = sumy2sig / sumsig;
  const double xymean = sumxysig / sumsig;
  
  const double xmean2 = xmean * xmean;
  const double ymean2 = ymean * ymean;
  const double meanxy = xmean * ymean;
  
  const double sdevx2 = x2mean - xmean2;
  const double sdevy2 = y2mean - ymean2;
  const double sdevxy = xymean - meanxy;

  ////////////////////////////////////////////////////////////////////////////
  ///////////////// image centroid, origin of axes and size //////////////////
  ////////////////////////////////////////////////////////////////////////////

  const double dist    = sqrt(xmean2+ymean2);

  param->setSize(sumsig);

  param->setX0(origin_x);
  param->setY0(origin_y);
  
  param->setXC(xmean);
  param->setYC(ymean);

  param->setDist(dist);

  ////////////////////////////////////////////////////////////////////////////
  /////////////// directional cosines of the semi-major axis /////////////////
  ////////////////////////////////////////////////////////////////////////////

  const double d = sdevy2 - sdevx2;
  const double z = sqrt(d*d + 4.0*sdevxy*sdevxy);

  double cospsi=0;
  double sinpsi=0;

  if ( fabs(sdevxy) > ZeroTolerence )  // length != width, semi-major axis 
    {                                 // not along x or y
      const double ac=(d+z)*ymean + 2.0*sdevxy*xmean;
      const double bc=2.0*sdevxy*ymean - (d-z)*xmean;
      const double cc=sqrt(ac*ac+bc*bc);
      cospsi=bc/cc;
      sinpsi=ac/cc;
    }
  else if ( z > ZeroTolerence ) // semi-major axis along x or y 
    {                           // and length != width
      cospsi = (sdevx2 > sdevy2) ? 1 : 0;
      sinpsi = (sdevx2 > sdevy2) ? 0 : 1;
    }
  else if ( dist > 0 ) // length = width so might as well have semi-major axis
    {                  // be consistant with miss = dist, ie alpha = 90
      cospsi=-ymean / dist;
      // There seems to be a strange FP problem with the code below.. 
      //      sinpsi= xmean / dist;  
      sinpsi= sqrt(1.0-cospsi*cospsi);
    }
  else
    {
      cospsi=1;
      sinpsi=0;
    }
  
  param->setCosPsi(cospsi);
  param->setSinPsi(sinpsi);

  ////////////////////////////////////////////////////////////////////////////
  //////////////// length, width and miss - image parameters /////////////////
  ////////////////////////////////////////////////////////////////////////////

  double length2 = (sdevx2+sdevy2+z)/2.0;
  if ( length2 < ZeroTolerence )
    {
      //if ( length2 < -(ZeroTolerence) )
      //throw Error("Length squared is less than -ZeroTolerence");
      
      length2=0;
    }
  
  double width2  = (sdevx2+sdevy2-z)/2.0;
  if ( width2 < ZeroTolerence )
    {
      //if ( width2 < -(ZeroTolerence) )
      //throw Error("Width squared is less than -ZeroTolerence");

      width2=0;
    }
  
  const double length  = sqrt(length2);
  const double width   = sqrt(width2);

#ifdef OLDMISS
  double miss2=0;
  if( z > ZeroTolerence )
    {
      const double u = 1+d/z;
      const double v = 2-u;
      miss2 = (u*xmean2 + v*ymean2)/2.0 - meanxy*(2.0*sdevxy/z);
      
      if ( miss2 < ZeroTolerence )
	{
	  //if ( miss2 < -(ZeroTolerence) )
	  //throw Error("Miss squared is less than -ZeroTolerence");
	  
	  miss2 = 0;
	}
    }
  else
    {
      miss2 = xmean2 + ymean2; // ie = dist ^ 2
    }
  const double miss=sqrt(miss2);
#else
  double miss    = fabs(-sinpsi*xmean + cospsi*ymean);
  if(miss > dist)miss=dist; // Weird rounding error
#endif
  
  param->setLength(length);
  param->setWidth(width);
  param->setMiss(miss);

  param->setLengthOverSize(length/sumsig);

  ////////////////////////////////////////////////////////////////////////////
  /////////////////////// orientation: sinalpha and alpha ////////////////////
  ////////////////////////////////////////////////////////////////////////////

  const double sinalpha = (dist>ZeroTolerence) ? miss/dist : 0;
  //  if(sinalpha>1.0)sinalpha=1.0; // Floating point sanity check

  const double alpha=fabs(180.0/M_PI*asin(sinalpha));
  
  param->setSinAlpha(sinalpha);
  param->setAlpha(alpha);
  
  ////////////////////////////////////////////////////////////////////////////
  //////////////////////////////// Azwidth ///////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////

#ifdef AZWIDTHOLD
  const double m = y2mean - x2mean;
  const double n = sqrt(m*m + 4.0*xymean*xymean);

  double azwidth2  = (x2mean + y2mean - n)/2.0;
  if ( azwidth2 < ZeroTolerence )
      {
 	//if ( azwidth2 < -(ZeroTolerence) )
	//throw("Azwidth squared is less than -ZeroTolerence");

	azwidth2=0;
      }
  const double azwidth   = sqrt(azwidth2);
#else
  double azwidth;
  if(width2 > ZeroTolerence)
    {
      const double s2a=sinalpha*sinalpha;
      const double c2a=1.0-s2a;
      const double azfactor=
	1.0+((sinalpha==0)?0.0:(length2-width2)/(width2+length2*c2a/s2a));
      azwidth=width*sqrt(azfactor);
    }
  else 
    {
      azwidth=length;
    }
#endif
  param->setAzwidth(azwidth);

  ////////////////////////////////////////////////////////////////////////////
  //////////////////////// asymmetry major and minor /////////////////////////
  ////////////////////////////////////////////////////////////////////////////

  double asymmetry = 0;
  double minorasymmetry = 0;
  
  if(length2 > ZeroTolerence)
    {
      const double x3mean = sumx3sig / sumsig;
      const double y3mean = sumy3sig / sumsig;
      const double x2ymean = sumx2ysig / sumsig;
      const double xy2mean = sumxy2sig / sumsig;

      const double sdevx3 = x3mean - 3.0*xmean*x2mean + 2.0*xmean*xmean2;
      const double sdevy3 = y3mean - 3.0*ymean*y2mean + 2.0*ymean*ymean2;
      const double sdevx2y = 
	x2ymean - 2.0*xymean*xmean + 2.0*xmean2*ymean - x2mean*ymean;
      const double sdevxy2 =
	xy2mean - 2.0*xymean*ymean + 2.0*xmean*ymean2 - xmean*y2mean;

      const double cospsi2=cospsi*cospsi;
      const double sinpsi2=sinpsi*sinpsi;

      const double asymmetry3length3 =
	sdevx3*cospsi*cospsi2 + 3.0*sdevx2y*sinpsi*cospsi2 +
	3.0*sdevxy2*cospsi*sinpsi2 + sdevy3*sinpsi*sinpsi2;
      
      if ( fabs(asymmetry3length3) > ZeroTolerence )
	{
	  asymmetry = pow(fabs(asymmetry3length3),0.333333333333) / length;
	  if ( asymmetry3length3 < 0 )asymmetry = -asymmetry;
	}

      if(width2 > ZeroTolerence)
	{
	  const double minorasymmetry3width3 =
	    -sdevx3*sinpsi*sinpsi2 + 3.0*sdevx2y*cospsi*sinpsi2 -
	    3.0*sdevxy2*sinpsi*cospsi2 + sdevy3*cospsi*cospsi2;
	  
	  if ( fabs(minorasymmetry3width3) > ZeroTolerence )
	    {
	      minorasymmetry = pow(fabs(minorasymmetry3width3),
				   0.333333333333) / width;
	      if ( minorasymmetry3width3 < 0 )minorasymmetry = -minorasymmetry;
	    }
	}
    }
  
  param->setAsymmetry(asymmetry);
  param->setMinorAsymmetry(minorasymmetry);
  
  return;
}

///////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Shift Origin //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void 
NS_Analysis::HillasParameterization::
shiftOrigin(HillasParam* param, double origin_x, double origin_y) const
{
  if(param->length() > ZeroTolerence)
    {
      const double x0=origin_x;
      const double y0=origin_y;
      
      const double xc=param->getXC() + param->getX0() - origin_x;
      const double yc=param->getYC() + param->getY0() - origin_y;
      
      const double dist=sqrt(xc*xc+yc*yc);
      const double miss=fabs(-param->getSinPsi()*xc + param->getCosPsi()*yc);
      
      const double sinalpha = (dist>ZeroTolerence) ? miss/dist : 0;
      const double alpha=fabs(180.0/M_PI*asin(sinalpha));
      
      param->setX0(x0);
      param->setY0(y0);
      
      param->setXC(xc);
      param->setYC(yc);
      
      param->setDist(sqrt(xc*xc+yc*yc));
      
      const double length=param->getLength();
      const double width=param->getWidth();

      if((length-width) > ZeroTolerence)param->setMiss(miss);
      else param->setMiss(dist);
      
      param->setSinAlpha(sinalpha);
      param->setAlpha(alpha);

      const double l2=length*length;
      const double w2=width*width;
      const double s2a=sinalpha*sinalpha;
      const double c2a=1-s2a;

      const double azfactor=1.0+((sinalpha==0)?0.0:(l2-w2)/(w2+l2*c2a/s2a));
      const double azwidth=width*sqrt(azfactor);

      param->setAzwidth(azwidth);
    }
}

