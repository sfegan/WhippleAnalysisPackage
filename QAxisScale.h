//-*-mode:c++; mode:font-lock;-*-

#ifndef QAXISSCALE_H
#define QAXISSCALE_H

#define __STL_USE_NAMESPACES

namespace NS_Analysis {

  class QAxisTics
  {
  protected:
    unsigned int nbig;
    unsigned int nsub;

  public:
    QAxisTics(): nbig(0), nsub(0) {}
    double nBigTics() const { return nbig; }
    double nSubTics() const { return nsub; }
    virtual double tic(unsigned int tn, unsigned int sn=0) const = 0;
  };

  class QAxisScale
  {
  private:
    double m_linmin;
    double m_linmax;

    int m_pixels;
    int m_pixelsize;
    
    double m_rounder;

  protected:
    double linRange() const 
      { return ((m_linmax==m_linmin)?1.0:m_linmax-m_linmin); }
    
    // These map world coordinates to and from intermediate internal linear
    // coordinates, basically intended for performing log scaling etc
    virtual double worldFTransform(double w) const = 0;
    virtual double worldRTransform(double w) const = 0;
    
  public:
    QAxisScale(): 
      m_linmin(0), m_linmax(0), 
      m_pixels(0), m_pixelsize(1), m_rounder(0.5) {}
    
    // Size of underlying device
    void setPhysicalSize(int pixels) { if(pixels>0)m_pixels=pixels; }
    void setPixelSize(int pixelsize) { if(pixelsize>0)m_pixelsize=pixelsize; }
    int physicalSize() const { return m_pixels; }
    int pixelSize() const { return m_pixelsize; }
    
    // Rounding to apply (0.0==floor, 0.5=nearest, 1.0=ceil)
    void setRounder(double rounder) { m_rounder=rounder; }
    double rounder() const { return m_rounder; }
    
    // Range of the "world" we are describing with this axis
    void setWorldRange(double min, double max);
    double worldMin() const { return worldRTransform(m_linmin); }
    double worldMax() const { return worldRTransform(m_linmax); }
    double worldSize() const { return worldMax()-worldMin(); }
    
    int mapWorldToPhysical(double w) const;
    double mapPhysicalToWorld(int p) const;

  }; // class QAxisScale

  class QLinearAxisTics: public QAxisTics
  {
    double first_tic;
    double delta_tic;
  public:
    QLinearAxisTics(const QAxisScale* a, 
		    unsigned int nb=10, unsigned int ns=10);

    virtual double tic(unsigned int tn, unsigned int sn=0) const;
  };

  class QLinearAxisScale: public QAxisScale
  {
  protected:
    virtual double worldFTransform(double w) const;
    virtual double worldRTransform(double w) const;
  public:
    QLinearAxisScale(): QAxisScale() {} 
  };
  
  class QLogAxisScale: public QAxisScale
  {
  protected:
    virtual double worldFTransform(double w) const;
    virtual double worldRTransform(double w) const;
  public:
    QLogAxisScale(): QAxisScale() {}
  };
}

inline void 
NS_Analysis::QAxisScale::
setWorldRange(double min, double max)
{ 
  m_linmin=worldFTransform(min);
  m_linmax=worldFTransform(max);
}

inline int 
NS_Analysis::QAxisScale::
mapWorldToPhysical(double w) const
{
  double lin=worldFTransform(w);
  return int((lin-m_linmin)/linRange()*physicalSize()+rounder());
}

inline double 
NS_Analysis::QAxisScale::
mapPhysicalToWorld(int p) const
{
  double lin=((double(p)+0.5)-rounder())/physicalSize()*linRange()+m_linmin;
  return worldRTransform(lin);
}

#endif // QAXISSCALE_H
