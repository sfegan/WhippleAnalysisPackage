@perl use lib ".";
@perl use Utility;
@//
@// Generate the class structures
@//
@foreach class_list          

@perl $myfilename=${class_name}.".cxx";
@perl print "Generating $myfilename\n";
@openfile $myfilename
@include noedit.tpl

#define __STL_USE_NAMESPACES

#include <iostream>

using std::endl;
using std::ostream;

#ifdef COMPOUND_H5
#include<hdf5/hdf5.h>
#endif

#include"${class_name}.h"

ostream& 
NS_Analysis::operator<<(ostream& stream, const ${class_name}& me)
{
  me.streamDumpLong(stream);
  return stream;
}

ostream& 
NS_Analysis::${class_name}::streamDumpLong(ostream& stream) const
{
  @foreach attr_list 
  stream << "$attr_name: ";
    @if ( defined($dim) ) and ( $attr_type ne "string" )
  for(unsigned int i=0;i<size${attr_name}();i++)
    {
      stream << get${attr_name}(i);
      if(i!=(size${attr_name}()-1)) stream << ' ';
    }
    @else
  stream << get${attr_name}();
    @endif
    @if ( ( not defined $unit ) or ( $unit eq "none" ) )
  stream << "\\n";
    @else
  stream << " ${unit}\\n";
    @endif
  @end
  return stream;
}

ostream& 
NS_Analysis::${class_name}::streamDumpShort(ostream& stream) const
{
  @foreach attr_list 
    @if ( defined($dim) ) and ( $attr_type ne "string" )
  for(unsigned int i=0;i<size${attr_name}();i++)
    {
      stream << get${attr_name}(i);
    }
    @else
  stream << get${attr_name}();
    @endif
  stream << '\\t';
  @end
  return stream;
}

#ifndef NO_COMPOUND_CH5
//-----------------------------------------------------------------------------
// Create the *unpacked* HDF5 memory representation of this structure... uses
// NATIVE types which are required when manipulating data in memory.
//-----------------------------------------------------------------------------

hid_t 
NS_Analysis::${class_name}::
compoundCH5Core(unsigned int version)
{
  herr_t err;
//  hsize_t dims[4];
//  hid_t temp_hid_t;
  hid_t h5type_core;
//  unsigned int v=version;

  h5type_core=H5Tcreate(H5T_COMPOUND,sizeof(${class_name}));
  if(h5type_core<0)return(h5type_core);

@foreach attr_list 
  @if ( not defined $nohdf )
    @perl $hdf_name=Utility::HDF5Name($attr_name);
    @perl $hdf_type=Utility::HDF5_M_Type($attr_type);

  /* m_$attr_name -> $hdf_name */
    @if ( defined $version )
      @perl $version_only_text=Utility::ExpandVersion($version);
  if($version_only_text)
    {
    @endif
    @if ( $hdf_type eq "STRING" )
  {
    hid_t temp_hid_t;
    temp_hid_t=H5Tcopy (H5T_C_S1);
    H5Tset_size (temp_hid_t, $dim);
    err=H5Tinsert(h5type_core, "$hdf_name",
                  HOFFSET($class_name,m_$attr_name),temp_hid_t); 
  }
    @elsif ( not defined $dim )
  err=H5Tinsert(h5type_core, "$hdf_name",
                HOFFSET($class_name,m_$attr_name), 
                $hdf_type); 
    @else
  {
    hsize_t dims[4];
    dims[0]=size${attr_name}(version);
#if ((H5_VERS_MAJOR > 1) || ((H5_VERS_MAJOR == 1)&&(H5_VERS_MINOR >= 4)))
    hid_t temp_hid_t;
    temp_hid_t=H5Tarray_create( $hdf_type, 1, dims, NULL);
    err=H5Tinsert(h5type_core, "$hdf_name",
                  HOFFSET($class_name,m_$attr_name), temp_hid_t);
#else
    err=H5Tinsert_array(h5type_core,"$hdf_name", 
                        HOFFSET($class_name,m_$attr_name), 
                        1, dims, NULL, $hdf_type);
#endif
  }
    @endif      
  if(err<0)return(err);
    @if ( defined $version )
  } /* $version_only_text */
    @endif
  @endif
@end

  return h5type_core;
}

//-----------------------------------------------------------------------------
// Create the packed HDF5 disk representation of this structure... uses INTEL
// type Little-Endian data types to adhere to standards for VERITAS arch.
//-----------------------------------------------------------------------------

hid_t 
NS_Analysis::${class_name}::
compoundCH5Disk(unsigned int version, bool pack)
{
  herr_t err;
//  hsize_t dims[4];
//  hid_t temp_hid_t;
  hid_t h5type_disk;
//  unsigned int v=version;

  h5type_disk=H5Tcreate(H5T_COMPOUND,sizeof(${class_name}));
  if(h5type_disk<0)return(h5type_disk);

@foreach attr_list 
  @if ( not defined $nohdf )
    @perl $hdf_name=Utility::HDF5Name($attr_name);
    @perl $hdf_type=Utility::HDF5_D_Type($attr_type);

  /* m_$attr_name -> $hdf_name */
    @if ( defined $version )
      @perl $version_only_text=Utility::ExpandVersion($version);
  if($version_only_text)
    {
    @endif
    @if ( $hdf_type eq "STRING" )
  {
    hid_t temp_hid_t;
    temp_hid_t=H5Tcopy (H5T_C_S1);
    H5Tset_size (temp_hid_t, $dim);
    err=H5Tinsert(h5type_disk, "$hdf_name",
                  HOFFSET($class_name,m_$attr_name),temp_hid_t); 
  }
    @elsif ( not defined $dim )
  err=H5Tinsert(h5type_disk, "$hdf_name",
                HOFFSET($class_name,m_$attr_name), 
                $hdf_type);
    @else
  {
    hsize_t dims[4];
    dims[0]=size${attr_name}(version);
#if ((H5_VERS_MAJOR > 1) || ((H5_VERS_MAJOR == 1)&&(H5_VERS_MINOR >= 4)))
    hid_t temp_hid_t;
    temp_hid_t=H5Tarray_create( $hdf_type, 1, dims, NULL);
    err=H5Tinsert(h5type_disk, "$hdf_name",
                HOFFSET($class_name,m_$attr_name), temp_hid_t);
#else
    err=H5Tinsert_array(h5type_disk,"$hdf_name", 
                        HOFFSET($class_name,m_$attr_name), 
                        1, dims, NULL, $hdf_type);
#endif
  }
    @endif      
  if(err<0)return(err);
    @if ( defined $version )
  } /* $version_only_text */
    @endif
  @endif
@end

  if(pack)H5Tpack(h5type_disk);

  return h5type_disk;
}
#endif /* NO_COMPOUND_CH5 */
@end
