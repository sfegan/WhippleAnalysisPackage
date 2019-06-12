//-*-mode:c++; mode:font-lock;-*-

#ifndef CAMERACONFIGURATION_H
#define CAMERACONFIGURATION_H

#define __STL_USE_NAMESPACES

#include<string>
#include<iostream>
#include<iomanip>
#include<vector>
#include<sstream>

#include<cmath>

#include"Exceptions.h"
#include"ChannelData.h"

namespace NS_Analysis {

  const unsigned int CAMVER=1;

  using std::string;
  using std::istream;
  using std::ostream;
  using std::vector;


  //
  //
  // ChannelDescription: The id, coordinates of a channel.
  //
  //

  class ChannelDescription: public ChannelMask
  {
  public:
    typedef vector<channelnum_type>        neighbors_type;
    typedef neighbors_type::size_type      size_type; 

  private:
    bool m_realchannel;

    // Tube name and number
    channelnum_type m_num;             // The PMT id.. suggestion 0,1,2,3,
    string m_printable_name;

    // Population
    unsigned int m_population;

    // Tube location and size
    double m_x;
    double m_y;
    double m_r;

    // Tube light relative collection area
    double m_relarea;

    // Neighborhood
    neighbors_type m_neighbors;

    void ncheck(size_type n) const
    { 
      if(n>=neighbors().size())
	{
	  ChannelOutOfRange err("ChannelDescription::ncheck",n);
	  err.stream() << "Neighbor " << n << " is out or range" << endl;
	  throw(err); 
	}
    }

  public:
    ChannelDescription(): ChannelMask("Not Real"), m_realchannel(false), 
      m_num(0), m_printable_name("none"), m_population(0),
      m_x(0), m_y(0), m_r(0), m_relarea(0) {}
    
    ChannelDescription(channelnum_type num): ChannelMask("Not Real"), 
      m_realchannel(false), m_num(num), m_printable_name("none"), 
      m_population(0),
      m_x(0), m_y(0), m_r(0), m_relarea(0) {}
    
    ChannelDescription(channelnum_type num, 
		       double x, double y, double r, 
		       unsigned int pop=0, double ceff=1.0):
      ChannelMask(),
      m_realchannel(true), m_num(num), m_printable_name(), 
      m_population(pop), m_x(x), m_y(y), m_r(r), m_relarea(ceff)
    { std::ostringstream s; s << num+1; m_printable_name=s.str(); }
    
    ChannelDescription(channelnum_type num, const string& name,
		       double x, double y, double r,
		       unsigned int pop=0, double ceff=1.0):
      ChannelMask(),
      m_realchannel(true), m_num(num), m_printable_name(name), 
      m_population(pop), m_x(x), m_y(y), m_r(r), m_relarea(ceff)
      {}
    
    ChannelDescription(const ChannelDescription& o):
      ChannelMask(o), m_realchannel(o.m_realchannel), 
      m_num(o.m_num), m_printable_name(o.m_printable_name),
      m_population(o.m_population), m_x(o.m_x), m_y(o.m_y), m_r(o.m_r), 
      m_relarea(o.m_relarea) 
      {}
    
    const ChannelDescription& operator=(const ChannelDescription& o);

    // Is it real
    bool isRealChannel() const { return m_realchannel; }

    // Name and number
    channelnum_type num() const { return m_num; }
    const string& name() const { return m_printable_name; }

    // Location
    double x() const { return m_x; }
    double y() const { return m_y; }
    double r() const { return m_r; }

    double dist2(double x0, double y0) const 
      { x0-=x(); y0-=y(); return x0*x0+y0*y0; }
    double dist(double x0=0.0, double y0=0.0) const 
      { return sqrt(dist2(x0,y0)); }
    double edgedist() const { return dist()+r(); }
    
    // these next two are used in seeing whether we are inside a tube of not
    double edist2(double x0=0.0, double y0=0.0) const 
      { return dist2(x0,y0)-r()*r(); } // CAUTION: this is not edist()*edist()
    double edist(double x0=0.0, double y0=0.0) const 
      { return dist(x0,y0)-r(); }
    
    // Population and area
    unsigned int population() const { return m_population; }
    double relarea() const { return m_relarea; }

    // Neighbors
    size_type numneighbors() const { return neighbors().size(); }
    const neighbors_type& neighbors() const { return m_neighbors; }
    channelnum_type neighbor(size_type n) const 
    { ncheck(n); return neighbors()[n]; }
    void add_neighbor(channelnum_type c) { m_neighbors.push_back(c); }
  };
  
