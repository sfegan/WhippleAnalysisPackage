#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "f77redfile.h"

using namespace std;

F77RedFile::F77RedFile(const string& filename, int nchannels)
{
  m_fileno=open(filename.c_str(),O_RDONLY);
  if(m_fileno<0)
    {
      cerr << "Could not open filename: " << filename << endl;
      perror(filename.c_str());
      exit(EXIT_FAILURE);
    }

  cRedHeader crh;

  read_f77(m_fileno,"a4idia3a20iddffiiffa6a404i7", crh.runid,
	   &crh.nevents, &crh.live_time, &crh.stdur, crh.mode, crh.source,
	   &crh.date, &crh.mjd, &crh.frjd, &crh.ra, &crh.dec, &crh.ut,
	   &crh.st, &crh.azimuth, &crh.elevation, crh.skyq, crh.comms, 
	   crh.gpsbeg);

  // Member: RunID
  int s;
  for(s=0;s<sizeof(crh.runid);s++)if(crh.runid[s]==0)break;
  m_header.setRunID(string(crh.runid,s));
  // Member: NEvents
  m_header.setNEvents(crh.nevents);
  // Member: LiveTime
  m_header.setLiveTime(crh.live_time);
  // Member: STDur
  m_header.setSTDur(crh.stdur);
  // Member: Mode
  m_header.setMode(crh.mode);
  // Member: Source
  m_header.setSource(crh.source);
  // Member: Date
  m_header.setDate(crh.date);
  // Member: MJD
  m_header.setMJD(crh.mjd);
  // Member: FRJD
  m_header.setFRJD(crh.frjd);
  // Member: RA
  m_header.setRA(crh.ra);
  // Member: Dec
  m_header.setDec(crh.dec);
  // Member: UT
  m_header.setUT(crh.ut);
  // Member: ST
  m_header.setST(crh.st);
  // Member: Azimuth
  m_header.setAzimuth(crh.azimuth);
  // Member: Elevation
  m_header.setElevation(crh.elevation);
  // Member: SkyQ
  for(s=0;s<sizeof(crh.skyq);s++)if(crh.skyq[s]==0)break;
  m_header.setSkyQ(string(crh.skyq,s));
  // Member: Comms
  for(s=0;s<sizeof(crh.comms);s++)if(crh.comms[s]==0)break;
  m_header.setComms(string(crh.comms,s));
  // Member: GPSBeg
  m_header.setGPSBeg(crh.gpsbeg);

  sprintf(format,"iddds%d",nchannels);
}

F77RedFile::~F77RedFile()
{
  close(m_fileno);
}

void
F77RedFile::getHeader(RedHeader* rh)
{
  *rh = m_header;
}

void
F77RedFile::getNextEvent(RedEvent* re)
{
  cRedEvent cre;

  read_f77(m_fileno,format,
	   &cre.code,
	   &cre.time,
	   &cre.gpsutc,
	   &cre.livetime,
	   cre.adc);
  
  // Member: Code
  re->setCode(cre.code);
  // Member: Time
  re->setTime(cre.time);
  // Member: GPSUTC
  re->setGPSUTC(cre.gpsutc);
  // Member: LiveTime
  re->setLiveTime(cre.livetime);
  // Member: ADC
  re->setADC(cre.adc);
}

/******************************************************************************

   Subroutine: read_f77 

   Read the "unformatted" dec fortran 77 record (reduced file produced by
   fz2red).

   Input: fileno - file number of open file
          fmt    - string representing format of record to read, for example
	           "ida10s100" represents an int, double, string*10 and
		   short*100
	  ...    - pointers to variables to return the data as, in the above
	           example we would need int*, double*, char[10], short[100]

   Output: no of bytes read or exit(EXIT_FAILURE) on failure

******************************************************************************/

int F77RedFile::size_f77(char *fmt)
{
  char *fmt_ptr=NULL;
  int nbytes_expected=0;

  fmt_ptr=fmt;
  while(*fmt_ptr)
    {
      char type=*fmt_ptr++;
      int count=0;
      
      if(isdigit(*fmt_ptr))
	{
	  count=strtol(fmt_ptr,&fmt_ptr,10);
	}
      else
	{
	  count=1;
	}

      switch((FORMAT)type)
	{
	case STRING:
	case CHAR:
	  nbytes_expected+=count*sizeof(char);
	  break;
	  
	case DOUBLE:
	  nbytes_expected+=count*sizeof(double);
	  break;
	  
	case FLOAT:
	  nbytes_expected+=count*sizeof(float);
	  break;
	  
	case INT:
	  nbytes_expected+=count*sizeof(int);
	  break;
	  
	case SHORT:
	  nbytes_expected+=count*sizeof(short);
	  break;
	}
    }

  return nbytes_expected;
}

