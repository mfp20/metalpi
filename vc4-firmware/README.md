# vc4-firmware

VC4-Firmware is a libre replacement for the stock `bootcode.bin` normally loaded from the SD card by Raspberry Pi. It is part of the [Metal-Pi platform](https://github.com/mfp20/metalpi) but can be used standalone for your own delight. It depends on [Libvc4](https://github.com/mfp20/vc4-lib).

The firmware is split into two parts:
 1. [vc4boot](src/vc4boot): initializes PLLC and moves VPU0 over to it, brings up UART, performs SDRAM initialization, mapping it to `0xC0000000` (uncached alias), and loads VPU0 payload, which in turn loads VPU1 payload. Then ARM is initialized, the embedded bootloader is mapped to ARM address `0x0`, and `armload` is executed.
 2. [armload](src/armload): the ARM second-stage chainloader initializes the eMMC controller and accesses the FAT boot partition. From here, it chainloads other payloads like the Linux kernel, Micropython or any other custom payload.


## License and more info

For more info see [Metal-Pi's README](https://github.com/mfp20/metalpi/blob/main/README.md).
