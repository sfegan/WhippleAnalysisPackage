///////////////////////////////////////////////////////////////////////////////

#include<string>

#include<Types.h>
#include<CameraConfiguration.h>
#include<ChannelData.h>
#include<RedEvent.h>
#include<RedFile.h>
#include<Exceptions.h>

namespace NS_Analysis {

  class FinderError: public Error
  {
  public:
    FinderError(const std::string& err): Error(err) {}
  };

  class CameraFinder
  {
  public:
    std::string defaultDirectory() const;

    std::string filenameByNChannels(channelnum_type nchannels) const;
    std::string filenameForRedFile(RedFile* rf) const;

    CameraConfiguration* cameraByNChannels(channelnum_type nchannels) const;
    CameraConfiguration* cameraForRedFile(RedFile* rf) const;
  };
  
}

std::string
NS_Analysis::CameraFinder::
filenameForRedFile(RedFile* rf) const
{
  int rfEventVersion=rf->events()->tVersion();
  int nchannels=RedEvent::sizeADC(rfEventVersion);
  return filenameByNChannels(nchannels);
}

std::string
NS_Analysis::CameraFinder::
filenameByNChannels(channelnum_type nchannels) const
{
  std::string filename;
  if((nchannels == 492) || (nchannels == 490))filename="WC490.h5";
  else if((nchannels == 336) || (nchannels == 331))filename="WC331.h5";
  else if((nchannels == 156) || (nchannels == 151))filename="WC151.h5";
  else if((nchannels == 120) || (nchannels == 109))filename="WC109.h5";
  else throw FinderError("No camera found");

  filename="/home/sfegan/Projects/analysis/package/bin/"+filename;
  //filename="/home/observer/sfegan/package/bin/"+filename;

  return filename;
}

NS_Analysis::CameraConfiguration* 
NS_Analysis::CameraFinder::
cameraForRedFile(RedFile* rf) const
{
  return new CameraConfiguration(filenameForRedFile(rf));
}

NS_Analysis::CameraConfiguration* 
NS_Analysis::CameraFinder::
cameraByNChannels(channelnum_type nchannels) const
{
  return new CameraConfiguration(filenameByNChannels(nchannels));
}


///////////////////////////////////////////////////////////////////////////////

#include <qwidget.h>
#include <qmainwindow.h>
#include <qmime.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qkeycode.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qprinter.h>
#include <qapplication.h>
#include <qaccel.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qwhatsthis.h>
#include <qwindowsstyle.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qsizepolicy.h>
#include <qtimer.h>
#include <qcolor.h>
#include <qcolordialog.h>
#include <qaccel.h>

#include <qwt/qwt_knob.h>
#include <qwt/qwt_slider.h>

#include <unistd.h>
#include <getopt.h>

#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>
#include <vector>

#include <Types.h>
#include <Exceptions.h>
#include <VSFA.h>
#include <RedEvent.h>
#include <RedHeader.h>
#include <RedFile.h>
#include <ParamFile.h>
#include <EventInfo.h>
#include <Pedestals.h>
#include <Gains.h>
#include <PedsAndGainsFactory.h>
#include <ODBCPedsAndGainsFactory.h>
#include <Random.h>
#include <ChannelData.h>
#include <ChannelRepresentation.h>
#include <Cleaning.h>
#include <CameraConfiguration.h>
#include <HillasParam.h>
#include <HillasParameterization.h>

#include <QParamCamera.h>

#include "qteventviewer_loadfile.h"

using namespace NS_Analysis;
using namespace NS_QComponents;

class EventSelector : public QWidget
{
  Q_OBJECT

public:
  EventSelector(QWidget *parent, char* name=0);
  virtual ~EventSelector();

  int getEventNumber() const;

  virtual QSize sizeHint () const;
  //  virtual QSize maximumSize () const;
  virtual QSize minimumSize () const;

  void deactivate();
  void setupForAllEvents(int nevents);
  void setupForSelectedEvents(const std::vector<int> eventlist);

signals:
  void newEventNumber(int);

public slots:
  void nextEvent();
  void prevEvent();

private slots:
  void next();
  void last();
  void play();
  void stop();

  void sliderMoved(double);
  void sliderTentativelyMoved(double);

private:
  void activateComponents();
  void setButtonStates();

  bool                   m_active;

  bool                   m_playing;

  int                    m_nevents;
  bool                   m_use_event_list;
  std::vector<int>            m_event_list;

  // UI Components
  QHBoxLayout*           layoutMain;
  QGridLayout*           layoutSlider;
  QGridLayout*           layoutPlayControls;

  QPushButton*           buttonSlLast;
  QPushButton*           buttonSlNext;
  QwtSlider*             sliderMain;
  QLabel*                labelEvNum;
  QTimer*                timerPlaying;

  QHBoxLayout*           layoutSpeedSlider;
  QwtSlider*             sliderPCSpeed;
  QLabel*                labelPCSpeed;

  QPushButton*           buttonPCPlay;
  QPushButton*           buttonPCStop;
};