  inline 
  ostream& operator<< (ostream &stream, const ChannelDescription& channel);

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  ////////////////////////// Camera Configuration /////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  class CameraConfiguration
  {
  private:
    string                            m_description;

    channelnum_type                   m_nchannels;
    channelnum_type                   m_ntrigger;
    vector<ChannelDescription>        m_channels;

    double                            m_latitude;
    double                            m_longitude;
    double                            m_elevation;

    void check(channelnum_type t) const
      { 
	if( t>=m_channels.size() )
	  {
	    ChannelOutOfRange err("CameraConfiguration::check",t);
	    err.stream() << "Request for channel number " << t
			 << " is out of range. Max is:" << nchannels() << endl;
	    throw(err); 
	}
      } 

  public:
    CameraConfiguration(const string& filename):
      m_nchannels(0) { read(filename); }
    CameraConfiguration(const string& camdesc,
			channelnum_type nc, channelnum_type ntrig=0,
			double lat =  31.680472,
			double lon = 110.879056, 
			double el  =  2312): 
      m_description(camdesc), 
      m_nchannels(nc), m_ntrigger(ntrig), m_channels(nc),
      m_latitude(lat), m_longitude(lon), m_elevation(el) {}

    const string& description() const { return m_description; }

    channelnum_type nchannels() const { return m_nchannels; }
    channelnum_type ntrigger() const { return m_ntrigger; }

    double latitude() const { return m_latitude; }
    double longitude() const { return m_longitude; }
    double elevation() const { return m_elevation; }

    const vector<ChannelDescription>& channels() const { return m_channels; }
    vector<ChannelDescription>& channels() { return m_channels; }

    const ChannelDescription& channel(channelnum_type c) const
    { check(c); return m_channels[c]; }
    ChannelDescription& channel(channelnum_type c)
    { check(c); return m_channels[c]; }

    void set_channel(channelnum_type c, const ChannelDescription& cd) 
    { check(c); m_channels[c]=cd; }
    
    // FIND NEAREST TUBE

    double tubeDistance2(channelnum_type c, double x, double y) const;
    double tubeDistance(channelnum_type c, double x, double y) const;
    bool insideTube(channelnum_type c, double x, double y) const;
    channelnum_type findNearest(double x, double y) const;

    // READ AND WRITE CAMERA FILE

    void read(const string& filename);
    void write(const string& filename);

    void read_h5(const string& filename);
    void write_h5(const string& filename);

    void read_ascii(const string& filename);
    void write_ascii(const string& filename);
  };

  ostream& operator<< (ostream &stream, const ChannelDescription& channel);
  ostream& operator<< (ostream &stream, const CameraConfiguration& camera);
} // namespace NS_Analysis

inline std::string
NS_Analysis::ChannelMask::
whyMasked() const
{
  std::string reason;
  vector<string>::const_iterator r = m_mask_reason.begin();
  while(r!=m_mask_reason.end())
    {
      reason=reason+"["+(*r)+"]";
      r++;
    }
  return reason;
}

inline void 
NS_Analysis::ChannelMask::
addMask(const ChannelMask& o, const string& prefix)
{
  if(o.isMasked())mask();

  vector<string>::const_iterator r = o.m_mask_reason.begin();
  while(r!=o.m_mask_reason.end())
    {
      m_mask_reason.push_back(prefix+(*r));
      r++;
    }
}

inline const NS_Analysis::ChannelMask&
NS_Analysis::ChannelMask::
operator=(const ChannelMask& o)
{
  m_mask=o.m_mask;
  m_mask_reason=o.m_mask_reason;
  return *this;
}

inline const NS_Analysis::ChannelDescription& 
NS_Analysis::ChannelDescription::
operator=(const ChannelDescription& o)
{
  *static_cast<ChannelMask*>(this)=static_cast<ChannelMask>(o);
  m_realchannel=o.m_realchannel;
  m_num=o.m_num;
  m_printable_name=o.m_printable_name;
  m_x=o.m_x;
  m_y=o.m_y;
  m_r=o.m_r;
  m_population=o.m_population;
  m_relarea=o.m_relarea;
  m_neighbors=o.m_neighbors;
  return *this;
}

inline double 
NS_Analysis::CameraConfiguration::
tubeDistance2(channelnum_type c, double x, double y) const
{
  return(channel(c).edist2(x,y));
}

inline double 
NS_Analysis::CameraConfiguration::
tubeDistance(channelnum_type c, double x, double y) const
{
  return(channel(c).edist(x,y));
}

inline bool
NS_Analysis::CameraConfiguration::
insideTube(channelnum_type c, double x, double y) const
{
  return tubeDistance2(c,x,y) <= 0.0;
}

#endif // CAMERACONFIGURATION_H
