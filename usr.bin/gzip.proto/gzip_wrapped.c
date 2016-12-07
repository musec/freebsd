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
#include <sysexits.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <zlib.h>
#include "gzip_wrapped.h"
#include "gzip.h"
#include "ipc_utils.h"
#include "gzip.pb-c.h"

void gz_sandbox(int sock)
{
    fclose(stdin);
    fclose(stdout);

    cap_rights_t rights;

    // Setup rights for socket 
    cap_rights_init(&rights, CAP_EVENT, CAP_READ, CAP_WRITE );
    cap_rights_limit( sock, &rights );

    if (cap_enter() < 0 && errno != ENOSYS)
		err(1, "unable to enter capability mode \n");

    // Receive two file descriptors
    int fds[2];
    int nonces[2];
    fds[0] = recv_fd(sock, &nonces[0]);
    fds[1] = recv_fd(sock, &nonces[1]);

    ssize_t size;
    uint8_t buf[MAX_MSG_SIZE];
    
    size = read(sock, &buf, MAX_MSG_SIZE);
    if ( size < 0 )
    {
        perror("Read request: ");
        err(EX_IOERR, "Could not get request: ");
    }

    Request* req;
    req = request__unpack(NULL, size, buf);
    
    if ( req == NULL ) 
    {
        fprintf(stderr, "Request: Buffer could not be decoded. \n");
    }
    else
    {
        if (req->type == REQUEST__TYPE__COMPRESS)
        {
            gz_compress_unwrapper( req->compress, fds, nonces, sock );  
        }
        else if (req->type == REQUEST__TYPE__UNCOMPRESS)
        {
            gz_uncompress_unwrapper( req->uncompress, fds, nonces, sock );
        }
        else 
        {
            fprintf(stderr, "Unknown operation. \n");
        }
    }
}


off_t gz_compress_wrapper(int in, int out, off_t *gsizep, const char *origname, uint32_t mtime, int sock)
{
    limit_fds(in, out);

    Compress msg = COMPRESS__INIT;

    // Send the file descriptors and get nonces
    int in_nonce = send_fd(sock, in);
    int out_nonce = send_fd(sock, out);

    msg.fd_in = in_nonce;
    msg.fd_out = out_nonce;
    msg.original_name = strdup(origname);
    msg.mtime = mtime;

    Request req = REQUEST__INIT;
    req.type = REQUEST__TYPE__COMPRESS;
    req.compress = &msg;

    uint8_t *buf;
    ssize_t size;

    size = request__get_packed_size(&req);
    buf = malloc(size);
    request__pack(&req,buf);
    fprintf( stderr, "The compress request has size %zd \n", size );
    // Send the buffer
    if ( write( sock, buf, size ) < 0 )
    {
        free(buf);
        err(EX_IOERR, "Could not send compress request");
    }

    // Clean up
    free(buf);

    // Handle Reponse
    uint8_t rbuf[MAX_MSG_SIZE];
    
    size = read(sock, &rbuf, MAX_MSG_SIZE);
    if (size < 0)
    {
        err(EX_IOERR, "Could not get compress request response: ");
    }

    fprintf(stderr, "Got a return of size %zd \n", size);
    Return* rtn;
    rtn = return__unpack(NULL, size, rbuf);

    off_t bytes_read = 0;
    *gsizep = 0;
    if (rtn == NULL)
    {
        fprintf(stderr, "Compress Return: Buffer could not be decoded. \n");
    }
    else
    {
        *gsizep = rtn->size;
        bytes_read = rtn->bytes;
    }
    return bytes_read;
}



