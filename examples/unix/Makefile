CXX = g++
CPPFLAGS = -std=c++20 -Wall
CPPFLAGS += -g -fsanitize=address -fno-omit-frame-pointer
SRCDIR = ../..
IFLAGS = -I$(SRCDIR)
SOURCES = $(wildcard $(SRCDIR)/http2/*.cpp)
SOURCES += $(wildcard $(SRCDIR)/grpc/*.cpp)
SOURCES += $(wildcard $(SRCDIR)/utils/*.cpp)
SOURCES += $(wildcard $(SRCDIR)/unix/*.cpp)
SOURCES += $(wildcard $(SRCDIR)/examples/gen/*.c)

PLATFORM=$(shell uname -s | tr '[:upper:]' '[:lower:]')
ifeq ($(PLATFORM),darwin)
	IFLAGS+=-I/opt/homebrew/opt/boost/include
endif

clean:
	
build_server: server.cpp
	mkdir -p bin
	$(CXX) $(CPPFLAGS) -o bin/server server.cpp $(SOURCES) $(IFLAGS) $(LDFLAGS) $(GRPCFLAGS) $(OTHER)

run_server: build_server
	./bin/server
