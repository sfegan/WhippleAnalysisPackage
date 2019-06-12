//-*-mode:c++; mode:font-lock;-*-

#ifndef QAXISSCALE_H
#define QAXISSCALE_H

#define __STL_USE_NAMESPACES

#include <qwidget.h>
#include <qcolor.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>

namespace NS_Analysis {
  

  class QAxis : public QWidget
  {
    Q_OBJECT
    
  private:
    QAxisScale* m_lscale;
    QAxisScale* m_rscale;
    QAxisScale* m_bscale;
    QAxisScale* m_tscale;

    bool m_enable_axes;  // Leave space on the painter for the axes
    bool m_render_axes;  // Actually render the axes in the space left for them
    bool m_render_first;

    bool accountForAxes() const { return m_enable_axes; }
    bool renderAxes() const { return m_enable_axes && m_render_axes; }
    

  protected:
    virtual double worldMax() const = 0;
    virtual double worldMin() const = 0;
    
  public:
    QAxis(): 
      m_render_first(true), m_render_axes(true), m_enable_axes(true),
      m_lscale(0), m_rscale(0), m_bscale(0), m_tscale(0) {}
    

  }; // class QAxis

} // namespace NS_Analysis

#endif // QAXISSCALE_H
