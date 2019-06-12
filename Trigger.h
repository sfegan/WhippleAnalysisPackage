//-*-mode:c++; mode:font-lock;-*-

#ifndef TRIGGER_H
#define TRIGGER_H

#define __STL_USE_NAMESPACES

#include<vector>
#include"CameraConfiguration.h"

namespace NS_Analysis {

  
  class Trigger
  {
    const CameraConfiguration* m_camera;
    double m_level;
  public:
    Trigger(double lev, const CameraConfiguration* cam): 
      m_camera(cam), m_level(lev) {}

    // Access
    double level() const { return m_level; }
    const CameraConfiguration* camera() const { return m_camera; }

    // Implement these in your trigger
    virtual bool trigger(const vector<double>& signal) const = 0;
    virtual double maxThreshold(const vector<double>& signal) const = 0;
  };

  class TriggerWithMult: public Trigger
  {
  private:
    unsigned int m_multiplicity;
    
  public:
    TriggerWithMult(unsigned int mult, double lev, const CameraConfiguration* cam):
      Trigger(lev,cam), m_multiplicity(mult) {}
    
    unsigned int multiplicity() const { return m_multiplicity; }
  };
  
  class MUL: public TriggerWithMult
  {
  public:
    MUL(unsigned int mult, double lev, const CameraConfiguration* cam):
      TriggerWithMult(mult,lev,cam) {}

    virtual bool trigger(const vector<double>& signal) const;
    virtual double maxThreshold(const vector<double>& signal) const;
  };

  class PST: public TriggerWithMult
  {
  public:
    PST(unsigned int mult, double lev, const CameraConfiguration* cam):
      TriggerWithMult(mult, lev,cam) {}

    virtual bool trigger(const vector<double>& signal) const;
    virtual double maxThreshold(const vector<double>& signal) const;
  };

}

#endif // TRIGGER_H
