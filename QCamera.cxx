#include <qpainter.h>
#include <qpixmap.h>
#include <qbrush.h>
#include <qpaintdevicemetrics.h> 
#include <qfont.h>
#include <qevent.h>
#include <qnamespace.h>

#include <sstream>

#include"CameraConfiguration.h"
#include"QCamera.h"

#define MIN(x,y) (((x)<(y))?(x):(y))
#define MAX(x,y) (((x)>(y))?(x):(y))

NS_QComponents::QCamera::
QCamera(const CameraConfiguration* cam, QWidget *parent, const char *name):
  QWidget(parent,name,WResizeNoErase),
  m_camera(0), m_camerasize(0), m_demandsize(), m_rotationangle(0),
  m_longest_tube_number(), m_largest_label_size(0),
  m_channelvalue(), m_channelcolor(), m_datamax(0), m_demandmax(0),
  m_label_tubes(false), m_draw_neighbor_vertices(false), 
  m_highlight_trigger(false), m_draw_range_key(false), m_draw_outline(true),
  m_style(STYLE_AREA), m_repaint_lock(0), m_repaint_scheduled(false)
{
  //  setPalette( QPalette(QColor("black"),QColor("grey")) );
  setMinimumSize(100,100);

  setCamera(cam);
}

NS_QComponents::QCamera::
~QCamera()
{

}


void
NS_QComponents::QCamera::
setCamera(const CameraConfiguration* cam)
{
  double oldsize=angularSize();

  m_camera=cam;
  m_camerasize=0;

  bool first = false;
  int largest_namelen=0;
  double smallest_radius=0;

  if(cam)
    for(unsigned int i=0; i<m_camera->nchannels(); i++)
      {
	if(!m_camera->channel(i).isRealChannel())continue;

	double channel_edgedist=m_camera->channel(i).edgedist();
	if(channel_edgedist > m_camerasize)m_camerasize=channel_edgedist;

	int namelen = m_camera->channel(i).name().length();
	double radius  = m_camera->channel(i).r();
	
	if(!first)
	  {
	    largest_namelen = namelen;
	    smallest_radius = radius;
	    first = true;
	  }

	largest_namelen = MAX(namelen,largest_namelen);
	smallest_radius = MIN(radius,smallest_radius);
      }

  m_longest_tube_number = "";
  for(int i=0; i<largest_namelen; i++)m_longest_tube_number+="8";
  m_largest_label_size = (2.0*smallest_radius)*0.85;

  if(oldsize != angularSize())
    emit angularSizeChanged(angularSize());
    
  scheduleRepaint();
}

void
NS_QComponents::QCamera::
drawTube(QPainter& p, double x, double y, double r, 
	 double fraction, const QColor& color, double scale,
	 bool outline, const QColor& outlineColor) const
{
  if(scale==0)scale=getScale(p);

  int xc=scaleDistance(x,scale);
  int yc=scaleDistance(y,scale);
  int ro=scaleDistance(r,scale);

  int ri=int(double(ro)*sqrt(fabs(fraction))+0.5);

  if(ri)
    {
      if(fraction <= 1.0)p.setPen(color);
      else p.setPen(outlineColor);

      if(fraction > 0)p.setBrush(color);
      else p.setBrush(QBrush());

      p.drawEllipse(transX(xc-ri),transY(yc+ri),2*ri,2*ri);
    }

  p.setPen(outlineColor);
  p.setBrush(QBrush());
  if((outline)&&(fraction <= 1.0))
    {
      p.drawEllipse(transX(xc-ro),transY(yc+ro),2*ro,2*ro);
    }
}

void
NS_QComponents::QCamera::
labelTube(QPainter& p, double x, double y,
	  const std::string &text, double scale) const
{
  if(scale==0)scale=getScale(p);

  int xc=scaleDistance(x,scale);
  int yc=scaleDistance(y,scale);
  
  p.setPen(foregroundColor());
  p.drawText(transX(xc), transY(yc), 0, 0,AlignCenter|DontClip, text.c_str());
}

static inline int mix(double v, int f, double omv, int b)
{
  return int(floor(v*double(f) + omv*double(b))+0.5);
}