EventSelector::EventSelector(QWidget *parent, char* name): 
  QWidget(parent,name), m_use_event_list(false), m_active(false), 
  m_playing(false)
{
  // Create all the components

  layoutMain = new QHBoxLayout(this);

  layoutSlider = new QGridLayout(0,4,3);

  buttonSlLast = new QPushButton("Last",this);
  sliderMain   = new QwtSlider(this, 0, QwtSlider::Horizontal, QwtSlider::Left,
			       QwtSlider::BgSlot|QwtSlider::BgTrough);
  sliderMain -> setThumbWidth(buttonSlLast->height());
  sliderMain -> setFixedHeight(buttonSlLast->height());
  sliderMain -> setTracking(false);
  buttonSlNext = new QPushButton("Next",this);
  labelEvNum   = new QLabel("",this);
  labelEvNum -> setAlignment(AlignHCenter|AlignVCenter);

  layoutSlider -> addRowSpacing(0, 0);
  layoutSlider -> addWidget(buttonSlLast, 1, 0);
  layoutSlider -> addWidget(sliderMain,   1, 1);
  layoutSlider -> addWidget(buttonSlNext, 1, 2);
  layoutSlider -> addWidget(labelEvNum,   2, 1);
  layoutSlider -> addRowSpacing(3, 0);

  layoutSlider -> setColStretch(0,0);
  layoutSlider -> setColStretch(1,1);
  layoutSlider -> setColStretch(2,0);
  layoutSlider -> setRowStretch(1,0);
  layoutSlider -> setRowStretch(2,0);

  layoutSlider -> setSpacing(1);

  layoutPlayControls = new QGridLayout(0,2,1);
  
  layoutSpeedSlider = new QHBoxLayout(0);
  sliderPCSpeed = new QwtSlider(this, 0, QwtSlider::Horizontal, 
				QwtSlider::Left,
				QwtSlider::BgSlot|QwtSlider::BgTrough);
  labelPCSpeed = new QLabel("Speed",this);
  labelPCSpeed -> setAlignment(AlignHCenter|AlignVCenter);

  sliderPCSpeed -> setThumbWidth(labelPCSpeed->height());
  sliderPCSpeed -> setFixedHeight(labelPCSpeed->height());
  sliderPCSpeed -> setRange(-1,1.0,0.1,1);
  sliderPCSpeed -> setValue(0.0);

  layoutSpeedSlider -> addWidget(labelPCSpeed,0);
  layoutSpeedSlider -> addSpacing(3);
  layoutSpeedSlider -> addWidget(sliderPCSpeed,0);

  buttonPCPlay  = new QPushButton("Play",this);
  buttonPCStop  = new QPushButton("Stop",this);
  timerPlaying = new QTimer(this);

  buttonPCPlay -> setFixedWidth(3*buttonPCPlay->sizeHint().width()/2);
  buttonPCStop -> setFixedWidth(3*buttonPCPlay->sizeHint().width()/2);

  int s=buttonPCPlay->sizeHint().width() + buttonPCStop->sizeHint().width();

  layoutPlayControls -> addMultiCellLayout(layoutSpeedSlider,  0,0,0,1);
  layoutPlayControls -> addWidget(buttonPCStop, 1,0);
  layoutPlayControls -> addWidget(buttonPCPlay, 1,1);

  layoutMain -> addLayout(layoutSlider,1);
  layoutMain -> addSpacing(20);
  layoutMain -> addLayout(layoutPlayControls,0);

  setFixedHeight(layoutMain->sizeHint().height());

  // Connect up the components

  connect(buttonPCPlay,SIGNAL(clicked()),this,SLOT(play()));
  connect(buttonPCStop,SIGNAL(clicked()),this,SLOT(stop()));
  connect(buttonSlLast,SIGNAL(clicked()),this,SLOT(last()));
  connect(buttonSlNext,SIGNAL(clicked()),this,SLOT(next()));

  connect(sliderMain,SIGNAL(valueChanged(double)), 
	  this, SLOT(sliderMoved(double)));

  connect(sliderMain,SIGNAL(sliderMoved(double)), 
	  this, SLOT(sliderTentativelyMoved(double)));

  connect(timerPlaying,SIGNAL(timeout()),this,SLOT(next()));

  // Set the state of the components to inactive
  activateComponents();
}

EventSelector::~EventSelector()
{
  // No need to delete any of the Widget components that we created, they
  // are deleted automagically by Qt
}

void EventSelector::activateComponents()
{
  buttonPCStop  -> setEnabled(m_active);
  buttonPCPlay  -> setEnabled(m_active);
  labelPCSpeed  -> setEnabled(m_active);
  sliderPCSpeed -> setEnabled(m_active);
  labelEvNum    -> setEnabled(m_active);
  sliderMain    -> setEnabled(m_active);
  buttonSlNext  -> setEnabled(m_active);
  buttonSlLast  -> setEnabled(m_active);
}

QSize EventSelector::sizeHint () const
{
  return layoutMain->sizeHint();
}

QSize EventSelector::minimumSize () const
{
  return QSize(200,200);
}


int EventSelector::getEventNumber() const
{
  int num=int(sliderMain->value());
  if(m_use_event_list)num=m_event_list[num];
  return num+1; 
}

void EventSelector::nextEvent()
{
  if(!m_active)return;
  if(m_playing)return;
  if(sliderMain->value() == sliderMain->maxValue())return;
  next();
}

void EventSelector::prevEvent()
{
  if(!m_active)return;
  if(m_playing)return;
  if(sliderMain->value() == 0)return;
  last();
}

void EventSelector::next()
{
  sliderMain -> setTracking(true);
  sliderMain -> incValue(1);
  sliderMain -> setTracking(false);
}

