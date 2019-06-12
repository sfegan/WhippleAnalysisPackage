CXX=g++
LIBS=-lhdf5 -lz -lm
DEBUG=#-DDEBUG_PERL_INTERFACE
PERLINCLUDE=-I/usr/lib/perl5/5.00503/i386-linux/CORE/
CXXFLAGS=$(DEBUG) -DCOMPOUND_H5 $(PERLINCLUDE) #-O6

export JEEVESLIBDIR=../Jeeves/lib
export JEEVESTEMPLATEDIR=.
JEEVES=perl -I.. -w ../Jeeves/jeeves -s SchemaParser 

TARGETS=RedHeader_perl.cxx RedHeader_perl.o \
	RedEvent_perl.cxx RedEvent_perl.o \
	HillasParam_perl.cxx HillasParam_perl.o \

OBJECTS=RedEvent.so RedHeader.so \
	VSFA_H5_perl.o VSFA_H5.so \
	RedFile_perl.o RedFile.so \
	HillasParam.so

all:	$(TARGETS) obj

obj:	$(OBJECTS)

RedHeader_perl.cxx: ../reduced.sc classperl.tpl
	$(JEEVES) -t classperl.tpl ../reduced.sc

RedEvent_perl.cxx: ../reduced.sc classperl.tpl
	$(JEEVES) -t classperl.tpl ../reduced.sc

HillasParam_perl.cxx: ../hillasparam.sc classperl.tpl
	$(JEEVES) -t classperl.tpl ../hillasparam.sc

%.o: %.cxx
	$(CXX) -c $(CXXFLAGS) $<

RedEvent.so: RedEvent_perl.o ../RedEvent.o
RedHeader.so: RedHeader_perl.o ../RedHeader.o
HillasParam.so: HillasParam_perl.o ../HillasParam.o

VSFA_H5_perl.o: VSFA_perl.h VSFA_H5_perl.cxx
VSFA_H5.so: VSFA_H5_perl.o ../RedEvent.o

RedFile_perl.o: ../RedEvent.h ../RedHeader.h ../RedFile.h RedFile_perl.cxx
RedFile.so: RedFile_perl.o ../RedHeader.o ../RedEvent.o

%.so:
	$(CXX) -shared -o $@ $^ -lhdf5 -lz

clean: 
	rm -f $(OBJECTS) $(TARGETS) \
		 RedHeader.pm RedEvent.pm \
		 classperl.tpl.pl \
		 *~ 
