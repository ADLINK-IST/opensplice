# NOTE: This is a temporary solution, has to be improved

.PHONY: all all_no_installer
all:
	sh make_all

all_no_installer:
	sh make_all -noinstaller

clean:
	(cd service_installer; make clean)
	@rm -rf HDE RTS VC
	@rm -f x86.linux2.6*.xml
	@rm -f x86.win32*.xml

# From here, the improvements will be incorporated

PLATFORM = $(firstword $(subst ., ,$(SOURCE_TARGET)))
OS       = $(subst $(PLATFORM).,,$(SOURCE_TARGET))

INSTALL_BUILDER     = /opt/ospldev/installbuilder/bin/builder
OUTPUT_DIR          = output

HDE_PROJECTFILE         = opensplice-HDE.xml
RTS_PROJECTFILE         = opensplice-RTS.xml
PATCHED_HDE_PROJECTFILE = patched_$(HDE_PROJECTFILE)
PATCHED_RTS_PROJECTFILE = patched_$(RTS_PROJECTFILE)

#all: Linux Solaris
Linux: LinuxHDEInstaller LinuxRTSInstaller
Solaris: SolarisHDEInstaller SolarisRTSInstaller
Aix: AIXHDEInstaller AIXRTSInstaller 

HDE_INSTALLER = $(OUTPUT_DIR)/osplhde_$(RELEASE_VERSION)_$(BUILDER_TARGET)_installer.bin
RTS_INSTALLER = $(OUTPUT_DIR)/osplrts_$(RELEASE_VERSION)_$(BUILDER_TARGET)_installer.bin


LinuxHDEInstaller: SOURCE_TARGET=x86.linux2.6
LinuxHDEInstaller: BUILDER_TARGET=linux
#LinuxHDEInstaller: PATCHED_HDE_PROJECTFILE=$(SOURCE_TARGET)_$(HDE_PROJECTFILE)
LinuxHDEInstaller: BUILDER_PROJECT=$(PATCHED_HDE_PROJECTFILE)
LinuxHDEInstaller: $(HDE_INSTALLER)

LinuxRTSInstaller: SOURCE_TARGET=x86.linux2.6
LinuxRTSInstaller: BUILDER_TARGET=linux
#LinuxRTSInstaller: PATCHED_RTS_PROJECTFILE=$(SOURCE_TARGET)_$(RTS_PROJECTFILE)
LinuxRTSInstaller: BUILDER_PROJECT=$(PATCHED_RTS_PROJECTFILE)
LinuxRTSInstaller: $(RTS_INSTALLER)

SolarisHDEInstaller: SOURCE_TARGET=sparc.solaris8
SolarisHDEInstaller: BUILDER_TARGET=solaris-sparc
#SolarisHDEInstaller: PATCHED_HDE_PROJECTFILE=$(SOURCE_TARGET)_$(HDE_PROJECTFILE)
SolarisHDEInstaller: BUILDER_PROJECT=$(PATCHED_HDE_PROJECTFILE)
SolarisHDEInstaller: $(HDE_INSTALLER)

SolarisRTSInstaller: SOURCE_TARGET=sparc.solaris8
SolarisRTSInstaller: BUILDER_TARGET=solaris-sparc
#SolarisRTSInstaller: PATCHED_RTS_PROJECTFILE=$(SOURCE_TARGET)_$(RTS_PROJECTFILE)
SolarisRTSInstaller: BUILDER_PROJECT=$(PATCHED_RTS_PROJECTFILE)
SolarisRTSInstaller: $(RTS_INSTALLER)

AIXHDEInstaller: SOURCE_TARGET=PPC5plus.AIX5.3
AIXHDEInstaller: BUILDER_TARGET=aix
#AIXHDEInstaller: PATCHED_HDE_PROJECTFILE=$(SOURCE_TARGET)_$(HDE_PROJECTFILE)
AIXHDEInstaller: BUILDER_PROJECT=$(PATCHED_HDE_PROJECTFILE)
AIXHDEInstaller: $(HDE_INSTALLER)

AIXRTSInstaller: SOURCE_TARGET=PPC5plus.AIX5.3
AIXRTSInstaller: BUILDER_TARGET=aix
#AIXRTSInstaller: PATCHED_RTS_PROJECTFILE=$(SOURCE_TARGET)_$(RTS_PROJECTFILE)
AIXRTSInstaller: BUILDER_PROJECT=$(PATCHED_RTS_PROJECTFILE)
AIXRTSInstaller: $(RTS_INSTALLER)


$(PATCHED_HDE_PROJECTFILE): RELEASE $(HDE_PROJECTFILE)
	( \
	sed 's/__VERSION__/$(RELEASE_VERSION)/g' $(HDE_PROJECTFILE) > $@ \
	)

