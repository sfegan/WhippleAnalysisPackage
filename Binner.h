//-*-mode:c++; mode:font-lock;-*-

#ifndef BINNER_H
#define BINNER_H

#define __STL_USE_NAMESPACES
#include<cmath>

#include<iostream>
#include<string>
#include<vector>
#include<utility>
#include<stdexcept>

namespace NS_Analysis
{
  using std::vector;
  using std::endl;
  using std::pair;
  using std::ostream;

  class BinnerBaseDefs
  {
  public:
    typedef enum { BIN_CENTER, BIN_START } BinAlignment;
  };
  
  template<class T> class BinnerBase: public BinnerBaseDefs
  {
  public:
    BinnerBase(const double& bs, BinAlignment a=BIN_CENTER, const T& z=T()):
      align(a), binsize(bs), pdata(0,z), ndata(0,z), 
      pzero(0), active(0), zero(z) {}

    BinnerBase(const BinnerBase& h):
      align(h.align), binsize(h.binsize), pdata(h.pdata), ndata(h.ndata), 
      pzero(h.pzero), active(h.active), zero(h.zero) {}

    virtual ~BinnerBase() {}

    double binSize() const { return binsize; }
    
    int valToBin(double x) const
    { return int(floor(double(x/binsize)+(align==BIN_CENTER?0.5:0.0)))-pzero; }

    double binToVal(int b) const
    { return((double(b+pzero) + (align!=BIN_CENTER?0.5:0.0))*binsize); }
    
    int minBin() const { return -ndata.size(); }
    int maxBin() const { return pdata.size()-1; }

    double minVal() const { return binToVal(minBin()); }
    double maxVal() const { return binToVal(maxBin()); }

    virtual void clear();

  protected:
    T& getBinByBinNo(int bin);
    const T& getBinByBinNo(int bin) const;
    
    T& getBinByVal(double val) { return getBinByBinNo(valToBin(val)); }
    const T& getBinByVal(double val) const 
    { return getBinByBinNo(valToBin(val)); }
    
  private:
    typedef vector<T>              datastore_type;
    typedef datastore_type         dst;
    
    BinAlignment   align;
    double         binsize;
    
    datastore_type pdata;
    datastore_type ndata;
    int            pzero;
    
    int            active;
    T              zero;

    //
    // HELPER FUNCTIONS
    //
    
    // which store does this bin belong in, the negative or positive one
    dst& store(int bin) { return bin<0?ndata:pdata; }
    const dst& store(int bin) const { return bin<0?ndata:pdata; }
    
    // return the bin within the store.. ie if bin<0 we want -1-bin
    unsigned int storebin(int bin) const 
    { return bin<0?unsigned(-1-bin):unsigned(bin); }
    void extend(dst &s, unsigned int sb) 
    { if(s.size() <= sb)s.resize(sb+1,zero); }
    
    // return the actual bin itself for manipulation
    T& storebinval(int b) 
    { dst& s=store(b); int sb=storebin(b); extend(s,sb); return s[sb]; }
    
    const T& storebinval(int b) const
    { const dst& s=store(b); 
    unsigned int sb=storebin(b); return((sb<s.size())?s[sb]:zero); }
  };

  template<class T> inline T& BinnerBase<T>::getBinByBinNo(int bin)
  {
    if(active==0)
      {
	pzero=bin;
	bin=0;
	active=1;
      } 
    return storebinval(bin);
  }
  
  template<class T> inline const T& BinnerBase<T>::getBinByBinNo(int bin) const
  {
    if(active==0)return zero;
    return storebinval(bin);
  }

