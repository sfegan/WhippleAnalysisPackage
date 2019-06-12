//-*-mode:c++; mode:font-lock;-*-

int errno;

#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

#include <iostream>
#include <iomanip>

#include <cmath>

#include <assert.h>
#include <unistd.h>
#include <getopt.h>

#include "fz.h"

using namespace std;

#define PATCHCHAN  19
#define PATCHES     5
#define MODULES    13

template<class T, unsigned int N> class NTuple
{
public:
  NTuple(const T a[N]) { for(unsigned int i=0;i<N;i++)m_x[i]=a[i]; }
  bool operator<(NTuple o) const
  {
    for(unsigned int i=0;i<N;i++)
      {
	if(m_x[i]<o.m_x[i])return true; 
	else if(m_x[i]>o.m_x[i])return false;
      }
    return false;
  }
  const T& x(unsigned int i) const { return m_x[i]; }
private:
  T m_x[N];
};

class ChannelPatchInfo
{
public:
  ChannelPatchInfo(unsigned int cn, 
		   unsigned int pmn, unsigned int ppn, unsigned int pbn)
    : channel_num(cn), 
      pst_module_num(pmn), pst_patch_num(ppn), pst_bit_num(pbn) {}
  unsigned int channel_num;
  unsigned int pst_module_num;
  unsigned int pst_patch_num;
  unsigned int pst_bit_num;
};

// ----------------------------------------------------------------------------
// GDFPSTAnalyzer - examine PST patch data for "funny business"
// ----------------------------------------------------------------------------

class GDFPSTAnalyzer: public GDFRecordHandler
{
public:
  GDFPSTAnalyzer(const vector<ChannelPatchInfo>& patchinfo);

  virtual void ev10(GDFRecordDispatcher &dispatcher, 
		    const struct gdf_ev10_s& record);

  virtual void starting_file(GDFRecordDispatcher &dispatcher,
			     const std::string& filename);

  virtual void finished_file(GDFRecordDispatcher &dispatcher,
			     const std::string& filename);

private:
  string title(const string& message, unsigned int width = 80);
  void list(ostream& stream, vector<pair<unsigned int, string> > data,
	    unsigned limit=0, unsigned columns=0, bool dosort=true, 
	    unsigned int width=80, unsigned minspace=3);
  
  unsigned int keyMPB(unsigned int m, unsigned int p, unsigned int b)
  { return ((m*PATCHES)+p)*PATCHCHAN+b; }
  unsigned int unkeyM(unsigned int k) { return k/(PATCHCHAN*PATCHES); }
  unsigned int unkeyP(unsigned int k) { return (k/PATCHCHAN)%PATCHES; }
  unsigned int unkeyB(unsigned int k) { return k%PATCHCHAN; }

  vector<ChannelPatchInfo>                       m_patchinfo;

  vector<vector<vector<ChannelPatchInfo*> > >    m_pst_channel_unwind;
  vector<vector<ChannelPatchInfo*> >             m_pst_patch_unwind;

  unsigned int                                   m_nevents;

  vector<unsigned int>                           m_npatch_hist;

  map<unsigned int, unsigned int>                m_channel_hist;
  map<unsigned int, unsigned int>                m_module_hist;
  map<unsigned int, unsigned int>                m_patch_hist;
  map<unsigned int, unsigned int>                m_bit_hist;

  map<NTuple<unsigned int, 2>, unsigned int>     m_channel_pair_hist;
  map<NTuple<unsigned int, 2>, unsigned int>     m_bit_pair_hist;
  map<NTuple<unsigned int, 2>, unsigned int>     m_samemod_bit_pair_hist;
};

