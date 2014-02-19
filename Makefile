AR = ar
CC = gcc
CXX = clang++ -stdlib=libc++ -std=gnu++0x

INCLUDES = -Imprpc -Imprpc/tamer
CPPFLAGS = $(INCLUDES) 
CXXFLAGS = -W -Wall -g -O2

TAMER = mprpc/tamer/compiler/tamer
LIBTAMER = mprpc/tamer/tamer/.libs/libtamer.a
LIBMPRPC = mprpc/json.o  mprpc/string.o mprpc/straccum.o  mprpc/str.o  mprpc/msgpack.o \
	 mprpc/clp.o mprpc/compiler.o mprpc/mpfd.o
LIBS = $(LIBMPRPC) $(LIBTAMER) `$(TAMER) -l`  -lpthread -lm

CXXCOMPILE = $(CXX) $(DEFS) $(CPPFLAGS) $(CXXFLAGS)
CXXLINK = $(CXX) $(CXXFLAGS)

DEPSDIR := .deps

%.o: $(DEPSDIR)/%.cc config.h $(DEPSDIR)/stamp
	$(CXXCOMPILE) $(DEPCFLAGS) -include config.h -c -o $@ $<

$(DEPSDIR)/%.cc: %.tcc config.h $(DEPSDIR)/stamp $(TAMER)
	$(TAMER) $(TAMERFLAGS) -F $(DEPSDIR)/$*.cc.d -o $@ $<

all: null.o
	$(CXX) $(CFLAGS) -o nulltest $^ $(LDFLAGS) $(LIBS)

paxos: paxos.o
	$(CXX) $(CFLAGS) -o paxos $^ $(LDFLAGS) $(LIBS)

message-size: message-size.o
	$(CXX) $(CFLAGS) -o message-size $^ $(LDFLAGS) $(LIBS)

windowed-message-size: windowed-message-size.o
	$(CXX) $(CFLAGS) -o windowed-message-size $^ $(LDFLAGS) $(LIBS)

multi-client: multi-client.o
	$(CXX) $(CFLAGS) -o multi-client $^ $(LDFLAGS) $(LIBS)