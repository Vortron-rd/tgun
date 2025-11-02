include config.mk

SRC = main.c
OBJ = ${SRC:.c=.o}

all: main 

.c.o:
	${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

config.h:
	cp config.def.h $@

main: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}
test: ${OBJ}
	${CC} -o $@ ${SRC} ${TESTCFLAGS} ${LDFLAGS} 
clean:
	rm -f main ${OBJ} main-${VERSION}.tar.gz ;

dist: clean
	mkdir -p main-${VERSION}
	cp -R LICENSE Makefile README config.mk\
		main.1 ${SRC} main-${VERSION}
	tar -cf main-${VERSION}.tar main-${VERSION}
	gzip main-${VERSION}.tar
	rm -rf main-${VERSION}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f main ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/main
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	sed "s/VERSION/${VERSION}/g" < main.1 > ${DESTDIR}${MANPREFIX}/man1/main.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/main.1

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/main\
		${DESTDIR}${MANPREFIX}/man1/main.1

.PHONY: all clean dist install uninstall
