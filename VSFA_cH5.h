//-*-mode:c++; mode:font-lock;-*-

#ifndef VSFA_CH5_H
#define VSFA_CH5_H

#define __STL_USE_NAMESPACES

#include<string>
#include<hdf5/hdf5.h>

#include "cH5Utils.h"

#include "VSFA.h"


// This parameter sets the number of elements (of class T) that reside
// in one H5 chunk. The more that do the better the H5 caching is but
// the bigger the file is. So my prescription is to set some nominal
// size for the chunk, say 10K and combine that with a minimum number
// of elements to get the chunk dimension. There are probably better
// ways.
#define VSFA_cH5_NOMINAL_SIZE_K 10  // Kilobytes
#define VSFA_cH5_MINIMUM_NUMBER 20  // Elements
#define VSFA_cH5_CHUNKING_SIZE (VSFA_cH5_NOMINAL_SIZE_K*1024/sizeof(T)+VSFA_cH5_MINIMUM_NUMBER)

namespace NS_Analysis
{
  template <class T> class VSFA_PH5; // MPI version
}

namespace NS_Analysis
{
  using std::string;

  /////////////////////////////////////////////////////////////////////////////
  ////////////////////// VSFA_cH5 - HDF5 File Association /////////////////////
  /////////////////////////////////////////////////////////////////////////////
  
  template <class T> 
  class VSFA_cH5: public VSFA<T>
  {
    //    friend class VSFA_PH5<T>;

  private:
    VSFA_cH5();
    VSFA_cH5(VSFA_cH5 &);
    VSFA_cH5& operator=(VSFA_cH5<T> &);
    
  private:
    // Useful things
    void throwerror(const string& func, const string& errm) const 
    { 
      VSFA_Error err(func);
      err.stream() << "VSFA_cH5(" << id() << "): " << errm << '\n';
      throw(err);
    }
    
    // H5 Dataset
    string               m_dataset_name;
    cH5Dataset*          m_dataset;

    // H5 Datatypes for disk and memory (could be T::compound_h5d/m)
    hid_t                m_disktype;
    hid_t                m_coretype;
    
    // H5 Disk Dataspace
    fasize_type          m_diskspace_extent;
    fasize_type          m_diskspace_max_extent;
    hid_t                m_diskspace;

    fasize_type          m_stride;
    fasize_type          m_offset;

    // H5 Memory Dataspace cache
    fasize_type          m_corespace_extent;
    hid_t                m_corespace;

    void create(unsigned int ver,
		const cH5DirectoryNode& parent_node, hid_t dataset_properties,
		fasize_type maximum_size, 
		fasize_type stride, fasize_type offset);

    void open(const cH5DirectoryNode& parent_node, 
	      fasize_type stride, fasize_type offset);

    void common_init(unsigned int ver);
    
    // Dataset functions
    void dataset_extend(fasize_type start, fasize_type count);

    // Disk dataspace functions
    void diskspace_sync(void);
    hid_t select_hyperslab(fasize_type start, fasize_type count);

    // Memory dataspace functions
    hid_t corespace_get(fasize_type extent);

    // Map between diskspace and userspace
    fasize_type rounder() const 
    { return m_stride-m_offset-1; }
    fasize_type disk2userS(fasize_type fs) const 
    { return (fs+rounder())/m_stride; }
    fasize_type user2diskS(fasize_type us) const 
    { return (us*m_stride)-rounder(); }
    fasize_type user2diskN(fasize_type un) const 
    { return (un*m_stride)+m_offset; }

    // Various disk space size functions
    fasize_type usersize() const { return disk2userS(m_diskspace_extent); }
    fasize_type umaxsize() const { return disk2userS(m_diskspace_max_extent); }
    fasize_type disksize() const { return m_diskspace_extent; }
    fasize_type fmaxsize() const { return m_diskspace_max_extent; }
    
  public:

    // CONSTRUCTORS
    VSFA_cH5(unsigned int              ver, 
	     const cH5DirectoryNode&   parent_node, 
	     const string&             dataset_name, 
	     hid_t                     dataset_properties=H5P_DEFAULT, 
	     fasize_type               maximum_size=0, 
	     fasize_type               stride=1,
	     fasize_type               offset=0
	     ) throw(Error):
      VSFA<T>(), 
      m_dataset_name(dataset_name), m_dataset(0), 
      m_disktype(-1), m_coretype(-1),
      m_diskspace_extent(0), m_diskspace_max_extent(0), m_diskspace(-1),
      m_stride(stride), m_offset(offset), 
      m_corespace_extent(0), m_corespace(-1)
    { 
      create(ver,parent_node,dataset_properties,maximum_size,
	     stride,offset);
    }

