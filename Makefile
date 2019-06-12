#CC=gcc32
#CXX=g++32
#CC=icc
#CXX=icc
LIBS=-lhdf5 -lz -lodbc -lodbc++ -lm
CFLAGS=-Wall -g -I/usr/local/include -fPIC
SLOWCXXFLAGS=-Wall -I$(QTDIR)/include -g -I/usr/local/include -fPIC
CXXFLAGS=$(SLOWCXXFLAGS)
MOC=$(QTDIR)/bin/moc

#CC=/usr/local/intel/bin/icc
#CXX=/usr/local/intel/bin/icc
#LIBS=-lhdf5 -lz -lodbc -lodbc++ -lm
#CFLAGS=#-tpp6 -O2
#SLOWCXXFLAGS=-O0 -I/usr/local/include -I/usr/local/intel/include -I/usr/local/qt/include 
#CXXFLAGS=$(SLOWCXXFLAGS) #-tpp6 -O2 -Kc++
#MOC=/usr/local/qt/bin/moc

export JEEVESLIBDIR=Jeeves/lib
export JEEVESTEMPLATEDIR=.
JEEVES=perl -w Jeeves/jeeves -s SchemaParser 

TARGETS=BaseCutsHillasParam.sc BaseCutsHillasParam.h BaseCutsHillasParam.cxx \
	CutsHillasParam.h CutsHillasParam.cxx \
	BaseCutsEventInfo.sc BaseCutsEventInfo.h BaseCutsEventInfo.cxx \
	CutsEventInfo.h CutsEventInfo.cxx \
	HillasParam.h HillasParam.cxx \
	EventInfo.h EventInfo.cxx \
	RedHeader.h RedHeader.cxx \
	RedEvent.h RedEvent.cxx \
	StorableChannelMask.h StorableChannelMask.cxx \
	StorableChannelDescription.h StorableChannelDescription.cxx \
	StorableNeighborVertex.h StorableNeighborVertex.cxx \
	StorableCameraConfiguration.h 	StorableCameraConfiguration.cxx

OBJECTS=Exceptions.o \
	BaseCutsHillasParam.o CutsHillasParam.o \
	BaseCutsEventInfo.o CutsEventInfo.o \
	HillasParam.o EventInfo.o \
	RedHeader.o RedEvent.o \
	StorableChannelMask.o \
	StorableChannelDescription.o StorableNeighborVertex.o \
	StorableCameraConfiguration.o \
	cH5Utils.o ProgressBar.o RedFile.o ParamFile.o \
	ParamFileGenerator.o TwoDimensionalHistGenerator.o \
	ChannelData.o CameraConfiguration.o \
	Random.o Binner.o ADCSpectrum.o \
	PedsAndGainsBase.o Pedestals.o Gains.o \
	PedestalsCalc.o GainsCalc.o \
	PedsAndGainsFactory.o \
	ODBCPedsAndGainsFactory.o \
	Cleaning.o ChannelRepresentation.o \
	Trigger.o \
	HillasParameterization.o TwoDimensionalParameterization.o \
	AlphaPlotGenerator.o CutParamFile.o \
	UtilityFunctions.o StringTokenizer.o ScriptParser.o

QTSOURCES=QCamera_moc.cxx QEventCamera_moc.cxx QParamCamera_moc.cxx

QTOBJECTS=QAxisScale.o \
	  QCamera.o QCamera_moc.o \
	  QEventCamera.o QEventCamera_moc.o \
	  QParamCamera.o QParamCamera_moc.o

all:	$(TARGETS) obj libAnalysis.so libAnalysis.a bins

obj:	$(OBJECTS)

qt:	all $(QTSOURCES) $(QTOBJECTS) libQComponents.so libQComponents.a \
        qtbins

###############################################################################
########################### JEEVES CODE GENERATION ############################
###############################################################################

BaseCutsHillasParam.sc: hillasparam.sc basecutsc.tpl
	$(JEEVES) -t basecutsc.tpl hillasparam.sc

BaseCutsHillasParam.h: BaseCutsHillasParam.sc classdef.tpl
	$(JEEVES) -t classdef.tpl BaseCutsHillasParam.sc

BaseCutsHillasParam.cxx: BaseCutsHillasParam.sc classcode.tpl
	$(JEEVES) -t classcode.tpl BaseCutsHillasParam.sc

CutsHillasParam.h: hillasparam.sc cutsclassdef.tpl
	$(JEEVES) -t cutsclassdef.tpl hillasparam.sc

CutsHillasParam.cxx: hillasparam.sc cutsclassdef.tpl
	$(JEEVES) -t cutsclassdef.tpl hillasparam.sc

BaseCutsEventInfo.sc: hillasparam.sc basecutsc.tpl
	$(JEEVES) -t basecutsc.tpl hillasparam.sc

