# Gallium Operating System (GaOS)

An operating system targeting (for the moment) x86_64 hardware.
See [roadmap here](docs/ROADMAP.MD).

## Font

Used font is a modified version of [Hack](https://github.com/source-foundry/Hack), licensed under the MIT license.

## Running as guest on a x86_64 Debian-based host

First, install all the dependencies using `sudo make deps-debian`. Make sure your package list cache is up-to-date. <br>
Then, build the system toolchains using `make toolchain`. This step might take a while depending on your computer. <br>
And, in the end, launch QEMU using `make run`. 