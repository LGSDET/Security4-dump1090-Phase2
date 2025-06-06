CFLAGS?=-O2 -g -Wall -W $(shell pkg-config --cflags librtlsdr)
LDLIBS+=$(shell pkg-config --libs librtlsdr) -lpthread -lm
#LG_SECURITY_ENHANCEMENT
LDLIBS+=-lssl -lcrypto
CC?=gcc
PROGNAME=dump1090

all: dump1090 enlog_viewer

%.o: %.c
	$(CC) $(CFLAGS) -c $<

dump1090: dump1090.o anet.o tserver.o enlog.o
	$(CC) -g -o dump1090 dump1090.o anet.o tserver.o enlog.o $(LDFLAGS) $(LDLIBS)

tserver.o: TLSsample/tserver.c TLSsample/tls.h
	$(CC) $(CFLAGS) -c $<

dump1090.o: TLSsample/tls.h

enlog_test_suite: enlog_test enlog_viewer

enlog_test: enlog_test.o enlog.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

enlog.o: enlog.c enlog.h
	$(CC) $(CFLAGS) -c -o $@ $<

#enlog_test.o: enlog.c enlog.h
#	$(CC) -DENLOG_TEST $(CFLAGS) -c -o $@ $<

enlog_viewer: enlog_viewer.o enlog.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LDLIBS)

enlog_viewer.o: enlog_viewer.c enlog.h
	$(CC) $(CFLAGS) -c -o $@ $<

lgess2025s4rpilogkey:
	openssl rand -hex 32 > lgess2025s4rpilogkey.hex

distclean: clean
	rm -f dump1090 enlog_test enlog_viewer
clean:
	rm -f *.o *.log*
