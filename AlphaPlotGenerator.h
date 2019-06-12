//-*-mode:c++; mode:font-lock;-*-

#ifndef ALPHAPLOTGENERATOR_H
#define ALPHAPLOTGENERATOR_H

#include <iostream>

#include "Types.h"

#include "RedHeader.h"
#include "HillasParam.h"
#include "EventInfo.h"
#include "ParamFile.h"

#include "CutsHillasParam.h"

#include "Binner.h"

#include "ProgressBar.h"

namespace NS_Analysis {

  class AlphaPlotGenerator: private ParamEventOperator
  {
  public:
    AlphaPlotGenerator(const CutsHillasParam* cuts, 
		       double on_al_lo=0,
		       double on_al_hi=15,
		       double off_al_lo=20,
		       double off_al_hi=65,
		       double alphaBinWidth=5 /* degrees */,
		       ProgressBar* pb=0)
      : ParamEventOperator(), m_sel(new Code8PESelector,true),
	m_file(alphaBinWidth), m_total(alphaBinWidth),
	m_cuts(cuts), m_on_alpha_lower(on_al_lo), m_on_alpha_upper(on_al_hi),
	m_off_alpha_lower(off_al_lo), m_off_alpha_upper(off_al_hi), m_pb(pb) 
    {
    }

    AlphaPlotGenerator(const CutsHillasParam* cuts, 
		       MPtr<ParamEventSelector>& sel,
		       double on_al_lo=0,
		       double on_al_hi=15,
		       double off_al_lo=20,
		       double off_al_hi=65,
		       double alphaBinWidth=5 /* degrees */,
		       ProgressBar* pb=0)
      : ParamEventOperator(), m_sel(sel),
	m_file(alphaBinWidth), m_total(alphaBinWidth),
	m_cuts(cuts), m_on_alpha_lower(on_al_lo), m_on_alpha_upper(on_al_hi),
	m_off_alpha_lower(off_al_lo), m_off_alpha_upper(off_al_hi), m_pb(pb) 
    {
    }
    
    virtual ~AlphaPlotGenerator();

    virtual void operateOnEvent(int evno, 
				const EventInfo* ei, const HillasParam* hp);

    void cut(ParamFile* pf);

    class Results
    {
    public:
      Results(double binWidth)
	: m_nfiles(0), m_livetime(0), m_raw(0), m_image3(0), m_trigger(0), 
	m_shape(0), m_orient(0), m_on(0), m_off(0),
	m_alphaplot(binWidth,Binner::BIN_START) 
      {}

      Results(const Results& r);
      
      int nFiles() const         { return m_nfiles; }
      double liveTime() const    { return m_livetime; }
      int nPassedRaw() const     { return m_raw; }
      int nPassedImage3() const  { return m_image3; }
      int nPassedTrigger() const { return m_trigger; }
      int nPassedShape() const   { return m_shape; }
      int nPassedOrient() const  { return m_orient; }
      int nPassedOn() const      { return m_on; }
      int nPassedOff() const     { return m_off; }
      
      const Summer& alphaPlot() const { return m_alphaplot; }

      int& nFiles()              { return m_nfiles; }
      double& liveTime()         { return m_livetime; }
      int& nPassedRaw()          { return m_raw; }
      int& nPassedImage3()       { return m_image3; }
      int& nPassedTrigger()      { return m_trigger; }
      int& nPassedShape()        { return m_shape; }
      int& nPassedOrient()       { return m_orient; }
      int& nPassedOn()           { return m_on; }
      int& nPassedOff()          { return m_off; }

      Summer& alphaPlot()        { return m_alphaplot; }

      void clear();
      void insert(const Results& r);

    private:
      
      int m_nfiles;

      double m_livetime;

      int m_raw;
      int m_image3;
      int m_trigger;
      int m_shape;
      int m_orient;
      int m_on;
      int m_off;

      Summer m_alphaplot;

      friend class AlphaPlotGenerator;
    };

    const Results& fileResults()  const { return m_file; }
    const Results& totalResults() const { return m_total; }
    
  private:    
    MPtr<ParamEventSelector> m_sel;

    Results m_file;
    Results m_total;

    const CutsHillasParam* m_cuts;

    double m_on_alpha_lower;
    double m_on_alpha_upper;
    double m_off_alpha_lower;
    double m_off_alpha_upper;

    ProgressBar* m_pb;
  };

  ostream& operator<< (ostream& str, const AlphaPlotGenerator::Results& r);

} // namespace NS_Analysis

#endif // defined ALPHAPLOTGENERATOR_H
