// Copyright (c) 2023 Ally Sommers
// This code is licensed under the BSD 3-Clause License. A copy of this license
// is included in the repository.

#include "ubjson.h"

#include <stdlib.h>
#include <string.h>

bool ubjson_ctx_enter_collection(struct ubjson_ctx *ctx)
{
    struct ubjson_value next;

    if (ctx->current->type == UBJSON_TYPE_OBJECT)
    {
        struct ubjson_object object = ctx->current->collection.object;
        if (object.kv_pairs == NULL)
            return false;

        if (ctx->current->is_from_parse)
            next = object.kv_pairs[ctx->current->index].value;
        else
            next = object.kv_pairs[ctx->current->collection.object.count - 1].value;
    }
    else
    {
        struct ubjson_array array = ctx->current->collection.array;
        if (array.values == NULL)
            return false;

        if (ctx->current->is_from_parse)
            next = array.values[ctx->current->index];
        else
            next = array.values[ctx->current->collection.array.count - 1];
    }

    if (next.type != UBJSON_TYPE_OBJECT && next.type != UBJSON_TYPE_ARRAY)
        return false;

    struct ubjson_collection_list *parent = ctx->current;
    ctx->current = calloc(sizeof(*ctx->current), 1);
    ctx->current->parent = parent;

    if (next.type == UBJSON_TYPE_OBJECT)
    {
        ctx->current->collection.object = next.v.object;
        ctx->current->type = UBJSON_TYPE_OBJECT;
    }
    else
    {
        ctx->current->collection.array = next.v.array;
        ctx->current->type = UBJSON_TYPE_ARRAY;
    }

    ctx->current->is_from_parse = parent->is_from_parse;

    return true;
}

bool ubjson_ctx_exit_collection(struct ubjson_ctx *ctx)
{
    struct ubjson_collection_list *parent = ctx->current->parent;
    if (parent == NULL)
        return false;

    if (!ctx->current->is_from_parse)
    {
        if (parent->type == UBJSON_TYPE_OBJECT)
        {
            if (ctx->current->type == UBJSON_TYPE_OBJECT)
                parent->collection.object.kv_pairs[parent->collection.object.count - 1].value.v.object = ctx->current->collection.object;
            else
                parent->collection.object.kv_pairs[parent->collection.object.count - 1].value.v.array = ctx->current->collection.array;
        }
        else
        {
            if (ctx->current->type == UBJSON_TYPE_OBJECT)
                parent->collection.array.values[parent->collection.array.count - 1].v.object = ctx->current->collection.object;
            else
                parent->collection.array.values[parent->collection.array.count - 1].v.array = ctx->current->collection.array;
        }
    }

    free(ctx->current);
    ctx->current = parent;

    return true;
}

bool ubjson_ctx_add_kv_pair(struct ubjson_ctx *ctx, char const *key, struct ubjson_value value)
{
    if (ctx->current->type != UBJSON_TYPE_OBJECT)
        return false;

    char *key_copy = malloc(strlen(key) + 1);
    strcpy(key_copy, key);
    struct ubjson_kv_pair kv = { key_copy, value };

    if (ctx->current->collection.object.count + 1 > ctx->current->collection.object.capacity)
    {
        if (ctx->current->collection.object.capacity)
            ctx->current->collection.object.capacity *= 2;
        else
            ctx->current->collection.object.capacity = 4;
        ctx->current->collection.object.kv_pairs =
            realloc(ctx->current->collection.object.kv_pairs,
                    sizeof(*ctx->current->collection.object.kv_pairs) * ctx->current->collection.object.capacity);
    }

    ctx->current->collection.object.kv_pairs[ctx->current->collection.object.count++] = kv;

    return true;
}

// clang-format off
bool ubjson_ctx_add_kv_pair_object(struct ubjson_ctx *ctx, char const *key)
{ struct ubjson_value param = { .v.object = { NULL, 0, 0 }, .type = UBJSON_TYPE_OBJECT }; return ubjson_ctx_add_kv_pair(ctx, key, param); }

bool ubjson_ctx_add_kv_pair_array(struct ubjson_ctx *ctx, char const *key)
{ struct ubjson_value param = { .v.array = { NULL, 0, 0 }, .type = UBJSON_TYPE_ARRAY }; return ubjson_ctx_add_kv_pair(ctx, key, param); }

bool ubjson_ctx_add_kv_pair_int8(struct ubjson_ctx *ctx, char const *key, i8 value)
{ struct ubjson_value param = { .v.int8 = value, .type = UBJSON_TYPE_INT8 }; return ubjson_ctx_add_kv_pair(ctx, key, param); }

bool ubjson_ctx_add_kv_pair_uint8(struct ubjson_ctx *ctx, char const *key, u8 value)
{ struct ubjson_value param = { .v.uint8 = value, .type = UBJSON_TYPE_UINT8 }; return ubjson_ctx_add_kv_pair(ctx, key, param); }

bool ubjson_ctx_add_kv_pair_int16(struct ubjson_ctx *ctx, char const *key, i16 value)
{ struct ubjson_value param = { .v.int16 = value, .type = UBJSON_TYPE_INT16 }; return ubjson_ctx_add_kv_pair(ctx, key, param); }

bool ubjson_ctx_add_kv_pair_int32(struct ubjson_ctx *ctx, char const *key, i32 value)
{ struct ubjson_value param = { .v.int32 = value, .type = UBJSON_TYPE_INT32 }; return ubjson_ctx_add_kv_pair(ctx, key, param); }

bool ubjson_ctx_add_kv_pair_int64(struct ubjson_ctx *ctx, char const *key, i64 value)
{ struct ubjson_value param = { .v.int64 = value, .type = UBJSON_TYPE_INT64 }; return ubjson_ctx_add_kv_pair(ctx, key, param); }

