//-*-mode:c++; mode:font-lock;-*-

#ifndef F77REDFILE_H
#define F77REDFILE_H

#include <RedEvent.h>
#include <RedHeader.h>

using std::string;

using NS_Analysis::RedHeader;
using NS_Analysis::RedEvent;

class F77RedFile
{
public:
  F77RedFile(const string& filename, int nchannels);
  ~F77RedFile();
  
  void getHeader(RedHeader* rh);
  void getNextEvent(RedEvent* re);
  
private:
  int size_f77(char *fmt);
  int read_f77(int fileno, char *fmt, ...);
  int write_f77(int fileno, char *fmt, ...);

  typedef enum
  { STRING='a', CHAR='c', DOUBLE='d', FLOAT='f', INT='i', SHORT='s' }
  FORMAT;

  typedef struct
  {
    char runid[4];
    int nevents;
    double live_time;
    int stdur;
    char mode[3];
    char source[20];
    int date;
    double mjd;
    double frjd;
    float ra;
    float dec;
    int ut;
    int st;
    float azimuth;
    float elevation;
    char skyq[6];
    char comms[404];
    int gpsbeg[7];
  } cRedHeader;

  typedef struct
  {
    int code;
    double time;
    double gpsutc;
    double livetime;
    short adc[492];
  } cRedEvent;

  int m_fileno;
  char format[40];
  RedHeader m_header;
};

#endif // defined F77REDFILE_H
