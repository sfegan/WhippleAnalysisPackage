#!/bin/perl -w

use lib 'perl/';

use strict;
use FileHandle;
use POSIX;

use RedHeader;
use RedEvent;
use RedFile;

sub read_f77
{
    my $fp=shift;
    my $format=shift;
    
    my $readbuffer;

    my $packlength=length pack($format);
    
    sysread($fp,$readbuffer,4);
    my $f77length=unpack("L",$readbuffer);
    
    die "F77 record of $f77length bytes != expected $packlength bytes"
        if($f77length != $packlength);

    sysread($fp,$readbuffer,$packlength);
    my @buffer=unpack($format,$readbuffer);

    sysread($fp,$readbuffer,4);
    $f77length=unpack("L",$readbuffer);
    
    die "Could not find F77 terminator"
        if($f77length != $packlength);

    return @buffer;
}

my $filename=shift(@ARGV);
die "No file specified" unless (defined $filename);

my $fp=new FileHandle $filename,"r";
die "Could not open $filename: $!" unless (defined $fp);

my $h5fp=new RedFile $filename.".h5",1;

# runid nevents live_time stdur mode source date mjd frjd ra dec ut st
# azimuth elevation skyq comms gpsbeg(x7)
my ($runid, $nevents, $live_time, $stdur, $mode, $source, $date, $mjd,
    $frjd, $ra, $dec, $ut, $st, $azimuth, $elevation, $skyq, $comms,
    @gpsbeg) = read_f77($fp,"a4idia3a20iddffiiffa6a404i7");

my $TUBES=492;

my $h=new RedHeader;
$h->runid($runid);
$h->nevents($nevents);
$h->live_time($live_time);
$h->stdur($stdur);
$h->mode($mode);
$h->source($source);
$h->date($date);
$h->mjd($mjd);
$h->frjd($frjd);
$h->ra($ra);
$h->dec($dec);
$h->ut($ut);
$h->st($st);
$h->azimuth($azimuth);
$h->elevation($elevation);
$h->skyq($skyq);
$h->comms($comms);
$h->gpsbeg(@gpsbeg);

$h5fp->header->write(0,$h);

my $e=new RedEvent;
my $eventslist=$h5fp->events;
my $ev;
for($ev=0;$ev<$nevents;$ev++)
  {
    my ($code, $time, $gpsutc, $livetime, @adc ) = 
      read_f77($fp,"iddds".$TUBES);
    
    $e->code($code);
    $e->time($time);
    $e->gpsutc($gpsutc);
    $e->livetime($livetime);
    $e->adc(@adc);

    $eventslist->write($ev,$e);

    updatepb($ev,$nevents);
  }

print STDERR "\n";

sub updatepb
  {
    my $e=shift;
    my $n=shift;
    my $pb=shift;

    if(($e % int(($n+78)/79))==0)
      {
	print STDERR "\r",("=" x int($e/$n*79));
      }
  }
