# $FreeBSD$

LDADD+=-lprotobuf
LDADD+=-lprotobuf-c
CFLAGS+= -I${.CURDIR}/include \
	     -L${.CURDIR}/lib