    VSFA_cH5(const cH5DirectoryNode&   parent_node, 
	     const string&             dataset_name, 
	     fasize_type               stride=1, 
	     fasize_type               offset=0
	     ) throw(Error):
      VSFA<T>(), 
      m_dataset_name(dataset_name), m_dataset(0), 
      m_disktype(-1), m_coretype(-1),
      m_diskspace_extent(0), m_diskspace_max_extent(0), m_diskspace(-1),
      m_stride(stride), m_offset(offset), 
      m_corespace_extent(0), m_corespace(-1)
    { 
      open(parent_node,stride,offset);
    }

    // DESTRUCTOR.. close all the H5 stuff
    virtual ~VSFA_cH5()
    {
      delete m_dataset;
      if(H5Tclose(m_disktype) < 0)
	throwerror("VSFA_cH5::~VSFA_cH5","Could not H5TClose(m_disktype)");
      
      if(H5Tclose(m_coretype) < 0)
	throwerror("VSFA_cH5::~VSFA_cH5","Could not H5TClose(m_coretype)");
      
      if(H5Sclose(m_diskspace) < 0)
	throwerror("VSFA_cH5::~VSFA_cH5","Could not H5SClose(m_diskspace)");

      if(H5Sclose(m_corespace) < 0)
	throwerror("VSFA_cH5::~VSFA_cH5","Could not H5SClose(m_corespace)");
    }

    // The standard interface, as defined by the base class

    const string& id() const { return m_dataset_name; }
    fasize_type size() const;
    bool unlimited() const;
    fasize_type maxsize() const;
    fasize_type read(fasize_type start, T* p, fasize_type count);
    fasize_type write(fasize_type start, const T* p, fasize_type count);
    void sync() {};
  };

} /* namespace NS_Analysis */

template <class T>
void 
NS_Analysis::VSFA_cH5<T>::
create(unsigned int ver,
       const cH5DirectoryNode& parent_node, hid_t dataset_properties,
       fasize_type maximum_size, fasize_type stride, fasize_type offset)
{
  herr_t error;

  // If the user hasn't given us a property type then create one
  hid_t property;
  if( (dataset_properties == H5P_DEFAULT ) ||
      (H5Pget_class(dataset_properties) != H5P_DATASET_CREATE) )
    {
      property=H5Pcreate(H5P_DATASET_CREATE);
      if(property < 0)throwerror("VSFA_cH5<>::create",
				 "cannot create dataset property");
    }
  else
    {
      property=H5Pcopy(dataset_properties);
      if(property < 0)throwerror("VSFA_cH5<>::create",
				 "cannot copy property list");
    }

  if(maximum_size == 0) 
    {
      // Set chunking option if maximum_size==0
      hsize_t chunk_dim[1]={VSFA_cH5_CHUNKING_SIZE};
      error=H5Pset_chunk (property, 1, chunk_dim);
      if(error < 0)throwerror("VSFA_cH5<>::create",
			      "cannot set chunking on dataset");
    }
      
  // Use the core_type / disk_type that T defines
  m_disktype=T::compoundCH5Disk(ver,false);

  // Setup data space... both initial no of elements and max elements
  // are given by maximum_size [or unlimited if maximum_size==0]
  m_diskspace_extent=maximum_size;
  m_diskspace_max_extent=maximum_size;
  
  hsize_t space_dim[1]={m_diskspace_extent};
  hsize_t space_max_dim[1]={m_diskspace_max_extent};
  if(m_diskspace_max_extent==0)space_max_dim[0]=H5S_UNLIMITED;
      
  m_diskspace=H5Screate_simple(1,space_dim,space_max_dim);
  if(m_diskspace < 0)throwerror("VSFA_cH5<>::create",
			      "cannot initialise data space");

  // Create the dataset
  m_dataset=
    parent_node.createDataset(m_dataset_name, m_disktype, 
			      m_diskspace, property);

  // Create and store the version number attribute
  cH5Attribute *attr=m_dataset->createAttribute("version",H5T_STD_U32LE,1);
  attr->write(H5T_NATIVE_UINT,&ver);
  delete attr;

  // Clean up
  H5Pclose(property);

  // Finally, set the common properties
  common_init(ver);
}

