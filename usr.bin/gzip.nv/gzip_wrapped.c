#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/capsicum.h>

#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <zlib.h>
#include "gzip_wrapped.h"
#include "gzip.h"

#define GZ_COMPRESS 1
#define GZ_UNCOMPRESS 2

#define NV_OPERATION "operation"
#define NV_IN_FILE "in"
#define NV_OUT_FILE "out"
#define NV_MTIME "mtime"
#define NV_NAME "name"
#define NV_GSIZEP "gsizep"
#define NV_BYTES_READ "bytes_read"
#define NV_PRE "pre"
#define NV_PRE_LEN "prelen"
#define NV_FILENAME "filename"

void gz_sandbox(int sock)
{
    fclose(stdin);
    fclose(stdout);

    cap_rights_t rights;

    // Setup rights for socket to be used by libnv
    cap_rights_init(&rights, CAP_EVENT, CAP_READ, CAP_WRITE );
    cap_rights_limit( sock, &rights );

    if (cap_enter() < 0 && errno != ENOSYS)
		err(1, "unable to enter capability mode");

    nvlist_t *nvl;
    while ( (nvl = nvlist_recv(sock, 0)) ) { // What are these flags
        
        uint64_t op;
        op = nvlist_get_number( nvl, NV_OPERATION );
        
        switch( op )
        {
        case GZ_COMPRESS:
            gz_compress_unwrapper(nvl, sock);
            nvlist_destroy(nvl);
            break;
        case GZ_UNCOMPRESS:
            gz_uncompress_unwrapper(nvl, sock);
            nvlist_destroy(nvl);
            break;
        default:
            fprintf(stderr, "Unknown operation.");
            break;
        }

    }
}


off_t gz_compress_wrapper(int in, int out, off_t *gsizep, const char *origname, uint32_t mtime, int sock)
{
    limit_fds(in, out);

    // Fill out nvlist
    nvlist_t *nvl;
    nvl = nvlist_create(0);
    nvlist_add_number(nvl, NV_OPERATION, GZ_COMPRESS);
    nvlist_add_descriptor(nvl, NV_IN_FILE, in);
    nvlist_add_descriptor(nvl, NV_OUT_FILE, out);
    nvlist_add_string(nvl, NV_NAME, origname);
    nvlist_add_number(nvl, NV_MTIME, mtime);

    // Send to sandbox
    if (nvlist_send(sock, nvl) < 0)
    {
        fprintf(stderr, "nvlist failed with error %d", nvlist_error(nvl));
        nvlist_destroy(nvl);
        err(1, "nvlist_send() failed");
    }
    nvlist_destroy(nvl);

    nvlist_t* nvl_ret;
    nvl_ret = nvlist_recv(sock, 0);

    if (nvl_ret ==	NULL) 
    {
	     err(1, "nvlist_recv() failed");
    }

    *gsizep = nvlist_get_number(nvl_ret, NV_GSIZEP);
    off_t bytes_read = nvlist_get_number(nvl_ret, NV_BYTES_READ);
    nvlist_destroy(nvl_ret);

    return bytes_read;
}



off_t gz_uncompress_wrapper(int in, int out, char *pre, size_t prelen, off_t *gsizep,
	      const char *filename, int sock)
{
    limit_fds(in, out);

    // Fill out nvlist
    nvlist_t *nvl;
    nvl = nvlist_create(0);
    nvlist_add_number(nvl, NV_OPERATION, GZ_UNCOMPRESS);
    nvlist_add_descriptor(nvl, NV_IN_FILE, in);
    nvlist_add_descriptor(nvl, NV_OUT_FILE, out);
    if (pre) 
    {
        nvlist_add_string(nvl, NV_PRE, pre);
        nvlist_add_number(nvl, NV_PRE_LEN, prelen);
    }
    nvlist_add_string(nvl, NV_FILENAME, filename);

    // Send to sandbox
    if (nvlist_send(sock, nvl) < 0)
    {
        nvlist_destroy(nvl);
        err(1, "nvlist_send() failed");
    }
    nvlist_destroy(nvl);
   
    nvlist_t* nvl_ret;
    nvl_ret = nvlist_recv(sock, 0);

    if (nvl_ret ==	NULL) 
    {
	     err(1, "nvlist_recv() failed");
    }

    if (gsizep)
    {
        *gsizep = nvlist_get_number(nvl_ret, NV_GSIZEP);
    }
    off_t bytes_read = nvlist_get_number(nvl_ret, NV_BYTES_READ);
    nvlist_destroy(nvl_ret);

    return bytes_read;
}


