//-*-mode:c++; mode:font-lock;-*-

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#define __STL_USE_NAMESPACES
#include<cmath>

#include<iostream>
#include<string>
#include<vector>
#include<stdexcept>

namespace NS_Analysis
{
  
  template<class T>
  class Histogram
  {
  public:
    typedef unsigned int                 count_type;
    typedef vector<count_type>           datastore_type;
    typedef datastore_type::size_type    size_type;
    typedef enum { HA_CENTER, HA_START } alignment;
    
  private:
    typedef datastore_type dst;
    
    count_type     zero;
    
    alignment      align;
    T              binsize;
    
    int pzero;
    datastore_type ndata;
    datastore_type pdata;
    
    unsigned int nval;
    
    // Manipulate the negative and positive data stores
    dst& store(int bin) { return bin<0?ndata:pdata; }
    const dst& store(int bin) const { return bin<0?ndata:pdata; }
    
    int storebin(int bin) const { return bin<0?(-1-bin):bin; }
    void extend(dst &s, int sb) { if(s.size() <= sb)s.resize(sb+1); }
    
    count_type& storebinval(int b) 
      { dst& s=store(b); int sb=storebin(b); extend(s,sb); return s[sb]; }
    
    const count_type& storebinval(int b) const
    { const dst& s=store(b); int sb=storebin(b); 
    return((sb<s.size())?s[sb]:zero); }
    
  public:
    Histogram(const T& bs=1, alignment a=HA_CENTER): 
      align(a), binsize(bs), pdata(), ndata(), nval(0), zero(), pzero(0) {}

    Histogram(const Histogram& h):
      align(h.align), binsize(h.binsize), pdata(h.pdata), ndata(h.ndata), 
      nval(h.nval), zero(h.zero), pzero(h.pzero) {}

    const T& binSize() const { return binsize; }
    
    // Map T values to ints
    int valToBin(const T& x) const
      { return int(floor(double(x/binsize)+(align==HA_CENTER?0.5:0.0)))-pzero; }
    T binToVal(int b) const
      { return(T(double(b+pzero) + (align!=HA_CENTER?0.5:0.0))*binsize); }
    
    // Insert elements
    void insert(const T& x);
    
    // Size
    unsigned int nvalues() const { return nval; }

    int minBin() const { return -ndata.size(); }
    int maxBin() const { return pdata.size()-1; }
    
    const count_type binCount(int bin) const { return storebinval(bin); }

    T mean() const;
    T var() const;
    T stdev() const { return sqrt(var()); }
    T median() const;

    ostream& dump(ostream& stream) const;
    ostream& graph(ostream& stream) const;
  };

}; // namespace NS_Analysis

template<class T>
inline void
NS_Analysis::Histogram<T>::insert(const T& x)
{ 
#ifdef DEBUG
  cerr << "Value: " << x << " bin:" << valToBin(x) 
       << " store:" << ((valToBin(x)<0)?'n':'p') 
       << " storebin: " << storebin(valToBin(x)) << '\n';
#endif
  int b=valToBin(x); 
  if(nval==0)
    {
      pzero=b;
      b=0;
    } 
  
  storebinval(b)++; 
  nval++; 
}

template<class T>
inline T
NS_Analysis::Histogram<T>::mean() const
{ 
  if(nval==0)return T();

  unsigned int psum=0;
  int psize=pdata.size();
  for(int pb=0;pb<psize;pb++)psum+=pdata[pb]*pb;

  unsigned int nsum=0;
  int nsize=ndata.size();
  for(int nb=0;nb<nsize;nb++)nsum+=ndata[nb]*(nb+1);

#ifdef DEBUG
  cerr << "Histogram::mean: psize: " << psize << '\n';
  cerr << "Histogram::mean: nsize: " << nsize << '\n';
  cerr << "Histogram::mean: psum: " << psum << '\n';
  cerr << "Histogram::mean: nsum: " << nsum << '\n';
#endif

  T sum=binsize*(T(psum) - T(nsum) + 
		 T(nval*(double(pzero) + (align!=HA_CENTER?0.5:0.0))));

  return sum/T(nval);
}

