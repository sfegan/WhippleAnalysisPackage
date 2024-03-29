#include<cmath>
#include<string>

#include"UtilityFunctions.h"

using std::string;

/*
string 
NS_Analysis::
getFilenameTrailer(const string& filename)
{
  int lastslash  = filename.rfind('/');
  if(lastslash == -1)return filename;
  else return filename.substr(lastslash+1);
}
*/

string 
NS_Analysis::
getFilenameRoot(const string& filename)
{
  int lastslash  = filename.rfind('/');
  int lastperiod = filename.rfind('.');
  if((lastslash!=-1)&&(lastperiod<=lastslash+1))return filename;
  else return filename.substr(0,lastperiod);
}

string 
NS_Analysis::
getFilenamePath(const string& filename)
{
  int lastslash  = filename.rfind('/');
  if(lastslash==-1)return "";
  else return filename.substr(0,lastslash+1);
}

string 
NS_Analysis::
getFilenameName(const string& filename)
{
  int lastslash  = filename.rfind('/');
  if(lastslash==-1)return filename;
  else return filename.substr(lastslash+1);
}


string 
NS_Analysis::
getFilenameExt(const string& filename)
{
  int lastslash  = filename.rfind('/');
  int lastperiod = filename.rfind('.');
  
  if((lastperiod<=0) || ((lastslash!=-1)&&(lastperiod<lastslash)))return "";
  else return filename.substr(lastperiod+1);
}

string 
NS_Analysis::
getFilenameBase(const string& filename)
{
  return getFilenameRoot(getFilenameName(filename));
}

void 
NS_Analysis::
fixDate2000(int& date)
{
  if(date < 800000)       date += 20000000;   // eg 000101
  else if(date < 4000000) date += 19000000;   // eg 990101 or 1000101
}

static double derfc(double x)
{
  int k;
  double w, t, y;
  static double a[65] = {
    5.958930743e-11, -1.13739022964e-9, 
    1.466005199839e-8, -1.635035446196e-7, 
    1.6461004480962e-6, -1.492559551950604e-5, 
    1.2055331122299265e-4, -8.548326981129666e-4, 
    0.00522397762482322257, -0.0268661706450773342, 
    0.11283791670954881569, -0.37612638903183748117, 
    1.12837916709551257377, 
    2.372510631e-11, -4.5493253732e-10, 
    5.90362766598e-9, -6.642090827576e-8, 
    6.7595634268133e-7, -6.21188515924e-6, 
    5.10388300970969e-5, -3.7015410692956173e-4, 
    0.00233307631218880978, -0.0125498847718219221, 
    0.05657061146827041994, -0.2137966477645600658, 
    0.84270079294971486929, 
    9.49905026e-12, -1.8310229805e-10, 
    2.39463074e-9, -2.721444369609e-8, 
    2.8045522331686e-7, -2.61830022482897e-6, 
    2.195455056768781e-5, -1.6358986921372656e-4, 
    0.00107052153564110318, -0.00608284718113590151, 
    0.02986978465246258244, -0.13055593046562267625, 
    0.67493323603965504676, 
    3.82722073e-12, -7.421598602e-11, 
    9.793057408e-10, -1.126008898854e-8, 
    1.1775134830784e-7, -1.1199275838265e-6, 
    9.62023443095201e-6, -7.404402135070773e-5, 
    5.0689993654144881e-4, -0.00307553051439272889, 
    0.01668977892553165586, -0.08548534594781312114, 
    0.56909076642393639985, 
    1.55296588e-12, -3.032205868e-11, 
    4.0424830707e-10, -4.71135111493e-9, 
    5.011915876293e-8, -4.8722516178974e-7, 
    4.30683284629395e-6, -3.445026145385764e-5, 
    2.4879276133931664e-4, -0.00162940941748079288, 
    0.00988786373932350462, -0.05962426839442303805, 
    0.49766113250947636708
  };
  static double b[65] = {
    -2.9734388465e-10, 2.69776334046e-9, 
    -6.40788827665e-9, -1.6678201321e-8, 
    -2.1854388148686e-7, 2.66246030457984e-6, 
    1.612722157047886e-5, -2.5616361025506629e-4, 
    1.5380842432375365e-4, 0.00815533022524927908, 
    -0.01402283663896319337, -0.19746892495383021487, 
    0.71511720328842845913, 
    -1.951073787e-11, -3.2302692214e-10, 
    5.22461866919e-9, 3.42940918551e-9, 
    -3.5772874310272e-7, 1.9999935792654e-7, 
    2.687044575042908e-5, -1.1843240273775776e-4, 
    -8.0991728956032271e-4, 0.00661062970502241174, 
    0.00909530922354827295, -0.2016007277849101314, 
    0.51169696718727644908, 
    3.147682272e-11, -4.8465972408e-10, 
    6.3675740242e-10, 3.377623323271e-8, 
    -1.5451139637086e-7, -2.03340624738438e-6, 
    1.947204525295057e-5, 2.854147231653228e-5, 
    -0.00101565063152200272, 0.00271187003520095655, 
    0.02328095035422810727, -0.16725021123116877197, 
    0.32490054966649436974, 
    2.31936337e-11, -6.303206648e-11, 
    -2.64888267434e-9, 2.050708040581e-8, 
    1.1371857327578e-7, -2.11211337219663e-6, 
    3.68797328322935e-6, 9.823686253424796e-5, 
    -6.5860243990455368e-4, -7.5285814895230877e-4, 
    0.02585434424202960464, -0.11637092784486193258, 
    0.18267336775296612024, 
    -3.67789363e-12, 2.0876046746e-10, 
    -1.93319027226e-9, -4.35953392472e-9, 
    1.8006992266137e-7, -7.8441223763969e-7, 
    -6.75407647949153e-6, 8.428418334440096e-5, 
    -1.7604388937031815e-4, -0.0023972961143507161, 
    0.0206412902387602297, -0.06905562880005864105, 
    0.09084526782065478489
  };
  
  w = x < 0 ? -x : x;
  if (w < 2.2) {
    t = w * w;
    k = (int) t;
    t -= k;
    k *= 13;
    y = ((((((((((((a[k] * t + a[k + 1]) * t + 
		   a[k + 2]) * t + a[k + 3]) * t + a[k + 4]) * t + 
		a[k + 5]) * t + a[k + 6]) * t + a[k + 7]) * t + 
	     a[k + 8]) * t + a[k + 9]) * t + a[k + 10]) * t + 
	  a[k + 11]) * t + a[k + 12]) * w;
  } else if (w < 6.9) {
    k = (int) w;
    t = w - k;
    k = 13 * (k - 2);
    y = (((((((((((b[k] * t + b[k + 1]) * t + 
		  b[k + 2]) * t + b[k + 3]) * t + b[k + 4]) * t + 
	       b[k + 5]) * t + b[k + 6]) * t + b[k + 7]) * t + 
            b[k + 8]) * t + b[k + 9]) * t + b[k + 10]) * t + 
	 b[k + 11]) * t + b[k + 12];
    y *= y;
    y *= y;
    y *= y;
    y = 1 - y * y;
  } else {
    y = 1;
  }
  return x < 0 ? -y : y;
}

