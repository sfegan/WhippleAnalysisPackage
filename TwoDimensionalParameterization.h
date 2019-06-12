//-*-mode:c++; mode:font-lock;-*-

#ifndef TWODIMENSIONALPARAMETERIZATION_H
#define TWODIMENSIONALPARAMETERIZATION_H

#define __STL_USE_NAMESPACES

#include "HillasParam.h"
//#include "TwoDimensionalParam.h"
#include "Binner.h"

namespace NS_Analysis {

  class TwoDimensionalDistCalculator
  {
  public:
    virtual ~TwoDimensionalDistCalculator();
    virtual double edist(const HillasParam* hp, 
			 double& edisterror, double& alphaerror) const = 0;
  };
  
  class TwoDimensionalParameterization
  {
  public:
    virtual ~TwoDimensionalParameterization();
    virtual void parameterize(double derotationAngle, const HillasParam* hp,
			      TwoDimBinner<Summer>* twodim,
			      Summer* radial) const = 0;
    
  protected:
    void rotateAxis(double theta, double& c, double& s) const;
  };
  
  class DoublePoint2DParametrization: public TwoDimensionalParameterization
  {
  public:
    DoublePoint2DParametrization(TwoDimensionalDistCalculator* dc)
      : m_distcalc(dc) {}
    virtual ~DoublePoint2DParametrization();
    virtual void parameterize(double derotationAngle, const HillasParam* hp,
			      TwoDimBinner<Summer>* twodim, 
			      Summer* radial) const;

  private:
    TwoDimensionalDistCalculator* m_distcalc;
  };

  class Asymmetry2DParametrization: public TwoDimensionalParameterization
  {
  public:
    Asymmetry2DParametrization(TwoDimensionalDistCalculator* dc)
      : m_distcalc(dc) {}
    virtual ~Asymmetry2DParametrization();
    virtual void parameterize(double derotationAngle, const HillasParam* hp,
			      TwoDimBinner<Summer>* twodim, 
			      Summer* radial) const;

  private:
    TwoDimensionalDistCalculator* m_distcalc;
  };
  
} // namespace NS_Analysis

#endif // TWODIMENSIONALPARAMETERIZATION_H
