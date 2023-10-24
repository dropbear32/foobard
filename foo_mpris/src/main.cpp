// Copyright (c) 2023 Ally Sommers
// This code is licensed under the BSD 3-Clause License. A copy of this license
// is included in the repository.

#include "socket.hpp"

#include <WinSock2.h>
#include <Windows.h>
#include <afunix.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <helpers/foobar2000+atl.h>
#include <string>

DECLARE_COMPONENT_VERSION("foo_mpris", "0.0.1", "");
VALIDATE_COMPONENT_FILENAME("foo_mpris.dll");

#define LOG(fmt, ...)                                       \
    do                                                      \
    {                                                       \
        console::printf("[foo_mpris] " fmt, ##__VA_ARGS__); \
    } while (0)
#define MPRIS_FLAG "/mpris" // will have a ':' between the flag and the subcommand

static FILE *outputFile;

class mpris_commandline_handler: public commandline_handler {
    result on_token(const char *token) override
    {
        return RESULT_NOT_OURS;
    }

    static_api_ptr_t<playback_control> playback;
    std::string currentTrackID = "None";
};

static commandline_handler_factory_t<mpris_commandline_handler> commandline_factory;

MPRIS *mpris;

class mpris_initquit: public initquit {
    virtual void FB2KAPI on_init()
    {
        // Required Winsock boilerplate, two lines because C++ can't be arsed to add compound literals
        WSADATA wsa_data;
        WSAStartup(MAKEWORD(2, 2), &wsa_data);

        mpris = new MPRIS {};
        MPRIS::initStatic();
        fb2k::inMainThread([] { play_callback_manager::get()->register_callback(mpris, MPRIS::flags(), true); });

        CreateThread(NULL, 0, MPRIS::connectToServer, NULL, 0, NULL);
    }

    virtual void FB2KAPI on_quit()
    {
        MPRIS::shouldExit = true;
        LOG("quitting...");
    }
};

static initquit_factory_t<mpris_initquit> initquit_factory;
