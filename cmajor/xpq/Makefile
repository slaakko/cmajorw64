include ../Makefile.common

OBJECTS = main.o

LIBRARIES = ../lib/$(config)/libutil.a ../lib/$(config)/libcodedom.a ../lib/$(config)/libparsing.a ../lib/$(config)/libxpath.a ../lib/$(config)/libdom.a ../lib/$(config)/libxml.a

%o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

ifeq ($(config),debug)
    exename=xpqd
else
    exename=xpq
endif

../bin/$(exename): $(OBJECTS) $(LIBRARIES)
	$(CXX) $(CXXFLAGS) -o ../bin/$(exename) $(OBJECTS) -lxpath -ldom -lxml -lparsing -lcodedom -lutil \
-lboost_filesystem -lboost_iostreams -lboost_system

clean:
	rm -f *.o
	rm -f ../bin/$(exename)