QColor& 
NS_QComponents::QCamera::
blendColors(double v, const QColor& f, const QColor& b) const
{
  static QColor c;

  if(v<0)v=0;
  if(v>1)v=1;
  double omv=1-v;

  c.setRgb(mix(v,f.red(),omv,b.red()),
	   mix(v,f.green(),omv,b.green()),
	   mix(v,f.blue(),omv,b.blue()));
  return c;
}

void
NS_QComponents::QCamera::
drawChannel(QPainter& p, channelnum_type c, double scale) const
{
  const ChannelDescription* cd=&m_camera->channel(c);
  if(!cd->isRealChannel())return;
  
  if(scale==0)scale=getScale(p);

  double valscale=m_datamax;
  if(m_demandmax>0)valscale=m_demandmax;
  if(valscale<=0)valscale=1.0;

  double value=0;
  QColor color=QColor();
  bool outline=true;
  
  if(m_channelvalue.size() > c)value=m_channelvalue[c];
  if(m_channelcolor.size() > c)color=m_channelcolor[c];

  switch(m_style)
    {
    case STYLE_AREA:
      value   = value/valscale;
      outline = m_draw_outline;
      break;
      
    case STYLE_INTENSITY:
      value   = value/valscale;
      color   = blendColors(value, color, p.backgroundColor());
      value   = 1.0;
      outline = false;
      break;
    }
  
  drawTube(p,cd->x(),cd->y(),cd->r(),value,color,scale,outline,
	   ((m_highlight_trigger)&&(c<camera()->ntrigger()))?red:black);

  if(m_label_tubes)labelTube(p,cd->x(),cd->y(),cd->name(),scale);
}

void
NS_QComponents::QCamera::
drawNeighborVertices( QPainter& p, channelnum_type c, double scale) const
{
  const ChannelDescription* cd=&m_camera->channel(c);
  if((!cd->isRealChannel())||(cd->isMasked()))return;

  if(scale==0)scale=getScale(p);

  int xc=scaleDistance(cd->x(),scale);
  int yc=scaleDistance(cd->y(),scale);

  p.setPen(QPen(QColor("red"),2));
  for(unsigned int n=0;n<cd->numneighbors();n++)
    {
      channelnum_type nc=cd->neighbor(n);
      const ChannelDescription* ncd=&m_camera->channel(nc);
      if((nc<c)&&(ncd->isRealChannel())&&(!ncd->isMasked()))
	{
	  int nxc=scaleDistance(ncd->x(),scale);
	  int nyc=scaleDistance(ncd->y(),scale);
	  p.drawLine(transX(xc),transY(yc),transX(nxc),transY(nyc));
	}
    }
}

#define SQDIST(x,y) ((x)*(x)+(y)*(y))

int 
NS_QComponents::QCamera::
getSuggestedPenWidth(QPainter& p, double factor)
{
  QPaintDeviceMetrics metrics(p.device());
  const double penWidthMM = 0.25;
  int dots = 
    int(floor(sqrt(SQDIST(metrics.logicalDpiX(), metrics.logicalDpiY())/2.0)/
	      25.4*penWidthMM*factor + 0.5));
  return dots;
}

//#define __DEBUG_FONT_SELECTION

