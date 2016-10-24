#	$NetBSD: Makefile,v 1.18 2013/11/13 11:12:24 pettai Exp $
# $FreeBSD$

.include <src.opts.mk>

PROG=		gzip.avro
SRCS= 		gzip.c gzip_wrapped.c ipc_utils.c
BINDIR=		usr/bin

MAN=		gzip.1 gzexe.1 zdiff.1 zforce.1 zmore.1 znew.1

LIBADD=		z lzma
LDADD+=-lavro
LDADD+=-ljansson

EXT_INC_DIR=-Iinclude
EXT_LIB_DIR=-Llib

CFLAGS+= ${EXT_INC_DIR} ${EXT_LIB_DIR}

.if ${MK_BZIP2_SUPPORT} != "no"
LIBADD+=	bz2
.else
CFLAGS+=	-DNO_BZIP2_SUPPORT
.endif

SCRIPTS=	gzexe zdiff zforce zmore znew

MLINKS+=	gzip.1 gunzip.1 \
		gzip.1 gzcat.1 \
		gzip.1 zcat.1 \
		zdiff.1 zcmp.1 \
		zmore.1 zless.1

LINKS+=		${BINDIR}/gzip.avro ${BINDIR}/gunzip.avro \
		${BINDIR}/gzip.avro ${BINDIR}/gzcat.avro \
		${BINDIR}/gzip.avro ${BINDIR}/zcat.avro\
		${BINDIR}/zdiff.avro ${BINDIR}/zcmp.avro

#.if ${MK_TESTS} != "no"
#SUBDIR+=	tests
#.endif

.include <bsd.prog.mk>
CFLAGS:=${CFLAGS:S/-Werror//}
