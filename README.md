**NOTE: this README is just a draft, the repo is under heavy rework.**

# Raspberry Pi bare metal development platform

Factory Raspberry Pi is defined as "low-cost, high performance, desktop computer" or "personal computer" or "single board computer". With a few exceptions (ie: Compute Module and Pico), it is made to hook a monitor, keyboard and mouse, and give a graphical interface to the users. In time it has been successfully used for many different electronic projects (motion control, radio, lightning) but its use in electronics was limited in software because of the different scope.

The Broadcom silicon is a [VideoCore4 GPU](https://en.wikipedia.org/wiki/VideoCore) made of VPUs (general purpose cores), QPUs (vector cores), and an ARM CPU. Factory firmware focuses on desktop use, giving to the users the chance to load Linux on the ARM cpu and keeping the GPU for maintenance and video output, using a closed source ThreadX operating system running in the background on the VPUs. This setup limits both the real time performance and the general purpose GPU computing (GPGPU). By replacing ThreadX with an open source alternative, users are able to fully exploit RPi hardware, turning any RPi in a "low-cost, high performance, electronics power house": Metal-Pi.

**Metal-Pi** is a bare metal development platform for all Raspberry Pi models. It is an extension of [Kristina Brooks's `rpi-open-firmware`](https://github.com/christinaa/rpi-open-firmware) and [Michael Bishop's `rpi-open-firmware`](https://github.com/librerpi/rpi-open-firmware). It is tailored by developers for developers: headless (but video output can be enabled), using the gpu (ie: 2 VPUs and 12 QPUs) for real time applications and general purpose computing (GPGPU).

This repository is the umbrella project packing all the different parts of the project.

## Quick-start

First you need to install Nix:

`curl -L https://nixos.org/nix/install | sh`

This will create the Nix's store at /nix and setup the command line utility in your path. Then you need to download a copy of this repo

`git clone --recurse-submodules https://github.com/mfp20/metalpi.git`

finally chdir to your local copy of this repo and

`nix-build -f . arm6.bootdir`

This will download and build the toolchain and the firmware. After completion copy `result` folder's contents on the SD card,
move the SD card to your RPi and boot!

For more info about installing and building Metal-Pi see the [install file](INSTALL.md).


## Technical Details

This repository includes [Nix](https://nixos.org/)-based [scripts](nix/) and all the submodules needed to setup the toolchain and build the default distribution.

The build process will download, build and pack a ready to use folder to be copied on your SD card. It will include:
- the [VC4 bare metal library](https://github.com/mfp20/vc4-lib) to access VC4 hardware,
- the [VC4 libre firmware](https://github.com/mfp20/vc4-firmware) to initialize the VC4 (VPUs, QPUs, ARM, peripherals) and boot the payloads,
- the [VC4 version](https://github.com/mfp20/vc4-lk) of [LittleKernel](https://github.com/littlekernel/lk) running on VPU0,
- the [VC4 GPIOd](https://github.com/mfp20/vc4-gpiod) running on VPU1,
- the [Metal-Pi library](https://github.com/mfp20/metalpi-lib) C++ helper library, built as a static library,
- the [Metal-Pi Linux tools](https://github.com/mfp20/metalpi-tools) linux command line tools,
- the vanilla RPi version of [TinyCore Linux](http://www.tinycorelinux.net/) running on the ARM core.

The result is intended to be a ready to use platform but, being 100% open source, any of these components can be replaced, and the firmware itself can be modified to fit your own needs.

See the [status file](STATUS.md) for more info about current features.


## Documentation

Each submodule includes a `docs` folder with its own documentation.


## Credits, license, contacts

Metal-Pi is the result of many people's work. For details see the [authors file](AUTHORS.md).

Our code is under [GPLv2+](LICENSE). Various external projects are mirrored under a mix of GPL-compatible licenses. Broadcom's code is licensed under 3-Clause BSD License.

**Please [contact us](CONTRIBUTING.md) before randomly hacking on the codebase.**
