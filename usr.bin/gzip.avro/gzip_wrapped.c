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

#define AV_COMPRESS     1
#define AV_UNCOMPRESS   2

static const char REQUEST_SCHEMA[] =
"{\
       \"type\": \"record\",\
       \"name\": \"Request\",\
       \"fields\": [\
           {\"name\": \"req_type\", \"type\": \"int\"},\
           {\"name\": \"fd_in\", \"type\": \"long\"},\
           {\"name\": \"fd_out\", \"type\": \"long\"},\
           {\"name\": \"name\", \"type\": \"string\"},\
           {\"name\": \"mtime\", \"type\": [\"null\", \"int\"]},\
           {\"name\": \"pre\", \"type\": [\"null\", \"string\"]},\
           {\"name\": \"prelen\", \"type\": [\"null\", \"int\"]}\
       ]\
}";

static const char RESPONSE_SCHEMA[] = 
"{\
    \"type\": \"record\",\
    \"name\": \"Response\",\
    \"fields\": [\
        {\"name\": \"size\", \"type\": \"long\"},\
        {\"name\": \"bytes\", \"type\": \"long\"}\
    ]\
 }";

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
    char buf[1024];
    if ( read(sock, &buf, sizeof(buf)) < 0 )
    {
        err(EX_IOERR, "Could not get request: ");
    }

    avro_reader_t reader = avro_reader_memory(buf, sizeof(buf));

    avro_schema_t request_schema;
    avro_value_iface_t *iface;
    avro_value_t request;
    initialize_record( &request_schema, iface, &request, REQUEST_SCHEMA);

    
    if ( avro_value_read( reader, &request) )
    {
        fprintf(stderr, "Could not deserialize request \n");
    }
    else
    {
        avro_value_t req_type_value;
        if ( avro_value_get_by_name( &request, "req_type", &req_type_value, NULL ) == 0 )
        {
           int32_t req_type = -1;
           avro_value_get_int( &req_type_value, &req_type );
           if (req_type == AV_COMPRESS)
           {
                gz_compress_unwrapper( &request, fds, nonces, sock );
           }
           else if (req_type == AV_UNCOMPRESS)
           {
                gz_uncompress_unwrapper( &request, fds, nonces, sock );
           }
           else
           {
                fprintf(stderr, "Unknown operation. \n");
           }
        }
    }
    avro_value_decref(&request);
    avro_value_iface_decref(iface);
    avro_schema_decref(request_schema);
    avro_reader_free(reader);
}


