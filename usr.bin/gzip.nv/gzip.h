
/* what type of file are we dealing with */
enum filetype {
	FT_GZIP,
#ifndef NO_BZIP2_SUPPORT
	FT_BZIP2,
#endif
#ifndef NO_COMPRESS_SUPPORT
	FT_Z,
#endif
#ifndef NO_PACK_SUPPORT
	FT_PACK,
#endif
#ifndef NO_XZ_SUPPORT
	FT_XZ,
#endif
	FT_LAST,
	FT_UNKNOWN
};

#ifndef NO_BZIP2_SUPPORT
#include <bzlib.h>

#define BZ2_SUFFIX	".bz2"
#define BZIP2_MAGIC	"\102\132\150"
#endif

#ifndef NO_COMPRESS_SUPPORT
#define Z_SUFFIX	".Z"
#define Z_MAGIC		"\037\235"
#endif

#ifndef NO_PACK_SUPPORT
#define PACK_MAGIC	"\037\036"
#endif

#ifndef NO_XZ_SUPPORT
#include <lzma.h>
#define XZ_SUFFIX	".xz"
#define XZ_MAGIC	"\3757zXZ"
#endif

#define GZ_SUFFIX	".gz"


#define GZIP_MAGIC0	0x1F
#define GZIP_MAGIC1	0x8B
#define GZIP_OMAGIC1	0x9E

#define GZIP_TIMESTAMP	(off_t)4
#define GZIP_ORIGNAME	(off_t)10

#define HEAD_CRC	0x02
#define EXTRA_FIELD	0x04
#define ORIG_NAME	0x08
#define COMMENT		0x10

#define OS_CODE		3	/* Unix */

typedef struct {
    const char	*zipped;
    int		ziplen;
    const char	*normal;	/* for unzip - must not be longer than zipped */
} suffixes_t;
static suffixes_t suffixes[] = {
#define	SUFFIX(Z, N) {Z, sizeof Z - 1, N}
	SUFFIX(GZ_SUFFIX,	""),	/* Overwritten by -S .xxx */
#ifndef SMALL
	SUFFIX(GZ_SUFFIX,	""),
	SUFFIX(".z",		""),
	SUFFIX("-gz",		""),
	SUFFIX("-z",		""),
	SUFFIX("_z",		""),
	SUFFIX(".taz",		".tar"),
	SUFFIX(".tgz",		".tar"),
#ifndef NO_BZIP2_SUPPORT
	SUFFIX(BZ2_SUFFIX,	""),
	SUFFIX(".tbz",		".tar"),
	SUFFIX(".tbz2",		".tar"),
#endif
#ifndef NO_COMPRESS_SUPPORT
	SUFFIX(Z_SUFFIX,	""),
#endif
#ifndef NO_XZ_SUPPORT
	SUFFIX(XZ_SUFFIX,	""),
#endif
	SUFFIX(GZ_SUFFIX,	""),	/* Overwritten by -S "" */
#endif /* SMALL */
#undef SUFFIX
};
#define NUM_SUFFIXES (sizeof suffixes / sizeof suffixes[0])
#define SUFFIX_MAXLEN	30

static	const char	gzip_version[] = "FreeBSD gzip 20150413";

#ifndef SMALL
static	const char	gzip_copyright[] = \
"   Copyright (c) 1997, 1998, 2003, 2004, 2006 Matthew R. Green\n"
"   All rights reserved.\n"
"\n"
"   Redistribution and use in source and binary forms, with or without\n"
"   modification, are permitted provided that the following conditions\n"
"   are met:\n"
"   1. Redistributions of source code must retain the above copyright\n"
"      notice, this list of conditions and the following disclaimer.\n"
"   2. Redistributions in binary form must reproduce the above copyright\n"
"      notice, this list of conditions and the following disclaimer in the\n"
"      documentation and/or other materials provided with the distribution.\n"
"\n"
"   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR\n"
"   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES\n"
"   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.\n"
"   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,\n"
"   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,\n"
"   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;\n"
"   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED\n"
"   AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,\n"
"   OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY\n"
"   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF\n"
"   SUCH DAMAGE.";
#endif

