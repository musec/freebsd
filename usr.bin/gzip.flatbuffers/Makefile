#	$NetBSD: Makefile,v 1.18 2013/11/13 11:12:24 pettai Exp $
# $FreeBSD$

.include <src.opts.mk>

PROG=		gzip.flatbuffers
SRCS= 		gzip.c gzip_wrapped.c ipc_utils.c
BINDIR=		usr/bin

MAN=		gzip.1 gzexe.1 zdiff.1 zforce.1 zmore.1 znew.1

LIBADD=		z lzma

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

LINKS+=		${BINDIR}/gzip.flatbuffers ${BINDIR}/gunzip.flatbuffers \
		${BINDIR}/gzip.flatbuffers ${BINDIR}/gzcat.flatbuffers \
		${BINDIR}/gzip.flatbuffers ${BINDIR}/zcat.flatbuffers\
		${BINDIR}/zdiff.flatbuffers ${BINDIR}/zcmp.flatbuffers

#.if ${MK_TESTS} != "no"
#SUBDIR+=	tests
#.endif

.include "${.CURDIR}/Makefile.inc"
.include <bsd.prog.mk>
CFLAGS:=${CFLAGS:S/-Werror//}
CFLAGS:=${CFLAGS:S/-Wincompatible-pointer-types-discards-qualifiers//}
CFLAGS:=${CFLAGS:S/-Wcast-qual//}
CFLAGS:=${CFLAGS:S/-Wcast-align//}
CFLAGS:=${CFLAGS:S/-Wincompatible-pointer-types-discards-qualifiers//}
