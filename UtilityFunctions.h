//-*-mode:c++; mode:font-lock;-*-

#ifndef UTILITYFUNCTIONS
#define UTILITYFUNCTIONS

#include<string>

namespace NS_Analysis
{
  using std::string;

  //  string getFilenameTrailer(const string& filename);
  string getFilenameRoot(const string& filename);
  string getFilenamePath(const string& filename);
  string getFilenameName(const string& filename);
  string getFilenameExt(const string& filename);
  string getFilenameBase(const string& filename);
  void fixDate2000(int& date);

  double rate(double nOn, double nOff, 
	      double exposureOn, double exposureOff,
	      double offOnRatio=1.0);

  double significance(double nOn, double nOff, 
		      double exposureOn, double exposureOff,
		      double offOnRatio=1.0);
} // namespace NS_Analysis

#endif // defined UTILITYFUNCTIONS