int 
NS_QComponents::QCamera::
getFontPointSize(QPainter& p, const std::string& text, 
		 double height, double width)
{
  double scale=getScale(p);

  int pix_height = scaleDistance(height,scale);
  int pix_width  = scaleDistance(width,scale);

  QFont f=p.font(); // save font for later
  
  int pointsize = pix_height/2;

  int font_height;
  int font_width;
  int font_pointsize;
  
  p.setFont(QFont("helvetica",pointsize,QFont::Normal));
  pointsize = p.fontInfo().pointSize();
  font_height = p.fontMetrics().height();
  font_width  = p.fontMetrics().width(text.c_str());
  font_pointsize = p.fontInfo().pointSize();
  double ratio_height = double(pix_height)/double(font_height);
  double ratio_width  = double(pix_width)/double(font_width);
#ifdef __DEBUG_FONT_SELECTION
  std::cerr << "START Point size: " << pointsize << '/'
       << font_pointsize << ' '
       << "Height: " << font_height << '/' << pix_height << ' '  
       << "Width: " << font_width << '/' << pix_width << std::endl;
#endif
  pointsize = int(double(pointsize)*MIN(ratio_width,ratio_height));
  p.setFont(QFont("helvetica",pointsize,QFont::Normal));
  pointsize = p.fontInfo().pointSize();
  font_height = p.fontMetrics().height();
  font_width  = p.fontMetrics().width(text.c_str());
  pointsize = font_pointsize = p.fontInfo().pointSize();
#ifdef __DEBUG_FONT_SELECTION
  std::cerr << "ZOOM Point size: " << pointsize << '/'
       << font_pointsize << ' '
       << "Height: " << font_height << '/' << pix_height << ' '  
       << "Width: " << font_width << '/' << pix_width << std::endl;
#endif
  
  if(((pix_height==0)||(font_height<pix_height))||
     ((pix_width==0)||(font_width<pix_width)))
    {
      int last_pointsize = font_pointsize;;
      while((pointsize<font_pointsize+20)&&
	    (((pix_height==0)||(font_height < pix_height))&&
	     ((pix_width==0)||(font_width < pix_width))))
	{
	  pointsize++;
	  p.setFont(QFont("helvetica",pointsize,QFont::Normal));
	  font_height = p.fontMetrics().height();
	  font_width  = p.fontMetrics().width(text.c_str());
	  font_pointsize = p.fontInfo().pointSize();
#ifdef __DEBUG_FONT_SELECTION
	  std::cerr << "UP Point size: " << pointsize << '/'
	       << font_pointsize << ' '
	       << "Height: " << font_height << '/' << pix_height << ' '  
	       << "Width: " << font_width << '/' << pix_width << std::endl;
#endif
	  if(font_pointsize > pointsize)pointsize=font_pointsize;
	  
	  if(((pix_height!=0)&&(font_height > pix_height))||
	     ((pix_width!=0)&&(font_width > pix_width)))break;
	  
	  last_pointsize = font_pointsize;
	}
      font_pointsize = last_pointsize;
    }
  else if(((pix_height==0)||(font_height>pix_height))||
	  ((pix_width==0)||(font_width>pix_width)))
    {
      while((pointsize>0)&&
	    (((pix_height==0)||(font_height > pix_height))&&
	     ((pix_width==0)||(font_width > pix_width))))
	{
	  pointsize--;
	  p.setFont(QFont("helvetica",pointsize,QFont::Normal));
	  font_height = p.fontMetrics().height();
	  font_width  = p.fontMetrics().width(text.c_str());
	  font_pointsize = p.fontInfo().pointSize();
#ifdef __DEBUG_FONT_SELECTION
	  std::cerr << "DOWN Point size: " << pointsize << '/'
	       << font_pointsize << ' '
	       << "Height: " << font_height << '/' << pix_height << ' '  
	       << "Width: " << font_width << '/' << pix_width << std::endl;
#endif
	  if(font_pointsize < pointsize)pointsize=font_pointsize;
	}
    }
  
  p.setFont(f);
  
#ifdef __DEBUG_FONT_SELECTION
  std::cerr << "END: " << font_pointsize << std::endl;
#endif
  return font_pointsize;
}

int 
NS_QComponents::QCamera::
getTubeFontPointSize(QPainter& p)
{
  static std::string cached_label    = "";
  static double cached_tubesize = 0;
  static double cached_scale    = 0;
  static int    cached_fontsize = 0;

  double scale = getScale(p);

  if((cached_tubesize != m_largest_label_size)||
     (cached_label != m_longest_tube_number)||
     (cached_scale != scale))
    {
      cached_fontsize =
	getFontPointSize(p,m_longest_tube_number,0,m_largest_label_size);
      cached_label = m_longest_tube_number;
      cached_tubesize = m_largest_label_size;
      cached_scale = scale;
    }

  return cached_fontsize;
}

int 
NS_QComponents::QCamera::
getKeyFontPointSize(QPainter& p)
{
  return getTubeFontPointSize(p);
}

