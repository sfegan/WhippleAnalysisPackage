//-*-mode:c++; mode:font-lock;-*-

#ifndef CH5CAMERACONF_H
#define CH5CAMERACONF_H

#define __STL_USE_NAMESPACES

#include<string>

#include<hdf5/hdf5.h>

#include"cH5Utils.h"

#include"VSFA.h"  
#include"VSFA_cH5.h"
#include"VSFA_Caching.h"

#include"StorableChannelDescription.h"
#include"StorableChannelMask.h"
#include"StorableNeighborVertex.h"
#include"StorableCameraConfiguration.h"

namespace NS_Analysis {
  
  using std::string;
  
  
  
  
  static const string H5GCamera("CameraConfiguration");
  static const string H5DCameraConfig("Config");
  static const string H5DCameraChannels("Channels");
  static const string H5DCameraChannelMasks("ChannelMasks");
  static const string H5DCameraNeighbors("Neighbors");
  
  class cH5CameraConf
  {
  private:
    MPtr<cH5File>                                       h5f;
    MPtr<cH5Group>                                      h5g;
    
    MPtr<VSFA_cH5<StorableCameraConfiguration> >        m_cameraconf;
    MPtr<VSFA_Caching<StorableChannelDescription> >     m_channels;
    MPtr<VSFA_Caching<StorableChannelMask> >            m_channelmasks;
    MPtr<VSFA_Caching<StorableNeighborVertex> >         m_neighborvertices;

    cH5CameraConf();
    cH5CameraConf& operator= (cH5CameraConf&);
    
  public:
    cH5CameraConf(const string& filename, unsigned int version);
    cH5CameraConf(const string& filename);
    ~cH5CameraConf();
    
    VSFA<StorableCameraConfiguration>& cameraconf() { return *m_cameraconf; }
    VSFA<StorableChannelDescription>&  channels()   { return *m_channels; }
    VSFA<StorableChannelMask>&       channelmasks() { return *m_channelmasks; }
    VSFA<StorableNeighborVertex>&    vertices() { return *m_neighborvertices; }
  };

}; // namespace NS_Analysis

NS_Analysis::cH5CameraConf::
cH5CameraConf(const string& filename, unsigned int version)
{
  h5f.manage(new cH5FileNewTruncate(filename));
  h5g.manage(h5f->createGroup(H5GCamera));

  m_cameraconf.manage(new VSFA_cH5<StorableCameraConfiguration>
		      (version,*h5g,H5DCameraConfig,H5P_DEFAULT,1));
  
  VSFA<StorableChannelDescription>* cfa=
    new VSFA_cH5<StorableChannelDescription>(version,*h5g,H5DCameraChannels);
  
  VSFA<StorableChannelMask>* cmfa=
    new VSFA_cH5<StorableChannelMask>(version,*h5g,H5DCameraChannelMasks);
  
  VSFA_cH5<StorableNeighborVertex>* nvfa=
    new VSFA_cH5<StorableNeighborVertex>(version, *h5g,H5DCameraNeighbors);
  
  m_channels.manage(new VSFA_Caching<StorableChannelDescription>
		    (cfa,500,500,true));
  
  m_channelmasks.manage(new VSFA_Caching<StorableChannelMask>
			(cmfa,500,500,true));
  
  m_neighborvertices.manage(new VSFA_Caching<StorableNeighborVertex>
			    (nvfa,500,500,true));
}

NS_Analysis::cH5CameraConf::
cH5CameraConf(const string& filename)
{
  h5f.manage(new cH5FileReadOnly(filename));
  h5g.manage(h5f->openGroup(H5GCamera));
  
  m_cameraconf.manage(new VSFA_cH5<StorableCameraConfiguration>
		      (*h5g,H5DCameraConfig));
  
  VSFA<StorableChannelDescription>* cfa=
    new VSFA_cH5<StorableChannelDescription>(*h5g,H5DCameraChannels);
  
  VSFA<StorableChannelMask>* cmfa=
    new VSFA_cH5<StorableChannelMask>(*h5g,H5DCameraChannelMasks);
  
  VSFA_cH5<StorableNeighborVertex>* nvfa=
    new VSFA_cH5<StorableNeighborVertex>(*h5g,H5DCameraNeighbors);
  
  m_channels.manage(new VSFA_Caching<StorableChannelDescription>
		    (cfa,500,500,true));
  
  m_channelmasks.manage(new VSFA_Caching<StorableChannelMask>
			(cmfa,500,500,true));
  
  m_neighborvertices.manage(new VSFA_Caching<StorableNeighborVertex>
			    (nvfa,500,500,true));
}

NS_Analysis::cH5CameraConf::
~cH5CameraConf()
{
  m_neighborvertices.reset();
  m_channelmasks.reset();
  m_channels.reset();
  m_cameraconf.reset();
  h5g.reset();
  h5f.reset();
}

#endif // CH5CAMERACONF_H
