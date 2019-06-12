//-*-mode:c++; mode:font-lock;-*-

#ifndef CLEANING_H
#define CLEANING_H

#define __STL_USE_NAMESPACES

#include<vector>

#include"ChannelData.h"
#include"CameraConfiguration.h"

namespace NS_Analysis {

  using std::vector;

  class CleanedState
  {
  public:
    enum  CL_ChannelState { CL_DISABLED=-1, CL_UNKNOWN=0,
			    CL_NOTIMAGE, CL_IMAGELOW, CL_IMAGEHIGH };
    
  private:
    CL_ChannelState m_state;
    
  public:
    CleanedState(): m_state(CL_UNKNOWN) {}
    CleanedState(CL_ChannelState s): m_state(s) {} 
    
    // Accessors
    CL_ChannelState state() const { return m_state; }
    void set_state(CL_ChannelState s) { m_state=s; }
    
    // Some logical queries
    bool disabled() const { return m_state==CL_DISABLED; }
    bool image() const 
    { return ((m_state==CL_IMAGELOW)||(m_state==CL_IMAGEHIGH)); }
    bool unknown() const { return m_state==CL_UNKNOWN; }
  };

  class Cleaner
  {
  protected:
    const CameraConfiguration* camera;
  public:
    Cleaner(const CameraConfiguration* c): camera(c) {}
    virtual void clean(ChannelData<CleanedState>& clean,
		       const ChannelData<double>& signaltonoise) const=0;
    virtual void resetCamera(CameraConfiguration* cam);

    static std::vector<unsigned> s_count;
  };
  
  class CleanerPicBnd: public Cleaner
  {
  private:
    double p_level;
    double b_level;
  public:
    CleanerPicBnd(const CameraConfiguration* c,
		  double plevel, double blevel):
      Cleaner(c), p_level(plevel), b_level(blevel) {}

    virtual void clean(ChannelData<CleanedState>& clean,
		       const ChannelData<double>& signaltonoise) const;
  };

  class CleanerRegional: public Cleaner
  {
  private:
    double r_level;
    double i_level;
    unsigned int multiplicity;
  public:
    CleanerRegional(const CameraConfiguration* c, 
		    double ilevel, double rlevel, unsigned int mult): 
      Cleaner(c), r_level(rlevel), i_level(ilevel), multiplicity(mult) {}
    
    virtual void clean(ChannelData<CleanedState>& clean,
		       const ChannelData<double>& signaltonoise) const;
  };

} // namespace NS_Analysis

#endif // CLEANING_H
