#include<cmath>

#include"Random.h"

using std::endl;

const int NS_Analysis::NRRand2::NTAB;
const int NS_Analysis::NRRand2::A1;
const int NS_Analysis::NRRand2::A2;
const int NS_Analysis::NRRand2::Q1;
const int NS_Analysis::NRRand2::Q2;
const int NS_Analysis::NRRand2::R1;
const int NS_Analysis::NRRand2::R2;
const int NS_Analysis::NRRand2::M1;
const int NS_Analysis::NRRand2::M2;

#ifndef _RANDOM_H_USE_LIMITS
const double NS_Analysis::NRRand2::RNMX;
#endif


NS_Analysis::RandomNumberGenerator::
~RandomNumberGenerator()
{
}

NS_Analysis::LinearRNG::
~LinearRNG()
{
}

NS_Analysis::NRRand2::
NRRand2(int idum): LinearRNG(), m_seeded(true)
{
  if(idum <0) idum=-idum;
  if(idum==0) idum=1;

  m_idum2=idum;
  for(int j=NTAB+7;j>=0;j--) {
    int k=idum/Q1;
    idum=A1*(idum-k*Q1)-R1*k; 
    if(idum<0)idum+=M1;
    if(j<NTAB)m_iv[j]=idum;
  }
  m_iy=m_iv[0];
  m_idum1=idum;
}

double
NS_Analysis::NRRand2::
rand()
  // Long period (>2e+18) random number generator of L'Ecuyer with  
  // Bays-Durham shuffle. Returns a uniform random deviate between 0.0 and 
  // 1.0 (exclusive of the endpoint values). 
{
  if(!m_seeded)
    {
      Error err("NRRand2::rand");
      err.stream() << "Random number generator has not been seeded!" << endl;
    }
  
  int    j;
  int    k;
  double am=(1.0/M1);
  int    imm1=(M1-1);
  int    ndiv=(1+(M1-1)/NTAB);
  double tmp;
  
  k=m_idum1/Q1;
  m_idum1=A1*(m_idum1-k*Q1)-R1*k; 
  if (m_idum1<0) m_idum1+=M1;
  k=m_idum2/Q2;
  m_idum2=A2*(m_idum2-k*Q2)-R2*k;
  if (m_idum2<0) m_idum2+=M2;
  j=m_iy/ndiv;
  m_iy=m_iv[j]-m_idum2;
  m_iv[j]=m_idum1;
  if(m_iy<1) m_iy+=imm1;
  tmp=(double)(am*m_iy);
#ifdef _RANDOM_H_USE_LIMITS
  if(tmp>RNMX()) return (double)RNMX();
#else  
  if(tmp>RNMX) return (double)RNMX;
#endif
  else return tmp;
}

///////////////////////////////////////////////////////////////////////////////

double 
NS_Analysis::BMLinearRNGToGaussianRNGAdaptor::
rand()
{
  if(m_iset)
    {
      m_iset=false;
      return m_gset;
    }
  else
    {
      double fac,rsq,v1,v2;
      do
	{
	  v1=2.0 * m_linrng->rand() - 1.0;
	  v2=2.0 * m_linrng->rand() - 1.0;
	  rsq=v1*v1+v2*v2;
	} while((rsq >= 1.0)||(rsq==0.0));
      fac=sqrt(-2.0*log(rsq)/rsq);
      m_gset=v1*fac;
      m_iset=true;
      return v2*fac;
    }
}
