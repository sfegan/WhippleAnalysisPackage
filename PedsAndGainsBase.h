//-*-mode:c++; mode:font-lock;-*-

#ifndef PEDSANDGAINSBASE_H
#define PEDSANDGAINSBASE_H

#define __STL_USE_NAMESPACES

#include<iostream>
#include<string>
#include<vector>
#include<algorithm>
#include<cmath>

#include"ChannelData.h"
#include"CameraConfiguration.h"

namespace NS_Analysis {
  

  class PedsAndGainsBase: public ChannelValDevAndMask
  {
  public:
    PedsAndGainsBase(channelnum_type nchannels): 
      ChannelValDevAndMask(nchannels), 
      m_nevents(), m_cameraname(), m_comment() {}
    
    virtual ~PedsAndGainsBase();

    // SET ACCESSORS

    void setComment(const string& c) { m_comment=c; }
    void addToComment(const string& c) { m_comment += c; }

    void setCamera(const string& c) { m_cameraname=c; }

    void setNEvents(unsigned int n) { m_nevents = n; }

    // GET ACCESSORS

    const string& comment() const { return m_comment; }

    const string& camera() const { return m_cameraname; }

    unsigned int nevents() const { return m_nevents; }

  private:
    unsigned int m_nevents;
    string m_cameraname;
    string m_comment;
  };

  ostream& operator<<(ostream& stream, const PedsAndGainsBase& p);
  
} // namespace NS_Analysis

#endif // PEDSANDGAINSBASE_H
