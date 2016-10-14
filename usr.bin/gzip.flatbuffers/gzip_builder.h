#ifndef GZIP_BUILDER_H
#define GZIP_BUILDER_H

/* Generated by flatcc 0.3.6-dev FlatBuffers schema compiler for C by dvide.com */

#ifndef GZIP_READER_H
#include "gzip_reader.h"
#endif
#ifndef FLATBUFFERS_COMMON_BUILDER_H
#include "flatbuffers_common_builder.h"
#endif
#define PDIAGNOSTIC_IGNORE_UNUSED
#include "flatcc/portable/pdiagnostic_push.h"
#ifndef flatbuffers_identifier
#define flatbuffers_identifier 0
#endif
#ifndef flatbuffers_extension
#define flatbuffers_extension ".bin"
#endif

typedef struct Gzip_Data_union_ref Gzip_Data_union_ref_t;

static const flatbuffers_voffset_t __Gzip_Compress_required[] = {0 };
__flatbuffers_build_table(flatbuffers_, Gzip_Compress, 4)
static const flatbuffers_voffset_t __Gzip_Uncompress_required[] = {0 };
__flatbuffers_build_table(flatbuffers_, Gzip_Uncompress, 5)
static const flatbuffers_voffset_t __Gzip_Return_required[] = {0 };
__flatbuffers_build_table(flatbuffers_, Gzip_Return, 2)
static const flatbuffers_voffset_t __Gzip_Message_required[] = {0 };
__flatbuffers_build_table(flatbuffers_, Gzip_Message, 2)
#define __Gzip_Compress_formal_args , uint64_t v0, uint64_t v1, flatbuffers_string_ref_t v2, uint32_t v3
#define __Gzip_Compress_call_args , v0, v1, v2, v3
static inline Gzip_Compress_ref_t Gzip_Compress_create(flatbuffers_builder_t *B __Gzip_Compress_formal_args);
#define __Gzip_Uncompress_formal_args ,\
  uint64_t v0, uint64_t v1, flatbuffers_string_ref_t v2, flatbuffers_string_ref_t v3, uint32_t v4
#define __Gzip_Uncompress_call_args ,\
  v0, v1, v2, v3, v4
static inline Gzip_Uncompress_ref_t Gzip_Uncompress_create(flatbuffers_builder_t *B __Gzip_Uncompress_formal_args);
#define __Gzip_Return_formal_args , uint32_t v0, uint32_t v1
#define __Gzip_Return_call_args , v0, v1
static inline Gzip_Return_ref_t Gzip_Return_create(flatbuffers_builder_t *B __Gzip_Return_formal_args);
#define __Gzip_Message_formal_args , Gzip_Data_union_ref_t v1
#define __Gzip_Message_call_args , v1
static inline Gzip_Message_ref_t Gzip_Message_create(flatbuffers_builder_t *B __Gzip_Message_formal_args);

struct Gzip_Data_union_ref {
    Gzip_Data_union_type_t type;
    union {
        flatbuffers_ref_t _member;
        flatbuffers_ref_t NONE;
        Gzip_Compress_ref_t Compress;
        Gzip_Uncompress_ref_t Uncompress;
        Gzip_Return_ref_t Return;
    };
};

static inline Gzip_Data_union_ref_t Gzip_Data_as_NONE()
{ Gzip_Data_union_ref_t uref; uref.type = Gzip_Data_NONE; uref._member = 0; return uref; }
static inline Gzip_Data_union_ref_t Gzip_Data_as_Compress(Gzip_Compress_ref_t ref)
{ Gzip_Data_union_ref_t uref; uref.type = Gzip_Data_Compress; uref.Compress = ref; return uref; }
static inline Gzip_Data_union_ref_t Gzip_Data_as_Uncompress(Gzip_Uncompress_ref_t ref)
{ Gzip_Data_union_ref_t uref; uref.type = Gzip_Data_Uncompress; uref.Uncompress = ref; return uref; }
static inline Gzip_Data_union_ref_t Gzip_Data_as_Return(Gzip_Return_ref_t ref)
{ Gzip_Data_union_ref_t uref; uref.type = Gzip_Data_Return; uref.Return = ref; return uref; }

