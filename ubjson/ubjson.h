// Copyright (c) 2023 Ally Sommers
// This code is licensed under the BSD 3-Clause License. A copy of this license
// is included in the repository.

#ifndef UBJSON_H
#define UBJSON_H

#define _POSIX_C_SOURCE 199309L
#define _DEFAULT_SOURCE

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;

enum ubjson_type
{
    UBJSON_TYPE_NULL,
    UBJSON_TYPE_NOOP,
    UBJSON_TYPE_TRUE,
    UBJSON_TYPE_FALSE,
    UBJSON_TYPE_INT8,
    UBJSON_TYPE_UINT8,
    UBJSON_TYPE_INT16,
    UBJSON_TYPE_INT32,
    UBJSON_TYPE_INT64,
    UBJSON_TYPE_FLOAT32,
    UBJSON_TYPE_FLOAT64,
    UBJSON_TYPE_HIGHPRECISION,
    UBJSON_TYPE_CHAR,
    UBJSON_TYPE_STRING,
    UBJSON_TYPE_ARRAY,
    UBJSON_TYPE_OBJECT
};

enum ubjson_ctx_state
{
    UBJSON_CTX_STATE_KEY_NEXT,
    UBJSON_CTX_STATE_OBJECT_NEXT,
};

struct _linked_list
{
    struct _linked_list *next;
    struct _linked_list *prev;
};

struct ubjson_object
{
    struct ubjson_kv_pair *kv_pairs;
    size_t count;
    size_t capacity;
};

struct ubjson_array
{
    struct ubjson_value *values;
    size_t count;
    size_t capacity;
};

struct ubjson_value
{
    union
    {
        i8 int8;
        u8 uint8;
        i16 int16;
        i32 int32;
        i64 int64;

        float float32;
        double float64;

        u8 character;
        char *string;

        struct ubjson_array array;
        struct ubjson_object object;
    } v;
    enum ubjson_type type;
};

struct ubjson_kv_pair
{
    char *key;
    struct ubjson_value value;
};

union ubjson_object_or_array
{
    struct ubjson_object object;
    struct ubjson_array array;
};

struct ubjson_ctx
{
    char *src_buf;
    size_t src_len;
    size_t src_index;

    struct ubjson_collection
    {
        union ubjson_object_or_array collection;
        enum ubjson_type type;
    } root;

    struct ubjson_collection_list
    {
        struct ubjson_collection_list *parent;

        union ubjson_object_or_array collection;
        enum ubjson_type type;
        size_t index;

        bool is_from_parse;
    } * current;

    char *render_buf;
    size_t render_index;
    size_t render_capacity;
};

void ubjson_ctx_init(struct ubjson_ctx *ctx, char const *buf, size_t size);
void ubjson_ctx_free(struct ubjson_ctx *ctx);

// PARSE //
bool ubjson_ctx_parse_string(struct ubjson_ctx *ctx, char **str);
bool ubjson_ctx_parse_value(struct ubjson_ctx *ctx, struct ubjson_value *value);
bool ubjson_ctx_parse_array(struct ubjson_ctx *ctx, struct ubjson_array *array);
bool ubjson_ctx_parse_object(struct ubjson_ctx *ctx, struct ubjson_object *object);
bool ubjson_ctx_parse(struct ubjson_ctx *ctx);
//

// RENDER //
bool ubjson_ctx_render_string(struct ubjson_ctx *ctx, char *str, bool is_key);
bool ubjson_ctx_render_value(struct ubjson_ctx *ctx, struct ubjson_value *value);
bool ubjson_ctx_render_array(struct ubjson_ctx *ctx, struct ubjson_array array);
bool ubjson_ctx_render_object(struct ubjson_ctx *ctx, struct ubjson_object object);
bool ubjson_ctx_render(struct ubjson_ctx *ctx);
bool ubjson_ctx_render_creation(struct ubjson_ctx *ctx);
//

// CREATE //
bool ubjson_ctx_create_object(struct ubjson_ctx *ctx);
bool ubjson_ctx_create_array(struct ubjson_ctx *ctx);

bool ubjson_ctx_enter_collection(struct ubjson_ctx *ctx);
bool ubjson_ctx_exit_collection(struct ubjson_ctx *ctx);

bool ubjson_ctx_add_kv_pair_object(struct ubjson_ctx *ctx, char const *key);
bool ubjson_ctx_add_kv_pair_array(struct ubjson_ctx *ctx, char const *key);
bool ubjson_ctx_add_kv_pair_int8(struct ubjson_ctx *ctx, char const *key, i8 value);
bool ubjson_ctx_add_kv_pair_uint8(struct ubjson_ctx *ctx, char const *key, u8 value);
bool ubjson_ctx_add_kv_pair_int16(struct ubjson_ctx *ctx, char const *key, i16 value);
bool ubjson_ctx_add_kv_pair_int32(struct ubjson_ctx *ctx, char const *key, i32 value);
bool ubjson_ctx_add_kv_pair_int64(struct ubjson_ctx *ctx, char const *key, i64 value);
bool ubjson_ctx_add_kv_pair_float32(struct ubjson_ctx *ctx, char const *key, float value);
bool ubjson_ctx_add_kv_pair_float64(struct ubjson_ctx *ctx, char const *key, double value);
bool ubjson_ctx_add_kv_pair_character(struct ubjson_ctx *ctx, char const *key, u8 value);
bool ubjson_ctx_add_kv_pair_string(struct ubjson_ctx *ctx, char const *key, char const *value);

bool ubjson_ctx_add_array(struct ubjson_ctx *ctx);
bool ubjson_ctx_add_object(struct ubjson_ctx *ctx);
bool ubjson_ctx_add_int8(struct ubjson_ctx *ctx, i8 value);
bool ubjson_ctx_add_uint8(struct ubjson_ctx *ctx, u8 value);
bool ubjson_ctx_add_int16(struct ubjson_ctx *ctx, i16 value);
bool ubjson_ctx_add_int32(struct ubjson_ctx *ctx, i32 value);
bool ubjson_ctx_add_int64(struct ubjson_ctx *ctx, i64 value);
bool ubjson_ctx_add_float32(struct ubjson_ctx *ctx, float value);
bool ubjson_ctx_add_float64(struct ubjson_ctx *ctx, double value);
bool ubjson_ctx_add_character(struct ubjson_ctx *ctx, u8 value);
bool ubjson_ctx_add_string(struct ubjson_ctx *ctx, char const *value);
//

// READ //
bool ubjson_ctx_read_kv_pair(struct ubjson_ctx *ctx, char **key, void *out, enum ubjson_type expected);
bool ubjson_ctx_read(struct ubjson_ctx *ctx, void *out, enum ubjson_type expected);
bool ubjson_ctx_next_value(struct ubjson_ctx *ctx);
//

#ifdef __cplusplus
}
#endif

#endif
