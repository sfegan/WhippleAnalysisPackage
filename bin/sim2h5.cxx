#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>

#define _GNU_SOURCE
#include <getopt.h>

#include <iostream>
#include <fstream>

#include <Exceptions.h>
#include <RedFile.h>
#include <RedEvent.h>
#include <RedHeader.h>
#include <ProgressBar.h>
#include <VSFA.h>

using namespace NS_Analysis;

#define TUBE_TYPE 0

extern "C" {
#include "simtohdf5/hdf5.h"
#include "annotate.h"
#include "RandomNumbers.h"
}

#include "WhippleCams.h" // trigger SIM

int triggerPST(int nchannels, const int neighbors[][7],
               const float signal[], float level, int multiplicity)
{
  if(multiplicity==1)
    {
      int i;
      for(i=0;i<nchannels;i++)if(signal[i]>=level)return 1;
      return 0;
    }
  else if(multiplicity==2)
    {
      int i;
      for(i=0;i<nchannels;i++)if(signal[i]>=level)
        {
          int j;
          for(j=0;j<7;j++)
            {
              int n=neighbors[i][j];
              if((n!=-1)&&(n<i)&&(signal[n]>=level))return 1;
	    }
        }
      return 0;
    }
  else if(multiplicity==3)
    {
      int i;
      for(i=0;i<nchannels;i++)if(signal[i]>=level)
        {
          int N=0;
          int j;
          for(j=0;j<7;j++)
            {
              int n=neighbors[i][j];
              if((n!=-1)&&(n<nchannels)&&(signal[n]>=level))if(++N==2)return 1;
            }
        }
      return 0;
    }
  else
    {
      fprintf(stderr,"triggerPST is not yet implemented with mult=%d",
              multiplicity);
      abort();
    }
}

const double DC_PER_PE   = 3.00;
const double DEFAULT_NSB = 1.8;;

/* pmt data structure         */
struct pmt {
  int           n;  /* pmt number                                  [1]  */ 
  float         x;  /* pmt x coordinate                          [deg]  */
  float         y;  /* pmt y coordinate                          [deg]  */
  float         r;  /* pmt photocathode radius                   [deg]  */
  float         q;  /* photoelectron content                      [pe]  */
};

#define N 1141         /* number of pixels in simulated Whipple camera */
#define M  490         /* number of pixels in real      Whipple camera */

#define PI M_PI

void map( pmt [], pmt []);
void synopsis(FILE *);

