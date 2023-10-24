// Copyright (c) 2023 Ally Sommers
// This code is licensed under the BSD 3-Clause License. A copy of this license
// is included in the repository.

#include "ubjson.h"

#ifdef _WIN32
#define htobe16(n) __builtin_bswap16(n)
#define htobe32(n) __builtin_bswap32(n)
#define htobe64(n) __builtin_bswap64(n)
#define be16toh(n) __builtin_bswap16(n)
#define be32toh(n) __builtin_bswap32(n)
#define be64toh(n) __builtin_bswap64(n)
#else
#include <endian.h>
#endif
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void ubjson_ctx_init(struct ubjson_ctx *ctx, char const *buf, size_t size)
{
    memset(ctx, 0, sizeof(*ctx));

    if (buf && size)
    {
        ctx->src_buf = malloc(size);
        memcpy(ctx->src_buf, buf, size);
        ctx->src_len = size;
    }
}

void ubjson_free_object(struct ubjson_object object);
void ubjson_free_array(struct ubjson_array array);

void ubjson_free_object(struct ubjson_object object)
{
    for (size_t i = 0; i < object.count; i++)
    {
        free(object.kv_pairs[i].key);
        switch (object.kv_pairs[i].value.type)
        {
        case UBJSON_TYPE_OBJECT:
            ubjson_free_object(object.kv_pairs[i].value.v.object);
            break;
        case UBJSON_TYPE_ARRAY:
            ubjson_free_array(object.kv_pairs[i].value.v.array);
            break;
        case UBJSON_TYPE_STRING:
            free(object.kv_pairs[i].value.v.string);
            break;
        default:
            break;
        }
    }

    free(object.kv_pairs);
}

void ubjson_free_array(struct ubjson_array array)
{
    for (size_t i = 0; i < array.count; i++)
    {
        switch (array.values[i].type)
        {
        case UBJSON_TYPE_OBJECT:
            ubjson_free_object(array.values[i].v.object);
            break;
        case UBJSON_TYPE_ARRAY:
            ubjson_free_array(array.values[i].v.array);
            break;
        case UBJSON_TYPE_STRING:
            free(array.values[i].v.string);
            break;
        default:
            break;
        }
    }

    free(array.values);
}

void ubjson_ctx_free_creation(struct ubjson_ctx *ctx)
{
    if (ctx->current)
    {
        while (ubjson_ctx_exit_collection(ctx))
            ;

        if (!ctx->current->is_from_parse)
        {
            if (ctx->current->type == UBJSON_TYPE_OBJECT)
            {
                ubjson_free_object(ctx->current->collection.object);
            }
            if (ctx->current->type == UBJSON_TYPE_ARRAY)
            {
                ubjson_free_array(ctx->current->collection.array);
            }
        }

        free(ctx->current);
    }
}

void ubjson_ctx_free(struct ubjson_ctx *ctx)
{
    if (ctx->root.type == UBJSON_TYPE_OBJECT)
    {
        ubjson_free_object(ctx->root.collection.object);
    }
    if (ctx->root.type == UBJSON_TYPE_ARRAY)
    {
        ubjson_free_array(ctx->root.collection.array);
    }

    free(ctx->src_buf);
    free(ctx->render_buf);

    ubjson_ctx_free_creation(ctx);
}

char const *ubjson_ctx_consume(struct ubjson_ctx *ctx, size_t count)
{
    if (ctx->src_index + count >= ctx->src_len)
    {
        ctx->src_index = ctx->src_len;
        return NULL;
    }
    ctx->src_index += count;
    return ctx->src_buf + ctx->src_index - count;
}

char ubjson_ctx_peek(struct ubjson_ctx *ctx)
{
    return ctx->src_buf[ctx->src_index];
}

bool ubjson_ctx_parse_string(struct ubjson_ctx *ctx, char **str)
{
    i64 length;

    switch (*ubjson_ctx_consume(ctx, 1))
    {
    case 'i': // i8
        length = (i8)*ubjson_ctx_consume(ctx, 1);
        break;
    case 'U': // u8
        length = (u8)*ubjson_ctx_consume(ctx, 1);
        break;
    case 'I': // i16
        length = (i16)be16toh(*(i16 *)ubjson_ctx_consume(ctx, 2));
        break;
    case 'l': // i32
        length = (i32)be32toh(*(i32 *)ubjson_ctx_consume(ctx, 4));
        break;
    case 'L': // i64
        length = (i64)be64toh(*(i64 *)ubjson_ctx_consume(ctx, 8));
        break;
    default:
        return false;
    }

    *str = malloc(length + 1);
    memcpy(*str, ubjson_ctx_consume(ctx, length), length);
    (*str)[length] = '\0';

    return true;
}

bool ubjson_ctx_parse_array(struct ubjson_ctx *ctx, struct ubjson_array *array);
bool ubjson_ctx_parse_object(struct ubjson_ctx *ctx, struct ubjson_object *object);

