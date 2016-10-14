#ifndef FLATBUFFERS_COMMON_BUILDER_H
#define FLATBUFFERS_COMMON_BUILDER_H

/* Generated by flatcc 0.3.6-dev FlatBuffers schema compiler for C by dvide.com */

/* Common FlatBuffers build functionality for C. */

#define PDIAGNOSTIC_IGNORE_UNUSED
#include "flatcc/portable/pdiagnostic_push.h"
#ifndef FLATBUILDER_H
#include "flatcc/flatcc_builder.h"
#endif
typedef flatcc_builder_t flatbuffers_builder_t;
typedef flatcc_builder_ref_t flatbuffers_ref_t;
typedef flatcc_builder_ref_t flatbuffers_vec_ref_t;
/* integer return code (ref and ptr always fail on 0) */
#define flatbuffers_failed(x) ((x) < 0)
typedef flatbuffers_ref_t flatbuffers_root_t;
#define flatbuffers_root(ref) ((flatbuffers_root_t)(ref))

#define __flatbuffers_build_buffer(NS)\
typedef NS ## ref_t NS ## buffer_ref_t;\
static inline int NS ## buffer_start(NS ## builder_t *B, NS ##fid_t fid)\
{ return flatcc_builder_start_buffer(B, fid, 0); }\
static inline int NS ## buffer_start_aligned(NS ## builder_t *B, NS ##fid_t fid, uint16_t block_align)\
{ return flatcc_builder_start_buffer(B, fid, block_align); }\
static inline NS ## buffer_ref_t NS ## buffer_end(NS ## builder_t *B, NS ## ref_t root)\
{ return flatcc_builder_end_buffer(B, root); }

