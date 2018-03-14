include ../Makefile.common

OBJECTS = XmlContentHandler.o XmlGrammar.o XmlParser.o XmlProcessor.o

%o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

../lib/$(config)/libxml.a: $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)

clean:
	rm -f *.o
	rm -f ../lib/$(config)/libxml.a

