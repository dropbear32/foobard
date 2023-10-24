// Copyright (c) 2023 Ally Sommers
// This code is licensed under the BSD 3-Clause License. A copy of this license
// is included in the repository.

#include "ubjson.h"

#ifdef _WIN32
#include <malloc.h>
#else
#include <stdlib.h>
#endif
#include <string.h>

bool ubjson_read_value(void *out, struct ubjson_value value, enum ubjson_type expected)
{
    if (!((value.type == UBJSON_TYPE_FALSE || value.type == UBJSON_TYPE_TRUE) &&
          (expected == UBJSON_TYPE_FALSE || expected == UBJSON_TYPE_TRUE)))
        if (value.type != expected)
            return false;

    if (!out)
        return true;

    switch (value.type)
    {
    case UBJSON_TYPE_NULL:
    case UBJSON_TYPE_NOOP:
        break;
    case UBJSON_TYPE_TRUE:
        *(bool *)out = true;
        break;
    case UBJSON_TYPE_FALSE:
        *(bool *)out = false;
        break;
    case UBJSON_TYPE_INT8:
        *(i8 *)out = value.v.int8;
        break;
    case UBJSON_TYPE_UINT8:
        *(u8 *)out = value.v.uint8;
        break;
    case UBJSON_TYPE_INT16:
        *(i16 *)out = value.v.int16;
        break;
    case UBJSON_TYPE_INT32:
        *(i32 *)out = value.v.int32;
        break;
    case UBJSON_TYPE_INT64:
        *(i64 *)out = value.v.int64;
        break;
    case UBJSON_TYPE_FLOAT32:
        *(float *)out = value.v.float32;
        break;
    case UBJSON_TYPE_FLOAT64:
        *(double *)out = value.v.float64;
        break;
    case UBJSON_TYPE_HIGHPRECISION:
        break;
    case UBJSON_TYPE_CHAR:
        *(char *)out = value.v.character;
        break;
    case UBJSON_TYPE_STRING:
        *(char **)out = malloc(strlen(value.v.string) + 1);
        strcpy(*(char **)out, value.v.string);
        break;
    case UBJSON_TYPE_ARRAY:
        *(size_t *)out = value.v.array.count;
        break;
    case UBJSON_TYPE_OBJECT:
        *(size_t *)out = value.v.object.count;
        break;
    }

    return true;
}

bool ubjson_ctx_read_kv_pair(struct ubjson_ctx *ctx, char **key, void *out, enum ubjson_type expected)
{
    if (ctx->current->type != UBJSON_TYPE_OBJECT)
        return false;

    char *index_key = ctx->current->collection.object.kv_pairs[ctx->current->index].key;
    struct ubjson_value value = ctx->current->collection.object.kv_pairs[ctx->current->index].value;

    if (key)
    {
        *key = malloc(strlen(index_key) + 1);
        strcpy(*key, index_key);
    }

    if (!ubjson_read_value(out, value, expected))
        return false;

    return true;
}

bool ubjson_ctx_next_value(struct ubjson_ctx *ctx)
{
    if (ctx->current->type == UBJSON_TYPE_OBJECT)
    {
        if (ctx->current->index + 1 >= ctx->current->collection.object.count)
            return false;
    }
    else
    {
        if (ctx->current->index + 1 >= ctx->current->collection.array.count)
            return false;
    }

    ctx->current->index++;
    return true;
}

bool ubjson_ctx_read(struct ubjson_ctx *ctx, void *out, enum ubjson_type expected)
{
    if (ctx->current->type != UBJSON_TYPE_ARRAY)
        return false;

    struct ubjson_value value = ctx->current->collection.array.values[ctx->current->index];

    return ubjson_read_value(out, value, expected);
}
