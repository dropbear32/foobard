// Copyright (c) 2023 Ally Sommers
// This code is licensed under the BSD 3-Clause License. A copy of this license
// is included in the repository.

#include "ubjson/ubjson.h"

#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <systemd/sd-bus.h>
#include <unistd.h>

#define ok(cond, str, ...)                                     \
    do                                                         \
    {                                                          \
        if (!(cond))                                           \
            printf(("L# %d > " str), __LINE__, ##__VA_ARGS__); \
    } while (0)

#define SEND_SIMPLE_PACKET(command)                              \
    do                                                           \
    {                                                            \
        struct ubjson_ctx ctx;                                   \
        ubjson_ctx_init(&ctx, NULL, 0);                          \
        ubjson_ctx_create_object(&ctx);                          \
        ubjson_ctx_add_kv_pair_string(&ctx, "command", command); \
        ubjson_ctx_render_creation(&ctx);                        \
        send(peer, ctx.render_buf, ctx.render_index, 0);         \
        ubjson_ctx_free(&ctx);                                   \
    } while (0)

int peer;

int foobar2000_PROP_FALSE(sd_bus *bus,
                          const char *path,
                          const char *interface,
                          const char *property,
                          sd_bus_message *reply,
                          void *userdata,
                          sd_bus_error *ret_error)
{
    return sd_bus_message_append_basic(reply, 'b', &(bool) { false });
}

int foobar2000_PROP_TRUE(sd_bus *bus,
                         const char *path,
                         const char *interface,
                         const char *property,
                         sd_bus_message *reply,
                         void *userdata,
                         sd_bus_error *ret_error)
{
    return sd_bus_message_append_basic(reply, 'b', &(bool) { true });
}

int foobar2000_Raise(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    return sd_bus_reply_method_return(m, "");
}

int foobar2000_Quit(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    return sd_bus_reply_method_return(m, "");
}

int foobar2000_Identity(sd_bus *bus,
                        const char *path,
                        const char *interface,
                        const char *property,
                        sd_bus_message *reply,
                        void *userdata,
                        sd_bus_error *ret_error)
{
    return sd_bus_message_append_basic(reply, 's', "foobar2000");
}

int foobar2000_SupportedUriSchemes(sd_bus *bus,
                                   const char *path,
                                   const char *interface,
                                   const char *property,
                                   sd_bus_message *reply,
                                   void *userdata,
                                   sd_bus_error *ret_error)
{
    char *schemes[] = { "file", NULL };
    return sd_bus_message_append_strv(reply, schemes);
}

int foobar2000_SupportedMimeTypes(sd_bus *bus,
                                  const char *path,
                                  const char *interface,
                                  const char *property,
                                  sd_bus_message *reply,
                                  void *userdata,
                                  sd_bus_error *ret_error)
{
    char *mimeTypes[] = { NULL };
    return sd_bus_message_append_strv(reply, mimeTypes);
}

// clang-format off
static const sd_bus_vtable foobar2000_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("Raise",  "", "", foobar2000_Raise,   SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("Quit",   "", "", foobar2000_Quit,    SD_BUS_VTABLE_UNPRIVILEGED),

    SD_BUS_PROPERTY("CanQuit",              "b",    foobar2000_PROP_FALSE,          0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("CanRaise",             "b",    foobar2000_PROP_FALSE,          0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("HasTrackList",         "b",    foobar2000_PROP_FALSE,          0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("Identity",             "s",    foobar2000_Identity,            0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("SupportedUriSchemes",  "as",   foobar2000_SupportedUriSchemes, 0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("SupportedMimeTypes",   "as",   foobar2000_SupportedMimeTypes,  0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_VTABLE_END
};
// clang-format on

int foobar2000_player_Next(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    SEND_SIMPLE_PACKET("next");
    return sd_bus_reply_method_return(m, "");
}

int foobar2000_player_Previous(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    SEND_SIMPLE_PACKET("previous");
    return sd_bus_reply_method_return(m, "");
}

int foobar2000_player_Pause(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    SEND_SIMPLE_PACKET("pause");
    return sd_bus_reply_method_return(m, "");
}

int foobar2000_player_PlayPause(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    SEND_SIMPLE_PACKET("playpause");
    return sd_bus_reply_method_return(m, "");
}

int foobar2000_player_Stop(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    SEND_SIMPLE_PACKET("stop");
    return sd_bus_reply_method_return(m, "");
}

int foobar2000_player_Play(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    SEND_SIMPLE_PACKET("play");
    return sd_bus_reply_method_return(m, "");
}

int foobar2000_player_Seek(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    int64_t offset;
    sd_bus_message_read_basic(m, 'x', &offset);

    struct ubjson_ctx ctx;
    ubjson_ctx_init(&ctx, NULL, 0);
    ubjson_ctx_create_object(&ctx);
    ubjson_ctx_add_kv_pair_string(&ctx, "command", "seek");
    ubjson_ctx_add_kv_pair_int64(&ctx, "offset", offset);
    ubjson_ctx_render_creation(&ctx);
    send(peer, ctx.render_buf, ctx.render_index, 0);
    ubjson_ctx_free(&ctx);

    return sd_bus_reply_method_return(m, "");
}

int foobar2000_player_SetPosition(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    char *track_id;
    int64_t offset;
    sd_bus_message_read_basic(m, 'o', &track_id);
    sd_bus_message_read_basic(m, 'x', &offset);

    struct ubjson_ctx ctx;
    ubjson_ctx_init(&ctx, NULL, 0);
    ubjson_ctx_create_object(&ctx);
    ubjson_ctx_add_kv_pair_string(&ctx, "command", "setposition");
    ubjson_ctx_add_kv_pair_string(&ctx, "track_id", track_id);
    ubjson_ctx_add_kv_pair_int64(&ctx, "offset", offset);
    ubjson_ctx_render_creation(&ctx);
    send(peer, ctx.render_buf, ctx.render_index, 0);
    ubjson_ctx_free(&ctx);

    return sd_bus_reply_method_return(m, "");
}

int foobar2000_player_OpenUri(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    char *uri;
    sd_bus_message_read_basic(m, 's', &uri);
    printf("uri: %s\n", uri);
    return sd_bus_reply_method_return(m, "");
}

int foobar2000_PlaybackStatus(sd_bus *bus,
                              const char *path,
                              const char *interface,
                              const char *property,
                              sd_bus_message *reply,
                              void *userdata,
                              sd_bus_error *ret_error)
{
    SEND_SIMPLE_PACKET("playbackstatus");

    char buf[256] = { 0 };
    size_t recv_size = recv(peer, buf, sizeof(buf), 0);

    char *message = "Stopped";
    char *key = NULL;
    char *status = NULL;

    struct ubjson_ctx ctx;
    ubjson_ctx_init(&ctx, buf, recv_size);
    if (!ubjson_ctx_parse(&ctx))
        goto cleanup;

    ubjson_ctx_read_kv_pair(&ctx, &key, &status, UBJSON_TYPE_STRING);

    if (strcmp(key, "status"))
        goto cleanup;

    if (!strcmp(status, "Paused"))
        message = "Paused";
    if (!strcmp(status, "Playing"))
        message = "Playing";

cleanup:
    free(key);
    free(status);
    ubjson_ctx_free(&ctx);
    return sd_bus_message_append_basic(reply, 's', message);
}

int foobar2000_Rate(sd_bus *bus,
                    const char *path,
                    const char *interface,
                    const char *property,
                    sd_bus_message *reply,
                    void *userdata,
                    sd_bus_error *ret_error)
{
    return sd_bus_message_append_basic(reply, 'd', &(double) { 1.0 });
}

int foobar2000_Metadata(sd_bus *bus,
                        const char *path,
                        const char *interface,
                        const char *property,
                        sd_bus_message *reply,
                        void *userdata,
                        sd_bus_error *ret_error)
{
    SEND_SIMPLE_PACKET("metadata");

    char buf[4096] = { 0 };
    size_t recv_size = recv(peer, buf, sizeof(buf), 0);

    char *key = NULL;

    char *id = NULL;
    int64_t length;
    char *art_url = NULL;

    char *album = NULL;
    char **artist = NULL;
    size_t artist_count = 0;

    char *date = NULL;
    char *title = NULL;
    int16_t track_number;

    struct ubjson_ctx ctx;
    ubjson_ctx_init(&ctx, buf, recv_size);
    if (!ubjson_ctx_parse(&ctx))
        goto cleanup;

#define KEY_CHECK(str)                                                       \
    do                                                                       \
    {                                                                        \
        if (strcmp(key, str))                                                \
        {                                                                    \
            printf("%s != %s\n", key, str);                                  \
            free(key);                                                       \
            sd_bus_message_open_container(reply, 'a', "{sv}");               \
            sd_bus_message_append(reply, "{sv}", "mpris:trackid", "o", "/"); \
            sd_bus_message_close_container(reply);                           \
            goto cleanup;                                                    \
        }                                                                    \
        free(key);                                                           \
    } while (0);

    ubjson_ctx_read_kv_pair(&ctx, &key, &id, UBJSON_TYPE_STRING);
    KEY_CHECK("id");
    ubjson_ctx_next_value(&ctx);

    ubjson_ctx_read_kv_pair(&ctx, &key, &length, UBJSON_TYPE_INT64);
    KEY_CHECK("length");
    ubjson_ctx_next_value(&ctx);

    ubjson_ctx_read_kv_pair(&ctx, &key, &art_url, UBJSON_TYPE_STRING);
    KEY_CHECK("artUrl");
    ubjson_ctx_next_value(&ctx);

    ubjson_ctx_read_kv_pair(&ctx, &key, &album, UBJSON_TYPE_STRING);
    KEY_CHECK("album");
    ubjson_ctx_next_value(&ctx);

    ubjson_ctx_read_kv_pair(&ctx, &key, &artist_count, UBJSON_TYPE_ARRAY);
    KEY_CHECK("artist");
    artist = malloc(sizeof(char *) * artist_count);
    ubjson_ctx_enter_collection(&ctx);
    for (size_t i = 0; i < artist_count; i++)
    {
        ubjson_ctx_read(&ctx, &artist[i], UBJSON_TYPE_STRING);
        ubjson_ctx_next_value(&ctx);
    };
    ubjson_ctx_exit_collection(&ctx);
    ubjson_ctx_next_value(&ctx);

    ubjson_ctx_read_kv_pair(&ctx, &key, &date, UBJSON_TYPE_STRING);
    KEY_CHECK("date");
    ubjson_ctx_next_value(&ctx);

    ubjson_ctx_read_kv_pair(&ctx, &key, &title, UBJSON_TYPE_STRING);
    KEY_CHECK("title");
    ubjson_ctx_next_value(&ctx);

    ubjson_ctx_read_kv_pair(&ctx, &key, &track_number, UBJSON_TYPE_INT16);
    KEY_CHECK("track_number");
#undef KEY_CHECK

    sd_bus_message_open_container(reply, 'a', "{sv}");

    sd_bus_message_append(reply, "{sv}", "mpris:trackid", "o", id);
    sd_bus_message_append(reply, "{sv}", "mpris:length", "x", &length);
    sd_bus_message_append(reply, "{sv}", "mpris:artUrl", "s", art_url);
    sd_bus_message_append(reply, "{sv}", "mpris:album", "s", album);

    sd_bus_message_open_container(reply, 'e', "sv");
    sd_bus_message_append_basic(reply, 's', "xesam:artist");
    sd_bus_message_open_container(reply, 'v', "as");
    sd_bus_message_open_container(reply, 'a', "s");
    for (size_t i = 0; i < artist_count; i++)
        sd_bus_message_append_basic(reply, 's', artist[i]);
    sd_bus_message_close_container(reply);
    sd_bus_message_close_container(reply);
    sd_bus_message_close_container(reply);

    sd_bus_message_append(reply, "{sv}", "xesam:date", "s", date);
    sd_bus_message_append(reply, "{sv}", "xesam:title", "s", title);
    sd_bus_message_append(reply, "{sv}", "xesam:trackNumber", "n", &track_number);

    sd_bus_message_close_container(reply);

cleanup:
    free(id);
    free(art_url);
    free(album);
    for (size_t i = 0; i < artist_count; i++)
        free(artist[i]);
    free(artist);
    free(date);
    free(title);
    ubjson_ctx_free(&ctx);

    return 0;
}

int foobar2000_Volume(sd_bus *bus,
                      const char *path,
                      const char *interface,
                      const char *property,
                      sd_bus_message *reply,
                      void *userdata,
                      sd_bus_error *ret_error)
{
    return sd_bus_message_append_basic(reply, 'd', &(double) { 1.0 });
}

int foobar2000_Position(sd_bus *bus,
                        const char *path,
                        const char *interface,
                        const char *property,
                        sd_bus_message *reply,
                        void *userdata,
                        sd_bus_error *ret_error)
{
    SEND_SIMPLE_PACKET("position");

    char buf[256] = { 0 };
    size_t recv_size = recv(peer, buf, sizeof(buf), 0);

    int64_t position = 0;
    char *key = NULL;
    int64_t received_position = 0;

    struct ubjson_ctx ctx;
    ubjson_ctx_init(&ctx, buf, recv_size);
    if (!ubjson_ctx_parse(&ctx))
        goto cleanup;

    ubjson_ctx_read_kv_pair(&ctx, &key, &received_position, UBJSON_TYPE_INT64);

    if (strcmp(key, "position"))
        goto cleanup;

    position = received_position;

cleanup:
    free(key);
    ubjson_ctx_free(&ctx);
    return sd_bus_message_append_basic(reply, 'x', &position);
}

int foobar2000_MinimumRate(sd_bus *bus,
                           const char *path,
                           const char *interface,
                           const char *property,
                           sd_bus_message *reply,
                           void *userdata,
                           sd_bus_error *ret_error)
{
    return sd_bus_message_append_basic(reply, 'd', &(double) { 1.0 });
}

int foobar2000_MaximumRate(sd_bus *bus,
                           const char *path,
                           const char *interface,
                           const char *property,
                           sd_bus_message *reply,
                           void *userdata,
                           sd_bus_error *ret_error)
{
    return sd_bus_message_append_basic(reply, 'd', &(double) { 1.0 });
}

int foobar2000_LoopStatus(sd_bus *bus,
                          const char *path,
                          const char *interface,
                          const char *property,
                          sd_bus_message *reply,
                          void *userdata,
                          sd_bus_error *ret_error)
{
    return sd_bus_message_append_basic(reply, 's', &"none");
}

// clang-format off
static const sd_bus_vtable foobar2000_player_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("Next",           "",     "",     foobar2000_player_Next,         SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("Previous",       "",     "",     foobar2000_player_Previous,     SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("Pause",          "",     "",     foobar2000_player_Pause,        SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("PlayPause",      "",     "",     foobar2000_player_PlayPause,    SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("Stop",           "",     "",     foobar2000_player_Stop,         SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("Play",           "",     "",     foobar2000_player_Play,         SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("Seek",           "x",    "",     foobar2000_player_Seek,         SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("SetPosition",    "ox",   "",     foobar2000_player_SetPosition,  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("OpenUri",        "s",    "",     foobar2000_player_OpenUri,      SD_BUS_VTABLE_UNPRIVILEGED),

    SD_BUS_SIGNAL("Seeked", "x", 0),

    SD_BUS_PROPERTY("PlaybackStatus",   "s",        foobar2000_PlaybackStatus,  0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("Rate",             "d",        foobar2000_Rate,            0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("Metadata",         "a{sv}",    foobar2000_Metadata,        0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("Volume",           "d",        foobar2000_Volume,          0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("Position",         "x",        foobar2000_Position,        0, 0),
    SD_BUS_PROPERTY("MinimumRate",      "d",        foobar2000_MinimumRate,     0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("MaximumRate",      "d",        foobar2000_MaximumRate,     0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("CanGoNext",        "b",        foobar2000_PROP_TRUE,       0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("CanGoPrevious",    "b",        foobar2000_PROP_TRUE,       0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("CanPlay",          "b",        foobar2000_PROP_TRUE,       0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("CanPause",         "b",        foobar2000_PROP_TRUE,       0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("CanSeek",          "b",        foobar2000_PROP_TRUE,       0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("CanControl",       "b",        foobar2000_PROP_TRUE,       0, 0),
    SD_BUS_PROPERTY("LoopStatus",       "s",        foobar2000_LoopStatus,      0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("Shuffle",          "b",        foobar2000_PROP_TRUE,       0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_VTABLE_END
};
// clang-format on

void *sd_bus_loop(void *vp_bus)
{
    int ret;
    struct sd_bus *bus = vp_bus;
    while (true)
    {
        ret = sd_bus_process(bus, NULL);
        if (ret > 0)
            continue;

        ret = sd_bus_wait(bus, (uint64_t)-1);
    }
}

int main(void)
{
    int sock;
    struct sockaddr_un addr = { AF_UNIX, "/tmp/foo_mpris.sock" }, client;
    struct sd_bus *bus;
    int ret;

    sigaction(SIGPIPE, &(struct sigaction) { { SIG_IGN } }, NULL);

restart:
    bus = NULL;

    ret = sd_bus_open_user(&bus);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-ret));
        return 1;
    }

    ret = sd_bus_add_object_vtable(bus, NULL, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2", foobar2000_vtable, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to issue method call: %s\n", strerror(-ret));
        return 1;
    }

    ret = sd_bus_add_object_vtable(bus, NULL, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", foobar2000_player_vtable, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-ret));
        return 1;
    }

    ret = sd_bus_request_name(bus, "org.mpris.MediaPlayer2.foobar2000", 0);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to acquire service name: %s\n", strerror(-ret));
        return 1;
    }

    unlink(addr.sun_path);

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)))
    {
        fprintf(stderr, "Failed to bind to '%s'\n", addr.sun_path);
        return 1;
    }

    listen(sock, 1);
    peer = accept(sock, (struct sockaddr *)&client, &(socklen_t) { sizeof(client) });

    setsockopt(peer, SOL_SOCKET, SO_RCVTIMEO, &(struct timeval) { 1, 0 }, sizeof(struct timeval));

    char buf[19] = { 0 };
    ret = recv(peer, buf, sizeof(buf), 0);
    if (memcmp(buf, "{i\x07commandSi\x05hello}", ret))
    {
        printf("Received incorrect hello frame '%s', exiting...\n", buf);
        return 1;
    }

    while (true)
    {
        char ping[9];
        ret = send(peer, "{i\x04pingN}", 9, 0);
        if (ret <= 0)
        {
            sd_bus_flush_close_unref(bus);
            goto restart;
        }

        ret = recv(peer, ping, sizeof(ping), 0);
        if (ret <= 0 || memcmp(ping, "{i\x04pingN}", 9))
        {
            sd_bus_flush_close_unref(bus);
            goto restart;
        }

        ret = sd_bus_process(bus, NULL);
        while (ret > 0)
            ret = sd_bus_process(bus, NULL);

        ret = sd_bus_wait(bus, 1000000); // 1-second wait
    }

    sd_bus_flush_close_unref(bus);
}