GDFPSTAnalyzer::GDFPSTAnalyzer(const vector<ChannelPatchInfo>& patchinfo)
  : m_patchinfo(patchinfo),
    m_pst_channel_unwind(MODULES,vector<vector<ChannelPatchInfo*> >
			 (PATCHES, vector<ChannelPatchInfo*>(PATCHCHAN)))
{
  for(vector<ChannelPatchInfo>::iterator i = m_patchinfo.begin();
      i!=m_patchinfo.end(); i++)
    {
      unsigned int mod  = i->pst_module_num;
      unsigned int pat  = i->pst_patch_num;
      unsigned int bit  = i->pst_bit_num;

      unsigned int chan = i->channel_num;

      if(m_pst_channel_unwind.size() <= mod)
	m_pst_channel_unwind.resize(mod+1,vector<vector<ChannelPatchInfo*> >
			    (PATCHES, vector<ChannelPatchInfo*>(PATCHCHAN)));
      
      if(m_pst_channel_unwind[mod].size() <= pat)
	m_pst_channel_unwind[mod].resize(pat+1,
				 vector<ChannelPatchInfo*>(PATCHCHAN));
      
      if(m_pst_channel_unwind[mod][pat].size() <= bit)
	m_pst_channel_unwind[mod][pat].resize(bit+1);

      m_pst_channel_unwind[mod][pat][bit]=&(*i);

      if(m_pst_patch_unwind.size() <= chan)
	m_pst_patch_unwind.resize(chan+1, vector<ChannelPatchInfo*>());

      m_pst_patch_unwind[chan].push_back(&(*i));
    }
} 

void 
GDFPSTAnalyzer::ev10(GDFRecordDispatcher &dispatcher, 
		     const struct gdf_ev10_s& record)
{
  set<unsigned int>             patch_channel_set;
  set<unsigned int>             channel_set;
  set<unsigned int>             module_set;

  unsigned int npatterns = record.ntrg;
  if(npatterns<39)m_npatch_hist[npatterns]++;
  else m_npatch_hist[39]++;

  for(unsigned int i=0; i<npatterns; i++)
    {
      unsigned int pattern = record.pattern[i];
      unsigned int module = ((pattern>>(PATCHCHAN+PATCHES))&0x007F) - 1;
      unsigned int patchcode = ((pattern>>PATCHCHAN)&0x001F);
      unsigned int npatchbits = 0;
      unsigned int patch = 0;
      for(unsigned int p=0;p<PATCHES;p++)
	if(patchcode&(0x0001<<p))
	  {
	    npatchbits++;
	    patch=p;
	  }

      if(npatchbits != 1)
	{
	  cerr << "MEANINGLESS WARNING: Patch info has " << npatchbits
	       << " bits set, value was " << patchcode 
	       << " ... ignoring" << endl;
	  continue;
	}

      module_set.insert(module);

      unsigned int key = keyMPB(module,patch,0);
      m_patch_hist[key]=m_patch_hist[key]+1;

      const vector<ChannelPatchInfo*>* 
	patchinfo = &m_pst_channel_unwind[module][patch];

      for(unsigned int bit=0;bit<PATCHCHAN;bit++)
	if(pattern&(0x0001<<bit))
	  {
	    unsigned int key = keyMPB(module, patch, bit);
	    m_bit_hist[key]=m_bit_hist[key]+1;

	    unsigned int channel = (*patchinfo)[bit]->channel_num;
	    channel_set.insert(channel);

	    unsigned int cskey = keyMPB(module, patch, bit);
	    patch_channel_set.insert(cskey);
	  }
    }

  for(set<unsigned int>::const_iterator i = module_set.begin();
      i!=module_set.end(); i++)m_module_hist[*i]=m_module_hist[*i]+1;

  for(set<unsigned int>::const_iterator i = channel_set.begin();
      i!=channel_set.end(); i++)
    {
      unsigned int channel = *i;
      m_channel_hist[channel]=m_channel_hist[channel]+1;

      for(set<unsigned int>::const_iterator j = channel_set.begin();
	  j!=channel_set.end(); j++)
	{
	  if((*j)<=(*i))continue;
	  unsigned int k[2] = { channel, *j };
	  NTuple<unsigned int, 2> key(k);
	  m_channel_pair_hist[key]=m_channel_pair_hist[key]+1;
	}
    }

  for(set<unsigned int>::const_iterator i=
	patch_channel_set.begin(); i!=patch_channel_set.end(); i++)
    {
      for(set<unsigned int>::const_iterator j=
	    patch_channel_set.begin(); j!=patch_channel_set.end(); j++)
	{
	  if((*j)<=(*i))continue;

	  unsigned int bk[2] = { *i, *j };
	  NTuple<unsigned int, 2> bkey(bk);
	  m_bit_pair_hist[bkey]=m_bit_pair_hist[bkey]+1;

	  if((unkeyM(*i)==unkeyM(*j))&&(unkeyP(*i)==unkeyP(*j)))
	    m_samemod_bit_pair_hist[bkey]=m_samemod_bit_pair_hist[bkey]+1;
	}
    }

  m_nevents++;
}

