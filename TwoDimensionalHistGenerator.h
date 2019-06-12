//-*-mode:c++; mode:font-lock;-*-

#ifndef TWODIMENSIONALHISTGENERATOR_H
#define TWODIMENSIONALHISTGENERATOR_H

#define __STL_USE_NAMESPACES

#include"RedHeader.h"
#include"EventInfo.h"
#include"ParamFile.h"
#include"TwoDimensionalParameterization.h"
#include"ProgressBar.h"
#include"Binner.h"

namespace NS_Analysis {

  class TwoDimensionalHistGenerator: private ParamEventOperator
  {
  public:
    TwoDimensionalHistGenerator(double binsize,
				const TwoDimensionalParameterization* par,
				ProgressBar* pb)
      : m_parameterize(par), m_total(binsize), m_file(binsize), m_pb(pb) {}
    
    void generate(ParamFile* pf, double ra, double dec, double mjd);

    class Results
    {
    public:
      Results(double binsize): m_2d(binsize,Binner::BIN_CENTER), 
			       m_radial(binsize,Binner::BIN_START) {}

      const Summer& radial() const { return m_radial; }
      const TwoDimBinner<Summer>& twoDim() const { return m_2d; }         

      Summer& radial() { return m_radial; }
      TwoDimBinner<Summer>& twoDim() { return m_2d; }         

      void clear();
      void insert(const Results& r);

    private:
      TwoDimBinner<Summer>                m_2d;
      Summer                              m_radial;
    };

    const Results& totalResults() const { return m_total; }
    const Results& fileResults() const { return m_file; }
    
  private:
    virtual void operateOnEvent(int evno, const EventInfo* ei, 
				const HillasParam *hp);
    
    double          m_ra;
    double          m_dec;
    double          m_mjd_start;
    
    const TwoDimensionalParameterization* m_parameterize;

    Results         m_total;
    Results         m_file;

    ProgressBar*    m_pb;
  };

}

#endif // TWODIMENSIONALHISTGENERATOR_H