void gz_compress_unwrapper(struct nvlist* nvl, int sock)
{
    int in, out;

    const char* origname;
    uint32_t mtime;
    off_t gsizep;

    in = nvlist_get_descriptor(nvl, NV_IN_FILE);
    out = nvlist_get_descriptor(nvl, NV_OUT_FILE);
    origname = nvlist_get_string(nvl, NV_NAME);
    mtime = nvlist_get_number(nvl, NV_MTIME);
    
    off_t bytes_read = gz_compress(in, out, &gsizep, origname, mtime); 

    struct nvlist* nvl_ret = nvlist_create(0);
    nvlist_add_number(nvl_ret, NV_GSIZEP, gsizep);
    nvlist_add_number(nvl_ret, NV_BYTES_READ, bytes_read);

    if (nvlist_send(sock, nvl_ret) < 0)
    {
        nvlist_destroy(nvl_ret);
        err(1, "nvlist_send() failed");
    }
    nvlist_destroy(nvl_ret);
}

void gz_uncompress_unwrapper(struct nvlist* nvl, int sock)
{
    int in, out, prelen;
    off_t gsizep, bytes_written;
    char* pre;
    const char* filename;

    in = nvlist_get_descriptor(nvl, NV_IN_FILE);
    out = nvlist_get_descriptor(nvl, NV_OUT_FILE);
    
    if (nvlist_exists_string(nvl, NV_PRE))
    {
        pre = strdup( nvlist_get_string(nvl, NV_PRE) );
        prelen = nvlist_get_number(nvl, NV_PRE_LEN);
    } else {
        pre = NULL;
        prelen = 0;
    }

    if (nvlist_exists_string(nvl, NV_FILENAME))
    {
        filename = nvlist_get_string(nvl, NV_FILENAME);
    } else {
        filename = NULL;
    }

    bytes_written = gz_uncompress(in, out, pre, prelen, &gsizep, filename);

    struct nvlist* nvl_ret = nvlist_create(0);
    nvlist_add_number(nvl_ret, NV_GSIZEP, gsizep);
    nvlist_add_number(nvl_ret, NV_BYTES_READ, bytes_written);

    if (nvlist_send(sock, nvl_ret) < 0)
    {
        nvlist_destroy(nvl_ret);
        err(1, "nvlist_send() failed");
    }
    nvlist_destroy(nvl_ret);
}