void 
GDFPSTAnalyzer::starting_file(GDFRecordDispatcher &dispatcher,
			      const std::string& filename)
{
  m_nevents=0;

  m_npatch_hist.clear();
  m_npatch_hist.resize(40,0);

  m_channel_hist.clear();
  m_module_hist.clear();
  m_patch_hist.clear();
  m_bit_hist.clear();

  m_channel_pair_hist.clear();
  m_bit_pair_hist.clear();
  m_samemod_bit_pair_hist.clear();;
}

void 
GDFPSTAnalyzer::finished_file(GDFRecordDispatcher &dispatcher,
			      const std::string& filename)
{
  vector<pair<unsigned int, string> > listdata;

  // ==========================================================================
  // PATCH COUNT HISTOGRAM
  // ==========================================================================
  cout << title("PATCH COUNT HISTOGRAM");
  listdata.clear();
  for(unsigned i=0;i<40;i++)
    {
      ostringstream str;
      str << setw(2) << i << ' ' << setw(5) << setiosflags(ios::left)
	  << m_npatch_hist[i];
      listdata.push_back(make_pair(i, str.str()));
    }
  list(cout, listdata, 0, 0, false);
  
  // ==========================================================================
  // MODULE TRIGGER COUNT
  // ==========================================================================
  cout << endl << title("MODULE TRIGGER COUNT SORTED BY COUNT");
  listdata.clear();
  for(map<unsigned int, unsigned int>::const_iterator i=m_module_hist.begin();
      i!=m_module_hist.end(); i++)
    {
      ostringstream str;
      str << setw(2) << i->first << ' ' << setw(5) << setiosflags(ios::left)
	  << i->second;
      listdata.push_back(make_pair(i->second, str.str()));
    }
  list(cout, listdata);
  cout << endl << title("MODULE TRIGGER COUNT SORTED BY MODULE");
  list(cout, listdata, 0, 0, false);
  
  // ==========================================================================
  // PATCH TRIGGER COUNT
  // ==========================================================================
  cout << endl << title("PATCH TRIGGER COUNT SORTED BY COUNT");
  listdata.clear();
  for(map<unsigned int, unsigned int>::const_iterator i =
	m_patch_hist.begin(); i!=m_patch_hist.end(); i++)
    {
      unsigned int m = unkeyM(i->first);
      unsigned int p = unkeyP(i->first);
      ostringstream str;
      str << setw(2) << m << '/' << setw(1) << p << ' ' 
	  << setw(5) << setiosflags(ios::left) << i->second;
      listdata.push_back(make_pair(i->second, str.str()));
    }
  list(cout, listdata);
  cout << endl << title("PATCH TRIGGER COUNT SORTED BY MODULE");
  list(cout, listdata, 0, 0, false);

  // ==========================================================================
  // CHANNEL COUNT
  // ==========================================================================
  cout << endl << title("CHANNEL TRIGGER COUNT (TOP 30)");
  listdata.clear();
  for(map<unsigned int, unsigned int>::const_iterator i =
	m_channel_hist.begin(); i!=m_channel_hist.end(); i++)
    {
      ostringstream str;
      str << setw(3) << i->first+1 
	  << ' ' << setw(5) << setiosflags(ios::left)
	  << i->second;
      listdata.push_back(make_pair(i->second, str.str()));
    }
  list(cout, listdata, 30);

  // ==========================================================================
  // PATCH CHANNEL COUNT
  // ==========================================================================

  cout << endl << title("PATCH CHANNEL TRIGGER COUNT (TOP 100)");
  listdata.clear();
  for(map<unsigned int, unsigned int>::const_iterator i =
	m_bit_hist.begin(); i!=m_bit_hist.end(); i++)
    {
      unsigned m = unkeyM(i->first);
      unsigned p = unkeyP(i->first);
      unsigned b = unkeyB(i->first);
      unsigned c = m_pst_channel_unwind[m][p][b]->channel_num+1;
      ostringstream str;
      str << setw(2) << m << '/' << setw(1) << p 
	  << '/' << setw(2) << setiosflags(ios::left) << b
	  << resetiosflags(ios::left)
	  << " (" << setw(3) << c << ':' 
	  << setw(2) << setiosflags(ios::left) << c/16+1
	  << ") " << setw(5) << setiosflags(ios::left)
	  << i->second;
      listdata.push_back(make_pair(i->second, str.str()));
    }
  list(cout, listdata, 100);

  // ==========================================================================
  // PAIR CHANNEL COUNT
  // ==========================================================================

  cout << endl << title("PAIR CHANNEL TRIGGER COUNT (TOP 100)");
  listdata.clear();
  for(map<NTuple<unsigned int, 2>, unsigned int>::const_iterator i =
	m_channel_pair_hist.begin(); i!=m_channel_pair_hist.end(); i++)
    {
      ostringstream str;
      str << setw(3) << i->first.x(0)+1 << '/' 
	  << setw(3) << setiosflags(ios::left) << i->first.x(1)+1
	  << ' ' << setw(5) << setiosflags(ios::left)
	  << i->second;
      listdata.push_back(make_pair(i->second, str.str()));
    }
  list(cout, listdata, 100);

  // ==========================================================================
  // PAIR PATCH CHANNEL COUNT
  // ==========================================================================

  cout << endl << title("PAIR PATCH CHANNEL TRIGGER COUNT (TOP 100)");
  listdata.clear();
  for(map<NTuple<unsigned int, 2>, unsigned int>::const_iterator i =
	m_bit_pair_hist.begin(); i!=m_bit_pair_hist.end(); i++)
    {
      unsigned m1 = unkeyM(i->first.x(0));
      unsigned p1 = unkeyP(i->first.x(0));
      unsigned b1 = unkeyB(i->first.x(0));
      unsigned c1 = m_pst_channel_unwind[m1][p1][b1]->channel_num+1;
      unsigned m2 = unkeyM(i->first.x(1));
      unsigned p2 = unkeyP(i->first.x(1));
      unsigned b2 = unkeyB(i->first.x(1));
      unsigned c2 = m_pst_channel_unwind[m2][p2][b2]->channel_num+1;
      ostringstream str;
      str << setw(2) << m1 << '/' << setw(1) << p1 
	  << '/' << setw(2) << setiosflags(ios::left) << b1
	  << resetiosflags(ios::left)
	  << ' ' << setw(3) << c1 << ':' 
	  << setiosflags(ios::left) << setw(2) << c1/16+1
	  << "  " << resetiosflags(ios::left)
	  << setw(2) << m2 << '/' << setw(1) << p2 
	  << '/' << setw(2) << setiosflags(ios::left) << b2
	  << resetiosflags(ios::left)
	  << ' ' << setw(3) << c2 << ':' 
	  << setiosflags(ios::left) << setw(2) << c2/16+1
	  << ' ' << setw(5) << setiosflags(ios::left)
	  << i->second;
      listdata.push_back(make_pair(i->second, str.str()));
    }
  list(cout, listdata, 100);

  // ==========================================================================
  // SAME PATCH PAIR PATCH CHANNEL COUNT
  // ==========================================================================

  cout << endl << title("SAME PATCH PAIR PATCH CHANNEL TRIGGER COUNT (TOP 50)");
  listdata.clear();
  for(map<NTuple<unsigned int, 2>, unsigned int>::const_iterator i =
	m_samemod_bit_pair_hist.begin(); i!=m_samemod_bit_pair_hist.end(); i++)
    {
      unsigned m1 = unkeyM(i->first.x(0));
      unsigned p1 = unkeyP(i->first.x(0));
      unsigned b1 = unkeyB(i->first.x(0));
      unsigned c1 = m_pst_channel_unwind[m1][p1][b1]->channel_num+1;
      unsigned m2 = unkeyM(i->first.x(1));
      unsigned p2 = unkeyP(i->first.x(1));
      unsigned b2 = unkeyB(i->first.x(1));
      unsigned c2 = m_pst_channel_unwind[m2][p2][b2]->channel_num+1;
      ostringstream str;
      str << setw(2) << m1 << '/' << setw(1) << p1 
	  << '/' << setw(2) << setiosflags(ios::left) << b1
	  << resetiosflags(ios::left)
	  << ' ' << setw(3) << c1 << ':' 
	  << setiosflags(ios::left) << setw(2) << c1/16+1
	  << "  " << resetiosflags(ios::left)
	  << setw(2) << m2 << '/' << setw(1) << p2 
	  << '/' << setw(2) << setiosflags(ios::left) << b2
	  << resetiosflags(ios::left)
	  << ' ' << setw(3) << c2 << ':' 
	  << setiosflags(ios::left) << setw(2) << c2/16+1
	  << ' ' << setw(5) << setiosflags(ios::left)
	  << i->second;
      listdata.push_back(make_pair(i->second, str.str()));
    }
  list(cout, listdata, 50);
}

