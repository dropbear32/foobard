// Copyright (c) 2023 Ally Sommers
// This code is licensed under the BSD 3-Clause License. A copy of this license
// is included in the repository.

#include "socket.hpp"

#include "defines.hpp"
#include "ubjson/ubjson.h"

#include <WinSock2.h>
#include <afunix.h>
#include <cstring>
#include <helpers/foobar2000+atl.h>
#include <inttypes.h>
#include <stdint.h>

extern bool IsWine;

SOCKET MPRIS::sock = 0;
sockaddr_un MPRIS::sockAddress = { AF_UNIX };
bool MPRIS::shouldExit = false;

void MPRIS::initStatic()
{
    if (!sock)
    {
        sock = socket(AF_UNIX, SOCK_STREAM, 0);

        // Determine if we're running under Wine
        static const char *(CDECL * pwine_get_version)(void) = NULL;
        HMODULE hntdll = GetModuleHandleA("ntdll.dll");
        if (hntdll)
            pwine_get_version = (const char *(*)())GetProcAddress(hntdll, "wine_get_version");

        if (pwine_get_version != NULL)
            strcpy(sockAddress.sun_path, "Z:\\tmp\\foo_mpris.sock");
        else
        {
            if (strlen(getenv("TEMP")) > sizeof(sockAddress.sun_path) - strlen("\\foo_mpris.sock"))
            {
                LOG("TEMP directory too long, defaulting socket path to \"C:\\tmp\\foo_mpris.sock\"");
                strcpy(sockAddress.sun_path, "C:\\tmp\\foo_mpris.sock");
            }
            else
            {
                strcpy(sockAddress.sun_path, getenv("TEMP"));
                strcat(sockAddress.sun_path, "\\foo_mpris.sock");
            }
        }
    }
}

MPRIS::MPRIS() {}

void MPRIS::on_playback_new_track(metadb_handle_ptr p_track) {}
void MPRIS::on_playback_starting(play_control::t_track_command p_command, bool p_paused) {}
void MPRIS::on_playback_stop(play_control::t_stop_reason p_reason) {}
void MPRIS::on_playback_seek(double p_time) {}
void MPRIS::on_playback_pause(bool p_state) {}
void MPRIS::on_playback_edited(metadb_handle_ptr p_track) {}
void MPRIS::on_playback_dynamic_info(const file_info &p_info) {}
void MPRIS::on_playback_dynamic_info_track(const file_info &p_info) {}
void MPRIS::on_playback_time(double p_time) {}
void MPRIS::on_volume_change(float p_new_val) {}

DWORD __stdcall MPRIS::connectToServer(LPVOID ptr)
{
    int err;
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    while (!shouldExit)
    {
        if ((err = connect(sock, (sockaddr *)&sockAddress, sizeof(sockAddress))))
        {
            Sleep(1000);
            continue;
        };

        char buf[] = "{i\x07commandSi\x05hello}";
        send(sock, buf, sizeof(buf) - 1, 0);
        CreateThread(NULL, 0, watchSocket, NULL, 0, NULL);

        LOG("Connected to socket");
        break;
    }

    return 0;
}

