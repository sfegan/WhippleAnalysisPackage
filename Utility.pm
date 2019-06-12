package Utility;
use strict;

sub Type
  {
    my $type=shift;
    $type =~ tr[\~][ ];
    return $type;
  }

sub MType
  {
    my $type=Type(shift);
    $type="char" if ( $type eq "string" );
    return $type;
  }

sub MFormat
  {
    my $type=Type(shift);
    my $format;
    $format="%lf" if ($type eq "double");
    $format="%f" if  ($type eq "float");
    $format="%d" if  ($type eq "int");
    $format="%u" if  ($type eq "unsigned int");
    $format="%ld" if ($type eq "long");
    die "Unknown format for type ".$type if ( not defined $format );
    return $format;
  }

sub MakeType
  {
    my $type=shift;
    my $dim=shift;
    return MType($type).((defined $dim)?"*":"");
  }

sub LCName
  {
    my $name=shift;
    $name =~ s/([^A-Z])([A-Z])/$1_$2/g;
    return lc $name;
  }

sub MakePerlFunc
  {
    my $type=shift;
    my $dim=shift;
    my $mtype=MType($type);
    if($type eq "string")
      {
	return "";
      }
    elsif(($type eq "float")or($type eq "double"))
      {
	return "SvNV";
      }
    else
      {
	return "SvIV";
      }
  }

sub MakePerlNewFunc
  {
    my $type=shift;
    my $dim=shift;
    my $mtype=MType($type);
    if($type eq "string")
      {
	return "";
      }
    elsif(($type eq "float")or($type eq "double"))
      {
	return "newSVnv";
      }
    else
      {
	return "newSViv";
      }
  }

sub MakeVar
  {
    my $name=shift;
    return "m_".$name;
  }

sub PrintElement
  {
    my $type=shift;
    my $name=shift;
    my $dim=shift;
    my $comment=shift;
    my $unit=shift;
    my $fmtw=shift;

    my $line="";

    my $printname=
      sprintf("%-".$fmtw."s",
	      MType($type)." ".MakeVar($name).(defined $dim?"[".$dim."]":"").";");
    $line.=$printname;

    if(defined($comment))
      {
	my $clmax=77 - 7 - length($printname) - 
	  ((defined $unit)?(length($unit)+3):0);
	
	$line.=sprintf(" /* %-".$clmax.".".$clmax."s",$comment);
	$line.=" [".$unit."]" if ( defined $unit );
	$line.=" */";
      }

    return $line;
  }

sub PrintAccessor
  {
    my $type=shift;
    my $name=shift;
    my $dim=shift;

    my $line="";

    if(defined $dim)
      {
	if(Type($type) eq "string")
	  {
	    $line .= join(" ","string",$name."()","const","{",
			  "int n;",
	       "for(n=0;(n<".$dim.")&&(".MakeVar($name)."[n]!=0);n++);",
			  "return","string(".MakeVar($name).",n);","}");
	  }
	else
	  {
	    $line .= join(" ","const",MakeType($type,$dim),
			  $name."()","const",
			  "{ return ".MakeVar($name)."; }");
	  }
      }
    else
      {
	$line .= join(" ",MakeType($type,$dim),$name."()","const",
		      "{ return ".MakeVar($name)."; }");
      }

    return $line;
  }

sub PrintSetAccessor
  {
    my $type=shift;
    my $name=shift;
    my $dim=shift;

    my $line="";

    if(defined $dim)
      {
	if(Type($type) eq "string")
	  {
	    $line = join(" ","string",
			 $name."(const string& X)",
			 "{ X.copy(".MakeVar($name).",".$dim.');',
	 "if(X.length()<".$dim.")".MakeVar($name)."[X.length()]=0;",
			 'return '.$name.'();','}');
	  }
	else
	  {
	    $line = join(" ","const",MakeType($type,$dim),
			 $name."(".MakeType($type,$dim)." X)",
			 "{",
			 "memcpy(".MakeVar($name).",X,".$dim."*sizeof(*X));",
			 'return '.MakeVar($name).';',"}");
	  }
      }
    else
      {
	$line = join(" ",
		     MakeType($type,$dim),
		     $name."(".MakeType($type,$dim)." X)",
		     "{",MakeVar($name)."=X;",
		     "return ".MakeVar($name).';',"}");
      }
    
    return $line;
  }

