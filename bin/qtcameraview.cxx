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
#include <qmainwindow.h>
#include <qaccel.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qwhatsthis.h>
#include <qwindowsstyle.h>

#include "../VSFA.h"
#include "../QCamera.h"
#include "../CameraConfiguration.h"
#include "../Exceptions.h"
#include "../UtilityFunctions.h"

#include <cstdio>

using namespace NS_Analysis;
using namespace NS_QComponents;

#include "pixmaps/fileopen.xpm"
#include "pixmaps/fileprint.xpm"
#include "pixmaps/labelling.xpm"
#include "pixmaps/vertices.xpm"

const char * fileOpenText = 
"Click this button to open a <em>new camera</em>. <br><br>"
"You can also select the <b>Open command</b> from the File menu.";
const char * filePrintText = "Click this button to print the camera "
"are viewing.\n\n"
"You can also select the Print command from the File menu.";

class ApplicationWindow : public QMainWindow
{
  Q_OBJECT
  
public:
  ApplicationWindow();
  ~ApplicationWindow();

protected:
  void closeEvent( QCloseEvent* );
  
private slots:
  void load();
  void load( const char *fileName );
  void print();
  void viewlabel();
  void viewvertex();
  void lockscale();
  void htrigger();

  void about();
  void aboutQt();

public slots:
  void mousePress(channelnum_type n, const QMouseEvent* me);
  void mouseRelease(const QMouseEvent* me);

private:
  QPrinter *printer;
  CameraConfiguration* cam;
  QCamera *qcam;
  QToolBar *fileTools;
  std::string filename;
};

ApplicationWindow::ApplicationWindow( ):
  QMainWindow(0,"QCameraView",WDestructiveClose)
{
  int id;

  printer = new QPrinter;
#if (QT_VERSION > 300)
  printer->setResolution(600);
#endif
  printer->setCreator("qtcameraview");
  printer->setColorMode(QPrinter::Color);
  printer->setPageSize(QPrinter::Letter);

  QPixmap openIcon, printIcon, labellingIcon, verticesIcon;
  
  fileTools = new QToolBar( this, "file operations" );
  fileTools->setLabel( tr( "File Operations" ) );

  openIcon = QPixmap( fileopen );
  QToolButton * fileOpen
    = new QToolButton( openIcon, "Open File", QString::null,
		       this, SLOT(load()), fileTools, "open file" );
    
  printIcon = QPixmap( fileprint );
  QToolButton * filePrint
    = new QToolButton( printIcon, "Print File", QString::null,
		       this, SLOT(print()), fileTools, "print file" );
  
  (void)QWhatsThis::whatsThisButton( fileTools );
  
  QWhatsThis::add( fileOpen, fileOpenText );
  //  QMimeSourceFactory::defaultFactory()->setPixmap( "fileopen", openIcon );
  QWhatsThis::add( filePrint, filePrintText );

  QPopupMenu * file = new QPopupMenu( this );
  menuBar()->insertItem( "&File", file );

  id = file->insertItem( openIcon, "&Open",
			 this, SLOT(load()), CTRL+Key_O );
  file->setWhatsThis( id, fileOpenText );
  
  file->insertSeparator();
  id = file->insertItem( printIcon, "&Print",
			 this, SLOT(print()), CTRL+Key_P );
  file->setWhatsThis( id, filePrintText );
  file->insertSeparator();
  file->insertItem( "&Close", this, SLOT(close()), CTRL+Key_W );
  file->insertItem( "&Quit", qApp, SLOT( closeAllWindows() ), CTRL+Key_Q );

  /// VIEW MENU

  labellingIcon = QPixmap( labelling );
  QToolButton * labellingButton
    = new QToolButton( labellingIcon, "Label Tubes", QString::null,
		       this, SLOT(viewlabel()), fileTools, "label tubes" );
    
  verticesIcon = QPixmap( vertices );
  QToolButton * verticesButton
    = new QToolButton( verticesIcon, "Show Neighbors", QString::null,
		       this, SLOT(viewvertex()), fileTools, "show neighbors" );
    

  QPopupMenu * view = new QPopupMenu( this );
  menuBar()->insertItem( "&View", view );
  
  id = view->insertItem( "Label tubes", this, SLOT(viewlabel()),0,100);
  view->setItemChecked ( id, true );

  id = view->insertItem( "Draw Neighbors", this, SLOT(viewvertex()),0,101);
  view->setItemChecked ( id, false );

  id = view->insertItem( "Lock Scale", this, SLOT(lockscale()),0,102);
  view->setItemChecked ( id, false );

  id = view->insertItem( "Highlight Trigger", this, SLOT(htrigger()),0,103);
  view->setItemChecked ( id, false );

  //void viewlabel();
  //void viewvertex();
  //void lockscale();
  
  QPopupMenu * help = new QPopupMenu( this );
  menuBar()->insertSeparator();
  menuBar()->insertItem( "&Help", help );
  
  help->insertItem( "&About", this, SLOT(about()), Key_F1 );
  help->insertItem( "About &Qt", this, SLOT(aboutQt()) );
  help->insertSeparator();
  help->insertItem( "What's &This", this, SLOT(whatsThis()), SHIFT+Key_F1 );

  cam = 0;//new CameraConfiguration(0);
  qcam = new QCamera( cam, this, "camera" );
  qcam->setLabelling(true);
  qcam->show();
  setCentralWidget( qcam );

  connect(qcam, SIGNAL(mousePress(channelnum_type,const QMouseEvent*)), 
	  this, SLOT(mousePress(channelnum_type,const QMouseEvent*)));

  connect(qcam, SIGNAL(mouseRelease(const QMouseEvent*)), 
	  this, SLOT(mouseRelease(const QMouseEvent*)));

  statusBar()->message( "Ready", 2000 );
  resize(600,600);
}