string GDFPSTAnalyzer::title(const string& message, unsigned int width)
{
  unsigned int len = message.length()+2;
  ostringstream stream;
  stream << string((width-len)/2,'=') << ' ' << message << ' ' 
	 << string((width-len+1)/2,'=') << endl;
  return stream.str();
}

template<class A, class B> class pair_first_more
{
public:
  bool operator()(const pair<A,B> & e1, const pair<A,B> & e2) const 
  { return e2.first < e1.first; }
};

void GDFPSTAnalyzer::list(ostream& stream, 
			  vector<pair<unsigned int, string> > data,
			  unsigned limit, unsigned columns, bool dosort, 
			  unsigned int width, unsigned minspace)
{
  if(dosort)sort(data.begin(), data.end(), 
		 pair_first_more<unsigned int, string>());

  unsigned int maxlen = 0;
  for(vector<pair<unsigned int, string> >::const_iterator i = data.begin(); 
      i!=data.end(); i++)
    if(i->second.length()>maxlen)maxlen=i->second.length();

  if((columns==0)&&(maxlen>width))columns=1;
  else if(columns==0)columns=1+(width-1-maxlen)/(maxlen+minspace);
  if(columns<=0)columns=1;

  if((limit==0)||(limit>data.size()))limit=data.size();
  if(limit==0)return;

  minspace = (width-1-columns*maxlen)/(columns-1);

  unsigned int rows = (limit+columns-1)/columns;
  for(unsigned int row = 0; row<rows; row++)
    {
      for(unsigned int col = 0; col<columns; col++)
	{
	  unsigned int n = col*rows+row;
	  if(n>=limit)continue;
	  if(col!=0)cout << string(minspace,' ');
	  cout << setw(maxlen) << setprecision(maxlen) 
	       << data[n].second.c_str();
	}
      cout << endl;
    }
}

