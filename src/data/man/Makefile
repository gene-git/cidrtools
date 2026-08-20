#
# Man pages builder.
# rst2man provided by python-docutils
#

MAN_PAGES = man1/hostcheck.1.gz

VERSION = $(shell cat ../../version.txt)
DATE = $(shell date +'%B %Y')

default: $(MAN_PAGES)

man1/%.1.gz: %.1.rst
	@echo "$<"
	@/usr/bin/sed -e "s/@VERSION@/$(VERSION)/g" -e "s/@DATE@/$(DATE)/g" $< \
		| rst2man $(OPTS) | /usr/bin/gzip > $@

clean:
	rm -f $(MAN_PAGES)

.PHONY: default clean

