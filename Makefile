
CC?=gcc
CXX?=g++
PREFIX?=/usr/local
CXXFLAGS+=-Wall -Werror -Ithirdparty
LDFLAGS+=-lmodbus

all: ek9k-ctl

clean:
	rm -rf bin

ek9k-ctl: builddir 
	$(CXX) $(CXXFLAGS) -o bin/ek9k-ctl $(wildcard src/*.cpp) $(LDFLAGS)

builddir:
	@mkdir -p bin

install: ek9k-ctl
	install -m 777 bin/ek9k-ctl "$(PREFIX)/bin/ek9k-ctl"
