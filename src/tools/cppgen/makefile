include $(OSPL_HOME)/setup/makefiles/makefile.mak

ifeq ($(SPLICE_TARGET), $(SPLICE_HOST))

all link: bld/$(SPLICE_TARGET)/makefile
	@$(MAKE) -C bld/$(SPLICE_TARGET) $@

clean:
	@rm -rf bld/$(SPLICE_TARGET)

else

all link: bld/$(SPLICE_HOST)/makefile
	@(SPLICE_TARGET=$(SPLICE_HOST); export SPLICE_TARGET; SPLICE_LIBRARY_PATH=$(OSPL_HOME)/lib/$(SPLICE_HOST); export SPLICE_LIBRARY_PATH;SPLICE_EXEC_PATH=$(OSPL_HOME)/exec/$(SPLICE_HOST); export SPLICE_EXEC_PATH; $(MAKE) -C bld/$(SPLICE_HOST) $@)

clean:
	@(SPLICE_TARGET=$(SPLICE_HOST); export SPLICE_TARGET; $(MAKE) -C $(OSPL_HOME)/src/database $@)
	@rm -rf bld/$(SPLICE_HOST)
endif

.PHONY: reconf
reconf:
	$(shell \
      cd code; \
      rm ../include/y_tab.h y_tab.cpp lex_yy.cpp; \
      $(YACC) -y -d idl.yy; \
      expand y.tab.h > ../include/y_tab.h; \
      expand y.tab.c > y_tab.cpp; \
      $(LEX) idl.ll; \
      expand lex.yy.c > lex_yy.cpp; \
      rm y.tab.h y.tab.c lex.yy.c)
