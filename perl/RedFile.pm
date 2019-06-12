package RedFile;

require Exporter;
require DynaLoader;

@ISA = qw(Exporter DynaLoader);
# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXDORT_OK instead.
# Do not simply export all your public functions/methods/constants.
$EXPORT = qw(
           );

$VERSION = '1.0';

bootstrap RedFile $VERSION;