bool ubjson_ctx_parse_value(struct ubjson_ctx *ctx, struct ubjson_value *value)
{
    switch (*ubjson_ctx_consume(ctx, 1))
    {
    case 'Z': // null
        value->type = UBJSON_TYPE_NULL;
        return true;
    case 'N': // no-op
        value->type = UBJSON_TYPE_NOOP;
        return true;
    case 'T': // true
        value->type = UBJSON_TYPE_TRUE;
        return true;
    case 'F': // false
        value->type = UBJSON_TYPE_FALSE;
        return true;
    case 'i': // i8
        value->type = UBJSON_TYPE_INT8;
        value->v.int8 = (i8)*ubjson_ctx_consume(ctx, 1);
        return true;
    case 'U': // u8
        value->type = UBJSON_TYPE_UINT8;
        value->v.uint8 = (u8)*ubjson_ctx_consume(ctx, 1);
        return true;
    case 'I': // i16
        value->type = UBJSON_TYPE_INT16;
        value->v.int16 = (i16)be16toh(*(i16 *)ubjson_ctx_consume(ctx, 2));
        return true;
    case 'l': // i32
        value->type = UBJSON_TYPE_INT32;
        value->v.int32 = (i32)be32toh(*(i32 *)ubjson_ctx_consume(ctx, 4));
        return true;
    case 'L': // i64
        value->type = UBJSON_TYPE_INT64;
        value->v.int64 = (i64)be64toh(*(i64 *)ubjson_ctx_consume(ctx, 8));
        return true;
    case 'd': // float32
        value->type = UBJSON_TYPE_FLOAT32;
        value->v.float32 = *(float *)ubjson_ctx_consume(ctx, 4);
        return true;
    case 'D': // float64
        value->type = UBJSON_TYPE_FLOAT64;
        value->v.float64 = *(double *)ubjson_ctx_consume(ctx, 8);
        return true;
    case 'C': // char
        value->type = UBJSON_TYPE_CHAR;
        value->v.character = (u8)*ubjson_ctx_consume(ctx, 1);
        return true;
    case 'S': // string
        value->type = UBJSON_TYPE_STRING;
        return ubjson_ctx_parse_string(ctx, &value->v.string);
    case '[':
        value->type = UBJSON_TYPE_ARRAY;
        return ubjson_ctx_parse_array(ctx, &value->v.array);
    case '{':
        value->type = UBJSON_TYPE_OBJECT;
        return ubjson_ctx_parse_object(ctx, &value->v.object);
    default:
        return false;
    }
}

bool ubjson_ctx_parse_array(struct ubjson_ctx *ctx, struct ubjson_array *array)
{
    array->values = NULL;
    array->count = 0;
    array->capacity = 0;

    if (ubjson_ctx_peek(ctx) == ']')
    {
        ubjson_ctx_consume(ctx, 1);
        return true;
    }
    if (ubjson_ctx_peek(ctx) == '\0')
        return false;

    while (true)
    {
        if (ubjson_ctx_peek(ctx) == ']')
        {
            ubjson_ctx_consume(ctx, 1);
            return true;
        }
        if (ubjson_ctx_peek(ctx) == '\0')
            return false; // TODO: Free array contents

        struct ubjson_value value;
        if (!ubjson_ctx_parse_value(ctx, &value))
            return false; // TODO: Free array contents

        if (array->count + 1 > array->capacity)
        {
            if (array->capacity)
                array->capacity *= 2;
            else
                array->capacity = 4;
            array->values = realloc(array->values, sizeof(*array->values) * array->capacity);
        }

        array->values[array->count++] = value;
    }
}

bool ubjson_ctx_parse_object(struct ubjson_ctx *ctx, struct ubjson_object *object)
{
    object->kv_pairs = NULL;
    object->count = 0;
    object->capacity = 0;

    if (ubjson_ctx_peek(ctx) == '}')
    {
        ubjson_ctx_consume(ctx, 1);
        return true;
    }
    if (ubjson_ctx_peek(ctx) == '\0')
        return false;

    while (true)
    {
        if (ubjson_ctx_peek(ctx) == '}')
        {
            ubjson_ctx_consume(ctx, 1);
            return true;
        }
        if (ubjson_ctx_peek(ctx) == '\0')
            return false; // TODO: Free object contents

        struct ubjson_kv_pair kv_pair;
        if (!ubjson_ctx_parse_string(ctx, &kv_pair.key))
            return false; // TODO: Free object contents
        if (!ubjson_ctx_parse_value(ctx, &kv_pair.value))
            return false; // TODO: Free object contents

        if (object->count + 1 > object->capacity)
        {
            if (object->capacity)
                object->capacity *= 2;
            else
                object->capacity = 4;
            object->kv_pairs = realloc(object->kv_pairs, sizeof(*object->kv_pairs) * object->capacity);
        }

        object->kv_pairs[object->count++] = kv_pair;
    }
}

bool ubjson_ctx_parse(struct ubjson_ctx *ctx)
{
    ubjson_ctx_free_creation(ctx);

    char start = *ubjson_ctx_consume(ctx, 1);
    if (start == '{')
    {
        ctx->root.type = UBJSON_TYPE_OBJECT;
        ctx->root.collection.object.kv_pairs = NULL;
        ctx->root.collection.object.count = 0;
        ctx->root.collection.object.capacity = 0;
        bool result = ubjson_ctx_parse_object(ctx, &ctx->root.collection.object);
        if (result)
        {
            ctx->current = calloc(sizeof(*ctx->current), 1);
            ctx->current->collection = ctx->root.collection;
            ctx->current->type = UBJSON_TYPE_OBJECT;
            ctx->current->is_from_parse = true;
        }
        return result;
    }
    if (start == '[')
    {
        ctx->root.type = UBJSON_TYPE_ARRAY;
        ctx->root.collection.array.values = NULL;
        ctx->root.collection.array.count = 0;
        ctx->root.collection.array.capacity = 0;
        bool result = ubjson_ctx_parse_array(ctx, &ctx->root.collection.array);
        if (result)
        {
            ctx->current = calloc(sizeof(*ctx->current), 1);
            ctx->current->collection = ctx->root.collection;
            ctx->current->type = UBJSON_TYPE_ARRAY;
            ctx->current->is_from_parse = true;
        }
        return result;
    }
    return false;
}
