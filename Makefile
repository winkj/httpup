name=httpup
version=0.4.0j

prefix=		/usr/local
bindir=		$(prefix)/bin
libdir=		$(prefix)/lib
includedir=	$(prefix)/include
mandir=		$(prefix)/man


CXX=		g++
CXXFLAGS=	-O2 -pipe -DMF_VERSION=\"${version}\"
CXXFLAGS+=	-g -Wall -Werror
LDFLAGS= 	-lcurl

INSTALL=	/usr/bin/install
STRIP=		/usr/bin/strip


OBJS= 	httpup.o \
	fileutils.o \
	argparser.o \
	md5.o \
	httpupargparser.o \
	configparser.o \
	main.o

# # Portability stuff.
CXXFLAGS+= 	 -Wno-strict-aliasing
# OBJS+=		 strtonum.o strlcpy.o strlcat.o fgetln.o

.c.o:
	$(CC) $(CFLAGS) -c $<

all: $(name)


$(name): $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) -o $(name)

distclean: clean
	-rm -f Makefile config.log config.h *~ *.core core.*

clean:
	-rm -f *.o $(name)


install: $(name) $(name).8
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL) -d $(DESTDIR)$(mandir)/man8
	$(INSTALL) -m 755 $(name)	$(DESTDIR)$(bindir)/$(name)
	$(INSTALL) -m 755 httpup-repgen	$(DESTDIR)$(bindir)/httpup-repgen
	$(INSTALL) -m 644 $(name).8	$(DESTDIR)$(mandir)/man8/$(name).8
	$(INSTALL) -m 644 httpup-repgen.8 \
		$(DESTDIR)$(mandir)/man8/httpup-repgen.8

install-strip: install
	$(STRIP) $(DESTDIR)$(bindir)/$(name)

uninstall:
	rm -f \
	$(DESTDIR)$(bindir)/$(name) \
	$(DESTDIR)$(mandir)/man1/$(name).8

rebuild:
	make clean all


dist:   all
	rm -rf $(name)-$(version)
	mkdir $(name)-$(version)
	cp *.cpp *.h Makefile AUTHORS COPYING ChangeLog README TODO *.8 \
		httpup-repgen* httpup.conf* $(name)-$(version)
	tar cvzf $(name)-$(version).tar.gz $(name)-$(version)
	rm -rf $(name)-$(version)