static double dierfc(double y) // not my code!
{
  double s, t, u, w, x, z;
  
  z = y;
  if (y > 1) {
    z = 2 - y;
  }
  w = 0.916461398268964 - log(z);
  u = sqrt(w);
  s = (log(u) + 0.488826640273108) / w;
  t = 1 / (u + 0.231729200323405);
  x = u * (1 - s * (s * 0.124610454613712 + 0.5)) - 
    ((((-0.0728846765585675 * t + 0.269999308670029) * t + 
       0.150689047360223) * t + 0.116065025341614) * t + 
     0.499999303439796) * t;
  t = 3.97886080735226 / (x + 3.97886080735226);
  u = t - 0.5;
  s = (((((((((0.00112648096188977922 * u + 
	       1.05739299623423047e-4) * u - 0.00351287146129100025) * u - 
	     7.71708358954120939e-4) * u + 0.00685649426074558612) * u + 
	   0.00339721910367775861) * u - 0.011274916933250487) * u - 
	 0.0118598117047771104) * u + 0.0142961988697898018) * u + 
       0.0346494207789099922) * u + 0.00220995927012179067;
  s = ((((((((((((s * u - 0.0743424357241784861) * u - 
		 0.105872177941595488) * u + 0.0147297938331485121) * u + 
	       0.316847638520135944) * u + 0.713657635868730364) * u + 
	     1.05375024970847138) * u + 1.21448730779995237) * u + 
	   1.16374581931560831) * u + 0.956464974744799006) * u + 
        0.686265948274097816) * u + 0.434397492331430115) * u + 
       0.244044510593190935) * t - 
    z * exp(x * x - 0.120782237635245222);
  x += s * (x * s + 1);
  if (y > 1) {
    x = -x;
  }
  return x;
}

static double helene_ul(double Nsig, double sigma, double confidence)
{  
  double prob=1-confidence;
  double sigmaroot2=Nsig/sigma*M_SQRT2;
  return dierfc(prob*derfc(-Nsig/sigmaroot2))*sigmaroot2+Nsig;
}

static void liandma(double Non, double Noff, double alpha,
		    double& Nsig, double& Sig5, double& Sig9, double& Sig17)
{
  double alphasq;
  double oneplusalpha;
  double oneplusalphaoveralpha;

  double Ntot;

  alphasq=alpha*alpha;
  oneplusalpha=1.0+alpha;
  oneplusalphaoveralpha=oneplusalpha/alpha;

  Nsig   = Non - alpha*Noff;
  Ntot   = Non + Noff;

  Sig5  = Nsig/sqrt(Non+alphasq*Noff);
  Sig9  = Nsig/sqrt(alpha*Ntot);
  Sig17 = sqrt(2*( Non *log(oneplusalphaoveralpha*(Non/Ntot)) +
		   Noff*log(oneplusalpha*(Noff/Ntot)) ));
  if(Nsig<0)Sig17=-Sig17;

  return;
}

double NS_Analysis::rate(double nOn, double nOff, 
			 double exposureOn, double exposureOff,
			 double offOnRatio)
{
  double onOffExposureRatio=exposureOn/exposureOff/offOnRatio;
  return (nOn - nOff*onOffExposureRatio)/exposureOn*60.0;
}

double NS_Analysis::significance(double nOn, double nOff, 
				 double exposureOn, double exposureOff,
				 double offOnRatio)
{
  double nSig;
  double limaSig5;
  double limaSig9;
  double limaSig17;

  liandma(nOn,nOff,exposureOn/exposureOff/offOnRatio,
	  nSig,limaSig5,limaSig9,limaSig17);

  return limaSig17;
}
