#
# Copyright 2011 Nathan Fiedler. All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.
#

SUBDIRS = src

.PHONY: all clean $(SUBDIRS)

all: src

$(SUBDIRS):
	$(MAKE) -C $@

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