int main (int argc, char *argv[])
{
  pmt         pt[N],ptr[M];
  
  event       evnt;             /* create event structure  */
  telescope   tlscp;            /* create telescope structure  */
  pixel       pxl;              /* create pixel structure  */
  
  int         offset;           /* event identifier */
  
  char        *name;            /* pointer to detector configuration 
				 * file name */
  
  string      fname;            /* buffer for filename */
  
  annotation  annttn;           /* create annotation structure  */
  statistics  sttstcs;          /* create statistics structure */
  int         total, size[4];   /* SIM_size variables */
  
  float       nsb=DEFAULT_NSB;  /* NSB per pixel */
  
  float       rnd,rmd;          /* reserved for random numbers */

  int eventcount=0;
  int blockstart=0;
  int blockcount=-1;

  bool ofappend=false;
  string ofname;

  bool trigger = false;
  float trigger_level = 20.8;

  int tube_type = TUBE_TYPE;

  ifstream ifp;

  /* INITIALIZATIONS */
  
  ran2_read();
  rnd=ran2();
  ran2_write();
    
  /* Fill in pmt data structure for simulated Whipple camera */
  ifp.open("whipple.pmts");
  for(int j=0; !ifp.eof() && j<N; j++) {
    ifp >> pt[j].n >> pt[j].x >> pt[j].y >> pt[j].r;
    pt[j].q=0.;
  }
  ifp.close();

  /* Fill in pmt data structure for real      Whipple camera */
  ifp.open("whipple_real.pmts");
  for(int j=0; !ifp.eof() && j<N; j++) {
    ifp >> ptr[j].n >> ptr[j].x >> ptr[j].y >> ptr[j].r;
    ptr[j].q=0.;
  }
  ifp.close();

/* DECODE COMMAND LINE */

  while(1)
    {
      int c;
      int this_option_optind = optind ? optind : 1;
      int option_index = 0;
      static struct option long_options[] =
      {
	{"nsb", 1, 0, 'n'},
	{"append", 0, 0, 'a'},
	{"output", 1, 0, 'o'},
	{"count", 1, 0, 'c'},
	{"trigger", 1, 0, 't'},
	{"pmt", 1, 0, 'p'},
	{"block", 1, 0, 'b'},
	{0, 0, 0, 0}
      };
      
      c=getopt_long(argc, argv, "n:ao:c:b:t:p:",
		    long_options, &option_index);
      if (c == -1)
	break;
      
      switch (c)
	{
	case 'n':
	  sscanf(optarg,"%f",&nsb);
	  break;

	case 'a':
	  ofappend=true;
	  break;

	case 'o':
	  ofname=optarg;
	  break;
	  
	case 'c':
	  sscanf(optarg,"%d",&blockcount);
	  break;
	  
	case 'b':
	  sscanf(optarg,"%d",&blockstart);
	  break;

	case 'p':
	  sscanf(optarg,"%d",&tube_type);
	  break;

	case 't':
	  trigger = true;
	  sscanf(optarg,"%f",&trigger_level);
	  break;
	  
	default:
	  cerr << "unknown option " << optopt << ".. sorry!\n";
	  exit(EXIT_FAILURE);
	}
    }

  argc-=optind;
  argv+=optind;
  
  /* Identify first argument: filename */
  if(argc == 0) {
    synopsis(stdout);
    exit(EXIT_FAILURE);
  }

  fname=*argv;
  if(ofname == "")ofname=fname+".h5";

  cout << "\nNSB:  " << nsb << " [pe per small pixel]"
       << "\nDC/PE:" << DC_PER_PE << "\n\n";
  
  /* PROCESS SIM FILE */
  
  cout << "\nFILE: " << fname << "\n\n";

  if(SIM_open(fname.c_str()) == -1)
    {
      cerr << "File " << fname << " does not exist\n";
      exit(EXIT_FAILURE);
    }

  if(int result=SIM_annotation_read(fname.c_str(), &annttn) < 0)
    {
      if(result==-2)
	cerr << "File " << fname << "has no annotation attribute\n";
      else cerr << "File " << fname << " does not exist\n";
      exit(EXIT_FAILURE);
    }

  SIM_annotation_print(stdout, &annttn);

  /* read simulation statistics */
  if(int result=SIM_statistics_read(fname.c_str(), &sttstcs) < 0)
    {
      if(result==-2)
	cerr << "File " << fname << " has no statistics attribute\n";
      else cerr << "File " << fname << " does not exist\n";
      exit(EXIT_FAILURE);
    }
  
  SIM_statistics_print(stdout, &sttstcs);

  /* read sizes of the datasets */
  total=SIM_size(fname.c_str(), size);
  if(total==-1)
    {
      cerr << "File " << fname << " does not exist\n";
      exit(EXIT_FAILURE);
    }

  cout << "\nDATASETS:\n"
       << " PEs        records ( 2 bytes)  " << size[0] << '\n'
       << " PIXELs     records ( 6 bytes)  " << size[1] << '\n'
       << " TELESCOPEs records (16 bytes)  " << size[2] << '\n'
       << " EVENTs     records (34 bytes)  " << size[3] << '\n'
       << " total      bytes               " << total  << '\n';

  /* read detector configuration file name (DCF) 
   * This name provides a reference to file with the detector data
   * which were used during simulations and which can be used 
   * (optional) to load all detector data necessary for a 
   * particular analysis. In this example detector configuration 
   * file name is not used.
   */
  name = (char *)calloc(100, sizeof(char));
  SIM_read_config(name);
  printf("\nDCF NAME: %s\n\n",name);
  free(name);

/* OPEN REDUCED OUTPUT FILE */

  RedFile redfile;

  try
    {
      if(!ofappend)redfile.create(ofname,492);
      else redfile.openorcreate(ofname,492);
    }
  catch(Error x)
    {
      cerr << x;
      exit(EXIT_FAILURE);
    }

  float pedestal[M];

    {
      ofstream out("peds.dat");

      const int NPEDS=2000;

      if(!ofappend)
	cerr << "Writing " << NPEDS << " peds\n";
      else
	cerr << "Calculating pedestals" << endl;

      TextProgressBar pb;
      pb.reset(NPEDS);

      for(int i=0;i<NPEDS;i++)
	{
	  for(int j=0;j<M;j++)
	    {
	      if(ptr[j].r < 0.05)ptr[j].q=(float)poidev(nsb);
	      else ptr[j].q=(float)poidev(4.507365605*nsb);
	    }
	  
	  for(int j=0;j<M;j++) {
	    rmd=0.;
	    for(int k=0;k<(int)(ptr[j].q);k++) rmd+=pmt_pe(tube_type);
	    ptr[j].q=rmd;
	  }
	    
	  RedEvent re;
	  re.setVersion(492);
	  re.setCode(1);
	  re.setTime(double(i)/double(NPEDS));
	  re.setGPSUTC(re.time());
	  re.setLiveTime(re.time());
	  for(int j=0;j<M;j++)
	    {
	      float signal = ptr[j].q*DC_PER_PE;
	      re.setADC(short(signal+0.5),j);
	      pedestal[j] += signal;
	      if(j==0)
		out << ptr[j].q << '\t' << signal << endl;;
	    }

	  if(!ofappend)
	    redfile.events()->write(redfile.events()->size(),&re,1);
	  
	  pb.tick(i);
	}

      for(int j=0;j<M;j++)pedestal[j] /= NPEDS;
    }

/* LOOP THROUGH EVENTS IN FILE */

  /* Sequentially read SIM file */
  offset=0;

  /* Variable offset scans through events in the SIM file.
   * When SIM_event() is called it returns event record and sets
   * internal counter for SIM_telescope(). The value of this counter,
   * returned by SIM_telescope() function, is decremented with every
   * call until it reaches zero.  Each call returns record of
   * telescope which was hit in a given event and sets counter for
   * SIM_pixel().  Every call to SIM_pixel() returns pixel record and
   * sets offset for reading pixel data by SIM_pe.  Just to remind,
   * evnt.d is the number of hit telescopes in a given event,
   * therefore evnt.d+1 call to SIM_telescope() will return zero. By
   * analogy, tlscp.d is the number of hit pixels in a given
   * telescope, and tlscp.d+1 call to SIM_pixel() will return
   * zero. The sequence of these calls allows one to avoid explicit
   * tracing of offset variables correspondent to every dataset
   * (EVENTs, TELESCOPEs, PIXELs, PEs) in HDF5 sim file. Once event is
   * chosen by offset in the EVENTs dataset everything else is set
   * automatically.  */
  
  if(blockcount!=-1)
    {
      int eventstoskip=blockcount*blockstart;
      cerr << "Skipping " << eventstoskip << " events..\n";
      TextProgressBar pb;
      pb.reset(eventstoskip);
      while( eventcount<eventstoskip && !SIM_event(&offset, &evnt) ) 
	{    
	  offset++;
	  eventcount++;
	  pb.tick(eventcount);
	}
      eventcount=0;
    }

  if(blockcount==-1)blockcount=size[3];
  if(offset+blockcount > size[3])blockcount=size[3]-offset;
  cerr << "Translating " << blockcount << " events...\n";

  TextProgressBar pb;
  pb.reset(blockcount);

  /* loop through events     */
  while( eventcount<blockcount && !SIM_event(&offset, &evnt) ) 
    {    
      eventcount++;
      pb.tick(eventcount);

      /* SIM_print_event(stdout, &evnt); */
      
      /* loop through telescopes */
      while( SIM_telescope(&tlscp) ) 
	{       
	  /* initialize image */
	  for(int j=0;j<N;j++) pt[j].q=0.;
	    
	  /* loop through pixels     */
	  while ( SIM_pixel(&pxl) ) 
	    {          
	      
/* IMAGE INITIALIZATION */
	      
              /* this is the simplest version of generating 
               * image by counting pes in the pixel independently
               * from their arrival times */
              pt[pxl.n-1].q=(float)pxl.d;
	      
	    } /* pixels     loop end     */
	    
/* IMAGE CONDITIONING */
	    
            /* initialize real image */
            for(int j=0;j<M;j++) ptr[j].q=0.;
	    
            /* map simulated camera onto real camera */
            map(pt, ptr);
	    
            /* Sample signal in outer large pixels and add NSB */
            for(int j=0;j<M;j++) 
	      {
		rmd=ptr[j].q -floorf(ptr[j].q);
		if(rmd > 0.) 
		  {
		    if(rmd>ran2()) 
		      {
			ptr[j].q=floorf(ptr[j].q)+1.;
		      } 
		    else 
		      {
			ptr[j].q=floorf(ptr[j].q);
		      }
		  }
		
		if(ptr[j].r < 0.05) /* small pixel */
		  { 
		    ptr[j].q+=(float)poidev(nsb);
		  } 
		else                /* large pixel */ 
		  {
		    /* Factor 2.5072244107 comes from the ratio of
		     * large pixel photocathode area to an area of
		     * hexagon of cell. Additional factor 1.797751165
		     * is due to light loss in cones because on
		     * average 1.798 photons collected by cell result
		     * in 1 detected in pixel photon. The product of
		     * these factors, 4.507365605, estimates ratio of
		     * the night sky background photons in large and
		     * small pixels.  */
		    ptr[j].q+=(float)poidev(4.507365605*nsb);
		    /*fprintf(stdout,"l %d %f %f\n",j+1,ptr[j].r,ptr[j].q);*/
		  }
	      }
	    
            /* Simulate pmts */
            for(int j=0;j<M;j++) 
	      {
		rmd=0.;
		for(int k=0;k<(int)(ptr[j].q);k++) rmd+=pmt_pe(tube_type);
		ptr[j].q=rmd;
	      }

	    if(trigger)
	      {
		float light[M];
		for(int j=0; j<M; j++)
		  light[j] = ptr[j].q*DC_PER_PE - pedestal[j];

		if(triggerPST(331,WC490Neighbors,light,
			      trigger_level,3)==0)
		  {
		    //		    cerr << "Event failed to pass PST requirement 3>" 
		    //			 << trigger_level << endl;
		    continue;
		  }
	      }
	    
/* IMAGE RECONSTRUCTION */
	    RedEvent re;
	    re.setVersion(492);
	    re.setCode(8);
	    re.setTime(evnt.e);
	    re.setGPSUTC(evnt.n);
	    re.setLiveTime(0);
	    for(int j=0;j<M;j++)re.setADC(short(ptr[j].q*DC_PER_PE),j);
	    redfile.events()->write(redfile.events()->size(),&re,1);
	}  
	  
      /* increment event in the sim file */
      offset++;
    }     /* events     loop end     */
  
/* STICK IN SOME PEDEVENTS */

/* WRITE THE HEADER */

  RedHeader rh;
  rh.zero();
  rh.setVersion(492);
  rh.setRunID("sim");
  rh.setNEvents(redfile.events()->size());
  rh.setDate(20010101);
  redfile.header()->write(0,&rh,1);
  
  SIM_close();
  
  ran2_write();
  
  return 0;
}