void EventSelector::last()
{
  sliderMain -> setTracking(true);
  sliderMain -> incValue(-1);
  sliderMain -> setTracking(false);
}

void EventSelector::play()
{
  buttonPCStop -> setEnabled(true);
  buttonPCPlay -> setEnabled(false);
  buttonSlNext -> setEnabled(false);
  buttonSlLast -> setEnabled(false);
  sliderMain -> setEnabled(false);
  
  m_playing = true;

  timerPlaying->start(int(pow(10,3-sliderPCSpeed->value())),true);
}

void EventSelector::stop()
{
  timerPlaying->stop();

  m_playing = false;
  buttonPCStop -> setEnabled(false);
  sliderMain -> setEnabled(true);
  if(sliderMain->value() != sliderMain->maxValue())
    {
      buttonPCPlay -> setEnabled(true);
      buttonSlNext -> setEnabled(true);
    }
  if(sliderMain->value() != sliderMain->minValue())
    {
      buttonSlLast -> setEnabled(true);
    }
}

void EventSelector::sliderTentativelyMoved(double dnum)
{
  int num=int(dnum);
  if(m_use_event_list)num=m_event_list[num];
  labelEvNum->setNum(num+1);
}

void EventSelector::sliderMoved(double dnum)
{
  int num=int(dnum);
  if(m_use_event_list)num=m_event_list[num];

  if(sliderMain->value() == sliderMain->maxValue())
    {
      if(m_playing)stop();
      buttonPCPlay -> setEnabled(false);
      buttonSlNext -> setEnabled(false);
    }
  else if(!m_playing) 
    {
      buttonSlNext -> setEnabled(true);
      buttonPCPlay -> setEnabled(true);
    }

  if(sliderMain->value() == sliderMain->minValue())
    buttonSlLast -> setEnabled(false);
  else if(!m_playing)
    buttonSlLast -> setEnabled(true);

  labelEvNum->setNum(num+1);
  emit newEventNumber(num);

  if(m_playing)timerPlaying->start(int(pow(10,3-sliderPCSpeed->value())),true);
}

void EventSelector::setupForSelectedEvents(const std::vector<int> eventlist)
{
  if(eventlist.size() == 0)
    {
      QMessageBox::warning(this,"No events",
			   "No events were selected in the list");
      return;
    }
  
  for(std::vector<int>::const_iterator i=eventlist.begin(); i!=eventlist.end();i++)
    {
      if(*i < 0)
	{
	  QMessageBox::warning(this,"Out of range",
			       "One selected event number was less than zero");
	  return;
	}
      if(*i >= m_nevents)
	{
	  QMessageBox::warning(this,"Out of range",
		 "One selected event number larger than the number of events");
	  return;
	}
    }

  // Input list has been checked, we are good to go

    m_playing = false;
    m_use_event_list = true;
    m_event_list = eventlist;
    
    sliderMain->setRange(0,eventlist.size()-1,1,20);
    sliderMain->setValue(0);

    m_active=true;
    activateComponents();
    stop();
    sliderMoved(0);
}

void EventSelector::setupForAllEvents(int nevents)
{
  m_playing = false;
  m_use_event_list = false;

  m_nevents=nevents;
  sliderMain->setRange(0,nevents-1,1,20);
  sliderMain->setValue(0);

  m_active=true;
  activateComponents();
  stop();
  sliderMoved(0);
}

void EventSelector::deactivate()
{
  m_active=false;
  activateComponents();
  sliderMain->setRange(0,100,1,1);
  sliderMain->setValue(0);
  labelEvNum->setText("");
}

///////////////////////////////////////////////////////////////////////////////

#include "pixmaps/fileopen.xpm"
#include "pixmaps/fileprint.xpm"

class EventViewer : public QMainWindow
{
  Q_OBJECT
  
public:
  EventViewer(PedsAndGainsFactory *fac,
	      QWidget *parent, char* name=0);

  virtual ~EventViewer();

public slots:
  void gotoEvent(int num);
  void load(const std::string& filename);

private slots:
  void load();
  void load(const std::string& filename, 
	    int pedsdate, const std::string& pedsname,
	    int gaindate, const std::string& gainname,
	    const std::string& cameraname, RedFile *prf=0);

  void close();
    
  void printEvent();

  void viewZeroSuppression();
  void viewImageColoring();
  void viewDataSet(int set);
  void viewParameters();
  void viewMajorAxis();
  void viewWidthAxis();
  void viewEllipse();
  void viewAsymmetry();
  void viewOriginLine();
  void viewKey();
  void viewLabels();
  void viewLockScale();

  void viewStandardColors(int n);
  void viewStandardStyles(int n);
  void viewCustomColors(int n);

  void cutsAllEvents();
  void cutsAllCodeX(int code);
  void cutsEventsFromParamFile();

private:
  void initiate(PedsAndGainsFactory *fac,
		QWidget *parent, char* name);
  
  QPrinter*                    printer;

  EventSelector*               eventselector;
  QParamCamera*                paramcamera;

  QColor                       m_defcol_ellipse;
  QColor                       m_defcol_disabled;
  QColor                       m_defcol_normal;
  QColor                       m_defcol_imhigh;
  QColor                       m_defcol_imlow;

  PedsAndGainsFactory*         m_pgfact;

  std::string                       m_filename;
  MPtr<RedFile>                m_rf;
  MPtr<CameraConfiguration>    m_cam;
  MPtr<HillasParameterization> m_hillas;
  MPtr<Cleaner>                m_cleaner; // should not be here!!
  MPtr<ECRGenerator>           m_ecrgen;

