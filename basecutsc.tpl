@perl use lib ".";
@perl use Utility;
@//
@// Generate the base cuts class structures
@//
@foreach class_list          
  @perl $myfilename="BaseCuts".${class_name}.".sc";
  @perl print "Generating $myfilename\n";
  @openfile $myfilename
  @include noedit.tpl
class BaseCuts${class_name} {
  @foreach attr_list 
    @if((defined $cuts)&&(($cuts eq "both")||($cuts eq "lower")))
        hbool_t         ApplyLowerCut${attr_name};
	$attr_type	LowerCut${attr_name};
    @endif
    @if((defined $cuts)&&(($cuts eq "both")||($cuts eq "upper")))
        hbool_t         ApplyUpperCut${attr_name};
	$attr_type	UpperCut${attr_name};
    @endif
  @end
}
@end
