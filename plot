#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# plot  Copyright (C) 2011  Nils Dagsson Moskopp (erlehmann)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA 02110-1301, USA.

from sys import argv, stderr, stdout, exit

import math
from PIL import Image

try:
    filename = argv[1]
except IndexError:
    stderr.write('This program plots the function in the given file.\n\n')
    stderr.write('Usage: plot.py infile [width]\n\n\
If width is omitted, it is set to 1024.\n')
    exit()

try:
    with open(filename) as f:
        algorithm = f.read()
except IOError:
    stderr.write('File “%s” does not exist.\n' % filename)
    exit(1)

try:
    width = int(argv[2])
except IndexError:
    width = 1024
except ValueError:
    stderr.write('“%s” is not a width that can be interpreted in pixels.\n' % argv[2])
    exit(1)

height = 256
size = (width, height)
img = Image.new('1', size, 255)

for t in range(width):
    sample = int(
        eval(algorithm, {}, {
            'sin': math.sin,
            'cos': math.cos,
            'tan': math.tan,
            'M_PI': math.pi,
            't': t
        })
    )&0xff

    coord = (t, height - sample - 1)
    img.putpixel(coord, 0)

    t += 1

img.show()
