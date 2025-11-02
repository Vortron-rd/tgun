# version
VERSION = 0.0.0

# Customize below to fit your system

# paths
PREFIX = /usr
MANPREFIX = ${PREFIX}/share/man

# includes and libs
INCS =
LIBS = -lm -lSDL3 -lSDL3_image -lSDL3_ttf 

# flags
CPPFLAGS =  -DVERSION=\"${VERSION}\"
TESTCFLAGS   = -g -std=c99 -pedantic -Wall -O0 ${INCS} ${CPPFLAGS}
CFLAGS   = -pedantic -Wall -Wno-deprecated-declarations -Os ${INCS} ${CPPFLAGS}
LDFLAGS  = ${LIBS}


# compiler and linker
CC = cc
