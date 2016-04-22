CXXFLAGS=-std=c++14
LDLIBS=-lstdc++ -lpthread -lunistring

data_analyzer: data_analyzer.o stemwords.o snowball/libstemmer.o

data_analyzer.o: data_analyzer.cc stemwords.h 

snowball/libstemmer.o:
	$(MAKE) -C libstemmer