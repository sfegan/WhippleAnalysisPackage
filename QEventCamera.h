//-*-mode:c++; mode:font-lock;-*-

#ifndef QEVENTCAMERA_H
#define QEVENTCAMERA_H

#define __STL_USE_NAMESPACES

#include <qwidget.h>
#include <qcolor.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>

#include "QCamera.h"
#include "ChannelRepresentation.h"
#include "CameraConfiguration.h"
#include "Cleaning.h"

namespace NS_QComponents {

  using NS_Analysis::CleanedState;
  using NS_Analysis::EventChannelReps;
  
  class QEventCamera : public QCamera
  {
    Q_OBJECT
    
  public:
    enum DisplayDataSet { SIG_RAW, SIG_SUBTRACTED, SIG_GAINADJUSTED };

    QEventCamera( const CameraConfiguration* cam=0,
		  QWidget *parent=0, const char *name=0 );
    
    const QColor& colorDisabled() const { return m_cdisabled; }
    const QColor& colorNotImage() const { return m_cnotimage; }
    const QColor& colorImageLow() const { return m_cimagelow; }
    const QColor& colorImageHigh() const { return m_cimagehigh; }
    bool imageColoring() const { return m_imagecoloring; }
    DisplayDataSet displayDataSet() const { return m_displayset; }
    bool zeroSuppression() const { return m_zerosuppress; }

    bool drawImageColorKey() { return m_draw_color_key; }

    virtual void drawMe(QPainter& p);

  public slots:
    void setData(const EventChannelReps& ecr);
    void setZeroSuppression(bool zs);
    void setColorDisabled(const QColor& c);
    void setColorNotImage(const QColor& c);
    void setColorImageLow(const QColor& c);
    void setColorImageHigh(const QColor& c);

    void setImageColoring(bool c);
    void setDisplayDataSet(DisplayDataSet d);

    void setDrawImageColorKey(bool key);

  protected:
    virtual int getKeyFontPointSize(QPainter& p);

  private:
    EventChannelReps m_event;

    QColor m_cdisabled;
    QColor m_cnotimage;
    QColor m_cimagelow;
    QColor m_cimagehigh;
    bool             m_imagecoloring;

    DisplayDataSet   m_displayset;

    bool             m_draw_color_key;
    
    bool             m_zerosuppress;
    
    void updateCam();

    template<class T>
    void setAndUpdate(T& o, const T& n);

    void drawKeyChannel(QPainter& p, 
			double xc, double yc, double r, const QColor& c,
			const std::string& text, double scale);

  }; // class QEventCamera
  
} // namespace NS_QComponents

template<class T>
inline void
NS_QComponents::QEventCamera::
setAndUpdate(T &o, const T& n)
{
  if(o!=n)
    {
      o=n;
      updateCam();
    }
}

#endif // QEVENTCAMERA_H
