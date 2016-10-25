# $FreeBSD$

LDADD+=-lavro
LDADD+=-ljansson
CFLAGS+= -I${.CURDIR}/include \
	 -L${.CURDIR}/lib