  bool                         m_scalelocked;
};

EventViewer::EventViewer(PedsAndGainsFactory* fac,
			 QWidget *parent, char* name)
  : QMainWindow(parent,name), m_pgfact(fac), 
    m_rf(0), m_cam(0), m_hillas(0), m_ecrgen(0), m_scalelocked(false)
{
  initiate(fac,parent,name);
}

void EventViewer::initiate(PedsAndGainsFactory* fac,
			   QWidget *parent, char* name)
{
  int id;

  QFrame* frame           = new QFrame(this);
  QVBoxLayout* layoutMain = new QVBoxLayout(frame);

  // Event Selector

  eventselector=new EventSelector(frame);

  QAccel* a=new QAccel(this);
  id = a->insertItem(Key_Right);
  a->connectItem(id, eventselector, SLOT(nextEvent()));
  id = a->insertItem(Key_Left);
  a->connectItem(id, eventselector, SLOT(prevEvent()));

  // Camera Widget

  paramcamera=new QParamCamera(m_cam.get(),frame);
  paramcamera->disableRepaint();

  layoutMain -> addWidget(paramcamera,1);
  layoutMain -> addSpacing(20);
  layoutMain -> addWidget(eventselector,0);

  setCentralWidget(frame);

  connect(eventselector,SIGNAL(newEventNumber(int)),
	  this, SLOT(gotoEvent(int)));

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////// PRINTER /////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  printer = new QPrinter;
#if (QT_VERSION > 300)
  printer->setResolution(600);
#endif
  printer->setCreator("qteventviewer");
  printer->setColorMode(QPrinter::Color);
  printer->setPageSize(QPrinter::Letter);

  /////////////////////////////////////////////////////////////////////////////
  //////////////////////////////// COLORS /////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  QColorDialog::setCustomColor(0, paramcamera->colorDisabled().rgb());
  QColorDialog::setCustomColor(1, paramcamera->colorNotImage().rgb());
  QColorDialog::setCustomColor(2, paramcamera->colorImageHigh().rgb());
  QColorDialog::setCustomColor(3, paramcamera->colorImageLow().rgb());
  QColorDialog::setCustomColor(4, black.rgb());
  QColorDialog::setCustomColor(5, paramcamera->colorEllipse().rgb());

  m_defcol_ellipse   = paramcamera->colorEllipse();
  m_defcol_disabled  = paramcamera->colorDisabled();
  m_defcol_normal    = paramcamera->colorNotImage(); 
  m_defcol_imhigh    = paramcamera->colorImageHigh();
  m_defcol_imlow     = paramcamera->colorImageLow();

  /////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////// ICONS /////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  QPixmap printIcon = QPixmap( fileprint );
  QPixmap openIcon  = QPixmap( fileopen );
  
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////// MENU BARS ///////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  // FILE MENU

  QPopupMenu * file = new QPopupMenu( this );
  menuBar()->insertItem( "&File", file );

  id = file->insertItem( openIcon, "&Open",
			 this, SLOT(load()), CTRL+Key_O, 1 );
  id = file->insertItem( "&Close",
			 this, SLOT(close()), CTRL+Key_W, 2 );
  
  // file->insertSeparator();
  id = file->insertItem( printIcon, "&Print",
			 this, SLOT(printEvent()), CTRL+Key_P, 3 );
  //  file->setWhatsThis( id, filePrintText );

  file->insertSeparator();
  // file->insertItem( "&Close", this, SLOT(close()), CTRL+Key_W );
  file->insertItem( "&Quit", qApp, SLOT( closeAllWindows() ), CTRL+Key_Q );

  // DATA MENU

  QPopupMenu* data = new QPopupMenu( this );
  menuBar()->insertItem( "&Data", data, 4 );

  QPopupMenu* datacuts = new QPopupMenu(data);
  data->insertItem("Cuts", datacuts);

  datacuts->insertItem("All Events",                 
		       this, SLOT(cutsAllEvents()));
  datacuts->insertItem("Code 1",                 
		       this, SLOT(cutsAllCodeX(int)), 0, 111);
  datacuts->insertItem("Code 8",                 
		       this, SLOT(cutsAllCodeX(int)), 0, 112);
  datacuts->insertItem("Events from parameterized file", 
		       this, SLOT(cutsEventsFromParamFile()));

  datacuts->setItemParameter(111,1);
  datacuts->setItemParameter(112,8);

  // VIEW MENU

  QPopupMenu * view = new QPopupMenu( this );
  menuBar()->insertItem( "&View", view, 5 );
  
  view->insertItem( "Zero Suppression", this, SLOT(viewZeroSuppression()), 
		    CTRL+Key_Z,100);
  view->insertItem( "Image Coloring",this, SLOT(viewImageColoring()), 
		    CTRL+Key_C,101);

  view->insertItem( "Key", this, SLOT(viewKey()), CTRL+Key_K, 50);
  view->setItemChecked(50,paramcamera->drawRangeKey());
  paramcamera->setRangeUnits("DC");
  paramcamera->setDrawImageColorKey(paramcamera->drawRangeKey());

  view->insertItem( "Tube numbers", this, SLOT(viewLabels()), CTRL+Key_N, 51);
  view->setItemChecked(51,paramcamera->labelling());

  view->insertItem( "Lock scale", this, SLOT(viewLockScale()), CTRL+Key_L, 52);
  view->setItemChecked(52,false);
  
  view->insertSeparator();

  // VIEW->DATASET SUB MENU

  QPopupMenu* dataset = new QPopupMenu(view);
  view->insertItem("Dataset", dataset);

  dataset->insertItem("Raw",                 
		      this, SLOT(viewDataSet(int)), 0,120);
  dataset->insertItem("Pedestal Subtracted", 
		      this, SLOT(viewDataSet(int)), 0,121);
  dataset->insertItem("Gain Adjusted",
		      this, SLOT(viewDataSet(int)), 0,122);

  dataset->setItemChecked(120,(paramcamera->displayDataSet() == 
			       QEventCamera::SIG_RAW)? true:false);
  dataset->setItemChecked(121,(paramcamera->displayDataSet() == 
			       QEventCamera::SIG_SUBTRACTED)? true:false);
  dataset->setItemChecked(122,(paramcamera->displayDataSet() == 
			       QEventCamera::SIG_GAINADJUSTED)? true:false);

  dataset->setItemParameter(120,0);
  dataset->setItemParameter(121,1);
  dataset->setItemParameter(122,2);
  
  // END OF SUB MENU

  view->insertSeparator();

  view->insertItem( "Parameters", this, SLOT(viewParameters()), CTRL+Key_H,200);
  view->insertItem( "Major Axis", this, SLOT(viewMajorAxis()),  0,201);
  view->insertItem( "Minor Axis", this, SLOT(viewWidthAxis()),  0,202);
  view->insertItem( "Ellipse",    this, SLOT(viewEllipse()),    0,203);
  view->insertItem( "Asymmetry",  this, SLOT(viewAsymmetry()),  0,204);
  view->insertItem( "Origin",     this, SLOT(viewOriginLine()), 0,205);

  view->setItemChecked(100, paramcamera->zeroSuppression());
  view->setItemChecked(101, paramcamera->imageColoring());

  view->setItemChecked(200, !paramcamera->showNothing());
  view->setItemChecked(201, paramcamera->showMajorAxis());
  view->setItemChecked(202, paramcamera->showWidth());
  view->setItemChecked(203, paramcamera->showEllipse());
  view->setItemChecked(204, paramcamera->showAsymmetry());
  paramcamera->setShowMinorAsymmetry(paramcamera->showAsymmetry());
  view->setItemChecked(205, paramcamera->showAlpha());
  
  view->setItemEnabled(201, !paramcamera->showNothing());
  view->setItemEnabled(202, !paramcamera->showNothing());
  view->setItemEnabled(203, !paramcamera->showNothing());
  view->setItemEnabled(204, !paramcamera->showNothing());
  view->setItemEnabled(205, !paramcamera->showNothing());

  view->insertSeparator();

  // VIEW->STYLE SUB MENU

  QPopupMenu* standardstyle = new QPopupMenu(view);
  view->insertItem( "Styles", standardstyle);

  standardstyle->insertItem( "Show value by area",
			   this, SLOT(viewStandardStyles(int)), 0,330);
  standardstyle->insertItem( "Show value by intensity", 
			   this, SLOT(viewStandardStyles(int)), 0,331);
  
  standardstyle->setItemParameter(330,0);
  standardstyle->setItemParameter(331,1);

  // VIEW->COLOR SUB MENU

  QPopupMenu* standardcolor = new QPopupMenu(view);
  view->insertItem( "Color Schemes", standardcolor);

  standardcolor->insertItem( "Default",  
			   this, SLOT(viewStandardColors(int)), 0,320);
  standardcolor->insertItem( "Printing (Color)",   
			   this, SLOT(viewStandardColors(int)), 0,322);
  standardcolor->insertItem( "Printing (Greyscale)", 
			   this, SLOT(viewStandardColors(int)), 0,321);
  standardcolor->insertItem( "Printing (B/W)", 
			   this, SLOT(viewStandardColors(int)), 0,323);
  
  standardcolor->setItemParameter(320,0);
  standardcolor->setItemParameter(321,1);
  standardcolor->setItemParameter(322,2);
  standardcolor->setItemParameter(323,3);

  QPopupMenu* customcolor = new QPopupMenu(view);
  view->insertItem( "Custom Colors", customcolor);

  customcolor->insertItem( "Disabled Channels",  
			   this, SLOT(viewCustomColors(int)), 0,300);
  customcolor->insertItem( "Non-image Channels", 
			   this, SLOT(viewCustomColors(int)), 0,301);
  customcolor->insertItem( "Picture Channels",   
			   this, SLOT(viewCustomColors(int)), 0,302);
  customcolor->insertItem( "Boundary Channels",  
			   this, SLOT(viewCustomColors(int)), 0,303);
  customcolor->insertItem( "Ellipse",            
			   this, SLOT(viewCustomColors(int)), 0,304);

  customcolor->setItemParameter(300,0);
  customcolor->setItemParameter(301,1);
  customcolor->setItemParameter(302,2);
  customcolor->setItemParameter(303,3);
  customcolor->setItemParameter(304,4);

  // LETS GO!

  close();

  statusBar()->message( "Ready", 2000 );
  paramcamera->enableRepaint();
}

