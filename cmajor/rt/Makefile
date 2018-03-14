include ../Makefile.common

OBJECTS = CallStack.o Classes.o Compression.o ConditionVariable.o Environment.o Error.o InitDone.o Io.o Math.o Memory.o \
Multiprecision.o Mutex.o Profile.o Random.o Socket.o Statics.o String.o Thread.o Time.o UnitTest.o BZ2Interface.o ZlibInterface.o

LIBRARIES = ../lib/libutil.a ../lib/libcodedom.a ../lib/libparsing.a ../lib/libxpath.a ../lib/libdom.a ../lib/libxml.a \
../lib/libast.a ../lib/libsymbols.a ../lib/libparser.a ../lib/libeh.a

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
	$(CXX) $(CXXFLAGS) -Wl,--dynamic-list=export.txt -shared -o $@ $(OBJECTS) -lsymbols -lparser -last -leh -lxpath -ldom -lxml -lparsing -lcodedom -lutil -ldl -ltinfo -lgnutls
	ln -f ../lib/$(soname) ../lib/$(sobasename).so.2
	ln -f ../lib/$(soname) ../lib/$(sobasename).so

clean:
	rm -f *.o
	rm -f ../lib/$(soname)

