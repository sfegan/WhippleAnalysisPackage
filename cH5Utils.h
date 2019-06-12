//-*-mode:c++; mode:font-lock;-*-

//
// Some useful classes for using the HDF5 C-interface. There is a
// native C++ interface to HDF5 upcoming from NCSA. Personally I can't
// wait. In the mean time this class is meant to provide A FEW SIMPLE
// classes to ease the way. It is DEFINATELY not a fully functional
// interface.
//

#ifndef CH5_H
#define CH5_H

#define __STL_USE_NAMESPACES

#include<string>

#include<hdf5/hdf5.h>

#include"Exceptions.h"

namespace NS_Analysis {

  using std::string;

  class H5ErrorPrintingStopper
  {
  private:
    H5E_auto_t h5e_func;
    void *h5e_client_data;
    
  public:
    H5ErrorPrintingStopper() 
    { 
      H5Eget_auto(&h5e_func, &h5e_client_data); 
      H5Eset_auto(NULL, NULL);
    }
    
    ~H5ErrorPrintingStopper() { H5Eset_auto(h5e_func, h5e_client_data); };
  };
  
  class cH5Group;
  class cH5Dataset;
  class cH5Attribute;

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////// cH5Location /////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  
  class cH5Location
  {
  private:
    hid_t m_loc;
    string m_name;

    cH5Location& operator=(cH5Location&);
    cH5Location(cH5Location&);

  protected:
    void setloc(hid_t h5t) { m_loc=h5t; }
    void setname(const string& name) { m_name=name; }

  public:
    cH5Location(hid_t h5t, const string& name): m_loc(h5t), m_name(name) {}
    cH5Location(): m_loc(), m_name() {}

    hid_t h5t() const { return m_loc; }
    const string& name() const { return m_name; }

    cH5Attribute* createAttribute(const string& name, hid_t type, 
				  unsigned int size=1) const;
    cH5Attribute* openAttribute(const string& name) const;
  };
  
  /////////////////////////////////////////////////////////////////////////////
  //////////////////////////// cH5DirectoryNode ///////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  
  class cH5DirectoryNode: public cH5Location
  {
  private:
    string m_locstr;
    
  protected:
    void setnode(const string& loc, hid_t h5t, const string& name) 
      { m_locstr=loc; setloc(h5t); setname(name); }
    cH5DirectoryNode(): cH5Location(), m_locstr() {}
    
  public:
    cH5DirectoryNode(const string& loc, hid_t h5t): 
     cH5Location(h5t,loc),  m_locstr(loc) {}
    
    // Standard accessors
    const string& locstr() const { return m_locstr; }

    // What is this object anyway
    hid_t whatIs(const string& name, bool follow_link=true);
    bool isDataset(const string& name) { return whatIs(name)==H5G_DATASET; }
    bool isGroup(const string& name) { return whatIs(name)==H5G_GROUP; }
    bool isLink(const string& name) { return whatIs(name,false)==H5G_LINK; }
    bool isType(const string& name) { return whatIs(name)==H5G_TYPE; }

    // Functions common to both file and group
    cH5Group* createGroup(const string& name) const;
    cH5Group* openGroup(const string& name) const;
    
    cH5Dataset* createDataset(const string& name, hid_t type, hid_t space,
			      hid_t property=H5P_DEFAULT) const;
    cH5Dataset* openDataset(const string& name) const;
  };
  
  /////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////// cH5File ///////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  
  class cH5File: public cH5DirectoryNode
  {
  protected:
    void create(const string& filename, unsigned flags=H5F_ACC_EXCL, 
		hid_t create_id=H5P_DEFAULT, hid_t access_id=H5P_DEFAULT);
    void open(const string& filename, unsigned flags=H5F_ACC_RDONLY, 
	      hid_t access_id=H5P_DEFAULT);
    cH5File() {}
  public:
    cH5File(const string& filename, bool mustcreate);
    virtual ~cH5File() { H5Fclose(h5t());  H5garbage_collect(); }
  };

  class cH5FileNewTruncate: public cH5File
  {
  public:
    cH5FileNewTruncate(const string& filename) 
    { create(filename,H5F_ACC_TRUNC); }
  };

  class cH5FileNewExclusive: public cH5File
  {
  public:
    cH5FileNewExclusive(const string& filename) 
    { create(filename,H5F_ACC_EXCL); }
  };

  class cH5FileReadOnly: public cH5File
  {
  public:
    cH5FileReadOnly(const string& filename) { open(filename,H5F_ACC_RDONLY); }
  };

  class cH5FileReadWrite: public cH5File
  {
  public:
    cH5FileReadWrite(const string& filename) { open(filename,H5F_ACC_RDWR); }
  };
  
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////// cH5Group ////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  class cH5Group: public cH5DirectoryNode
  {
  public:
    cH5Group(const cH5DirectoryNode& base, string name, bool create);
    virtual ~cH5Group() { H5Gclose(h5t()); }
  };

