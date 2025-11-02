include config.mk

SRC = template.c
OBJ = ${SRC:.c=.o}

all: template

.c.o:
	${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

config.h:
	cp config.def.h $@

template: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}
test: ${OBJ}
	${CC} -o $@ ${SRC} ${TESTCFLAGS} ${LDFLAGS} 
clean:
	rm -f template ${OBJ} template-${VERSION}.tar.gz ;

dist: clean
	mkdir -p template-${VERSION}
	cp -R LICENSE Makefile README config.mk\
		template.1 ${SRC} template-${VERSION}
	tar -cf template-${VERSION}.tar template-${VERSION}
	gzip template-${VERSION}.tar
	rm -rf template-${VERSION}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f template ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/template
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	sed "s/VERSION/${VERSION}/g" < template.1 > ${DESTDIR}${MANPREFIX}/man1/template.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/template.1

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/template\
		${DESTDIR}${MANPREFIX}/man1/template.1

.PHONY: all clean dist install uninstall