EventViewer::~EventViewer()
{
  delete printer;
}

void EventViewer::load()
{
  QString filename = 
    QFileDialog::getOpenFileName(m_filename.c_str(),
				 "HDF5 Files (*.h5);;All Files (*)",
				 this,"","Load Data FIle");
  if(filename.isEmpty())return;

  load(filename.ascii());
}

void EventViewer::load(const std::string& filename)
{
  RedFile* rf = new RedFile(100);
  
  try
    {
      rf->open(filename);
    }
  catch(const Error& x)
    {
      QMessageBox::warning(this,"File Error",x.message().c_str());
      delete rf;
      return;
    }

  RedHeader rh;
  rf->header()->read(0,&rh,1);
  int idate = rh.date();
  fixDate2000(idate);
  
  std::ostringstream ost;
  ost << idate;

  std::vector<std::string> gains = m_pgfact -> listGainsByDate(idate);

  CameraFinder finder;

  LoadDialog ldialog(this);
  int r = ldialog.exec(std::string(filename),
		       ost.str(),
		       getFilenameBase(std::string(filename)),
		       gains,
		       finder.filenameForRedFile(rf).c_str());

  if(r != QDialog::Accepted)return;
  
  int pedsdate;
  std::string pedsfile;
  int gainsdate;
  std::string gainsfile;
  std::string camerafile;

  std::istringstream(ldialog.pedsDate()) >> pedsdate;
  pedsfile = ldialog.pedsFile();

  if(ldialog.gainsApply())
    {
      std::istringstream(ldialog.gainsDate()) >> gainsdate;
      gainsfile = ldialog.gainsFile();
    }

  camerafile = ldialog.cameraFile();

  load(std::string(filename),pedsdate,pedsfile,gainsdate,gainsfile,camerafile,rf);
}