off_t gz_compress_wrapper(int in, int out, off_t *gsizep, const char *origname, uint32_t mtime, int sock)
{
    limit_fds(in, out);

    // Send the file descriptors and get nonces
    int in_nonce = send_fd(sock, in);
    int out_nonce = send_fd(sock, out);

    avro_schema_t request_schema;
    avro_value_iface_t *iface;
    avro_value_t request;
    initialize_record( &request_schema, iface, &request, REQUEST_SCHEMA);
    
    avro_value_t    req_type_value,
                    fd_in_value,
                    fd_out_value,
                    name_value,
                    mtime_union_value,
                    mtime_value,
                    pre_union_value,
                    pre_value,
                    prelen_union_value,
                    prelen_value;
   
    if (
        avro_value_get_by_name( &request, "req_type", &req_type_value, NULL )
        || avro_value_get_by_name( &request, "fd_in", &fd_in_value, NULL )
        || avro_value_get_by_name( &request, "fd_out", &fd_out_value, NULL )
        || avro_value_get_by_name( &request, "name", &name_value, NULL )
        || avro_value_get_by_name( &request, "mtime", &mtime_union_value, NULL )
        || avro_value_get_by_name( &request, "pre", &pre_union_value, NULL )
        || avro_value_get_by_name( &request, "prelen", &prelen_union_value, NULL )
        )
    {
        fprintf(stderr, "%s \n", avro_strerror());
        exit(EXIT_FAILURE);
    }
    else
    {
        avro_value_set_int( &req_type_value, AV_COMPRESS );
        avro_value_set_long( &fd_in_value, in_nonce );
        avro_value_set_long( &fd_out_value, out_nonce );
        avro_value_set_string( &name_value, origname );
        
        avro_value_set_branch(&mtime_union_value, 1, &mtime_value);
        avro_value_set_int( &mtime_value, mtime );
        
        avro_value_set_branch(&pre_union_value, 0, &pre_value);
        avro_value_set_null( &pre_value );

        avro_value_set_branch(&prelen_union_value, 0, &prelen_value);
        avro_value_set_null( &prelen_value );
    }

    char buf[1024];
    avro_writer_t writer;
    if ( (writer = avro_writer_memory( buf, sizeof(buf))) == NULL )
    {
        err(EX_OSERR, "Could not create memory writer");
    }
    if (avro_value_write(writer, &request) != 0) {
        err(EX_OSERR, "Failed to write value into the buffer\n");
    }
   
    // Send buf to sandbox
    int len = avro_writer_tell(writer); 
    if ( write( sock, buf, len ) < 0 )
    {
        err(EX_IOERR, "Could not send compress request");
    }

    // Clean up send
    avro_value_decref(&request);
    avro_value_iface_decref(iface);
    avro_schema_decref(request_schema);
    
    
    // Handle Reponse
    char rbuf[1024];
    if (read(sock, &rbuf, sizeof(rbuf)) < 0)
    {
        perror("Read response: ");
        err(EX_IOERR, "Could not get compress request response: ");
    }

    off_t bytes_read = 0;

    avro_reader_t reader = avro_reader_memory(rbuf, sizeof(rbuf));
    
    avro_schema_t response_schema;
    avro_value_iface_t *response_iface;
    avro_value_t response;
    initialize_record( &response_schema, response_iface, &response, RESPONSE_SCHEMA);
    
    if ( avro_value_read( reader, &response) )
    {
        err(EX_IOERR,  "Could not deserialize respose \n");
    }
    else
    {
        avro_value_t size_value, bytes_value;
        if ( 
            avro_value_get_by_name( &response, "size", &size_value, NULL ) 
            || avro_value_get_by_name( &response, "bytes", &bytes_value, NULL )
           )
        {
            err(EX_IOERR,  "Could not read response \n");
        }
        else
        {
            avro_value_get_long( &size_value, gsizep );
            avro_value_get_long( &bytes_value, &bytes_read );
        }
    }
    avro_schema_decref(response_schema);
    avro_value_iface_decref(response_iface);
    avro_value_decref( &response );
    avro_reader_free(reader);
    return bytes_read;
}