// ----------------------------------------------------------------------------
// GDFPedDevCalculator - calculate pedestal val / dev directly from .fz file
// ----------------------------------------------------------------------------

class GDFPedDevCalculator: public GDFRecordHandler
{
public:
  GDFPedDevCalculator(unsigned int nchannel, short cr = 75);

  unsigned int nevents() const { return m_nevents; }
  double ped(unsigned int ch) const 
  { assert((ch>=0)&&(ch<m_nchannel)); return m_mean[ch]; }
  double dev(unsigned int ch) const
  { assert((ch>=0)&&(ch<m_nchannel)); return m_dev[ch]; }

  virtual void ev10(GDFRecordDispatcher &dispatcher, 
		    const struct gdf_ev10_s& record);

  virtual void starting_file(GDFRecordDispatcher &dispatcher,
			     const std::string& filename);

  virtual void finished_file(GDFRecordDispatcher &dispatcher,
			     const std::string& filename);

private:
  short          m_cr_threshold;

  unsigned int   m_nchannel;

  unsigned int   m_nevents;
  vector<double> m_sum;
  vector<double> m_sumsq;

  vector<double> m_mean;
  vector<double> m_dev;
};

GDFPedDevCalculator::GDFPedDevCalculator(unsigned int nchannel, short cr)
  :  m_cr_threshold(cr),
     m_nchannel(nchannel), m_nevents(0), m_sum(nchannel), m_sumsq(nchannel),
     m_mean(nchannel), m_dev(nchannel)
{
  // nothing to see here
}