void EventViewer::load(const std::string& filename, 
		       int pedsdate, const std::string& pedsname,
		       int gainsdate, const std::string& gainsname,
		       const std::string& cameraname, RedFile *prf)
{
  // DATA FILE
  MPtr<RedFile> rf;
  rf.manage(prf);

  if(rf.get()==0)
    {
      rf.manage(new RedFile(100));
      
      try
	{
	  rf->open(std::string(filename));
	}
      catch(const Error& x)
	{
	  QMessageBox::warning(this,"File Error",x.message().c_str());
	  return;
	}
    }

  // CAMERA
  MPtr<CameraConfiguration> cam;
  try
    {
      cam.manage(new CameraConfiguration(cameraname));
    }
  catch(const Error& x)
    {
      QMessageBox::warning(this,"Camera Error",x.message().c_str());
      return;
    }
    
  // PEDESTALS
  MPtr<Pedestals> peds;
  peds.manage(m_pgfact->loadPeds(pedsname,pedsdate));
  if(peds.get() == 0)
    {
      std::ostringstream ost;
      ost << "Pedestals for " << pedsname 
	  << " were not found on date " << pedsdate;

      QMessageBox::warning(this,"Pedestals",ost.str().c_str());
      return;
    }
      
  // GAINS
  MPtr<Gains> gains;

  if((gainsname != "")&&(gainsdate != 0))
    {
      gains.manage(m_pgfact->loadGains(gainsname,gainsdate));

      if(gains.get() == 0)
	{
	  std::ostringstream ost;
	  ost << "Gains for " << gainsname 
	      << " were not found on date " << gainsdate;

	  QMessageBox::warning(this,"Gains",ost.str().c_str());
	  return;
	}
    }
  else
    {
      gains.manage(new Gains(cam->nchannels()));
      for(int c=0;c<cam->nchannels();c++)gains->val(c)=1.0;
    }

  // CLEANER
  MPtr<Cleaner> cleaner;

  double pic = 4.25;
  double bnd = 2.25;
  cleaner.manage(new CleanerPicBnd(cam.get(), pic, bnd));
  //cleaner.manage(new CleanerRegional(cam.get(), 5.0,2.25,3));
  
  // CHANNEL REP GENERATOR
  MPtr<ECRGenerator> ecrgen;
  bool usepadding = 0;
  if(usepadding)
    {
      /*
	int pad_date=rh.date();
	MPtr<Pedestals> padpeds;
	padpeds.manage(pgfact.loadPeds(pad_name,pad_date));
	
	if(padpeds.get() == 0)
	{
	cout << std::endl
	<< "Pedestals could not be found!" << std::endl;
	exit(EXIT_FAILURE);
	}
	
	if(seed==0)seed=time(0);
	cout << "RNG Seed:         " << seed << std::endl;
	
	MPtr<LinearRNG> lrng;
	lrng.manage(new NRRand2(seed), true);

	MPtr<GaussianRNG> rng;
        rng.manage(new BMLinearRNGToGaussianRNGAdaptor(lrng),true);
	seed=0;
	
	ecrgen.manage(new Padding_ECRGenerator(cam.get(),&cleaner,
	rng,gains.get(),
	peds.get(),padpeds.get()));
      */
    }
  else
    {
      ecrgen.manage(new Standard_ECRGenerator(cam.get(),cleaner.get(),
					      gains.get(),peds.get()));
    }

  // HILLAS PARAMETERIZATION
  MPtr<HillasParameterization> hillas;
  hillas.manage(new HillasParameterization(cam.get()));
	   
  // SETUP THE CLASS FOR THIS FILE

  close();

  m_filename=filename;
  setCaption(("QTEventViewer: "+filename).c_str());
  m_rf=rf;
  m_cam=cam;

  paramcamera->disableRepaint();
  paramcamera->setCamera(m_cam.get());
  m_cleaner=cleaner; // should not be here!!
  m_ecrgen=ecrgen;
  m_hillas=hillas;

  eventselector->setupForAllEvents(m_rf->events()->size());

  menuBar()->setItemEnabled(2,true);
  menuBar()->setItemEnabled(3,true);
  menuBar()->setItemEnabled(4,true);
  menuBar()->setItemEnabled(5,true);
  paramcamera->enableRepaint();
}