template <class T>
void 
NS_Analysis::VSFA_cH5<T>::
open(const cH5DirectoryNode& parent_node, 
     fasize_type stride, fasize_type offset)
{
  unsigned int ver;

  // Open the dataset
  m_dataset=parent_node.openDataset(m_dataset_name);

  // Find out the version number
  cH5Attribute *attr=m_dataset->openAttribute("version");
  attr->read(H5T_NATIVE_UINT,&ver);
  delete attr;
      
  // Get the datatype (mostly for the laugh.. we don't use it)
  m_disktype=m_dataset->getType();
  if(m_disktype < 0)throwerror("VSFA_cH5<>::open",
			     "cannot get datatype from datset");

  // Synchronise the diskspace to the dataset
  diskspace_sync();

  common_init(ver);
}

template <class T>
void
NS_Analysis::VSFA_cH5<T>::
common_init(unsigned int ver)
{
  // Datatypes
  m_coretype=T::compoundCH5Core(ver);

  if((m_stride==0)||(m_offset>=m_stride))throwerror("VSFA_cH5<>::common_init",
					      "Invalid m_stride or m_offset");

  // Set the version
  VSFA<T>::setTVersion(ver);

  // Initialise the corespace cache
  m_corespace_extent=1;
  hsize_t core_space_dim[1]={m_corespace_extent};
  m_corespace=H5Screate_simple(1,core_space_dim,core_space_dim);
  if(m_corespace < 0)throwerror("VSFA_cH5<>::common_init",
			      "cannot initialise MEMORY data space");

  return;
}  

template <class T>
inline hid_t 
NS_Analysis::VSFA_cH5<T>::
corespace_get(fasize_type extent)
{
  herr_t err;
  if(m_corespace_extent == extent)return m_corespace;

  hsize_t new_extent[1]={extent};
  err=H5Sset_extent_simple(m_corespace, 1, new_extent, new_extent );
  if(err < 0)throwerror("VSFA_cH5<>::corespace_get",
			"could not extend corespace");
  m_corespace_extent=extent;
  return m_corespace;
}

template <class T>
void 
NS_Analysis::VSFA_cH5<T>::
dataset_extend(fasize_type start, fasize_type count) 
{
  fasize_type uextent=start+count;
  herr_t err;

#ifdef FA_DEBUG
  cerr << "VSFA_cH5::dataset_extend(" << start << ',' << count << ')' 
       << "; new_extent " << user2diskS(uextent) << '\n';
#endif 

  // Check extent against maxsize
  if((fmaxsize() > 0)&&(uextent > umaxsize()))
    {
      VSFA_RangeError err("VSFA_cH5<>::data_extend",
			  start,count,usersize(),umaxsize());
      err.stream() << "EXTEND request would exceed dataset maximum size\n";
    }
  
  // Extend the dataset
  hsize_t new_extent[1]={user2diskS(uextent)};

  err=m_dataset->extend(new_extent);
  if(err < 0)throwerror("VSFA_cH5<>::dataset_extend",
			"could not EXTEND dataset");
  
  // Get the new diskspace that describes this dataset
  diskspace_sync();
}

template <class T>
void 
NS_Analysis::VSFA_cH5<T>::
diskspace_sync(void) 
{
  // See if diskspace is being used and clear it if so
  if((m_diskspace != -1)&&(H5Iget_type(m_diskspace) == H5I_DATASPACE))
    {
      if(H5Sclose(m_diskspace) < 0)
	throwerror("VSFA_cH5::diskspace_sync",
		   "Could not H5Sclose diskspace");
    }
  
  // Get the dataspace from the dataset
  m_diskspace=m_dataset->getSpace();
  if(m_diskspace < 0)throwerror("VSFA_cH5<>::diskspace_sync",
			      "cannot get disk dataspace");

  // Extract the dimension of the dataspace
  hsize_t dims[1];
  hsize_t maxdims[1];
  herr_t err=H5Sget_simple_extent_dims(m_diskspace,dims,maxdims);
  if(err < 0)throwerror("VSFA_cH5<>::diskspace_sync",
			"could not get extent of dataspace");

  // Set the size
  m_diskspace_extent=dims[0];
  m_diskspace_max_extent=maxdims[0];
  if(m_diskspace_max_extent==H5S_UNLIMITED)m_diskspace_max_extent=0;
}

