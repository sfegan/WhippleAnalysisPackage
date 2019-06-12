CC=g++
LD=g++
CXX=g++
LIBS=-lAnalysis -lhdf5 -lz -lm -lodbc++ -lodbc -lsla -lpthread -ldl
CFLAGS=-g
SLOWCXXFLAGS=-I..
LDFLAGS=-L/usr/local/lib -L..
CXXFLAGS=$(SLOWCXXFLAGS) -g #-O2

#CC=/usr/local/intel/bin/icc
#CXX=/usr/local/intel/bin/icc
#LIBS=-lAnalysis -lhdf5 -lz -lm -lodbc -lodbc++ -lsla -lstdc++
#CFLAGS=#-tpp6 -O2
#SLOWCXXFLAGS=-I/usr/local/include -I/usr/local/intel/include -I/usr/local/qt/include -I..
#LDFLAGS=-L.. $(LIBS)
#CXXFLAGS=$(SLOWCXXFLAGS) -O0 #-tpp6 -O2 -Kc++

QTLIBS=-lqwt -lqt -lQComponents
QTLDFLAGS=-L$(QTDIR)/lib $(QTLIBS)
QTCXXFLAGS=-I$(QTDIR)/include
MOC=$(QTDIR)/bin/moc
UIC=$(QTDIR)/bin/uic

TARGETS=red2h5 gcpeds gn2gains gparamdat gcut gdistill goptomize2d ghist2d \
	dumpparams adcspectra fzdump skygains analyzepst writecams \
	fz2h5 mkredscript

#TARGETS=red2h5 gcpeds gn2gains gparamdat gcut gdistill goptomize2d ghist2d \
#	dumpparams adcspectra timesfft fzdump skygains analyzepst writecams \
#	fz2h5 mkredscript

QTTARGETS=qteventviewer qtcameraview

all: $(TARGETS)

qt: all $(QTTARGETS)

writecams: writecams.o

writecams.o: writecams.cxx ../WhippleCams.h

red2h5: red2h5.o f77redfile.o
	$(CXX) -o $@ $(LDFLAGS) $^ $(LIBS)

red2h5.o: red2h5.cxx f77redfile.h

f77redfile.o: f77redfile.cxx f77redfile.h

gcpeds: gcpeds.o
	$(CXX) -o $@ $(LDFLAGS) $^ $(LIBS)

gcpeds.o: gcpeds.cxx

gn2gains: gn2gains.o
	$(CXX) -o $@ $(LDFLAGS) $^ $(LIBS)

gn2gains.o: gn2gains.cxx

gparamdat: gparamdat.o
	$(CXX) -o $@ $(LDFLAGS) $^ $(LIBS)

gparamdat.o: gparamdat.cxx

adcspectra: adcspectra.o
	$(CXX) -o $@ $(LDFLAGS) $^ $(LIBS)

adcspectra.o: adcspectra.cxx

timesfft: timesfft.o
	$(CXX) $(LDFLAGS) -o $@ $< -lrfftw -lfftw 

timesfft.o: timesfft.cxx

gcut: gcut.o
	$(CXX) -o $@ $(LDFLAGS) $^ $(LIBS)

gcut.o: gcut.cxx

dumpparams: dumpparams.o
	$(CXX) -o $@ $(LDFLAGS) $^ $(LIBS)

dumpparams.o: dumpparams.cxx

gdistill: gdistill.o
	$(CXX) -o $@ $(LDFLAGS) $^ $(LIBS)

gdistill.o: gdistill.cxx

goptomize2d: goptomize2d.o
	$(CXX) -o $@ $(LDFLAGS) $^ $(LIBS)

goptomize2d.o: goptomize2d.cxx

ghist2d: ghist2d.o
	$(CXX) -o $@ $(LDFLAGS) $^ $(LIBS)

ghist2d.o: ghist2d.cxx

qtcameraview: qtcameraview.o
	$(CXX) $(LDFLAGS) $(QTLDFLAGS) -o $@ $^ $(LIBS)

qtcameraview.o: qtcameraview.cxx qtcameraview_moc.cxx
	$(CXX) $(CXXFLAGS) $(QTCXXFLAGS) -c $<

qtcameraview_moc.cxx: qtcameraview.cxx
	$(MOC) -o $@ $^

