include ../../../Makefile.common

OBJECTS = gmpintf.o

%o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

%o: %.c
	$(C) -c $(CFLAGS) -o $@ $<

../../../lib/$(config)/libgmpintf.a: $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)

clean:
	rm -f *.o
	rm -f ../../../lib/$(config)/libgmpintf.a

