#include <qpainter.h>
#include <qpixmap.h>
#include <qbrush.h>
#include <qpaintdevicemetrics.h> 
#include <qfont.h>
#include <qnamespace.h>

#include"CameraConfiguration.h"
#include"QParamCamera.h"


NS_QComponents::QParamCamera::
QParamCamera(const CameraConfiguration* cam, QWidget *parent, 
	     const char *name):
      QEventCamera(cam,parent,name), m_hparam(), m_ellipsecolor(yellow),
      m_shownothing(false), m_showmajoraxis(true), m_showwidth(true), 
      m_showellipse(true), m_showasymmetry(true), m_showminorasymmetry(true), 
      m_showalpha(true)
{
  m_hparam.zero();
}

void
NS_QComponents::QParamCamera::
drawMe(QPainter& p)
{
  if(camera() == 0)return;

  double scale=getScale(p);

  QEventCamera::drawMe(p);

  if((!showNothing())&&(m_hparam.getLength()>0.0))
    {
      int x0=transX(scaleDistance(m_hparam.getX0(),scale));
      int y0=transY(scaleDistance(m_hparam.getY0(),scale));
      int xc=transX(scaleDistance(m_hparam.getXC()+m_hparam.getX0(),scale));
      int yc=transY(scaleDistance(m_hparam.getYC()+m_hparam.getY0(),scale));
      int l=scaleDistance(m_hparam.getLength(),scale);
      int w=scaleDistance(m_hparam.getWidth(),scale);
      int asyl=scaleDistance(m_hparam.getLength()*m_hparam.getAsymmetry(),
			    scale);
      int asyw=scaleDistance(m_hparam.getWidth()*m_hparam.getMinorAsymmetry(),
			    scale);

      // Calculate the intersection of the major-axis and the outside of the
      // camera. Ie solve the equation of the intersection of a line and a
      // circle... where the line is given by (parameterized by s)

      // X = X0+XC + CosPsi * s
      // Y = Y0+YC + SinPsi * s
      
      // so we solve for "s" the equation

      // X^2 + Y^2 = (AngularSize/2)^2

      const double dxc    = m_hparam.getXC()+m_hparam.getX0();
      const double dyc    = m_hparam.getYC()+m_hparam.getY0();
      const double as     = angularSize()/2;
      const double a      = m_hparam.getCosPsi();
      const double b      = m_hparam.getSinPsi();
      const double xCa    = a * dxc;
      const double yCb    = b * dyc;
      const double dist   = m_hparam.getDist();
      const double length = m_hparam.getLength();
      
      const double s_root = xCa*xCa + yCb*yCb + 2*xCa*yCb - dist*dist + as*as;
      
      double s1;
      double s2;

      if(s_root < 0) 
	{
	  // should never happen... it would mean the centroid is outside the
	  // camera... actually it *can* happen if you artificially zoom in
	  s1 = -length;
	  s2 = +length;
	}
      else
	{
	  s1 = -(xCa+yCb) - sqrt(s_root);
	  s2 = -(xCa+yCb) + sqrt(s_root);

	  if(s1 > -length)s1=-length;
	  if(s2 <  length)s2= length;
	}
      
      int m1 = scaleDistance(s1,scale);
      int m2 = scaleDistance(s2,scale);

      int penwidth = getSuggestedPenWidth(p,3.0);

      p.save();
      p.setPen(QPen(QColor("black"),penwidth));
      if(showAlpha())p.drawLine(x0,y0,xc,yc);
      p.translate(xc,yc);
      p.rotate(atan2(-m_hparam.getSinPsi(),m_hparam.getCosPsi())/M_PI*180.0);
      //      if(showMajorAxis())p.drawLine(-3*as,0,3*as,0);
      if(showMajorAxis())p.drawLine(m1,0,m2,0);
      if(showWidth())p.drawLine(0,-w,0,w);
      p.setPen(QPen(m_ellipsecolor,penwidth));
      if(showEllipse())p.drawEllipse(-l,-w,2*l,2*w);
      if(showAsymmetry())p.drawLine(transX(asyl),-w/2,transX(asyl),w/2);
      if(showMinorAsymmetry())p.drawLine(-w/2,transY(asyw),w/2,transY(asyw));
      p.restore();
    }
}

void 
NS_QComponents::QParamCamera::
setData(const EventChannelReps& ecr, const HillasParam& hp)
{
  m_hparam=hp;
  QEventCamera::setData(ecr);
}

void 
NS_QComponents::QParamCamera::
setColorEllipse(const QColor& c)
{
  setAndRepaint(m_ellipsecolor,c);
}

void NS_QComponents::QParamCamera::setShowNothing(bool s)
{
  setAndRepaint(m_shownothing,s);
}

void NS_QComponents::QParamCamera::setShowMajorAxis(bool s)
{
  setAndRepaint(m_showmajoraxis,s);
}

void NS_QComponents::QParamCamera::setShowWidth(bool s)
{
  setAndRepaint(m_showwidth,s);
}

void NS_QComponents::QParamCamera::setShowEllipse(bool s)
{
  setAndRepaint(m_showellipse,s);
}

void NS_QComponents::QParamCamera::setShowAsymmetry(bool s)
{
  setAndRepaint(m_showasymmetry,s);
}

void NS_QComponents::QParamCamera::setShowMinorAsymmetry(bool s)
{
  setAndRepaint(m_showminorasymmetry,s);
}

void NS_QComponents::QParamCamera::setShowAlpha(bool s)
{
  setAndRepaint(m_showalpha,s);
}
