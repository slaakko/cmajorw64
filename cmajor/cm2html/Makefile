include ../Makefile.common

OBJECTS = Main.o

LIBRARIES = ../lib/$(config)/libutil.a ../lib/$(config)/libcodedom.a ../lib/$(config)/libparsing.a ../lib/$(config)/libxpath.a ../lib/$(config)/libdom.a ../lib/$(config)/libxml.a \
../lib/$(config)/libast.a ../lib/$(config)/libsymbols.a ../lib/$(config)/libparser.a

%o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

ifeq ($(config),debug)
    exename=cm2htmld
else
    exename=cm2html
endif

../bin/$(exename): $(OBJECTS) $(LIBRARIES)
	$(CXX) $(CXXFLAGS) -o ../bin/$(exename) $(OBJECTS) -lsymbols -lparser -last \
-lxpath -ldom -lxml -lparsing -lcodedom -lutil \
$(LLVMLIBS) -lboost_filesystem -lboost_iostreams -lboost_system -ldl -ltinfo -lz

clean:
	rm -f *.o
	rm -f ../bin/$(exename)

