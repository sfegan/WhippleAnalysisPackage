//-*-mode:c++; mode:font-lock;-*-

#ifndef RANDOM_H
#define RANDOM_H

#define __STL_USE_NAMESPACES

//#define _RANDOM_H_USE_LIMITS

#include <vector>

#include <cmath>
#ifdef _RANDOM_H_USE_LIMITS
#include <limits>
#else 
#include <float.h>
#endif

#include"Types.h"
#include"Exceptions.h"

namespace NS_Analysis {
  
  using std::vector;
  

  class RandomNumberGenerator
  {
  public:
    virtual ~RandomNumberGenerator();
    virtual double rand() = 0;
  };

  class LinearRNG: public RandomNumberGenerator
  {
  public:
    virtual ~LinearRNG();
    virtual double rand() = 0;
  };

  class NRRand2: public LinearRNG
  {
  public:
    NRRand2(): LinearRNG(), m_seeded(false) {}
    NRRand2(int idum);

    virtual double rand();

  private:
    static const int NTAB   = 32;
    static const int A1     = 40014;
    static const int A2     = 40692;
    static const int Q1     = 53668;
    static const int Q2     = 52774;
    static const int R1     = 12211;
    static const int R2     = 3791;
    static const int M1     = 2147483563;
    static const int M2     = 2147483399;

#ifdef _RANDOM_H_USE_LIMITS
    inline static double RNMX() 
    { return (1.0-std::numeric_limits<double>::epsilon()); }
#else 
    static const double  RNMX     = 1.0 - DBL_EPSILON;
#endif

    bool m_seeded;

    int m_idum1;
    int m_idum2;
    int m_iy;
    int m_iv[NTAB];
  };

  /////////////////////////////////////////////////////////////////////////////

  class GaussianRNG: public RandomNumberGenerator
  {
  public:
    virtual double rand() = 0;
  };

  class BMLinearRNGToGaussianRNGAdaptor: public GaussianRNG
  {
  public:
    BMLinearRNGToGaussianRNGAdaptor(MPtr<LinearRNG>& rng):
      m_linrng(rng), m_iset(false), m_gset(0) {}

    virtual double rand();

  private:
    MPtr<LinearRNG> m_linrng;
    bool   m_iset;
    double m_gset;
  };

} // namespace NS_Analysis

#endif // defined(RANDOM_H)
