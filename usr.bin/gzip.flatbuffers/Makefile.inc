# $FreeBSD$

LDADD+=-lflatcc
LDADD+=-lflatccrt
CFLAGS+= -I${.CURDIR}/include \
	 -L${.CURDIR}/lib
