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
#include <malloc.h>
#else
#include <endian.h>
#include <stdlib.h>
#endif
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

bool ubjson_ctx_append_bytes_to_render(struct ubjson_ctx *ctx, char *str, size_t len)
{
    while (ctx->render_index + len >= ctx->render_capacity)
        ctx->render_buf = realloc(ctx->render_buf, ctx->render_capacity *= 2);

    memcpy(ctx->render_buf + ctx->render_index, str, len);
    ctx->render_index += len;
    return true;
}

bool ubjson_ctx_append_byte_to_render(struct ubjson_ctx *ctx, char b)
{
    if (ctx->render_index + 1 >= ctx->render_capacity)
        ctx->render_buf = realloc(ctx->render_buf, ctx->render_capacity += 256);

    ctx->render_buf[ctx->render_index++] = b;
    return true;
}

bool ubjson_ctx_render_string(struct ubjson_ctx *ctx, char *str, bool is_key)
{
    if (!is_key)
        ubjson_ctx_append_byte_to_render(ctx, 'S');

    size_t len = strlen(str);

    if (len <= INT8_MAX)
    {
        ubjson_ctx_append_byte_to_render(ctx, 'i');
        ubjson_ctx_append_byte_to_render(ctx, len);
        goto write_string;
    }
    if (len <= UINT8_MAX)
    {
        ubjson_ctx_append_byte_to_render(ctx, 'U');
        ubjson_ctx_append_byte_to_render(ctx, len);
        goto write_string;
    }
    if (len <= INT16_MAX)
    {
        ubjson_ctx_append_byte_to_render(ctx, 'I');
        ubjson_ctx_append_bytes_to_render(ctx, (char *)&(i16) { htobe16(len) }, 2);
        goto write_string;
    }
    if (len <= INT32_MAX)
    {
        ubjson_ctx_append_byte_to_render(ctx, 'l');
        ubjson_ctx_append_bytes_to_render(ctx, (char *)&(i32) { htobe32(len) }, 4);
        goto write_string;
    }
    if (len <= INT64_MAX)
    {
        ubjson_ctx_append_byte_to_render(ctx, 'L');
        ubjson_ctx_append_bytes_to_render(ctx, (char *)&(i64) { htobe64(len) }, 8);
        goto write_string;
    }

write_string:
    ubjson_ctx_append_bytes_to_render(ctx, str, len);
    return false;
}

bool ubjson_ctx_render_value(struct ubjson_ctx *ctx, struct ubjson_value *value)
{
    switch (value->type)
    {
    case UBJSON_TYPE_NULL:
        ubjson_ctx_append_byte_to_render(ctx, 'Z');
        return true;
    case UBJSON_TYPE_NOOP:
        ubjson_ctx_append_byte_to_render(ctx, 'N');
        return true;
    case UBJSON_TYPE_TRUE:
        ubjson_ctx_append_byte_to_render(ctx, 'T');
        return true;
    case UBJSON_TYPE_FALSE:
        ubjson_ctx_append_byte_to_render(ctx, 'F');
        return true;
    case UBJSON_TYPE_INT8:
        ubjson_ctx_append_byte_to_render(ctx, 'i');
        ubjson_ctx_append_byte_to_render(ctx, value->v.int8);
        return true;
    case UBJSON_TYPE_UINT8:
        ubjson_ctx_append_byte_to_render(ctx, 'U');
        ubjson_ctx_append_byte_to_render(ctx, value->v.uint8);
        return true;
    case UBJSON_TYPE_INT16:
        ubjson_ctx_append_byte_to_render(ctx, 'I');
        ubjson_ctx_append_bytes_to_render(ctx, (char *)&(i16) { htobe16(value->v.int16) }, 2);
        return true;
    case UBJSON_TYPE_INT32:
        ubjson_ctx_append_byte_to_render(ctx, 'l');
        ubjson_ctx_append_bytes_to_render(ctx, (char *)&(i32) { htobe32(value->v.int32) }, 4);
        return true;
    case UBJSON_TYPE_INT64:
        ubjson_ctx_append_byte_to_render(ctx, 'L');
        ubjson_ctx_append_bytes_to_render(ctx, (char *)&(i64) { htobe64(value->v.int64) }, 8);
        return true;
    case UBJSON_TYPE_FLOAT32:
        ubjson_ctx_append_byte_to_render(ctx, 'd');
        ubjson_ctx_append_bytes_to_render(ctx, (char *)&value->v.float32, 4);
        return true;
    case UBJSON_TYPE_FLOAT64:
        ubjson_ctx_append_byte_to_render(ctx, 'D');
        ubjson_ctx_append_bytes_to_render(ctx, (char *)&value->v.float64, 8);
        return true;
    case UBJSON_TYPE_CHAR:
        ubjson_ctx_append_byte_to_render(ctx, 'C');
        ubjson_ctx_append_byte_to_render(ctx, value->v.character);
        return true;
    case UBJSON_TYPE_STRING:
        return ubjson_ctx_render_string(ctx, value->v.string, false);
    case UBJSON_TYPE_ARRAY:
        return ubjson_ctx_render_array(ctx, value->v.array);
    case UBJSON_TYPE_OBJECT:
        return ubjson_ctx_render_object(ctx, value->v.object);
    case UBJSON_TYPE_HIGHPRECISION:
    default:
        return false;
    }
}

bool ubjson_ctx_render_array(struct ubjson_ctx *ctx, struct ubjson_array array)
{
    ubjson_ctx_append_byte_to_render(ctx, '[');

    for (size_t i = 0; i < array.count; i++)
    {
        ubjson_ctx_render_value(ctx, &array.values[i]);
    }

    ubjson_ctx_append_byte_to_render(ctx, ']');

    return true;
}

bool ubjson_ctx_render_object(struct ubjson_ctx *ctx, struct ubjson_object object)
{
    ubjson_ctx_append_byte_to_render(ctx, '{');

    for (size_t i = 0; i < object.count; i++)
    {
        ubjson_ctx_render_string(ctx, object.kv_pairs[i].key, true);
        ubjson_ctx_render_value(ctx, &object.kv_pairs[i].value);
    }

    ubjson_ctx_append_byte_to_render(ctx, '}');

    return true;
}

bool ubjson_ctx_render(struct ubjson_ctx *ctx)
{
    if (ctx->root.type == UBJSON_TYPE_OBJECT)
    {
        return ubjson_ctx_render_object(ctx, ctx->root.collection.object);
    }
    if (ctx->root.type == UBJSON_TYPE_ARRAY)
    {
        return ubjson_ctx_render_array(ctx, ctx->root.collection.array);
    }
    return false;
}

bool ubjson_ctx_render_creation(struct ubjson_ctx *ctx)
{
    if (!ctx->current)
        return false;

    struct ubjson_collection_list *root = ctx->current;
    while (root->parent)
        root = root->parent;

    if (root->type == UBJSON_TYPE_OBJECT)
    {
        return ubjson_ctx_render_object(ctx, root->collection.object);
    }
    if (root->type == UBJSON_TYPE_ARRAY)
    {
        return ubjson_ctx_render_array(ctx, root->collection.array);
    }
    return false;
}
