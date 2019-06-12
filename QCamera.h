//-*-mode:c++; mode:font-lock;-*-

#ifndef QCAMERA_H
#define QCAMERA_H

#define __STL_USE_NAMESPACES

#include <qwidget.h>
#include <qcolor.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>

#include "CameraConfiguration.h"

namespace NS_QComponents {

  using NS_Analysis::channelnum_type;
  using NS_Analysis::CameraConfiguration;
  using NS_Analysis::ChannelDescription;
  using NS_Analysis::ChannelOutOfRange;

  class QCamera : public QWidget
  {
    Q_OBJECT
    
  public:
    enum DisplayStyle { STYLE_AREA, STYLE_INTENSITY };

    QCamera( const CameraConfiguration* cam=0,
	     QWidget *parent=0, const char *name=0 );
    virtual ~QCamera();
    
    const CameraConfiguration* camera() const { return m_camera; }
    double rotationAngle() const { return m_rotationangle; }
    double angularSize() const;
    double maxValue() const;
    const std::vector<double>& dataValue() const { return m_channelvalue; }
    const std::vector<QColor>& dataColor() const { return m_channelcolor; }
    bool labelling() const { return m_label_tubes; }
    bool drawVertices() const { return m_draw_neighbor_vertices; }
    bool highlightTrigger() const { return m_highlight_trigger; }
    bool drawRangeKey() const { return m_draw_range_key; }
    bool drawOutline() const { return m_draw_outline; }

    void setRangeUnits(const std::string& units) { m_range_units = units; }

    void disableRepaint() { m_repaint_lock++; }
    void enableRepaint() 
    { m_repaint_lock--;  if(m_repaint_lock==0)executeScheduledRepaint(); }

    DisplayStyle style() const { return m_style; }

    template<class T>
    void setData(const std::vector<T>& value, const std::vector<QColor>& color);

  public slots:
    void setCamera( const CameraConfiguration* cam );
    void setRotationAngle(double ra);
    void setAngularSize(double as);
    void setMaxValue(double mv);
    void setLabelling(bool label);
    void setDrawVertices(bool draw);
    void setHighlightTrigger(bool highlight);
    void setDrawRangeKey(bool key);
    void setDrawOutline(bool outline);
    void setStyle(DisplayStyle s);

  signals:
    void angularSizeChanged(double);
    void maxValueChanged(double);
    void mousePress(channelnum_type channel, const QMouseEvent* me);
    void mouseRelease(const QMouseEvent* me);

  public:
    virtual void drawMe(QPainter& p);

  protected:
    double getScale( const QPainter& pd ) const;
    int scaleDistance( double distance, const QPainter &p, 
		       double rounder ) const;
    int scaleDistance( double distance, double scale, double rounder=0.0 ) const;
    void drawTube(QPainter& p, double x, double y, double r, 
		  double fraction, const QColor& color, double scale=0,
		  bool outline=true, const QColor& outlineColor=black) const;
    void drawChannel(QPainter &p, channelnum_type c, double scale=0) const;
    void drawNeighborVertices( QPainter& p, channelnum_type c, 
			       double scale=0 ) const;
    void labelTube(QPainter& p, double x, double y, const std::string &text, 
		   double scale=0) const;
    int transX(int x) const { return x; }
    int transY(int y) const { return -y; }

    double pixToDegX(int x, double scale, double rounder=0.0) const;
    double pixToDegY(int y, double scale, double rounder=0.0) const;

    virtual int getSuggestedPenWidth(QPainter& p, double factor=1.0);

    QColor& blendColors(double v, const QColor& f, const QColor& b) const;

    // Locking off of repaints so that many updates can happen at once
    template<class T>
    void setAndRepaint(T& o, const T& n);

    void scheduleRepaint() 
    { m_repaint_scheduled=true; 
      if(m_repaint_lock==0)executeScheduledRepaint(); }
    void executeScheduledRepaint()
    { if(m_repaint_scheduled) { repaint(FALSE); m_repaint_scheduled=false; } }

    // Various font related methods
    int getFontPointSize(QPainter& p, const std::string& text, 
			 double height, double width);
    virtual int getTubeFontPointSize(QPainter& p);
    virtual int getKeyFontPointSize(QPainter& p);

    
  protected:
    // Overrides of Qt events / functions
    virtual QSize sizeHint () const;    
    void paintEvent( QPaintEvent * );
    void mousePressEvent( QMouseEvent* me );
    void mouseReleaseEvent( QMouseEvent* me );

  private:
    // Camera Stuff
    const CameraConfiguration*        m_camera;
    double                            m_camerasize;
    double                            m_demandsize;
    double                            m_rotationangle;

    // Font and max label size
    std::string                            m_longest_tube_number;
    double                            m_largest_label_size;;

    // Channel data and colors
    std::vector<double>                    m_channelvalue;
    std::vector<QColor>                    m_channelcolor;

    double                            m_datamax;
    double                            m_demandmax;

    // Some options
    bool                              m_label_tubes;
    bool                              m_draw_neighbor_vertices;
    bool                              m_highlight_trigger;
    bool                              m_draw_range_key;
    bool                              m_draw_outline;
    std::string                            m_range_units;

    DisplayStyle                      m_style;

    // Locking of repaint so multiple updates dont cause unpleasent redraws
    int                               m_repaint_lock;
    bool                              m_repaint_scheduled;
  }; // class QCamera

} // namespace NS_QComponents

template<class T>
inline void
NS_QComponents::QCamera::
setAndRepaint(T &o, const T& n)
{
  if(o!=n)
    {
      o=n;
      scheduleRepaint();
    }
}

inline double
NS_QComponents::QCamera::
angularSize() const 
{
  double angsize=m_camerasize*2.04;
  if(m_demandsize > 0)angsize=m_demandsize;
  return angsize; 
}

inline double
NS_QComponents::QCamera::
maxValue() const 
{ 
  double maxvalue=m_datamax;
  if(m_demandmax > 0)maxvalue=m_demandmax;
  return maxvalue; 
}

inline double
NS_QComponents::QCamera::
getScale( const QPainter& p ) const
{
  double angsize=angularSize();

  QPaintDeviceMetrics metrics(p.device());

  if(metrics.width() < metrics.height())return double(metrics.width())/angsize;
  else return double(metrics.height())/angsize;
}

inline int
NS_QComponents::QCamera::
scaleDistance( double distance, double scale, double rounder ) const
{
  return int(distance*scale+rounder);
}

inline int
NS_QComponents::QCamera::
scaleDistance( double distance, const QPainter &p, double rounder ) const
{
  return scaleDistance(distance,getScale(p),rounder);
}

inline double
NS_QComponents::QCamera::
pixToDegX(int x, double scale, double rounder) const
{
  return (double(x)+0.5-rounder)/scale;
}

inline double
NS_QComponents::QCamera::
pixToDegY(int y, double scale, double rounder) const
{
  return (double(-y)+0.5-rounder)/scale;
}

template<class T> void 
NS_QComponents::QCamera::
setData(const std::vector<T>& value, const std::vector<QColor>& color)
{
  double oldmv=maxValue();

  m_channelvalue.resize(value.size());
  for(unsigned int i=0;i<value.size();i++)
    m_channelvalue[i] = value[i];
  m_channelcolor=color;

  m_datamax=0;

  for(unsigned int i=0;i<m_channelvalue.size();i++)
    {
      double cv=fabs(m_channelvalue[i]);
      if(cv > m_datamax)m_datamax=cv;
    }

  if(oldmv != maxValue())emit maxValueChanged(maxValue());

  scheduleRepaint();
}

#endif // QCAMERA_H
