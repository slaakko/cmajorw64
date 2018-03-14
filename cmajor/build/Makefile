include ../Makefile.common

OBJECTS = Build.o

%o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

../lib/$(config)/libbuild.a: $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)

clean:
	rm -f *.o
	rm -f ../lib/$(config)/libbuild.a

