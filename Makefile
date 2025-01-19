CC = g++
CFLAGS = -g -Wall -I/home/machine/miniconda3/include
LDFLAGS = -g -L/home/machine/miniconda3/lib
LDLIBS = -lcurl

backtester: backtester.o
backtester.o: backtester.cpp