void EventViewer::close()
{
  menuBar()->setItemEnabled(2,false);
  menuBar()->setItemEnabled(3,false);
  menuBar()->setItemEnabled(4,false);
  menuBar()->setItemEnabled(5,false);

  m_hillas.reset();
  m_ecrgen.reset();
  m_cleaner.reset();
  m_cam.reset();
  paramcamera->setCamera(0);
  m_rf.reset();
  setCaption("QTEventViewer: No File");

  eventselector->deactivate();
}

void EventViewer::viewZeroSuppression()
{
  bool check=!menuBar()->isItemChecked(100);
  menuBar()->setItemChecked(100,check);
  paramcamera->setZeroSuppression(check);
}

void EventViewer::viewImageColoring()
{
  bool check=!menuBar()->isItemChecked(101);
  menuBar()->setItemChecked(101,check);
  paramcamera->setImageColoring(check);
}

void EventViewer::viewDataSet(int set)
{
  menuBar()->setItemChecked(120, (set==0)?true:false);
  menuBar()->setItemChecked(121, (set==1)?true:false);
  menuBar()->setItemChecked(122, (set==2)?true:false);

  if(set==0)paramcamera->setDisplayDataSet(QParamCamera::SIG_RAW);
  if(set==1)paramcamera->setDisplayDataSet(QParamCamera::SIG_SUBTRACTED);
  if(set==2)paramcamera->setDisplayDataSet(QParamCamera::SIG_GAINADJUSTED);
}

void EventViewer::viewParameters()
{
  bool check=!menuBar()->isItemChecked(200);
  menuBar()->setItemChecked(200,check);

  menuBar()->setItemEnabled(201, check);
  menuBar()->setItemEnabled(202, check);
  menuBar()->setItemEnabled(203, check);
  menuBar()->setItemEnabled(204, check);
  menuBar()->setItemEnabled(205, check);

  paramcamera->setShowNothing(!check);
}

void EventViewer::viewMajorAxis()
{
  bool check=!menuBar()->isItemChecked(201);
  menuBar()->setItemChecked(201,check);
  paramcamera->setShowMajorAxis(check);
}

void EventViewer::viewWidthAxis()
{
  bool check=!menuBar()->isItemChecked(202);
  menuBar()->setItemChecked(202,check);
  paramcamera->setShowWidth(check);
}

void EventViewer::viewEllipse()
{
  bool check=!menuBar()->isItemChecked(203);
  menuBar()->setItemChecked(203,check);
  paramcamera->setShowEllipse(check);
}

void EventViewer::viewAsymmetry()
{
  bool check=!menuBar()->isItemChecked(204);
  menuBar()->setItemChecked(204,check);

  paramcamera->disableRepaint();
  paramcamera->setShowAsymmetry(check);
  paramcamera->setShowMinorAsymmetry(check);
  paramcamera->enableRepaint();
}

void EventViewer::viewKey()
{
  bool check=!menuBar()->isItemChecked(50);
  menuBar()->setItemChecked(50,check);
  paramcamera->disableRepaint();
  paramcamera->setDrawRangeKey(check);
  paramcamera->setDrawImageColorKey(check);
  paramcamera->enableRepaint();
}

void EventViewer::viewLabels()
{
  bool check=!menuBar()->isItemChecked(51);
  menuBar()->setItemChecked(51,check);
  paramcamera->disableRepaint();
  paramcamera->setLabelling(check);
  paramcamera->enableRepaint();
}

void EventViewer::viewLockScale()
{
  m_scalelocked=!m_scalelocked;
  menuBar()->setItemChecked(52,m_scalelocked);
  paramcamera->disableRepaint();
  paramcamera->setMaxValue(m_scalelocked?paramcamera->maxValue():0);
  paramcamera->enableRepaint();
}

void EventViewer::viewOriginLine()
{
  bool check=!menuBar()->isItemChecked(205);
  menuBar()->setItemChecked(205,check);
  paramcamera->setShowAlpha(check);
}

void EventViewer::viewStandardStyles(int n)
{
  switch(n)
    {
    case 0:
      paramcamera->setStyle(QEventCamera::STYLE_AREA);
      break;

    case 1:
      paramcamera->setStyle(QEventCamera::STYLE_INTENSITY);
      break;
    }
  
  return;
}

