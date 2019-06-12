#include <string>

#include <CameraConfiguration.h>
#include <VSFA.h>
#include <WhippleCams.h>
#include <Exceptions.h>

using namespace NS_Analysis;
using namespace std;

CameraConfiguration*
newCam(const string& desc, int nchan, int npmt, int ntrig, 
       const float X[], const float Y[], const float R[],
       int nl[][7])
{
  CameraConfiguration *cam=new CameraConfiguration(desc,nchan,ntrig);
  
  for(int ch=0;ch<npmt;ch++)
    {
      cam->set_channel(ch,ChannelDescription(ch,X[ch],Y[ch],R[ch]));
      for(int n=0;n<7;n++)if((nl[ch][n]!=-1)&&(nl[ch][n]<npmt))
	cam->channel(ch).add_neighbor(nl[ch][n]);
    }

  for(int ch=npmt;ch<nchan;ch++)
    {
      cam->set_channel(ch,ChannelDescription(ch));
    }
  
  return cam;
}

CameraConfiguration*
newGR1Cam(const string& desc, int nchan, int npmt,  int ntrig, 
	 const float X[], const float Y[], const float R[],
	 int nl[][7], int gc, double rca)
{
  CameraConfiguration *cam=new CameraConfiguration(desc,nchan,ntrig);
  
  for(int ch=0;ch<npmt;ch++)
    {
      cam->set_channel(ch,ChannelDescription(ch,X[ch],Y[ch],R[ch],
					     ch<gc?0:1,
					     ch<gc?1.0:rca));

      for(int n=0;n<7;n++)if(nl[ch][n]!=-1)
	cam->channel(ch).add_neighbor(nl[ch][n]);
    }
  
  for(int ch=npmt;ch<nchan;ch++)
    {
      cam->set_channel(ch,ChannelDescription(ch));
    }

  return cam;
}

main()
{
  CameraConfiguration *cam;

  cam=newCam("Whipple10m::109/120",
	     120,109,91,WC109Xcoord,WC109Ycoord,WC109Radius,WC109Neighbors);
  try 
    {
      cam->write_h5("WC109.h5");
    }
  catch(Error x)
    {
      cerr << "Its an error!!\n";
      cerr << Error(x) << '\n';
      abort();
    }
  delete cam;

  cam=newCam("Whipple10m::151/156",
	     156,151,91,WC151Xcoord,WC151Ycoord,WC151Radius,WC151Neighbors);
  cam->write_h5("WC151.h5");
  delete cam;

  cam=newCam("Whipple10m::331/336",
	     336,331,331,WC331Xcoord,WC331Ycoord,WC331Radius,WC331Neighbors);
  cam->write_h5("WC331.h5");
  delete cam;

  cam=newGR1Cam("Whipple10m::490/492",
		492,490,331,WC490Xcoord,WC490Ycoord,WC490Radius,WC490Neighbors,
		379,4.507365605);
  cam->write_h5("WC490_outer.h5");
  delete cam;

  cam=newGR1Cam("Whipple10m::379/492(Inner)",
		492,490,331,WC490Xcoord,WC490Ycoord,WC490Radius,WC490Neighbors,
		379,4.507365605);
  for(int channel=379;channel<490;channel++)
    cam->channel(channel).mask("Outer Tube");
  cam->write_h5("WC490.h5");
  delete cam;

  cam=newCam("Whipple10m::379/379(Pseudo)",
	     379,379,331,WC490Xcoord,WC490Ycoord,WC490Radius,WC490Neighbors);
  cam->write_h5("WC490_no_outer.h5");
  delete cam;

  /*
  cam=newGR1Cam(490,490,331,WC490Xcoord,WC490Ycoord,WC490Radius,WC490Neighbors,
		379,4.507365605);
  for(int channel=379;channel<490;channel++)
    cam->channel(channel).mask("outer tube");
  cam->write_h5("WC490A.h5");
  delete cam;
  */

  cam=newCam("Veritas::Conceptual::499/499",
	     499,499,499,VC499Xcoord,VC499Ycoord,VC499Radius,VC499Neighbors);
  cam->write_h5("VC499.h5");
  delete cam;
}
