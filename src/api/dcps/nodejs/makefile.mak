# included by bld/$(SPLICE_TARGET)/makefile

include $(OSPL_HOME)/setup/makefiles/target.mak

# Check for changes in the js files under lib directory
SRC_FILES=$(wildcard ../../lib/**/*.js ../../examples/**/*.js)

# Only invoke nodejs dcps api packaging if prerequisites met from configure
TARGET_FILE=
ifeq "$(INCLUDE_API_DCPS_NODEJS)" "yes"
	TARGET_FILE = vortexdds-0.1.0.tgz
endif

all link: $(TARGET_FILE)

# For the win7e build, modify the path temporarily
ifeq "$(shell uname)" "CYGWIN_NT-6.1-WOW"
    TEMP_PATH:="$(OSPL_HOME):$(NODEJS_HOME):/usr/local/bin:/usr/bin:/bin:/cygdrive/c/Windows/system32:/cygdrive/c/Windows:/cygdrive/c/Windows/System32/WindowsPowerShell/v1.0"
else
    TEMP_PATH:="$(PATH)"
endif

# npm install the required modules, run eslint and create a nodejs package
# Launch npm directly from its installed path
npm_path=
ifeq "$(OS)" "linux"
    npm_path:=$(NODEJS_HOME)/bin/npm
endif
ifeq "$(OS)" "win"
    npm_path:=$(NODEJS_HOME)/npm
endif

$(TARGET_FILE): $(SRC_FILES)
	export PATH=$(TEMP_PATH); \
	mkdir -p src; \
	cp -r ../../examples src; \
	cp -r ../../lib src; \
	cp -r ../../test_data src; \
	cp ../../makefile.mak src; \
	cp ../../makefile src; \
	cp ../../package.json src; \
	cp ../../readme.md src; \
	cp ../../.gitignore src; \
	cp ../../.eslintrc.js src; \
	../.././make_copyright; \
	$(npm_path) install; \
	$(npm_path) run lint; \
	$(npm_path) run docs; \
	$(npm_path) pack ./src; \
	rm -r src

