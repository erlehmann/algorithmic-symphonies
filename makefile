# program by Johannes Schauer, 2011
#
# This program is free software. It comes without any warranty, to
# the extent permitted by applicable law. You can redistribute it
# and/or modify it under the terms of the Do What The Fuck You Want
# To Public License, Version 2, as published by Sam Hocevar. See
# http://sam.zoy.org/wtfpl/COPYING for more details.
#
# usage:
#
#   play music:
#     make DURATION=20 TARGET=4-512-s-11-63 play
#   create wav:
#     make DURATION=60 4-512-s-11-63.wav
#

DURATION=30
CC=gcc
TARGET=4-512-s-11-63

# convenience macros to create the RIFF header

define pack_int
	printf "%08X\n" $1 | sed 's/\([0-9A-F]\{2\}\)\([0-9A-F]\{2\}\)\([0-9A-F]\{2\}\)\([0-9A-F]\{2\}\)/\\\\\\x\4\\\\\\x\3\\\\\\x\2\\\\\\x\1/I' | xargs printf >> $2
endef

define pack_short
	printf "%04X\n" $1 | sed 's/\([0-9A-F]\{2\}\)\([0-9A-F]\{2\}\)/\\\\\\x\2\\\\\\x\1/I' | xargs printf >> $2
endef

define calculate
	$(shell echo $$(($1)))
endef

define riffhead
	echo -n RIFF > $1
	$(call pack_int, $(ChunkSize), $1)
	echo -n "WAVEfmt " >> $1
	$(call pack_int, $(Subchunk1Size), $1)
	$(call pack_short, 1, $1)
	$(call pack_short, $(channels), $1)
	$(call pack_int, $(sample), $1)
	$(call pack_int, $(call calculate, $(bps)/8 * $(channels) * $(sample)), $1)
	$(call pack_short, $(call calculate, $(bps)/8 * $(channels)), $1)
	$(call pack_short, $(bps), $1)
	$(call pack_short, 0, $1)
	echo -n data >> $1
	$(call pack_int, $(Subchunk2Size), $1)
endef

# variables defining the RIFF header information

channels=1
bps=8
sample=8000
Subchunk1Size=18
Subchunk2Size=$(call calculate, $(DURATION)*$(sample)*$(channels)*$(bps)/8)
ChunkSize=$(call calculate, $((20 + $(Subchunk1Size) + $(Subchunk2Size))))

# C header and footer snippets

C_HEADER="\
\#include <math.h>\n\
\#include <stdio.h>\n\
main() {long int t;for(t=0;;t++) {fputc((int)("

C_FOOTER="), stdout);}}"

play: ${TARGET}.a
	./$< | head -c $(Subchunk2Size) | aplay -r 8000 -f U8
	rm $<

%.c:
	echo ${C_HEADER} > $@
	cat $(basename $@) >> $@
	echo ${C_FOOTER} >> $@

%.a: %.c
	${CC} -lm $< -o $@

%.wav: %.a
	$(call riffhead, $@)
	./$< | head -c $(Subchunk2Size) >> $@