static	int	cflag;			/* stdout mode */
static	int	dflag;			/* decompress mode */
static	int	lflag;			/* list mode */
static	int	numflag = 6;		/* gzip -1..-9 value */

#ifndef SMALL
static	int	fflag;			/* force mode */
static	int	kflag;			/* don't delete input files */
static	int	nflag;			/* don't save name/timestamp */
static	int	Nflag;			/* don't restore name/timestamp */
static	int	qflag;			/* quiet mode */
static	int	rflag;			/* recursive mode */
static	int	tflag;			/* test */
static	int	vflag;			/* verbose mode */
static	const char *remove_file = NULL;	/* file to be removed upon SIGINT */
#else
#define		qflag	0
#define		tflag	0
#endif

static	int	exit_value = 0;		/* exit value */

static	char	*infile;		/* name of file coming in */

void	maybe_err(const char *fmt, ...) __printflike(1, 2) __dead2;
/* #if !defined(NO_BZIP2_SUPPORT) || !defined(NO_PACK_SUPPORT) ||	\ */
/*     !defined(NO_XZ_SUPPORT) */
/* static	void	maybe_errx(const char *fmt, ...) __printflike(1, 2) __dead2; */
/* #endif */
void	maybe_warn(const char *fmt, ...) __printflike(1, 2);
void	maybe_warnx(const char *fmt, ...) __printflike(1, 2);
/* static	enum filetype file_gettype(u_char *); */
/* #ifdef SMALL */
/* #define gz_compress(if, of, sz, fn, tm) gz_compress(if, of, sz) */
/* #endif */
/* static	off_t	file_compress(char *, char *, size_t); */
/* static	off_t	file_uncompress(char *, char *, size_t); */
/* static	void	handle_pathname(char *); */
/* static	void	handle_file(char *, struct stat *); */
/* static	void	handle_stdin(void); */
/* static	void	handle_stdout(void); */
/* static	void	print_ratio(off_t, off_t, FILE *); */
/* static	void	print_list(int fd, off_t, const char *, time_t); */
/* static	void	usage(void) __dead2; */
/* static	void	display_version(void) __dead2; */
/* #ifndef SMALL */
/* static	void	display_license(void); */
/* static	void	sigint_handler(int); */
/* #endif */
/* static	const suffixes_t *check_suffix(char *, int); */
/* static	ssize_t	read_retry(int, void *, size_t); */

/* #ifdef SMALL */
/* #define unlink_input(f, sb) unlink(f) */
/* #else */
/* static	off_t	cat_fd(unsigned char *, size_t, off_t *, int fd); */
/* static	void	prepend_gzip(char *, int *, char ***); */
/* static	void	handle_dir(char *); */
/* static	void	print_verbage(const char *, const char *, off_t, off_t); */
/* static	void	print_test(const char *, int); */
/* static	void	copymodes(int fd, const struct stat *, const char *file); */
/* static	int	check_outfile(const char *outfile); */
/* #endif */

/* #ifndef NO_BZIP2_SUPPORT */
/* static	off_t	unbzip2(int, int, char *, size_t, off_t *); */
/* #endif */

/* #ifndef NO_COMPRESS_SUPPORT */
/* static	FILE 	*zdopen(int); */
/* static	off_t	zuncompress(FILE *, FILE *, char *, size_t, off_t *); */
/* #endif */

/* #ifndef NO_PACK_SUPPORT */
/* static	off_t	unpack(int, int, char *, size_t, off_t *); */
/* #endif */

/* #ifndef NO_XZ_SUPPORT */
/* static	off_t	unxz(int, int, char *, size_t, off_t *); */
/* #endif */

/*-
 * Copyright (c) 1997, 1998, 2003, 2004, 2006 Matthew R. Green
 * Copyright (c) 2016 Brian J. Kidney
 * All rights reserved.
 *
 * Portions of this software were developed by BAE Systems, the University of
 * Cambridge Computer Laboratory, and Memorial University under DARPA/AFRL
 * contract FA8650-15-C-7558 ("CADETS"), as part of the DARPA Transparent
 * Computing (TC) research program.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