off_t gz_uncompress_wrapper(int in, int out, char *pre, size_t prelen, off_t *gsizep,
	      const char *filename, int sock)
{ 
    limit_fds(in, out);

    // Send the file descriptors and get nonces
    int in_nonce = send_fd(sock, in);
    int out_nonce = send_fd(sock, out);

    avro_schema_t request_schema;
    avro_value_iface_t *iface;
    avro_value_t request;
    initialize_record( &request_schema, iface, &request, REQUEST_SCHEMA);

    avro_value_t    req_type_value,
                    fd_in_value,
                    fd_out_value,
                    name_value,
                    mtime_union_value,
                    mtime_value,
                    pre_union_value,
                    pre_value,
                    prelen_union_value,
                    prelen_value;
    
    if (
        avro_value_get_by_name( &request, "req_type", &req_type_value, NULL )
        || avro_value_get_by_name( &request, "fd_in", &fd_in_value, NULL )
        || avro_value_get_by_name( &request, "fd_out", &fd_out_value, NULL )
        || avro_value_get_by_name( &request, "name", &name_value, NULL )
        || avro_value_get_by_name( &request, "mtime", &mtime_union_value, NULL )
        || avro_value_get_by_name( &request, "pre", &pre_union_value, NULL )
        || avro_value_get_by_name( &request, "prelen", &prelen_union_value, NULL )
        )
    {
        fprintf(stderr, "%s \n", avro_strerror());
        exit(EXIT_FAILURE);
    }
    else
    {
        avro_value_set_int( &req_type_value, AV_UNCOMPRESS );
        avro_value_set_long( &fd_in_value, in_nonce );
        avro_value_set_long( &fd_out_value, out_nonce );
        avro_value_set_string( &name_value, filename );
        
        avro_value_set_branch(&mtime_union_value, 0, &mtime_value);
        avro_value_set_null( &mtime_value );
       
        if (pre)
        {
            avro_value_set_branch(&pre_union_value, 1, &pre_value);
            avro_value_set_string( &pre_value, pre );

            avro_value_set_branch(&prelen_union_value, 1, &prelen_value);
            avro_value_set_int( &prelen_value, prelen );
        }
        else
        {
            avro_value_set_branch(&pre_union_value, 0, &pre_value);
            avro_value_set_null( &pre_value );

            avro_value_set_branch(&prelen_union_value, 0, &prelen_value);
            avro_value_set_null( &prelen_value );
        }
    }

    char buf[1024];
    avro_writer_t writer;
    if ( (writer = avro_writer_memory( buf, sizeof(buf))) == NULL )
    {
        err(EX_OSERR, "Could not create memory writer");
    }
    if (avro_value_write(writer, &request) != 0) {
        err(EX_OSERR, "Failed to write value into the buffer\n");
    }
   
    // Send buf to sandbox
    int len = avro_writer_tell(writer); 
    if ( write( sock, buf, len ) < 0 )
    {
        err(EX_IOERR, "Could not send compress request");
    }

    // Clean up send
    avro_value_decref(&request);
    avro_value_iface_decref(iface);
    avro_schema_decref(request_schema);
    
    
    // Handle Reponse
    char rbuf[1024];
    if (read(sock, &rbuf, sizeof(rbuf)) < 0)
    {
        err(EX_IOERR, "Could not get compress request response: ");
    }

    off_t bytes_read = 0;

    avro_reader_t reader = avro_reader_memory(rbuf, sizeof(rbuf));
    
    avro_schema_t response_schema;
    avro_value_iface_t *response_iface;
    avro_value_t response;
    initialize_record( &response_schema, response_iface, &response, RESPONSE_SCHEMA);
    
    if ( avro_value_read( reader, &response) )
    {
        err(EX_IOERR,  "Could not deserialize respose \n");
    }
    else
    {
        avro_value_t size_value, bytes_value;
        if ( 
            avro_value_get_by_name( &response, "size", &size_value, NULL ) 
            || avro_value_get_by_name( &response, "bytes", &bytes_value, NULL )
           )
        {
            err(EX_IOERR,  "Could not read response \n");
        }
        else
        {
            if (gsizep)
            {
                avro_value_get_long( &size_value, gsizep );
            }
            avro_value_get_long( &bytes_value, &bytes_read );
        }
    }
    
    avro_schema_decref(response_schema);
    avro_value_iface_decref(response_iface);
    avro_value_decref( &response );
    avro_reader_free(reader);
    return bytes_read;
}


void gz_compress_unwrapper(avro_value_t* request, int* fds, int* nonces, int sock)
{
    int in, out;

    // Get file descriptors
    avro_value_t fd_in_nonce_value;
    avro_value_t fd_out_nonce_value;
    int64_t fd_in_nonce = 0;
    int64_t fd_out_nonce = 0;

    if (avro_value_get_by_name( request, "fd_in", &fd_in_nonce_value, NULL ) == 0 )
    {
        avro_value_get_long(&fd_in_nonce_value, &fd_in_nonce);
    }
    if (avro_value_get_by_name( request, "fd_out", &fd_out_nonce_value, NULL ) == 0 )
    {
        avro_value_get_long(&fd_out_nonce_value, &fd_out_nonce);
    }

    if (nonces[0] == fd_in_nonce && nonces[1] == fd_out_nonce)
    {
        in = fds[0];
        out = fds[1];
    }
    else
    {
        err(1, "File descriptor nonces do not match");
    }

    avro_value_t origname_value;
    const char* origname;
    if (avro_value_get_by_name( request, "filename", &origname_value, NULL ) == 0 )
    {
        avro_value_get_string(&origname_value, &origname, NULL);
    }
    
    avro_value_t mtime_value;
    uint32_t mtime = 0;
    if (avro_value_get_by_name( request, "mtime", &mtime_value, NULL ) == 0 )
    {
        avro_value_get_int(&mtime_value, &mtime);
    }
    
    avro_value_decref(request);
    
    off_t gsizep;
    off_t bytes_read = gz_compress(in, out, &gsizep, origname, mtime); 

    send_response( gsizep, bytes_read, sock );
}