  template<class T> void BinnerBase<T>::clear()
  {
    pdata.clear();
    ndata.clear();
    pzero=0;
    active=0;
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  class Binner: public BinnerBase<pair<bool,double> >
  {
  public:
    typedef pair<bool,double> el_type;

    Binner(const double& bs, BinAlignment a=BinnerBaseDefs::BIN_CENTER):
      BinnerBase<el_type>(bs,a,el_type(false,0)) {}

    Binner(const Binner& h):
      BinnerBase<el_type>(h) {}
    
    virtual ~Binner();

    virtual void insert(double val, double count=1.0) = 0;
    virtual bool binOccupied(int bin) const;
    virtual double binCount(int bin) const = 0;

    double valCount(double val) const { return binCount(valToBin(val)); }
  };

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  class MaxFinder: public Binner
  {
  public:
    MaxFinder(const double& bs, BinAlignment a=BIN_CENTER): 
      Binner(bs,a) {}
    
    MaxFinder(const MaxFinder& h):
      Binner(h) {}

    virtual ~MaxFinder();
      
    virtual void insert(double val, double count=1.0);
    virtual double binCount(int bin) const;
  };
  
///////////////////////////////////////////////////////////////////////////////
  
  class MinFinder: public Binner
  {
  public:
    MinFinder(const double& bs, BinAlignment a=BIN_CENTER): 
      Binner(bs,a) {}
    
    MinFinder(const MinFinder& h):
      Binner(h) {}
    
    virtual ~MinFinder();
    
    virtual void insert(double val, double count=1.0);
    virtual double binCount(int bin) const;
  };
  
///////////////////////////////////////////////////////////////////////////////

  class Summer: public Binner
  {
  public:
    Summer(const double& bs, BinAlignment a=BIN_CENTER): 
      Binner(bs,a) {}
    
    Summer(const Summer& h):
      Binner(h) {}
    
    virtual ~Summer();

    double totalCount() const;
    double mean() const;
    double variance() const;
    
    void getStats(double& totalCount, double& mean, double& variance) const;

    double median() const;
    
    virtual void insert(double val, double count=1.0);
    void insert(const Summer& o);
    virtual double binCount(int bin) const;

  private:
    void getCounts(double& totalCount, 
		   double& valTimesCount, double& valTimesCountSquared) const;
  };
  
  inline void Summer::getCounts(double& totalCount, double& valTimesCount, 
				double& valTimesCountSquared) const
  {
    totalCount=0;
    valTimesCount=0;
    valTimesCountSquared=0;
    
    int minbin=minBin();
    int maxbin=maxBin();
    for(int bin=minbin;bin<=maxbin;bin++)
      {
	double val=binToVal(bin);
	double count=binCount(bin);
	totalCount += count;
	valTimesCount += val*count;
	valTimesCountSquared += val*val*count;
      }
  }

  inline double Summer::totalCount() const
  {
    double totalCount;
    double valTimesCount;
    double valTimesCountSquared;
    getCounts(totalCount,valTimesCount,valTimesCountSquared);
    return totalCount;
  }

  inline double Summer::mean() const
  {
    double totalCount;
    double valTimesCount;
    double valTimesCountSquared;
    getCounts(totalCount,valTimesCount,valTimesCountSquared);
    if(totalCount==0)return 0;
    else return valTimesCount/totalCount;
  }

  inline double Summer::variance() const
  {
    double totalCount;
    double valTimesCount;
    double valTimesCountSquared;
    getCounts(totalCount,valTimesCount,valTimesCountSquared);
    if(totalCount==0)return 0;
    double mean=valTimesCount/totalCount;
    return valTimesCountSquared/totalCount-mean*mean;
  }
  
  inline void 
  Summer::getStats(double& totalCount, double& mean, double& variance) const
  {
    double valTimesCount;
    double valTimesCountSquared;
    getCounts(totalCount,valTimesCount,valTimesCountSquared);
    if(totalCount==0)mean=variance=0;
    else 
      { 
	mean=valTimesCount/totalCount; 
	variance=valTimesCountSquared/totalCount-mean*mean;
      }
  }

  inline double Summer::median() const
  {
    double totalCount=0;
    int minbin=minBin();
    int maxbin=maxBin();
    for(int bin=minbin;bin<=maxbin;bin++)totalCount+=binCount(bin);
    if(totalCount==0)return 0;
    
    double medianCount=floor(totalCount/2);
    double count=0;
    int bin=0;
    for(bin=minbin;bin<=maxbin;bin++)
      {
	count+=binCount(bin);
	if(count>=medianCount)break;
      }

    if(count>medianCount)return binToVal(bin);
    else
      {
	double firstBinVal=binToVal(bin);
	for(bin++;bin<=maxbin;bin++)if(binCount(bin)>=0)break;
	double secondBinVal=binToVal(bin);
	return(firstBinVal+secondBinVal)/2;
      }
  }

  inline void dumpBinner(const Binner &binner, ostream &stream)
  {
    int minbin=binner.minBin();
    int maxbin=binner.maxBin();
    for(int bin=minbin;bin<=maxbin;bin++)
      stream << binner.binToVal(bin) << '\t' << binner.binCount(bin) << endl;
  }  

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  template<class B>
  class TwoDimBinner: private BinnerBase<B>
  {
  public:
    typedef Binner::el_type el_type;
    typedef BinnerBaseDefs::BinAlignment BinAlignment;

    TwoDimBinner(const double& bs, BinAlignment a=BinnerBaseDefs::BIN_CENTER):
      BinnerBase<B>(bs,a,B(bs,a)) {}

    virtual ~TwoDimBinner();

    void insert(double xval, double yval, double count=1.0);
    void insert(const TwoDimBinner& o);

    double binCount(int xbin, int ybin) const;

    int valToXBin(double x, int ybin) const;
    int valToXBin(double x, double y) const;

    int valToYBin(double y) const;
    int valToYBin(double x, double y) const;

    double binToXVal(int xb, int yb) const;
    double binToYVal(int xb, int yb) const;
    
    int maxYBin() const;
    int minYBin() const;

    int maxXBin(int ybin) const;
    int minXBin(int ybin) const;

    double maxYVal() const;
    double minYVal() const;
    double maxXVal() const;
    double minXVal() const;

    virtual void clear();
  };

  template<class B> 
  TwoDimBinner<B>::~TwoDimBinner()
  {
  }

  template<class B> void 
  TwoDimBinner<B>::insert(double xval, double yval, double count)
  {
    const int ybin=valToYBin(xval,yval);
    BinnerBase<B>::getBinByBinNo(ybin).insert(xval,count);
  }

  template<class B> void 
  TwoDimBinner<B>::insert(const TwoDimBinner& o)
  {
    int oy_minbin=o.minBin();
    int oy_maxbin=o.maxBin();
    for(int oy_bin=oy_minbin;oy_bin<=oy_maxbin;oy_bin++)
      {
	double y_val=o.binToVal(oy_bin);
	int ty_bin=BinnerBase<B>::valToBin(y_val);
	BinnerBase<B>::getBinByBinNo(ty_bin).insert(o.getBinByBinNo(oy_bin));
      }
  }
  
  template<class B> double 
  TwoDimBinner<B>::binCount(int xbin, int ybin) const
  {
    return BinnerBase<B>::getBinByBinNo(ybin).binCount(xbin);
  }

  template<class B> inline int
  TwoDimBinner<B>::valToXBin(double x, double y) const
  {
    const int ybin=BinnerBase<B>::valToBin(y);
    const int xbin=BinnerBase<B>::getBinByBinNo(ybin).valToBin(x);
    return xbin;
  }

  template<class B> inline int
  TwoDimBinner<B>::valToXBin(double x, int ybin) const
  {
    const int xbin=BinnerBase<B>::getBinByBinNo(ybin).valToBin(x);
    return xbin;
  }

  template<class B> inline int
  TwoDimBinner<B>::valToYBin(double y) const
  {
    const int ybin=BinnerBase<B>::valToBin(y);
    return ybin;
  }

  template<class B> inline int
  TwoDimBinner<B>::valToYBin(double x, double y) const
  {
    const int ybin=BinnerBase<B>::valToBin(y);
    return ybin;
  }

  template<class B> inline double 
  TwoDimBinner<B>::binToXVal(int xb, int yb) const
  {
    return BinnerBase<B>::getBinByBinNo(yb).binToVal(xb);
  }

  template<class B> inline double 
  TwoDimBinner<B>::binToYVal(int xb, int yb) const
  {
    return BinnerBase<B>::binToVal(yb);
  }
  
  template<class B> inline int 
  TwoDimBinner<B>::minYBin() const
  {
    return BinnerBase<B>::minBin();
  }

  template<class B> inline int 
  TwoDimBinner<B>::maxYBin() const
  {
    return BinnerBase<B>::maxBin();
  }

  template<class B> inline int 
  TwoDimBinner<B>::maxXBin(int ybin) const
  {
    return BinnerBase<B>::getBinByBinNo(ybin).maxBin();
  }

  template<class B> inline int 
  TwoDimBinner<B>::minXBin(int ybin) const
  {
    return BinnerBase<B>::getBinByBinNo(ybin).minBin();
  }

  template<class B> double TwoDimBinner<B>::maxYVal() const
  {
    return BinnerBase<B>::binToVal(maxYBin());
  }

  template<class B> double TwoDimBinner<B>::minYVal() const
  {
    return BinnerBase<B>::binToVal(minYBin());
  }

  template<class B> double TwoDimBinner<B>::maxXVal() const
  {
    int minY=minYBin();
    int maxY=maxYBin();
    
    bool defined;
    double maxX;
    for(int bin=minY; bin<=maxY; bin++)
      {
	double max=binToXval(maxXBin(bin),bin);
	if((!defined)||(max>maxX))maxX=max,defined=true;
      }
    return maxX;
  }
  
  template<class B> double TwoDimBinner<B>::minXVal() const
  {
    int minY=minYBin();
    int maxY=maxYBin();
    
    bool defined;
    double minX;
    for(int bin=minY; bin<=maxY; bin++)
      {
	double min=binToXval(minXBin(bin),bin);
	if((!defined)||(min<minX))minX=min,defined=true;
      }
    return minY;
  }
 
  template<class B> void 
  TwoDimBinner<B>::clear()
  {
    BinnerBase<B>::clear();
  }

} // namespace NS_Analysis

#endif // defined BINNER_H
