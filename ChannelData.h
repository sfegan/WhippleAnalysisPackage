//-*-mode:c++; mode:font-lock;-*-

#ifndef CHANNELDATA_H
#define CHANNELDATA_H

#define __STL_USE_NAMESPACES

#include<iostream>
#include<string>
#include<vector>
#include<algorithm>

#include"Types.h"
#include"Exceptions.h"

namespace NS_Analysis {
  
  using std::endl;
  using std::string;
  using std::vector;

  template<class T> class ChannelData
  {
  public:
    typedef typename vector<T>::iterator       iterator;
    typedef typename vector<T>::const_iterator const_iterator;

    ChannelData(channelnum_type n)
      : m_nchannels(n), m_data(n) {}
    ChannelData(channelnum_type n, const T& defT)
      : m_nchannels(n), m_data(n,defT) {}
    ChannelData(const ChannelData &o)
      : m_nchannels(o.m_nchannels), m_data(o.m_data) {}

    ChannelData& operator= (const ChannelData &o) 
    { m_nchannels=o.m_nchannels; m_data=o.m_data; return *this; }
    
    channelnum_type nchannels() const { return m_nchannels; }

    bool isCompatible(channelnum_type n) const { return n==m_nchannels; }
    bool isCompatible(const ChannelData& o) const 
    { return isCompatible(o.nchannels()); }

    void requireCompatible(channelnum_type n) const
    { 
      if(!isCompatible(n))
	{
	  ChannelDataIncompatible err("Channeldata<>::requireCompatible",
				      nchannels(),n);
	  err.stream() << "Meshing of object with " << n 
		       << " channels is incompatible with this " 
		       << nchannels() << endl;
	  throw(err);
	}
    }
    
    void requireCompatible(const ChannelData& o) const 
    { requireCompatible(o.nchannels()); }
    
    T& operator()(channelnum_type n) { validate(n); return m_data[n]; }
    T& operator[](channelnum_type n) { validate(n); return m_data[n]; }
    
    const T& operator()(channelnum_type n) const 
    { validate(n); return m_data[n]; }

    // Iterators
    iterator begin() { return m_data.begin(); }
    const_iterator begin() const { return m_data.begin(); }

    iterator end() { return m_data.end(); }
    const_iterator end() const { return m_data.end(); }

    const vector<T>& data() const { return m_data; }

    // Some statistical functions... probably only useful when T==double!
    //    T mean() const;
    //    T variance() const;
    //    T median() const;

  private:
    channelnum_type  m_nchannels;
    vector<T>        m_data;

    void validate(channelnum_type n) const
#ifdef NO_RANGE_CHECKS
    {}
#else
    {
      if(n>=m_nchannels)
	{
	  ChannelOutOfRange err("ChannelData<>::validate",n);
	  err.stream() << "Request for channel number " << n 
		       << " is out of range. Max is:" << nchannels() 
		       << endl;
	  throw(err); 
	}
    }
#endif
  };

  class ChannelMask
  {
  private:
    bool m_mask;
    vector<string> m_mask_reason;
  public:
    ChannelMask(): m_mask(false), m_mask_reason() {}
    ChannelMask(const string& r): m_mask(false), m_mask_reason() { mask(r); }
    ChannelMask(const ChannelMask& o): 
      m_mask(o.m_mask), m_mask_reason(o.m_mask_reason) {}
    const ChannelMask& operator=(const ChannelMask& o);

    void addMask(const ChannelMask& o, const string& prefix="");

    void mask() { m_mask=true; }
    void mask(const string& r) { mask(); m_mask_reason.push_back(r); }
    void unmask() { m_mask=false; m_mask_reason.clear(); }
    bool isMasked() const { return m_mask; }
    string whyMasked() const;

    int nMaskedReasons() const { return m_mask_reason.size(); }
    const string& maskedReason(int i) const 
    { 
      if((i<0)||(i>=nMaskedReasons()))
	{
	  OutOfRange err("ChannelMask::maskedReason",i);
	  err.stream() << "Attempt to access maskedReason(" << i 
		       << ") when there are only " << nMaskedReasons()
		       << " reasons why this channel is masked." << endl;
	  throw err;
	}
      return m_mask_reason[i]; }
  };
  
  class ChannelValDevAndMask
  {
  public:
    ChannelValDevAndMask(channelnum_type nchannels)
      : m_nchannels(nchannels), 
        m_val(nchannels), m_dev(nchannels), m_mask(nchannels) {}

    bool isCompatible(channelnum_type n) const 
    { return m_val.isCompatible(n); }

    void requireCompatible(channelnum_type n) const
    { return m_val.requireCompatible(n); }

    // SET ACCESSORS

    ChannelData<double>& val() { return m_val; }
    ChannelData<double>& dev() { return m_dev; }
    ChannelData<ChannelMask>& mask() { return m_mask; }
    
    double& val(channelnum_type i) { return m_val(i); }
    double& dev(channelnum_type i) { return m_dev(i); }
    ChannelMask& mask(channelnum_type i) { return m_mask(i); }

    void setVal(channelnum_type i, double value) { m_val(i)=value; }
    void setDev(channelnum_type i, double value) { m_dev(i)=value; }
    void setMask(channelnum_type i, const ChannelMask& value) 
    { m_mask(i)=value; }

    // GET ACCESSORS

    double val(channelnum_type i) const { return m_val(i); }
    double dev(channelnum_type i) const { return m_dev(i); }
    const ChannelMask& mask(channelnum_type i) const { return m_mask(i); }

    const ChannelData<double>& vals() const { return m_val; }
    const ChannelData<double>& devs() const { return m_dev; }
    const ChannelData<ChannelMask>& masks() const { return m_mask; }

    channelnum_type nchannels() const { return m_nchannels; }

    void addYourMask(ChannelData<ChannelMask>& o,
		     const string& prefix="") const;

  private:
    channelnum_type m_nchannels;
    ChannelData<double> m_val;
    ChannelData<double> m_dev;
    ChannelData<ChannelMask> m_mask; 
  };

  inline void 
  ChannelValDevAndMask::
  addYourMask(ChannelData<ChannelMask>& o, const string& prefix) const
  {
    m_mask.requireCompatible(o);
    ChannelData<ChannelMask>::const_iterator i = m_mask.begin();
    ChannelData<ChannelMask>::iterator j = o.begin();
    while(i != m_mask.end())
      {
	j->addMask(*i,prefix);
	i++;
	j++;
      }
  }

  ostream& operator<<(ostream& stream, const ChannelValDevAndMask& p);

} // NS_Analysis

/*
template<class T> T NS_Analysis::ChannelData<T>::mean() const
{
  T sum;
  for(i=0; i<nchannels(); i++)sum+=m_data[i];
  return sum/nchannels();
}

template<class T> T NS_Analysis::ChannelData<T>:: variance() const
{
  T mean=mean()
  T sumsq;
  for(i=0; i<nchannels(); i++)sumsq+=m_data[i]*m_data[i];
  return sumsq/nchannels() - mean*mean;
}

template<class T> T NS_Analysis::ChannelData<T>:: median() const
{
  vector<T> sorteddata=m_data;
  
}
*/

#endif // CHANNELDATA_H