__flatbuffers_build_scalar_field(0, flatbuffers_, Gzip_Compress_fd_in, flatbuffers_uint64, uint64_t, 8, 8, 0)
__flatbuffers_build_scalar_field(1, flatbuffers_, Gzip_Compress_fd_out, flatbuffers_uint64, uint64_t, 8, 8, 0)
__flatbuffers_build_string_field(2, flatbuffers_, Gzip_Compress_orignal_name)
__flatbuffers_build_scalar_field(3, flatbuffers_, Gzip_Compress_mtime, flatbuffers_uint32, uint32_t, 4, 4, 0)

static inline Gzip_Compress_ref_t Gzip_Compress_create(flatbuffers_builder_t *B __Gzip_Compress_formal_args)
{
    if (Gzip_Compress_start(B)
        || Gzip_Compress_fd_in_add(B, v0)
        || Gzip_Compress_fd_out_add(B, v1)
        || Gzip_Compress_orignal_name_add(B, v2)
        || Gzip_Compress_mtime_add(B, v3)) {
        return 0;
    }
    return Gzip_Compress_end(B);
}
__flatbuffers_build_table_prolog(flatbuffers_, Gzip_Compress, Gzip_Compress_identifier, Gzip_Compress_type_identifier)

__flatbuffers_build_scalar_field(0, flatbuffers_, Gzip_Uncompress_fd_in, flatbuffers_uint64, uint64_t, 8, 8, 0)
__flatbuffers_build_scalar_field(1, flatbuffers_, Gzip_Uncompress_fd_out, flatbuffers_uint64, uint64_t, 8, 8, 0)
__flatbuffers_build_string_field(2, flatbuffers_, Gzip_Uncompress_filename)
__flatbuffers_build_string_field(3, flatbuffers_, Gzip_Uncompress_pre)
__flatbuffers_build_scalar_field(4, flatbuffers_, Gzip_Uncompress_prelen, flatbuffers_uint32, uint32_t, 4, 4, 0)

static inline Gzip_Uncompress_ref_t Gzip_Uncompress_create(flatbuffers_builder_t *B __Gzip_Uncompress_formal_args)
{
    if (Gzip_Uncompress_start(B)
        || Gzip_Uncompress_fd_in_add(B, v0)
        || Gzip_Uncompress_fd_out_add(B, v1)
        || Gzip_Uncompress_filename_add(B, v2)
        || Gzip_Uncompress_pre_add(B, v3)
        || Gzip_Uncompress_prelen_add(B, v4)) {
        return 0;
    }
    return Gzip_Uncompress_end(B);
}
__flatbuffers_build_table_prolog(flatbuffers_, Gzip_Uncompress, Gzip_Uncompress_identifier, Gzip_Uncompress_type_identifier)

__flatbuffers_build_scalar_field(0, flatbuffers_, Gzip_Return_size, flatbuffers_uint32, uint32_t, 4, 4, 0)
__flatbuffers_build_scalar_field(1, flatbuffers_, Gzip_Return_bytes_read, flatbuffers_uint32, uint32_t, 4, 4, 0)

static inline Gzip_Return_ref_t Gzip_Return_create(flatbuffers_builder_t *B __Gzip_Return_formal_args)
{
    if (Gzip_Return_start(B)
        || Gzip_Return_size_add(B, v0)
        || Gzip_Return_bytes_read_add(B, v1)) {
        return 0;
    }
    return Gzip_Return_end(B);
}
__flatbuffers_build_table_prolog(flatbuffers_, Gzip_Return, Gzip_Return_identifier, Gzip_Return_type_identifier)

__flatbuffers_build_union_field(1, flatbuffers_, Gzip_Message_data, Gzip_Data)
__flatbuffers_build_union_member_field(flatbuffers_, Gzip_Message_data, Gzip_Data, Compress, Gzip_Compress)
__flatbuffers_build_union_member_field(flatbuffers_, Gzip_Message_data, Gzip_Data, Uncompress, Gzip_Uncompress)
__flatbuffers_build_union_member_field(flatbuffers_, Gzip_Message_data, Gzip_Data, Return, Gzip_Return)

static inline Gzip_Message_ref_t Gzip_Message_create(flatbuffers_builder_t *B __Gzip_Message_formal_args)
{
    if (Gzip_Message_start(B)
        || Gzip_Message_data_add_member(B, v1)
        || Gzip_Message_data_add_type(B, v1.type)) {
        return 0;
    }
    return Gzip_Message_end(B);
}
__flatbuffers_build_table_prolog(flatbuffers_, Gzip_Message, Gzip_Message_identifier, Gzip_Message_type_identifier)

#include "flatcc/portable/pdiagnostic_pop.h"
#endif /* GZIP_BUILDER_H */
