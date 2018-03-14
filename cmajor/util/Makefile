include ../Makefile.common

OBJECTS = BinaryReader.o BinaryWriter.o CodeFormatter.o InitDone.o Json.o MappedInputFile.o MemoryReader.o Mutex.o Path.o Prime.o Random.o Sha1.o \
System.o TextUtils.o Time.o Unicode.o

%o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

../lib/$(config)/libutil.a: $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)

clean:
	rm -f *.o
	rm -f ../lib/$(config)/libutil.a

