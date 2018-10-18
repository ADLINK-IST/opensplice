man3dir  = $(mandir)/man3
docdir   = $(datadir)/doc/$(PACKAGE)-$(VERSION)
docfiles = \
	$(top_srcdir)/README \
	$(top_srcdir)/AUTHORS \
	$(top_srcdir)/NEWS \
	$(top_srcdir)/COPYING \
	$(top_srcdir)/ChangeLog

.PHONY: doxygen

targets = doxygen

if HAVE_LATEX
targets += $(PACKAGE).pdf
docfiles += $(PACKAGE).pdf
endif

all-local: $(targets)

doxygen: Doxyfile main.doc
	@DOXYGEN@	

#
# If you tell Doxygen to generate latex via the GENERATE_LATEX option
# in Doxyfile then it creates a 'latex' subdirectory containing
# a Makefile to generate the stuff.
#
$(PACKAGE).pdf:
	$(MAKE) -C ./latex pdf
	cp -f ./latex/refman.pdf $(PACKAGE).pdf

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(man3dir)
	@for i in ./man/man3/*.3; do \
		inst=$(PACKAGE)_`basename $$i | sed -e 's/^$(PACKAGE)_//g' `; \
		sed 's,man3/,man3/$(PACKAGE)_,' $$i > $$inst; \
		echo "$(INSTALL_DATA) $$i $(DESTDIR)$(man3dir)/$$inst"; \
		$(INSTALL_DATA) $$inst $(DESTDIR)$(man3dir)/$$inst; \
		rm $$inst; \
	done
	$(mkinstalldirs) $(DESTDIR)$(docdir)
	@for i in $(docfiles); do \
		echo "$(INSTALL_DATA) $$i $(DESTDIR)$(docdir)"; \
		$(INSTALL_DATA) $$i $(DESTDIR)$(docdir); \
	done
	cp -r html $(DESTDIR)$(docdir)

uninstall-local:
	@for i in ./man/man3/*.3; do \
		inst=$(PACKAGE)_`basename $$i | sed -e 's/^$(PACKAGE)_//g' `; \
		echo "rm -f $(DESTDIR)$(man3dir)/$$inst"; \
		rm -f $(DESTDIR)$(man3dir)/$$inst; \
	done
	rm -rf $(DESTDIR)$(docdir)/*

clean-local:
	$(RM) -r latex man html $(PACKAGE).pdf doxygen.log

