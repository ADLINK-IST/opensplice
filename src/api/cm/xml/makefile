include $(OSPL_HOME)/setup/makefiles/makefile.mak

# If SPLICE_TARGET and SPLICE_HOST are equal this a normal makefile.  However, if 
# this is a cross compile (i.e. TARGET and HOST are different), then the cm xml lib 
# is required for both host and target, which is why SPLICE_TARGET is temporarily
# set to SPLICE_HOST and the necessary depenencies on the host get build below

ifeq ($(SPLICE_TARGET), $(SPLICE_HOST))

all compile link gcov qac: bld/$(SPLICE_TARGET)/makefile
	@$(MAKE) -C bld/$(SPLICE_TARGET) $@

analyse:
	@$(ANALYSE) bld/$(SPLICE_TARGET) code

clean:
	@rm -rf bld/$(SPLICE_TARGET)

else

all compile link gcov qac: bld/$(SPLICE_HOST)/makefile bld/$(SPLICE_TARGET)/makefile
	@$(MAKE) -C bld/$(SPLICE_TARGET) $@
ifneq ($(DDS_RRSTORAGE_SUBSYSTEM),)
	@(SPLICE_TARGET=$(SPLICE_HOST); export SPLICE_TARGET; SPLICE_LIBRARY_PATH=$(OSPL_HOME)/lib/$(SPLICE_HOST); export SPLICE_LIBRARY_PATH; SPLICE_EXEC_PATH=$(OSPL_HOME)/exec/$(SPLICE_HOST); export SPLICE_EXEC_PATH; cd $(DDS_RRSTORAGE_SUBSYSTEM); make)
endif
	@(SPLICE_TARGET=$(SPLICE_HOST); export SPLICE_TARGET; SPLICE_LIBRARY_PATH=$(OSPL_HOME)/lib/$(SPLICE_HOST); export SPLICE_LIBRARY_PATH; SPLICE_EXEC_PATH=$(OSPL_HOME)/exec/$(SPLICE_HOST); export SPLICE_EXEC_PATH; cd $(OSPL_HOME)/src/osplcore;make)
	@(SPLICE_TARGET=$(SPLICE_HOST); export SPLICE_TARGET; SPLICE_LIBRARY_PATH=$(OSPL_HOME)/lib/$(SPLICE_HOST); export SPLICE_LIBRARY_PATH; SPLICE_EXEC_PATH=$(OSPL_HOME)/exec/$(SPLICE_HOST); export SPLICE_EXEC_PATH; $(MAKE) -C bld/$(SPLICE_HOST) $@)

analyse:
	@$(ANALYSE) bld/$(SPLICE_HOST) code
	@$(ANALYSE) bld/$(SPLICE_TARGET) code

clean:
	@rm -rf bld/$(SPLICE_HOST)
	@rm -rf bld/$(SPLICE_TARGET)
endif
