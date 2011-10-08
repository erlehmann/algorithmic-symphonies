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

# WAVHEADER created from the following python code:
#
# duration = 0
# channels = 1
# bps = 8
# sample = 8000
# ExtraParamSize = 0
# Subchunk1Size = 16+2+ExtraParamSize
# Subchunk2Size = duration*sample*channels*bps/8
# ChunkSize = 4 + (8 + Subchunk1Size) + (8 + Subchunk2Size)
#
# stdout.write("".join([
#     'RIFF',                                # ChunkID (magic)      # 0x00
#     pack('<I', ChunkSize),                 # ChunkSize            # 0x04
#     'WAVE',                                # Format               # 0x08
#     'fmt ',                                # Subchunk1ID          # 0x0c
#     pack('<I', Subchunk1Size),             # Subchunk1Size        # 0x10
#     pack('<H', 1),                         # AudioFormat (1=PCM)  # 0x14
#     pack('<H', channels),                  # NumChannels          # 0x16
#     pack('<I', sample),                    # SampleRate           # 0x18
#     pack('<I', bps/8 * channels * sample), # ByteRate             # 0x1c
#     pack('<H', bps/8 * channels),          # BlockAlign           # 0x20
#     pack('<H', bps),                       # BitsPerSample        # 0x22
#     pack('<H', ExtraParamSize),            # ExtraParamSize       # 0x22
#     'data',                                # Subchunk2ID          # 0x24
#     pack('<I', Subchunk2Size)              # Subchunk2Size        # 0x28
# ]))

WAVHEADER="\
0000000: 5249 4646 a638 0100 5741 5645 666d 7420 \n\
0000010: 1200 0000 0100 0100 401f 0000 401f 0000 \n\
0000020: 0100 0800 0000 6461 7461 0000 0000 "

C_HEADER="\
\#include <math.h>\n\
\#include <stdio.h>\n\
main() {long int t;for(t=0;;t++) {fputc((int)("

C_FOOTER="), stdout);}}"

play: ${TARGET}.a
	./$< | head -c $$((${DURATION}*8000)) | aplay -r 8000 -f U8
	rm $<

%.c:
	echo ${C_HEADER} > $@
	cat $(basename $@) >> $@
	echo ${C_FOOTER} >> $@

%.a: %.c
	${CC} -lm $< -o $@

%.wav: %.a
	echo ${WAVHEADER} | xxd -r - $@
	./$< | head -c $$((${DURATION}*8000)) >> $@
	printf "%08X\n" $$((${DURATION}*8000)) | sed 's/\(..\)\(..\)\(..\)\(..\)/000002A \4\3 \2\1/' | xxd -r - $@