/* compress input to output. Return bytes read, -1 on error */
off_t
gz_compress(int in, int out, off_t *gsizep, const char *origname, uint32_t mtime)
{
	z_stream z;
	char *outbufp, *inbufp;
	off_t in_tot = 0, out_tot = 0;
	ssize_t in_size;
	int i, error;
	uLong crc;
#ifdef SMALL
	static char header[] = { GZIP_MAGIC0, GZIP_MAGIC1, Z_DEFLATED, 0,
				 0, 0, 0, 0,
				 0, OS_CODE };
#endif

	outbufp = malloc(BUFLEN);
	inbufp = malloc(BUFLEN);
	if (outbufp == NULL || inbufp == NULL) {
		maybe_err("malloc failed");
		goto out;
	}

	memset(&z, 0, sizeof z);
	z.zalloc = Z_NULL;
	z.zfree = Z_NULL;
	z.opaque = 0;

#ifdef SMALL
	memcpy(outbufp, header, sizeof header);
	i = sizeof header;
#else
	if (nflag != 0) {
		mtime = 0;
		origname = "";
	}

	i = snprintf(outbufp, BUFLEN, "%c%c%c%c%c%c%c%c%c%c%s", 
		     GZIP_MAGIC0, GZIP_MAGIC1, Z_DEFLATED,
		     *origname ? ORIG_NAME : 0,
		     mtime & 0xff,
		     (mtime >> 8) & 0xff,
		     (mtime >> 16) & 0xff,
		     (mtime >> 24) & 0xff,
		     numflag == 1 ? 4 : numflag == 9 ? 2 : 0,
		     OS_CODE, origname);
	if (i >= BUFLEN)     
		/* this need PATH_MAX > BUFLEN ... */
		maybe_err("snprintf");
	if (*origname)
		i++;
#endif

	z.next_out = (unsigned char *)outbufp + i;
	z.avail_out = BUFLEN - i;

	error = deflateInit2(&z, numflag, Z_DEFLATED,
			     (-MAX_WBITS), 8, Z_DEFAULT_STRATEGY);
	if (error != Z_OK) {
		maybe_warnx("deflateInit2 failed");
		in_tot = -1;
		goto out;
	}

	crc = crc32(0L, Z_NULL, 0);
	for (;;) {
		if (z.avail_out == 0) {
			if (write(out, outbufp, BUFLEN) != BUFLEN) {
				maybe_warn("write");
				out_tot = -1;
				goto out;
			}

			out_tot += BUFLEN;
			z.next_out = (unsigned char *)outbufp;
			z.avail_out = BUFLEN;
		}

		if (z.avail_in == 0) {
			in_size = read(in, inbufp, BUFLEN);
			if (in_size < 0) {
				maybe_warn("read");
				in_tot = -1;
				goto out;
			}
			if (in_size == 0)
				break;

			crc = crc32(crc, (const Bytef *)inbufp, (unsigned)in_size);
			in_tot += in_size;
			z.next_in = (unsigned char *)inbufp;
			z.avail_in = in_size;
		}

		error = deflate(&z, Z_NO_FLUSH);
		if (error != Z_OK && error != Z_STREAM_END) {
			maybe_warnx("deflate failed");
			in_tot = -1;
			goto out;
		}
	}

	/* clean up */
	for (;;) {
		size_t len;
		ssize_t w;

		error = deflate(&z, Z_FINISH);
		if (error != Z_OK && error != Z_STREAM_END) {
			maybe_warnx("deflate failed");
			in_tot = -1;
			goto out;
		}

		len = (char *)z.next_out - outbufp;

		w = write(out, outbufp, len);
		if (w == -1 || (size_t)w != len) {
			maybe_warn("write");
			out_tot = -1;
			goto out;
		}
		out_tot += len;
		z.next_out = (unsigned char *)outbufp;
		z.avail_out = BUFLEN;

		if (error == Z_STREAM_END)
			break;
	}

	if (deflateEnd(&z) != Z_OK) {
		maybe_warnx("deflateEnd failed");
		in_tot = -1;
		goto out;
	}

	i = snprintf(outbufp, BUFLEN, "%c%c%c%c%c%c%c%c", 
		 (int)crc & 0xff,
		 (int)(crc >> 8) & 0xff,
		 (int)(crc >> 16) & 0xff,
		 (int)(crc >> 24) & 0xff,
		 (int)in_tot & 0xff,
		 (int)(in_tot >> 8) & 0xff,
		 (int)(in_tot >> 16) & 0xff,
		 (int)(in_tot >> 24) & 0xff);
	if (i != 8)
		maybe_err("snprintf");
	if (write(out, outbufp, i) != i) {
		maybe_warn("write");
		in_tot = -1;
	} else
		out_tot += i;

out:
	if (inbufp != NULL)
		free(inbufp);
	if (outbufp != NULL)
		free(outbufp);
	if (gsizep)
		*gsizep = out_tot;
	return in_tot;
}

/*
 * uncompress input to output then close the input.  return the
 * uncompressed size written, and put the compressed sized read
 * into `*gsizep'.
 */