void GDFPedDevCalculator::starting_file(GDFRecordDispatcher &dispatcher,
					const std::string& filename)
{
  m_nevents=0;
}

void GDFPedDevCalculator::finished_file(GDFRecordDispatcher &dispatcher,
					const std::string& filename)
{
  assert(m_nevents > 0);

  for(unsigned int i=0; i<m_nchannel; i++)
    {
      m_mean[i] = m_sum[i]/double(m_nevents);
      m_dev[i]  = sqrt(m_sumsq[i]/double(m_nevents) - m_mean[i]*m_mean[i]);
    }
}

void 
GDFPedDevCalculator::ev10(GDFRecordDispatcher &dispatcher, 
			  const struct gdf_ev10_s& record)
{
  int trigger = record.trigger;
  // if((trigger&0x1)==0)return; // pedestal events have bit 1 set

  // the above condition usually would be trigger!=0x01 but we want to
  // allow for the case when the PST is misbehaving so allow other
  // bits to be set also.

  unsigned int nchannel=m_nchannel;
  if(nchannel > record.nadc)nchannel=record.nadc;

  if(m_cr_threshold>=0)
    for(unsigned int i=0; i<nchannel; i++)
      if(record.adc[i] > m_cr_threshold)return;

  for(unsigned int i=0; i<nchannel; i++)
    {
      double adc=double(record.adc[i]);
      m_sum[i]   += adc;
      m_sumsq[i] += adc*adc;
    }

  m_nevents++;
}