DWORD __stdcall MPRIS::watchSocket(LPVOID ptr)
{
    titleformat_object::ptr md5Format;
    titleformat_compiler::get()->compile_safe(md5Format, "/$info(md5)");

    titleformat_object::ptr albumFormat;
    titleformat_compiler::get()->compile_safe(albumFormat, "%album%");

    titleformat_object::ptr artistCountFormat;
    titleformat_compiler::get()->compile_safe(artistCountFormat, "$meta_num(artist)");

    titleformat_object::ptr dateFormat;
    titleformat_compiler::get()->compile_safe(dateFormat, "$meta(date, 0)");

    titleformat_object::ptr titleFormat;
    titleformat_compiler::get()->compile_safe(titleFormat, "%title%");

    titleformat_object::ptr trackNumberFormat;
    titleformat_compiler::get()->compile_safe(trackNumberFormat, "%track number%");

    abort_callback_impl abortCallback {};

    {
        int const timeout = 1000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char const *)&timeout, sizeof(timeout));
    }

    struct pollfd fds = { sock, POLLRDNORM };
    int constexpr CHUNK_SIZE = 256;

    while (!shouldExit)
    {
        if (WSAPoll(&fds, 1, 50))
        {
            size_t input_len = CHUNK_SIZE;
            char *input = (char *)malloc(CHUNK_SIZE);
            size_t received_length = recv(sock, input, CHUNK_SIZE, 0);
            if (received_length == SOCKET_ERROR || received_length == 0)
            {
                free(input);
                CreateThread(NULL, 0, connectToServer, NULL, 0, NULL);
                break;
            }

            while (input[input_len] == '!')
            {
                char *tmp = (char *)realloc(input, input_len += CHUNK_SIZE);
                if (!tmp)
                {
                    LOG("Failed to allocate enough memory to hold the received packet!");
                    free(input);
                    continue;
                }

                input = tmp;
                received_length += recv(sock, input + (input_len - CHUNK_SIZE), CHUNK_SIZE, 0);
            }

            if (!received_length)
            {
                free(input);
                continue;
            }

            if (!memcmp(input, "{i\x04pingN}", received_length))
            {
                send(sock, "{i\x04pingN}", 9, 0);
                free(input);
                continue;
            }

            LOG("Parsing %d bytes...", (int)received_length);
            ubjson_ctx ctx;
            ubjson_ctx_init(&ctx, input, received_length);
            free(input);

            if (!ubjson_ctx_parse(&ctx))
            {
                LOG("Failed to parse received packet!");
                ubjson_ctx_free(&ctx);
                continue;
            }

            char *command_buf;
            char command[32];
            ubjson_ctx_read_kv_pair(&ctx, NULL, &command_buf, UBJSON_TYPE_STRING);
            if (strlen(command_buf) + 1 > sizeof(command))
            {
                ubjson_ctx_free(&ctx);
                free(command_buf);
                continue;
            }
            strcpy(command, command_buf);
            free(command_buf);

#define MPRIS_COMMAND(cmd, str, call)      \
    if (!strcmp(cmd, str))                 \
    {                                      \
        fb2k::inMainThread([&] { call; }); \
        ubjson_ctx_free(&ctx);             \
        continue;                          \
    }
            MPRIS_COMMAND(command, "pause", playback_control::get()->pause(true));
            MPRIS_COMMAND(command, "play", playback_control::get()->play_or_unpause());
            MPRIS_COMMAND(command, "playpause", playback_control::get()->play_or_pause());
            MPRIS_COMMAND(command, "next", playback_control::get()->next());
            MPRIS_COMMAND(command, "previous", playback_control::get()->previous());
            MPRIS_COMMAND(command, "stop", playback_control::get()->stop());
#undef MPRIS_COMMAND

            if (!strcmp(command, "playbackstatus"))
            {
                ubjson_ctx send_ctx;
                ubjson_ctx_init(&send_ctx, NULL, 0);
                ubjson_ctx_create_object(&send_ctx);

                fb2k::inMainThreadSynchronous(
                    [&] {
                        if (playback_control::get()->is_playing())
                        {
                            if (playback_control::get()->is_paused())
                            {
                                ubjson_ctx_add_kv_pair_string(&send_ctx, "status", "Paused");
                            }
                            else
                            {
                                ubjson_ctx_add_kv_pair_string(&send_ctx, "status", "Playing");
                            }
                        }
                        else
                            ubjson_ctx_add_kv_pair_string(&send_ctx, "status", "Stopped");
                    },
                    abortCallback);

                ubjson_ctx_render_creation(&send_ctx);
                send(sock, send_ctx.render_buf, send_ctx.render_index, 0);
                ubjson_ctx_free(&ctx);
                ubjson_ctx_free(&send_ctx);
                continue;
            }

            if (!strcmp(command, "metadata"))
            {
                ubjson_ctx send_ctx;
                ubjson_ctx_init(&send_ctx, NULL, 0);
                ubjson_ctx_create_object(&send_ctx);

                metadb_handle_ptr p_track;

                bool is_playback;
                fb2k::inMainThreadSynchronous([&] { is_playback = playback_control::get()->get_now_playing(p_track); }, abortCallback);
                if (!is_playback)
                {
                    ubjson_ctx_add_kv_pair_string(&send_ctx, "id", "/");
                    ubjson_ctx_render_creation(&send_ctx);
                    send(sock, send_ctx.render_buf, send_ctx.render_index, 0);
                    ubjson_ctx_free(&ctx);
                    ubjson_ctx_free(&send_ctx);
                    continue;
                }

                fb2k::inMainThreadSynchronous(
                    [&] {
                        pfc::string p_out {};

                        p_track->format_title(NULL, p_out, md5Format, NULL);
                        ubjson_ctx_add_kv_pair_string(&send_ctx, "id", p_out.c_str());
                        ubjson_ctx_add_kv_pair_int64(&send_ctx, "length", (int64_t)(p_track->get_length() * USEC_PER_SEC));
                        // now_playing_album_art_notify_manager_v2::get()->current_v2().paths->get_path(0)
                        ubjson_ctx_add_kv_pair_string(&send_ctx, "artUrl", "");
                        p_track->format_title(NULL, p_out, albumFormat, NULL);
                        ubjson_ctx_add_kv_pair_string(&send_ctx, "album", p_out.c_str());

                        ubjson_ctx_add_kv_pair_array(&send_ctx, "artist");
                        ubjson_ctx_enter_collection(&send_ctx);
                        p_track->format_title(NULL, p_out, artistCountFormat, NULL);
                        int artist_count = atoi(p_out.c_str());
                        for (int i = 0; i < artist_count; i++)
                        {
                            char *formatString = (char *)malloc(snprintf(NULL, 0, "$meta(artist,%d)", i) + 1);
                            sprintf(formatString, "$meta(artist,%d)", i);
                            titleformat_object::ptr artistFormat;
                            titleformat_compiler::get()->compile_safe(artistFormat, formatString);
                            p_track->format_title(NULL, p_out, artistFormat, NULL);
                            ubjson_ctx_add_string(&send_ctx, p_out.c_str());
                            free(formatString);
                        }
                        ubjson_ctx_exit_collection(&send_ctx);

                        p_track->format_title(NULL, p_out, dateFormat, NULL);
                        ubjson_ctx_add_kv_pair_string(&send_ctx, "date", p_out.c_str());
                        p_track->format_title(NULL, p_out, titleFormat, NULL);
                        ubjson_ctx_add_kv_pair_string(&send_ctx, "title", p_out.c_str());
                        p_track->format_title(NULL, p_out, trackNumberFormat, NULL);
                        ubjson_ctx_add_kv_pair_int16(&send_ctx, "track_number", atoi(p_out.c_str()));
                    },
                    abortCallback);
                ubjson_ctx_render_creation(&send_ctx);
                send(sock, send_ctx.render_buf, send_ctx.render_index, 0);
                ubjson_ctx_free(&ctx);
                ubjson_ctx_free(&send_ctx);
                continue;
            }

            if (!strcmp(command, "position"))
            {
                ubjson_ctx send_ctx;
                ubjson_ctx_init(&send_ctx, NULL, 0);
                ubjson_ctx_create_object(&send_ctx);

                fb2k::inMainThreadSynchronous(
                    [&] {
                        ubjson_ctx_add_kv_pair_int64(&send_ctx,
                                                     "position",
                                                     (int64_t)(playback_control::get()->playback_get_position() * USEC_PER_SEC));
                    },
                    abortCallback);

                ubjson_ctx_render_creation(&send_ctx);
                send(sock, send_ctx.render_buf, send_ctx.render_index, 0);
                ubjson_ctx_free(&ctx);
                ubjson_ctx_free(&send_ctx);
                continue;
            }

            if (!ubjson_ctx_next_value(&ctx))
            {
                LOG("Invalid parameter count for command '%s'!", command);
                ubjson_ctx_free(&ctx);
                continue;
            }

            if (!strcmp(command, "seek"))
            {
                char *key;
                int64_t offset;
                ubjson_ctx_read_kv_pair(&ctx, &key, &offset, UBJSON_TYPE_INT64);
                if (strcmp(key, std::string { "offset" }.c_str()))
                {
                    LOG("Invalid parameter '%s' for command '%s'!", key, command);
                    free(key);
                    ubjson_ctx_free(&ctx);
                    continue;
                }

                LOG("Seeking by %" PRId64, offset);
                fb2k::inMainThread([&] { playback_control::get()->playback_seek_delta((double)offset / USEC_PER_SEC); });
                ubjson_ctx_free(&ctx);
                free(key);
            }

            if (!strcmp(command, "setposition"))
            {
                char *key;
                char *track_id;
                int64_t offset;
                ubjson_ctx_read_kv_pair(&ctx, &key, &track_id, UBJSON_TYPE_STRING);
                if (strcmp(key, "track_id"))
                {
                    LOG("Invalid parameter '%s' for command '%s'!", key, command);
                    ubjson_ctx_free(&ctx);
                    free(key);
                    continue;
                }
                free(key);
                ubjson_ctx_next_value(&ctx);
                ubjson_ctx_read_kv_pair(&ctx, &key, &offset, UBJSON_TYPE_INT64);
                if (strcmp(key, "offset"))
                {
                    LOG("Invalid parameter '%s' for command '%s'!", key, command);
                    ubjson_ctx_free(&ctx);
                    free(key);
                    continue;
                }
                free(key);
                LOG("seeking to %d", offset);
                fb2k::inMainThreadSynchronous(
                    [&] {
                        metadb_handle_ptr p_track;
                        pfc::string p_out {};

                        playback_control::get()->get_now_playing(p_track);
                        p_track->format_title(NULL, p_out, md5Format, NULL);
                        char current_id[34];
                        strcpy(current_id, p_out.c_str());
                        if (strcmp(track_id, current_id))
                        {
                            LOG("Tried to seek in non-current track ('%s' != '%s')", track_id, current_id);
                            return;
                        }

                        playback_control::get()->playback_seek((double)offset / USEC_PER_SEC);
                    },
                    abortCallback);

                free(track_id);
                ubjson_ctx_free(&ctx);
            }
        }
    }
    return 0;
}
