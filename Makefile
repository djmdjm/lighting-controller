MCU=at90usb1286
CPUFREQ=2000000
LOADER=teensy

#LOADER=avrdude
#AVRDUDE_PORT=/dev/cuaU1
#AVRDUDE_PART=m328p # m324pa t85 t861 t2313
#AVRDUDE_HW=buspirate
#AVRDUDE_EXTRA=

OPT=-Os

WARNFLAGS=-Wall -Wextra 
WARNFLAGS+=-Werror -Wno-type-limits

CFLAGS=-mmcu=${MCU} -DF_CPU=${CPUFREQ}UL ${WARNFLAGS} ${OPT} -std=gnu99
CFLAGS+=-funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums

LIBAVR_OBJS=demux.o lcd.o rgbled.o num_format.o spi.o ad56x8.o encoder.o event.o

CC=avr-gcc
OBJCOPY=avr-objcopy

all: blinky.hex

blinky.elf: blinky.o ${LIBAVR_OBJS}
	${CC} ${CFLAGS} -o $@ blinky.o ${LIBAVR_OBJS}

blinky.hex: blinky.elf
	${OBJCOPY} -j .text -j .data -O ihex blinky.elf $@

load: ${LOADER}

teensy: blinky.hex
	${SUDO} teensy_loader_cli -w -mmcu=${MCU} blinky.hex

avrdude: blinky.hex
	avrdude -P ${AVRDUDE_PORT} -p ${AVRDUDE_PART} -c ${AVRDUDE_HW} \
	    ${AVRDUDE_EXTRA} -e -U flash:w:blinky.hex

clean:
	rm -f *.elf *.hex *.o *.core *.hex