BaseCutsEventInfo.h: BaseCutsEventInfo.sc classdef.tpl
	$(JEEVES) -t classdef.tpl BaseCutsEventInfo.sc

BaseCutsEventInfo.cxx: BaseCutsEventInfo.sc classcode.tpl
	$(JEEVES) -t classcode.tpl BaseCutsEventInfo.sc

CutsEventInfo.h: hillasparam.sc cutsclassdef.tpl
	$(JEEVES) -t cutsclassdef.tpl hillasparam.sc

CutsEventInfo.cxx: hillasparam.sc cutsclassdef.tpl
	$(JEEVES) -t cutsclassdef.tpl hillasparam.sc

HillasParam.h: hillasparam.sc classdef.tpl
	$(JEEVES) -t classdef.tpl hillasparam.sc

HillasParam.cxx: hillasparam.sc classcode.tpl
	$(JEEVES) -t classcode.tpl hillasparam.sc

EventInfo.h: hillasparam.sc classdef.tpl
	$(JEEVES) -t classdef.tpl hillasparam.sc

EventInfo.cxx: hillasparam.sc classcode.tpl
	$(JEEVES) -t classcode.tpl hillasparam.sc

RedHeader.h: reduced.sc classdef.tpl
	$(JEEVES) -t classdef.tpl reduced.sc

RedHeader.cxx: reduced.sc classcode.tpl
	$(JEEVES) -t classcode.tpl reduced.sc

StorableChannelMask.h: camera.sc classdef.tpl
	$(JEEVES) -t classdef.tpl camera.sc

StorableChannelMask.cxx: camera.sc classcode.tpl
	$(JEEVES) -t classcode.tpl camera.sc

StorableChannelDescription.h: camera.sc classdef.tpl
	$(JEEVES) -t classdef.tpl camera.sc

StorableChannelDescription.cxx: camera.sc classcode.tpl
	$(JEEVES) -t classcode.tpl camera.sc

StorableNeighborVertex.h:  camera.sc classdef.tpl
	$(JEEVES) -t classdef.tpl camera.sc

StorableNeighborVertex.cxx:  camera.sc classcode.tpl
	$(JEEVES) -t classdef.tpl camera.sc

StorableCameraConfiguration.h:  camera.sc classdef.tpl
	$(JEEVES) -t classdef.tpl camera.sc

StorableCameraConfiguration.cxx:  camera.sc classcode.tpl
	$(JEEVES) -t classdef.tpl camera.sc

###############################################################################
############# SLOW TO COMPILE CODE THAT DOES NOT NEED OPTIMIZATION ############
###############################################################################

BaseCutsHillasParam.o: BaseCutsHillasParam.cxx
	$(CXX) $(SLOWCXXFLAGS) -c $<

CutsHillasParam.o: CutsHillasParam.cxx
	$(CXX) $(SLOWCXXFLAGS) -c $<

BaseCutsEventInfo.o: BaseCutsEventInfo.cxx
	$(CXX) $(SLOWCXXFLAGS) -c $<

CutsEventInfo.o: CutsEventInfo.cxx
	$(CXX) $(SLOWCXXFLAGS) -c $<

HillasParam.o: HillasParam.cxx
	$(CXX) $(SLOWCXXFLAGS) -c $<

EventInfo.o: EventInfo.cxx
	$(CXX) $(SLOWCXXFLAGS) -c $<

RedEvent.o: RedEvent.cxx
	$(CXX) $(SLOWCXXFLAGS) -c $<

RedHeader.o: RedHeader.cxx
	$(CXX) $(SLOWCXXFLAGS) -c $<

StorableChannelMask.o: StorableChannelMask.cxx
	$(CXX) $(SLOWCXXFLAGS) -c $<

StorableChannelDescription.o: StorableChannelDescription.cxx
	$(CXX) $(SLOWCXXFLAGS) -c $<

StorableNeighborVertex.o: StorableNeighborVertex.cxx
	$(CXX) $(SLOWCXXFLAGS) -c $<

StorableCameraConfiguration.o: StorableCameraConfiguration.cxx
	$(CXX) $(SLOWCXXFLAGS) -c $<

###############################################################################

Exceptions.o: Exceptions.h Types.h

ProgressBar.o: ProgressBar.cxx ProgressBar.h

cH5Utils.o: cH5Utils.h

RedFile.o: RedFile.cxx cH5Utils.h VSFA_cH5.h ProgressBar.h

ParamFile.o: ParamFile.cxx ParamFile.h cH5Utils.h VSFA_cH5.h ProgressBar.h

ParamFileGenerator.o: ParamFileGenerator.cxx ParamFile.h cH5Utils.h \
                      VSFA_cH5.h ProgressBar.h