#define __flatbuffers_build_table_root(NS, N, FID, TFID)\
static inline int N ## _start_as_root(NS ## builder_t *B)\
{ return NS ## buffer_start(B, FID) ? -1 : N ## _start(B); }\
static inline int N ## _start_as_typed_root(NS ## builder_t *B)\
{ return NS ## buffer_start(B, TFID) ? -1 : N ## _start(B); }\
static inline NS ## buffer_ref_t N ## _end_as_root(NS ## builder_t *B)\
{ return NS ## buffer_end(B, N ## _end(B)); }\
static inline NS ## buffer_ref_t N ## _end_as_typed_root(NS ## builder_t *B)\
{ return NS ## buffer_end(B, N ## _end(B)); }\
static inline NS ## buffer_ref_t N ## _create_as_root(NS ## builder_t *B __ ## N ## _formal_args)\
{ if (NS ## buffer_start(B, FID)) return 0; return NS ## buffer_end(B, N ## _create(B __ ## N ## _call_args)); }\
static inline NS ## buffer_ref_t N ## _create_as_typed_root(NS ## builder_t *B __ ## N ## _formal_args)\
{ if (NS ## buffer_start(B, TFID)) return 0; return NS ## buffer_end(B, N ## _create(B __ ## N ## _call_args)); }

#define __flatbuffers_build_table_prolog(NS, N, FID, TFID)\
__flatbuffers_build_table_vector_ops(NS, N ## _vec, N)\
__flatbuffers_build_table_root(NS, N, FID, TFID)

#define __flatbuffers_build_struct_root(NS, N, A, FID, TFID)\
static inline N ## _t *N ## _start_as_root(NS ## builder_t *B)\
{ return NS ## buffer_start(B, FID) ? 0 : N ## _start(B); }\
static inline N ## _t *N ## _start_as_typed_root(NS ## builder_t *B)\
{ return NS ## buffer_start(B, TFID) ? 0 : N ## _start(B); }\
static inline NS ## buffer_ref_t N ## _end_as_root(NS ## builder_t *B)\
{ return NS ## buffer_end(B, N ## _end(B)); }\
static inline NS ## buffer_ref_t N ## _end_as_typed_root(NS ## builder_t *B)\
{ return NS ## buffer_end(B, N ## _end(B)); }\
static inline NS ## buffer_ref_t N ## _end_pe_as_root(NS ## builder_t *B)\
{ return NS ## buffer_end(B, N ## _end_pe(B)); }\
static inline NS ## buffer_ref_t N ## _end_pe_as_typed_root(NS ## builder_t *B)\
{ return NS ## buffer_end(B, N ## _end_pe(B)); }\
static inline NS ## buffer_ref_t N ## _create_as_root(NS ## builder_t *B __ ## N ## _formal_args)\
{ return flatcc_builder_create_buffer(B, FID, 0,\
  N ## _create(B __ ## N ## _call_args), A, 0); }\
static inline NS ## buffer_ref_t N ## _create_as_typed_root(NS ## builder_t *B __ ## N ## _formal_args)\
{ return flatcc_builder_create_buffer(B, TFID, 0,\
  N ## _create(B __ ## N ## _call_args), A, 0); }

#define __flatbuffers_build_nested_table_root(NS, N, TN, FID, TFID)\
static inline int N ## _start_as_root(NS ## builder_t *B)\
{ return NS ## buffer_start(B, FID) ? -1 : TN ## _start(B); }\
static inline int N ## _start_as_typed_root(NS ## builder_t *B)\
{ return NS ## buffer_start(B, TFID) ? -1 : TN ## _start(B); }\
static inline int N ## _end_as_root(NS ## builder_t *B)\
{ return N ## _add(B, NS ## buffer_end(B, TN ## _end(B))); }\
static inline int N ## _end_as_typed_root(NS ## builder_t *B)\
{ return N ## _add(B, NS ## buffer_end(B, TN ## _end(B))); }\
static inline int N ## _nest(NS ## builder_t *B, void *data, size_t size, uint16_t align)\
{ if (NS ## buffer_start(B, FID)) return -1;\
  return N ## _add(B, NS ## buffer_end(B, flatcc_builder_create_vector(B, data, size, 1,\
  align ? align : 8, FLATBUFFERS_COUNT_MAX(1)))); }\
static inline int N ## _typed_nest(NS ## builder_t *B, void *data, size_t size, uint16_t align)\
{ if (NS ## buffer_start(B, TFID)) return -1;\
  return N ## _add(B, NS ## buffer_end(B, flatcc_builder_create_vector(B, data, size, 1,\
  align ? align : 8, FLATBUFFERS_COUNT_MAX(1)))); }

#define __flatbuffers_build_nested_struct_root(NS, N, TN, A, FID, TFID)\
static inline TN ## _t *N ## _start_as_root(NS ## builder_t *B)\
{ return NS ## buffer_start(B, FID) ? 0 : TN ## _start(B); }\
static inline TN ## _t *N ## _start_as_typed_root(NS ## builder_t *B)\
{ return NS ## buffer_start(B, FID) ? 0 : TN ## _start(B); }\
static inline int N ## _end_as_root(NS ## builder_t *B)\
{ return N ## _add(B, NS ## buffer_end(B, TN ## _end(B))); }\
static inline int N ## _end_as_typed_root(NS ## builder_t *B)\
{ return N ## _add(B, NS ## buffer_end(B, TN ## _end(B))); }\
static inline int N ## _end_pe_as_root(NS ## builder_t *B)\
{ return N ## _add(B, NS ## buffer_end(B, TN ## _end_pe(B))); }\
static inline int N ## _create_as_root(NS ## builder_t *B __ ## TN ## _formal_args)\
{ return N ## _add(B, flatcc_builder_create_buffer(B, FID, 0,\
static inline int N ## _create_as_typed_root(NS ## builder_t *B __ ## TN ## _formal_args)\
{ return N ## _add(B, flatcc_builder_create_buffer(B, TFID, 0,\
  TN ## _create(B __ ## TN ## _call_args), A, 1)); }\
static inline int N ## _nest(NS ## builder_t *B, void *data, size_t size, uint16_t align)\
{ if (NS ## buffer_start(B, FID)) return -1;\
  return N ## _add(B, NS ## buffer_end(B, flatcc_builder_create_vector(B, data, size, 1,\
  align < A ? A : align, FLATBUFFERS_COUNT_MAX(1)))); }\
static inline int N ## _typed_nest(NS ## builder_t *B, void *data, size_t size, uint16_t align)\
{ if (NS ## buffer_start(B, TFID)) return -1;\
  return N ## _add(B, NS ## buffer_end(B, flatcc_builder_create_vector(B, data, size, 1,\
  align < A ? A : align, FLATBUFFERS_COUNT_MAX(1)))); }

#define __flatbuffers_build_vector_ops(NS, V, N, TN, T)\
static inline T *V ## _extend(NS ## builder_t *B, size_t len)\
{ return flatcc_builder_extend_vector(B, len); }\
static inline T *V ## _append(NS ## builder_t *B, const T *data, size_t len)\
{ return flatcc_builder_append_vector(B, data, len); }\
static inline int V ## _truncate(NS ## builder_t *B, size_t len)\
{ return flatcc_builder_truncate_vector(B, len); }\
static inline T *V ## _edit(NS ## builder_t *B)\
{ return flatcc_builder_vector_edit(B); }\
static inline size_t V ## _reserved_len(NS ## builder_t *B)\
{ return flatcc_builder_vector_count(B); }\
static inline T *V ## _push(NS ## builder_t *B, const T *p)\
{ T *_p; return (_p = flatcc_builder_extend_vector(B, 1)) ? (memcpy(_p, p, TN ## __size()), _p) : 0; }\
static inline T *V ## _push_copy(NS ## builder_t *B, const T *p)\
{ T *_p; return (_p = flatcc_builder_extend_vector(B, 1)) ? TN ## _copy(_p, p) : 0; }\
static inline T *V ## _push_create(NS ## builder_t *B __ ## TN ## _formal_args)\
{ T *_p; return (_p = flatcc_builder_extend_vector(B, 1)) ? TN ## _assign(_p __ ## TN ## _call_args) : 0; }

#define __flatbuffers_build_vector(NS, N, T, S, A)\
typedef NS ## ref_t N ## _vec_ref_t;\
static inline int N ## _vec_start(NS ## builder_t *B)\
{ return flatcc_builder_start_vector(B, S, A, FLATBUFFERS_COUNT_MAX(S)); }\
static inline N ## _vec_ref_t N ## _vec_end_pe(NS ## builder_t *B)\
{ return flatcc_builder_end_vector(B); }\
static inline N ## _vec_ref_t N ## _vec_end(NS ## builder_t *B)\
{ if (!NS ## is_native_pe()) { size_t i, n; T *p = flatcc_builder_vector_edit(B);\
    for (i = 0, n = flatcc_builder_vector_count(B); i < n; ++i)\
    { N ## _to_pe(N ## __ptr_add(p, i)); }} return flatcc_builder_end_vector(B); }\
static inline N ## _vec_ref_t N ## _vec_create_pe(NS ## builder_t *B, const T *data, size_t len)\
{ return flatcc_builder_create_vector(B, data, len, S, A, FLATBUFFERS_COUNT_MAX(S)); }\
static inline N ## _vec_ref_t N ## _vec_create(NS ## builder_t *B, const T *data, size_t len)\
{ if (!NS ## is_native_pe()) { size_t i; T *p; int ret = flatcc_builder_start_vector(B, S, A, FLATBUFFERS_COUNT_MAX(S)); if (ret) { return ret; }\
  p = flatcc_builder_extend_vector(B, len); if (!p) return 0;\
  for (i = 0; i < len; ++i) { N ## _copy_to_pe(N ## __ptr_add(p, i), N ## __const_ptr_add(data, i)); }\
  return flatcc_builder_end_vector(B); } else return flatcc_builder_create_vector(B, data, len, S, A, FLATBUFFERS_COUNT_MAX(S)); }\
static inline N ## _vec_ref_t N ## _vec_clone(NS ## builder_t *B, N ##_vec_t vec)\
{ return flatcc_builder_create_vector(B, vec, N ## _vec_len(vec), S, A, FLATBUFFERS_COUNT_MAX(S)); }\
static inline N ## _vec_ref_t N ## _vec_slice(NS ## builder_t *B, N ##_vec_t vec, size_t index, size_t len)\
{ size_t n = N ## _vec_len(vec); if (index >= n) index = n; n -= index; if (len > n) len = n;\
  return flatcc_builder_create_vector(B, N ## __const_ptr_add(vec, index), len, S, A, FLATBUFFERS_COUNT_MAX(S)); }\
__flatbuffers_build_vector_ops(NS, N ## _vec, N, N, T)

#define __flatbuffers_build_string_vector_ops(NS, N)\
static inline int N ## _push_start(NS ## builder_t *B)\
{ return NS ## string_start(B); }\
static inline NS ## string_ref_t *N ## _push_end(NS ## builder_t *B)\
{ return NS ## string_vec_push(B, NS ## string_end(B)); }\
static inline NS ## string_ref_t *N ## _push_create(NS ## builder_t *B, const char *s, size_t len)\
{ return NS ## string_vec_push(B, NS ## string_create(B, s, len)); }\
static inline NS ## string_ref_t *N ## _push_create_str(NS ## builder_t *B, const char *s)\
{ return NS ## string_vec_push(B, NS ## string_create_str(B, s)); }\
static inline NS ## string_ref_t *N ## _push_create_strn(NS ## builder_t *B, const char *s, size_t max_len)\
{ return NS ## string_vec_push(B, NS ## string_create_strn(B, s, max_len)); }\
static inline NS ## string_ref_t *N ## _push_clone(NS ## builder_t *B, NS ## string_t string)\
{ return NS ## string_vec_push(B, NS ## string_clone(B, string)); }\
static inline NS ## string_ref_t *N ## _push_slice(NS ## builder_t *B, NS ## string_t string, size_t index, size_t len)\
{ return NS ## string_vec_push(B, NS ## string_slice(B, string, index, len)); }

#define __flatbuffers_build_table_vector_ops(NS, N, TN)\
static inline int N ## _push_start(NS ## builder_t *B)\
{ return TN ## _start(B); }\
static inline TN ## _ref_t *N ## _push_end(NS ## builder_t *B)\
{ return N ## _push(B, TN ## _end(B)); }\
static inline TN ## _ref_t *N ## _push_create(NS ## builder_t *B __ ## TN ##_formal_args)\
{ return N ## _push(B, TN ## _create(B __ ## TN ## _call_args)); }

#define __flatbuffers_build_offset_vector_ops(NS, V, N, TN)\
static inline TN ## _ref_t *V ## _extend(NS ## builder_t *B, size_t len)\
{ return flatcc_builder_extend_offset_vector(B, len); }\
static inline TN ## _ref_t *V ## _append(NS ## builder_t *B, const TN ## _ref_t *data, size_t len)\
{ return flatcc_builder_append_offset_vector(B, data, len); }\
static inline int V ## _truncate(NS ## builder_t *B, size_t len)\
{ return flatcc_builder_truncate_offset_vector(B, len); }\
static inline TN ## _ref_t *V ## _edit(NS ## builder_t *B)\
{ return flatcc_builder_offset_vector_edit(B); }\
static inline size_t V ## _reserved_len(NS ## builder_t *B)\
{ return flatcc_builder_offset_vector_count(B); }\
static inline TN ## _ref_t *V ## _push(NS ## builder_t *B, const TN ## _ref_t ref)\
{ return ref ? flatcc_builder_offset_vector_push(B, ref) : 0; }

#define __flatbuffers_build_offset_vector(NS, N)\
typedef NS ## ref_t N ## _vec_ref_t;\
static inline int N ## _vec_start(NS ## builder_t *B)\
{ return flatcc_builder_start_offset_vector(B); }\
static inline N ## _vec_ref_t N ## _vec_end(NS ## builder_t *B)\
{ return flatcc_builder_end_offset_vector(B); }\
static inline N ## _vec_ref_t N ## _vec_create(NS ## builder_t *B, const N ## _ref_t *data, size_t len)\
{ return flatcc_builder_create_offset_vector(B, data, len); }\
__flatbuffers_build_offset_vector_ops(NS, N ## _vec, N, N)

#define __flatbuffers_build_string_ops(NS, N)\
static inline char *N ## _append(NS ## builder_t *B, const char *s, size_t len)\
{ return flatcc_builder_append_string(B, s, len); }\
static inline char *N ## _append_str(NS ## builder_t *B, const char *s)\
{ return flatcc_builder_append_string_str(B, s); }\
static inline char *N ## _append_strn(NS ## builder_t *B, const char *s, size_t len)\
{ return flatcc_builder_append_string_strn(B, s, len); }\
static inline size_t N ## _reserved_len(NS ## builder_t *B)\
{ return flatcc_builder_string_len(B); }\
static inline char *N ## _extend(NS ## builder_t *B, size_t len)\
{ return flatcc_builder_extend_string(B, len); }\
static inline char *N ## _edit(NS ## builder_t *B)\
{ return flatcc_builder_string_edit(B); }\
static inline int N ## _truncate(NS ## builder_t *B, size_t len)\
{ return flatcc_builder_truncate_string(B, len); }

#define __flatbuffers_build_string(NS)\
typedef NS ## ref_t NS ## string_ref_t;\
static inline int NS ## string_start(NS ## builder_t *B)\
{ return flatcc_builder_start_string(B); }\
static inline NS ## string_ref_t NS ## string_end(NS ## builder_t *B)\
{ return flatcc_builder_end_string(B); }\
static inline NS ## ref_t NS ## string_create(NS ## builder_t *B, const char *s, size_t len)\
{ return flatcc_builder_create_string(B, s, len); }\
static inline NS ## ref_t NS ## string_create_str(NS ## builder_t *B, const char *s)\
{ return flatcc_builder_create_string_str(B, s); }\
static inline NS ## ref_t NS ## string_create_strn(NS ## builder_t *B, const char *s, size_t len)\
{ return flatcc_builder_create_string_strn(B, s, len); }\
static inline NS ## string_ref_t NS ## string_clone(NS ## builder_t *B, NS ## string_t string)\
{ return flatcc_builder_create_string(B, string, NS ## string_len(string)); }\
static inline NS ## string_ref_t NS ## string_slice(NS ## builder_t *B, NS ## string_t string, size_t index, size_t len)\
{ size_t n = NS ## string_len(string); if (index >= n) index = n; n -= index; if (len > n) len = n;\
  return flatcc_builder_create_string(B, string + index, len); }\
__flatbuffers_build_string_ops(NS, NS ## string)\
__flatbuffers_build_offset_vector(NS, NS ## string)

#define __flatbuffers_copy_from_pe(P, P2, N) (*(P) = N ## _cast_from_pe(*P2), (P))
#define __flatbuffers_from_pe(P, N) (*(P) = N ## _cast_from_pe(*P), (P))
#define __flatbuffers_copy_to_pe(P, P2, N) (*(P) = N ## _cast_to_pe(*P2), (P))
#define __flatbuffers_to_pe(P, N) (*(P) = N ## _cast_to_pe(*P), (P))
#define __flatbuffers_define_scalar_primitives(NS, N, T)\
static inline T *N ## _from_pe(T *p) { return __ ## NS ## from_pe(p, N); }\
static inline T *N ## _to_pe(T *p) { return __ ## NS ## to_pe(p, N); }\
static inline T *N ## _copy(T *p, const T *p2) { *p = *p2; return p; }\
static inline T *N ## _copy_from_pe(T *p, const T *p2)\
{ return __ ## NS ## copy_from_pe(p, p2, N); }\
static inline T *N ## _copy_to_pe(T *p, const T *p2) \
{ return __ ## NS ## copy_to_pe(p, p2, N); }\
static inline T *N ## _assign(T *p, const T v0) { *p = v0; return p; }\
static inline T *N ## _assign_from_pe(T *p, T v0)\
{ *p = N ## _cast_from_pe(v0); return p; }\
static inline T *N ## _assign_to_pe(T *p, T v0)\
{ *p = N ## _cast_to_pe(v0); return p; }
#define __flatbuffers_build_scalar(NS, N, T)\
__ ## NS ## define_scalar_primitives(NS, N, T)\
__ ## NS ## build_vector(NS, N, T, sizeof(T), sizeof(T))
/* Depends on generated copy_to/from_pe functions, and the type. */
#define __flatbuffers_define_struct_primitives(NS, N)\
static inline N ## _t *N ##_to_pe(N ## _t *p)\
{ if (!NS ## is_native_pe()) { N ## _copy_to_pe(p, p); }; return p; }\
static inline N ## _t *N ##_from_pe(N ## _t *p)\
{ if (!NS ## is_native_pe()) { N ## _copy_from_pe(p, p); }; return p; }\
static inline N ## _t *N ## _clear(N ## _t *p) { return memset(p, 0, N ## __size()); }

/* Depends on generated copy/assign_to/from_pe functions, and the type. */
#define __flatbuffers_build_struct(NS, N, S, A, FID, TFID)\
__ ## NS ## define_struct_primitives(NS, N)\
typedef NS ## ref_t N ## _ref_t;\
static inline N ## _t *N ## _start(NS ## builder_t *B)\
{ return flatcc_builder_start_struct(B, S, A); }\
static inline N ## _ref_t N ## _end(NS ## builder_t *B)\
{ if (!NS ## is_native_pe()) { N ## _to_pe(flatcc_builder_struct_edit(B)); }\
  return flatcc_builder_end_struct(B); }\
static inline N ## _ref_t N ## _end_pe(NS ## builder_t *B)\
{ return flatcc_builder_end_struct(B); }\
static inline N ## _ref_t N ## _create(NS ## builder_t *B __ ## N ## _formal_args)\
{ N ## _t *_p = N ## _start(B); if (!_p) return 0; N ##_assign_to_pe(_p __ ## N ## _call_args);\
  return N ## _end(B); }\
__flatbuffers_build_vector(NS, N, N ## _t, S, A)\
__flatbuffers_build_struct_root(NS, N, A, FID, TFID)

#define __flatbuffers_build_table(NS, N, K)\
typedef NS ## ref_t N ## _ref_t;\
static inline int N ## _start(NS ## builder_t *B)\
{ return flatcc_builder_start_table(B, K); }\
static inline N ## _ref_t N ## _end(NS ## builder_t *B)\
{ assert(flatcc_builder_check_required(B, __ ## N ## _required,\
  sizeof(__ ## N ## _required) / sizeof(__ ## N ## _required[0]) - 1));\
  return flatcc_builder_end_table(B); }\
__flatbuffers_build_offset_vector(NS, N)

#define __flatbuffers_build_table_field(ID, NS, N, TN)\
static inline int N ## _add(NS ## builder_t *B, TN ## _ref_t ref)\
{ TN ## _ref_t *_p; return (ref && (_p = flatcc_builder_table_add_offset(B, ID))) ?\
  ((*_p = ref), 0) : -1; }\
static inline int N ## _start(NS ## builder_t *B)\
{ return TN ## _start(B); }\
static inline int N ## _end(NS ## builder_t *B)\
{ return N ## _add(B, TN ## _end(B)); }\
static inline TN ## _ref_t N ## _create(NS ## builder_t *B __ ## TN ##_formal_args)\
{ return N ## _add(B, TN ## _create(B __ ## TN ## _call_args)); }

#define __flatbuffers_build_union_field(ID, NS, N, TN)\
static inline int N ## _add(NS ## builder_t *B, TN ## _union_ref_t uref)\
{ NS ## ref_t *_p; TN ## _union_type_t *_pt; if (uref.type == TN ## _NONE) return 0; if (uref._member == 0) return -1;\
  if (!(_pt = flatcc_builder_table_add(B, ID - 1, sizeof(*_pt), sizeof(_pt))) ||\
  !(_p = flatcc_builder_table_add_offset(B, ID))) return -1; *_pt = uref.type; *_p = uref._member; return 0; }\
static inline int N ## _add_type(NS ## builder_t *B, TN ## _union_type_t type)\
{ TN ## _union_type_t *_pt; if (type == TN ## _NONE) return 0; return (_pt = flatcc_builder_table_add(B, ID - 1,\
  sizeof(*_pt), sizeof(*_pt))) ? ((*_pt = type), 0) : -1; }\
static inline int N ## _add_member(NS ## builder_t *B, TN ## _union_ref_t uref)\
{ NS ## ref_t *p; if (uref.type == TN ## _NONE) return 0; return (p = flatcc_builder_table_add_offset(B, ID)) ?\
  ((*p = uref._member), 0) : -1; }

/* M is the union member name and T is its type, i.e. the qualified name. */
#define __flatbuffers_build_union_member_field(NS, N, NU, M, T)\
static inline int N ## _ ## M ## _add(NS ## builder_t *B, T ## _ref_t ref)\
{ return N ## _add(B, NU ## _as_ ## M (ref)); }\
static inline int N ## _ ## M ## _start(NS ## builder_t *B)\
{ return T ## _start(B); }\
static inline int N ## _ ## M ## _end(NS ## builder_t *B)\
{ return N ## _ ## M ## _add(B, T ## _end(B)); }

/* NS: common namespace, ID: table field id (not offset), TN: name of type T,
 * S: sizeof of scalar type, A: alignment of type T, default value V of type T. */
#define __flatbuffers_build_scalar_field(ID, NS, N, TN, T, S, A, V)\
static inline int N ## _add(NS ## builder_t *B, const T v)\
{ T *_p; if (v == V) return 0; if (!(_p = flatcc_builder_table_add(B, ID, S, A))) return -1;\
  TN ## _assign_to_pe(_p, v); return 0; }\
static inline int N ## _force_add(NS ## builder_t *B, const T v)\
{ T *_p; if (!(_p = flatcc_builder_table_add(B, ID, S, A))) return -1;\
  TN ## _assign_to_pe(_p, v); return 0; }\

#define __flatbuffers_build_struct_field(ID, NS, N, TN, S, A)\
static inline TN ## _t *N ## _start(NS ## builder_t *B)\
{ return flatcc_builder_table_add(B, ID, S, A); }\
static inline int N ## _end(NS ## builder_t *B)\
{ if (!NS ## is_native_pe()) { TN ## _to_pe(flatcc_builder_table_edit(B, S)); } return 0; }\
static inline int N ## _end_pe(NS ## builder_t *B) { return 0; }\
static inline int N ## _create(NS ## builder_t *B __ ## TN ## _formal_args)\
{ TN ## _t *_p = N ## _start(B); if (!_p) return 0; TN ##_assign_to_pe(_p __ ## TN ## _call_args);\
 return 0; }\
static inline int N ## _add(NS ## builder_t *B, const TN ## _t *p)\
{ TN ## _t *_p = N ## _start(B); if (!_p) return -1; TN ##_copy_to_pe(_p, p); return 0; }\
static inline int N ## _clone(NS ## builder_t *B, TN ## _struct_t p)\
{ return 0 == flatcc_builder_table_add_copy(B, ID, p, S, A) ? -1 : 0; }

#define __flatbuffers_build_vector_field(ID, NS, N, TN, T)\
static inline int N ## _add(NS ## builder_t *B, TN ## _vec_ref_t ref)\
{ TN ## _vec_ref_t *_p; return (ref && (_p = flatcc_builder_table_add_offset(B, ID))) ? ((*_p = ref), 0) : -1; }\
static inline int N ## _start(NS ## builder_t *B)\
{ return TN ## _vec_start(B); }\
static inline int N ## _end_pe(NS ## builder_t *B)\
{ return N ## _add(B, TN ## _vec_end_pe(B)); }\
static inline int N ## _end(NS ## builder_t *B)\
{ return N ## _add(B, TN ## _vec_end(B)); }\
static inline int N ## _create_pe(NS ## builder_t *B, T *data, size_t len)\
{ return N ## _add(B, TN ## _vec_create_pe(B, data, len)); }\
static inline int N ## _create(NS ## builder_t *B, T *data, size_t len)\
{ return N ## _add(B, TN ## _vec_create(B, data, len)); }\
static inline int N ## _clone(NS ## builder_t *B, TN ## _vec_t vec)\
{ return N ## _add(B, TN ## _vec_clone(B, vec)); }\
static inline int N ## _slice(NS ## builder_t *B, TN ## _vec_t vec, size_t index, size_t len)\
{ return N ## _add(B, TN ## _vec_slice(B, vec, index, len)); }\
__flatbuffers_build_vector_ops(NS, N, N, TN, T)

#define __flatbuffers_build_offset_vector_field(ID, NS, N, TN)\
static inline int N ## _add(NS ## builder_t *B, TN ## _vec_ref_t ref)\
{ TN ## _vec_ref_t *_p; return (ref && (_p = flatcc_builder_table_add_offset(B, ID))) ? ((*_p = ref), 0) : -1; }\
static inline int N ## _start(NS ## builder_t *B)\
{ return flatcc_builder_start_offset_vector(B); }\
static inline int N ## _end(NS ## builder_t *B)\
{ return N ## _add(B, flatcc_builder_end_offset_vector(B)); }\
static inline int N ## _create(NS ## builder_t *B, const TN ## _ref_t *data, size_t len)\
{ return N ## _add(B, flatcc_builder_create_offset_vector(B, data, len)); }\
__flatbuffers_build_offset_vector_ops(NS, N, N, TN)

#define __flatbuffers_build_string_field(ID, NS, N)\
static inline int N ## _add(NS ## builder_t *B, NS ## string_ref_t ref)\
{ NS ## string_ref_t *_p; return (ref && (_p = flatcc_builder_table_add_offset(B, ID))) ? ((*_p = ref), 0) : -1; }\
static inline int N ## _start(NS ## builder_t *B)\
{ return flatcc_builder_start_string(B); }\
static inline int N ## _end(NS ## builder_t *B)\
{ return N ## _add(B, flatcc_builder_end_string(B)); }\
static inline int N ## _create(NS ## builder_t *B, const char *s, size_t len)\
{ return N ## _add(B, flatcc_builder_create_string(B, s, len)); }\
static inline int N ## _create_str(NS ## builder_t *B, const char *s)\
{ return N ## _add(B, flatcc_builder_create_string_str(B, s)); }\
static inline int N ## _create_strn(NS ## builder_t *B, const char *s, size_t max_len)\
{ return N ## _add(B, flatcc_builder_create_string_strn(B, s, max_len)); }\
static inline int N ## _clone(NS ## builder_t *B, NS ## string_t string)\
{ return N ## _add(B, NS ## string_clone(B, string)); }\
static inline int N ## _slice(NS ## builder_t *B, NS ## string_t string, size_t index, size_t len)\
{ return N ## _add(B, NS ## string_slice(B, string, index, len)); }\
__flatbuffers_build_string_ops(NS, N)

#define __flatbuffers_build_table_vector_field(ID, NS, N, TN)\
__flatbuffers_build_offset_vector_field(ID, NS, N, TN)\
__flatbuffers_build_table_vector_ops(NS, N, TN)

#define __flatbuffers_build_string_vector_field(ID, NS, N)\
__flatbuffers_build_offset_vector_field(ID, NS, N, NS ## string)\
__flatbuffers_build_string_vector_ops(NS, N)

#define __flatbuffers_uint8_formal_args , uint8_t v0
#define __flatbuffers_uint8_call_args , v0
#define __flatbuffers_int8_formal_args , int8_t v0
#define __flatbuffers_int8_call_args , v0
#define __flatbuffers_bool_formal_args , flatbuffers_bool_t v0
#define __flatbuffers_bool_call_args , v0
#define __flatbuffers_uint16_formal_args , uint16_t v0
#define __flatbuffers_uint16_call_args , v0
#define __flatbuffers_uint32_formal_args , uint32_t v0
#define __flatbuffers_uint32_call_args , v0
#define __flatbuffers_uint64_formal_args , uint64_t v0
#define __flatbuffers_uint64_call_args , v0
#define __flatbuffers_int16_formal_args , int16_t v0
#define __flatbuffers_int16_call_args , v0
#define __flatbuffers_int32_formal_args , int32_t v0
#define __flatbuffers_int32_call_args , v0
#define __flatbuffers_int64_formal_args , int64_t v0
#define __flatbuffers_int64_call_args , v0
#define __flatbuffers_float_formal_args , float v0
#define __flatbuffers_float_call_args , v0
#define __flatbuffers_double_formal_args , double v0
#define __flatbuffers_double_call_args , v0

__flatbuffers_build_scalar(flatbuffers_, flatbuffers_uint8, uint8_t)
__flatbuffers_build_scalar(flatbuffers_, flatbuffers_int8, int8_t)
__flatbuffers_build_scalar(flatbuffers_, flatbuffers_bool, flatbuffers_bool_t)
__flatbuffers_build_scalar(flatbuffers_, flatbuffers_uint16, uint16_t)
__flatbuffers_build_scalar(flatbuffers_, flatbuffers_uint32, uint32_t)
__flatbuffers_build_scalar(flatbuffers_, flatbuffers_uint64, uint64_t)
__flatbuffers_build_scalar(flatbuffers_, flatbuffers_int16, int16_t)
__flatbuffers_build_scalar(flatbuffers_, flatbuffers_int32, int32_t)
__flatbuffers_build_scalar(flatbuffers_, flatbuffers_int64, int64_t)
__flatbuffers_build_scalar(flatbuffers_, flatbuffers_float, float)
__flatbuffers_build_scalar(flatbuffers_, flatbuffers_double, double)

__flatbuffers_build_string(flatbuffers_)

__flatbuffers_build_buffer(flatbuffers_)
#include "flatcc/portable/pdiagnostic_pop.h"
#endif /* FLATBUFFERS_COMMON_BUILDER_H */