bool ubjson_ctx_add_kv_pair_float32(struct ubjson_ctx *ctx, char const *key, float value)
{ struct ubjson_value param = { .v.float32 = value, .type = UBJSON_TYPE_FLOAT32 }; return ubjson_ctx_add_kv_pair(ctx, key, param); }

bool ubjson_ctx_add_kv_pair_float64(struct ubjson_ctx *ctx, char const *key, double value)
{ struct ubjson_value param = { .v.float64 = value, .type = UBJSON_TYPE_FLOAT64 }; return ubjson_ctx_add_kv_pair(ctx, key, param); }

bool ubjson_ctx_add_kv_pair_character(struct ubjson_ctx *ctx, char const *key, u8 value)
{ struct ubjson_value param = { .v.character = value, .type = UBJSON_TYPE_CHAR }; return ubjson_ctx_add_kv_pair(ctx, key, param); }
// clang-format on

bool ubjson_ctx_add_kv_pair_string(struct ubjson_ctx *ctx, char const *key, char const *value)
{
    char *value_copy = malloc(strlen(value) + 1);
    strcpy(value_copy, value);
    struct ubjson_value param = { .v.string = value_copy, .type = UBJSON_TYPE_STRING };
    if (!ubjson_ctx_add_kv_pair(ctx, key, param))
    {
        free(value_copy);
        return false;
    }

    return true;
}

bool ubjson_ctx_add(struct ubjson_ctx *ctx, struct ubjson_value value)
{
    if (ctx->current->type != UBJSON_TYPE_ARRAY)
        return false;

    if (ctx->current->collection.array.count + 1 > ctx->current->collection.array.capacity)
    {
        if (ctx->current->collection.array.capacity)
            ctx->current->collection.array.capacity *= 2;
        else
            ctx->current->collection.array.capacity = 4;
        ctx->current->collection.array.values =
            realloc(ctx->current->collection.array.values,
                    sizeof(*ctx->current->collection.array.values) * ctx->current->collection.array.capacity);
    }

    ctx->current->collection.array.values[ctx->current->collection.array.count++] = value;

    return true;
}

// clang-format off
bool ubjson_ctx_add_object(struct ubjson_ctx *ctx)
{ struct ubjson_value param = { .v.object = { NULL, 0, 0 }, .type = UBJSON_TYPE_OBJECT }; return ubjson_ctx_add(ctx, param); }

bool ubjson_ctx_add_array(struct ubjson_ctx *ctx)
{ struct ubjson_value param = { .v.array = { NULL, 0, 0 }, .type = UBJSON_TYPE_ARRAY }; return ubjson_ctx_add(ctx, param); }

bool ubjson_ctx_add_int8(struct ubjson_ctx *ctx, i8 value)
{ struct ubjson_value param = { .v.int8 = value, .type = UBJSON_TYPE_INT8 }; return ubjson_ctx_add(ctx, param); }

bool ubjson_ctx_add_uint8(struct ubjson_ctx *ctx, u8 value)
{ struct ubjson_value param = { .v.uint8 = value, .type = UBJSON_TYPE_UINT8 }; return ubjson_ctx_add(ctx, param); }

bool ubjson_ctx_add_int16(struct ubjson_ctx *ctx, i16 value)
{ struct ubjson_value param = { .v.int16 = value, .type = UBJSON_TYPE_INT16 }; return ubjson_ctx_add(ctx, param); }

bool ubjson_ctx_add_int32(struct ubjson_ctx *ctx, i32 value)
{ struct ubjson_value param = { .v.int32 = value, .type = UBJSON_TYPE_INT32 }; return ubjson_ctx_add(ctx, param); }

bool ubjson_ctx_add_int64(struct ubjson_ctx *ctx, i64 value)
{ struct ubjson_value param = { .v.int64 = value, .type = UBJSON_TYPE_INT64 }; return ubjson_ctx_add(ctx, param); }

bool ubjson_ctx_add_float32(struct ubjson_ctx *ctx, float value)
{ struct ubjson_value param = { .v.float32 = value, .type = UBJSON_TYPE_FLOAT32 }; return ubjson_ctx_add(ctx, param); }

bool ubjson_ctx_add_float64(struct ubjson_ctx *ctx, double value)
{ struct ubjson_value param = { .v.float64 = value, .type = UBJSON_TYPE_FLOAT64 }; return ubjson_ctx_add(ctx, param); }

bool ubjson_ctx_add_character(struct ubjson_ctx *ctx, u8 value)
{ struct ubjson_value param = { .v.character = value, .type = UBJSON_TYPE_CHAR }; return ubjson_ctx_add(ctx, param); }
// clang-format on

bool ubjson_ctx_add_string(struct ubjson_ctx *ctx, char const *value)
{
    char *value_copy = malloc(strlen(value) + 1);
    strcpy(value_copy, value);
    struct ubjson_value param = { .v.string = value_copy, .type = UBJSON_TYPE_STRING };
    if (!ubjson_ctx_add(ctx, param))
    {
        free(value_copy);
        return false;
    }

    return true;
}

bool ubjson_ctx_create_object(struct ubjson_ctx *ctx)
{
    ctx->current = calloc(sizeof(*ctx->current), 1);
    ctx->current->type = UBJSON_TYPE_OBJECT;
    return true;
}

bool ubjson_ctx_create_array(struct ubjson_ctx *ctx)
{
    ctx->current = calloc(sizeof(*ctx->current), 1);
    ctx->current->type = UBJSON_TYPE_ARRAY;
    return true;
}
