GCC=gcc
CFLAGS=-std=c99 -Wall -pedantic
EXECUTABLE=googauth
BINPATH=bin
VPATH=src
INCLUDE=-Iinclude
STDARGS=$(CFLAGS) $(INCLUDE) $^ $(LIBS) -o $(BINPATH)/$@

.SILENT:

all: $(EXECUTABLE)

$(EXECUTABLE)_debug: main.c google-authenticator.c hmac.c sha1.c base32.c
	$(GCC) -g $(STDARGS)

$(EXECUTABLE): main.c google-authenticator.c hmac.c sha1.c base32.c
	$(GCC) $(STDARGS)

$(EXECUTABLE)_win32: GCC = i586-mingw32msvc-gcc
$(EXECUTABLE)_win32: main.c google-authenticator.c hmac.c sha1.c base32.c
	$(GCC) -DLINUX_HOST $(STDARGS).exe

clean:
	rm -f $(BINPATH)/*