#$(PATCHED_HDE_PROJECTFILE): RELEASE $(HDE_PROJECTFILE)
#	( \
#	sed 's/__VERSION__/$(RELEASE_VERSION)/g' $(HDE_PROJECTFILE) > $@ \
#	sed 's/__INSTTYPE__/HDE/g' $(HDE_PROJECTFILE) > $@ \
#	sed 's/__INSTTYPEFULL/Hoset Development Environment/g' $(HDE_PROJECTFILE) > $@ \
#	sed 's/__PLATFORM__/$(SOURCE_TARGET)/g' $(HDE_PROJECTFILE) > $@ \
#	sed 's/__BUILDERPLATFORM__/$(BUILDER_TARGET)/g' $(HDE_PROJECTFILE) > $@ \
#	)

$(PATCHED_RTS_PROJECTFILE): RELEASE $(RTS_PROJECTFILE)
	@sed 's/__VERSION__/$(RELEASE_VERSION)/g' $(RTS_PROJECTFILE) > $@

$(HDE_INSTALLER): $(PATCHED_HDE_PROJECTFILE) \
	$(wildcard $(OSPL_HOME)/lib/$(SOURCE_TARGET)/*) \
	$(wildcard $(OSPL_HOME)/jar/$(SOURCE_TARGET)/*) \
	$(wildcard $(OSPL_HOME)/exec/$(SOURCE_TARGET)/*) \
	$(wildcard $(OSPL_HOME)/etc/idlpp/TAO/corba*) \
	$(wildcard $(OSPL_HOME)/etc/idlpp/SAC/sac*) \
	$(wildcard $(OSPL_HOME)/etc/ospl.xml) \
	$(wildcard $(OSPL_HOME)/src/api/dcps/corbac++/include/*.[hi]) \
	$(wildcard $(OSPL_HOME)/src/api/dcps/sac/include/*.h) \
	$(wildcard $(OSPL_HOME)/src/abstraction/os/posix/include/os_*.h) \
	$(wildcard $(OSPL_HOME)/src/abstraction/os/$(OS)/include/os_*.h) \
	$(wildcard $(OSPL_HOME)/src/abstraction/os/$(OS)/include/pa_*.h) \
	$(wildcard $(OSPL_HOME)/src/abstraction/os/$(OS)/vxworks/*.h) \
	$(wildcard $(OSPL_HOME)/src/abstraction/os/$(OS)/*.h) \
	$(wildcard $(OSPL_HOME)/src/abstraction/os/$(OS)/vxworks/sys/*.h) \
	$(wildcard $(OSPL_HOME)/src/abstraction/os/include/os_*.h) \
	$(wildcard $(OSPL_HOME)/src/abstraction/os/include/pa_*.h) \
        $(wildcard $(OSPL_HOME)/src/database/database/include/[cq]_*.h) \
        $(wildcard $(OSPL_HOME)/src/kernel/include/v_*.h) \
        $(wildcard $(OSPL_HOME)/src/user/include/u_*.h) \
        $(wildcard $(OSPL_HOME)/src/include/kernelModule.h) \
	$(wildcard $(OSPL_HOME)/examples/dcps/C++/TAO/*.cpp) \
	$(wildcard $(OSPL_HOME)/examples/dcps/C++/TAO/*.idl) \
	$(wildcard $(OSPL_HOME)/examples/dcps/C++/TAO/BUILD) \
	$(wildcard $(OSPL_HOME)/examples/dcps/C++/TAO/RUN) \
	$(wildcard $(OSPL_HOME)/examples/dcps/C/SAC/*.c) \
	$(wildcard $(OSPL_HOME)/examples/dcps/C/SAC/*.idl) \
	$(wildcard $(OSPL_HOME)/examples/dcps/C/SAC/BUILD) \
	$(wildcard $(OSPL_HOME)/examples/dcps/C/SAC/BUILD_vxworks) \
	$(wildcard $(OSPL_HOME)/examples/dcps/C/SAC/BUILD_int509) \
	$(wildcard $(OSPL_HOME)/examples/dcps/C/SAC/RUN) \
	$(wildcard images/*.png)
	@( \
	sh ./make_hde_mirror $(SOURCE_TARGET) $(RELEASE_VERSION) $(RELEASE_DATE); \
	mkdir -p $(OUTPUT_DIR); \
	$(INSTALL_BUILDER) build $(PATCHED_HDE_PROJECTFILE) $(BUILDER_TARGET) \
	)


$(RTS_INSTALLER): $(PATCHED_RTS_PROJECTFILE) \
	$(wildcard $(OSPL_HOME)/lib/$(SOURCE_TARGET)/*) \
	$(wildcard $(OSPL_HOME)/jar/$(SOURCE_TARGET)/*) \
	$(wildcard $(OSPL_HOME)/exec/$(SOURCE_TARGET)/*) \
	$(wildcard $(OSPL_HOME)/etc/ospl.xml) \
	$(wildcard images/*.png)
	@( \
	sh ./make_rts_mirror $(SOURCE_TARGET) $(RELEASE_VERSION) $(RELEASE_DATE); \
	mkdir -p $(OUTPUT_DIR); \
	$(INSTALL_BUILDER) build $(PATCHED_RTS_PROJECTFILE) $(BUILDER_TARGET) \
	)
