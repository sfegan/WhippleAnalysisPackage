#!/usr/bin/perl -w
use strict;

my @n2;
my @onoff;
my @trk;

my $line;
foreach $line (<>)
  {
    chomp $line;
    $line =~ s/^\s+//;
    $line =~ s/\s+$//;

    my @bits=split(/\s+/,$line);

    if($bits[0] eq 'n2')
      {
	my $shortname=$bits[1];
	$shortname =~ s/gt[0-9]{2}([0-9]{4})/gt$1/;
	push @n2,$shortname;
      }
    elsif(($bits[0] eq 'pr') && ($bits[2] eq "none"))
      {
	my $shortname=$bits[1];
	$shortname =~ s/gt[0-9]{2}([0-9]{4})/gt$1/;
	push @trk,[$shortname,"gt".$bits[3]];
      }
    elsif($bits[0] eq 'pr')
      {
	my $on_shortname=$bits[1];
	$on_shortname =~ s/gt[0-9]{2}([0-9]{4})/gt$1/;
	my $off_shortname=$bits[2];
	$off_shortname =~ s/gt[0-9]{2}([0-9]{4})/gt$1/;
	push @onoff,[$on_shortname, $off_shortname, "gt".$bits[3]];
      }
  }

print "red2h5 ",join(" ",
		     @n2,
		     (map { $_->[0] } @trk),
		     (map { ($_->[0], $_->[1]) } @onoff)),"\n";
print "gcpeds ",join(" ",
		     ( map { $_.".h5" }
		       @n2,
		       (map { $_->[0] } @trk),
		       (map { ($_->[0], $_->[1]) } @onoff))),"\n";
print "gn2gains ",join(" ",map { $_.".h5" } @n2),"\n";
print join("\n",
	   (map { "gparamdat ".$_->[0].".h5 ".$_->[1] } @trk),
	   "");
print join("\n",
	   (map { ( "gparamdat --pad=".$_->[1]." ".$_->[0].".h5 ".$_->[2],
		    "gparamdat --pad=".$_->[0]." ".$_->[1].".h5 ".$_->[2] ) }
	    @onoff),"");
print "gcut `cat Supercuts` ",join(" ",map { $_->[0]."_p.h5" } @trk),"\n";
print "gcut `cat Supercuts` ",join(" ",map { $_->[0]."_p.h5" } @onoff),"\n";
print "gcut `cat Supercuts` ",join(" ",map { $_->[1]."_p.h5" } @onoff),"\n";
