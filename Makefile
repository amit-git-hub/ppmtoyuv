# Makefile for Saliency using libsaliency.a

.IGNORE:

.KEEP_STATE:

.SUFFIXES: .o .cpp

.cpp.o: 
	$(CXX) $(CXXFLAGS) -c $< -o $@

CXX = clang++

CXXFLAGS := -std=c++14
CXXFLAGS += -g -Wall

BIN = ppmtoyuv

SRCS = $(wildcard *.cpp) 
OBJS = $(SRCS:.cpp=.o)

all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(BIN) $(OBJS)

clean:
	rm -rf $(OBJS) $(BIN) $(BIN).gdb


