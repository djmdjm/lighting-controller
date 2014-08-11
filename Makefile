MCU=at90usb1286
#MCU=atmega32u4
#CPUFREQ=8000000
CPUFREQ=16000000
LOADER=teensy

#LOADER=avrdude
#AVRDUDE_PORT=/dev/cuaU1
#AVRDUDE_PART=m328p # m324pa t85 t861 t2313
#AVRDUDE_HW=buspirate
#AVRDUDE_EXTRA=

OPT=-Os

WARNFLAGS=-Wall -Wextra 
WARNFLAGS+=-Werror -Wno-type-limits -Wno-unused

CFLAGS=-mmcu=${MCU} -DF_CPU=${CPUFREQ}UL ${WARNFLAGS} ${OPT} -std=gnu99
CFLAGS+=-funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums

LIBAVR_OBJS=demux.o rgbled.o num_format.o spi.o ad56x8.o encoder.o event.o
LIBAVR_OBJS+=mcp23s1x.o midi.o lcd.o

CC=avr-gcc
OBJCOPY=avr-objcopy

all: sequencer.hex

sequencer.elf: main.o ${LIBAVR_OBJS}
	${CC} ${CFLAGS} -o $@ main.o ${LIBAVR_OBJS}

sequencer.hex: sequencer.elf
	${OBJCOPY} -j .text -j .data -O ihex sequencer.elf $@

load: ${LOADER}

teensy: sequencer.hex
	${SUDO} teensy_loader_cli -v -w -mmcu=${MCU} sequencer.hex

avrdude: sequencer.hex
	avrdude -P ${AVRDUDE_PORT} -p ${AVRDUDE_PART} -c ${AVRDUDE_HW} \
	    ${AVRDUDE_EXTRA} -e -U flash:w:sequencer.hex

clean:
	rm -f *.elf *.hex *.o *.core *.hex