template <class T>
inline hid_t 
NS_Analysis::VSFA_cH5<T>::
select_hyperslab(fasize_type start, fasize_type count)
{
  herr_t err;
  hsize_t h1st[1]={user2diskN(start)};
  hsize_t hstr[1]={m_stride};
  hsize_t hcnt[1]={count};
#ifdef FA_DEBUG
  cerr << "VSFA_cH5::select_hyperslab(" << start << ',' << count << ')' 
       << "; h1st:" << h1st[0] << " hstr:" << hstr[0] 
       << " hcnt:" << hcnt[0] << '\n';
#endif 
  err=H5Sselect_hyperslab(m_diskspace, H5S_SELECT_SET, h1st, hstr, hcnt, NULL);
  if(err < 0)throwerror("VSFA_cH5<>::select_hyperslab","cannot set hyperslab");
  return m_diskspace;
}

template <class T>
inline NS_Analysis::fasize_type
NS_Analysis::VSFA_cH5<T>::
size() const
{ 
#ifdef FA_DEBUG
  cerr << "VSFA_cH5::size() ; size=" << usersize() << '\n';
#endif 

  return usersize(); 
}

template <class T>
inline bool
NS_Analysis::VSFA_cH5<T>::
unlimited() const
{ 
#ifdef FA_DEBUG
  cerr << "VSFA_cH5::unlimited() ; disk-maxsize=" << fmaxsize() << '\n';
#endif 

  return (fmaxsize()==0);
}

template <class T>
inline NS_Analysis::fasize_type
NS_Analysis::VSFA_cH5<T>::
maxsize() const
{ 
#ifdef FA_DEBUG
  cerr << "VSFA_cH5::maxsize() ; maxsize=" << umaxsize() << '\n';
#endif 

  return umaxsize(); 
}

template <class T>
NS_Analysis::fasize_type
NS_Analysis::VSFA_cH5<T>::
read(fasize_type start, T* p, fasize_type count)
{
#ifdef FA_DEBUG
  cerr << "VSFA_cH5::read(" << start << ',' << count << ")\n";
#endif 

  herr_t err;
  
  if(start>=usersize())
    {
      VSFA_RangeError err("VSFA_cH5<>::read",
			  start,count,usersize(),umaxsize());
      err.stream() << "VSFA_cH5(" << id() << "): read: start out of range\n";
      throw(err);
    }
  
  if((start+count) > disksize())count=disksize()-start;
  
  hid_t disk_space=select_hyperslab(start,count);
  hid_t core_space=corespace_get(count);
  
  err=m_dataset->read(m_coretype, core_space, disk_space, p);
  if(err < 0)throwerror("VSFA_cH5<>::read","could not READ dataset");
  
  for(unsigned int i=0;i<count;i++)p[i].setVersion(VSFA<T>::tVersion());
  
  return count;
}

template <class T>
NS_Analysis::fasize_type
NS_Analysis::VSFA_cH5<T>::
write(fasize_type start, const T* p, fasize_type count)
{
#ifdef FA_DEBUG
  cerr << "VSFA_cH5::write(" << start << ',' << count << ")\n";
#endif 

  herr_t err;

  // ok so you can WRITE to any element from 0 to size() inclusive and any
  // number (count) in one go... we extend the dataset when required
  if(start > usersize())
    {
      sync();
      VSFA_RangeError err("VSFA_cH5<>::write",
			  start,count,usersize(),umaxsize());
      err.stream() 
	<< "VSFA_cH5(" << id() << "): write: start out of range";
      throw(err);
    }
  
  if((start+count) > usersize())dataset_extend(start,count);

  hid_t disk_space=select_hyperslab(start,count);
  hid_t core_space=corespace_get(count);
  
  err=m_dataset->write(m_coretype, core_space, disk_space, p);
  if(err < 0)throwerror("VSFA_cH5<>::write","could not WRITE dataset");
  
  return count;
}

#endif // VSFA_CH5_H
