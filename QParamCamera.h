//-*-mode:c++; mode:font-lock;-*-

#ifndef QPARAMCAMERA_H
#define QPARAMCAMERA_H

#define __STL_USE_NAMESPACES

#include <qwidget.h>
#include <qcolor.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>

#include "QEventCamera.h"
#include "ChannelRepresentation.h"
#include "HillasParam.h"
#include "CameraConfiguration.h"

namespace NS_QComponents {

  using NS_Analysis::HillasParam;

  class QParamCamera : public QEventCamera
  {
    Q_OBJECT

  public:
    
  private:
    HillasParam m_hparam;
    QColor m_ellipsecolor;

    bool m_shownothing;
    bool m_showmajoraxis;
    bool m_showwidth;
    bool m_showellipse;
    bool m_showasymmetry;
    bool m_showminorasymmetry;
    bool m_showalpha;

  public:
    QParamCamera( const CameraConfiguration* cam=0,
		  QWidget *parent=0, const char *name=0 );

    const QColor& colorEllipse() const { return m_ellipsecolor; }
    
    bool showNothing() const { return m_shownothing; }
    bool showMajorAxis() const { return m_showmajoraxis; }
    bool showWidth() const { return m_showwidth; }
    bool showEllipse() const { return m_showellipse; }
    bool showAsymmetry() const { return m_showasymmetry; }
    bool showMinorAsymmetry() const { return m_showminorasymmetry; }
    bool showAlpha() const { return m_showalpha; }

  public slots:
    void setData(const EventChannelReps& ecr, const HillasParam& hp);
    void setColorEllipse(const QColor& c);
    void setShowNothing(bool s);
    void setShowMajorAxis(bool s);
    void setShowWidth(bool s);
    void setShowEllipse(bool s);
    void setShowAsymmetry(bool s);
    void setShowMinorAsymmetry(bool s);
    void setShowAlpha(bool s);

  public:
    virtual void drawMe(QPainter& p);
    
  protected:
    //    void  paintEvent( QPaintEvent * );
  }; // class QParamCamera

} // namespace NS_QComponents

#endif // QPARAMCAMERA_H
