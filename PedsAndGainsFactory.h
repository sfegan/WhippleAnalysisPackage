//-*-mode:c++; mode:font-lock;-*-

#ifndef PEDSANDGAINSFACTORY_H
#define PEDSANDGAINSFACTORY_H

#define __STL_USE_NAMESPACES

#include<iostream>
#include<string>
#include<vector>
#include<algorithm>
#include<cmath>

#include"Types.h"
#include"ChannelData.h"
#include"Pedestals.h"
#include"Gains.h"

namespace NS_Analysis {
  

  class PedsAndGainsFactory
  {
  public:
    virtual ~PedsAndGainsFactory();

    virtual void       deletePeds(const string& runname, int date, 
				  const string& compartment="") = 0;
    virtual Pedestals*   loadPeds(const string& runname, int date,
				  const string& compartment="") = 0;
    virtual void         savePeds(const Pedestals* peds,
				  const string& runname, int date,
				  const string& compartment="") = 0;

    virtual void      deleteGains(const string& runname, int date, 
				  const string& compartment="") = 0;
    virtual Gains*      loadGains(const string& runname, int date,
				  const string& compartment="") = 0;
    virtual void        saveGains(const Gains* gains,
				  const string& runname, int date,
				  const string& compartment="") = 0;

    virtual vector<string> listGainsByDate(int date, 
					   const string& compartment="") = 0;
  };   

} // namespace NS_Analysis

#endif // PEDSANDGAINSFACTORY_H