off_t gz_uncompress_wrapper(int in, int out, char *pre, size_t prelen, off_t *gsizep,
	      const char *filename, int sock)
{
    limit_fds(in, out);

    Uncompress msg = UNCOMPRESS__INIT;

    // Send the file descriptors and get nonces
    int in_nonce = send_fd(sock, in);
    int out_nonce = send_fd(sock, out);

    msg.fd_in = in_nonce;
    msg.fd_out = out_nonce;
    msg.filename = strdup(filename);

    if (pre)
    {
        msg.prelen = 1;
        msg.pre = pre;
        msg.prelen = prelen;
    }

    Request req = REQUEST__INIT;
    req.type = REQUEST__TYPE__UNCOMPRESS;
    req.uncompress = &msg;

    uint8_t *buf;
    ssize_t size;
    
    size = request__get_packed_size(&req);
    buf = malloc(size);
    request__pack(&req,buf);

    // Send buf to sandbox
    if ( write( sock, buf, size ) < 0 )
    {
        free(buf);
        err(EX_IOERR, "Could not send uncompress request");
    }

    // Clean up send
    free(buf);
    
    // Handle Reponse
    uint8_t rbuf[MAX_MSG_SIZE];

    size = read(sock, &rbuf, MAX_MSG_SIZE);
    if (size < 0)
    {
        err(EX_IOERR, "Could not get uncompress request response: ");
    }
    
    Return* rtn;
    rtn = return__unpack(NULL, size, rbuf);

    off_t bytes_read = 0;
    if (rtn == NULL)
    {
        fprintf(stderr, "Uncompress Return: Buffer could not be decoded. \n");
    }
    else
    {
        if (gsizep) 
        {
            *gsizep = rtn->size;
        }
        bytes_read = rtn->bytes;
    }
    return bytes_read;
}


void gz_compress_unwrapper(Compress* compress, int* fds, int* nonces, int sock)
{
    fprintf(stderr, "Got a compress request. \n");
    int in, out;

    // Get file descriptors
    uint32_t fd_in_nonce = compress->fd_in;
    uint32_t fd_out_nonce = compress->fd_out;

    if ((uint32_t)nonces[0] == fd_in_nonce && (uint32_t)nonces[1] == fd_out_nonce)
    {
        in = fds[0];
        out = fds[1];
    }
    else
    {
        err(1, "File descriptor nonces do not match");
    }

    const char* origname = compress->original_name;
    uint32_t mtime = compress->mtime;
    
    off_t gsizep;
    off_t bytes_read = gz_compress(in, out, &gsizep, origname, mtime); 

    send_response( gsizep, bytes_read, sock );
}

void gz_uncompress_unwrapper(Uncompress* uncompress, int* fds, int* nonces, int sock)
{
    int in, out, prelen;
    off_t gsizep, bytes_written;
    char* pre; 
    const char* filename;

    // Get file descriptors
    uint32_t fd_in_nonce = uncompress->fd_in;
    uint32_t fd_out_nonce = uncompress->fd_out;
    
    if ((uint32_t)nonces[0] == fd_in_nonce && (uint32_t)nonces[1] == fd_out_nonce)
    {
        in = fds[0];
        out = fds[1];
    }
    else
    {
        err(1, "File descriptor nonces do not match");
    }

    // Check to see if pre was sent
    if ( uncompress->has_prelen == 1 )
    {
        pre = uncompress->pre;
        prelen = uncompress->prelen;
    }
    else
    {
        pre = NULL;
        prelen = 0;
    }


    filename = uncompress->filename; 
    bytes_written = gz_uncompress(in, out, pre, prelen, &gsizep, filename);

    send_response( gsizep, bytes_written, sock );
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

void send_response( off_t gsizep, off_t bytes, int sock)
{
    Return rtn = RETURN__INIT;

    rtn.size = gsizep;
    rtn.bytes = bytes;

    uint8_t *buf;
    ssize_t size;
    
    size = return__get_packed_size(&rtn);
    buf = malloc(size);
    return__pack(&rtn, buf);
    fprintf( stderr, "Sending return of size %zd \n", size);
    // Send buf to sandbox
    if ( write( sock, buf, size ) < 0 )
    {
        free(buf);
        err(EX_IOERR, "Could not send compress request");
    }

    // Clean up send
    free(buf);
}
