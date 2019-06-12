#include <cmath>

#include "TwoDimensionalParameterization.h"

NS_Analysis::TwoDimensionalDistCalculator::
~TwoDimensionalDistCalculator()
{
}
  
NS_Analysis::TwoDimensionalParameterization::
~TwoDimensionalParameterization()
{
}

void
NS_Analysis::TwoDimensionalParameterization::
rotateAxis(double theta, double& c, double& s) const
{
  double cc =  c*cos(theta) + s*sin(theta);
  double ss = -c*sin(theta) + s*cos(theta);
  c=cc;
  s=ss;
}

///////////////////////////////////////////////////////////////////////////////

NS_Analysis::DoublePoint2DParametrization::
~DoublePoint2DParametrization()
{

}

void
NS_Analysis::DoublePoint2DParametrization::
parameterize(double derotationAngle, const HillasParam* hp,
	     TwoDimBinner<Summer>* twodim, Summer* radial) const 
{
  double majorerror         = 0;
  double minorerror         = 0;

  double cospsi             = hp->cos_psi();
  double sinpsi             = hp->sin_psi();

  const double edist        = m_distcalc->edist(hp,majorerror,minorerror);

  double X1                 = hp->xc() - edist*cospsi;
  double Y1                 = hp->yc() - edist*sinpsi;

  double X2                 = hp->xc() + edist*cospsi;
  double Y2                 = hp->yc() + edist*sinpsi;

  if(twodim)
    {
      rotateAxis(derotationAngle,X1,Y1);
      rotateAxis(derotationAngle,X2,Y2);
      twodim->insert(X1, Y1);
      twodim->insert(X2, Y2);
    }
      
  if(radial)
    {
      radial->insert(sqrt(X1*X1 + Y1*Y1));
      radial->insert(sqrt(X2*X2 + Y2*Y2));
    }

  return;
}

///////////////////////////////////////////////////////////////////////////////

NS_Analysis::Asymmetry2DParametrization::
~Asymmetry2DParametrization()
{

}

void
NS_Analysis::Asymmetry2DParametrization::
parameterize(double derotationAngle, const HillasParam* hp,
	     TwoDimBinner<Summer>* twodim, Summer* radial) const 
{
  double majorerror         = 0;
  double minorerror         = 0;
  
  double cospsi             = hp->cos_psi();
  double sinpsi             = hp->sin_psi();
  
  const double edist        = m_distcalc->edist(hp,majorerror,minorerror);

  double X                  = hp->xc() - edist*cospsi;
  double Y                  = hp->yc() - edist*sinpsi;

  if(hp->asymmetry() < 0)
    {
      X                     = hp->xc() + edist*cospsi;
      Y                     = hp->yc() + edist*sinpsi;
    }

  if(twodim)
    {
      rotateAxis(derotationAngle,X,Y);
      twodim->insert(X, Y);
    }
      
  if(radial)
    {
      radial->insert(sqrt(X*X + Y*Y));
    }

  return;
}