template<class T>
inline T
NS_Analysis::Histogram<T>::var() const
{ 
  if(nval<=1)return T();

  unsigned int psum=0;
  unsigned int psumsq=0;
  int psize=pdata.size();
  for(int pb=0;pb<psize;pb++)
    {
      unsigned int count=pdata[pb];
      psum+=count*pb;
      psumsq+=count*pb*pb;
    }

  unsigned int nsum=0;
  unsigned int nsumsq=0;
  int nsize=ndata.size();
  for(int nb=0;nb<nsize;nb++)
    {
      unsigned int count=ndata[nb];
      unsigned int val=nb+1;
      nsum+=count*val;
      nsumsq+=count*val*val;
    }

#ifdef DEBUG
  cerr << "Histogram::var: psize: " << psize << '\n';
  cerr << "Histogram::var: nsize: " << nsize << '\n';
  cerr << "Histogram::var: psum: " << psum << '\n';
  cerr << "Histogram::var: nsum: " << nsum << '\n';
  cerr << "Histogram::var: psumsq: " << psumsq << '\n';
  cerr << "Histogram::var: nsumsq: " << nsumsq << '\n';
#endif

  double sum=binsize*
    (double(psum) - double(nsum) + 
     (double(nval)*(double(pzero) + (align!=HA_CENTER?0.5:0.0))));
  
  double sumsq=binsize*binsize*
    (double(psumsq) + double(nsumsq) + 
     (2.0*(double(psum) - double(nsum))*(double(pzero) + (align!=HA_CENTER?0.5:0.0)))+
     (double(nval)*pow(double(pzero) + (align!=HA_CENTER?0.5:0.0),2.0)));

  return T(sumsq/double(nval)-(sum/double(nval))*(sum/double(nval)));
}

template<class T>
inline T
NS_Analysis::Histogram<T>::median() const
{
  if(nval==0)return T();

  unsigned int halfWayPoint=nval/2+(nval%2==0?0:1);
  unsigned int np=0;
  
  int bin=ndata.size();
  while(bin>0)
    {
      bin--;
      np+=ndata[bin];
      if(np>=halfWayPoint)break;
    }

  if(np<halfWayPoint)  /* we haven't found the half way pount */
    {
      int psize=pdata.size();
      for(bin=0;bin<psize;bin++)
	{
	  np+=pdata[bin];
	  if(np>=halfWayPoint)break;
	}

      if((np==halfWayPoint)&&(nval%2==0)) /* found the half halfway point */
	{
	  T val=binToVal(bin++);
	  for(;bin<psize;bin++)if(pdata[bin])return (val+binToVal(bin))*0.5;
	}
      
      return binToVal(bin);
    }
  else if((np==halfWayPoint)&&(nval%2==0)) /* found the half halfway point */
    {
      T val=binToVal(-1-bin);
      while(bin>0)
	{
	  bin--;
	  if(ndata[bin])return (val+binToVal(-1-bin))*0.5;
	}

      int psize=pdata.size();
      for(bin=0;bin<psize;bin++)if(pdata[bin])return (val+binToVal(bin))*0.5;
    }
  else /* we found the halfway point ! */
    {
      return binToVal(-1-bin);
    }
}

template<class T>
inline ostream& 
NS_Analysis::Histogram<T>::dump(ostream& stream) const
{
  int maxbin=maxBin();
  for(int bin=minBin();bin<=maxbin;bin++)
    stream << binToVal(bin) << ' ' << binCount(bin) << '\n';
  return stream;
}

template<class T>
inline ostream& 
NS_Analysis::Histogram<T>::graph(ostream& stream) const
{
  int maxbin=maxBin();

  unsigned maxcount=0;
  for(int bin=minBin();bin<=maxbin;bin++)
    if(binCount(bin) > maxcount)maxcount=binCount(bin);

  for(int bin=minBin();bin<=maxbin;bin++)
    stream << string(int(floor(double(binCount(bin))/double(maxcount)*80)),'=')
	   << '\n';
  return stream;
}

#endif // HISTOGRAM_H