ApplicationWindow::~ApplicationWindow()
{
  delete printer;
  delete qcam;
  if(cam)delete cam;
}

void ApplicationWindow::load()
{
  QString fn = QFileDialog::getOpenFileName( QString::null, QString::null,
					     this);
  if ( !fn.isEmpty() )
    load( fn );
  else
    statusBar()->message( "Loading aborted", 2000 );
}

void ApplicationWindow::load( const char *fileName )
{
  if(cam)delete cam;
  try 
    {
      cam=new CameraConfiguration(fileName);
      qcam->setCamera(cam);
      setCaption(fileName);
    }
  catch(Error x)
    {
      std::cerr << x << '\n';
      cam=0;
      qcam->setCamera(cam);
    }

  qcam->setCamera(cam);
}

void ApplicationWindow::print()
{
  const int Margin = 10;
  int pageNo = 1;
  
  bool otf = printer->outputToFile();
  std::ostringstream filename_stream;
  filename_stream << getFilenameBase(filename) << ".ps";

  printer->setDocName(filename_stream.str().c_str());
  printer->setOutputFileName(filename_stream.str().c_str());
  printer->setOutputToFile(otf);

  if ( printer->setup(this) ) {		// printer dialog
    statusBar()->message( "Printing..." );
    QPainter p;
    p.begin( printer );			// paint on printer
    qcam->drawMe(p);
    statusBar()->message( "Printing completed", 2000 );
  } else {
    statusBar()->message( "Printing aborted", 2000 );
  }
}

void ApplicationWindow::about()
{
  QMessageBox::about( this, "QtCameraView",
		      "View a camera file");
}

void ApplicationWindow::viewlabel()
{
  bool check=true;
  if(menuBar()->isItemChecked(100))check=false;
  menuBar()->setItemChecked(100,check);
  qcam->setLabelling(check);
}

void ApplicationWindow::viewvertex()
{
  bool check=true;
  if(menuBar()->isItemChecked(101))check=false;
  menuBar()->setItemChecked(101,check);
  qcam->setDrawVertices(check);
}

void ApplicationWindow::lockscale()
{
  bool check=true;
  if(menuBar()->isItemChecked(102))check=false;
  menuBar()->setItemChecked(102,check);

  if(check)qcam->setAngularSize(qcam->angularSize());
  else qcam->setAngularSize(0);
}

void ApplicationWindow::htrigger()
{
  bool check=true;
  if(menuBar()->isItemChecked(103))check=false;
  menuBar()->setItemChecked(103,check);
  qcam->setHighlightTrigger(check);
}

void ApplicationWindow::closeEvent( QCloseEvent* ce )
{
  ce->accept();
}

void ApplicationWindow::aboutQt()
{
  QMessageBox::aboutQt( this, "Qt Application Example" );
}

void ApplicationWindow::mousePress(channelnum_type n, const QMouseEvent* me)
{
  if(cam==0)return;
  std::vector<QColor> color(cam->nchannels(),blue);
  std::vector<double> value(cam->nchannels(),0.0);
  for(int i=0;i<cam->channel(n).numneighbors();i++)
    value[cam->channel(n).neighbor(i)]=1.0;
  qcam->setData(value,color);
}

void ApplicationWindow::mouseRelease(const QMouseEvent* me)
{
  if(cam==0)return;
  std::vector<QColor> color(cam->nchannels(),blue);
  std::vector<double> value(cam->nchannels(),0.0);
  qcam->setData(value,color);
}

#include "qtcameraview_moc.cxx"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int main( int argc, char **argv )
{
  bool drawvertex=false;
  double angsize=0.0;

  argc--,argv++;
 
  QApplication a( argc, argv );
  //a.setStyle(new QWindowsStyle());
  ApplicationWindow * mw = new ApplicationWindow();
  mw->setCaption( "No Camera" );
  mw->show();
  a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
  return a.exec();
}
