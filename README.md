Photographic Lighting Timer
===========================

This is a small lighting controller circuit based around an AVR
microcontroller. It supports two inputs and two outputs and
configurable timing delays. The outputs use MOSFETs that should
be good for up to 50VDC and 20A.

The `board/` directory contains the Eagle files for the board and
`firmware/` contains the source for the firmware. You'll need
avr-gcc et al. to build it. `make load` will attempt to program
the firmware using my setup (avrdude w/ buspirate on
`/dev/cuaU1`).

NB. the current version of the board has not been built and the
old version that I did build had some problems that required a
bit of rework. I beat the old version into working shape and that
was good enough for my immediate needs.

Not included in the schematics are the front-panel controls: a
power toggle, momentary switch, generic rotary encoder (w/ press
switch) and 4x20 HD44870-style LCD. The switches and encoder need
pull-ups and debouncing caps.

The firmware allows triggering from logical combinations of the
inputs (e.g. "input 1 high and input 2 low"). Upon triggering it
waits a configurable delay (from 1us to ~1ksec) and enables one
or both outputs for a configurable duration. It also supports a
strobe mode that goes from milli-hertz to close to 1MHz frequency
and has adjustable duration and duty cycle.

The outputs are not isolated from each other, but they are
isolated from the inputs and the main board. Likewise the inputs
are isolated from the main board and the output but not from each
other.

This was only my 2nd attempt at cutting a board, so it's probably
terrible. You have been warned :)