off_t
gz_uncompress(int in, int out, char *pre, size_t prelen, off_t *gsizep,
	      const char *filename)
{
	z_stream z;
	char *outbufp, *inbufp;
	off_t out_tot = -1, in_tot = 0;
	uint32_t out_sub_tot = 0;
	enum {
		GZSTATE_MAGIC0,
		GZSTATE_MAGIC1,
		GZSTATE_METHOD,
		GZSTATE_FLAGS,
		GZSTATE_SKIPPING,
		GZSTATE_EXTRA,
		GZSTATE_EXTRA2,
		GZSTATE_EXTRA3,
		GZSTATE_ORIGNAME,
		GZSTATE_COMMENT,
		GZSTATE_HEAD_CRC1,
		GZSTATE_HEAD_CRC2,
		GZSTATE_INIT,
		GZSTATE_READ,
		GZSTATE_CRC,
		GZSTATE_LEN,
	} state = GZSTATE_MAGIC0;
	int flags = 0, skip_count = 0;
	int error = Z_STREAM_ERROR, done_reading = 0;
	uLong crc = 0;
	ssize_t wr;
	int needmore = 0;

#define ADVANCE()       { z.next_in++; z.avail_in--; }

	if ((outbufp = malloc(BUFLEN)) == NULL) {
		maybe_err("malloc failed");
		goto out2;
	}
	if ((inbufp = malloc(BUFLEN)) == NULL) {
		maybe_err("malloc failed");
		goto out1;
	}

	memset(&z, 0, sizeof z);
	z.avail_in = prelen;
	z.next_in = (unsigned char *)pre;
	z.avail_out = BUFLEN;
	z.next_out = (unsigned char *)outbufp;
	z.zalloc = NULL;
	z.zfree = NULL;
	z.opaque = 0;

	in_tot = prelen;
	out_tot = 0;

	for (;;) {
		if ((z.avail_in == 0 || needmore) && done_reading == 0) {
			ssize_t in_size;

			if (z.avail_in > 0) {
				memmove(inbufp, z.next_in, z.avail_in);
			}
			z.next_in = (unsigned char *)inbufp;
			in_size = read(in, z.next_in + z.avail_in,
			    BUFLEN - z.avail_in);

			if (in_size == -1) {
				maybe_warn("failed to read stdin");
				goto stop_and_fail;
			} else if (in_size == 0) {
				done_reading = 1;
			}

			z.avail_in += in_size;
			needmore = 0;

			in_tot += in_size;
		}
		if (z.avail_in == 0) {
			if (done_reading && state != GZSTATE_MAGIC0) {
				maybe_warnx("%s: unexpected end of file",
					    filename);
				goto stop_and_fail;
			}
			goto stop;
		}
		switch (state) {
		case GZSTATE_MAGIC0:
			if (*z.next_in != GZIP_MAGIC0) {
				if (in_tot > 0) {
					maybe_warnx("%s: trailing garbage "
						    "ignored", filename);
					exit_value = 2;
					goto stop;
				}
				maybe_warnx("input not gziped (MAGIC0)");
				goto stop_and_fail;
			}
			ADVANCE();
			state++;
			out_sub_tot = 0;
			crc = crc32(0L, Z_NULL, 0);
			break;

		case GZSTATE_MAGIC1:
			if (*z.next_in != GZIP_MAGIC1 &&
			    *z.next_in != GZIP_OMAGIC1) {
				maybe_warnx("input not gziped (MAGIC1)");
				goto stop_and_fail;
			}
			ADVANCE();
			state++;
			break;

		case GZSTATE_METHOD:
			if (*z.next_in != Z_DEFLATED) {
				maybe_warnx("unknown compression method");
				goto stop_and_fail;
			}
			ADVANCE();
			state++;
			break;

		case GZSTATE_FLAGS:
			flags = *z.next_in;
			ADVANCE();
			skip_count = 6;
			state++;
			break;

		case GZSTATE_SKIPPING:
			if (skip_count > 0) {
				skip_count--;
				ADVANCE();
			} else
				state++;
			break;

		case GZSTATE_EXTRA:
			if ((flags & EXTRA_FIELD) == 0) {
				state = GZSTATE_ORIGNAME;
				break;
			}
			skip_count = *z.next_in;
			ADVANCE();
			state++;
			break;

		case GZSTATE_EXTRA2:
			skip_count |= ((*z.next_in) << 8);
			ADVANCE();
			state++;
			break;

		case GZSTATE_EXTRA3:
			if (skip_count > 0) {
				skip_count--;
				ADVANCE();
			} else
				state++;
			break;

		case GZSTATE_ORIGNAME:
			if ((flags & ORIG_NAME) == 0) {
				state++;
				break;
			}
			if (*z.next_in == 0)
				state++;
			ADVANCE();
			break;

		case GZSTATE_COMMENT:
			if ((flags & COMMENT) == 0) {
				state++;
				break;
			}
			if (*z.next_in == 0)
				state++;
			ADVANCE();
			break;

		case GZSTATE_HEAD_CRC1:
			if (flags & HEAD_CRC)
				skip_count = 2;
			else
				skip_count = 0;
			state++;
			break;

		case GZSTATE_HEAD_CRC2:
			if (skip_count > 0) {
				skip_count--;
				ADVANCE();
			} else
				state++;
			break;

		case GZSTATE_INIT:
			if (inflateInit2(&z, -MAX_WBITS) != Z_OK) {
				maybe_warnx("failed to inflateInit");
				goto stop_and_fail;
			}
			state++;
			break;

		case GZSTATE_READ:
			error = inflate(&z, Z_FINISH);
			switch (error) {
			/* Z_BUF_ERROR goes with Z_FINISH... */
			case Z_BUF_ERROR:
				if (z.avail_out > 0 && !done_reading)
					continue;

			case Z_STREAM_END:
			case Z_OK:
				break;

			case Z_NEED_DICT:
				maybe_warnx("Z_NEED_DICT error");
				goto stop_and_fail;
			case Z_DATA_ERROR:
				maybe_warnx("data stream error");
				goto stop_and_fail;
			case Z_STREAM_ERROR:
				maybe_warnx("internal stream error");
				goto stop_and_fail;
			case Z_MEM_ERROR:
				maybe_warnx("memory allocation error");
				goto stop_and_fail;

			default:
				maybe_warn("unknown error from inflate(): %d",
				    error);
			}
			wr = BUFLEN - z.avail_out;

			if (wr != 0) {
				crc = crc32(crc, (const Bytef *)outbufp, (unsigned)wr);
				if (
#ifndef SMALL
				    /* don't write anything with -t */
				    tflag == 0 &&
#endif
				    write(out, outbufp, wr) != wr) {
					maybe_warn("error writing to output");
					goto stop_and_fail;
				}

				out_tot += wr;
				out_sub_tot += wr;
			}

			if (error == Z_STREAM_END) {
				inflateEnd(&z);
				state++;
			}

			z.next_out = (unsigned char *)outbufp;
			z.avail_out = BUFLEN;

			break;
		case GZSTATE_CRC:
			{
				uLong origcrc;

				if (z.avail_in < 4) {
					if (!done_reading) {
						needmore = 1;
						continue;
					}
					maybe_warnx("truncated input");
					goto stop_and_fail;
				}
				origcrc = ((unsigned)z.next_in[0] & 0xff) |
					((unsigned)z.next_in[1] & 0xff) << 8 |
					((unsigned)z.next_in[2] & 0xff) << 16 |
					((unsigned)z.next_in[3] & 0xff) << 24;
				if (origcrc != crc) {
					maybe_warnx("invalid compressed"
					     " data--crc error");
					goto stop_and_fail;
				}
			}

			z.avail_in -= 4;
			z.next_in += 4;

			if (!z.avail_in && done_reading) {
				goto stop;
			}
			state++;
			break;
		case GZSTATE_LEN:
			{
				uLong origlen;

				if (z.avail_in < 4) {
					if (!done_reading) {
						needmore = 1;
						continue;
					}
					maybe_warnx("truncated input");
					goto stop_and_fail;
				}
				origlen = ((unsigned)z.next_in[0] & 0xff) |
					((unsigned)z.next_in[1] & 0xff) << 8 |
					((unsigned)z.next_in[2] & 0xff) << 16 |
					((unsigned)z.next_in[3] & 0xff) << 24;

				if (origlen != out_sub_tot) {
					maybe_warnx("invalid compressed"
					     " data--length error");
					goto stop_and_fail;
				}
			}
				
			z.avail_in -= 4;
			z.next_in += 4;

			if (error < 0) {
				maybe_warnx("decompression error");
				goto stop_and_fail;
			}
			state = GZSTATE_MAGIC0;
			break;
		}
		continue;
stop_and_fail:
		out_tot = -1;
stop:
		break;
	}
	if (state > GZSTATE_INIT)
		inflateEnd(&z);

	free(inbufp);
out1:
	free(outbufp);
out2:
	if (gsizep)
		*gsizep = in_tot;
	return (out_tot);
}

// Limit file descriptors
void limit_fds(int in, int out) 
{
    cap_rights_t rights;

    cap_rights_init(&rights, CAP_FSTAT, CAP_READ, CAP_SEEK);
    cap_rights_limit(in, &rights);

    cap_rights_clear(&rights);
    cap_rights_set(&rights, CAP_FSTAT, CAP_WRITE, CAP_SEEK, CAP_FCHMOD, CAP_FCHOWN, CAP_FUTIMES, CAP_FCHFLAGS );
    cap_rights_limit(out, &rights);
}
