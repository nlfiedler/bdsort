#
# Copyright 2011 Nathan Fiedler. All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.
#

CFLAGS=-std=c99

objects = bds.o bfile.o bms.o hsort.o main.o say.o

all: bdsort

$(objects): %.o: %.c bdsort.h
	$(CC) -c $(CFLAGS) $< -o $@

bdsort: $(objects)
	cc -o bdsort $(objects)

clean:
	rm -f bdsort $(objects)
