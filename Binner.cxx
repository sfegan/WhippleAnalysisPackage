#include"Binner.h"

using std::vector;

NS_Analysis::Binner::
~Binner()
{
  
}

bool
NS_Analysis::Binner::
binOccupied(int bin) const
{
  const el_type &bincount = getBinByBinNo(bin);
  return bincount.first;
}

void 
NS_Analysis::Summer::
insert(const Summer& o)
{
  int minbin=o.minBin();
  int maxbin=o.maxBin();
  for(int bin=minbin;bin<=maxbin;bin++)
    if(o.binOccupied(bin))insert(o.binToVal(bin), o.binCount(bin));
}

///////////////////////////////////////////////////////////////////////////////

NS_Analysis::MaxFinder::
~MaxFinder()
{

}

void 
NS_Analysis::MaxFinder::
insert(double val, double count)
{
  el_type &bincount = getBinByVal(val);
  if((!bincount.first)||(bincount.second < count))
    {
      bincount.first=true;
      bincount.second=count;
    }
}

double 
NS_Analysis::MaxFinder::
binCount(int bin) const
{
  return getBinByBinNo(bin).second;
}
  
///////////////////////////////////////////////////////////////////////////////

NS_Analysis::MinFinder::
~MinFinder()
{

}

void 
NS_Analysis::MinFinder::
insert(double val, double count)
{
  el_type &bincount = getBinByVal(val);
  if((!bincount.first)||(bincount.second > count))
    {
      bincount.first=true;
      bincount.second=count;
    }
}

double 
NS_Analysis::MinFinder::
binCount(int bin) const
{
  return getBinByBinNo(bin).second;
}
  
///////////////////////////////////////////////////////////////////////////////

NS_Analysis::Summer::
~Summer()
{

}

void 
NS_Analysis::Summer::
insert(double val, double count)
{
  el_type &bincount = getBinByVal(val);
  
  bincount.first=true;
  bincount.second+=count;
}

double 
NS_Analysis::Summer::
binCount(int bin) const
{
  return getBinByBinNo(bin).second;
}
  
///////////////////////////////////////////////////////////////////////////////

