/*-
 * Copyright (c) 2016 Brian J. Kidney
 * All rights reserved.
 *
 * This software was developed by BAE Systems, the University of Cambridge
 * Computer Laboratory, and Memorial University under DARPA/AFRL contract
 * FA8650-15-C-7558 ("CADETS"), as part of the DARPA Transparent Computing
 * (TC) research program.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

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