void EventViewer::viewStandardColors(int n)
{
  paramcamera->disableRepaint();

  switch(n)
    {
    case 0:
      paramcamera->setColorEllipse(m_defcol_ellipse);
      paramcamera->setColorDisabled(m_defcol_disabled);
      paramcamera->setColorNotImage(m_defcol_normal);
      paramcamera->setColorImageHigh(m_defcol_imhigh);
      paramcamera->setColorImageLow(m_defcol_imlow);
      paramcamera->setBackgroundMode(PaletteBackground);
      break;

    case 1:
      paramcamera->setColorEllipse(black);
      paramcamera->setColorDisabled(QColor(220,220,220));
      paramcamera->setColorNotImage(QColor(220,220,220));
      paramcamera->setColorImageHigh(QColor(40,40,40));
      paramcamera->setColorImageLow(QColor(128,128,128));
      paramcamera->setBackgroundMode(PaletteBase);
      break;

    case 2:
      paramcamera->setColorEllipse(black);
      paramcamera->setColorDisabled(m_defcol_disabled);
      paramcamera->setColorNotImage(m_defcol_normal);
      paramcamera->setColorImageHigh(m_defcol_imhigh);
      paramcamera->setColorImageLow(m_defcol_imlow);
      paramcamera->setBackgroundMode(PaletteBase);
      break;

    case 3:
      paramcamera->setColorEllipse(black);
      paramcamera->setColorDisabled(black);
      paramcamera->setColorNotImage(black);
      paramcamera->setColorImageHigh(black);
      paramcamera->setColorImageLow(black);
      paramcamera->setBackgroundMode(PaletteBase);
      break;
    }

  paramcamera->enableRepaint();
}


void EventViewer::viewCustomColors(int n)
{
  paramcamera->disableRepaint();

  QColor c;
  switch(n)
    {
    case 0: c=paramcamera->colorDisabled(); break;
    case 1: c=paramcamera->colorNotImage(); break;
    case 2: c=paramcamera->colorImageHigh(); break;
    case 3: c=paramcamera->colorImageLow(); break;
    case 4: c=paramcamera->colorEllipse(); break;
    }

  c=QColorDialog::getColor(c,this);

  if(!c.isValid())return;

  switch(n)
    {
    case 0: paramcamera->setColorDisabled(c); break;
    case 1: paramcamera->setColorNotImage(c); break;
    case 2: paramcamera->setColorImageHigh(c); break;
    case 3: paramcamera->setColorImageLow(c); break;
    case 4: paramcamera->setColorEllipse(c); break;
    }

  paramcamera->enableRepaint();
}

void EventViewer::gotoEvent(int num)
{
  if(m_rf.get() == 0)return;

  RedEvent re;
  m_rf->events()->read(num,&re,1);
  EventChannelReps* ecr=m_ecrgen->generate(&re);

  HillasParam hp;
  m_hillas->parameterize(&hp,ecr,0,0);

  paramcamera->setData(*ecr,hp);
 
  delete ecr;
}

void EventViewer::printEvent()
{
  const int Margin = 10;
  int pageNo = 1;
  
  bool otf = printer->outputToFile();
  std::ostringstream filename_stream;
  filename_stream << getFilenameBase(m_filename) << "_ev" 
		  << eventselector->getEventNumber() << ".ps";

  printer->setDocName(filename_stream.str().c_str());
  printer->setOutputFileName(filename_stream.str().c_str());
  printer->setOutputToFile(otf);

  if ( printer->setup(this) ) {		// printer dialog
    statusBar()->message( "Printing..." );
    QPainter p;
    p.begin( printer );			// paint on printer
    paramcamera->drawMe(p);
    statusBar()->message( "Printing completed", 2000 );
  } else {
    statusBar()->message( "Printing aborted", 2000 );
  }
}

void EventViewer::cutsAllEvents()
{
  eventselector->setupForAllEvents(m_rf->events()->size());
}

void EventViewer::cutsAllCodeX(int code)
{
  std::vector<int> eventlist;
  for(unsigned i=0; i<m_rf->events()->size(); i++)
    {
      RedEvent re;
      m_rf->events()->read(i,&re,1);
      if(re.getCode() == code)eventlist.push_back(i);
    }
  eventselector->setupForSelectedEvents(eventlist);
}

void EventViewer::cutsEventsFromParamFile()
{
  std::string suggested_filename = getFilenameBase(m_filename)+"_cut.ph5";
  QString filename = 
    QFileDialog::getOpenFileName(suggested_filename.c_str(),
		       "Parametrized HDF5 Files (*.ph5);;All Files (*)",
				 this,"","Load Cuts FIle");
  if(filename.isEmpty())return;
  
  ParamFile pf(100);
  try
    {
      pf.open(filename.ascii());
    }
  catch(const Error& x)
    {
      QMessageBox::warning(this,"File Error",x.message().c_str());
      return;
    }

  std::vector<int> eventlist;
  for(int i=0;i<pf.evinfo()->size();i++)
    {
      EventInfo ei;
      pf.evinfo()->read(i,&ei,1);
      eventlist.push_back(ei.getEventNumber());
    }

  eventselector->setupForSelectedEvents(eventlist);
}

#include "qteventviewer_moc.cxx"

int main( int argc, char **argv )
{
  QApplication app( argc, argv );

  bool   usepadding   = false;
  std::string pad_name;

  bool   usegains     = false;
  std::string gains_name;

  int seed=0;

  try
    {
      argc--,argv++;
  
      ODBCPedsAndGainsFactory pgfact("DSN=AnalysisDB");

      EventViewer viewer(&pgfact,0);
      
      app.setMainWidget(&viewer);
      viewer.show();

      if(argc)viewer.load(*argv);
      return app.exec();
    }      
  catch(const Error& x)
    {
      std::cerr << x;
    }
  catch(const odbc::SQLException& x)
    {
      std::cerr << "ODBC++ Exception: " << x.what() << std::endl;
    }
  catch(...)
    {
      std::cerr << "Unresolved error!" << std::endl;
    }
}
