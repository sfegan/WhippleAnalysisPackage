#include <iostream>
#include <iomanip>

#include "AlphaPlotGenerator.h"

NS_Analysis::AlphaPlotGenerator::Results::
Results(const Results& r):
  m_nfiles(r.m_nfiles), m_livetime(r.m_livetime), m_raw(r.m_raw), 
  m_image3(r.m_image3), m_trigger(r.m_trigger), m_shape(r.m_shape), 
  m_orient(r.m_orient), m_on(r.m_on), m_off(r.m_off),
  m_alphaplot(r.m_alphaplot.binSize(),NS_Analysis::Binner::BIN_CENTER) 
{
}

std::ostream& 
NS_Analysis::
operator<< (ostream& str, const AlphaPlotGenerator::Results& r)
{
  std::ios::fmtflags fl = str.flags();
  str << resetiosflags(std::ios::floatfield) 
      << setiosflags(std::ios::fixed)
      << std::setw(3) << std::setprecision(3) << r.nFiles() << ' '
      << std::setw(9) << std::setprecision(1) << r.liveTime() << ' '
      << std::setw(6) << std::setprecision(6) << r.nPassedOn() << ' ' 
      << std::setw(6) << std::setprecision(6) << r.nPassedOff() << ' '
      << std::setw(8) << std::setprecision(8) << r.nPassedRaw() << ' ' 
      << std::setw(8) << std::setprecision(8) << r.nPassedImage3() << ' ' 
      << std::setw(7) << std::setprecision(7) << r.nPassedShape() << ' ' 
      << std::setw(7) << std::setprecision(7) << r.nPassedOrient();
  
  int zeroBin=r.alphaPlot().valToBin(0);
  for(int i=0;i<18;i++)
    str << ' ' << std::setw(5) << std::setprecision(5) 
	<< int(r.alphaPlot().binCount(zeroBin+i));
  
  str.flags(fl);
  return str;
}

void 
NS_Analysis::AlphaPlotGenerator::Results::
clear()
{
  m_nfiles   = 0;
  m_livetime = 0;
  m_raw      = 0;
  m_image3   = 0;
  m_trigger  = 0;
  m_shape    = 0;
  m_orient   = 0;
  m_on       = 0;
  m_off      = 0;
  m_alphaplot.clear();
}

void 
NS_Analysis::AlphaPlotGenerator::Results::
insert(const Results& r)
{
  m_nfiles   += r.m_nfiles;
  m_livetime += r.m_livetime;
  m_raw      += r.m_raw;
  m_image3   += r.m_image3;
  m_trigger  += r.m_trigger;
  m_shape    += r.m_shape;
  m_orient   += r.m_orient;
  m_on       += r.m_on;
  m_off      += r.m_off;
  m_alphaplot.insert(r.m_alphaplot);
}

NS_Analysis::AlphaPlotGenerator::
~AlphaPlotGenerator()
{
  
}

void 
NS_Analysis::AlphaPlotGenerator::
operateOnEvent(int evno, const EventInfo* ei, const HillasParam* hp)
{
  CutsPassedHillasParam cp;

  bool passed=m_cuts->test(*hp,cp);
  
  bool passed_raw        = true;
  bool passed_image3     = false;
  bool passed_trigger    = false;
  bool passed_shape      = false;
  bool passed_on_orient  = false;
  bool passed_off_orient = false;
  //  bool passed_on         = false;
  //  bool passed_off        = false;
  
  if(cp.passedLowerNImage())passed_image3=true;

  if((passed_image3)&&
     (cp.passedLowerMax1()) &&
     (cp.passedLowerMax2()) &&
     (cp.passedLowerMax3()) &&
     (cp.passedLowerSize()) &&
     (cp.passedUpperLengthOverSize()) &&
     (cp.passedLowerLengthOverSize())) 
    passed_trigger=true;

  if((passed_trigger)&&
     (cp.passedLowerLength()) && (cp.passedUpperLength())&&
     (cp.passedLowerWidth())  && (cp.passedUpperWidth()) &&
     (cp.passedLowerDist())   && (cp.passedUpperDist())) 
    passed_shape=true;
  
  if((passed_trigger)&&
     (hp->alpha()>=m_on_alpha_lower) && (hp->alpha()<=m_on_alpha_upper))
    passed_on_orient=true;
  
  if((passed_trigger)&&
     (hp->alpha()>=m_off_alpha_lower) && (hp->alpha()<=m_off_alpha_upper))
    passed_off_orient=true;
  
  /////////////////////////////////////////////////////////////////////////////

  if(passed_raw)m_file.nPassedRaw()++;

  if(passed_image3)m_file.nPassedImage3()++;

  if(passed_trigger)m_file.nPassedTrigger()++;

  if(passed_shape)m_file.nPassedShape()++;

  if(passed_on_orient)m_file.nPassedOrient()++;

  if((passed)&&(passed_on_orient))m_file.nPassedOn()++;

  if((passed)&&(passed_off_orient))m_file.nPassedOff()++;

  /////////////////////////////////////////////////////////////////////////////

  if(passed)
    {
      try
	{
	  m_file.alphaPlot().insert(hp->alpha());
	}
      catch (std::bad_alloc)
	{
	  std::cerr << "Alphaplot insertion: " << hp->alpha() << std::endl;
	}
    }
}

void 
NS_Analysis::AlphaPlotGenerator::
cut(ParamFile* pf)
{
  m_file.clear();
  
  RedHeader rh;
  pf->header()->read(0,&rh,1);
  m_file.nFiles()++;
  m_file.liveTime() += rh.live_time();
  
  ParamEventSelectedOperator peso(+this,m_sel.get());
  ParamFileEventProcessor pfproc(&peso,m_pb);
  pfproc.run(pf);

  m_total.insert(m_file);
}
