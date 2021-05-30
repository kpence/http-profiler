HOST_SYSTEM = $(shell uname | cut -f 1 -d_)
SYSTEM ?= $(HOST_SYSTEM)
CXX = g++
CPPFLAGS += `pkg-config --cflags openssl` -g -std=c++1y
CXXFLAGS += -std=c++1y -g
LDFLAGS += -L /usr/local/lib `pkg-config --libs openssl` -g -std=c++1y
all: httprofile

coverage: CPPFLAGS += -fprofile-arcs -ftest-coverage
coverage: LDFLAGS += -fprofile-arcs -ftest-coverage
coverage: httprofile

%.o : %.cc
	$(CXX) $^ -o $@ -c $(CPPFLAGS)

httprofile: main.o pch.o socket.o helpers.o urlparser.o httpprofiler.o
	$(CXX) $^ -o $@ $(LDFLAGS)



clean:
	rm -f *.o httprofile
