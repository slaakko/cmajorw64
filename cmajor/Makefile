include Makefile.common

.PHONY : build install clean

all: build

build:
	$(MAKE) -C util
	$(MAKE) -C codedom
	$(MAKE) -C parsing
	$(MAKE) -C code
	$(MAKE) -C syntax
	$(MAKE) -C generator
	$(MAKE) -C xml
	$(MAKE) -C dom
	$(MAKE) -C xpath
	$(MAKE) -C xpq
	$(MAKE) -C ast
	$(MAKE) -C parser
	$(MAKE) -C symbols
	$(MAKE) -C ir
	$(MAKE) -C emitter
	$(MAKE) -C binder
	$(MAKE) -C build
	$(MAKE) -C compiler
	$(MAKE) -C cm2html
	$(MAKE) -C cmdoc
	$(MAKE) -C cmmdump
	$(MAKE) -C cmprof
	$(MAKE) -C cmunit
	$(MAKE) -C system/ext/gmp
	$(MAKE) -C rt

prefix := /usr

ifeq ($(config),debug)
    cmrtsoname=libcmrtd
else
    cmrtsoname=libcmrt
endif

install:
	mkdir -p $(prefix)/bin
	cp bin/* $(prefix)/bin
	cp lib/$(config)/$(cmrtsoname).so.2.1.0 $(prefix)/lib
	ln -f $(prefix)/lib/$(cmrtsoname).so.2.1.0 $(prefix)/lib/$(cmrtsoname).so.2
	ln -f $(prefix)/lib/$(cmrtsoname).so.2.1.0 $(prefix)/lib/$(cmrtsoname).so

clean:
	$(MAKE) -C util clean
	$(MAKE) -C codedom clean
	$(MAKE) -C parsing clean
	$(MAKE) -C code clean
	$(MAKE) -C syntax clean
	$(MAKE) -C generator clean
	$(MAKE) -C xml clean
	$(MAKE) -C dom clean
	$(MAKE) -C xpath clean
	$(MAKE) -C xpq clean
	$(MAKE) -C ast clean
	$(MAKE) -C parser clean
	$(MAKE) -C symbols clean
	$(MAKE) -C ir clean
	$(MAKE) -C emitter clean
	$(MAKE) -C binder clean
	$(MAKE) -C build clean
	$(MAKE) -C compiler clean
	$(MAKE) -C cm2html clean
	$(MAKE) -C cmdoc clean
	$(MAKE) -C cmmdump clean
	$(MAKE) -C cmprof clean
	$(MAKE) -C cmunit clean
	$(MAKE) -C system/ext/gmp clean
	$(MAKE) -C rt clean

