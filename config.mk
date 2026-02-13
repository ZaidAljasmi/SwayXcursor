VERSION = 1.0

PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

GTKINC = $(shell pkg-config --cflags gtk4)
GTKLIB = $(shell pkg-config --libs gtk4) -lXcursor

CPPFLAGS = -DVERSION=\"${VERSION}\"
CFLAGS = -std=c99 -pedantic -Wall -Os ${GTKINC} ${CPPFLAGS}
LDFLAGS = ${GTKLIB}

CC = cc
