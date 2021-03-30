NOTE: these tables are just a stub, they don't reflect the real status (yet! work in progress)

## Libvc4 hardware support

||RPi0|RPi1|RPi2|RPi3|RPi4|notes|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|VPU(1)|no|no|no|no|no||
|V3D(2)|no|no|no|no|no||
|HVS(3)|no|no|no|no|no||
|PV(4)|no|no|no|no|no||
|VEC(5)|yes|yes|yes|yes|yes|[tested NTSC video only](https://www.youtube.com/watch?v=XBdUVmar7Rg)|
|DPI(6)|no|no|no|no|no||
|DSI(7)|no|no|no|no|no||
|HDMI(8)|no|no|no|no|no||
|UNICAM(9)|no|no|no|no|no||
|SMI(10)|no|no|no|no|no||
|ISP(11)|no|no|no|no|no||
|VCE(12)|no|no|no|no|no||
|POWER(13)|no|no|no|no|no||
|CLOCK(14)|no|no|no|no|no||
|ARM(15)|yes|yes|yes|yes|yes|see details below|

(1) 2 general purpose cores (VPU0, VPU1) on the VideoCore IV.

(2) 12 QPUs on the VideoCore IV.

(3) Hardware Video Scaler: takes a list of numeric pixels in the framebuffer, convert it into scanlines and feed them into a FIFO.

(4) Pixel Valve: takes video timing information, generates the hsync/vsync pulses, and throttles the flow of pixels from the HVS FIFO to the output PHY.

(5) Video Encoder Composite: video PHY, it generates NTSC/PAL signals.

(6) Display Parallel Interface: video PHY.

(7) Display Serial Interface: video PHY.

(8) HDMI: video PHY.

(9) UNICAM: camera interfaces, SoC have 2 of those. Normal RPis only expose the second instance, with 2 data lanes (1 Gbit/s each).

(10) Secondary Memory Interface: fast parallel interface to external memory chips; can be used as video PHY.

(11) Image Sensor Pipeline: hw logic to convert bayer images to RGB or YUV, and correcting lens artifacts.

(12) Video Codec Engine: hw logic to assist the video codec hardware blocks.

(13) Power management and frequency scaling.

(14) Timing generation in the SoC. [Details and a nice diagram](https://elinux.org/The_Undocumented_Pi#Clocks).

(15) ARM core and ARM-accessible peripherals


## Basic tasks

||RPi0|RPi1|RPi2|RPi3|RPi4|notes|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|JTAG|no|no|no|no|no||
|SD boot|no|no|yes|yes|no||
|USB-dev boot|no|no|NA|no|NA||
|USB-host (net/storage) boot|NA|NA|no|no|no||
|Start LK on VPU0|no|no|no|no|no||
|Start GPIOd on VPU1|no|no|no|no|no||
|Start Linux on ARM|no|no|no|no|no||
|LK console output|no|no|no|no|no||
|Linux console output|no|no|no|no|no||
|LK video output|no|no|no|no|no||
|Linux video output|no|no|no|no|no||


## Libmetalpi features

||RPi0|RPi1|RPi2|RPi3|RPi4|notes|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|Timers|no|no|no|no|no||
|Interrupt controller|no|no|no|no|no||
|DMA controller|no|no|no|no|no||
|GPIO|no|no|no|no|no||
|PWM|no|no|no|no|no||
|PCM / I2S|no|no|no|no|no||
|UART0, UART1|no|no|no|no|no||
|I2C master|no|no|no|no|no||
|I2C / SPI slave|no|no|no|no|no||
|SPI0, SPI1, SPI2|no|no|no|no|no||
|USB|yes|yes|yes|yes|yes|USB-host works, USB-device might work|
|Ethernet|NA|yes|yes|yes|yes|built-in usb ethernet|
|WiFi|no|no|no|no|no||
|Create/start/stop/destroy LK threads on VPU0|no|no|no|no|no||
|Manage gpiod|no|no|no|no|no||
|GPGPU on QPUs|no|no|no|no|no||
