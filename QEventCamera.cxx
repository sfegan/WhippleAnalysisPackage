#include <qpainter.h>
#include <qpixmap.h>
#include <qbrush.h>
#include <qpaintdevicemetrics.h> 
#include <qfont.h>
#include <qnamespace.h>

#include"CameraConfiguration.h"
#include"QEventCamera.h"

NS_QComponents::QEventCamera::
QEventCamera(const CameraConfiguration* cam, QWidget *parent, 
	     const char *name):
  QCamera(cam,parent,name), m_event(),
  m_cdisabled(gray), m_cnotimage(blue), m_cimagelow(green), m_cimagehigh(red), 
  m_imagecoloring(true), m_displayset(SIG_GAINADJUSTED),
  m_draw_color_key(false), m_zerosuppress(false)
{
}

void 
NS_QComponents::QEventCamera::
updateCam()
{
  std::vector<double> value(m_event.nchannels());
  std::vector<QColor> color(m_event.nchannels());

  for(unsigned int i=0;i<m_event.nchannels();i++)
    {
      // VALUE
      switch(m_displayset)
	{
	case SIG_RAW:          value[i] = m_event.rawsignal(i); break;
	case SIG_SUBTRACTED:   value[i] = m_event.signal(i); break;
	case SIG_GAINADJUSTED: value[i] = m_event.light(i); break;
	};
      if((m_zerosuppress) && (!m_event.clean(i).image()))value[i]=0.0;
      
      // COLOR
      if(m_imagecoloring)
	{
	  switch(m_event.clean(i).state())
	    {
	    case CleanedState::CL_DISABLED:
	      color[i]=m_cdisabled;
	      break;
	    case CleanedState::CL_NOTIMAGE:
	      color[i]=m_cnotimage;
	      break;
	    case CleanedState::CL_IMAGELOW:
	      color[i]=m_cimagelow;
	      break;
	    case CleanedState::CL_IMAGEHIGH:
	      color[i]=m_cimagehigh;
	      break;
	    case CleanedState::CL_UNKNOWN:
	      // erm what to do ?
	      color[i]=black;
	      break;
	    }
	}
      else
	{
	  color[i]=m_cnotimage;
	}
    }

  QCamera::setData(value,color);
}

int 
NS_QComponents::QEventCamera::
getKeyFontPointSize(QPainter& p)
{
  static double cached_tubesize = 0;
  static double cached_scale    = 0;
  static int    cached_fontsize = 0;

  double tubesize = angularSize()/75.0*2.0;

  double scale = getScale(p);

  if((cached_tubesize != tubesize)||
     (cached_scale != scale))
    {
      cached_fontsize =	getFontPointSize(p,"",tubesize,0);
      cached_tubesize = tubesize;
      cached_scale = scale;
    }
  //  std::cerr << "Font: " << cached_tubesize << '/' << cached_fontsize << std::endl;
  return cached_fontsize;
}  

void 
NS_QComponents::QEventCamera::
drawKeyChannel(QPainter& p, 
	       double xc, double yc, double r, const QColor& c,
	       const std::string& text, double scale)
{
  drawTube(p, xc, yc, r, 1.0, c, scale, true, black);
  int ixc = scaleDistance(xc+1.2*r, scale);
  int iyc = scaleDistance(yc,       scale);
  p.setPen(foregroundColor());
  p.drawText(transX(ixc), transY(iyc), 0, 0, 
	     AlignLeft|AlignVCenter|DontClip, text.c_str());
}

void
NS_QComponents::QEventCamera::
drawMe(QPainter& p)
{
  if(camera() == 0)return;

  QCamera::drawMe(p);

  double scale=getScale(p);

  if((m_draw_color_key) && (m_imagecoloring))
    { 
      double r = angularSize()/75.0;

      double xc = -(angularSize()/2.04 - r);
      double yc =  (angularSize()/2.04 - r);

      int i=0;

      p.setFont(QFont("helvetica" /*"times"*/,
		      getKeyFontPointSize(p),
		      QFont::Normal));

      drawKeyChannel(p,xc,yc-(2.4*r*i),r,m_cimagehigh,"Picture",  scale);
      i++;
      drawKeyChannel(p,xc,yc-(2.4*r*i),r,m_cimagelow, "Boundary", scale);
      i++;

      if(!m_zerosuppress)
	{
	  drawKeyChannel(p,xc,yc-(2.4*r*i),r,m_cnotimage,"Non-image", scale);
	  i++;
	  drawKeyChannel(p,xc,yc-(2.4*r*i),r,m_cdisabled, "Disabled",  scale);
	  i++;
	}
    }
}

void 
NS_QComponents::QEventCamera::
setData(const EventChannelReps& ecr)
{
  m_event=ecr;
  updateCam();
}

void 
NS_QComponents::QEventCamera::
setZeroSuppression(bool zs)
{
  setAndUpdate(m_zerosuppress,zs);
}

void 
NS_QComponents::QEventCamera::
setColorDisabled(const QColor& c)
{
  setAndUpdate(m_cdisabled,c);
}

void 
NS_QComponents::QEventCamera::
setColorNotImage(const QColor& c)
{
  setAndUpdate(m_cnotimage,c);
}

void 
NS_QComponents::QEventCamera::
setColorImageLow(const QColor& c)
{
  setAndUpdate(m_cimagelow,c);
}

void 
NS_QComponents::QEventCamera::
setColorImageHigh(const QColor& c)
{
  setAndUpdate(m_cimagehigh,c);
}

void 
NS_QComponents::QEventCamera::
setImageColoring(bool c)
{
  setAndUpdate(m_imagecoloring,c);
}

void 
NS_QComponents::QEventCamera::
setDisplayDataSet(DisplayDataSet d)
{
  setAndUpdate(m_displayset,d);
}

void 
NS_QComponents::QEventCamera::
setDrawImageColorKey(bool key)
{
  setAndUpdate(m_draw_color_key,key);
}
