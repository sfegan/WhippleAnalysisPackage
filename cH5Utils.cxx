#include"cH5Utils.h"

using std::endl;

///////////////////////////////////////////////////////////////////////////////
////////////////////////////// cH5Group functions /////////////////////////////
///////////////////////////////////////////////////////////////////////////////

NS_Analysis::cH5Group::
cH5Group(const cH5DirectoryNode& base, string name, bool create)
{
  hid_t ch5g;

  string::size_type start=0;
  string::size_type count=name.size();
  if(name[count-1]=='/')count--;
  if(name[start]=='/')start++,count--;
  name=name.substr(start,count);

  if(create)
    ch5g=H5Gcreate(base.h5t(),name.c_str(),0);
  else
    ch5g=H5Gopen(base.h5t(),name.c_str());
  
  if(ch5g<0)
    {
      cH5Error err("cH5Group::cH5Group");
      err.stream() << "Could not " << ((create)?"create":"open") 
		   << " group " << name << endl;
      throw(err);
    }
  
  setnode(base.locstr()+name+'/',ch5g, name);
}

///////////////////////////////////////////////////////////////////////////////
////////////////////////// cH5Attribute functions /////////////////////////////
///////////////////////////////////////////////////////////////////////////////

NS_Analysis::cH5Attribute::
cH5Attribute(const cH5Location& base, const string& name, 
	     hid_t type, hsize_t size)
{
  hid_t space=H5Screate_simple(1,&size,&size);
  if(space<0)
    {
      cH5Error err("cH5Attribute::cH5Attribute");
      err.stream() << "Could not create attribute space of size " 
		   << size << endl;
      throw(err);
    }

  hid_t attr=H5Acreate(base.h5t(), name.c_str(), type, space, H5P_DEFAULT); 
  if(attr<0)
    {
      cH5Error err("cH5Attribute::cH5Attribute");
      err.stream() << "Could not create attribute " << name 
		   << " under " << base.name() << endl;
      throw(err);
    }
  
  setloc(attr);
  setname(name);

  if(H5Sclose(space) < 0)
    {
      cH5Error err("cH5Attribute::cH5Attribute");
      err.stream() << "Could not close attribute space " << name 
		   << " under " << base.name() << endl;
      throw(err);
    }
}

NS_Analysis::cH5Attribute::
cH5Attribute(const cH5Location& base, const string& name)
{
  hid_t attr=H5Aopen_name(base.h5t(),name.c_str());
  if(attr<0)
    {
      cH5Error err("cH5Attribute::cH5Attribute");
      err.stream() << "Could not open attribute " << name 
		   << " under " << base.name() << endl;
      throw(err);
    }

  setloc(attr);
  setname(name);
}

///////////////////////////////////////////////////////////////////////////////
/////////////////////////// cH5Dataset Functions //////////////////////////////
///////////////////////////////////////////////////////////////////////////////

NS_Analysis::cH5Dataset::
cH5Dataset(const cH5DirectoryNode& base, const string& name, 
	   hid_t type, hid_t space, hid_t property)
{
  hid_t dset=H5Dcreate(base.h5t(), name.c_str(), type, space, property); 
  if(dset<0)
    {
      cH5Error err("cH5Dataset::cH5Dataset");
      err.stream() << "Could not create dataset " << name 
		   << " under " << base.locstr() << endl;
      throw(err);
    }
  
  setloc(dset);
  setname(name);
}

NS_Analysis::cH5Dataset::
cH5Dataset(const cH5DirectoryNode& base, const string& name)
{
  hid_t dset=H5Dopen(base.h5t(),name.c_str());
  if(dset<0)
    {
      cH5Error err("cH5Dataset::cH5Dataset");
      err.stream() << "Could not open dataset " << name 
		   << " under " << base.locstr() << endl;
      throw(err);
    }

  setloc(dset);
  setname(name);
}

///////////////////////////////////////////////////////////////////////////////
////////////////////////////// cH5File functions //////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void
NS_Analysis::cH5File::
create(const string& filename, unsigned flags, 
       hid_t create_id, hid_t access_id)
{
  hid_t ch5f;
  ch5f=H5Fcreate(filename.c_str(),flags,create_id,access_id);
  if(ch5f<0)
    {
      cH5Error err("cH5File::cH5File");
      err.stream() << "Could not create file " << filename << endl;
      throw(err);
    }
  setnode(filename+" : /",ch5f,filename);
}

void
NS_Analysis::cH5File::
open(const string& filename, unsigned flags,
     hid_t access_id)
{
  hid_t ch5f;
  ch5f=H5Fopen(filename.c_str(),flags,access_id);
  if(ch5f<0)
    {
      cH5Error err("cH5File::cH5File");
      err.stream() << "Could not open file " << filename << endl;
      throw(err);
    }
  setnode(filename+" : /",ch5f,filename);
}

NS_Analysis::cH5File::
cH5File(const string& filename, bool mustcreate)
{
  if(mustcreate)create(filename);
  else open(filename);
}
