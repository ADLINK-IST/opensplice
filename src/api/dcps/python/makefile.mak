#
# included by bld/$(SPLICE_TARGET)/makefile

# Don't use inherited C compile rules from target.mak
C_FILES=

include $(OSPL_HOME)/setup/makefiles/target.mak

# This list of files is what will be included in the source assembly, as well as
# what is needed for the wheel packaging.
SOURCE_FILES = dds.pxd dds.pyx ddsutil.py setup.py README.txt

# The source assembly is unconditionally picked up by the HDE install for all platforms
SOURCE_ASSEMBLY_DIR = src
SOURCE_ASSEMBLY = $(addprefix $(SOURCE_ASSEMBLY_DIR)/, $(SOURCE_FILES))

# Only invoke python3 binary wheel packaging if prerequisites are met from configure
# This is picked up by HDE and RTS installs for select platforms
BINARY_PACKAGE = dist/*.whl
ifeq "$(INCLUDE_API_DCPS_PYTHON)" "yes"
	TARGET = $(BINARY_PACKAGE)
else
	TARGET = $(SOURCE_ASSEMBLY)
endif

all link: $(TARGET)

$(SOURCE_ASSEMBLY): $(addprefix $(CODE_DIR)/, $(SOURCE_FILES))
	@mkdir -p $(SOURCE_ASSEMBLY_DIR)
	cp $^ $(SOURCE_ASSEMBLY_DIR)

# For every python install defined in PYTHON3_EXECS, create a wheel for each
$(BINARY_PACKAGE): $(SOURCE_ASSEMBLY)
	@cp -r $(SOURCE_ASSEMBLY_DIR)/* .
	@for pyexec in $$(echo "$$PYTHON3_EXECS" | tr ":" "\n"); do \
		$$pyexec setup.py build_ext --inplace bdist_wheel; \
	done