void build_331_patch_map(vector<ChannelPatchInfo>& patchinfo)
{
  int pmtU[331];
  int pmtV[331];

  pmtU[0]=pmtV[0]=0;

  int segStartU[6] = { +1,+1,+0,-1,-1,+0 };
  int segStartV[6] = { +0,-1,-1,+0,+1,+1 };
  int segDeltaU[6] = { +0,-1,-1,+0,+1,+1 };
  int segDeltaV[6] = { -1,+0,+1,+1,+0,-1 };

  unsigned pmtnum=1;
  for(unsigned int ring=1; ring<11; ring++)
    for(unsigned int hexseg=0; hexseg<6; hexseg++)
      for(unsigned int segchan=0; segchan<ring; segchan++)
	{
	  pmtU[pmtnum] = segStartU[hexseg]*ring + segDeltaU[hexseg]*segchan;
	  pmtV[pmtnum] = segStartV[hexseg]*ring + segDeltaV[hexseg]*segchan;
	  pmtnum++;
	}
  
  int chanU[PATCHCHAN] = 
    { +0,-1,-2,-2,-2,+1,+0,-1,-1,-1,+2,+1,+0,+0,+0,+2,+1,+1,+2 };
  int chanV[PATCHCHAN] =
    { -2,-1,+0,+1,+2,-2,-1,+0,+1,+2,-2,-1,+0,+1,+2,-1,+0,+1,+0 };

  int patchU[PATCHES] = { -4,-2,+0,+2,+4 };
  int patchV[PATCHES] = { +0,+0,+0,+0,+0 };

  int moduleU[MODULES] = { +4, +4,+4,+4,+4, -2,+0,+2,+4, -2,-4,-6,-8 };
  int moduleV[MODULES] = { +0, -2,+0,+2,+4, -2,-4,-6,-8, +4,+4,+4,+4 };

  int moduleA[MODULES] = { +1, +1,+1,+1,+1, -1,-1,-1,-1, +0,+0,+0,+0 };
  int moduleB[MODULES] = { +0, +1,+1,+1,+1, +0,+0,+0,+0, -1,-1,-1,-1 };
  int moduleC[MODULES] = { +0, -1,-1,-1,-1, +0,+0,+0,+0, +1,+1,+1,+1 };
  int moduleD[MODULES] = { +1, +0,+0,+0,+0, -1,-1,-1,-1, +1,+1,+1,+1 };

  for(unsigned int module = 0; module<MODULES; module++)
    for(unsigned int patch = 0; patch<PATCHES;  patch++)
      for(unsigned int chan = 0; chan<PATCHCHAN; chan++)
	{
	  int modU = patchU[patch]+chanU[chan];
	  int modV = patchV[patch]+chanV[chan];

	  int A = moduleA[module];
	  int B = moduleB[module];
	  int C = moduleC[module];
	  int D = moduleD[module];

	  int camU = A*modU + B*modV + moduleU[module];
	  int camV = C*modU + D*modV + moduleV[module];

	  for(pmtnum=0;pmtnum<331; pmtnum++)
	    {
	      if((camU==pmtU[pmtnum])&&(camV==pmtV[pmtnum]))
		break;
	    }
	  
	  if(pmtnum != 331)
	    {
	      patchinfo.push_back(ChannelPatchInfo(pmtnum,module,patch,chan));
	    }
	  else
	    {
	      cerr << "Mod: " << module
		   << "  Patch: " << patch
		   << "  Chan: " << chan 
		   << "  (U=" << camU << ", " << "V=" << camV << ")" 
		   << " has no PMT" << endl;
	    }
	}
}

int main(int argc,char **argv)
{
  try
    {
      while (1)
	{
	  int this_option_optind = optind ? optind : 1;
	  int option_index = 0;
	  
	  static struct option long_options[] =
	    {
	      {"help",  0, 0, 'h'},
	      {0, 0, 0, 0}
	    };
	  
	  int c=getopt_long_only(argc, argv, "h", long_options, 
				 &option_index);
	  
	  if(c==-1)break;
	  
	  switch(c)
	    {
	    case 'h':
	      cout << "Usage: " << *argv << " [options] filename" << endl
		   << endl 
		   << "Options: " << endl
		;
	      exit(EXIT_SUCCESS);
	      
	    case '?':
	    default:
	      cerr << "Unknown option dumm dumm" << endl;
	      exit(EXIT_FAILURE);
	    }
	}

      char* progname=*argv;
      argc-=optind;
      argv+=optind;

      if(argc != 1)
	exit(EXIT_FAILURE);

      GDFSystem gdf;
      GDFRecordDispatcher dispatcher(0,true);

      // First pass, calculate the pedestals and variances
      //      GDFPedDevCalculator pedcalc(331);
      //      dispatcher.resetHandler(&pedcalc);
      //      dispatcher.process(*argv);

      // Second pass, analyze the PST data
      vector<ChannelPatchInfo> patchmap;
      build_331_patch_map(patchmap);
      GDFPSTAnalyzer analyzer(patchmap);
      dispatcher.resetHandler(&analyzer);
      dispatcher.process(*argv);
    }
  catch(const Error& x)
    {
      cerr << x;
    }
}
