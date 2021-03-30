
CFLAGS_BASE=-Iinclude -Wall -g -ffunction-sections
CFLAGS_ARM=-mtune=arm1176jzf-s -march=armv6zk
CFLAGS_ARM_FPU=-mfpu=vfp -mfloat-abi=softfp
OBJS=hexdump.o crc32.o pl011.o

ifeq (${BAREMETAL},1)
  OBJS+=hardware.o io.o xprintf.o puts.o putchar.o printf.o
  CFLAGS=${CFLAGS_BASE} -nostdlib -nostartfiles -ffreestanding -DBAREMETAL
  CFLAGS_ARM+=-mthumb ${CFLAGS_ARM_FPU}
else
  CFLAGS=${CFLAGS_BASE}
endif

ifeq (${CC},vc4-elf-gcc)
  CFLAGS=${CFLAGS_BASE}
else
  ifeq (${CC},aarch64-unknown-linux-gnu-gcc)
    CFLAGS=${CFLAGS_BASE}
  else
    CFLAGS=${CFLAGS_BASE} ${CFLAGS_ARM}
  endif
endif

CXXFLAGS=${CFLAGS} -fno-exceptions -fno-rtti
LDFLAGS=-nostdlib -nostartfiles -ffreestanding

all: libvc4.a

libvc4.a: ${OBJS}
	${AR} rcs $@ $^

install: libvc4.a
	mkdir -pv ${OUTDIR}/lib
	cp -r include ${OUTDIR}/
	cp libvc4.a ${OUTDIR}/lib/

