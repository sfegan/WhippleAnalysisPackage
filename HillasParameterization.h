//-*-mode:c++; mode:font-lock;-*-

#ifndef HILLASPARAMETERIZATION_H
#define HILLASPARAMETERIZATION_H

#define __STL_USE_NAMESPACES

#include "HillasParam.h"
#include "CameraConfiguration.h"
#include "ChannelRepresentation.h"

namespace NS_Analysis {
  
  const static double ZeroTolerence = 1e-8;
    
  class HillasParameterization
  {
  private:
    const CameraConfiguration* m_cam;
    
  public:
    HillasParameterization(const CameraConfiguration* camera):
      m_cam(camera) {}

    virtual ~HillasParameterization();

    virtual void parameterize(HillasParam* param,
			      const EventChannelReps* ecr,
			      double origin_x, double origin_y) const;
    
    virtual void shiftOrigin(HillasParam* param,
			     double origin_x, double origin_y) const;
  };
  
} // namespace NS_Analysis

#endif // HILLASPARAMETERIZATION_H