void gz_uncompress_unwrapper(avro_value_t* request, int* fds, int* nonces, int sock)
{
    int in = 0, out = 0, prelen = 0;
    off_t gsizep = 0, bytes_written = 0;
    char* pre;
    const char* c_pre;
    const char* filename;

    // Get file descriptors
    avro_value_t fd_in_nonce_value;
    avro_value_t fd_out_nonce_value;
    int64_t fd_in_nonce = 0;
    int64_t fd_out_nonce = 0;

    if (avro_value_get_by_name( request, "fd_in", &fd_in_nonce_value, NULL ) == 0 )
    {
        avro_value_get_long(&fd_in_nonce_value, &fd_in_nonce);
    }
    if (avro_value_get_by_name( request, "fd_out", &fd_out_nonce_value, NULL ) == 0 )
    {
        avro_value_get_long(&fd_out_nonce_value, &fd_out_nonce);
    }
    
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
    avro_value_t pre_union_value;
    if ( avro_value_get_by_name( request, "pre", &pre_union_value, NULL ) == 0 )
    {
        int disc;
        if ( avro_value_get_discriminant( &pre_union_value, &disc ) == 0 )
        {
            if (disc == 1)
            {
                avro_value_t prelen_union_value;
                avro_value_t pre_value, prelen_value;
                if ( avro_value_get_current_branch( &pre_union_value, &pre_value ) == 0 )
                {
                    avro_value_get_string( &pre_value, &c_pre, NULL );
                }
               
                if ( avro_value_get_by_name( request, "prelen", &prelen_union_value, NULL ) == 0 )
                {
                    if ( avro_value_get_current_branch( &prelen_union_value, &prelen_value ) == 0 )
                    {
                        avro_value_get_int( &prelen_value, &prelen );
                    }
                }
            }
            else
            {
                c_pre = NULL;
                prelen = 0;
            }
        }
    }

    
    avro_value_t filename_value;
    if (avro_value_get_by_name( request, "name", &filename_value, NULL ) == 0 )
    {
        avro_value_get_string(&filename_value, &filename, NULL);
    }
   

    if (c_pre) 
    {
        pre = strdup(c_pre);
    }


    bytes_written = gz_uncompress(in, out, pre, prelen, &gsizep, filename);
    send_response( gsizep, bytes_written, sock );

    avro_value_decref(request);
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
    avro_schema_t response_schema;
    avro_value_iface_t *iface;
    avro_value_t response;
    initialize_record( &response_schema, iface, &response, RESPONSE_SCHEMA);

    avro_value_t size_value, bytes_value;
    
    
    if (avro_value_get_by_name( &response, "size", &size_value, NULL ) == 0 )
    {
        avro_value_set_long( &size_value, gsizep );
    }
    if (avro_value_get_by_name( &response, "bytes", &bytes_value, NULL ) == 0 )
    {
        avro_value_set_long( &bytes_value, bytes );
    }

    char buf[1024];
    avro_writer_t writer;
    if ( (writer = avro_writer_memory( buf, sizeof(buf))) == NULL )
    {
        err(EX_OSERR, "Could not create memory writer");
    }
    if (avro_value_write(writer, &response) != 0) {
        err(EX_OSERR, "Failed to write value into the buffer\n");
    }
   
    // Send buf to sandbox
    int len = avro_writer_tell(writer); 
    if ( write( sock, buf, len ) < 0 )
    {
        perror("Write response: ");
        err(EX_IOERR, "Could not send response");
    }

    // Clean up send
    avro_value_decref(&response);
    avro_value_iface_decref(iface);
    avro_schema_decref(response_schema);
}

void initialize_record(avro_schema_t *schema, avro_value_iface_t *iface, avro_value_t *value, const char* json )
{
    if (avro_schema_from_json_length(json, strlen(json), schema)) {
        fprintf(stderr, "%s \n %s \n", json, avro_strerror());
        exit(EXIT_FAILURE);
    }
    iface = avro_generic_class_from_schema(*schema);

    if ( avro_generic_value_new(iface, value) )
    {
        fprintf(stderr, "%s \n", avro_strerror());
    }
}