int F77RedFile::read_f77(int fileno, char *fmt, ...)
{
  va_list ap;
  int nbytes_expected=0;
  int nbytes_found=0;
  char *buffer;
  char *buf_ptr;
  char *fmt_ptr=NULL;

  nbytes_expected=size_f77(fmt);
  
  if(read(fileno,&nbytes_found,sizeof(nbytes_found)) != sizeof(nbytes_found))
    {
      fprintf(stderr,"HEADER: Premature end of file or read error\n");
      cerr << "Nbytes: " << nbytes_found << endl;
      perror("ERROR");
      exit(EXIT_FAILURE);
    }
  
  if(nbytes_found != nbytes_expected)
    {
      fprintf(stderr,
  "No bytes found in FORTRAN record (%d) doesn't match no expected (%d)\n",
	      nbytes_found,nbytes_expected);
      exit(EXIT_FAILURE);
    }
	      
  buffer=new char[nbytes_expected];
	      
  if(read(fileno,buffer,nbytes_expected) != nbytes_expected)
    {
      fprintf(stderr,"DATA: Premature end of file or read error\n");
      perror("ERROR");
      exit(EXIT_FAILURE);
    }
  
  va_start(ap,fmt);
  
  fmt_ptr=fmt;
  buf_ptr=buffer;

  while(*fmt_ptr)
    {
      char type=*fmt_ptr++;

      int count=0;
      int nbytes_atom=0;
      void *arg_ptr;
      
      if(isdigit(*fmt_ptr))
	{
	  count=strtol(fmt_ptr,&fmt_ptr,10);
	}
      else
	{
	  count=1;
	}

      switch((FORMAT)type)
	{
	case STRING:
	case CHAR:
	  nbytes_atom=sizeof(char);
	  break;
	  
	case DOUBLE:
	  nbytes_atom=sizeof(double);
	  break;
	  
	case FLOAT:
	  nbytes_atom=sizeof(float);
	  break;
	  
	case INT:
	  nbytes_atom=sizeof(int);
	  break;
	  
	case SHORT:
	  nbytes_atom=sizeof(short);
	  break;
	}

      if((arg_ptr=va_arg(ap,void *)) == NULL)
	{
	  fprintf(stderr,"Insufficient no of arguments to read_f77: %s\n",
		  fmt_ptr);
	  exit(EXIT_FAILURE);
	}

      memcpy(arg_ptr,buf_ptr,count*nbytes_atom);
      buf_ptr+=count*nbytes_atom;
    }

  va_end(ap);
  delete[] buffer;

  if(read(fileno,&nbytes_found,sizeof(nbytes_found)) != sizeof(nbytes_found))
    {
      fprintf(stderr,"TRAILER: Premature end of file or read error\n");
      perror("ERROR");
      exit(EXIT_FAILURE);
    }
  
  if(nbytes_found != nbytes_expected)
    {
      fprintf(stderr,
  "No bytes found in FORTRAN trailer (%d) doesn't match no expected (%d)\n",
	      nbytes_found,nbytes_expected);
      exit(EXIT_FAILURE);
    }

  return nbytes_found;
}

/******************************************************************************

   Subroutine: write_f77 

   Write the "unformatted" dec fortran 77 record (reduced file produced by
   fz2red).

   Input: fileno - file number of open file
          fmt    - string representing format of record to read, for example
	           "ida10s100" represents an int, double, string*10 and
		   short*100
	  ...    - pointers to variables to return the data as, in the above
	           example we would need int*, double*, char[10], short[100]

   Output: no of bytes read or exit(EXIT_FAILURE) on failure

******************************************************************************/

int F77RedFile::write_f77(int fileno, char *fmt, ...)
{
  va_list ap;
  int nbytes_expected=0;
  int nbytes_towrite=0;
  char *buffer;
  char *buf_ptr;
  char *fmt_ptr=NULL;

  nbytes_expected = size_f77(fmt);

  nbytes_towrite = nbytes_expected;
  if(write(fileno,&nbytes_towrite,sizeof(nbytes_towrite)) != 
     sizeof(nbytes_towrite))
    {
      fprintf(stderr,"HEADER: Premature end of file or write error\n");
      perror("ERROR");
      exit(EXIT_FAILURE);
    }
	      
  buffer=new char[nbytes_expected];
	      
  va_start(ap,fmt);
  
  fmt_ptr=fmt;
  buf_ptr=buffer;

  while(*fmt_ptr)
    {
      char type=*fmt_ptr++;

      int count=0;
      int nbytes_atom=0;
      void *arg_ptr;
      
      if(isdigit(*fmt_ptr))
	{
	  count=strtol(fmt_ptr,&fmt_ptr,10);
	}
      else
	{
	  count=1;
	}

      switch((FORMAT)type)
	{
	case STRING:
	case CHAR:
	  nbytes_atom=sizeof(char);
	  break;
	  
	case DOUBLE:
	  nbytes_atom=sizeof(double);
	  break;
	  
	case FLOAT:
	  nbytes_atom=sizeof(float);
	  break;
	  
	case INT:
	  nbytes_atom=sizeof(int);
	  break;
	  
	case SHORT:
	  nbytes_atom=sizeof(short);
	  break;
	}

      if((arg_ptr=va_arg(ap,void *)) == NULL)
	{
	  fprintf(stderr,"Insufficient no of arguments to write_f77\n");
	  exit(EXIT_FAILURE);
	}

      memcpy(buf_ptr,arg_ptr,count*nbytes_atom);
      buf_ptr+=count*nbytes_atom;
    }

  va_end(ap);

  if(write(fileno,buffer,nbytes_expected) != nbytes_expected)
    {
      fprintf(stderr,"DATA: Premature end of file or write error\n");
      perror("ERROR");
      exit(EXIT_FAILURE);
    }
  
  delete[] buffer;

  nbytes_towrite = nbytes_expected;
  if(write(fileno,&nbytes_towrite,sizeof(nbytes_towrite)) != 
     sizeof(nbytes_towrite))
    {
      fprintf(stderr,"TRAILER: Premature end of file or write error\n");
      perror("ERROR");
      exit(EXIT_FAILURE);
    }
  
  return nbytes_towrite;
}
