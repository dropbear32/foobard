// Copyright (c) 2023 Ally Sommers
// This code is licensed under the BSD 3-Clause License. A copy of this license
// is included in the repository.

#include <WinSock2.h>
#include <afunix.h>
#include <helpers/foobar2000+atl.h>

#define SOCKET_LOCK(a, b)                                                \
    do                                                                   \
    {                                                                    \
        while (!__atomic_test_and_set(&b->socketLock, __ATOMIC_SEQ_CST)) \
            Sleep(10);                                                   \
        a;                                                               \
        __atomic_clear(&b->socketLock, __ATOMIC_SEQ_CST);                \
    } while (0)

class MPRIS: public play_callback {
    public:
    static SOCKET sock;
    static sockaddr_un sockAddress;
    static bool shouldExit;

    MPRIS();
    ~MPRIS();

    constexpr static int flags()
    {
        return flag_on_playback_all;
    }

    void on_playback_new_track(metadb_handle_ptr p_track);
    void on_playback_starting(play_control::t_track_command p_command, bool p_paused);
    void on_playback_stop(play_control::t_stop_reason p_reason);
    void on_playback_seek(double p_time);
    void on_playback_pause(bool p_state);
    void on_playback_edited(metadb_handle_ptr p_track);
    void on_playback_dynamic_info(const file_info &p_info);
    void on_playback_dynamic_info_track(const file_info &p_info);
    void on_playback_time(double p_time);
    void on_volume_change(float p_new_val);

    static void initStatic();
    static DWORD __stdcall connectToServer(LPVOID ptr);
    static DWORD __stdcall watchSocket(LPVOID ptr);
};