my %HDF5_D_Types=( 
		  "char"           => "H5T_STD_I8LE",
		  "short"          => "H5T_STD_I16LE",
		  "int"            => "H5T_STD_I32LE",
		  "signed char"    => "H5T_STD_I8LE",
		  "signed short"   => "H5T_STD_I16LE",
		  "signed int"     => "H5T_STD_I32LE",
		  "unsigned char"  => "H5T_STD_U8LE",
		  "unsigned short" => "H5T_STD_U16LE",
		  "unsigned"       => "H5T_STD_U32LE",
		  "unsigned int"   => "H5T_STD_U32LE",
		  "double"         => "H5T_IEEE_F64LE",
		  "float"          => "H5T_IEEE_F32LE",
		  "string"         => "STRING",
		  "hbool_t"        => "H5T_NATIVE_HBOOL",
		 );

my %HDF5_M_Types=( 
		  "char"           => "H5T_NATIVE_CHAR",
		  "short"          => "H5T_NATIVE_SHORT",
		  "int"            => "H5T_NATIVE_INT",
		  "signed char"    => "H5T_NATIVE_CHAR",
		  "signed short"   => "H5T_NATIVE_SHORT",
		  "signed int"     => "H5T_NATIVE_INT",
		  "unsigned char"  => "H5T_NATIVE_UCHAR",
		  "unsigned short" => "H5T_NATIVE_USHORT",
		  "unsigned"       => "H5T_NATIVE_UINT",
		  "unsigned int"   => "H5T_NATIVE_UINT",
		  "double"         => "H5T_NATIVE_DOUBLE",
		  "float"          => "H5T_NATIVE_FLOAT",
		  "string"         => "STRING",
		  "hbool_t"        => "H5T_NATIVE_HBOOL",
		 );

sub HDF5Name
  {
    my $name=shift;
    return $name;
  }

sub ISCompound
  {
    my $type=shift;
    return(not exists $HDF5_M_Types{Type($type)});
  }

sub HDF5_M_Type
  {
    my $type=shift;
    my $ctype=$type."::compound_h5m";
    $ctype=$HDF5_M_Types{Type($type)} if ( exists $HDF5_M_Types{Type($type)} );
    return $ctype;
  };

sub HDF5_D_Type
  {
    my $type=shift;
    my $ctype=$type."::compound_h5d";
    $ctype=$HDF5_D_Types{Type($type)} if ( exists $HDF5_D_Types{Type($type)} );
    return $ctype;
  };

sub ExpandVersion
  {
    my $vstring=shift;

    my @vparts;
    @vparts = split(/_/, $vstring);

    my @vexpandedparts;

    my $vpart;
    foreach $vpart ( @vparts )
      {
	next unless ( ($vpart =~ /^\d+/) or ($vpart =~ /\d+$/) );

	my $range = $vpart =~ /^(\d+)?[\-](\d+)?$/;
	my $vfirst;
	$vfirst = $1 if ($vpart =~ /^(\d+)/);
	my $vlast;
	$vlast = $1 if ($vpart =~ /[\-](\d+)$/);

	if(not $range)
	  {
	    push @vexpandedparts,"v==".$vfirst;
	  }
	elsif((defined $vfirst)and(defined $vlast))
	  {
	    push @vexpandedparts,"(v>=".$vfirst.")&&(v<=".$vlast.")";
	  }
	elsif(defined $vfirst)
	  {
	    push @vexpandedparts,"v>=".$vfirst;
	  }
	elsif(defined $vlast)
	  {
	    push @vexpandedparts,"v<=".$vlast;
	  }
      }
    
    return "(".join(")||(",@vexpandedparts).")";
  }


1;
