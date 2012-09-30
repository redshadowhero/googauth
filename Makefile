GCC=gcc
CFLAGS=-std=c99 -Wall -pedantic
EXECUTABLE=googauth
BINPATH=bin
VPATH=src
INCLUDE=-Iinclude
LIBS=

.SILENT:

all: $(EXECUTABLE)

googauth_debug: main.c google-authenticator.c hmac.c sha1.c base32.c
	$(GCC) -DRSHDEBUG -DLEVEL=13 $(CFLAGS) $(INCLUDE) $^ $(LIBS) -o $(BINPATH)/$@

$(EXECUTABLE): main.c google-authenticator.c hmac.c sha1.c base32.c
	$(GCC) $(CFLAGS) $(INCLUDE) $^ $(LIBS) -o $(BINPATH)/$@

.PHONY library: $(LIBNAME)

$(LIBNAME): 
	$(GCC) $(CFLAGS) $(INCLUDE) $^ -fPIC -shared -o $(LIBPATH)/$(LIBNAME)

clean:
	rm -f $(BINPATH)/$(EXECUTABLE) $(BINPATH)/googauth_debug