  /////////////////////////////////////////////////////////////////////////////
  ///////////////////////////// cH5Attribute //////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  class cH5Attribute: public cH5Location
  {
  public:
    cH5Attribute(const cH5Location& base, const string& name);
    cH5Attribute(const cH5Location& base, const string& name, hid_t type,
		 hsize_t size=1);
    virtual ~cH5Attribute() { H5Aclose(h5t()); }
    
    hid_t getSpace() const { return H5Aget_space(h5t()); }
    hid_t getType() const { return H5Aget_type(h5t()); }
    herr_t write(hid_t core_type, const void *buffer) const;
    herr_t read(hid_t core_type, void *buffer) const;
  };

  /////////////////////////////////////////////////////////////////////////////
  ////////////////////////////// cH5Dataset ///////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  class cH5Dataset: public cH5Location
  {
  public:
    cH5Dataset(const cH5DirectoryNode& base, const string& name);
    cH5Dataset(const cH5DirectoryNode& base, const string& name, 
	       hid_t type, hid_t space, hid_t property=H5P_DEFAULT);
    virtual ~cH5Dataset() { H5Dclose(h5t()); }
    
    hid_t getSpace() const { return H5Dget_space(h5t()); }
    hid_t getType() const { return H5Dget_type(h5t()); }
    herr_t extend(const hsize_t* size) const { return H5Dextend(h5t(),size); } 

    herr_t write(hid_t core_type, hid_t core_space, 
		 hid_t file_space, const void *buffer, 
		 hid_t xfer_plist=H5P_DEFAULT) const;
    herr_t read(hid_t core_type, hid_t core_space, 
		hid_t file_space, void *buffer, 
		hid_t xfer_plist=H5P_DEFAULT) const;
  };

} // namespace NS_Analysis



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///                                                                         ///
///                                                                         ///
///                                                                         ///
///                                                                         ///
///                       Member function definitions                       ///
///                                                                         ///
///                                                                         ///
///                                                                         ///
///                                                                         ///
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
///////////////////////////// cH5Location functions ///////////////////////////
///////////////////////////////////////////////////////////////////////////////

inline NS_Analysis::cH5Attribute*
NS_Analysis::cH5Location::createAttribute(const string& name, hid_t type, 
					  unsigned int size) const
{
  return new cH5Attribute(*this, name, type, size);
}

inline NS_Analysis::cH5Attribute*
NS_Analysis::cH5Location::openAttribute(const string& name) const
{
  return new cH5Attribute(*this, name);
}

///////////////////////////////////////////////////////////////////////////////
////////////////////////// cH5DirectoryNode functions /////////////////////////
///////////////////////////////////////////////////////////////////////////////

inline NS_Analysis::cH5Group* 
NS_Analysis::cH5DirectoryNode::
createGroup(const string& name) const
{
  return new cH5Group(*this, name, true);
}

inline NS_Analysis::cH5Group* 
NS_Analysis::cH5DirectoryNode::
openGroup(const string& name) const
{
  return new cH5Group(*this, name, false);
}

inline NS_Analysis::cH5Dataset*
NS_Analysis::cH5DirectoryNode::
createDataset(const string& name, hid_t type, hid_t space,
	      hid_t property) const
{
  return new cH5Dataset(*this, name, type, space, property);
}

inline NS_Analysis::cH5Dataset*
NS_Analysis::cH5DirectoryNode::
openDataset(const string& name) const
{
  return new cH5Dataset(*this, name);
}

inline int 
NS_Analysis::cH5DirectoryNode::
whatIs(const string& name, bool follow_link)
{
  H5ErrorPrintingStopper stopper;
  H5G_stat_t statbuf;
  
  if(herr_t err=H5Gget_objinfo(h5t(),name.c_str(),follow_link,&statbuf) < 0)
    return err;
  
  return statbuf.type;
}

///////////////////////////////////////////////////////////////////////////////
////////////////////////// cH5Attribute functions /////////////////////////////
///////////////////////////////////////////////////////////////////////////////

inline herr_t
NS_Analysis::cH5Attribute::write(hid_t core_type, const void *buffer) const
{
  return H5Awrite(h5t(),core_type,const_cast<void *>(buffer));
}

inline herr_t
NS_Analysis::cH5Attribute::read(hid_t core_type, void *buffer) const
{
  return H5Aread(h5t(),core_type,buffer);
}

///////////////////////////////////////////////////////////////////////////////
/////////////////////////// cH5Dataset Functions //////////////////////////////
///////////////////////////////////////////////////////////////////////////////

inline herr_t
NS_Analysis::cH5Dataset::
write(hid_t core_type, hid_t core_space, 
      hid_t file_space, const void *buffer, hid_t xfer_plist) const
{
  return H5Dwrite(h5t(), core_type, core_space, file_space, xfer_plist, buffer);
}

inline herr_t
NS_Analysis::cH5Dataset::
read(hid_t core_type, hid_t core_space, 
     hid_t file_space, void *buffer, hid_t xfer_plist) const
{
  return H5Dread(h5t(), core_type, core_space, file_space, xfer_plist, buffer);
}

#endif // CH5UTILS
