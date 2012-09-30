GCC=gcc
CFLAGS=-std=c99 -Wall -pedantic
EXECUTABLE=googauth
BINPATH=bin
VPATH=src
INCLUDE=-Iinclude

.SILENT:

all: $(EXECUTABLE)

$(EXECUTABLE): main.c google-authenticator.c hmac.c sha1.c base32.c
	$(GCC) $(CFLAGS) $(INCLUDE) $^ $(LIBS) -o $(BINPATH)/$@

clean:
	rm -f $(BINPATH)/$(EXECUTABLE) $(BINPATH)/googauth_debug