/**************************************************************************/
void map( pmt *pt, pmt ptr[])
/*
 * This routine maps simulated Whipple camera (fine resolution, 
 * homogeneous, hexagonal 1141 pixels camera) onto real Whipple
 * camera (non-homogeneous, 490 pixel camera). This routine 
 * is suitable for converting the total number of pes in pixels.
 * For simulation of waveforms of pulses one must process every
 * pe individually, which is possible with this routine but will
 * likely be inefficient and slow way of doing this. Another
 * routine which will do such mapping more efficiently is planned 
 * to be written. Please, contact me for further clarifications
 * on this subject. 
 *
 * March 28, 2000
 *
 * vvv
 */
{
    /* pmt map data structure */
    typedef struct pmtmap {
     int           n;  /* simulated camera pmt number            [1]  */ 
     int        i[3];  /* real camera pmt number                 [1]  */
     float      c[3];  /* real camera pe conversion factor       [1]  */
    } pmtmap;
    static pmtmap  mp[N];
    static int     k=0;

    int j,i,l;

    /* mapping initialization */
    if(k==0) {
      FILE *fp;
      
      fp=fopen("490to1141.map","r");
      j=0;
      while(!feof(fp) && j<N) {
         fscanf(fp,"%d %d %f %d %f %d %f",&mp[j].n,
                  &mp[j].i[0],&mp[j].c[0],
                  &mp[j].i[1],&mp[j].c[1],
                  &mp[j].i[2],&mp[j].c[2]);
         /*
         fprintf(stdout,"%d %d %f %d %f %d %f\n",mp[j].n,
                  mp[j].i[0],mp[j].c[0],
                  mp[j].i[1],mp[j].c[1],
                  mp[j].i[2],mp[j].c[2]);
	 */
         j++; 
      }
      fclose(fp);
      k=1;
    }

    /* mapping */
    j=0;
    for(j=0;j<N;j++) {
      if(pt->q != 0.) {
        i=pt->n-1;
        l=0;
        while(mp[i].i[l] !=0 && l<3){
          ptr[mp[i].i[l]-1].q+=mp[i].c[l]*(pt->q);
          l++;
        }
      }
      pt++;
    }

   return;
}

/**************************************************************************/
void synopsis(FILE *fp)
{
    fprintf(fp,"\n\n");
    fprintf(fp,"SYNOPSIS\n\n");
    fprintf(fp," sim_scan filename [NSB]\n\n");
    fprintf(fp,"DESCRIPTION\n\n");
    fprintf(fp," This is an example program which shows how to scan\n");
    fprintf(fp," simulation file, read and condition shower images, and\n");
    fprintf(fp," finally calculate phase space factors such as collecting\n");
    fprintf(fp," area and solid angle. Mean Night Sky Background [NSB] is\n");
    fprintf(fp," accepted from command line and then added to all images. \n");
    fprintf(fp," If mean NSB per small pixel is not specified, zero pe \n");
    fprintf(fp," per pixel is assumed.\n\n");
    fprintf(fp,"EXAMPLES\n\n");
    fprintf(fp," sim_scan tmp/g010002000.sim 1.00\n");
    fprintf(fp,"\n");
   return;
}
