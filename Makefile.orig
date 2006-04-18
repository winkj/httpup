all: httpup

############################################################################
###
## Configuration
#
NAME=httpup
VERSION="0.4.0h"
CXX=g++
CXXFLAGS=-Wall -ansi -pedantic -DMF_VERSION='${VERSION}' 
LDFLAGS=-lcurl

objects=httpupargparser.o argparser.o main.o httpup.o \
	fileutils.o md5.o configparser.o

httpupargparser.o: 	httpupargparser.cpp httpupargparser.h
argparser.o: 		argparser.cpp argparser.h
main.o: 		main.cpp
httpup.o:		httpup.cpp httpup.h
fileutils.o:		fileutils.cpp fileutils.h
md5.o:			md5.cpp md5.h
configparser.o:		configparser.cpp configparser.h



############################################################################
$(objects): %.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@


httpup: $(objects) *.cpp *.h
	g++ -o httpup $(objects) $(LDFLAGS)

clean:
	rm -f httpup $(objects)

dist:
	rm -rf ${NAME}-${VERSION}
	mkdir ${NAME}-${VERSION}
	cp *.cpp *.h Makefile AUTHORS COPYING ChangeLog README TODO *.8 \
		httpup-repgen* httpup.conf* ${NAME}-${VERSION}
	tar cvzf ${NAME}-${VERSION}.tar.gz ${NAME}-${VERSION}
	rm -rf ${NAME}-${VERSION}