qteventviewer: qteventviewer.o qteventviewer_loadfile.o \
	       qteventviewer_loadfile_base.o 
	$(CXX) $(LDFLAGS) $(QTLDFLAGS) -o $@ $^ $(LIBS) 

qteventviewer.o: qteventviewer.cxx qteventviewer_moc.cxx \
		 qteventviewer_loadfile_base.h
	$(CXX) $(CXXFLAGS) $(QTCXXFLAGS) -c $< $(LIBS)

qteventviewer_moc.cxx: qteventviewer.cxx
	$(MOC) -o $@ $^

qteventviewer_loadfile_base.h: qteventviewer_loadfile.ui
	$(UIC) $^ > $@

qteventviewer_loadfile_base.cxx: qteventviewer_loadfile_base.h \
				 qteventviewer_loadfile.ui
	$(UIC) -impl $^ > $@

qteventviewer_loadfile_base_moc.cxx: qteventviewer_loadfile_base.h
	$(MOC) -o $@ $^

qteventviewer_loadfile_base.o: qteventviewer_loadfile_base.cxx
	$(CXX) $(CXXFLAGS) $(QTCXXFLAGS) -c $<

qteventviewer_loadfile_moc.cxx: qteventviewer_loadfile.h
	$(MOC) -o $@ $^

qteventviewer_loadfile.o: qteventviewer_loadfile.cxx \
			       qteventviewer_loadfile_moc.cxx \
			       qteventviewer_loadfile_base.h \
			       qteventviewer_loadfile_base_moc.cxx
	$(CXX) $(CXXFLAGS) $(QTCXXFLAGS) -c $<

mkredscript: mkredscript.o
	$(CXX) -o $@ $(LDFLAGS) $^ $(LIBS)

mkredscript.o: mkredscript.cxx

GDFCXXFLAGS=-Igdf
GDFLDFLAGS=-z muldefs -Lcern/99/lib -Lgdf 
GDFLIBS=-lgdf -lmathlib -lpacklib -lkernlib -lfio -lf77math -lf90math -lg2c

fz.o: fz.cxx fz.h
	$(CXX) $(CXXFLAGS) $(GDFCXXFLAGS) -c $<

fzdump: fzdump.o fz.o ../Exceptions.o
	$(CXX) $(GDFLDFLAGS) -o $@ $^ $(GDFLIBS) -lm $(LIBS)

fzdump.o: fzdump.cxx
	$(CXX) $(CXXFLAGS) $(GDFCXXFLAGS) -c $<

analyzepst: analyzepst.o fz.o ../Exceptions.o
	$(CXX) $(GDFLDFLAGS) -o $@ $^ $(GDFLIBS) -lm $(LIBS)

analyzepst.o: analyzepst.cxx
	$(CXX) $(CXXFLAGS) $(GDFCXXFLAGS) -c $<

skygains: skygains.o fz.o ../Exceptions.o
	$(CXX) $(GDFLDFLAGS) -o $@ $^ $(GDFLIBS) -lm $(LIBS)

skygains.o: skygains.cxx
	$(CXX) $(CXXFLAGS) $(GDFCXXFLAGS) -c $<

fz2h5: fz2h5.o fz.o
	$(CXX) $(LDFLAGS) $(GDFLDFLAGS) -o $@ $^ $(GDFLIBS) $(LIBS)


fz2h5.o: fz2h5.cxx
	$(CXX) $(CXXFLAGS) $(GDFCXXFLAGS) -c $<

sim2h5: sim2h5.o
	$(CXX) -o $@ -L/home/sfegan/Devel/sim/lib -I/home/sfegan/Devel/sim/include $< $(LDFLAGS) -lannotate -lsimtohdf5 -lRandomNumbers

sim2h5.o: sim2h5.cxx
	$(CXX) -c $(CXXFLAGS) -I/home/sfegan/Devel/sim/include $<

%.o: %.cxx
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(TARGETS) $(QTTARGETS) *_moc.cxx *.o *~ core \
		qteventviewer_loadfile_base.cxx qteventviewer_loadfile_base.h

fz2h5-static: fz2h5.o fz.o
	$(CXX) -L.. $(GDFLDFLAGS) -o$@ $^ $(GDFLIBS) -lm /usr/local/lib/libhdf5.a -lz ../Exceptions.o ../RedHeader.o ../RedEvent.o ../RedFile.o ../UtilityFunctions.o ../ScriptParser.o ../cH5Utils.o ../StringTokenizer.o
