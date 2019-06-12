#include<hdf5/hdf5.h>

#include"Exceptions.h"

#include"CameraConfiguration.h"

using std::setw;
using std::setprecision;
using std::setiosflags;
using std::ostream;

NS_Analysis::channelnum_type 
NS_Analysis::CameraConfiguration::
findNearest(double x, double y) const
{
  unsigned int nearest;
  for(nearest=0;nearest<nchannels();nearest++)
    if(channels()[nearest].isRealChannel())break;

  if(nearest==nchannels())
    {
      Error err("CameraConfiguration::findNearest");
      err.stream() << "No channel is nearest to x=" << x << " y=" << y << endl
		   << "This must mean no channels are defined. nchannel=" 
		   << nchannels() << endl;
      throw(err);
    }

  double nearestdist2=tubeDistance2(nearest,x,y);
  if(nearestdist2 <= 0.0)return nearest;
  
  for(unsigned int c=nearest+1;c<nchannels();c++)
    if(channels()[c].isRealChannel())
      {
	double dist2=tubeDistance2(c,x,y);
	if(dist2 <= 0.0)return c;
	if(dist2 < nearestdist2)nearestdist2=dist2,nearest=c;
      }
  
  return nearest;
}

ostream& 
NS_Analysis::
operator<< (ostream &stream, const ChannelDescription& channel)
{
  stream << "Channel: " << setw(3) << channel.num() 
	 << setw(11) << string(" (name=")+channel.name()+")"
	 << ", x=" << setw(5) << channel.x() 
	 << ", y=" << setw(5) << channel.y()
	 << ", r=" << setw(5) << channel.r()
	 << ", pop=" << setw(2) << channel.population()
	 << ", eff=" << setw(5) << channel.relarea() 
	 << ", neighbors=";

  if(channel.numneighbors() > 0)
    {
      stream << channel.neighbor(0);
      for(unsigned int i=1;i<channel.numneighbors();i++)
	stream << ',' << channel.neighbor(i);
    }
  else stream << "none";
  
  if(channel.isMasked())stream << ", masked:" << channel.whyMasked();

  return stream;
}

