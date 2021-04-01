# vc4-gpiod

GPIO daemon for [Metal-Pi platform](https://github.com/mfp20/metalpi). It runs on VPU1 and uses both DMA and PWM to trigger actions on other peripherals with perfect timing and without any jitter introduced by other activities on VPU0 and ARM cores (ex: store incoming data).
It reads data from its memory space and produces single pulses (or pulse trains) accordingly.

## DMA Control Blocks

DMA controller provides a certain amount of independent channels; the only shared resource is the bus bandwidth, and the bandwidth can be assigned to channels depending on current DMA usage. Taken from [Broadcom's "BCM2835 ARM Peripherals" document](https://github.com/mfp20/vc4-lib/blob/main/docs/BCM2835-ARM-Peripherals.pdf):

`Each  DMA  channel  operates  by  loading  a Control  Block  (CB)  data  structure  from  memory into internal registers.  The Control Block defines the required DMA operation.  Each Control Block  can  point  to  a  further Control  Block  to  be  loaded  and  executed  once  the  operation described  in  the  current Control  Block  has  completed.    In  this  way  a  linked  list  of Control Blocks  can  be  constructed  in  order  to  execute  a  sequence  of  DMA operations  without software intervention.`

![DMA control blocks chain](docs/dma_cb_chain.png)
*(picture from https://github.com/fandahao17/Raspberry-Pi-DMA-Tutorial)*

The control block is read from memory at the beginning of each DMA transfer, at the end of which the NEXT_CB block is loaded to walk all the linked list. The NEXT_CB register can be ovewritten at run time; this allows GPIOd to update the control blocks at any time.


## PWM pacing

The speed of DMA accesses depends on the content of the control block, but it is usually in the MHz speed. In order to achieve slower speeds and perfect timing the PWM peripheral is used, because it has the capability to precisely control timing. Data blocks are interleaved with delay blocks and each delay block invokes a DMA write to the PWM FIFO. Then the PWM controller reads from its FIFO and sends this data to its own output, during which the DMA keeps waiting until PWM sets the DREQ signal to high. Controlling the number of clock cycles the PWM controller spend sending the data, GPIOd can delay the DMA to produce accurate delays.

![PWM-paced DMA](docs/dma_pwm_paced.png)
*(picture from https://github.com/fandahao17/Raspberry-Pi-DMA-Tutorial)*

Moreover PWM can be set to idle waiting for data, and repeat the last received data. This can efficency of DMA bandwidth use.

## GPIOd management
 
VC4 have 2 MMUs to map memory and peripherial addresses across 3 busses: system, physical, virtual. Cores and peripheral need to use specific busses to access data.
 
![Broadcom's BCM2835 memory mapping](docs/broadcom_mmap.png)
*(picture from [Broadcom's "BCM2835 ARM Peripherals" document](https://github.com/mfp20/vc4-lib/blob/main/docs/BCM2835-ARM-Peripherals.pdf))*

At boot the firmware reserves chunks of RAM for LK, GPIOd and Linux. The factory firmware uses a mailbox mechanism for message passing; _(TO BE TESTED) the mailbox produces a 100us delay for each message_. _(TO BE TESTED) GPIOd bypasses this delay by allowing direct write access from LK and Linux to its memory space._ Libmetalpi includes all the methods to manage GPIOd.


## GPIOd features

- By having a dedicated core and an efficient message passing GPIOd can bitbang many protocols at high speed on any pin. 
- Past tests indicated a maximum raw toggling speed [exceeding 20-40MHz on RPi1-RPi2](https://codeandlife.com/2015/03/25/raspberry-pi-2-vs-1-gpio-benchmark/), and up to 100MHz+ on some RPi versions. GPIOd adds overhead so maximum speed is lower, but still high enough for plenty applications.
- In the case 8-9-16-18-24 pins need to be used in parallel, pins can be used in SMI or DPI mode to increase the efficency and parallelism.

_TODO_


### SMI mode (8-16 parallel pins)

Secondary Memory Interface (SMI) is a high-speed I/O parallel interface to drive external memory chips. But can be used as a very useful general purpose interface as well: it has been used to succesfully drive ADCs, DACs, and LEDs, thanks to its fully programmable (with 4 nanosecond resolution) setup, strobe & hold times.

![Secondary Memory Interface](docs/smi.png)
*(picture from [Jeremy P Bentham](https://iosoft.blog/2020/07/16/raspberry-pi-smi/))*

The SMI interface has up to 18 bits of data, 6 address lines, read & write select lines.

![SMI pins](docs/smi_pins.png)
*(picture from [Jeremy P Bentham](https://iosoft.blog/2020/07/16/raspberry-pi-smi/))*

All its pins are exported on the RPi's 40-pin GPIO connector.

![SMI pins on RPi header](docs/smi_pins2.png)
*(picture from [Jeremy P Bentham](https://iosoft.blog/2020/07/16/raspberry-pi-smi/))*

Individual pins can be set to ALT1 mode in order to make them part of the SMI peripheral, leaving the rest to other functions (SPI, I2C, and so on). Since a single 32-bit DMA transfer can automatically be converted into four 8-bit accesses, or two 16-bit accesses, it is particularly comfortable to be used with 8 or 16 parallel pins. It can be used in 9 or 18 pins configuration too, using 0-padding in DMA data transfers.


### DPI mode (24 parallel pins)

Display Parallel Interface (DPI) is an high-speed output parallel interface generally used to drive displays. Basically, the DPI peripheral takes a list of 24bit samples (normally as pixel colors) and shove them out 24 gpio pins, at a defined pixel clock. The only problem is that the hsynv and vsync will cause regular gaps in the output, making it unsuitable for some applications.


## Performance

_TODO_


## License and more info

For more info see [Metal-Pi's README](https://github.com/mfp20/metalpi/blob/main/README.md).
