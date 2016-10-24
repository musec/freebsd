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

    // Receive Flatbuffer for Operation
    uint8_t buf[255];
    if ( read(sock, &buf, 255) < 0 )
    {
        perror("Read request: ");
        err(EX_IOERR, "Could not get request: ");
    }

    ns(Message_table_t) message = ns(Message_as_root(buf));
    
    if ( message != 0 ) 
    {
        if (ns(Message_data_type(message) == ns(Data_Compress)))
        {
            ns(Compress_table_t) compress = ns(Message_data(message));
            gz_compress_unwrapper( compress, fds, nonces, sock );  
        }
        else if (ns(Message_data_type(message) == ns(Data_Uncompress)))
        {
            ns(Uncompress_table_t) uncompress = ns(Message_data(message));
            gz_uncompress_unwrapper( uncompress, fds, nonces, sock );
        }
        else 
        {
            fprintf(stderr, "Unknown operation. \n");
        }
    }
    else
    {
        fprintf(stderr, "Buffer could not be decoded. \n");
    }
}


off_t gz_compress_wrapper(int in, int out, off_t *gsizep, const char *origname, uint32_t mtime, int sock)
{
    limit_fds(in, out);

    // Send the file descriptors and get nonces
    int in_nonce = send_fd(sock, in);
    int out_nonce = send_fd(sock, out);

    flatcc_builder_t builder, *B;
    B = &builder;

    // Init the builder
    flatcc_builder_init(B);

    // Build the flatbuffer
    uint64_t fd_in = in_nonce;
    uint64_t fd_out = out_nonce;
    ns(Compress_ref_t) original_filename = flatbuffers_string_create_str(B, origname);

    ns(Compress_ref_t) request = ns(Compress_create(B, fd_in, fd_out, original_filename, mtime));
    ns(Data_union_ref_t) data = ns(Data_as_Compress(request));
    ns(Message_create_as_root(B, data));

    uint8_t *buf;
    size_t size;

    buf = flatcc_builder_finalize_buffer(B, &size);

    // Send the buffer
    if ( write( sock, buf, size ) < 0 )
    {
        perror("Write compress request: ");
        free(buf);
        flatcc_builder_clear(B);
        err(EX_IOERR, "Could not send compress request");
    }

    // Clean up
    free(buf);
    flatcc_builder_clear(B);

    // Handle Reponse
    uint8_t rbuf[255];
    if (read(sock, &rbuf, 255) < 0)
    {
        perror("Read response: ");
        err(EX_IOERR, "Could not get compress request response: ");
    }


    ns(Message_table_t) message = ns(Message_as_root(rbuf));
    ns(Return_table_t) rtn = ns(Message_data(message));

    *gsizep = ns(Return_size(rtn));
    off_t bytes_read = (off_t)(ns(Return_bytes(rtn)));
    return bytes_read;
}



off_t gz_uncompress_wrapper(int in, int out, char *pre, size_t prelen, off_t *gsizep,
	      const char *filename, int sock)
{
    limit_fds(in, out);

    // Send the file descriptors and get nonces
    int in_nonce = send_fd(sock, in);
    int out_nonce = send_fd(sock, out);

    flatcc_builder_t builder, *B;
    B = &builder;

    // Init the builder
    flatcc_builder_init(B);

    // Build the flatbuffer
    ns(Uncompress_ref_t) request;

    uint64_t fd_in = in_nonce;
    uint64_t fd_out = out_nonce;
    ns(Uncompress_ref_t) fname = flatbuffers_string_create_str(B, filename);
    if (pre)
    {
        request = ns(Uncompress_create(B, fd_in, fd_out, fname, pre, prelen));   
    }
    else
    {
        ns(Uncompress_start(B));
        ns(Uncompress_fd_in_add(B, fd_in));
        ns(Uncompress_fd_out_add(B, fd_out));
        ns(Uncompress_filename_add(B, fname));
        request =  ns(Uncompress_end(B));
    }
    
    ns(Data_union_ref_t) data = ns(Data_as_Uncompress(request));
    ns(Message_create_as_root(B, data));

    uint8_t *buf;
    size_t size;

    buf = flatcc_builder_finalize_buffer(B, &size);
    
    // Send buf to sandbox
    if ( write( sock, buf, size ) < 0 )
    {
        perror("Write uncompress request: ");
        free(buf);
        flatcc_builder_clear(B);
        err(EX_IOERR, "Could not send uncompress request");
    }

    // Clean up send
    free(buf);
    flatcc_builder_clear(B);
    
    // Handle Reponse
    uint8_t rbuf[255];
    if (read(sock, &rbuf, 255) < 0)
    {
        perror("Read response: ");
        err(EX_IOERR, "Could not get uncompress request response: ");
    }
    
    ns(Message_table_t) message = ns(Message_as_root(rbuf));
    ns(Return_table_t) rtn = ns(Message_data(message));

    if (gsizep)
    {
        *gsizep = ns(Return_size(rtn));
    }
    off_t bytes_read = ns(Return_bytes(rtn));
    return bytes_read;
}


void gz_compress_unwrapper(ns(Compress_table_t) compress, int* fds, int* nonces, int sock)
{
    int in, out;

    // Get file descriptors
    uint64_t fd_in_nonce = ns(Compress_fd_in(compress));
    uint64_t fd_out_nonce = ns(Compress_fd_out(compress));

    if (nonces[0] == fd_in_nonce && nonces[1] == fd_out_nonce)
    {
        in = fds[0];
        out = fds[1];
    }
    else
    {
        err(1, "File descriptor nonces do not match");
    }

    const char* origname = ns(Compress_original_name(compress));
    uint32_t mtime = ns(Compress_mtime(compress));
    
    off_t gsizep;
    off_t bytes_read = gz_compress(in, out, &gsizep, origname, mtime); 

    send_response( gsizep, bytes_read, sock );
}

void gz_uncompress_unwrapper(ns(Uncompress_table_t) uncompress, int* fds, int* nonces, int sock)
{
    int in, out, prelen;
    off_t gsizep, bytes_written;
    char* pre; 
    const char* filename;

    // Get file descriptors
    uint64_t fd_in_nonce = ns(Uncompress_fd_in(uncompress));
    uint64_t fd_out_nonce = ns(Uncompress_fd_out(uncompress));
    
    if (nonces[0] == fd_in_nonce && nonces[1] == fd_out_nonce)
    {
        in = fds[0];
        out = fds[1];
    }
    else
    {
        err(1, "File descriptor nonces do not match");
    }

    // Check to see if pre was sent
    if ( ns(Uncompress_pre_is_present(uncompress)) )
    {
        pre = ns(Uncompress_pre(compress));
        prelen = ns(Uncompress_prelen(compress));
    }
    else
    {
        pre = NULL;
        prelen = 0;
    }


    filename = ns(Uncompress_filename(uncompress)); 
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
    flatcc_builder_t builder, *B;
    B = &builder;

    // Init the builder
    flatcc_builder_init(B);

    ns(Return_ref_t) rtn = ns(Return_create(B, gsizep, bytes)); 
    ns(Data_union_ref_t) data = ns(Data_as_Return(rtn));
    ns(Message_create_as_root(B, data));

    uint8_t *buf;
    size_t size;

    buf = flatcc_builder_finalize_buffer(B, &size);
    
    // Send buf to sandbox
    if ( write( sock, buf, size ) < 0 )
    {
        perror("Write response: ");
        free(buf);
        flatcc_builder_clear(B);
        err(EX_IOERR, "Could not send compress request");
    }

    // Clean up send
    free(buf);
    flatcc_builder_clear(B);
}