ostream& 
NS_Analysis::
operator<< (ostream &stream, const CameraConfiguration& camera)
{
  stream << "Description: " << camera.description() << endl;
  stream << "Nchannels: " << camera.nchannels() << " (" 
	 << camera.channels().size() << ")\n";
  stream << "NTrigger: " << camera.ntrigger() << '\n';
  stream << "Latitude:  " << camera.latitude() << " deg\n";
  stream << "Longitude: " << camera.longitude() << " deg\n";
  stream << "Elevation: " << camera.elevation() << " m\n";

  double maxdist_trig=0;
  double maxdist_fov=0;
  for(unsigned int i=0;i<camera.channels().size();i++)
    {
      const ChannelDescription& chan=camera.channel(i);
      stream << chan << '\n';

      if((i<camera.ntrigger())&&(!chan.isMasked())&&(chan.dist()>maxdist_trig))
	maxdist_trig=chan.dist();

      if((!chan.isMasked()) && (chan.dist()>maxdist_fov))
	maxdist_fov=chan.dist();
    }
  stream << "Approximate field of view (trigger): "
	 << maxdist_trig*2 << " deg\n";
  stream << "Approximate field of view: " 
	 << maxdist_fov*2 << " deg\n";

  return stream;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////// General routine to start read/write ////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void 
NS_Analysis::CameraConfiguration::
write(const string& filename)
{
  write_h5(filename);
}

void 
NS_Analysis::CameraConfiguration::
read(const string& filename)
{
  if(H5Fis_hdf5(filename.c_str())!=0)return read_h5(filename);
  FileError err("CameraConfiguration:: read");
  err.stream() << "Unrecognised file format: " << filename << endl;
  throw(err);
}

///////////////////////////////////////////////////////////////////////////////
//
// HDF5 specific read/write routines
//
///////////////////////////////////////////////////////////////////////////////

#include"StorableChannelDescription.h"
#include"StorableNeighborVertex.h"
#include"StorableCameraConfiguration.h"

#include"cH5CameraConf.h"



void 
NS_Analysis::CameraConfiguration::
write_h5(const string& filename)
{
  cH5CameraConf *cH5cc=new cH5CameraConf(filename,CAMVER);

  // Store the camera configuration
  StorableCameraConfiguration scc;
  scc.setDescription(description());
  scc.setNChannel(nchannels());
  scc.setNTrigger(ntrigger());
  scc.setLatitude(latitude());
  scc.setLongitude(longitude());
  scc.setElevation(elevation());
  cH5cc->cameraconf().write(0,&scc,1);

  // Store the channels
  channelnum_type c;
  for(c=0;c<nchannels();c++)
    {
      StorableChannelDescription scd;
      scd.setRealChannel(channel(c).isRealChannel());
      scd.setNum(channel(c).num());
      scd.setX(channel(c).x());
      scd.setY(channel(c).y());
      scd.setR(channel(c).r());
      scd.setPopulation(channel(c).population());
      scd.setRelativeCollectionArea(channel(c).relarea());
      scd.setPrintableName(channel(c).name());
      scd.setMasked(channel(c).isMasked());
      //      scd.setMaskedReason(channel(c).whyMasked());
      cH5cc->channels().write(c,&scd,1);

      if(channel(c).isMasked())
	{
	  int nreasons=channel(c).nMaskedReasons();
	  for(int i=0; i<nreasons; i++)
	    {
	      StorableChannelMask scm;
	      scm.setNum(c);
	      scm.setReason(channel(c).maskedReason(i));
	      cH5cc->channelmasks().write(cH5cc->channelmasks().size(),
					  &scm,1);
	    }
	}
    }
  
  // Neighbors
  unsigned int vertexcount=0;
  for(c=0;c<nchannels();c++)
    {
      unsigned int numneighbors=channels()[c].numneighbors();
      unsigned int n;
      for(n=0;n<numneighbors;n++)
	{
	  channelnum_type b=channel(c).neighbor(n);
	  if(b>c)
	    {
	      StorableNeighborVertex snv;
	      snv.setChannelA(c);
	      snv.setChannelB(b);
	      cH5cc->vertices().write(vertexcount++,&snv,1);
	    }
	}
    }

  delete cH5cc;
}

void 
NS_Analysis::CameraConfiguration::
read_h5(const string& filename)
{
  cH5CameraConf *cH5cc=new cH5CameraConf(filename);

  // Retreive the camera configuration
  StorableCameraConfiguration scc;
  cH5cc->cameraconf().read(0,&scc,1);
  if((m_nchannels)&&(m_nchannels!=scc.nchannel()))
    {
      Error err("CameraConfiguration::read_h5");
      err.stream() << "Incompatable camera configuration file" << endl;
      throw err;
    }

  m_description=scc.description();
  m_nchannels=scc.nchannel();
  m_ntrigger=scc.ntrigger();
  m_channels.resize(m_nchannels);
  m_latitude=scc.latitude();
  m_longitude=scc.longitude();
  m_elevation=scc.elevation();

  // Retreive the channels
  channelnum_type c=0;

  for(c=0;c<nchannels();c++)
    {
      StorableChannelDescription scd;
      cH5cc->channels().read(c,&scd,1);

      if(scd.real_channel())
	{
	  ChannelDescription cd(scd.num(),scd.printable_name(),
				scd.x(),scd.y(),scd.r(),
				scd.population(),scd.relative_collection_area()
				);
	  if(scd.masked())cd.mask();
	  set_channel(c,cd);
	}
      else set_channel(c,ChannelDescription(scd.num()));
    }
  
  int nmasks=cH5cc->channelmasks().size();
  for(int i=0; i<nmasks; i++)
    {
      StorableChannelMask scm;
      cH5cc->channelmasks().read(i,&scm,1);
      channel(scm.num()).mask(scm.reason());
    }

  // Neighbors
  unsigned int vertexcount=0;
  for(vertexcount=0;vertexcount<cH5cc->vertices().size();vertexcount++)
    {
      StorableNeighborVertex snv;
      cH5cc->vertices().read(vertexcount,&snv,1);
      channel(snv.channel_a()).add_neighbor(snv.channel_b());
      channel(snv.channel_b()).add_neighbor(snv.channel_a());
    }

  delete cH5cc;
}