void
NS_QComponents::QCamera::
drawMe(QPainter& p)
{
  if(camera()==0)return;

  QPaintDeviceMetrics metrics(p.device());
  
  p.translate(metrics.width()/2,metrics.height()/2);
  p.scale(1.0,1.0);

  double scale=getScale(p);

  int pointsize= getTubeFontPointSize(p);
  p.setFont(QFont("helvetica" /*"times"*/,pointsize,QFont::Normal));

  for(unsigned int i=0; i<m_camera->nchannels(); i++)drawChannel(p,i,scale);

  if(m_draw_neighbor_vertices)
    for(unsigned int i=0; i<m_camera->nchannels(); i++)
      drawNeighborVertices(p,i,scale);

  if(m_draw_range_key)
    {
      p.setFont(QFont("helvetica" /*"times"*/,
      		      getKeyFontPointSize(p), QFont::Normal));
      
      double maxval = maxValue();
      maxval = floor(maxval+0.5);

      std::ostringstream textstream;
      textstream << "Scale: " << maxval;
      if(m_range_units != "")textstream << ' ' << m_range_units;

      int xc = scaleDistance( angularSize()/2.04,scale);
      int yc = scaleDistance(-angularSize()/2.04,scale);

      p.setPen(foregroundColor());
      p.drawText(transX(xc), transY(yc), 0, 0, 
		 AlignRight|AlignBottom|DontClip, textstream.str().c_str());
    }
}

void 
NS_QComponents::QCamera::
paintEvent( QPaintEvent *e )
{
  QRect cr = rect();
  QPixmap pix( cr.size() );
  pix.fill( this, cr.topLeft() );

  QPainter p( &pix );
  p.setBackgroundColor(backgroundColor());
  drawMe(p);
  p.end();
  
  bitBlt(this,e->rect().topLeft(),&pix,e->rect(),CopyROP);
}

void 
NS_QComponents::QCamera::
mousePressEvent( QMouseEvent* me )
{
  QRect cr = rect();
  int xc=me->x()-cr.width()/2;
  int yc=me->y()-cr.height()/2;

  double scale=getScale(this);
  double x=pixToDegX(xc,scale);
  double y=pixToDegY(yc,scale);

  if(camera()==0)return;

  int nearest;
  try
    {
      nearest=camera()->findNearest(x,y);
    }
  catch(const ChannelOutOfRange& coor)
    {
      std::cerr << "QCamera::mousePressEvent: " << coor << '\n';
    }
  
  if(camera()->insideTube(nearest,x,y))
    emit mousePress( nearest, me );
}

void 
NS_QComponents::QCamera::
mouseReleaseEvent( QMouseEvent* me )
{
  emit mouseRelease(me);
}

/*
QSizePolicy 
NS_QComponents::QCamera::
sizePolicy() const
{
  return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}
*/

QSize 
NS_QComponents::QCamera::
sizeHint () const
{
  return QSize(600,600);
}

void 
NS_QComponents::QCamera::
setRotationAngle(double ra)
{
  setAndRepaint(m_rotationangle,ra);
}

void 
NS_QComponents::QCamera::
setAngularSize(double as)
{
  double oldas=angularSize();
  if(as != m_demandsize)m_demandsize=as;

  if(oldas != angularSize())
    {
      scheduleRepaint();
      emit angularSizeChanged(angularSize());
    }
}

void 
NS_QComponents::QCamera::
setMaxValue(double mv)
{
  double oldmv=maxValue();

  if(mv != m_demandmax)m_demandmax=mv;

  if(oldmv != maxValue())
    {
      scheduleRepaint();
      emit maxValueChanged(maxValue());
    }
}

/*
void 
NS_QComponents::QCamera::
setData(const std::vector<double>& value, const vector<QColor>& color)
{
  double oldmv=maxValue();

  m_channelvalue=value;
  m_channelcolor=color;
  m_datamax=0;

  for(int i=0;i<m_channelvalue.size();i++)
    {
      double cv=fabs(m_channelvalue[i]);
      if(cv > m_datamax)m_datamax=cv;
    }

  if(oldmv != maxValue())emit maxValueChanged(maxValue());

  scheduleRepaint();
}
*/

void 
NS_QComponents::QCamera::
setLabelling(bool label)
{
  setAndRepaint(m_label_tubes,label);
}

void 
NS_QComponents::QCamera::
setDrawVertices(bool draw)
{
  setAndRepaint(m_draw_neighbor_vertices,draw);
}

void 
NS_QComponents::QCamera::
setDrawRangeKey(bool key)
{
  setAndRepaint(m_draw_range_key,key);
}

void 
NS_QComponents::QCamera::
setDrawOutline(bool outline)
{
  setAndRepaint(m_draw_outline,outline);
}

void 
NS_QComponents::QCamera::
setHighlightTrigger(bool highlight)
{
  setAndRepaint(m_highlight_trigger,highlight);
}

void 
NS_QComponents::QCamera::
setStyle(DisplayStyle s)
{
  setAndRepaint(m_style,s);
}
