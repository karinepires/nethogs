VERSION      := 0
SUBVERSION   := 8
MINORVERSION := 0

#DESTDIR := /usr
DESTDIR := /usr/local

sbin  := $(DESTDIR)/sbin
man8 := $(DESTDIR)/share/man/man8/

all: nethogs decpcap_test
# nethogs_testsum

CFLAGS=-g -Wall -Wextra
#CFLAGS=-O2
OBJS=packet.o connection.o process.o refresh.o decpcap.o cui.o inode2prog.o conninode.o devices.o
.PHONY: tgz

tgz: clean
	cd .. ; tar czvf nethogs-$(VERSION).$(SUBVERSION).$(MINORVERSION).tar.gz --exclude-vcs nethogs/*

.PHONY: check
check:
	echo "Not implemented"

install: nethogs nethogs.8
	install -d -m 755 $(sbin)
	install -m 755 nethogs $(sbin)
	install -d -m 755 $(man8)
	install -m 644 nethogs.8 $(man8)

nethogs: nethogs.cpp $(OBJS)
	$(CXX) $(CFLAGS) nethogs.cpp $(OBJS) -o nethogs -lpcap -lm -lncurses -DVERSION=\"$(VERSION)\" -DSUBVERSION=\"$(SUBVERSION)\" -DMINORVERSION=\"$(MINORVERSION)\"
nethogs_testsum: nethogs_testsum.cpp $(OBJS)
	$(CXX) $(CFLAGS) -g nethogs_testsum.cpp $(OBJS) -o nethogs_testsum -lpcap -lm -lncurses -DVERSION=\"$(VERSION)\" -DSUBVERSION=\"$(SUBVERSION)\" -DMINORVERSION=\"$(MINORVERSION)\"

decpcap_test: decpcap_test.cpp decpcap.o
	$(CXX) $(CFLAGS) decpcap_test.cpp decpcap.o -o decpcap_test -lpcap -lm

#-lefence

refresh.o: refresh.cpp refresh.h nethogs.h
	$(CXX) $(CFLAGS) -c refresh.cpp
process.o: process.cpp process.h nethogs.h
	$(CXX) $(CFLAGS) -c process.cpp
packet.o: packet.cpp packet.h nethogs.h
	$(CXX) $(CFLAGS) -c packet.cpp
connection.o: connection.cpp connection.h nethogs.h
	$(CXX) $(CFLAGS) -c connection.cpp
decpcap.o: decpcap.c decpcap.h
	$(CC) $(CFLAGS) -c decpcap.c
inode2prog.o: inode2prog.cpp inode2prog.h nethogs.h
	$(CXX) $(CFLAGS) -c inode2prog.cpp
conninode.o: conninode.cpp nethogs.h conninode.h
	$(CXX) $(CFLAGS) -c conninode.cpp
#devices.o: devices.cpp devices.h
#	$(CXX) $(CFLAGS) -c devices.cpp
cui.o: cui.cpp cui.h nethogs.h
	$(CXX) $(CFLAGS) -c cui.cpp -DVERSION=\"$(VERSION)\" -DSUBVERSION=\"$(SUBVERSION)\" -DMINORVERSION=\"$(MINORVERSION)\"

.PHONY: clean
clean:
	rm -f $(OBJS)
	rm -f nethogs