TwoDimensionalHistGenerator.o: TwoDimensionalHistGenerator.cxx \
	TwoDimensionalParameterization.h Binner.h ProgressBar.h

RedEvent.h: reduced.sc classdef.tpl
RedEvent.cxx: reduced.sc classcode.tpl

ChannelData.o: ChannelData.cxx ChannelData.h

CameraConfiguration.o: CameraConfiguration.cxx CameraConfiguration.h \
                       StorableCameraConfiguration.h \
                       StorableChannelDescription.h StorableNeighborVertex.h \
                       cH5CameraConf.h

Random.o: Random.cxx Random.h

Binner.o: Binner.cxx Binner.h

ADCSpectrum.o: ADCSpectrum.cxx ADCSpectrum.h RedFile.h RedEvent.h

PedestalsCalc.o: PedestalsCalc.cxx PedestalsCalc.h Pedestals.h \
             ChannelData.h CameraConfiguration.h \
	     ProgressBar.h Binner.h ADCSpectrum.h RedFile.h RedEvent.h

GainsCalc.o: GainsCalc.cxx GainsCalc.h Pedestals.h \
             ChannelData.h CameraConfiguration.h \
	     ProgressBar.h RedFile.h RedEvent.h

PedsAndGainsBase.o: PedsAndGainsBase.cxx PedsAndGainsBase.h ChannelData.h

Pedestals.o: Pedestals.cxx Pedestals.h PedsAndGainsBase.h ChannelData.h

Gains.o: Gains.cxx Gains.h PedsAndGainsBase.h ChannelData.h

PedsAndGainsFactory.o: PedsAndGainsFactory.cxx PedsAndGainsFactory.h

ODBCPedsAndGainsFactory.o: ODBCPedsAndGainsFactory.cxx \
             ODBCPedsAndGainsFactory.h PedsAndGainsFactory.h

Cleaning.o: Cleaning.cxx Cleaning.h CameraConfiguration.h 

Trigger.o: Trigger.cxx Trigger.h  CameraConfiguration.h 

ChannelRepresentation.o: ChannelRepresentation.cxx ChannelRepresentation.h \
			 CameraConfiguration.h Cleaning.h Pedestals.h

HillasParameterization.o: HillasParameterization.cxx HillasParameterization.h \
			  HillasParam.h Binner.h

TwoDimensionalParameterization.o: TwoDimensionalParameterization.cxx \
		TwoDimensionalParameterization.h Binner.h \
		HillasParam.h

AlphaPlotGenerator.o: AlphaPlotGenerator.cxx AlphaPlotGenerator.h \
		      HillasParam.h Binner.h

CurParamFile.o: CurParamFile.cxx CurParamFile.h ParamFile.h\
		   HillasParam.h

Pedestals.o: Pedestals.cxx Pedestals.h 

QAxisScale.o: QAxisScale.cxx QAxisScale.h

VQCamera_moc.cxx: VQCamera.h
	$(MOC) -o $@ $<

VQCamera.o: VQCamera.cxx VQCamera.h

QCamera_moc.cxx: QCamera.h
	$(MOC) -o $@ $<

QCamera.o: QCamera.cxx QCamera.h

QEventCamera_moc.cxx: QEventCamera.h
	$(MOC) -o $@ $<

QEventCamera.o: QEventCamera.cxx QEventCamera.h QCamera.h

QParamCamera_moc.cxx: QParamCamera.h
	$(MOC) -o $@ $<

QParamCamera.o: QParamCamera.cxx QParamCamera.h QCamera.h

UtilityFunctions.o: UtilityFunctions.cxx UtilityFunctions.h

StringTokenizer.o: StringTokenizer.cxx StringTokenizer.h

ScriptParser.o: ScriptParser.cxx ScriptParser.h

libAnalysis.so: $(OBJECTS)
	$(CXX) -shared -o $@ $^ 

libAnalysis.a: $(OBJECTS)
	ar r $@ $^


libQComponents.so: $(QTOBJECTS)
	$(CXX) -shared -o $@ $^ $(LIBS)

libQComponents.a: $(QTOBJECTS)
	ar r $@ $^

bins: libAnalysis.so
	$(MAKE) -C bin

qtbins: libQComponents.so
	$(MAKE) -C bin qt

%.o: %.cxx
	$(CXX) $(CXXFLAGS) -c $<

clean: 
	rm -f $(OBJECTS) $(TARGETS) $(QTSOURCES) $(QTOBJECTS) \
		 classdef.tpl.pl classcode.tpl.pl \
		 basecutsc.tpl.pl cutsclassdef.tpl.pl \
		 libAnalysis.{a,so} libQComponents.{a,so} \
		 *~ 
	$(MAKE) -C bin clean

emacs:
	xemacs *.sc Makefile *.{cxx,h} *.tpl \
		bin/Makefile bin/*.{cxx,h} \
		&
