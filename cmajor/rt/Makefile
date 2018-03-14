include ../Makefile.common

OBJECTS = CallStack.o Classes.o Compression.o ConditionVariable.o Environment.o Error.o InitDone.o Io.o Math.o Memory.o \
Multiprecision.o Mutex.o Profile.o Random.o Socket.o Statics.o String.o Thread.o Time.o UnitTest.o BZ2Interface.o ZlibInterface.o

LIBRARIES = ../lib/$(config)/libutil.a ../lib/$(config)/libcodedom.a ../lib/$(config)/libparsing.a ../lib/$(config)/libxpath.a ../lib/$(config)/libdom.a ../lib/$(config)/libxml.a \
../lib/$(config)/libast.a ../lib/$(config)/libsymbols.a ../lib/$(config)/libparser.a

%o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

%o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

ifeq ($(config),debug)
    soname=libcmrtd.so.2.1.0
    sobasename=libcmrtd
else
    soname=libcmrt.so.2.1.0
    sobasename=libcmrt
endif

../lib/$(config)/$(soname): $(OBJECTS) $(LIBRARIES)
	$(CXX) $(CXXFLAGS) -shared -o $@  $(OBJECTS) -lsymbols -lparser -last -lxpath -ldom -lxml -lparsing -lcodedom -lutil -ldl -ltinfo -lgnutls
	ln -f ../lib/$(config)/$(soname) ../lib/$(config)/$(sobasename).so.2
	ln -f ../lib/$(config)/$(soname) ../lib/$(config)/$(sobasename).so

clean:
	rm -f *.o
	rm -f ../lib/$(config)/$(soname)

