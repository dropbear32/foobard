# foobard

foobard is a program which, when combined with foo_mpris, adds
[MPRIS](https://specifications.freedesktop.org/mpris-spec/latest/) support to
[foobar2000](https://www.foobar2000.org/) running under
[Wine](https://www.winehq.org/).

foo_mpris is a foobar2000 component written in C++ which responds to commands
it receives with the current state of the player (including metadata,
playing/paused/stopped, etc.) or by controlling the player (play/pause, seek,
etc.); these are enabled via foobar2000's SDK.

foobard is a daemon written in C99 which runs on the Linux side and sits
between foo_mpris and D-Bus, translating the commands it receives over D-Bus
into commands for foo_mpris. foobard uses
[sd-bus](https://www.freedesktop.org/software/systemd/man/latest/sd-bus.html)
for the actual D-Bus interfacing.

Communication is via Unix socket, and packets are encoded in UBJSON.

*Note: Upstream Wine does not currently support Unix sockets. I have submitted
[a merge request](https://gitlab.winehq.org/wine/wine/-/merge_requests/2786)
implementing support for them.*

## Building
To build foobard, you must have a C compiler, GNU make, and libsystemd installed.
I'm pretty sure it's possible to have libsystemd on other init systems. Then run
`make`, which will give you a binary at `build/foobard`.

To build foo_mpris, you must be on Windows (probably; I haven't tried building
under Wine) and have the LLVM toolchain and MinGW installed, and LLVM must be
in your MinGW path. You must also have `make` installed. Change directories to
`foo_mpris/`, then run `make`, which will give you a DLL at
`./build/foo_mpris.dll`. Place this DLL at
`%APPDATA%\foobar2000-v2\user-components-x64\foo_mpris\`. Optionally modify
`foo_mpris/Makefile` and provide a path accurate to your system in the
`install` recipe.

## License
This code is licensed under the BSD 3-Clause License. A copy of this license is
included in the repository. Please note that only the code under the following
paths (`-maxdepth 1`) is under this license:

* `.`
* `foo_mpris/src/`
* `ubjson/`

The remaining code is the foobar2000 SDK and its dependencies.
