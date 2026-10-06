# docs/Makefile
SPHINXOPTS    ?=
SPHINXBUILD   ?= sphinx-build
SOURCEDIR     = .
BUILDDIR      = _build
PKG           = cidrtools

SOURCES = conf.py index.rst c-api.rst Readme.rst ../../version.txt \
          $(wildcard lib/*.c) \
          $(wildcard lib/*.h)

MAN_READY = ./man-ready/hostcheck.1.rst

.PHONY: help Makefile clean html latexpdf

HTML_EXCL   = --exclude _sources/ --exclude .buildinfo  --exclude .doctrees/

html: $(BUILDDIR)/html/index.html
latexpdf: $(PKG).pdf

help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

$(BUILDDIR)/html/index.html: $(SOURCES) $(MAN_READY)
	@$(SPHINXBUILD) -b html "$(SOURCEDIR)" "$(BUILDDIR)/html" $(SPHINXOPTS) $(O)
	@$(SPHINXBUILD) -b html "$(SOURCEDIR)" "$(BUILDDIR)/html" $(SPHINXOPTS) $(O)
	@rsync -a $(HTML_EXCL) _build/html .
	@echo "HTML doc build complete. Path: $@"

$(PKG).pdf: $(SOURCES) $(MAN_READY)
	@$(SPHINXBUILD) -b latex "$(SOURCEDIR)" "$(BUILDDIR)/latex" $(SPHINXOPTS) $(O)
	@echo " .. xelatex "
	$(MAKE) -C "$(BUILDDIR)/latex" all-pdf LATEXOPTS="-interaction=nonstopmode" PDFLATEX=xelatex
	$(MAKE) -C "$(BUILDDIR)/latex" all-pdf LATEXOPTS="-interaction=nonstopmode" PDFLATEX=xelatex
	@rsync -av $(BUILDDIR)/latex/$(PKG).pdf .
	@touch $(PKG).pdf
	@echo "PDF doc build complete. Path: $@"

man-ready/%.1.rst: ../man/%.1.rst
	@/usr/bin/mkdir -p ./man-ready
	@echo "$<"
	@/usr/bin/grep -Ev '^:Author|^:Manual|^:Date|^:Version' $< > $@

clean:
	rm -rf $(BUILDDIR)
	rm -f $(PKG).pdf
	@rm -rf ./html

